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
    int sz = triangle[gl_PrimitiveID].size;

//    color =vec4(float(gl_PrimitiveID/8), 0, 0, 1); //goal

    color = vec4(
                triangle[gl_PrimitiveID].r/float(sz*255.),
                triangle[gl_PrimitiveID].g/float(sz*255.),
                triangle[gl_PrimitiveID].b/float(sz*255.),
                1
                );

}
