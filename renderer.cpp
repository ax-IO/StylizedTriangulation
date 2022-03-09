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

void Renderer::init_buffers(const Triangulation &tri){
    std::vector<VertexIndice> indices = compute_indices(tri);
    //glGenbuffers, glBindBuffer, glBufferData ....

    vertex_buffer.create();           // avec les QOpenGLBuffer glGenBuffers, glBind, glBufferData
    vertex_buffer.bind();
    vertex_buffer.allocate(&tri.vertices()[0], tri.size() * sizeof(Vec2));
    vertex_buffer.release();

    indice_buffer.create();
    indice_buffer.bind();
    indice_buffer.allocate(&indices[0], indices.size() * sizeof(VertexIndice));
    indice_buffer.release();
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
    std::vector<VertexIndice> indices = compute_indices(tri);


    glEnableClientState(GL_VERTEX_ARRAY);
    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT);

    int vert_loc = program.attributeLocation("vertex");
    Q_ASSERT(vert_loc != -1);

    //VERTICES
    program.bind();
    vertex_buffer.bind();
    program.enableAttributeArray(vert_loc);
    program.setAttributeBuffer(vert_loc, GL_FLOAT, 0, 2); // == glVertexAttribPointer(...)
    vertex_buffer.release();

    //DRAW
    indice_buffer.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
    indice_buffer.release();

    //CLEAN
    glDisableClientState(GL_VERTEX_ARRAY);
    vertex_buffer.destroy();
    indice_buffer.destroy();
    program.release();

}

Renderer::~Renderer()
{
    
}



