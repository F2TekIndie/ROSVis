#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 0) out vec3 worldNormal;

layout(push_constant) uniform DrawConstants {
    mat4 viewProjection;
    vec4 lightDirection;
    vec4 lightColor;
    vec4 baseColor;
    vec4 lighting;
} scene;

void main() {
    // Mesh vertices and normals are currently already in world coordinates.
    gl_Position = scene.viewProjection * vec4(position, 1.0);
    worldNormal = normal;
}
