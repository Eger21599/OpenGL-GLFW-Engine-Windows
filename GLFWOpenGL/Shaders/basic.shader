  #shader vertex
  #version 460

  layout (location = 0) in vec3 aPos;
  layout (location = 1) in vec3 aNormal;
  layout (location = 2) in vec2 aTexCoords;

  out vec2 TexCoords;
  out vec3 Normal;
  out vec3 FragPos;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 projection;

  void main()
  {
      TexCoords = aTexCoords;
      Normal = mat3(transpose(inverse(model))) * aNormal;
      FragPos = vec3(model * vec4(aPos, 1.0f));

      gl_Position = projection * view * model * vec4(aPos, 1.0);
  }

  #shader fragment
  #version 460

  out vec4 FragColor;

  in vec2 TexCoords;
  in vec3 Normal;
  in vec3 FragPos;

  struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
  };

  struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
  };

  struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
  };
  #define NR_POINT_LIGHTS 10
  #define NR_SPOT_LIGHTS 10

  uniform DirLight dirLight;
  uniform PointLight pointLights[NR_POINT_LIGHTS];
  uniform SpotLight spotLights[NR_SPOT_LIGHTS];

  uniform int numberOfPointLights;
  uniform int numberOfSpotLights;
  uniform sampler2D texture_diffuse1;
  uniform sampler2D texture_specular1;
  uniform sampler2D texture_normal1;
  uniform float shininess;
  uniform vec3 viewPos;

  vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
  vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
  vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

  void main()
  {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    if(numberOfPointLights > 0)
    {
        for (int i = 0; i < numberOfPointLights; i++)
            result = CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    if(numberOfSpotLights > 0)
    {
      for (int j = 0; j < numberOfSpotLights; j++)
          result += CalcSpotLight(spotLights[j], norm, FragPos, viewDir);
    }

    FragColor = vec4(result, 1.0);
  }

  vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
  {
    vec3 lightDir = normalize(light.direction - FragPos);
    float sDotN = max(dot(lightDir, normal), 0.0f);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = vec3(0.0f);
    if (sDotN > 0.0f)
    {
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
      specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    }
    return (ambient + diffuse + specular);
  }

  vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
  {
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
  }

  vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
  {
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
  }