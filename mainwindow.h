#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renderer.h"
#include "triangulation.h"
#include "triangulationoptimizer.h"
#include "glwidget.h"


#include <QWidget>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <QSpinBox>
#include <QRadioButton>

enum OptimisationType { NORMAL, SPLIT };

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  bool loadFile(const QString &);

private:
  void createActions();
  void updateActions();
  bool saveFile(const QString &fileName);
  void setImage(const QImage &newImage);
  void loadImageToTexture(const QImage &img);
  void scaleImage(double factor);
  void adjustScrollBar(QScrollBar *scrollBar, double factor);

  int filebar_height = 34;
  int statusbar_height = 27;

  QString map;
  QLabel *mapLabel;
  QWidget *mapWindow;
  QWidget *regularWindow;
  QWidget *splitWindow;
  QWidget *gradientWindow;
  QWidget *sobelWindow;
  QWidget *optimisationWindow;

  //--------------------------------------------
  QSpinBox *splitMaxVarianceSpinBox;
  QSpinBox *splitMaxDistanceSpinBox;

  QSpinBox *resolutionSpinBox;
  QSpinBox *gradientSeuilSpinBox;
  QSpinBox *gradientMaxPointsSpinBox;
  QDoubleSpinBox *gradientPointRateSpinBox;

  QSpinBox *sobelSeuilFiltreSpinBox;
  QSpinBox *sobelSeuilSpinBox;
  QSpinBox *sobelMaxPointsSpinBox;
  QDoubleSpinBox *sobelPointRateSpinBox;

  QDoubleSpinBox *optimizationEnergySplitThresholdSpinBox;
  QDoubleSpinBox *optimizationMinTriangleAreaSpinBox;

  QSpinBox *optimizationSpeedSpinBox;
  int optimisationType = NORMAL;
  QTimer *optimisationTimer;
  int optimisationTimeInterval=999;
  //--------------------------------------------

  QImage image;
  QImage image_to_save;
  QImage image_to_display;
  QString pgm_filename;

  QScrollArea *scrollArea;
  double scaleFactor = 1;

  bool isImageSquare;
  QLabel *imageLabel;
  GLWidget *openGL;

  //--------------------------------------------

  QAction *openAct;
  QAction *saveAsAct;
  QAction *printAct;
  QAction *copyAct;

  //--------------------------------------------

  QAction *renderModeConstantAct;
  QAction *renderModeGradientAct;

  //--------------------------------------------

  QAction *showInitializeRegularGridWindowAct;
  QAction *showInitializeSplitGridWindowAct;
  QAction *showInitializeGradientGridWindowAct;
  QAction *showInitializeSobelGridWindowAct;

  //--------------------------------------------

  QAction *optimizationPassAct;
  QAction *optimizationSplitPassAct;
  QAction *optimizationContinuousAct;



  GLuint texture;

private slots:
  void open();
  void saveAs();
  void zoomIn();
  void zoomOut();
  void normalSize();
  void fitToWindow();
  void about();

  void initializeMapWindow();

  void initializeRegularGridWindow();
  void initializeSplitGridWindow();
  void initializeGradientGridWindow();
  void initializeSobelGridWindow();
  void initializeOptimizationContinuousWindow();

  void callChangeResolution();
  void callUpdateSplitGrid();
  void callUpdateGradientGrid();
  void callUpdateSobelGrid();

  void callRenderModeConstant();
  void callRenderModeGradient();

  void callOptimizationPass();
  void callOptimizationNormalPass();
  void callOptimizationSplitPass();

  void callOptimizationTypeChangeToNormal();
  void callOptimizationTypeChangeToSplit();
  void callOptimizationContinuousPause();
  void callOptimizationContinuousPlay();
  void callChangeOptimizationSpeed();

};
#endif // MAINWINDOW_H
