#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 39) out;
out vec2 pos;

int i = 0;

void sendTransformedTriangle(int offset_pos, vec2 offset)
{
    int prim = i++;

    for(int i = 0; i < 3; ++i)
    {
        pos = gl_in[i].gl_Position.xy + (offset_pos == i+1 ? offset : vec2(0));
        gl_Position = vec4(pos.x * 2. - 1., pos.y * 2. - 1., 0, 1);
        gl_PrimitiveID = gl_PrimitiveIDIn * 13 + prim;
        EmitVertex();
    }
    EndPrimitive();
}

uniform sampler2D image;

void main()
{
    ivec2 dims = textureSize(image,0);
    vec2 pixel_size = vec2(1,1) / vec2(dims);
    sendTransformedTriangle(0, vec2(0));

    for(int i = 1; i <= 3; ++i)
    {
        sendTransformedTriangle(i, pixel_size * vec2(1,0)); //right
        sendTransformedTriangle(i, pixel_size * vec2(0,1)); //up
        sendTransformedTriangle(i, pixel_size * vec2(-1,0));//left
        sendTransformedTriangle(i, pixel_size * vec2(0,-1));//down
    }
}
