#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat4 uLightSpaceMatrix;
uniform vec2 uUvTiling;
uniform vec2 uUvOffset;
out vec3 vWorldNormal;
out vec3 vWorldPosition;
out vec4 vLightSpacePosition;
out vec2 vTexCoord;
void main() {
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vWorldNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vWorldPosition = worldPosition.xyz;
    vLightSpacePosition = uLightSpaceMatrix * worldPosition;
    vTexCoord = aTexCoord * uUvTiling + uUvOffset;
    gl_Position = uMVP * vec4(aPosition, 1.0);
}