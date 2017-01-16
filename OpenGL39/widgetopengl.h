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
    void move_light(float x, float y, float z); // !!!
    void move_slider(int _slider);
    void set_ssao_radius(float _radius);

protected:
    bool init_ok;
    int triangles_cnt, triangles_floor_cnt, triangles_light_cnt;

    QMatrix4x4 m_matrix, v_matrix, p_matrix;
    int slider;

    Light light;

    static const int ssao_kernel_size = 64;
    QVector3D ssao_kernel[ssao_kernel_size];
    float ssao_radius;

    GLuint shaderProgram_gbuffer, shaderProgram_shading, shaderProgram_ssao;
    GLuint VAO, VAO_floor, VAO_light, VAO_screen;
    GLuint tex_color, tex_color_floor, tex_color_light;

    GLuint FBO_gBuffer;
    GLuint tex_FBO_gBuffer_position, tex_FBO_gBuffer_normal, tex_FBO_gBuffer_color, tex_FBO_gBuffer_viewdepth, tex_FBO_gBuffer_depth;

    GLuint FBO_ssao;
    GLuint tex_FBO_ssao_color;

    GLuint loadShader(GLenum type, QString fname);
    GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = 0, GLuint tess_eval_shader = 0, GLuint tess_control_shader = 0);
    GLuint getUniformLocation(GLuint program, const GLchar *uniform);
    GLuint getAttribLocation(GLuint program, const GLchar *attrib);
    QMatrix3x3 upperLeftMatrix3x3(QMatrix4x4 const &m);
    GLuint loadTexture2D(QString fname, bool srgb);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    float f_rand(float min, float max);
    void generateSSAOKernel();
    void paintScene();
    void paintSSAO();
    void paintAndShade();
};


#endif // WIDGETOPENGL_H
