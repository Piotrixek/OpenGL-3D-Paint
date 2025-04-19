#version 330 core
out vec4 FragColor;

in vec3 FragPos; // Position from vertex shader (world space)
in vec3 Normal;  // Normal from vertex shader (world space)

struct Material {
    vec4 ambient;   // Use vec4 for color, potentially alpha later
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

struct Light {
    vec3 position;  // Assuming point light in world space
    vec3 color;     // Use vec3 for light color intensity

    // Intensity components (could be part of color or separate)
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos; // Camera position in world space

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    // Ambient
    vec3 ambient = light.ambient * material.ambient.rgb * light.color; // Modulate by light color

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse.rgb) * light.color; // Modulate by light color

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    // Alternative: Blinn-Phong
    // vec3 halfwayDir = normalize(lightDir + viewDir);
    // float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular.rgb) * light.color; // Modulate by light color

    vec3 result = ambient + diffuse + specular;

    // Use material diffuse alpha for overall transparency
    FragColor = vec4(result, material.diffuse.a);
}