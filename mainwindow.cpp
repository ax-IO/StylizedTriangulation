#include "mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QColorSpace>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QStandardPaths>
#include <QStatusBar>

#include <QGridLayout>
#include <QLabel>
#include <QTimer>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPushButton>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scrollArea(new QScrollArea), imageLabel(new QLabel), resolutionSpinBox (new QSpinBox) {

  imageLabel->setBackgroundRole(QPalette::Base);
  imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  imageLabel->setScaledContents(true);



  createActions();
  resize(QGuiApplication::primaryScreen()->availableSize() * (3.0f / 5.0f));
}

MainWindow::~MainWindow() {}

bool MainWindow::loadFile(const QString &fileName) {
  QImageReader reader(fileName);
  reader.setAutoTransform(true);
  const QImage newImage = reader.read();
  if (newImage.isNull()) {
    QMessageBox::information(
        this, QGuiApplication::applicationDisplayName(),
        tr("Cannot load %1: %2")
            .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
    return false;
  }

  setImage(newImage);
  //  loadImageToTexture(newImage);
  updateActions();
  //  qDebug()<< << Qt::endl;

  setWindowFilePath(fileName);

  const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
                              .arg(QDir::toNativeSeparators(fileName))
                              .arg(image.width())
                              .arg(image.height())
                              .arg(image.depth());
  statusBar()->showMessage(message);


  resize(image.width(), image.height() + filebar_height + statusbar_height);
//------------------------------------------------------------------------------------------
  openGL = new GLWidget(fileName, this);

//  image_to_display = openGL->grabFramebuffer();
//  if (image_to_display.colorSpace().isValid())
//    image_to_display.convertToColorSpace(QColorSpace::SRgb);
//  imageLabel->setPixmap(QPixmap::fromImage(image_to_display));


//  imageLabel->setBackgroundRole(QPalette::Base);
//  imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
//  imageLabel->setScaledContents(true);

//  imageLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
//  imageLabel->setText("first line\nsecond line");
//  imageLabel->setAlignment(Qt::AlignBottom | Qt::AlignRight);


//  scrollArea->setBackgroundRole(QPalette::Dark);
//  scrollArea->setWidget(imageLabel);
//  scrollArea->setVisible(true);

  setCentralWidget(openGL);


//------------------------------------------------------------------------------------------
  return true;
}

void MainWindow::createActions() {
  QMenu *fileMenu = menuBar()->addMenu(tr("&Fichier"));

  openAct =
      fileMenu->addAction(tr("&Ouvrir Image..."), this, &MainWindow::open);
  openAct->setShortcut(QKeySequence::Open);


  saveAsAct = fileMenu->addAction(tr("Enregistrer &sous..."), this,
                                  &MainWindow::saveAs);
  saveAsAct->setShortcut(QKeySequence::Save);
  saveAsAct->setEnabled(false);

  fileMenu->addSeparator();

  QAction *exitAct = fileMenu->addAction(tr("&Quitter"), this, &QWidget::close);
  exitAct->setShortcut(tr("Ctrl+Q"));

  //------------------------------------------------------------------------------------------


  QMenu *viewMenu = menuBar()->addMenu(tr("A&ffichage"));

  zoomInAct =
      viewMenu->addAction(tr("Zoom A&vant (25%)"), this, &MainWindow::zoomIn);
  zoomInAct->setShortcut(QKeySequence::ZoomIn);
  zoomInAct->setEnabled(false);

  zoomOutAct = viewMenu->addAction(tr("Zoom A&rrière (25%)"), this,
                                   &MainWindow::zoomOut);
  zoomOutAct->setShortcut(QKeySequence::ZoomOut);
  zoomOutAct->setEnabled(false);

  normalSizeAct =
      viewMenu->addAction(tr("&Taille réelle"), this, &MainWindow::normalSize);
  normalSizeAct->setShortcut(tr("Ctrl+="));
  normalSizeAct->setEnabled(false);

  viewMenu->addSeparator();

  fitToWindowAct = viewMenu->addAction(tr("A&juster à la fenêtre"), this,
                                       &MainWindow::fitToWindow);
  fitToWindowAct->setEnabled(false);
  fitToWindowAct->setCheckable(true);
  fitToWindowAct->setShortcut(tr("Ctrl+F"));


  //------------------------------------------------------------------------------------------
  QMenu *renderMenu = menuBar()->addMenu(tr("&Modes de rendu"));

  renderModeConstantAct = renderMenu->addAction(tr("Mode de rendu Constant"), this, &MainWindow::callRenderModeConstant);
  renderModeConstantAct->setShortcut(tr("Ctrl+&"));
  renderModeConstantAct->setEnabled(false);

  renderModeGradientAct = renderMenu->addAction(tr("Mode de rendu Gradient"), this, &MainWindow::callRenderModeGradient);
  renderModeGradientAct->setShortcut(tr("Ctrl+é"));
  renderModeGradientAct->setEnabled(false);

  renderMenu->addSeparator();
  showResolutionWindowAct= renderMenu->addAction(tr("Changer la résolution"), this, &MainWindow::showResolutionWindow);
  showResolutionWindowAct->setShortcut(tr("Ctrl+r"));
  showResolutionWindowAct->setEnabled(false);

  renderMenu->addSeparator();

  optimizationPassAct = renderMenu->addAction(tr("Passe d'optimisation"), this, &MainWindow::callOptimizationPass);
  optimizationPassAct->setShortcut(tr("o"));
  optimizationPassAct->setEnabled(false);

  optimizationSplitPassAct = renderMenu->addAction(tr("Passe d'optimisation Split"), this, &MainWindow::callOptimizationSplitPass);
  optimizationSplitPassAct->setShortcut(tr("s"));
  optimizationSplitPassAct->setEnabled(false);



  //------------------------------------------------------------------------------------------
  QMenu *helpMenu = menuBar()->addMenu(tr("&Aide"));

  helpMenu->addAction(tr("À propos de &StylizedTriangulation"), this,
                      &MainWindow::about);
  helpMenu->addAction(tr("À propos de &Qt"), &QApplication::aboutQt);
}

void MainWindow::updateActions() {
  saveAsAct->setEnabled(!image.isNull());
  zoomInAct->setEnabled(!fitToWindowAct->isChecked());
  zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
  normalSizeAct->setEnabled(!fitToWindowAct->isChecked());

  renderModeConstantAct->setEnabled(!image.isNull());
  renderModeGradientAct->setEnabled(!image.isNull());
  showResolutionWindowAct->setEnabled(!image.isNull());
  optimizationPassAct->setEnabled(!image.isNull());
  optimizationSplitPassAct->setEnabled(!image.isNull());


}

bool MainWindow::saveFile(const QString &fileName) {

  QImageWriter writer(fileName);

  if (!writer.write(image_to_save)) {
    QMessageBox::information(
        this, QGuiApplication::applicationDisplayName(),
        tr("Cannot write %1: %2").arg(QDir::toNativeSeparators(fileName)),
        writer.errorString());
    return false;
  }
  const QString message =
      tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
  statusBar()->showMessage(message);
  return true;
}

void MainWindow::setImage(const QImage &newImage) {
  image = newImage;
  if (image.colorSpace().isValid())
    image.convertToColorSpace(QColorSpace::SRgb);
  //    imageLabel->setPixmap(QPixmap::fromImage(image));
  scaleFactor = 1.0;

  //    scrollArea->setVisible(true);
  fitToWindowAct->setEnabled(true);
  updateActions();

  //    if (!fitToWindowAct->isChecked())
  //        imageLabel->adjustSize();
}


void MainWindow::scaleImage(double factor) {}

void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor) {}

//
// SLOTS
//

static void initializeImageFileDialog(QFileDialog &dialog,
                                      QFileDialog::AcceptMode acceptMode) {
  static bool firstDialog = true;

  if (firstDialog) {
    firstDialog = false;
    const QStringList picturesLocations =
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath()
                                                    : picturesLocations.last());
  }

  QStringList mimeTypeFilters;
  const QByteArrayList supportedMimeTypes =
      acceptMode == QFileDialog::AcceptOpen
          ? QImageReader::supportedMimeTypes()
          : QImageWriter::supportedMimeTypes();
  for (const QByteArray &mimeTypeName : supportedMimeTypes)
    mimeTypeFilters.append(mimeTypeName);
  mimeTypeFilters.sort();
  dialog.setMimeTypeFilters(mimeTypeFilters);
  //  dialog.selectMimeTypeFilter("image/jpeg");
  dialog.selectMimeTypeFilter("image/png");
  //  dialog.selectMimeTypeFilter("application/octet-stream");

  if (acceptMode == QFileDialog::AcceptSave)
    dialog.setDefaultSuffix("jpg");
  //    dialog.setDefaultSuffix("png");
}

void MainWindow::open() {
  QFileDialog dialog(this, tr("Ouvrir un fichier Image"));
  initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

  while (dialog.exec() == QDialog::Accepted &&
         !loadFile(dialog.selectedFiles().first())) {
  }
}

void MainWindow::saveAs() {
  image_to_save = openGL->grabFramebuffer();

  QFileDialog dialog(this, tr("Enregistrer un fichier Image"));
  initializeImageFileDialog(dialog, QFileDialog::AcceptSave);
  while (dialog.exec() == QDialog::Accepted &&
         !saveFile(dialog.selectedFiles().first())) {
  }
}

void MainWindow::zoomIn() {
  qDebug() << "Zoom In" << Qt::endl;
  openGL->resize(800, 800);
  resize(800, 800 + filebar_height + statusbar_height);

}
void MainWindow::zoomOut() {}

void MainWindow::normalSize() {}

void MainWindow::fitToWindow() {}

void MainWindow::about() {
  QMessageBox::about(
      this, tr("About Image Viewer"),
      tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
         "and QScrollArea to display an image. QLabel is typically used "
         "for displaying a text, but it can also display an image. "
         "QScrollArea provides a scrolling view around another widget. "
         "If the child widget exceeds the size of the frame, QScrollArea "
         "automatically provides scroll bars. </p><p>The example "
         "demonstrates how QLabel's ability to scale its contents "
         "(QLabel::scaledContents), and QScrollArea's ability to "
         "automatically resize its contents "
         "(QScrollArea::widgetResizable), can be used to implement "
         "zooming and scaling features. </p><p>In addition the example "
         "shows how to use QPainter to print an image.</p>"));
}

void MainWindow::showResolutionWindow()
{
    QWidget *resolutionWindow = new QWidget;

    int min = 0;
    int max = 2000;

    QLabel *resolutionintegerLabel = new QLabel(tr("Enter a value between ""%1 and %2:").arg(min).arg(max));

    resolutionSpinBox->setRange(min, max);
    resolutionSpinBox->setSingleStep(1);
//    resolutionSpinBox->setValue(0);
    resolutionSpinBox->setValue(openGL->getGridResolution());
    connect(resolutionSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::callChangeResolution);
    QPushButton *applyResolutionButton = new QPushButton("Appliquer");
    connect(applyResolutionButton, &QPushButton::released, this, &MainWindow::callChangeResolution);

    QVBoxLayout *layout = new QVBoxLayout(resolutionWindow);

    layout->addWidget(resolutionintegerLabel);
    layout->addWidget(resolutionSpinBox);
    layout->addWidget(applyResolutionButton);
    resolutionWindow->show();
}

void MainWindow::callChangeResolution()
{
    int resolution = resolutionSpinBox->value();
//    qDebug()<<"Change Resolution"<< resolution;
    openGL->changeGridResolution(resolution);
    openGL->update();
}

void MainWindow::callRenderModeConstant()
{
    openGL->renderModeConstant();
    openGL->update() ;
}

void MainWindow::callRenderModeGradient()
{
    openGL->renderModeGradient();
    openGL->update() ;
}

void MainWindow::callOptimizationPass()
{
    qDebug()<< "callOptimizationPass() :";
    openGL->optimizationPass();
    openGL->update() ;
}
void MainWindow::callOptimizationSplitPass()
{
    qDebug()<< "callOptimizationSplitPass() :";
    openGL->optimizationSplitPass();
    openGL->update() ;
}
// Souvent : optimize(Triangulation, )
// A chaque frame : rendererer->render(triangulation_optimisee, )
