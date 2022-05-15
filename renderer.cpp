#include "renderer.h"

struct Triangle_frag
{
    GLuint r = 0;
    GLuint g = 0;
    GLuint b = 0;

    int size = 0;
};

struct Linear_coeff
{
    // A est une matrice symétrique -> besoin de stocker 6 coefficients seulement
    uint A_upper[6] = {0};
    uint R_b_vec[3] = {0};
    uint G_b_vec[3] = {0};
    uint B_b_vec[3] = {0};
};

// Coefficients a, b, c pour chaque canal
struct Computed_coeff
{
    float R[3], G[3], B[3];
    int singular = 0;
};

// Matrice triangulaire dont la partie supérieure seulement est remplie
Eigen::SparseMatrix<float> matrix_from_coeff(uint comp[6])
{
    Eigen::SparseMatrix<float> output(3, 3);
    std::vector<Eigen::Triplet<float>> entries;
    entries.reserve(6);

    int k = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = i; j < 3; j++)
        {
            entries.push_back(Eigen::Triplet<float>(i, j, comp[k]));
            k++;
        }
    }

    output.setFromTriplets(entries.begin(), entries.end());
    return output;
}

// Flatten triangles to send to DrawElements
std::vector<VertexIndice> compute_indices(const Triangulation tri, bool linify = false)
{
    std::vector<VertexIndice> indices;
    if (linify)
    {
        indices.resize(6 * tri.triangles().size());
    }
    else
    {
        indices.resize(3 * tri.triangles().size());
    }

    int k = 0;
    for (Triangle t : tri.triangles())
    {
        indices[k] = t.a;
        indices[k + 1] = t.b;

        if (linify)
        {
            indices[k + 2] = t.b;
            indices[k + 3] = t.c;
            indices[k + 4] = t.c;
            indices[k + 5] = t.a;
            k += 6;
        }
        else
        {
            indices[k + 2] = t.c;
            k += 3;
        }
    }

    return indices;
}

Renderer::Renderer()
    : vertex_buffer(QOpenGLBuffer::VertexBuffer), indice_buffer(QOpenGLBuffer::IndexBuffer)
{

    gl_fct = QOpenGLContext::currentContext()->extraFunctions();

    // besoin de fonctions suppémentaires (glBufferSubData)
    gl_fct_special = new QOpenGLFunctions_2_0();
    gl_fct_special->initializeOpenGLFunctions();

    init_GL();
}

void Renderer::init_buffers(const Triangulation &tri, int style)
{
    std::vector<VertexIndice> indices = compute_indices(tri);
    //    std::vector<VertexIndice> indices = compute_indices(tri, true);

    vertex_buffer.create(); // avec les QOpenGLBuffer glGenBuffers, glBind, glBufferData
    vertex_buffer.bind();
    vertex_buffer.allocate(tri.vertices().data(), tri.size() * sizeof(Vec2));
    vertex_buffer.release();

    indice_buffer.create();
    indice_buffer.bind();
    indice_buffer.allocate(indices.data(), indices.size() * sizeof(VertexIndice));
    indice_buffer.release();

    // Shader storage buffer pas supporté par QOpenGLBuffer ...
    std::vector<Triangle_frag> tri_storage(tri.triangles().size());
    std::vector<Linear_coeff> coeff_storage(tri.triangles().size());

    gl_fct->glGenBuffers(1, &storage_buffer);
    gl_fct->glBindBuffer(GL_SHADER_STORAGE_BUFFER, storage_buffer);
    if (style == COLOR_CONSTANT)
    {
        gl_fct->glBufferData(GL_SHADER_STORAGE_BUFFER, tri.triangles().size() * sizeof(Triangle_frag), tri_storage.data(), GL_DYNAMIC_COPY);
    }
    else
    {
        gl_fct->glBufferData(GL_SHADER_STORAGE_BUFFER, tri.triangles().size() * sizeof(Linear_coeff), coeff_storage.data(), GL_DYNAMIC_COPY);
    }

    gl_fct->glDisable(GL_DEPTH_TEST);
}

void Renderer::init_GL()
{
    program[0].addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertex_shader.glsl");
    program[0].addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader.glsl");

    if (!program[0].link())
    {
        QString error = program[0].log();
        std::cout << "ERREUR au link des shaders: " << error.toStdString() << std::endl;
    }

    program[1].addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertex_shader.glsl");
    program[1].addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader_final.glsl");

    if (!program[1].link())
    {
        std::cout << "FRAG 2" << std::endl;
        QString error = program[1].log();
        std::cout << "ERREUR au link des shaders: " << error.toStdString() << std::endl;
    }
}

// TODO: texture !!!
void Renderer::render(const Triangulation &tri, unsigned int tex, int style, bool lines)
{
    init_buffers(tri, style);
    GLuint sampler_loc;

    // TEXTURE
    int h, w;
    gl_fct->glActiveTexture(GL_TEXTURE0);
    gl_fct->glBindTexture(GL_TEXTURE_2D, tex);
    gl_fct->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    gl_fct->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

    //    std::cout<<"texture w, h : "<<w<< ", "<<h<<std::endl;
    gl_fct->glViewport(0, 0, w, h);
    gl_fct->glClear(GL_COLOR_BUFFER_BIT);
    gl_fct->glEnableVertexAttribArray(0);

    // VERTICES
    vertex_buffer.bind();
    gl_fct->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    // SSBO
    if (style == COLOR_CONSTANT)
    {
        //        std::cout<<"Couleur constante"<<std::endl;
        gl_fct->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, storage_buffer);
    }
    else
    {
        //        std::cout<<"Couleur gradients linéaires"<<std::endl;
        gl_fct->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, storage_buffer);
    }

    // Contiendra les coefficients a, b et c pour chaque triangles.
    int coeff_index = 0;
    std::vector<Computed_coeff> computed_coeff(tri.triangles().size());
    bool first_pass = true;

    // 1ere passe:
    //   couleur constante: somme des couleurs aux sommets de chaque primitive.
    //   couleur gradients linéaires: calcul des coefficients a,b et c de chaque primitives (triangles).

    for (int i = 0; i < 2; i++)
    {
        sampler_loc = program[i].uniformLocation("img");
        program[i].bind();
        program[i].setUniformValue(sampler_loc, 0);
        program[i].setUniformValue(program[i].uniformLocation("color_constant"), style);

        // DRAW
        indice_buffer.bind();
        if (lines)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        gl_fct->glDrawElements(GL_TRIANGLES, tri.triangles().size() * 3, GL_UNSIGNED_INT, nullptr);
        //        gl_fct->glDrawElements(GL_LINES, tri.triangles().size()*6, GL_UNSIGNED_INT, nullptr);

        if (style == COLOR_GRADIENT && first_pass)
        {
            first_pass = false;
            gl_fct->glMemoryBarrier(GL_ALL_BARRIER_BITS);

            std::vector<Linear_coeff> computed(tri.triangles().size());
            gl_fct_special->glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, tri.triangles().size() * sizeof(Linear_coeff), &computed[0]);

            // Décomposition de Cholesky pour calculer a, b et c à partir de la matrice remplie en première passe pour chaque triangle
            for (Linear_coeff tri_matrices : computed)
            {
                Computed_coeff to_load;

                Eigen::SparseMatrix<float> A_mat = matrix_from_coeff(tri_matrices.A_upper);
                Eigen::SimplicialLDLT<Eigen::SparseMatrix<float>, Eigen::Upper> solver;

                solver.compute(A_mat);
                if (solver.info() != Eigen::Success)
                {
                    // A est singulière -> couleur constante
                    //                    std::cout<<"A -> LLT Decomposition failed."<<std::endl;
                    for (int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j < 3; j++)
                        {
                            std::cout << A_mat.coeffRef(i, j) << " ";
                        }
                        std::cout << std::endl;
                    }

                    to_load.singular = 1;
                    int sz = A_mat.coeffRef(2, 2);
                    to_load.R[0] = (float)tri_matrices.R_b_vec[2] / (float)sz;
                    to_load.G[0] = (float)tri_matrices.G_b_vec[2] / (float)sz;
                    to_load.B[0] = (float)tri_matrices.B_b_vec[2] / (float)sz;
                }
                else
                {
                    Eigen::Vector3<float> rb, gb, bb;
                    for (int i = 0; i < 3; i++)
                    {
                        rb(i) = tri_matrices.R_b_vec[i];
                        gb(i) = tri_matrices.G_b_vec[i];
                        bb(i) = tri_matrices.B_b_vec[i];
                    }

                    // solver Ax = b pour chaque canal (on réutilise la même décomposition de A)
                    Eigen::Vector3<float> abc;
                    abc = solver.solve(rb);
                    if (solver.info() != Eigen::Success)
                    {
                        std::cout << "SOLVER FOR Ax=b FAILED. (red)" << std::endl;
                    }
                    for (int i = 0; i < 3; i++)
                        to_load.R[i] = abc(i);

                    abc = solver.solve(gb);
                    if (solver.info() != Eigen::Success)
                    {
                        std::cout << "SOLVER FOR Ax=b FAILED. (green)" << std::endl;
                    }
                    for (int i = 0; i < 3; i++)
                        to_load.G[i] = abc(i); /*std::cout<<"ABC GB "<<abc(i)<<std::endl;*/

                    abc = solver.solve(bb);
                    if (solver.info() != Eigen::Success)
                    {
                        std::cout << "SOLVER FOR Ax=b FAILED. (blue)" << std::endl;
                    }
                    for (int i = 0; i < 3; i++)
                        to_load.B[i] = abc(i); /*std::cout<<"ABC BB "<<abc(i)<<std::endl;*/
                }

                // Ajout dans le tableau des coefficients (abc) calculés pour le triangle courant à charger au gpu plus tard.
                computed_coeff[coeff_index] = to_load;
                coeff_index++;
            }

            // Charger les coefficients a, b et c calculés dans le GPU pour la 2nde passe
            gl_fct->glBindBuffer(GL_SHADER_STORAGE_BUFFER, storage_buffer);
            gl_fct->glBufferData(GL_SHADER_STORAGE_BUFFER, tri.triangles().size() * sizeof(Computed_coeff), computed_coeff.data(), GL_DYNAMIC_COPY);
            gl_fct->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, storage_buffer);
        }

        program[i].release();
    }

    //    int tri_id = 0;
    //    for(Triangle_frag tf : computed)
    //    {
    //        std::cout<<tri_id<<" :TO AFFECT: R "<<tf.r/float (255.*tf.size)<<", G "<<tf.g/float (255.*tf.size)<<", B "<<tf.b/float (255.*tf.size)<<std::endl;
    //        tri_id ++;
    //    }
    // CLEAN
    //    glDisableClientState(GL_VERTEX_ARRAY);
    vertex_buffer.release();
    indice_buffer.release();
    gl_fct->glDeleteBuffers(1, &storage_buffer);
}

Renderer::~Renderer()
{
}
