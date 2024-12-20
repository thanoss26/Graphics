#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;

uniform vec4 baseColor = vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4 highlightColor = vec4(1.0, 0.5, 0.0, 1.0);
uniform int isHighlighted;

out vec4 color;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);

    vec4 currentColor = isHighlighted == 1 ? highlightColor : baseColor;
    color = currentColor * (0.2 + 0.8 * diffuse);
}