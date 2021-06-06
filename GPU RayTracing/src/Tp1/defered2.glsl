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

vec4 hdr_to_ldr(vec4 hdrColor, float exposure) {
    return vec4(1.0) - exp(-hdrColor * exposure);
}
  
in vec2 TexCoords2;

uniform sampler2D image;

void main()
{             
    vec3 Albedo = texture(image, TexCoords2).rgb;
    
    vec4 Albedo1 = hdr_to_ldr(vec4(Albedo, 1),0.3);
    
    Albedo1 = pow(Albedo1,vec4(1.0/2.2));
    
    FragColor = Albedo1;
}  

#endif 
