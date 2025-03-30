#version 330

in vec3 fragNormal;
in vec3 fragPosition;

uniform vec3 lightPos;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = ambientColor;
    vec3 diffuse = diff * diffuseColor;
    
    FragColor = vec4(ambient + diffuse, 1.0);
}
