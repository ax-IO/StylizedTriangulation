QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    renderer.cpp \
    triangulation.cpp \
    triangulationoptimizer.cpp

HEADERS += \
    mainwindow.h \
    renderer.h \
    triangulation.h \
    triangulationoptimizer.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    fragment_shader.glsl \
    vertex_shader.glsl

RESOURCES += \
    RenderingRessources.qrc

RESOURCES += \
    TriangulationOptimizerResources.qrc


#LIBS += -L//home/spon/lib/gsl-2.7.1 -lgsl -lgslcblas -lm
#LIBS += -L $$PWD/gsl/lib/ -lgsl -lgslcblas -lm
#INCLUDEPATH += $$PWD/gsl/include/


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/gsl/lib/release/ -lgsl -lgslcblas
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/gsl/lib/debug/ -lgsl -lgslcblas
else:unix: LIBS += -L$$PWD/gsl/lib/ -lgsl -lgslcblas

INCLUDEPATH += $$PWD/gsl/include
DEPENDPATH += $$PWD/gsl/include
