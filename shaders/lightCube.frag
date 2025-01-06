#version 410 core
in vec2 fTexCoords;
out vec4 fColor;

uniform sampler2D diffuseTexture;

void main() 
{
//    vec3 color = min(texture(diffuseTexture, fTexCoords).rgb, 1.0f);
    vec3 color = vec3(1.0f, 0.5f, 0.4f);
    fColor = vec4(color,1.0f);
}
