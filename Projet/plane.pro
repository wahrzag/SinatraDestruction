QT       += core gui widgets

TARGET = plane
TEMPLATE = app
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17

SOURCES += main.cpp \
    quadnode.cpp \
    camera.cpp

SOURCES += \
    mainwidget.cpp \
    geometryengine.cpp

HEADERS += \
    mainwidget.h \
    geometryengine.h \
    quadnode.h \
    camera.h \
    destructmesh.h

RESOURCES += \
    shaders.qrc \
    textures.qrc

# install
target.path = .
INSTALLS += target
