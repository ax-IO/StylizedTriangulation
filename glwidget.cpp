#include "glwidget.h"

#include <QPaintEvent>

GLWidget::GLWidget(QString filepath, QWidget *parent)
    : QOpenGLWidget{parent}, fp(filepath) {
  }

GLWidget::~GLWidget()
{
    delete m_renderer;
    delete m_tri_opt;
}

int GLWidget::getGridResolution()
{
    return m_gridResolution;
}

//------------------------------------------------------------------------------------------
void GLWidget::changeRegularGridResolution(int resolution)
{
    m_gridResolution = resolution;
    m_tri = Triangulation{m_gridResolution};
}

void GLWidget::updateSplitGrid(QString filename, double maxVariance,int maxDist)
{
    GenerateGrid *splitGrid = new GenerateGrid(m_width, m_height);
    splitGrid->computeTriangulationSplitAndMerge(filename,  maxVariance, maxDist);
    m_tri = Triangulation(splitGrid->getVertices(), splitGrid->getTriangles());
}
void GLWidget::updateGradientGrid(QString filename, int seuil, int maxPoints, float pointRate)
{
    GenerateGrid *splitGrid = new GenerateGrid(m_width, m_height);
    splitGrid->computeTriangulationGradientMap(filename, seuil, maxPoints, pointRate);
    m_tri = Triangulation(splitGrid->getVertices(), splitGrid->getTriangles());
}

void GLWidget::updateSobelGrid(QString filename, int seuilFiltre, int seuil, int maxPoints, float pointRate)
{
    GenerateGrid *splitGrid = new GenerateGrid(m_width, m_height);
    splitGrid->computeTriangulationSobelMap(filename, seuilFiltre, seuil, maxPoints, pointRate);
    m_tri = Triangulation(splitGrid->getVertices(), splitGrid->getTriangles());
}
//------------------------------------------------------------------------------------------
void GLWidget::renderModeConstant()
{
    m_renderMode = COLOR_CONSTANT;
}

void GLWidget::renderModeGradient()
{
    m_renderMode = COLOR_GRADIENT;
}
//------------------------------------------------------------------------------------------
void GLWidget::optimizationPass(float energySplitThreshold, float minTriangleArea)
{
    m_tri_opt->energySplitThreshold(energySplitThreshold);
    m_tri_opt->minTriangleArea(minTriangleArea);
    m_tri_opt->optimize(m_tri, tex);
}
void GLWidget::optimizationSplitPass(float energySplitThreshold, float minTriangleArea)
{
    m_tri_opt->energySplitThreshold(energySplitThreshold);
    m_tri_opt->minTriangleArea(minTriangleArea);
    m_tri_opt->optimizeSplit(m_tri, tex);
}
//------------------------------------------------------------------------------------------

void GLWidget::initializeGL() {
  gl_fct = QOpenGLContext::currentContext()->extraFunctions();
  float r, g, b;
  qColorToRGB(Qt::white, r, g, b);
  gl_fct->glClearColor(r, g, b, 1.0f);

  QImage img(fp);
  m_renderMode = COLOR_CONSTANT;
  m_width = img.width();
  m_height = img.height();

  img.convertTo(QImage::Format_RGBA8888);

  // texture setup
  gl_fct->glGenTextures(1, &tex);
  gl_fct->glBindTexture(GL_TEXTURE_2D, tex);
  gl_fct->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
  gl_fct->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl_fct->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  m_renderer = new Renderer;

  m_tri = Triangulation{m_gridResolution};

  m_tri_opt = new TriangulationOptimizer;
}

void GLWidget::paintGL() {
  gl_fct->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Créer un framebuffer

  // Rendu
  m_renderer->render(m_tri, tex, m_renderMode);

  // Exporter la texture puis en QImage
}

void GLWidget::resizeGL(int w, int h) {
  // qDebug() << "resizeGL(" << w << "," << h << ") :";
  //  gl_fct->glViewport(0, 0, w, h);
  //  glMatrixMode(GL_PROJECTION);
  //  glLoadIdentity();
  //  glMatrixMode(GL_MODELVIEW);
  //  glLoadIdentity();
}

void GLWidget::qColorToRGB(const QColor &C, float &r, float &g,
                           float &b) const {
  r = normalize_0_1(C.red(), RGB_MIN, RGB_MAX);
  g = normalize_0_1(C.green(), RGB_MIN, RGB_MAX);
  b = normalize_0_1(C.blue(), RGB_MIN, RGB_MAX);
}

float GLWidget::normalize_0_1(float val, float min, float max) const {
  return (val - min) / (max - min);
}

//------------------------------------------------------------------------------------------
// Retourne les coordonnées textures [0, 1] à partir des coordonnées de l'image
// et ses dimensions
Vec2 GLWidget::coord_imageToTexture(QPoint point_image, int width, int height) {
  Vec2 point_texture;
  point_texture.x = (float)point_image.x() / (float)width;
  point_texture.y = (float)point_image.y() / (float)height;
  point_texture.y = 1.0f - point_texture.y ;
  return point_texture;
}

// Distance entre deux vec2
float distance_vec2(Vec2 a, Vec2 b) {
  return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// Retourne l'indice du sommet correspondant aux coordonnées de textures
// (distance euclidienne minimale)
int GLWidget::coord_textureToIndex(Vec2 texturePoint, Triangulation tri) {
  std::vector<float> distances;
  distances.reserve(tri.size());

  for (int i = 0; i < tri.size(); i++) {
    distances.push_back(distance_vec2(texturePoint, tri.vertices()[i]));
  }

  auto it = std::min_element(std::begin(distances), std::end(distances));
  return std::distance(std::begin(distances), it);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  point = event->pos();
  Vec2 texturepoint = coord_imageToTexture(point, m_width, m_height);
  int index = coord_textureToIndex(texturepoint, m_tri);
//   qDebug() << "(" << point.x() << "," << point.y() << ") --> ("
//           << texturepoint.x << "," << texturepoint.y
//           << ") correspond au sommet" << index << "("
//           << m_tri.vertices()[index].x << "," << m_tri.vertices()[index].y
//           << ")" << Qt::endl;

  if (!(event->buttons() & Qt::RightButton))
      m_tri.deleteVertex(index);

//  if (!(event->buttons() & Qt::LeftButton))
//      m_tri.deleteTriangle(index);
  this->update();
}
