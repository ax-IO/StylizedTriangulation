#include "renderer.h"


// Flatten triangles to send to DrawElements
std::vector<VertexIndice> compute_indices(const Triangulation tri)
{
    std::vector<VertexIndice> indices;
    indices.resize(3 * tri.triangles().size());

    int k = 0;
    for(Triangle t : tri.triangles())
    {
        indices[k] = t.a; 
        indices[k + 1] = t.b; 
        indices[k + 2] = t.c; 
        k += 3;
    }

    return indices;
}

Renderer::Renderer()
    :vertex_buffer(QOpenGLBuffer::VertexBuffer), indice_buffer(QOpenGLBuffer::IndexBuffer)
{

}

void Renderer::init_buffers(const Triangulation& tri){
    std::vector<VertexIndice> indices = compute_indices(tri);

    vertex_buffer.create();           // avec les QOpenGLBuffer glGenBuffers, glBind, glBufferData
    vertex_buffer.bind();
    vertex_buffer.allocate(tri.vertices().data(), tri.size() * sizeof(Vec2));
    vertex_buffer.release();

    indice_buffer.create();
    indice_buffer.bind();
    indice_buffer.allocate(indices.data(), indices.size() * sizeof(VertexIndice));
    indice_buffer.release();
    glDisable(GL_DEPTH_TEST);
}

void Renderer::init_GL(){
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertex_shader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader.glsl");

    if(!program.link()){
        QString error = program.log();
        std::cout<<"ERREUR au link des shaders: "<<error.toStdString()<<std::endl;
    }

}

//TODO: texture !!!
void Renderer::render(const Triangulation& tri, unsigned int tex){
    init_GL();
    init_buffers(tri);
    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);

    program.bind();

    //VERTICES
    vertex_buffer.bind();
    glVertexPointer(2, GL_FLOAT, 0, NULL);
    vertex_buffer.release();

    std::cout<<tri.triangles().size()*3<<std::endl;
    //DRAW
    indice_buffer.bind();
    glDrawElements(GL_TRIANGLES, tri.triangles().size() * 3, GL_UNSIGNED_INT, nullptr);
    indice_buffer.release();

    //CLEAN
    glDisableClientState(GL_VERTEX_ARRAY);
    vertex_buffer.release();
    indice_buffer.release();
    program.release();

}

Renderer::~Renderer()
{
    
}



