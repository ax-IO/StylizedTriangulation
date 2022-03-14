#include "renderer.h"

struct Triangle_frag
{
    GLuint r=0;
    GLuint g=0;
    GLuint b=0;

    int size=0;
};

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
    gl_fct = QOpenGLContext::currentContext()->extraFunctions();
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

    //Shader storage buffer pas supporté par QOpenGLBuffer ...
    std::vector<Triangle_frag> tri_storage(tri.triangles().size());
    gl_fct->glGenBuffers(1, &storage_buffer);
    gl_fct->glBindBuffer(GL_SHADER_STORAGE_BUFFER, storage_buffer);
    gl_fct->glBufferData(GL_SHADER_STORAGE_BUFFER, tri.triangles().size() * sizeof(Triangle_frag), tri_storage.data(), GL_DYNAMIC_COPY);

    glDisable(GL_DEPTH_TEST);
}

void Renderer::init_GL(){
    program[0].addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertex_shader.glsl");
    program[0].addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader.glsl");

    if(!program[0].link()){
        QString error = program[0].log();
        std::cout<<"ERREUR au link des shaders: "<<error.toStdString()<<std::endl;
    }

    program[1].addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertex_shader.glsl");
    program[1].addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader_final.glsl");

    if(!program[1].link()){
        std::cout<<"FRAG 2"<<std::endl;
        QString error = program[1].log();
        std::cout<<"ERREUR au link des shaders: "<<error.toStdString()<<std::endl;
    }



}

//TODO: texture !!!
void Renderer::render(const Triangulation& tri, unsigned int tex){
    init_GL();
    init_buffers(tri);
    GLuint sampler_loc;

    //TEXTURE
    int h,w;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

    std::cout<<"texture w, h : "<<w<< ", "<<h<<std::endl;
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);


    //VERTICES
    vertex_buffer.bind();
    glVertexPointer(2, GL_FLOAT, 0, NULL);
    vertex_buffer.release();

    //SSBO
    gl_fct->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, storage_buffer);

    //1ere passe: somme des couleurs aux sommets de chaque primitive
    for(int i = 0; i < 2; i ++){
        sampler_loc = program[i].uniformLocation("img");
        program[i].bind();
        program[i].setUniformValue(sampler_loc, 0);

        //DRAW
        indice_buffer.bind();
        glDrawElements(GL_TRIANGLES, tri.triangles().size() * 3, GL_UNSIGNED_INT, nullptr);
        indice_buffer.release();

        program[i].release();
    }

    //CLEAN
    glDisableClientState(GL_VERTEX_ARRAY);
    vertex_buffer.release();
    indice_buffer.release();
    gl_fct->glDeleteBuffers(1, &storage_buffer);

}

Renderer::~Renderer()
{
    
}



