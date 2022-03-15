#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renderer.h"
#include "triangulation.h"
#include "triangulationoptimizer.h"
#include "helper.h"
#include "glwidget.h"
#include "glwindow.h"


#include <QWidget>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>

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

  QImage image;
  QLabel *imageLabel;
  QScrollArea *scrollArea;
  double scaleFactor = 1;

  QAction *saveAsAct;
  QAction *printAct;
  QAction *copyAct;
  QAction *zoomInAct;
  QAction *zoomOutAct;
  QAction *normalSizeAct;
  QAction *fitToWindowAct;

  Helper helper;

  GLuint texture;

private slots:
  void open();
  void saveAs();
  void zoomIn();
  void zoomOut();
  void normalSize();
  void fitToWindow();
  void about();
};
#endif // MAINWINDOW_H
