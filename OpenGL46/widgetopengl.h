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

struct Fog
{
    QVector3D color;
    float density1, density2, level;
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

    void v_transform(float rot_x, float rot_y, float rot_z, float zoom, bool paint = true);
    void m_transform(float height_scale, float height_offset, bool paint = true);
    void move_slider(int _slider);
    void set_fog(float brightness, float density1, float density2, float level);

protected:
    bool init_ok;
    int slider;

    QMatrix4x4 m_matrix, v_matrix, p_matrix;

    Fog fog;

    GLuint shaderProgram;
    GLuint VAO;
    GLuint tex_color, tex_height;

    GLuint loadShader(GLenum type, QString fname);
    GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = 0, GLuint tess_control_shader = 0, GLuint tess_eval_shader = 0);
    GLuint getUniformLocation(GLuint program, const GLchar *uniform);
    GLuint getAttribLocation(GLuint program, const GLchar *attrib);
    GLuint loadTexture2D(QString fname, bool srgb);


    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
};

#endif // WIDGETOPENGL_H
