#include "openglwidget.h"

#include <QDebug>


void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    qDebug() << (char *)glGetString(GL_EXTENSIONS);
    qDebug() << (char *)glGetString(GL_RENDERER);
    qDebug() << (char *)glGetString(GL_VERSION);
    qDebug() << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // http://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt
    GLint x;
    glGetIntegerv(0x9049, &x); // GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
    qDebug() << "MEM:" << x << "KB";
}


void OpenGLWidget::paintGL()
{

}
