#version 430 core

// layout(std430, binding=1) buffer

uniform sampler2D img;
out vec4 color;
in vec2 uv;
void main(void)
{
//    color = texture(img, uv);
    color = texture(img, vec2(0.5,0.4));

//    color = vec4(uv.x, uv.y, 0, 1);
//    color = vec4(1., 0., 0., 1.);
}
