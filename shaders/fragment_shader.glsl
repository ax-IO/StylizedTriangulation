#version 430 core

struct Triangle_frag
{
    uint r,g,b;
    int size;
};

//struct Debug_info
//{
//    vec2 frag_coord;
//    vec2 uv;
//};

struct Linear_coeff
{
    //A est une matrice symétrique -> besoin de stocker 6 coefficients seulement
    uint A_upper[6];
    uint R_b_vec[3];
    uint G_b_vec[3];
    uint B_b_vec[3];

};

layout(std430, binding=1) buffer frag_col
{
    Triangle_frag triangle[];

};

layout(std430, binding=2) buffer coeff_storage
{
    Linear_coeff coeff[];

};

uniform sampler2D img;

uniform bool color_constant;
out vec4 color;
in vec2 uv;
void main(void)
{
    //color -> [0,1] (*255 pour convertir en unsigned)
    color = texture(img, uv);
    uvec4 col_to_add = uvec4(color*255.);
    memoryBarrierBuffer();
    if(color_constant)
    {
        atomicAdd(triangle[gl_PrimitiveID].r, col_to_add.r);
        atomicAdd(triangle[gl_PrimitiveID].g, col_to_add.g);
        atomicAdd(triangle[gl_PrimitiveID].b, col_to_add.b);
        atomicAdd(triangle[gl_PrimitiveID].size, 1);
    }
    else
    {
        //Surement pas les bonnes coordonées
        int x = int(gl_FragCoord.x);
        int y = int(gl_FragCoord.y);

//        int x = int(uv.x);
//        int y = int(uv.y);

        //A matrix
        atomicAdd(coeff[gl_PrimitiveID].A_upper[0], x*x);
        atomicAdd(coeff[gl_PrimitiveID].A_upper[1], x*y);
        atomicAdd(coeff[gl_PrimitiveID].A_upper[2], x);
        atomicAdd(coeff[gl_PrimitiveID].A_upper[3], y*y);
        atomicAdd(coeff[gl_PrimitiveID].A_upper[4], y);
        atomicAdd(coeff[gl_PrimitiveID].A_upper[5], 1);

        //b vector
        atomicAdd(coeff[gl_PrimitiveID].R_b_vec[0], x*col_to_add.r);
        atomicAdd(coeff[gl_PrimitiveID].R_b_vec[1], y*col_to_add.r);
        atomicAdd(coeff[gl_PrimitiveID].R_b_vec[2], col_to_add.r);

        atomicAdd(coeff[gl_PrimitiveID].G_b_vec[0], x*col_to_add.g);
        atomicAdd(coeff[gl_PrimitiveID].G_b_vec[1], y*col_to_add.g);
        atomicAdd(coeff[gl_PrimitiveID].G_b_vec[2], col_to_add.g);

        atomicAdd(coeff[gl_PrimitiveID].B_b_vec[0], x*col_to_add.b);
        atomicAdd(coeff[gl_PrimitiveID].B_b_vec[1], y*col_to_add.b);
        atomicAdd(coeff[gl_PrimitiveID].B_b_vec[2], col_to_add.b);
    }
    memoryBarrierBuffer();




}
