#ifndef WIDGETOPENGL_H
#define WIDGETOPENGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_0>
#include <QOpenGLFunctions_4_5_Core>
#include <QMatrix4x4>

#include "light.h"
#include "material.h"


#define MIN_OPENGL_VERSION "4.5"


class OpenGLVersionTest: public QOpenGLFunctions_1_0
{
public:
    QString version()
    {
        initializeOpenGLFunctions();
        return (char *)glGetString(GL_VERSION);
    }
};


class WidgetOpenGL: public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
public:
    WidgetOpenGL(QWidget *parent = 0): QOpenGLWidget(parent){}

    void v_transform(float rot_x, float rot_y, float rot_z, float zoom);

protected:
    bool init_ok;
    int triangles_cnt, triangles_light_cnt;

    QMatrix4x4 m_matrix, v_matrix, p_matrix;

    GLuint shaderProgram;
    GLuint VAO, VAO_light, MAT; // !!!
    bool MAT_uploaded; // !!!

    GLuint loadShader(GLenum type, QString fname);
    QMatrix3x3 upperLeftMatrix3x3(QMatrix4x4 const &m);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
};


#endif // WIDGETOPENGL_H
