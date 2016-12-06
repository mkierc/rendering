#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class OpenGLWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    OpenGLWidget(QWidget *parent = 0): QOpenGLWidget(parent) {}

    void initializeGL();
    void paintGL();
};

#endif // OPENGLWIDGET_H
