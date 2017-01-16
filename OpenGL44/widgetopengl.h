#ifndef WIDGETOPENGL_H
#define WIDGETOPENGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_0>
#include <QOpenGLFunctions_4_5_Core>
#include <QMatrix4x4>

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
    WidgetOpenGL(QWidget *parent = 0): QOpenGLWidget(parent)
    {
        QSurfaceFormat format;
        format.setSamples(16);
        setFormat(format);
    }

    void v_transform(float rot_x, float rot_y, float rot_z, float zoom);
    void set_tess(float tess_in0, float tess_in1, float tess_out0, float tess_out1, float tess_out2, float tess_out3);

protected:
    bool init_ok;

    QMatrix4x4 m_matrix, v_matrix, p_matrix;

    GLuint shaderProgram;
    GLuint VAO;

    GLfloat tessInner0, tessInner1, tessOuter0, tessOuter1, tessOuter2, tessOuter3;

    GLuint loadShader(GLenum type, QString fname);
    GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = 0, GLuint tess_control_shader = 0, GLuint tess_eval_shader = 0);
    GLuint getUniformLocation(GLuint program, const GLchar *uniform);
    GLuint getAttribLocation(GLuint program, const GLchar *attrib);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
};

#endif // WIDGETOPENGL_H
