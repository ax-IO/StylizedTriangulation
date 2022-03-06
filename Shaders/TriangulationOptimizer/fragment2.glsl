#version 430 core

struct TriangleData
{
    uvec4 color_total_and_count;
};

struct ErrorData
{
    uint error_total;
    uint count;
};

layout(std430, binding=1) coherent restrict readonly buffer TrianglesData
{
    TriangleData data[];
};

layout(std430, binding=2) coherent restrict buffer ErrorsData
{
    ErrorData errors[];
};

uniform sampler2D image;

float sqrDist( vec3 l, vec3 r )
{
    vec3 x = l - r;
    return dot( x, x );
}

in vec2 pos;
out vec4 c;

void main()
{
    vec3 color = texture(image, pos).xyz;
    vec3 mean_color = vec3(data[gl_PrimitiveID].color_total_and_count.xyz) / float(data[gl_PrimitiveID].color_total_and_count.w) / 255.;
    memoryBarrierBuffer();
    atomicAdd(errors[gl_PrimitiveID].error_total, uint(sqrDist(color, mean_color) * 255.));
    atomicAdd(errors[gl_PrimitiveID].count, 1);
    memoryBarrierBuffer();
    c = texture(image, pos);
}

