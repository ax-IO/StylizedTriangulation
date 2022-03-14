#version 430 core

struct Triangle_frag
{
    uint r,g,b;
    int size;
};

layout(std430, binding=1) buffer frag_col
{
    Triangle_frag triangle[];
};

uniform sampler2D img;

out vec4 color;
in vec2 uv;
void main(void)
{
    //color -> [0,1] (*255 pour convertir en unsigned)
    color = texture(img, uv);
    uvec4 col_to_add = uvec4(color*255.);

    atomicAdd(triangle[gl_PrimitiveID].r, col_to_add.r);
    atomicAdd(triangle[gl_PrimitiveID].g, col_to_add.g);
    atomicAdd(triangle[gl_PrimitiveID].b, col_to_add.b);
    atomicAdd(triangle[gl_PrimitiveID].size, 1);



}
