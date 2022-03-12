#version 430 core
layout(location=0) in vec2 vertex;

void main(void)
{
    gl_Position = vec4(vertex*2 - 1, 0, 1.);
}
