QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    glwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    renderer.cpp \
    triangulation.cpp \
    triangulationoptimizer.cpp

HEADERS += \
    glwidget.h \
    mainwindow.h \
    renderer.h \
    triangulation.h \
    triangulationoptimizer.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    fragment_shader.glsl \
    vertex_shader.glsl

RESOURCES += \
    RenderingRessources.qrc
