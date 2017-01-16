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


class VolumetricDataInfo
{
public:
    int width, height, slices, bits;
    float spacing_x, spacing_y, spacing_z;

    VolumetricDataInfo(){}
    VolumetricDataInfo(int _width, int _height, int _slices, int _bits, float _spacing_x, float _spacing_y, float _spacing_z):
        width(_width), height(_height), slices(_slices), bits(_bits),
        spacing_x(_spacing_x), spacing_y(_spacing_y), spacing_z(_spacing_z) {}
};


class WidgetOpenGL: public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
public:
    WidgetOpenGL(QWidget *parent = 0): QOpenGLWidget(parent){}

    void v_transform(float rot_x, float rot_y, float rot_z, float zoom);
    void set_slider(int slider);
    void set_alpha(float alpha);
    void set_treshold(float treshold);

protected:
    bool init_ok;

    QMatrix4x4 m_matrix, v_matrix, p_matrix;

    GLuint shaderProgram;
    GLuint VAO;
    GLuint *texarray;
    VolumetricDataInfo dataInfo;

    int slider;
    float alpha, treshold;

    GLuint loadShader(GLenum type, QString fname);
    GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = 0, GLuint tess_eval_shader = 0, GLuint tess_control_shader = 0);
    GLuint getUniformLocation(GLuint program, const GLchar *uniform);
    GLuint getAttribLocation(GLuint program, const GLchar *attrib);
    GLuint loadTexture2D(QString fname, bool srgb);
    GLuint *loadTexturesFromRaw(QString fname, VolumetricDataInfo const &dataInfo);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
};

#endif // WIDGETOPENGL_H
