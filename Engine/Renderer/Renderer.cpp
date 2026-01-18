#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"
#include "../Core/Log.h"

#include <GL/glew.h>

namespace Xi {

    // Default shader sources
    static const char* s_DefaultVertexShader = R"(
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoord;
out mat3 v_TBN;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    v_Normal = normalize(normalMatrix * a_Normal);

    vec3 T = normalize(normalMatrix * a_Tangent);
    vec3 N = v_Normal;
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    v_TBN = mat3(T, B, N);

    v_TexCoord = a_TexCoord;

    gl_Position = u_Projection * u_View * worldPos;
}
)";

    static const char* s_DefaultFragmentShader = R"(
#version 450 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;
in mat3 v_TBN;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec4 u_AlbedoColor;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AO;
uniform vec3 u_Emissive;

uniform int u_HasAlbedoMap;
uniform int u_HasNormalMap;
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;

// Lights
uniform int u_NumLights;
uniform vec3 u_LightPositions[8];
uniform vec3 u_LightDirections[8];
uniform vec3 u_LightColors[8];
uniform float u_LightIntensities[8];
uniform int u_LightTypes[8]; // 0 = directional, 1 = point, 2 = spot

const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main() {
    vec4 albedo = u_AlbedoColor;
    if (u_HasAlbedoMap == 1) {
        albedo *= texture(u_AlbedoMap, v_TexCoord);
    }

    vec3 N = normalize(v_Normal);
    if (u_HasNormalMap == 1) {
        N = texture(u_NormalMap, v_TexCoord).rgb;
        N = N * 2.0 - 1.0;
        N = normalize(v_TBN * N);
    }

    vec3 V = normalize(u_CameraPos - v_WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, u_Metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < u_NumLights && i < 8; i++) {
        vec3 L;
        float attenuation = 1.0;

        if (u_LightTypes[i] == 0) {
            // Directional light
            L = normalize(-u_LightDirections[i]);
        } else {
            // Point or spot light
            L = normalize(u_LightPositions[i] - v_WorldPos);
            float distance = length(u_LightPositions[i] - v_WorldPos);
            attenuation = 1.0 / (distance * distance);
        }

        vec3 H = normalize(V + L);
        vec3 radiance = u_LightColors[i] * u_LightIntensities[i] * attenuation;

        float NDF = DistributionGGX(N, H, u_Roughness);
        float G = GeometrySmith(N, V, L, u_Roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - u_Metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo.rgb * u_AO;
    vec3 color = ambient + Lo + u_Emissive;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, albedo.a);
}
)";

    static const char* s_UnlitVertexShader = R"(
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}
)";

    static const char* s_UnlitFragmentShader = R"(
#version 450 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform vec4 u_AlbedoColor;
uniform int u_HasAlbedoMap;
uniform sampler2D u_AlbedoMap;

void main() {
    vec4 color = u_AlbedoColor;
    if (u_HasAlbedoMap == 1) {
        color *= texture(u_AlbedoMap, v_TexCoord);
    }
    FragColor = color;
}
)";

    static const char* s_SpriteVertexShader = R"(
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}
)";

    static const char* s_SpriteFragmentShader = R"(
#version 450 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform vec4 u_AlbedoColor;
uniform int u_HasAlbedoMap;
uniform sampler2D u_AlbedoMap;

void main() {
    vec4 color = u_AlbedoColor;
    if (u_HasAlbedoMap == 1) {
        color *= texture(u_AlbedoMap, v_TexCoord);
    }
    if (color.a < 0.01) discard;
    FragColor = color;
}
)";

    Renderer::Renderer() = default;
    Renderer::~Renderer() = default;

    void Renderer::Init() {
        XI_LOG_INFO("Renderer initializing...");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        CreateDefaultShaders();

        // Set default camera
        m_Camera.SetPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

        XI_LOG_INFO("Renderer initialized");
    }

    void Renderer::Shutdown() {
        m_DefaultShader.reset();
        m_UnlitShader.reset();
        m_SpriteShader.reset();
    }

    void Renderer::CreateDefaultShaders() {
        m_DefaultShader = std::make_shared<Shader>();
        if (!m_DefaultShader->LoadFromSource(s_DefaultVertexShader, s_DefaultFragmentShader)) {
            XI_LOG_ERROR("Failed to create default shader");
        }

        m_UnlitShader = std::make_shared<Shader>();
        if (!m_UnlitShader->LoadFromSource(s_UnlitVertexShader, s_UnlitFragmentShader)) {
            XI_LOG_ERROR("Failed to create unlit shader");
        }

        m_SpriteShader = std::make_shared<Shader>();
        if (!m_SpriteShader->LoadFromSource(s_SpriteVertexShader, s_SpriteFragmentShader)) {
            XI_LOG_ERROR("Failed to create sprite shader");
        }
    }

    void Renderer::BeginFrame() {
        ResetStats();
        m_RenderQueue.Clear();
    }

    void Renderer::EndFrame() {
        m_RenderQueue.Sort(m_Camera.GetPosition());

        // Render opaque objects
        for (const auto& cmd : m_RenderQueue.GetOpaqueCommands()) {
            if (cmd.material && cmd.mesh) {
                DrawMesh(*cmd.mesh, *cmd.material, cmd.transform);
            }
        }

        // Render transparent objects
        glDepthMask(GL_FALSE);
        for (const auto& cmd : m_RenderQueue.GetTransparentCommands()) {
            if (cmd.material && cmd.mesh) {
                DrawMesh(*cmd.mesh, *cmd.material, cmd.transform);
            }
        }
        glDepthMask(GL_TRUE);

        ClearLights();
    }

    void Renderer::SetCamera(const Camera& camera) {
        m_Camera = camera;
    }

    void Renderer::Submit(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const glm::mat4& transform) {
        RenderCommand cmd;
        cmd.mesh = mesh;
        cmd.material = material;
        cmd.transform = transform;
        cmd.transparent = material ? material->transparent : false;
        m_RenderQueue.Submit(cmd);
    }

    void Renderer::AddLight(const LightData& light) {
        if (m_Lights.size() < 8) {
            m_Lights.push_back(light);
        }
    }

    void Renderer::ClearLights() {
        m_Lights.clear();
    }

    void Renderer::DrawMesh(Mesh& mesh, Material& material, const glm::mat4& transform) {
        material.Bind();

        Shader* shader = material.GetShader().get();
        if (!shader) {
            shader = m_DefaultShader.get();
            shader->Bind();
        }

        shader->SetMat4("u_Model", transform);
        shader->SetMat4("u_View", m_Camera.GetViewMatrix());
        shader->SetMat4("u_Projection", m_Camera.GetProjectionMatrix());
        shader->SetVec3("u_CameraPos", m_Camera.GetPosition());

        SetupLightUniforms(*shader);

        mesh.Draw();

        material.Unbind();

        m_Stats.drawCalls++;
        m_Stats.triangles += mesh.GetIndexCount() / 3;
    }

    void Renderer::SetupLightUniforms(Shader& shader) {
        shader.SetInt("u_NumLights", static_cast<int>(m_Lights.size()));

        for (size_t i = 0; i < m_Lights.size() && i < 8; i++) {
            std::string prefix = "u_LightPositions[" + std::to_string(i) + "]";
            shader.SetVec3(prefix, m_Lights[i].position);

            prefix = "u_LightDirections[" + std::to_string(i) + "]";
            shader.SetVec3(prefix, m_Lights[i].direction);

            prefix = "u_LightColors[" + std::to_string(i) + "]";
            shader.SetVec3(prefix, m_Lights[i].color);

            prefix = "u_LightIntensities[" + std::to_string(i) + "]";
            shader.SetFloat(prefix, m_Lights[i].intensity);

            prefix = "u_LightTypes[" + std::to_string(i) + "]";
            shader.SetInt(prefix, static_cast<int>(m_Lights[i].type));
        }
    }

    void Renderer::ResetStats() {
        m_Stats.drawCalls = 0;
        m_Stats.triangles = 0;
    }

}
