#-------------------------------------------------
#
# Project created by QtCreator 2015-09-12T13:02:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenGL62
TEMPLATE = app

#LIBS += -lglu32  #-lgdi32 #-luser32 #-lmingw32
LIBS += -lopengl32 -lqtmain

#-lQt5OpenGL #s-lQt5Widgets #-lQt5Gui #-lQt5Core

SOURCES += main.cpp\
        mainwindow.cpp \
    widgetopengl.cpp \
    model.cpp

HEADERS  += mainwindow.h \
    widgetopengl.h \
    model.h \
    light.h \
    material.h

FORMS    += mainwindow.ui

DISTFILES += \
    readme.txt \
    gbuffer_fragment.glsl \
    gbuffer_vertex.glsl \
    shading_fragment.glsl \
    shading_vertex.glsl
