#version 330


#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 2) in vec3 normal;


out vec3 position2;
out vec3 Normal2;

void main( )
{
    gl_Position= vec4(position, 1);
    position2=position;
}
#endif

#ifdef FRAGMENT_SHADER


//~ const vec3 source= vec3(0, 0, 0);   // source dans le repere du monde

out vec4 FragColor;

uniform mat4 invMatrix;
uniform vec3 camera_position;
uniform samplerCube texture0;

in vec3 position2;
in vec3 Normal2;

void main( )
{
	vec4 p= invMatrix * vec4(gl_FragCoord.xyz, 1);
	vec3 pixel= p.xyz / p.w;

	vec3 direction= normalize(pixel - camera_position);
	FragColor= texture(texture0, direction);
}
#endif

