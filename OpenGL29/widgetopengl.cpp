#include "widgetopengl.h"
#include "model.h"

#include <QDebug>
#include <QFile>

#define MODEL         "texcube.obj"
#define MODEL_FLOOR   "plane.obj"
#define TEXTURE       "wood2.jpg"
#define TEXTURE_FLOOR "bricks2.jpg" // stones2 bricks2


#define SHADOW_FRAMEBUFFER_WIDTH   1024
#define SHADOW_FRAMEBUFFER_HEIGHT  1024


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


GLuint WidgetOpenGL::loadTexture2D(QString fname)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width(), tex.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, tex.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return t;
}


GLuint WidgetOpenGL::loadTextureCube(QString fname, QString fext)
{
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_CUBE_MAP, t);

    QString m[] = {"right", "left", "top", "bottom", "front", "back"};

    for (int i = 0; i < 6; i++)
    {
        QImage tex(fname + "_" + m[i] + fext);
        if (tex.byteCount() == 0) throw QString("Nie udalo sie wczytac tekstury '%1_%2%3'").arg(fname).arg(m[i]).arg(fext);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, tex.width(), tex.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, tex.bits());

        // wrapping
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // filtry
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return t;
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


        ////////////////////////////////////////////////////////////////
        // CZ 1. SHADERY
        ////////////////////////////////////////////////////////////////

        // ladujemy shadery, tworzymy program...
        GLuint vertexShader   = loadShader(GL_VERTEX_SHADER,   "vertex.glsl");
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment.glsl");
        shaderProgram = linkProgram(vertexShader, fragmentShader);

        vertexShader   = loadShader(GL_VERTEX_SHADER,   "vertex_shadow.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment_shadow.glsl");
        shaderProgramShadow = linkProgram(vertexShader, fragmentShader);


        ////////////////////////////////////////////////////////////////
        // CZ 2. Wczytanie modeli
        ////////////////////////////////////////////////////////////////

        Model model;
        model.readFile("../Modele/" MODEL, true, true, 0.8);
        triangles_cnt = model.getVertDataCount();

        Model model_floor;
        model_floor.readFile("../Modele/" MODEL_FLOOR, true, true, 0.8);
        triangles_floor_cnt = model_floor.getVertDataCount();

        ////////////////////////////////////////////////////////////////
        // CZ 2T. Wczytanie tekstur
        ////////////////////////////////////////////////////////////////

        TEX = loadTexture2D("../Modele/" TEXTURE);
        TEX_floor = loadTexture2D("../Modele/" TEXTURE_FLOOR);


        ////////////////////////////////////////////////////////////////
        // CZ 3. Vertex Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model.getVertDataSize(), model.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // wspolrzene wierzcholkow
        GLint attr = getAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // wspolrzedne tekstury
        attr = getAttribLocation(shaderProgram, "textureCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3F. Vertex Buffer Object + Vertex Array Object (podloga)
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_floor.getVertDataSize(), model_floor.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_floor);
        glBindVertexArray(VAO_floor);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_floor.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_floor.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // wspolrzedne tekstury
        attr = getAttribLocation(shaderProgram, "textureCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, model_floor.getVertDataStride()*sizeof(GLfloat), (void *)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3L. Oświetlenie kierunkowe
        ////////////////////////////////////////////////////////////////

        light.setDir(0.0, 1.0, 0.0);


        ////////////////////////////////////////////////////////////////
        // CZ 4. Framebuffer
        ////////////////////////////////////////////////////////////////

        glGenTextures(1, &TEX_FBO_DEPTH);
        glBindTexture(GL_TEXTURE_2D, TEX_FBO_DEPTH);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_FRAMEBUFFER_WIDTH, SHADOW_FRAMEBUFFER_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TEX_FBO_DEPTH, 0);
        glDrawBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        ////////////////////////////////////////////////////////////////
        // CZ 5. Inne inicjalizacje OpenGL
        ////////////////////////////////////////////////////////////////

        glClearColor(0, 0, 0, 1);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        v_transform(20, 0, 0, 1);

        init_ok = true;
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w initializeGL():" << msg;
    }
}


QMatrix3x3 WidgetOpenGL::upperLeftMatrix3x3(QMatrix4x4 const &m)
{
    QMatrix3x3 r;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            r(i, j) = m(i, j);
    return r;
}


void WidgetOpenGL::paintGL()
{
    if (!init_ok) return;

    try
    {
        GLint FBO_screen;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &FBO_screen);

        // rendering cienia
        glViewport(0, 0, SHADOW_FRAMEBUFFER_WIDTH, SHADOW_FRAMEBUFFER_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        makeShadowMatrix();
        paintScene(true);


        // rendering wlasciwy
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_screen);
        glViewport(0, 0, width()*devicePixelRatio(), height()*devicePixelRatio());
        paintScene(false);
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w paintGL():" << msg;
    }
}


void WidgetOpenGL::paintScene(boolean gen_shadow_map)
{
    GLuint prog = gen_shadow_map ? shaderProgramShadow : shaderProgram;


    // czyscimy ekran i bufor glebokosci
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rysujemy
    glUseProgram(prog);
    glBindVertexArray(VAO);

    // udostepnienie tekstury
    if (!gen_shadow_map)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TEX);
        int attr_tex = getUniformLocation(prog, "textureBitmap");
        glUniform1i(attr_tex, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TEX_FBO_DEPTH);
        attr_tex = getUniformLocation(prog, "textureShadow");
        glUniform1i(attr_tex, 1);
    }

    // macierze
    int attr_lm = getUniformLocation(prog, "light_matrix");
    glUniformMatrix4fv(attr_lm, 1, GL_FALSE, l_matrix.data());

    if (!gen_shadow_map)
    {
        int attr_pm = getUniformLocation(prog, "p_matrix");
        glUniformMatrix4fv(attr_pm, 1, GL_FALSE, p_matrix.data());

        int attr_vm = getUniformLocation(prog, "v_matrix");
        glUniformMatrix4fv(attr_vm, 1, GL_FALSE, v_matrix.data());
    }

    int attr_mm = getUniformLocation(prog, "m_matrix");

    // macierz dla normalnych
    int attr_n = 0;
    if (!gen_shadow_map)
        attr_n = getUniformLocation(prog, "norm_matrix");

    // obserwator
    if (!gen_shadow_map)
    {
        int attr_eye = getUniformLocation(prog, "eyePos");
        QVector4D eye = v_matrix.inverted()*QVector4D(0, 0, 5, 1);
        GLfloat eyefl[] = {eye.x(), eye.y(), eye.z()};
        glUniform3fv(attr_eye, 1, eyefl);
    }

    // swiatlo kierunkowe
    if (!gen_shadow_map)
    {
        int attr_light = getUniformLocation(prog, "light.dir");
        glUniform3fv(attr_light, 1, light.dir);
        attr_light = getUniformLocation(prog, "light.color");
        glUniform3fv(attr_light, 1, light.color);
    }

    // slider
    if (!gen_shadow_map)
    {
        int attr_s = getUniformLocation(prog, "slider");
        glUniform1i(attr_s, slider);
    }

    // material...
    if (!gen_shadow_map)
    {
        Material material(1.0,      1.0,      1.0,        0.3, // ambient
                          1.0,      1.0,      1.0,        1.0, // diffuse
                          1.0,      1.0,      1.0,        1.0, // specular
                          0.4*128.0,
                          true);

        int attr_material = getUniformLocation(prog, "material.ambient");
        glUniform3fv(attr_material, 1, material.ambient);
        attr_material = getUniformLocation(prog, "material.diffuse");
        glUniform3fv(attr_material, 1, material.diffuse);
        attr_material = getUniformLocation(prog, "material.specular");
        glUniform3fv(attr_material, 1, material.specular);
        attr_material = getUniformLocation(prog, "material.shininess");
        glUniform1f(attr_material, material.shininess);
        attr_material = getUniformLocation(prog, "material.useTex");
        glUniform1i(attr_material, material.useTexture);
    }

    for (int i = 0; i < 10; i++)
    {
        m_matrix.setToIdentity();
        m_matrix.rotate(36*i, 0, 1, 0);
        m_matrix.translate(0, 0, -2);
        m_matrix.scale(0.5);
        m_matrix.rotate(36*i, 1, 1, 0);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        if (!gen_shadow_map)
        {
            QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
            glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());
        }

        glDrawArrays(GL_TRIANGLES, 0, 3*triangles_cnt);
    }

    // podloga...
    glBindVertexArray(VAO_floor);

    m_matrix.setToIdentity();
    m_matrix.translate(0, -1, -2);
    m_matrix.scale(5.0, 1.0, 5.0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEX_floor);

    glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

    if (!gen_shadow_map)
    {
        QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
        glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());
    }

    glDrawArrays(GL_TRIANGLES, 0, 3*triangles_floor_cnt);

    // odczepiamy VAO
    glBindVertexArray(0);
}


void WidgetOpenGL::readTexture()
{
    qDebug() << "Zapisuje depth buffer...";
    QImage tex_d(SHADOW_FRAMEBUFFER_WIDTH, SHADOW_FRAMEBUFFER_HEIGHT, QImage::Format_Grayscale8);
    glBindTexture(GL_TEXTURE_2D, TEX_FBO_DEPTH);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, tex_d.bits());
    tex_d.mirrored(false, true).save("C:\\temp\\depthbuffer.png");

    qDebug() << "Ok";
}


void WidgetOpenGL::resizeGL(int w, int h)
{
    p_matrix.setToIdentity();
    float r = float(w)/float(h);

    // macierz perspektywy...
    p_matrix.perspective(45, r, 0.1, 100);
    p_matrix.translate(0, 0, -5);
}


// !!!
void WidgetOpenGL::makeShadowMatrix()
{
    l_matrix.setToIdentity();

    l_matrix.ortho(-5.0, 5.0, // x
                   -5.0, 5.0, // y
                   0.0, 18.0); // z

    l_matrix.lookAt(QVector3D(light.dir[0], light.dir[1], light.dir[2]).normalized()*5.0, // eye/camera
             QVector3D(0, 0, 0), //   center
             QVector3D(1, 1, 1)); //  up
}


void WidgetOpenGL::v_transform(float rot_x, float rot_y, float rot_z, float zoom)
{
    v_matrix.setToIdentity();
    v_matrix.rotate(rot_x, 1, 0, 0);
    v_matrix.rotate(rot_y, 0, 1, 0);
    v_matrix.rotate(rot_z, 0, 0, 1);
    v_matrix.scale(zoom);

    repaint();
}


void WidgetOpenGL::move_light(float x, float y, float z)
{
    light.setDir(x, y, z);
    repaint();
}


void WidgetOpenGL::move_slider(int _slider)
{
    slider = _slider;

    repaint();
}
