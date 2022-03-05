#include "renderer.h"

// Flatten triangles to send to DrawElements
std::vector<VertexIndice> compute_indices(const Triangulation tri){
    std::vector<VertexIndice> indices;
    indices.resize(3 * tri.triangles().size());

    int k = 0;
    for(Triangle t : tri.triangles()){
        indices[k] = t.a; 
        indices[k + 1] = t.b; 
        indices[k + 2] = t.c; 
        k += 3;
    }

    return indices;
}

Renderer::Renderer(){
    vertex_buffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    indice_buffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
//    context = new QOpenGLContext;
//    context->create();
}

void Renderer::init_buffers(const Triangulation &tri){
    std::vector<VertexIndice> indices = compute_indices(tri);
    //glGenbuffers, glBindBuffer, glBufferData ....
    vertex_buffer.create();
    vertex_buffer.bind();
    vertex_buffer.allocate(&tri, tri.size() * sizeof(Vec2));
    vertex_buffer.release();

    indice_buffer.create();
    indice_buffer.bind();
    indice_buffer.allocate(&indices, indices.size() * sizeof(VertexIndice));
    indice_buffer.release();
}

void Renderer::init_GL(){
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, "vertex_shader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, "fragment_shader.glsl");

    if(!program.link()){
        QString error = program.log();
        std::cout<<"ERREUR au link des shaders: "<<error.toStdString()<<std::endl;
    }
}

void Renderer::render(const Triangulation& tri, unsigned int tex){
    init_GL();
    init_buffers(tri);

    program.bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glViewport(0, 0, 400, 400);
    glClear(GL_COLOR_BUFFER_BIT);

    vertex_buffer.bind();
    program.enableAttributeArray(program.attributeLocation("vertex"));
    program.setAttributeBuffer(program.attributeLocation("vertex"), GL_FLOAT, 0, 3);
    vertex_buffer.release();

    QMatrix4x4 mvp;
    mvp.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    mvp.translate(0, 0, -2);
    program.setUniformValue(program.uniformLocation("ModelViewProjectionMatrix"), mvp);

    indice_buffer.bind();
    glDrawElements(GL_TRIANGLES, tri.triangles().size(), GL_UNSIGNED_INT, NULL);
    indice_buffer.release();

    glDisableClientState(GL_VERTEX_ARRAY);
    program.release();

}



