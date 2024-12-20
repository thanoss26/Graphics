#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 modelview;
uniform mat4 projection;

out vec3 fragNormal;
out vec3 fragPosition;

void main() {
    vec4 worldPosition = modelview * vec4(position, 1.0);
    fragPosition = worldPosition.xyz;
    fragNormal = mat3(transpose(inverse(modelview))) * normal;
    gl_Position = projection * worldPosition;
}