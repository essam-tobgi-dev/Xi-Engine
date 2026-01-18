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
