#version 430 core
layout(location=0) in vec2 vertex;

//coordonées de texture qui ont pour origine le coin haut gauche
vec2 upside_down(vec2 v){
    vec2 vec_output;
    vec_output.x = v.x;
    vec_output.y = -v.y;
    return vec_output;
}

out vec2 uv;
void main(void)
{
    gl_Position = vec4(vertex *2 - 1, 0, 1.);
    uv = upside_down(vertex);
//    uv = vertex;
}
