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
    void move_light(float x, float y, float z);
    void move_slider(int _slider);
    void set_gamma(float _gamma);
    void set_light_brightness(float b);

protected:
    bool init_ok;
    int triangles_cnt, triangles_shadow_cnt, triangles_light_cnt, triangles_floor_cnt;

    QMatrix4x4 m_matrix, v_matrix, p_matrix, l_matrix[6];
    int slider;

    Light light;
    float gamma;

    GLuint shaderProgram_Shading, shaderProgram_ShadowMap, shaderProgram_HDR, shaderProgram_Filter;
    GLuint VAO, VAO_shadow, VAO_light, VAO_floor, VAO_screen;
    GLuint tex_color, tex_color_floor, tex_FBO_depth, tex_FBO_HDR_color[2], tex_FBO_HDR_depth, tex_FBO_filter[2]; // !!!
    GLuint FBO, FBO_HDR, FBO_filter[2]; // !!!

    GLuint loadShader(GLenum type, QString fname);
    GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = 0, GLuint tess_eval_shader = 0, GLuint tess_control_shader = 0);
    GLuint loadTexture2D(QString fname, bool srgb);
    GLuint loadTextureCube(QString fname, QString fext);
    GLuint getUniformLocation(GLuint program, const GLchar *uniform);
    GLuint getAttribLocation(GLuint program, const GLchar *attrib);
    QMatrix3x3 upperLeftMatrix3x3(QMatrix4x4 const &m);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void makeShadowMatrixes();
    void paintScene(boolean gen_shadow_map);
    void paintHDR();
    void paintFilter(GLuint textureIn, boolean horizontal);
};


#endif // WIDGETOPENGL_H
