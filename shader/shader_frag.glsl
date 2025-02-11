#version 330 core

#define MAX_LIGHTS 128

float specularStrength = 0.3;
vec3 gradiant = vec3(1.0, 1.0, 1.0);

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct Light {
    vec3 color;
    vec3 pos;
    float ambient;

    // darker over distance
    float constant;
    float linear;
    float quadratic;
};

uniform int lightCount;
uniform Light lights[MAX_LIGHTS];

uniform vec3 viewPos;
uniform sampler2D texture1;

out vec4 FragColor;


vec3 CalcLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.pos - fragPos);

    // ambient
    vec3 ambient = light.ambient * gradiant;

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * gradiant;

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0);
    vec3 specular = specularStrength * spec * gradiant;

    // attenuation
    float distance    = length(light.pos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * light.color * vec3(texture(texture1, TexCoord));
}


void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0, 0.0, 0.0);

    for(int i = 0; i < lightCount; i++)
        result += CalcLight(lights[i], norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}