#version 450

layout(location = 0) in vec3 worldNormal;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform DrawConstants {
    mat4 viewProjection;
    vec4 lightDirection;
    vec4 lightColor;
    vec4 baseColor;
    vec4 lighting;
} scene;

void main() {
    float diffuse = max(dot(normalize(worldNormal), -scene.lightDirection.xyz), 0.0);
    vec3 illumination = vec3(scene.lighting.y)
        + scene.lightColor.rgb * scene.lighting.x * diffuse;
    outColor = vec4(scene.baseColor.rgb * illumination, 1.0);
}
