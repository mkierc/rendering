#include "widgetopengl.h"

#include <QDebug>
#include <QFile>

#define GRID_SIZE 128
#define TERRAIN   "terrain" // terrain mandelbrot m

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


GLuint WidgetOpenGL::linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader, GLuint tess_control_shader, GLuint tess_eval_shader)
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // filtry
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // zaladowanie
    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB : GL_RGB, tex.width(), tex.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, tex.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

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
        qDebug() << (char *)glGetString(GL_RENDERER);
        qDebug() << (char *)glGetString(GL_VERSION);
        qDebug() << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

        // !!!
        GLint MaxPatchVertices;
        glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
        qDebug() << "Max patch vertices:" << MaxPatchVertices;



        ////////////////////////////////////////////////////////////////
        // Shadery
        ////////////////////////////////////////////////////////////////

        // ladujemy shadery
        GLuint vertexShader   = loadShader(GL_VERTEX_SHADER,   "vertex.glsl");
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment.glsl");
        GLuint tessControlShader = loadShader(GL_TESS_CONTROL_SHADER, "tess_control.glsl");
        GLuint tessEvalShader = loadShader(GL_TESS_EVALUATION_SHADER, "tess_eval.glsl");
        shaderProgram = linkProgram(vertexShader, fragmentShader, 0, tessControlShader, tessEvalShader);


        ////////////////////////////////////////////////////////////////
        // Kwadrat
        ////////////////////////////////////////////////////////////////

        GLfloat square[4*3] = {-0.5, 0, -0.5,
                               -0.5, 0,  0.5,
                                0.5, 0,  0.5,
                                0.5, 0, -0.5};

        ////////////////////////////////////////////////////////////////
        // Bitmapy
        ////////////////////////////////////////////////////////////////

        tex_color = loadTexture2D("../Modele/" TERRAIN "_color.jpg", false);
        tex_height = loadTexture2D("../Modele/" TERRAIN "_height.jpg", false);


        ////////////////////////////////////////////////////////////////
        // Mgla
        ////////////////////////////////////////////////////////////////

        fog.color = QVector3D(1.0, 1.0, 1.0)*0.75;
        fog.density1 = 0.03;
        fog.density2 = 0.18;
        fog.level    = 0.6;


        ////////////////////////////////////////////////////////////////
        // Vertex Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), square, GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // wspolrzene wierzcholkow
        GLint attr = getAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // Inne inicjalizacje OpenGL
        ////////////////////////////////////////////////////////////////

        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);

        v_transform(21.8, 9.4, 0.0, 0.58, false);
        m_transform(3.3, -0.65, false);
        slider = 0;

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
        glClearColor(fog.color.x(), fog.color.y(), fog.color.z(), 1.0);

        // czyscimy ekran i bufor glebokosci
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rysujemy
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // macierz
        int attr = getUniformLocation(shaderProgram, "pvm_matrix");
        QMatrix4x4 pvm_matrix = p_matrix*v_matrix*m_matrix;
        glUniformMatrix4fv(attr, 1, GL_FALSE, pvm_matrix.data());

        // bok grida
        attr = getUniformLocation(shaderProgram, "grid_size");
        glUniform1i(attr, GRID_SIZE);

        // tekstury...
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_color);
        glUniform1i(getUniformLocation(shaderProgram, "textureColor"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_height);
        glUniform1i(getUniformLocation(shaderProgram, "textureHeight"), 1);

        // mgla
        attr = getUniformLocation(shaderProgram, "fogColor");
        glUniform3f(attr, fog.color.x(), fog.color.y(), fog.color.z());

        attr = getUniformLocation(shaderProgram, "fogDensity1");
        glUniform1f(attr, fog.density1);

        attr = getUniformLocation(shaderProgram, "fogDensity2");
        glUniform1f(attr, fog.density2);

        attr = getUniformLocation(shaderProgram, "fogLevel");
        glUniform1f(attr, fog.level);

        // slider
        attr = getUniformLocation(shaderProgram, "slider");
        glUniform1i(attr, slider);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glDrawArraysInstanced(GL_PATCHES, 0, 4, GRID_SIZE*GRID_SIZE);

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

    // macierz perspektywy...
    // p_matrix.ortho(-r, r, -1, 1, 0.1, 7);
    p_matrix.perspective(45, r, 0.1, 500);
    p_matrix.translate(0, 0, -5);
}



void WidgetOpenGL::v_transform(float rot_x, float rot_y, float rot_z, float zoom, bool paint)
{
    v_matrix.setToIdentity();
    v_matrix.rotate(rot_x, 1, 0, 0);
    v_matrix.rotate(rot_y, 0, 1, 0);
    v_matrix.rotate(rot_z, 0, 0, 1);
    v_matrix.scale(zoom);

    if (paint)
        repaint();
}


void WidgetOpenGL::m_transform(float height_scale, float height_offset, bool paint)
{
    m_matrix.setToIdentity();
    m_matrix.scale(1.0, height_scale, 1.0);
    m_matrix.translate(0.0, height_offset, 0.0);

    if (paint)
        repaint();
}


void WidgetOpenGL::move_slider(int _slider)
{
    slider = _slider;

    repaint();
}


void WidgetOpenGL::set_fog(float brightness, float density1, float density2, float level)
{
    fog.color = QVector3D(1.0, 1.0, 1.0)*brightness;
    fog.density1 = density1;
    fog.density2 = density2;
    fog.level = level;

    repaint();
}
