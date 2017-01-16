#include "mainwindow.h"
#include "widgetopengl.h"
#include "model.h"

#include <QDebug>
#include <QFile>
#include <QtMath>

#define LAYERS 200

GLuint WidgetOpenGL::loadShader(GLenum type, QString fname)
{
  // wczytanie pliku
  QFile f(fname);
  if (!f.open(QFile::ReadOnly | QFile::Text))
      throw QString("Nie moge odczytac pliku '%1'").arg(fname);

  QTextStream in(&f);
  std::string s = in.readAll().toStdString();
  GLchar *shader_source = (GLchar *)(s.c_str());
  f.close();

  // zaladowanie shadera i kompilacja
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &shader_source, NULL);
  glCompileShader(shader);

  // czy kompilacja ok?
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  GLchar infoLog[512];
  glGetShaderInfoLog(shader, 512, NULL, infoLog);
  qDebug() << "Kompilacja shadera:" << fname << "\n" << infoLog;
  if (!success)
      throw QString("Blad shadera '%1': %2").arg(fname).arg(infoLog);

  return shader;
}


GLuint WidgetOpenGL::linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader, GLuint tess_eval_shader, GLuint tess_control_shader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    if (geometry_shader)
        glAttachShader(program, geometry_shader);
    if (tess_eval_shader)
        glAttachShader(program, tess_eval_shader);
    if (tess_control_shader)
        glAttachShader(program, tess_control_shader);
    glLinkProgram(program);

    // czy kompilacja ok?
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        throw QString("Blad linkowania: %1").arg(infoLog);
    }

    // shadery staly sie czescia programu, mozna skasowac
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (geometry_shader)
        glDeleteShader(geometry_shader);
    if (tess_eval_shader)
        glDeleteShader(tess_eval_shader);
    if (tess_control_shader)
        glDeleteShader(tess_control_shader);

    return program;
}


GLuint WidgetOpenGL::getUniformLocation(GLuint program, const GLchar *uniform)
{
    int a = glGetUniformLocation(program, uniform);
    if (a < 0) throw QString("Nieprawidlowy parametr '%1' (program %2)").arg(uniform).arg(program);
    return a;
}


GLuint WidgetOpenGL::getAttribLocation(GLuint program, const GLchar *attrib)
{
    int a = glGetAttribLocation(program, attrib);
    if (a < 0) throw QString("Nieprawidlowy parametr '%1' (program %2)").arg(attrib).arg(program);
    return a;
}


GLuint WidgetOpenGL::loadTexture2D(QString fname, bool srgb)
{
    QImage tex(fname);
    if (tex.byteCount() == 0) throw QString("Nie udalo sie wczytac tekstury '%1'").arg(fname);

    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);

    // wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // filtry
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // zaladowanie
    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB : GL_RGB, tex.width(), tex.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, tex.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return t;
}


GLuint WidgetOpenGL::loadTexture3DFromRaw(QString fname, VolumetricDataInfo const &dataInfo)
{
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly))
        throw QString("Nie udalo sie otworzyc pliku '%1'").arg(fname);

    QDataStream ds(&f);

    GLuint t;
    glGenTextures(1, &t);

    GLbyte *data = new GLbyte[dataInfo.width*dataInfo.height*dataInfo.slices*4]; // RGBA

    qDebug() << dataInfo.width*dataInfo.height*dataInfo.slices*4;

    int p = 0;
    for (int s = 0; s < dataInfo.slices; s++)
    {
        for (int i = 0; i < dataInfo.width*dataInfo.height; i++, p++)
        {
            GLbyte r, g, b, a;

            if (dataInfo.bits <= 8)
            {
                quint8 c;
                ds >> c;
                r = g = b = a = c;
            }
            else if (dataInfo.bits <= 16)
            {
                quint16 c;
                ds >> c;
                r = g = b = a = (c >> (dataInfo.bits - 8));
            }
            else
                throw QString("Liczba bitow wieksza niz 16");

            data[p*4 + 0] = r;
            data[p*4 + 1] = g;
            data[p*4 + 2] = b;
            data[p*4 + 3] = a;
        }

        qDebug() << (s + 1) << "/" << dataInfo.slices;
    }

    f.close();

    glBindTexture(GL_TEXTURE_3D, t);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA,
                 dataInfo.width, dataInfo.height, dataInfo.slices,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, 0);

    return t;
}


void WidgetOpenGL::initializeGL()
{
    // zaczynamy pesymistycznie...
    init_ok = false;

    try
    {
        // jaka jest dostepna wersja OpenGL?
        OpenGLVersionTest test;
        QString version = test.version();
        if (version < MIN_OPENGL_VERSION)
            throw QString("Zla wersja OpenGL: %1").arg(version);

        // specyficzne dla Qt:
        initializeOpenGLFunctions();

        // jakie mamy dostepne rozszerzenia, itp.
        qDebug() << (char *)glGetString(GL_EXTENSIONS);
        qDebug() << (char *)glGetString(GL_RENDERER);
        qDebug() << (char *)glGetString(GL_VERSION);
        qDebug() << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        GLint x;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &x);
        qDebug() << "max textures:" << x;

        ////////////////////////////////////////////////////////////////
        // Shadery
        ////////////////////////////////////////////////////////////////

        GLuint vertexShader   = loadShader(GL_VERTEX_SHADER,   "vertex.glsl");
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment.glsl");
        shaderProgram = linkProgram(vertexShader, fragmentShader);


        ////////////////////////////////////////////////////////////////
        // Model (kwadrat)
        ////////////////////////////////////////////////////////////////

        /*

        2 (0,0)    3 (1,0)
             +------+
             |\     |
             | \    |
             |  \   |
             |   \  |
             |    \ |
             |     \|
             +------+
        0 (0,1)    1 (1,1)

        */

        GLfloat model[4*5] = { -1.0, -1.0, 0.0,    0.0, 0.0,
                                1.0, -1.0, 0.0,    1.0, 0.0,
                               -1.0,  1.0, 0.0,    0.0, 1.0,
                                1.0,  1.0, 0.0,    1.0, 1.0 };


        ////////////////////////////////////////////////////////////////
        // Tekstury
        ////////////////////////////////////////////////////////////////

//        dataInfo = VolumetricDataInfo(416, 512, 112, 16, 0.412, 0.412, 0.412);
//        tex3D = loadTexture3DFromRaw("../Modele/Vol/angio.raw", dataInfo);

        dataInfo = VolumetricDataInfo(512, 512, 174, 16, 0.8398, 0.8398, 3.2);
        tex3D = loadTexture3DFromRaw("../Modele/Vol/stent16.raw", dataInfo);

//        dataInfo = VolumetricDataInfo(256, 256, 256, 8, 1.0, 1.0, 1.0);
//        tex3D = loadTexture3DFromRaw("../Modele/Vol/skull.raw", dataInfo);


        mainWindow->setSplitter(LAYERS);

        ////////////////////////////////////////////////////////////////
        // Vertex Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 4*5*sizeof(GLfloat), model, GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // wspolrzene wierzcholkow
        GLint attr = getAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // wspolrzene tekstur
        attr = getAttribLocation(shaderProgram, "texCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 4. Inne inicjalizacje OpenGL
        ////////////////////////////////////////////////////////////////

        glClearColor(0, 0, 0, 1);
//        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        v_transform(90, 0, 0, 1);
        slider = LAYERS;
        alpha = 1.0;
        treshold = 0.0;

        init_ok = true;
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w initializeGL():" << msg;
    }
}


void WidgetOpenGL::paintGL()
{
    if (!init_ok) return;

    try
    {
        // czyscimy ekran i bufor glebokosci
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int attr = getUniformLocation(shaderProgram, "alpha");
        glUniform1f(attr, alpha);

        attr = getUniformLocation(shaderProgram, "treshold");
        glUniform1f(attr, treshold);

        // tekstura
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, tex3D);
        attr = getUniformLocation(shaderProgram, "tex");
        glUniform1i(attr, 0);

        QMatrix4x4 pv_matrix = v_matrix*p_matrix;
        attr = getUniformLocation(shaderProgram, "pv_matrix");
        glUniformMatrix4fv(attr, 1, GL_FALSE, pv_matrix.data());

        for (int i = 0; i < LAYERS; i++)
        {
            if (i > slider) break;
            attr = getUniformLocation(shaderProgram, "layer");
            glUniform1f(attr, 2.0*i/float(LAYERS) - 0.5);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        // odczepiamy VAO
        glBindVertexArray(0);
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w paintGL():" << msg;
    }
}


void WidgetOpenGL::resizeGL(int w, int h)
{
    p_matrix.setToIdentity();
    float r = float(w)/float(h);

    p_matrix.translate(0.5, 0.5, 0.5);
    p_matrix.scale(r, 1.0, 1.0);
    p_matrix.translate(-0.5, -0.5, -0.5);
}


void WidgetOpenGL::v_transform(float rot_x, float rot_y, float rot_z, float zoom)
{
    v_matrix.setToIdentity();
    v_matrix.translate(0.5, 0.5, 0.5);

    v_matrix.scale(1.0,
                   dataInfo.width*dataInfo.spacing_x/(dataInfo.height*dataInfo.spacing_y),
                   dataInfo.width*dataInfo.spacing_x/(dataInfo.slices*dataInfo.spacing_z));

    v_matrix.rotate(rot_x, 1, 0, 0);
    v_matrix.rotate(rot_y, 0, 1, 0);
    v_matrix.rotate(rot_z, 0, 0, 1);
    v_matrix.scale(1.5/zoom, 1.5/zoom, 1.0);

    v_matrix.translate(-0.5, -0.5, -0.5);

    repaint();
}

void WidgetOpenGL::set_slider(int slider)
{
    this->slider = slider;
    repaint();
}


void WidgetOpenGL::set_alpha(float alpha)
{
    this->alpha = alpha;
    repaint();
}


void WidgetOpenGL::set_treshold(float treshold)
{
    this->treshold = treshold;
    repaint();
}
