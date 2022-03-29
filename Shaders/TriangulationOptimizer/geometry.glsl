#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 39) out;
out vec2 pos;

int i = 0;

void sendTransformedTriangle(int offset_pos, vec4 offset)
{
    int prim = i++;

    gl_Position = gl_in[0].gl_Position + (offset_pos == 1 ? offset : vec4(0));
    pos = (gl_Position.xy + 1.) / 2.;
    gl_PrimitiveID = gl_PrimitiveIDIn * 13 + prim;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position + (offset_pos == 2 ? offset : vec4(0));
    pos = (gl_Position.xy + 1.) / 2.;
    gl_PrimitiveID = gl_PrimitiveIDIn * 13 + prim;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position + (offset_pos == 3 ? offset : vec4(0));
    pos = (gl_Position.xy + 1.) / 2.;
    gl_PrimitiveID = gl_PrimitiveIDIn * 13 + prim;
    EmitVertex();
    EndPrimitive();
}

uniform sampler2D image;

void main()
{
    ivec2 dims = textureSize(image,0);
    vec2 pixel_size = vec2(2,2) / vec2(dims);
    sendTransformedTriangle(0, vec4(0));

    for(int i = 1; i <= 3; ++i)
    {
        sendTransformedTriangle(i, vec4(pixel_size * vec2(1,0), 0, 0)); //right
        sendTransformedTriangle(i, vec4(pixel_size * vec2(0,1), 0, 0)); //up
        sendTransformedTriangle(i, vec4(pixel_size * vec2(-1,0), 0, 0));//left
        sendTransformedTriangle(i, vec4(pixel_size * vec2(0,-1), 0, 0));//down
    }
}

