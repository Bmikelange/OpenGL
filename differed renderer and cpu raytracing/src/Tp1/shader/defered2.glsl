#version 450

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;

out vec2 TexCoords2;

void main()
{
    gl_Position= vec4(position, 1);
    TexCoords2=texcoord;
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;
  
in vec2 TexCoords2;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform mat4 invMatrix;
uniform vec3 camera_position;
uniform samplerCube texture0;

const int NR_LIGHTS = 52;
uniform vec3 lightspos[NR_LIGHTS];
uniform vec3 lightscol[NR_LIGHTS];
uniform mat4 viewInvMatrix;

const vec3 emission= vec3(1);
const float k= 3;
const float alpha=3;
const float PI= 3.14159265359;
const float h=3;
const float r=10;
float a=0.2;
float f=800;


vec3 ComputeLightSphere(int i,vec3 Albedo, vec3 FragPos,vec3 normal,vec3 camera,float ray, vec3 lighting)
{
    vec3 lightColor2 = lightscol[i];
    vec3 ambient2 = 0.5 * Albedo;
    vec3 dist2 = lightspos[i]- FragPos;
    vec3 lightDir2 = normalize(lightspos[i]- FragPos);
    if(length(dist2)<=ray)
    {
        float attenuation=clamp(2.0-2*length(dist2)/ray,0.0,1.0);
        float diff2 = max(dot(lightDir2, normal), 0.0);
        vec3 diffuse2 = diff2 * lightColor2;
        vec3 viewDir2 = normalize(camera - FragPos);
        float spec2 = 0.0;
        vec3 halfwayDir2 = normalize(lightDir2 + viewDir2);  
        spec2 = pow(max(dot(normal, halfwayDir2), 0.0), 64.0);
        vec3 specular2 = spec2 * lightColor2;    
        lighting+=attenuation *(ambient2 + (diffuse2 + specular2)) * Albedo;
    }
    return lighting;
}

vec3 ComputeLight(int i,vec3 Albedo, vec3 FragPos,vec3 normal,vec3 camera,vec3 lighting)
{
    vec3 lightColor = lightscol[i];
    vec3 ambient = 0.5 * Albedo;
    vec3 lightDir = normalize(lightspos[0]- FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(camera - FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;       
    lighting+=(ambient + (diffuse + specular)) * Albedo * vec3(0.03,0.03,0.3);
    return lighting;
}

void main()
{             
    vec3 FragPos = texture(gPosition, TexCoords2).rgb;
    vec3 Normal = texture(gNormal, TexCoords2).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords2).rgb;
    
    vec3 lighting =  vec3(0.1,0.1,0.1);
    vec3 MaterialAmbientColor = vec3(0,0,0);
    vec3 camera= vec3(viewInvMatrix * vec4(0, 0, 0, 1)); 
    vec3 normal = normalize(Normal);
    lighting=ComputeLight(0,Albedo, FragPos, normal,camera,lighting);
    MaterialAmbientColor += vec3(0.1,0.1,0.1) * lighting;
    for(int i = 1; i < NR_LIGHTS-1; ++i)
    {
        lighting=ComputeLightSphere(i,Albedo, FragPos, normal,camera,r,lighting);
    }
    lighting=ComputeLightSphere(NR_LIGHTS-1,Albedo, FragPos, normal,camera,50.0,lighting);
    
    FragColor = vec4(MaterialAmbientColor + lighting, 1);
    if(Normal.x==0 && Normal.y==0 && Normal.z==0)
    {
        vec4 p= invMatrix * vec4(gl_FragCoord.xyz, 1);
        vec3 pixel= p.xyz / p.w;
        
        vec3 direction= normalize(pixel - camera_position);
        FragColor= texture(texture0, direction);
    }
}  

#endif 
