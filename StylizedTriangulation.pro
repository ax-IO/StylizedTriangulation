#QT       += core gui opengl
QT       += core gui opengl openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++2a

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PWD/lib.pri)

SOURCES += \
    common.cpp \
    delaunator.cpp \
    generategrid.cpp \
    image_ppm.cpp \
    main.cpp \
    glwidget.cpp\
    mainwindow.cpp \
    renderer.cpp \
    triangulation.cpp \
    triangulationoptimizer.cpp

HEADERS += \
    common.h\
    image_ppm.h\
    delaunator.hpp\
    generategrid.h \
    mainwindow.h \
    glwidget.h\
    renderer.h \
    triangulation.h \
    triangulationoptimizer.h

FORMS +=

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    fragment_shader.glsl \
    vertex_shader.glsl

RESOURCES += \
    LogoRessource.qrc \
    RenderingRessources.qrc

RESOURCES += \
    TriangulationOptimizerResources.qrc

INCLUDEPATH += $$PWD/lib/


#installer gsl
#LIBS += -L /usr/include/gsl -lgsl -lgslcblas

