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
