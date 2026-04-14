#version 330 core
in vec3 vWorldPosition;

uniform vec3 uCameraPosition;
uniform float uFadeDistance;

out vec4 FragColor;

void main()
{
    vec2 coord = vWorldPosition.xz;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float minorLine = 1.0 - min(min(grid.x, grid.y), 1.0);

    vec2 majorCoord = coord / 10.0;
    vec2 majorDerivative = fwidth(majorCoord);
    vec2 majorGrid = abs(fract(majorCoord - 0.5) - 0.5) / majorDerivative;
    float majorLine = 1.0 - min(min(majorGrid.x, majorGrid.y), 1.0);

    float axisXLine = 1.0 - min(abs(vWorldPosition.z) / derivative.y, 1.0);
    float axisZLine = 1.0 - min(abs(vWorldPosition.x) / derivative.x, 1.0);

    float dist = length(vWorldPosition.xz - uCameraPosition.xz);
    float fade = 1.0 - smoothstep(uFadeDistance * 0.6, uFadeDistance, dist);

    vec3 color = vec3(0.25, 0.25, 0.28);
    float alpha = minorLine * 0.25;

    color = mix(color, vec3(0.4, 0.4, 0.45), majorLine * 0.8);
    alpha = max(alpha, majorLine * 0.35);

    color = mix(color, vec3(0.7, 0.15, 0.15), axisXLine);
    alpha = max(alpha, axisXLine * 0.7);

    color = mix(color, vec3(0.15, 0.15, 0.7), axisZLine);
    alpha = max(alpha, axisZLine * 0.7);

    alpha *= fade;
    if (alpha < 0.005) {
        discard;
    }

    FragColor = vec4(color, alpha);
}
