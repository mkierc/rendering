#include "widgetopengl.h"
#include "model.h"

#include <QDebug>
#include <QFile>

#define MODEL         "texcube" // texcube dragon kubek rcube
#define MODEL_FLOOR   "plane"
#define TEXTURE       "wood2"  // wood2 dragonskingold
#define TEXTURE_FLOOR "bricks" // stones2 bricks2


#define SHADOW_FRAMEBUFFER_WIDTH   1024
#define SHADOW_FRAMEBUFFER_HEIGHT  1024
#define SHADOW_RANGE               100.0


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
        GLuint geometryShader = loadShader(GL_GEOMETRY_SHADER, "geometry_shadow.glsl");
        shaderProgramShadow = linkProgram(vertexShader, fragmentShader, geometryShader);

        // !!!
        vertexShader   = loadShader(GL_VERTEX_SHADER,   "vertex_hdr.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment_hdr.glsl");
        shaderProgramHDR = linkProgram(vertexShader, fragmentShader);

        ////////////////////////////////////////////////////////////////
        // CZ 2. Wczytanie modeli
        ////////////////////////////////////////////////////////////////

        Model model;
        model.readFile("../Modele/" MODEL ".obj", true, true, 0.8);
        triangles_cnt = model.getVertDataCount();

        Model model_floor;
        model_floor.readFile("../Modele/" MODEL_FLOOR ".obj", true, true, 0.8);
        triangles_floor_cnt = model_floor.getVertDataCount();

        ////////////////////////////////////////////////////////////////
        // CZ 2T. Wczytanie tekstur
        ////////////////////////////////////////////////////////////////

        tex_color = loadTexture2D("../Modele/" TEXTURE ".jpg", true);
        tex_color_floor = loadTexture2D("../Modele/" TEXTURE_FLOOR ".jpg", true);


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
        // CZ 3F. Vertex Buffer Object + Vertex Array Object (screen)
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
        attr = getAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_screen.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3L. Oswietlenie punktowe
        ////////////////////////////////////////////////////////////////

        Model model_light;
        model_light.readFile("../Modele/sphere.obj", true, true, 0.4);
        triangles_light_cnt = model_light.getVertDataCount();

        light.setPos(1.0, 3.0, 0.0);
        light.setColor(1.0, 1.0, 1.0, 1.0);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_light.getVertDataSize(), model_light.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_light);
        glBindVertexArray(VAO_light);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_light.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_light.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 4. Framebuffer mapy cieni
        ////////////////////////////////////////////////////////////////

        glGenTextures(1, &tex_FBO_depth);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex_FBO_depth);
        for (int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_DEPTH_COMPONENT, SHADOW_FRAMEBUFFER_WIDTH, SHADOW_FRAMEBUFFER_HEIGHT,
                         0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_FBO_depth, 0); // !!!
        glDrawBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        ////////////////////////////////////////////////////////////////
        // CZ 4. Framebuffer HDR
        ////////////////////////////////////////////////////////////////

        // !!!
        glGenTextures(1, &tex_FBO_HDR_color);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_color);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);


        glGenTextures(1, &tex_FBO_HDR_depth);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glGenFramebuffers(1, &FBO_HDR);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_HDR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_FBO_HDR_color, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_FBO_HDR_depth, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        ////////////////////////////////////////////////////////////////
        // CZ 5. Inne inicjalizacje OpenGL
        ////////////////////////////////////////////////////////////////

        glClearColor(0, 0, 0, 1);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        v_transform(20, 0, 0, 1);
        gamma = 2.2;

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
        makeShadowMatrixes();
        paintScene(true);

        // rendering do HDR
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_HDR);
        glViewport(0, 0, width()*devicePixelRatio(), height()*devicePixelRatio());
        paintScene(false);

        // rendering wlasciwy
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_screen);
        paintHDR();
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w paintGL():" << msg;
    }
}


void WidgetOpenGL::paintHDR()
{
    // czyscimy ekran i bufor glebokosci
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rysujemy
    glUseProgram(shaderProgramHDR);
    glBindVertexArray(VAO_screen);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_color);
    int attr_tex = getUniformLocation(shaderProgramHDR, "textureHDR");
    glUniform1i(attr_tex, 0);

    int attr_g = getUniformLocation(shaderProgramHDR, "gamma");
    glUniform1f(attr_g, gamma);

    int attr_s = getUniformLocation(shaderProgramHDR, "slider");
    glUniform1i(attr_s, slider);


    glDrawArrays(GL_TRIANGLES, 0, 6);

    // odczepiamy VAO_screen
    glBindVertexArray(0);
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
        glBindTexture(GL_TEXTURE_2D, tex_color);
        int attr_tex = getUniformLocation(prog, "textureColor");
        glUniform1i(attr_tex, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex_FBO_depth);
        attr_tex = getUniformLocation(prog, "textureShadow");
        glUniform1i(attr_tex, 1);
    }

    // macierze
    if (gen_shadow_map)
        for (int i = 0; i < 6; i++)
        {
            int attr_lm = getUniformLocation(prog, QString("light_matrix[%1]").arg(i).toStdString().c_str());
            glUniformMatrix4fv(attr_lm, 1, GL_FALSE, l_matrix[i].data());
        }


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

    // swiatlo punktowe
    int attr_light = getUniformLocation(prog, "light.pos");
    glUniform3fv(attr_light, 1, light.pos);
    if (!gen_shadow_map)
    {
        attr_light = getUniformLocation(prog, "light.color");
        glUniform3fv(attr_light, 1, light.color);
    }

    int attr_far = getUniformLocation(prog, "far");
    glUniform1f(attr_far, SHADOW_RANGE);

    // material...
    if (!gen_shadow_map)
    {
        Material material(1.0,      1.0,      1.0,        0.1, // ambient
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
    if (!gen_shadow_map)
    {
        glBindVertexArray(VAO_floor);

        m_matrix.setToIdentity();
        m_matrix.translate(0, -1, -2);
        m_matrix.scale(5.0, 1.0, 5.0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_color_floor);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        GLuint attr_material = getUniformLocation(shaderProgram, "material.useTex");
        glUniform1i(attr_material, true);

        QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
        glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

        glDrawArrays(GL_TRIANGLES, 0, 3*triangles_floor_cnt);

        // odczepiamy VAO
        glBindVertexArray(0);
    }

    // swiatlo...
    if (!gen_shadow_map)
    {
        glBindVertexArray(VAO_light);

        m_matrix.setToIdentity();
        m_matrix.translate(light.pos[0], light.pos[1], light.pos[2]);
        m_matrix.scale(0.2);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
        glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

        GLuint attr_material = getUniformLocation(shaderProgram, "material.ambient");
        glUniform3fv(attr_material, 1, light.color);
        attr_material = getUniformLocation(shaderProgram, "material.useTex");
        glUniform1i(attr_material, false);

        glDrawArrays(GL_TRIANGLES, 0, 3*triangles_light_cnt);

        // odczepiamy VAO_light
        glBindVertexArray(0);
    }
}


void WidgetOpenGL::resizeGL(int w, int h)
{
    p_matrix.setToIdentity();
    float r = float(w)/float(h);

    // macierz perspektywy...
    p_matrix.perspective(45, r, 0.1, 100);
    p_matrix.translate(0, 0, -5);

    // zmiana wielkosci tekstur framebuffera
    // !!!
    glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}


// !!!
void WidgetOpenGL::makeShadowMatrixes()
{
    QVector3D lp = QVector3D(light.pos[0], light.pos[1], light.pos[2]);

    for (int i = 0; i < 6; i++)
    {
        l_matrix[i].setToIdentity();
        l_matrix[i].perspective(90.0, (float)SHADOW_FRAMEBUFFER_WIDTH / (float)SHADOW_FRAMEBUFFER_HEIGHT, 0, SHADOW_RANGE);
    }

    l_matrix[0].lookAt(lp, lp + QVector3D( 1, 0, 0), QVector3D(0, -1, 0));
    l_matrix[1].lookAt(lp, lp + QVector3D(-1, 0, 0), QVector3D(0, -1, 0));
    l_matrix[2].lookAt(lp, lp + QVector3D(0,  1, 0), QVector3D(0, 0, -1));
    l_matrix[3].lookAt(lp, lp + QVector3D(0, -1, 0), QVector3D(0, 0, -1));
    l_matrix[4].lookAt(lp, lp + QVector3D(0, 0 , 1), QVector3D(0, -1, 0));
    l_matrix[5].lookAt(lp, lp + QVector3D(0, 0, -1), QVector3D(0, -1, 0));
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


void WidgetOpenGL::set_gamma(float _gamma)
{
    gamma = _gamma;

    repaint();
}


void WidgetOpenGL::set_light_brightness(float b)
{
    light.setColor(1.0, 1.0, 1.0, b);

    repaint();
}
