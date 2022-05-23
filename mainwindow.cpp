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
    : QMainWindow(parent), scrollArea(new QScrollArea), imageLabel(new QLabel)
{

  imageLabel->setBackgroundRole(QPalette::Base);
  imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  imageLabel->setScaledContents(true);

  mapLabel = new QLabel;
  mapWindow = new QWidget;
  regularWindow = new QWidget;
  splitWindow = new QWidget;
  gradientWindow = new QWidget;
  sobelWindow = new QWidget;
  optimisationWindow = new QWidget;

  setWindowIcon(QIcon(":logo.png"));
  createActions();
  resize(QGuiApplication::primaryScreen()->availableSize() * (3.0f / 5.0f));
}

MainWindow::~MainWindow() {}

bool MainWindow::loadFile(const QString &fileName)
{
  QImageReader reader(fileName);
  reader.setAutoTransform(true);
  const QImage newImage = reader.read();
  if (newImage.isNull())
  {
    QMessageBox::information(
        this, QGuiApplication::applicationDisplayName(),
        tr("Cannot load %1: %2")
            .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
    return false;
  }


  setImage(newImage);
  updateActions();

  setWindowFilePath(fileName);

  const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
                              .arg(QDir::toNativeSeparators(fileName))
                              .arg(image.width())
                              .arg(image.height())
                              .arg(image.depth());
  statusBar()->setFixedHeight(statusbar_height);
  statusBar()->showMessage(message);

  this->setFixedSize(QSize(image.width(), image.height() + filebar_height + statusbar_height));
  //------------------------------------------------------------------------------------------
  openGL = new GLWidget(fileName, this);

  setCentralWidget(openGL);

  pgm_filename = QDir::toNativeSeparators(fileName).left(fileName.lastIndexOf('.')) + ".pgm";
  newImage.save(pgm_filename, "pgm", -1);

  //------------------------------------------------------------------------------------------
  return true;
}

void MainWindow::createActions()
{
  menuBar()->setFixedHeight(filebar_height);
  QMenu *fileMenu = menuBar()->addMenu(tr("&Fichier"));

  openAct = fileMenu->addAction(tr("&Ouvrir Image..."), this, &MainWindow::open);
  openAct->setShortcut(QKeySequence::Open);

  saveAsAct = fileMenu->addAction(tr("Enregistrer &sous..."), this,
                                  &MainWindow::saveAs);
  saveAsAct->setShortcut(QKeySequence::Save);
  saveAsAct->setEnabled(false);

  fileMenu->addSeparator();

  QAction *exitAct = fileMenu->addAction(tr("&Quitter"), this, &QWidget::close);
  exitAct->setShortcut(tr("Ctrl+Q"));


  //------------------------------------------------------------------------------------------
  QMenu *renderMenu = menuBar()->addMenu(tr("&Modes de rendu"));

  renderModeConstantAct = renderMenu->addAction(tr("Mode de rendu Constant"), this, &MainWindow::callRenderModeConstant);
  renderModeConstantAct->setShortcut(tr("Ctrl+&"));
  renderModeConstantAct->setEnabled(false);

  renderModeGradientAct = renderMenu->addAction(tr("Mode de rendu Gradient"), this, &MainWindow::callRenderModeGradient);
  renderModeGradientAct->setShortcut(tr("Ctrl+é"));
  renderModeGradientAct->setEnabled(false);

  //------------------------------------------------------------------------------------------
  QMenu *gridMenu = menuBar()->addMenu(tr("&Génération Grille"));
  showInitializeRegularGridWindowAct = gridMenu->addAction(tr("Générer un grille régulière"), this, &MainWindow::initializeRegularGridWindow);
  showInitializeRegularGridWindowAct->setShortcut(tr("Ctrl+r"));
  showInitializeRegularGridWindowAct->setEnabled(false);

  gridMenu->addSeparator();

  showInitializeSplitGridWindowAct = gridMenu->addAction(tr("Générer un grille par Split and Merge"), this, &MainWindow::initializeSplitGridWindow);
  showInitializeSplitGridWindowAct->setShortcut(tr("Ctrl+t"));
  showInitializeSplitGridWindowAct->setEnabled(false);

  gridMenu->addSeparator();

  showInitializeGradientGridWindowAct = gridMenu->addAction(tr("Générer un grille par Carte de gradient"), this, &MainWindow::initializeGradientGridWindow);
  showInitializeGradientGridWindowAct->setShortcut(tr("Ctrl+y"));
  showInitializeGradientGridWindowAct->setEnabled(false);

  showInitializeSobelGridWindowAct = gridMenu->addAction(tr("Générer un grille par Carte de Sobel"), this, &MainWindow::initializeSobelGridWindow);
  showInitializeSobelGridWindowAct->setShortcut(tr("Ctrl+u"));
  showInitializeSobelGridWindowAct->setEnabled(false);
  //------------------------------------------------------------------------------------------
  QMenu *optimisationMenu = menuBar()->addMenu(tr("&Optimisation"));

  optimizationContinuousAct = optimisationMenu->addAction(tr("Optimisation en continu"), this, &MainWindow::initializeOptimizationContinuousWindow);
  optimizationContinuousAct->setShortcut(tr("Ctrl+c"));
  optimizationContinuousAct->setEnabled(false);

  optimisationMenu->addSeparator();

  optimizationPassAct = optimisationMenu->addAction(tr("Passe d'optimisation"), this, &MainWindow::callOptimizationNormalPass);
  optimizationPassAct->setShortcut(tr("o"));
  optimizationPassAct->setEnabled(false);

  optimizationSplitPassAct = optimisationMenu->addAction(tr("Passe d'optimisation Split"), this, &MainWindow::callOptimizationSplitPass);
  optimizationSplitPassAct->setShortcut(tr("s"));
  optimizationSplitPassAct->setEnabled(false);

  //------------------------------------------------------------------------------------------
  QMenu *helpMenu = menuBar()->addMenu(tr("&Aide"));

  helpMenu->addAction(tr("À propos de &StylizedTriangulation"), this,
                      &MainWindow::about);
  helpMenu->addAction(tr("À propos de &Qt"), &QApplication::aboutQt);
}

void MainWindow::updateActions()
{
  saveAsAct->setEnabled(!image.isNull());

  renderModeConstantAct->setEnabled(!image.isNull());
  renderModeGradientAct->setEnabled(!image.isNull());

  showInitializeRegularGridWindowAct->setEnabled(!image.isNull());
  showInitializeSplitGridWindowAct->setEnabled(!image.isNull() && isImageSquare);
  showInitializeGradientGridWindowAct->setEnabled(!image.isNull());
  showInitializeSobelGridWindowAct->setEnabled(!image.isNull());

  optimizationPassAct->setEnabled(!image.isNull());
  optimizationSplitPassAct->setEnabled(!image.isNull());
  optimizationContinuousAct->setEnabled(!image.isNull());
}

bool MainWindow::saveFile(const QString &fileName)
{

  QImageWriter writer(fileName);

  if (!writer.write(image_to_save))
  {
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

void MainWindow::setImage(const QImage &newImage)
{
  image = newImage;
  if (image.colorSpace().isValid())
    image.convertToColorSpace(QColorSpace::SRgb);
  scaleFactor = 1.0;

  if (image.width() == image.height())
  {
    isImageSquare = true;
  }
  else
  {
    isImageSquare = false;
  }

  updateActions();

}

void MainWindow::scaleImage(double factor) {}

void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor) {}

//
// SLOTS
//

static void initializeImageFileDialog(QFileDialog &dialog,
                                      QFileDialog::AcceptMode acceptMode)
{
  static bool firstDialog = true;

  if (firstDialog)
  {
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
  dialog.selectMimeTypeFilter("image/png");

  if (acceptMode == QFileDialog::AcceptSave)
      dialog.setDefaultSuffix("png");
}

void MainWindow::open()
{
  QFileDialog dialog(this, tr("Ouvrir un fichier Image"));
  initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

  while (dialog.exec() == QDialog::Accepted &&
         !loadFile(dialog.selectedFiles().first()))
  {
  }
}

void MainWindow::saveAs()
{
  image_to_save = openGL->grabFramebuffer();

  QFileDialog dialog(this, tr("Enregistrer un fichier Image"));
  initializeImageFileDialog(dialog, QFileDialog::AcceptSave);
  while (dialog.exec() == QDialog::Accepted &&
         !saveFile(dialog.selectedFiles().first()))
  {
  }
}

void MainWindow::zoomIn()
{
  openGL->resize(800, 800);
  resize(800, 800 + filebar_height + statusbar_height);
}
void MainWindow::zoomOut() {}

void MainWindow::normalSize() {}

void MainWindow::fitToWindow() {}

void MainWindow::about()
{
  QMessageBox::about(
      this, tr("About StylizedTriangulation"),
      tr(""));
}

//------------------------------------------------------------------------------------------
void MainWindow::initializeMapWindow()
{
  mapWindow->setAttribute( Qt::WA_QuitOnClose, false );
  QVBoxLayout *layout = new QVBoxLayout(mapWindow);
  layout->addWidget(mapLabel);
  mapLabel->setPixmap(QPixmap(map));
  mapWindow->show();
}

void MainWindow::initializeRegularGridWindow()
{
  regularWindow->setAttribute( Qt::WA_QuitOnClose, false );
  int min = 0;
  int max = 2000;

  QLabel *resolutionintegerLabel = new QLabel(tr("Génération d'une grille initiale régulière"));

  QLabel *resolutionLabel = new QLabel(tr("Résolution de la grille : "));
  resolutionSpinBox = new QSpinBox();
  resolutionSpinBox->setRange(min, max);
  resolutionSpinBox->setSingleStep(1);
  resolutionSpinBox->setValue(openGL->getGridResolution());
  connect(resolutionSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::callChangeResolution);
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(resolutionLabel);
  hbox->addWidget(resolutionSpinBox);

  QPushButton *applyResolutionButton = new QPushButton("Appliquer");
  connect(applyResolutionButton, &QPushButton::released, this, &MainWindow::callChangeResolution);

  QVBoxLayout *layout = new QVBoxLayout(regularWindow);

  layout->addWidget(resolutionintegerLabel);
  layout->addLayout(hbox);
  layout->addWidget(applyResolutionButton);
  regularWindow->show();
}

void MainWindow::initializeSplitGridWindow()
{
  splitWindow->setAttribute( Qt::WA_QuitOnClose, false );
  QLabel *SplitLabel = new QLabel(tr("Génération d'une grille initiale par Split and Merge"));

  QLabel *SplitMaxVarianceLabel = new QLabel(tr("Variance Maximale : "));
  splitMaxVarianceSpinBox = new QSpinBox();
  splitMaxVarianceSpinBox->setRange(1, 10000);
  splitMaxVarianceSpinBox->setSingleStep(1);
  splitMaxVarianceSpinBox->setValue(500);
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(SplitMaxVarianceLabel);
  hbox->addWidget(splitMaxVarianceSpinBox);

  QLabel *SplitMaxDistanceLabel = new QLabel(tr("Distance Maximale : "));
  splitMaxDistanceSpinBox = new QSpinBox();
  splitMaxDistanceSpinBox->setRange(1, 10000);
  splitMaxDistanceSpinBox->setSingleStep(1);
  splitMaxDistanceSpinBox->setValue(5);
  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addWidget(SplitMaxDistanceLabel);
  hbox2->addWidget(splitMaxDistanceSpinBox);

  QPushButton *applyResolutionButton = new QPushButton("Appliquer");
  connect(applyResolutionButton, &QPushButton::released, this, &MainWindow::callUpdateSplitGrid);

  QVBoxLayout *layout = new QVBoxLayout(splitWindow);

  layout->addWidget(SplitLabel);
  layout->addLayout(hbox);
  layout->addLayout(hbox2);
  layout->addWidget(applyResolutionButton);
  splitWindow->show();
}
void MainWindow::initializeGradientGridWindow()
{
  gradientWindow->setAttribute( Qt::WA_QuitOnClose, false );
  QLabel *gradientintegerLabel = new QLabel(tr("Génération d'une grille initiale par carte de gradient"));

  QLabel *GradientSeuilLabel = new QLabel(tr("Seuil Points: "));
  gradientSeuilSpinBox = new QSpinBox();
  gradientSeuilSpinBox->setRange(0, 10000);
  gradientSeuilSpinBox->setSingleStep(1);
  gradientSeuilSpinBox->setValue(2);
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(GradientSeuilLabel);
  hbox->addWidget(gradientSeuilSpinBox);

  QLabel *SplitMaxPointsLabel = new QLabel(tr("Max Points : "));
  gradientMaxPointsSpinBox = new QSpinBox();
  gradientMaxPointsSpinBox->setRange(5, 10000);
  gradientMaxPointsSpinBox->setSingleStep(1);
  gradientMaxPointsSpinBox->setValue(300);
  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addWidget(SplitMaxPointsLabel);
  hbox2->addWidget(gradientMaxPointsSpinBox);

  QLabel *GradientPointRateLabel = new QLabel(tr("Taux de Points: "));
  gradientPointRateSpinBox = new QDoubleSpinBox();
  gradientPointRateSpinBox->setRange(0, 1);
  gradientPointRateSpinBox->setSingleStep(0.01);
  gradientPointRateSpinBox->setValue(0.88);
  QHBoxLayout *hbox3 = new QHBoxLayout;
  hbox3->addWidget(GradientPointRateLabel);
  hbox3->addWidget(gradientPointRateSpinBox);

  QPushButton *applyResolutionButton = new QPushButton("Appliquer");
  connect(applyResolutionButton, &QPushButton::released, this, &MainWindow::callUpdateGradientGrid);

  QVBoxLayout *layout = new QVBoxLayout(gradientWindow);

  layout->addWidget(gradientintegerLabel);
  layout->addLayout(hbox);
  layout->addLayout(hbox2);
  layout->addLayout(hbox3);
  layout->addWidget(applyResolutionButton);
  gradientWindow->show();
}
void MainWindow::initializeSobelGridWindow()
{
  sobelWindow->setAttribute( Qt::WA_QuitOnClose, false );
  QLabel *sobelintegerLabel = new QLabel(tr("Génération d'une grille initiale par carte de Sobel"));

  QLabel *SobelSeuilFiltreLabel = new QLabel(tr("Seuil pour le filtre de Sobel : "));
  sobelSeuilFiltreSpinBox = new QSpinBox();
  sobelSeuilFiltreSpinBox->setRange(0, 255);
  sobelSeuilFiltreSpinBox->setSingleStep(1);
  sobelSeuilFiltreSpinBox->setValue(128);
  QHBoxLayout *hbox0 = new QHBoxLayout;
  hbox0->addWidget(SobelSeuilFiltreLabel);
  hbox0->addWidget(sobelSeuilFiltreSpinBox);

  QLabel *SobelSeuilLabel = new QLabel(tr("Seuil Points: "));
  sobelSeuilSpinBox = new QSpinBox();
  sobelSeuilSpinBox->setRange(1, 10000);
  sobelSeuilSpinBox->setSingleStep(1);
  sobelSeuilSpinBox->setValue(128);
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(SobelSeuilLabel);
  hbox->addWidget(sobelSeuilSpinBox);

  QLabel *SobelMaxPointsLabel = new QLabel(tr("Max Points : "));
  sobelMaxPointsSpinBox = new QSpinBox();
  sobelMaxPointsSpinBox->setRange(5, 10000);
  sobelMaxPointsSpinBox->setSingleStep(1);
  sobelMaxPointsSpinBox->setValue(300);
  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addWidget(SobelMaxPointsLabel);
  hbox2->addWidget(sobelMaxPointsSpinBox);

  QLabel *SobelPointRateLabel = new QLabel(tr("Taux de Points: "));
  sobelPointRateSpinBox = new QDoubleSpinBox();
  sobelPointRateSpinBox->setRange(0, 1);
  sobelPointRateSpinBox->setSingleStep(0.01);
  sobelPointRateSpinBox->setValue(0.88);
  QHBoxLayout *hbox3 = new QHBoxLayout;
  hbox3->addWidget(SobelPointRateLabel);
  hbox3->addWidget(sobelPointRateSpinBox);

  QPushButton *applySobelButton = new QPushButton("Appliquer");
  connect(applySobelButton, &QPushButton::released, this, &MainWindow::callUpdateSobelGrid);

  QVBoxLayout *layout = new QVBoxLayout(sobelWindow);

  layout->addWidget(sobelintegerLabel);
  layout->addLayout(hbox0);
  layout->addLayout(hbox);
  layout->addLayout(hbox2);
  layout->addLayout(hbox3);
  layout->addWidget(applySobelButton);
  sobelWindow->show();
}
void MainWindow::initializeOptimizationContinuousWindow()
{
  optimisationWindow->setAttribute( Qt::WA_QuitOnClose, false );

  QLabel *OptimizationEnergySplitThresholdLabel = new QLabel(tr("Seuil Énergie pour split: "));
  optimizationEnergySplitThresholdSpinBox = new QDoubleSpinBox();
  optimizationEnergySplitThresholdSpinBox->setDecimals(3);
  optimizationEnergySplitThresholdSpinBox->setRange(0.001, 100);
  optimizationEnergySplitThresholdSpinBox->setSingleStep(0.001);
  optimizationEnergySplitThresholdSpinBox->setValue(0.20);
  QHBoxLayout *hbox4 = new QHBoxLayout;
  hbox4->addWidget(OptimizationEnergySplitThresholdLabel);
  hbox4->addWidget(optimizationEnergySplitThresholdSpinBox);

  QLabel *OptimizationMinTriangleAreaLabel = new QLabel(tr("Aire du triangle minimum: "));
  optimizationMinTriangleAreaSpinBox = new QDoubleSpinBox();
  optimizationMinTriangleAreaSpinBox->setDecimals(4);
  optimizationMinTriangleAreaSpinBox->setRange(0.0000, 10);
  optimizationMinTriangleAreaSpinBox->setSingleStep(0.0001);
  optimizationMinTriangleAreaSpinBox->setValue(0.0000);
  QHBoxLayout *hbox5 = new QHBoxLayout;
  hbox5->addWidget(OptimizationMinTriangleAreaLabel);
  hbox5->addWidget(optimizationMinTriangleAreaSpinBox);

  QLabel *resolutionintegerLabel = new QLabel(tr("Choisissez votre mode d'optimisation :"));

  QRadioButton *radioNormal = new QRadioButton(tr("Optimisation normale"));
  radioNormal->setChecked(true);
  connect(radioNormal, &QRadioButton::released, this, &MainWindow::callOptimizationTypeChangeToNormal);
  QRadioButton *radioSplit = new QRadioButton(tr("Optimisation split"));
  connect(radioSplit, &QRadioButton::released, this, &MainWindow::callOptimizationTypeChangeToSplit);
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(radioNormal);
  hbox->addWidget(radioSplit);

  QLabel *vitesseOptimisationLabel = new QLabel(tr("Vitesse d'optimisation :"));
  optimizationSpeedSpinBox = new QSpinBox();
  optimizationSpeedSpinBox->setRange(1, 999);
  optimizationSpeedSpinBox->setSingleStep(1);
  optimizationSpeedSpinBox->setValue(800);
  connect(optimizationSpeedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::callChangeOptimizationSpeed);
  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addWidget(vitesseOptimisationLabel);
  hbox2->addWidget(optimizationSpeedSpinBox);

  optimisationTimer = new QTimer(this);
  connect(optimisationTimer, &QTimer::timeout, this, &MainWindow::callOptimizationPass);
  QPushButton *buttonPause = new QPushButton(tr("Pause"));
  connect(buttonPause, &QPushButton::released, this, &MainWindow::callOptimizationContinuousPause);
  QPushButton *buttonPlay = new QPushButton(tr("Play"));
  connect(buttonPlay, &QPushButton::released, this, &MainWindow::callOptimizationContinuousPlay);
  QHBoxLayout *hbox3 = new QHBoxLayout;
  hbox3->addWidget(buttonPause);
  hbox3->addWidget(buttonPlay);

  QVBoxLayout *layout = new QVBoxLayout(optimisationWindow);

  layout->addWidget(resolutionintegerLabel);
  layout->addLayout(hbox4);
  layout->addLayout(hbox5);
  layout->addLayout(hbox);
  layout->addLayout(hbox2);
  layout->addLayout(hbox3);
  optimisationWindow->show();
  isOptimisationWindowInitialized =true;
}
//------------------------------------------------------------------------------------------
void MainWindow::callChangeResolution()
{
  int resolution = resolutionSpinBox->value();
  openGL->changeRegularGridResolution(resolution);
  openGL->update();
}
void MainWindow::callUpdateSplitGrid()
{
  openGL->updateSplitGrid(pgm_filename, splitMaxVarianceSpinBox->value(), splitMaxDistanceSpinBox->value());
  map = "split.pgm";
  initializeMapWindow();
  openGL->update();
}
void MainWindow::callUpdateGradientGrid()
{
  openGL->updateGradientGrid(pgm_filename, gradientSeuilSpinBox->value(), gradientMaxPointsSpinBox->value(), gradientPointRateSpinBox->value());
  map = "gradient.pgm";
  initializeMapWindow();
  openGL->update();
}
void MainWindow::callUpdateSobelGrid()
{
  openGL->updateSobelGrid(pgm_filename, sobelSeuilFiltreSpinBox->value(), sobelSeuilSpinBox->value(), sobelMaxPointsSpinBox->value(), sobelPointRateSpinBox->value());
  map = "sobel.pgm";
  initializeMapWindow();
  openGL->update();
}

void MainWindow::callRenderModeConstant()
{
  openGL->renderModeConstant();
  openGL->update();
}

void MainWindow::callRenderModeGradient()
{
  openGL->renderModeGradient();
  openGL->update();
}

//------------------------------------------------------------------------------------------
void MainWindow::callOptimizationNormalPass()
{
  if (isOptimisationWindowInitialized)
  {
      openGL->optimizationPass(optimizationEnergySplitThresholdSpinBox->value(), optimizationMinTriangleAreaSpinBox->value());
  }
  else
  {
      openGL->optimizationPass(0.5f, 0.001f);
  }
  openGL->update();
}
void MainWindow::callOptimizationSplitPass()
{
  if (isOptimisationWindowInitialized)
  {
      openGL->optimizationSplitPass(optimizationEnergySplitThresholdSpinBox->value(), optimizationMinTriangleAreaSpinBox->value());
  }
  else
  {
      openGL->optimizationSplitPass(0.5f, 0.001);
  }
  openGL->update();
}


void MainWindow::callOptimizationTypeChangeToNormal()
{
  optimisationType = NORMAL;
}
void MainWindow::callOptimizationTypeChangeToSplit()
{
  optimisationType = SPLIT;
}
void MainWindow::callOptimizationContinuousPause()
{
  optimisationTimer->stop();
}
void MainWindow::callOptimizationContinuousPlay()
{
  optimisationTimer->start(optimisationTimeInterval);
}
void MainWindow::callOptimizationPass()
{
  if (optimisationType == NORMAL)
  {
    callOptimizationNormalPass();
  }
  if (optimisationType == SPLIT)
  {
    callOptimizationSplitPass();
  }
}
void MainWindow::callChangeOptimizationSpeed()
{
  optimisationTimeInterval = (int)1000 - optimizationSpeedSpinBox->value();
}
