#version 330 core
in vec3 vWorldNormal;
in vec3 vWorldPosition;
in vec4 vLightSpacePosition;
in vec2 vTexCoord;
uniform sampler2D uTexture_diffuse;
uniform sampler2D uTexture_emissive;
uniform sampler2D uShadowMap;
uniform int uHasTexture_diffuse;
uniform int uHasTexture_emissive;
uniform int uHasDirectionalLight;
uniform int uDirectionalLightCastsShadows;
uniform int uPointLightCount;
uniform vec3 uBaseColor;
uniform vec3 uEmissiveColor;
uniform vec3 uViewPosition;
uniform vec3 uDirectionalLightDirection;
uniform vec3 uDirectionalLightColor;
uniform float uDirectionalLightIntensity;
uniform vec3 uPointLightPositions[4];
uniform vec3 uPointLightColors[4];
uniform float uPointLightIntensities[4];
uniform float uPointLightRanges[4];
uniform float uRoughness;
uniform float uMetallic;
uniform float uAlphaCutoff;
uniform vec3 uSelectionTint;
uniform float uSelectionMix;
out vec4 FragColor;

float ComputeShadowFactor(vec4 lightSpacePosition, vec3 normal, vec3 lightDirection) {
    vec3 projected = lightSpacePosition.xyz / max(lightSpacePosition.w, 0.0001);
    projected = projected * 0.5 + 0.5;
    if (projected.z > 1.0 || projected.x < 0.0 || projected.x > 1.0 || projected.y < 0.0 || projected.y > 1.0) {
        return 0.0;
    }

    float currentDepth = projected.z;
    float bias = max(0.0008, 0.0035 * (1.0 - max(dot(normal, lightDirection), 0.0)));

    // --- PCF (Percentage-Closer Filtering) ---
    // Sample the shadow map in a 3x3 grid around the current texel.
    // This smooths shadow edges by averaging 9 depth comparisons.
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closestDepth = texture(uShadowMap, projected.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main() {
    // One primary shadowed directional light plus a few point lights keeps the renderer understandable.
    vec4 diffuseSample = vec4(1.0);
    if (uHasTexture_diffuse == 1) {
        diffuseSample = texture(uTexture_diffuse, vTexCoord);
    }
    if (diffuseSample.a < uAlphaCutoff) {
        discard;
    }

    vec3 albedo = uBaseColor * diffuseSample.rgb;
    vec3 emissive = uEmissiveColor;
    if (uHasTexture_emissive == 1) {
        emissive *= texture(uTexture_emissive, vTexCoord).rgb;
    }

    vec3 normal = normalize(vWorldNormal);
    vec3 viewDirection = normalize(uViewPosition - vWorldPosition);
    float ambient = 0.25;
    float shininess = mix(4.0, 64.0, 1.0 - uRoughness);
    float specularStrength = mix(0.04, 1.0, uMetallic);
    vec3 litColor = albedo * ambient;

    if (uHasDirectionalLight == 1) {
        vec3 lightDirection = normalize(-uDirectionalLightDirection);
        vec3 halfVector = normalize(lightDirection + viewDirection);
        float diffuse = max(dot(normal, lightDirection), 0.0);
        float specular = pow(max(dot(normal, halfVector), 0.0), shininess) * specularStrength;
        float shadow = uDirectionalLightCastsShadows == 1
            ? ComputeShadowFactor(vLightSpacePosition, normal, lightDirection)
            : 0.0;
        litColor +=
            (1.0 - shadow) *
            (albedo * diffuse + vec3(specular)) *
            uDirectionalLightColor *
            uDirectionalLightIntensity;
    }

    for (int i = 0; i < uPointLightCount; ++i) {
        vec3 toLight = uPointLightPositions[i] - vWorldPosition;
        float distanceToLight = length(toLight);
        if (distanceToLight > uPointLightRanges[i]) {
            continue;
        }

        vec3 lightDirection = normalize(toLight);
        vec3 halfVector = normalize(lightDirection + viewDirection);
        float diffuse = max(dot(normal, lightDirection), 0.0);
        float specular = pow(max(dot(normal, halfVector), 0.0), shininess) * specularStrength;
        float attenuation = 1.0 - clamp(distanceToLight / max(uPointLightRanges[i], 0.0001), 0.0, 1.0);
        attenuation *= attenuation;
        litColor +=
            (albedo * diffuse + vec3(specular)) *
            uPointLightColors[i] *
            uPointLightIntensities[i] *
            attenuation;
    }

    litColor += emissive;
    litColor = mix(litColor, uSelectionTint, uSelectionMix);
    FragColor = vec4(litColor, diffuseSample.a);
}