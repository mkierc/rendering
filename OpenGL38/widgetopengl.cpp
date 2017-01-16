#include "widgetopengl.h"
#include "model.h"
#include "mainwindow.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QThread>

#define MODEL         "dragon" // texcube dragon kubek rcube
#define MODEL_SHADOW  "dragon_s"
#define MODEL_FLOOR   "plane"
#define TEXTURE       "dragonskingold"   // wood2 dragonskingold wood2x
#define TEXTURE_FLOOR "bricks"  // stones2 stones3 bricks
#define TEXTURE_NORMAL_FLOOR "bricks_normal"
#define GAUSS_CNT     5


#define SHADOW_FRAMEBUFFER_WIDTH   1014
#define SHADOW_FRAMEBUFFER_HEIGHT  1014
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
        GLuint vertexShader   = loadShader(GL_VERTEX_SHADER,   "shading_vertex.glsl");
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shading_fragment.glsl");
        shaderProgram_Shading = linkProgram(vertexShader, fragmentShader);

        vertexShader   = loadShader(GL_VERTEX_SHADER,   "shadowmap_vertex.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shadowmap_fragment.glsl");
        GLuint geometryShader = loadShader(GL_GEOMETRY_SHADER, "shadowmap_geometry.glsl");
        shaderProgram_ShadowMap = linkProgram(vertexShader, fragmentShader, geometryShader);

        vertexShader   = loadShader(GL_VERTEX_SHADER,   "hdr_vertex.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "hdr_fragment.glsl");
        shaderProgram_HDR = linkProgram(vertexShader, fragmentShader);

        vertexShader   = loadShader(GL_VERTEX_SHADER,   "filter_vertex.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "filter_fragment.glsl");
        shaderProgram_Filter = linkProgram(vertexShader, fragmentShader);

        vertexShader   = loadShader(GL_VERTEX_SHADER,   "gbuffer_vertex.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "gbuffer_fragment.glsl");
        shaderProgram_gBuffer = linkProgram(vertexShader, fragmentShader);

        ////////////////////////////////////////////////////////////////
        // CZ 2. Wczytanie modeli
        ////////////////////////////////////////////////////////////////

        Model model;
        model.readFile("../Modele/" MODEL ".obj", true, true, true, 0.6);
        triangles_cnt = model.getVertDataCount();

        Model model_shadow;
        model_shadow.readFile("../Modele/" MODEL_SHADOW ".obj", true, true, true, 0.6);
        triangles_shadow_cnt = model_shadow.getVertDataCount();

        Model model_floor;
        model_floor.readFile("../Modele/" MODEL_FLOOR ".obj", true, true, true, 1.0);
        triangles_floor_cnt = model_floor.getVertDataCount();

        Model model_light;
        model_light.readFile("../Modele/sphere.obj", true, true, true, 0.4);
        triangles_light_cnt = model_light.getVertDataCount();

        Model model_screen;
        model_screen.readFile("../Modele/screen.obj", false, false, false, 1.0);


        ////////////////////////////////////////////////////////////////
        // CZ 2T. Wczytanie tekstur
        ////////////////////////////////////////////////////////////////

        tex_color = loadTexture2D("../Modele/" TEXTURE ".jpg", true);
        tex_normal = loadTexture2D("../Modele/normal.jpg", false);
        tex_color_floor = loadTexture2D("../Modele/" TEXTURE_FLOOR ".jpg", true);
        tex_normal_floor = loadTexture2D("../Modele/" TEXTURE_NORMAL_FLOOR ".jpg", false);
        tex_color_light = loadTexture2D("../Modele/white.jpg", true);


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
        GLint attr = getAttribLocation(shaderProgram_gBuffer, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram_gBuffer, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // wspolrzedne tekstury
        attr = getAttribLocation(shaderProgram_gBuffer, "textureCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // tangent U
        attr = getAttribLocation(shaderProgram_gBuffer, "tgnU");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(8*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // tangent V
        attr = getAttribLocation(shaderProgram_gBuffer, "tgnV");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(11*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3C. Vertex Buffer Object + Vertex Array Object (shadow)
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_shadow.getVertDataSize(), model_shadow.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_shadow);
        glBindVertexArray(VAO_shadow);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram_gBuffer, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_shadow.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram_gBuffer, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_shadow.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // wspolrzedne tekstury
        attr = getAttribLocation(shaderProgram_gBuffer, "textureCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, model_shadow.getVertDataStride()*sizeof(GLfloat), (void *)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // tangent U
        attr = getAttribLocation(shaderProgram_gBuffer, "tgnU");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_shadow.getVertDataStride()*sizeof(GLfloat), (void *)(8*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // tangent V
        attr = getAttribLocation(shaderProgram_gBuffer, "tgnV");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_shadow.getVertDataStride()*sizeof(GLfloat), (void *)(11*sizeof(GLfloat)));
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
        attr = getAttribLocation(shaderProgram_gBuffer, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_floor.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram_gBuffer, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_floor.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // wspolrzedne tekstury
        attr = getAttribLocation(shaderProgram_gBuffer, "textureCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, model_floor.getVertDataStride()*sizeof(GLfloat), (void *)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // tangent U
        attr = getAttribLocation(shaderProgram_gBuffer, "tgnU");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(8*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // tangent V
        attr = getAttribLocation(shaderProgram_gBuffer, "tgnV");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(11*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3F. Vertex Buffer Object + Vertex Array Object (screen)
        ////////////////////////////////////////////////////////////////


        // tworzymy VBO i przesylamy dane do serwera OpenGL
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_screen.getVertDataSize(), model_screen.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_screen);
        glBindVertexArray(VAO_screen);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram_HDR, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_screen.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3L. Oswietlenie punktowe
        ////////////////////////////////////////////////////////////////

        light.setPos(1.0, 3.0, 0.0);
        light.setColor(1.0, 1.0, 1.0, 1.0);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_light.getVertDataSize(), model_light.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_light);
        glBindVertexArray(VAO_light);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram_gBuffer, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model_light.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram_gBuffer, "normal");
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
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_FBO_depth, 0);
        glDrawBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        ////////////////////////////////////////////////////////////////
        // CZ 4. Framebuffer HDR
        ////////////////////////////////////////////////////////////////

        glGenTextures(2, tex_FBO_HDR_color);
        for (int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_color[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
        }

        glGenTextures(1, &tex_FBO_HDR_depth);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glGenFramebuffers(1, &FBO_HDR);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_HDR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_FBO_HDR_color[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_FBO_HDR_color[1], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_FBO_HDR_depth, 0);

        GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        ////////////////////////////////////////////////////////////////
        // CZ 4F. Framebuffer HDR filter
        ////////////////////////////////////////////////////////////////

        glGenFramebuffers(2, FBO_filter);
        glGenTextures(2, tex_FBO_filter);
        for (int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, tex_FBO_filter[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);

            glBindFramebuffer(GL_FRAMEBUFFER, FBO_filter[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_FBO_filter[i], 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ////////////////////////////////////////////////////////////////
        // CZ 4G. gBuffer
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // color...
        glGenTextures(1, &tex_FBO_gBuffer_color);
        glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width()*devicePixelRatio(), height()*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
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

        GLuint attachmentsG[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachmentsG);

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

        // rendering do g-bufora
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_gBuffer);
        glViewport(0, 0, width()*devicePixelRatio(), height()*devicePixelRatio());
        paintScene(false);

        // shading do HDR
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_HDR);
        glViewport(0, 0, width()*devicePixelRatio(), height()*devicePixelRatio());
        paintShading();

        // gauss
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_filter[0]);
        paintFilter(tex_FBO_HDR_color[1], true);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO_filter[1]);
        paintFilter(tex_FBO_filter[0], false);

        for (int i = 1; i < GAUSS_CNT; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, FBO_filter[0]);
            paintFilter(tex_FBO_filter[1], true);
            glBindFramebuffer(GL_FRAMEBUFFER, FBO_filter[1]);
            paintFilter(tex_FBO_filter[0], false);
        }

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
    glUseProgram(shaderProgram_HDR);
    glBindVertexArray(VAO_screen);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_color[0]);
    int attr_tex = getUniformLocation(shaderProgram_HDR, "textureHDR");
    glUniform1i(attr_tex, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_filter[1]);
    attr_tex = getUniformLocation(shaderProgram_HDR, "textureHDRBright");
    glUniform1i(attr_tex, 1);

    int attr_g = getUniformLocation(shaderProgram_HDR, "gamma");
    glUniform1f(attr_g, gamma);

    int attr_s = getUniformLocation(shaderProgram_HDR, "slider");
    glUniform1i(attr_s, slider);


    glDrawArrays(GL_TRIANGLES, 0, 6);

    // odczepiamy VAO_screen
    glBindVertexArray(0);
}


void WidgetOpenGL::paintShading()
{
    // czyscimy ekran i bufor glebokosci
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rysujemy
    glUseProgram(shaderProgram_Shading);
    glBindVertexArray(VAO_screen);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_position);
    int t = getUniformLocation(shaderProgram_Shading, "gPosition");
    glUniform1i(t, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_normal);
    t = getUniformLocation(shaderProgram_Shading, "gNormal");
    glUniform1i(t, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_color);
    t = getUniformLocation(shaderProgram_Shading, "gColor");
    glUniform1i(t, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_FBO_depth);
    t = getUniformLocation(shaderProgram_Shading, "textureShadow");
    glUniform1i(t, 3);

    int attr_far = getUniformLocation(shaderProgram_Shading, "far");
    glUniform1f(attr_far, SHADOW_RANGE);

    int attr_eye = getUniformLocation(shaderProgram_Shading, "eyePos");
    QVector4D eye = v_matrix.inverted()*QVector4D(0, 0, 5, 1);
    GLfloat eyefl[] = {eye.x(), eye.y(), eye.z()};
    glUniform3fv(attr_eye, 1, eyefl);

    int attr_light = getUniformLocation(shaderProgram_Shading, "light.pos");
    glUniform3fv(attr_light, 1, light.pos);
    attr_light = getUniformLocation(shaderProgram_Shading, "light.color");
    glUniform3fv(attr_light, 1, light.color);


    glDrawArrays(GL_TRIANGLES, 0, 6);

    // odczepiamy VAO_screen
    glBindVertexArray(0);
}


void WidgetOpenGL::paintFilter(GLuint textureIn, boolean horizontal)
{
    // czyscimy ekran i bufor glebokosci
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rysujemy
    glUseProgram(shaderProgram_Filter);
    glBindVertexArray(VAO_screen);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureIn);
    int attr_tex = getUniformLocation(shaderProgram_Filter, "textureIn");
    glUniform1i(attr_tex, 0);

    int attr_horizontal = getUniformLocation(shaderProgram_Filter, "horizontal");
    glUniform1f(attr_horizontal, horizontal ? 1.0 : 0.0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // odczepiamy VAO_screen
    glBindVertexArray(0);
}


void WidgetOpenGL::paintScene(boolean gen_shadow_map)
{
    GLuint prog = gen_shadow_map ? shaderProgram_ShadowMap : shaderProgram_gBuffer;

    // czyscimy ekran i bufor glebokosci
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rysujemy
    glUseProgram(prog);
    glBindVertexArray(gen_shadow_map ? VAO_shadow : VAO);

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

    // swiatlo punktowe
    if (gen_shadow_map)
    {
        int attr_light = getUniformLocation(prog, "light.pos");
        glUniform3fv(attr_light, 1, light.pos);

        int attr_far = getUniformLocation(prog, "far");
        glUniform1f(attr_far, SHADOW_RANGE);
    }

    // material...
    if (!gen_shadow_map)
    {
        int attr_ambient = getUniformLocation(prog, "ambient");
        glUniform1f(attr_ambient, 0.02);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_color);
        glUniform1i(getUniformLocation(prog, "textureBitmap"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_normal);
        glUniform1i(getUniformLocation(prog, "textureNormal"), 1);
    }

    for (int i = 0; i < 10; i++)
    {
        m_matrix.setToIdentity();
        m_matrix.rotate(36*i, 0, 1, 0);
        m_matrix.translate(0, 0, -2);
        m_matrix.scale(0.5);
//        m_matrix.rotate(36*i, 1, 1, 0);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        if (!gen_shadow_map)
        {
            QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
            glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());
        }

        glDrawArrays(GL_TRIANGLES, 0, 3*(gen_shadow_map ? triangles_shadow_cnt : triangles_cnt));
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
        glUniform1i(getUniformLocation(prog, "textureBitmap"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_normal_floor);
        glUniform1i(getUniformLocation(prog, "textureNormal"), 1);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_color_light);
        glUniform1i(getUniformLocation(prog, "textureBitmap"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_normal);
        glUniform1i(getUniformLocation(prog, "textureNormal"), 1);


        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
        glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

        int attr_ambient = getUniformLocation(prog, "ambient");
        glUniform1f(attr_ambient, light.color[0]);

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

    // zmiana wielkosci tekstur framebufferow
    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_color[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
    }

    glBindTexture(GL_TEXTURE_2D, tex_FBO_HDR_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, tex_FBO_filter[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
    }

    // przeskalowanie framebufora...
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, tex_FBO_gBuffer_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w*devicePixelRatio(), h*devicePixelRatio(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}



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
