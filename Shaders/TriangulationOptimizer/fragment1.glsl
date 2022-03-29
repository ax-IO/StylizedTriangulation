#version 430 core

struct TriangleData
{
    uvec4 color_total_and_count;
};

layout(std430, binding=1) coherent restrict buffer TrianglesData
{
    TriangleData data[];
};

uniform sampler2D image;

in vec2 pos;
out vec4 c;

void main()
{
    uvec4 color = uvec4(texture(image, pos)*255.);
    memoryBarrierBuffer();
    atomicAdd(data[gl_PrimitiveID].color_total_and_count.x, color.x);
    atomicAdd(data[gl_PrimitiveID].color_total_and_count.y, color.y);
    atomicAdd(data[gl_PrimitiveID].color_total_and_count.z, color.z);
    atomicAdd(data[gl_PrimitiveID].color_total_and_count.w, 1);
    memoryBarrierBuffer();

    c = texture(image, pos);
}

