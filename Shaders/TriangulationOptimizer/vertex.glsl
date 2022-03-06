#version 430 core

layout(location = 0) in vec2 vertice;

void main()
{
    gl_Position = vec4(vertice * 2. - 1., 0, 1);
}

