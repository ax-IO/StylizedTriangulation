#version 430 core

struct Triangle_frag
{
    uint r,g,b;
    int size;
};

struct Computed_coeff
{
    float R[3], G[3], B[3];
};


layout(std430, binding=1) buffer frag_col
{
    Triangle_frag triangle[];
};


layout(std430, binding=2) buffer coeff
{
    Computed_coeff abc[];
};




uniform sampler2D img;

uniform bool color_constant;
out vec4 color;
in vec2 uv;

void main(void)
{
    memoryBarrierBuffer();

//    color =vec4(float(gl_PrimitiveID/8), 0, 0, 1); //goal
    if(color_constant)
    {
        int sz = triangle[gl_PrimitiveID].size;

        color = vec4(
                    triangle[gl_PrimitiveID].r/float(sz*255.),
                    triangle[gl_PrimitiveID].g/float(sz*255.),
                    triangle[gl_PrimitiveID].b/float(sz*255.),
                    1
                    );
    }
    else
    {
        float x = gl_FragCoord.x;
        float y = gl_FragCoord.y;

        float r = abc[gl_PrimitiveID].R[0] * x + abc[gl_PrimitiveID].R[1] * y + abc[gl_PrimitiveID].R[2];
        float g = abc[gl_PrimitiveID].G[0] * x + abc[gl_PrimitiveID].G[1] * y + abc[gl_PrimitiveID].G[2];
        float b = abc[gl_PrimitiveID].B[0] * x + abc[gl_PrimitiveID].B[1] * y + abc[gl_PrimitiveID].B[2];

        color = vec4(r/float(255.), g/float(255.), b/float(255.), 1);
    }
    memoryBarrierBuffer();


}
