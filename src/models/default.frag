#version 330 core
out vec4 FragColor;

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform sampler2D u_DiffuseTexture;
uniform vec3 u_ViewPos;
uniform vec3 u_LightPos; // Пример для точечного источника света
uniform vec3 u_LightColor;

void main()
{
    // Ambient (фоновое освещение)
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_LightColor;

    // Diffuse (диффузное освещение - простой пример)
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;

    vec4 texColor = texture(u_DiffuseTexture, v_TexCoord);
    FragColor = vec4((ambient + diffuse) * texColor.rgb, texColor.a);
}