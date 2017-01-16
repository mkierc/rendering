#-------------------------------------------------
#
# Project created by QtCreator 2015-09-12T13:02:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenGL38
TEMPLATE = app

LIBS += -lopengl32 -lqtmain

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
    shading_fragment.glsl \
    shading_vertex.glsl \
    filter_fragment.glsl \
    filter_vertex.glsl \
    hdr_vertex.glsl \
    hdr_fragment.glsl \
    shadowmap_fragment.glsl \
    shadowmap_vertex.glsl \
    shadowmap_geometry.glsl \
    gbuffer_fragment.glsl \
    gbuffer_vertex.glsl
