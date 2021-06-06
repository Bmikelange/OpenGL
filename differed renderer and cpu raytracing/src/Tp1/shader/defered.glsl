#version 450
#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

out vec2 TexCoords2;
out vec3 position2;
out vec3 Normal2;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;

void main()
{
    gl_Position= mvpMatrix * vec4(position, 1);
    TexCoords2=texcoord;
    position2=vec3(modelMatrix * vec4(position, 1));
    Normal2=mat3(modelMatrix) * normal;
}

#endif

#ifdef FRAGMENT_SHADER
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

uniform sampler2D texture_diffuse1;

in vec2 TexCoords2;
in vec3 position2;
in vec3 Normal2;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = position2;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal2);
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords2).rgb;
    gAlbedoSpec.a=1;
} 
#endif 


