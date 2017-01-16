#include "widgetopengl.h"
#include "model.h"

#include <QDebug>
#include <QFile>

#define MODEL         "dragon"
#define MODEL_TEXTURE "dragonskingold"


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
        GLuint vertexShader   = loadShader(GL_VERTEX_SHADER,   "gbuffer_vertex.glsl");
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "gbuffer_fragment.glsl");
        shaderProgram_gbuffer = linkProgram(vertexShader, fragmentShader);

        vertexShader   = loadShader(GL_VERTEX_SHADER,   "shading_vertex.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shading_fragment.glsl");
        shaderProgram_shading = linkProgram(vertexShader, fragmentShader);


        ////////////////////////////////////////////////////////////////
        // CZ 2. Wczytanie modelu
        ////////////////////////////////////////////////////////////////

        Model model;
        model.readFile("../Modele/" MODEL ".obj", true, true, 0.6);
        triangles_cnt = model.getVertDataCount();

        ////////////////////////////////////////////////////////////////
        // CZ 3. Wczytanie tekstury
        ////////////////////////////////////////////////////////////////

        tex_color = loadTexture2D("../Modele/" MODEL_TEXTURE ".jpg", false);


        ////////////////////////////////////////////////////////////////
        // CZ 4. Vertex Buffer Object + Vertex Array Object
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
        GLint attr = getAttribLocation(shaderProgram_gbuffer, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram_gbuffer, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // wspolrzedne tekstury
        // !!!
        attr = getAttribLocation(shaderProgram_gbuffer, "textureCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 2L. Wczytanie modelu swiatla (sfera)
        ////////////////////////////////////////////////////////////////

        Model model_light;
        model_light.readFile("../Modele/sphere.obj", true, true, 0.4);
        triangles_light_cnt = model_light.getVertDataCount();

        light.setPos(1.0, 3.0, 0.0); // !!!
        light.setColor(1.0, 1.0, 1.0, 2.0);


        ////////////////////////////////////////////////////////////////
        // CZ 3L. Wczytanie tekstury
        ////////////////////////////////////////////////////////////////

        tex_color_light = loadTexture2D("../Modele/white.jpg", false);


        ////////////////////////////////////////////////////////////////
        // CZ 4L. Vertex Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_light.getVertDataSize(), model_light.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_light);
        glBindVertexArray(VAO_light);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram_gbuffer, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_light.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram_gbuffer, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_light.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 5. Ekran
        ////////////////////////////////////////////////////////////////

        Model model_screen;
        model_screen.readFile("../Modele/screen.obj", false, false, 1.0);

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_screen.getVertDataSize(), model_screen.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_screen);
        glBindVertexArray(VAO_screen);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram_shading, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_screen.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);



        ////////////////////////////////////////////////////////////////
        // CZ 6. gBuffer
        ////////////////////////////////////////////////////////////////

        glGenFramebuffers(1, &FBO_gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_gBuffer);

        // positions...
        glGenTextures(1, &tex_FBO_gBuffer_position);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // normals...
        glGenTextures(1, &tex_FBO_gBuffer_normal);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // color...
        glGenTextures(1, &tex_FBO_gBuffer_color);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // depth...
        glGenTextures(1, &tex_FBO_gBuffer_depth);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_FBO_gBuffer_position, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_FBO_gBuffer_normal, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tex_FBO_gBuffer_color, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_FBO_gBuffer_depth, 0);

        GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);


        ////////////////////////////////////////////////////////////////
        // CZ 7. Inne inicjalizacje OpenGL
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

        // rendering do g-bufora
        glViewport(0, 0, width()*devicePixelRatio(), height()*devicePixelRatio());
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_gBuffer);
        paintScene();

        // rendering wlasciwy
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_screen);
        paintAndShade();
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w paintGL():" << msg;
    }
}


void WidgetOpenGL::paintScene()
{
    if (!init_ok) return;

    try
    {
        // czyscimy ekran i bufor glebokosci
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rysujemy
        glUseProgram(shaderProgram_gbuffer);
        glBindVertexArray(VAO);

        // udostepnienie tekstury
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_color);
        int attr_tex = getUniformLocation(shaderProgram_gbuffer, "textureBitmap");
        glUniform1i(attr_tex, 0);

        // ambient
        int attr_am = getUniformLocation(shaderProgram_gbuffer, "ambient");
        glUniform1f(attr_am, 0.1);

        // macierze
        int attr_pm = getUniformLocation(shaderProgram_gbuffer, "p_matrix");
        glUniformMatrix4fv(attr_pm, 1, GL_FALSE, p_matrix.data());

        int attr_vm = getUniformLocation(shaderProgram_gbuffer, "v_matrix");
        glUniformMatrix4fv(attr_vm, 1, GL_FALSE, v_matrix.data());

        int attr_mm = getUniformLocation(shaderProgram_gbuffer, "m_matrix");

        // macierz dla normalnych
        int attr_n = getUniformLocation(shaderProgram_gbuffer, "norm_matrix");

        for (int i = 0; i < 10; i++)
        {
            m_matrix.setToIdentity();
            m_matrix.rotate(36*i, 0, 1, 0);
            m_matrix.translate(0, 0, -2);
            m_matrix.scale(0.5);

            glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

            QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
            glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

            glDrawArrays(GL_TRIANGLES, 0, 3*triangles_cnt);
        }

        // odczepiamy VAO
        glBindVertexArray(0);

        // swiatlo
        glBindVertexArray(VAO_light);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_color_light);
        attr_tex = getUniformLocation(shaderProgram_gbuffer, "textureBitmap");
        glUniform1i(attr_tex, 0);

        // ambient
        attr_am = getUniformLocation(shaderProgram_gbuffer, "ambient");
        glUniform1f(attr_am, 1.0);

        m_matrix.setToIdentity();
        m_matrix.translate(light.pos[0], light.pos[1], light.pos[2]);
        m_matrix.scale(0.2);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
        glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

        glDrawArrays(GL_TRIANGLES, 0, 3*triangles_light_cnt);

        // odczepiamy VAO_light
        glBindVertexArray(0);
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w paintGL():" << msg;
    }
}


void WidgetOpenGL::paintAndShade()
{
    // czyscimy ekran i bufor glebokosci
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rysujemy
    glUseProgram(shaderProgram_shading);
    glBindVertexArray(VAO_screen);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_position);
    int t = getUniformLocation(shaderProgram_shading, "gPosition");
    glUniform1i(t, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_normal);
    t = getUniformLocation(shaderProgram_shading, "gNormal");
    glUniform1i(t, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_color);
    t = getUniformLocation(shaderProgram_shading, "gColor");
    glUniform1i(t, 2);

    int attr_eye = getUniformLocation(shaderProgram_shading, "eyePos");
    QVector4D eye = v_matrix.inverted()*QVector4D(0, 0, 5, 1);
    GLfloat eyefl[] = {eye.x(), eye.y(), eye.z()};
    glUniform3fv(attr_eye, 1, eyefl);

    int attr_light = getUniformLocation(shaderProgram_shading, "light.pos");
    glUniform3fv(attr_light, 1, light.pos);
    attr_light = getUniformLocation(shaderProgram_shading, "light.color");
    glUniform3fv(attr_light, 1, light.color);


    glDrawArrays(GL_TRIANGLES, 0, 6);

    // odczepiamy VAO_screen
    glBindVertexArray(0);
}


void WidgetOpenGL::resizeGL(int w, int h)
{
    p_matrix.setToIdentity();
    float r = float(w)/float(h);

    // macierz perspektywy...
    p_matrix.perspective(45, r, 0.1, 100);
    p_matrix.translate(0, 0, -5);

    // przeskalowanie framebufora...
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGB, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
    light.setPos(x, y, z);
    repaint();
}


void WidgetOpenGL::move_slider(int _slider)
{
    slider = _slider;

    repaint();
}
