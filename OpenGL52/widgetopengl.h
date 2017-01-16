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


struct InstanceData
{
    GLfloat x, y, z;
    GLfloat r, g, b;
};


class WidgetOpenGL: public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
public:
    WidgetOpenGL(QWidget *parent = 0): QOpenGLWidget(parent)
    {
        QSurfaceFormat format;
        format.setSamples(16);
        setFormat(format);
    }

    void v_transform(float rot_x, float rot_y, float rot_z, float zoom);
    void move_slider(int _slider);
    void set_size(float p_size);
    void set_alpha(float _alpha);

protected:
    bool init_ok;

    QMatrix4x4 m_matrix, v_matrix, p_matrix;
    int slider;
    float particle_size, alpha;

    GLuint shaderProgram;
    GLuint VAO;
    GLuint tex_billboard;

    GLuint loadShader(GLenum type, QString fname);
    GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = 0, GLuint tess_eval_shader = 0, GLuint tess_control_shader = 0);
    GLuint getUniformLocation(GLuint program, const GLchar *uniform);
    GLuint getAttribLocation(GLuint program, const GLchar *attrib);
    QMatrix3x3 upperLeftMatrix3x3(QMatrix4x4 const &m);
    GLuint loadTexture2D(QString fname);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    float f_rand(float min, float max);
};


#endif // WIDGETOPENGL_H
