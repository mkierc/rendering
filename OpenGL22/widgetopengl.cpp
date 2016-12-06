#include "widgetopengl.h"
#include "model.h"

#include <QDebug>
#include <QFile>


#define MODEL   "texcube" // texcube plane
#define TEXTURE "wood" // wood bricks bricks2 stones stones2 stones3 relief


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


        ////////////////////////////////////////////////////////////////
        // CZ 2. Wczytanie modelu
        ////////////////////////////////////////////////////////////////

        Model model;
        model.readFile("../Modele/" MODEL ".obj", true, true, true, 0.8);
        triangles_cnt = model.getVertDataCount();

        ////////////////////////////////////////////////////////////////
        // CZ 2T. Wczytanie tekstury
        ////////////////////////////////////////////////////////////////

        tex_color  = loadTexture2D("../Modele/" TEXTURE ".jpg");
        tex_normal = loadTexture2D("../Modele/" TEXTURE "_normal.jpg");
        tex_depth  = loadTexture2D("../Modele/" TEXTURE "_depth.jpg");


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

        // tangent U
        attr = getAttribLocation(shaderProgram, "tgtU");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(8*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // tangent V
        attr = getAttribLocation(shaderProgram, "tgtV");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(11*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);




        ////////////////////////////////////////////////////////////////
        // CZ 2L. Wczytanie modelu swiatla (sfera)
        ////////////////////////////////////////////////////////////////

        Model model_light;
        model_light.readFile("../Modele/sphere.obj", true, true, true, 0.4);
        triangles_light_cnt = model_light.getVertDataCount();

        light.setPos(1.0, 3.0, 0.0);


        ////////////////////////////////////////////////////////////////
        // CZ 3L. Vertex Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_light.getVertDataSize(), model_light.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_light);
        glBindVertexArray(VAO_light);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);




        ////////////////////////////////////////////////////////////////
        // CZ 4. Inne inicjalizacje OpenGL
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
        // czyscimy ekran i bufor glebokosci
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rysujemy
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // udostepnienie tekstur
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_color);
        glUniform1i(getUniformLocation(shaderProgram, "textureColor"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_normal);
        glUniform1i(getUniformLocation(shaderProgram, "textureNormal"), 1);

        // !!!
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_depth);
        glUniform1i(getUniformLocation(shaderProgram, "textureDepth"), 2);

        // macierze
        int attr_pm = getUniformLocation(shaderProgram, "p_matrix");
        glUniformMatrix4fv(attr_pm, 1, GL_FALSE, p_matrix.data());

        int attr_vm = getUniformLocation(shaderProgram, "v_matrix");
        glUniformMatrix4fv(attr_vm, 1, GL_FALSE, v_matrix.data());

        int attr_mm = getUniformLocation(shaderProgram, "m_matrix");

        // macierz dla normalnych
        int attr_n = getUniformLocation(shaderProgram, "norm_matrix");

        // obserwator
        int attr_eye = getUniformLocation(shaderProgram, "eyePos");
        QVector4D eye = v_matrix.inverted()*QVector4D(0, 0, 5, 1);
        GLfloat eyefl[] = {eye.x(), eye.y(), eye.z()};
        glUniform3fv(attr_eye, 1, eyefl);

        // swiatlo
        int attr_light = getUniformLocation(shaderProgram, "light.pos");
        glUniform3fv(attr_light, 1, light.pos);
        attr_light = getUniformLocation(shaderProgram, "light.color");
        glUniform3fv(attr_light, 1, light.color);

        // slider
        int attr_s = getUniformLocation(shaderProgram, "slider");
        glUniform1i(attr_s, slider);

        // material...
        Material material(0.2,      0.2,      0.2,        1.0, // ambient
                          1.0,      1.0,      1.0,        1.0, // diffuse
                          1.0,      1.0,      1.0,        1.0, // specular
                          0.4*128.0,
                          1.0);

        int attr_material = getUniformLocation(shaderProgram, "material.ambient");
        glUniform3fv(attr_material, 1, material.ambient);
        attr_material = getUniformLocation(shaderProgram, "material.diffuse");
        glUniform3fv(attr_material, 1, material.diffuse);
        attr_material = getUniformLocation(shaderProgram, "material.specular");
        glUniform3fv(attr_material, 1, material.specular);
        attr_material = getUniformLocation(shaderProgram, "material.shininess");
        glUniform1f(attr_material, material.shininess);
        attr_material = getUniformLocation(shaderProgram, "material.useTex");
        glUniform1f(attr_material, material.useTexture);


        for (int i = 0; i < 10; i++)
        {
            m_matrix.setToIdentity();
            m_matrix.rotate(36*i, 0, 1, 0);
            m_matrix.translate(0, 0, -2);
            m_matrix.scale(0.5);

//            m_matrix.rotate(36*i, 1, 1, 0);
//            m_matrix.rotate(90*i, 0, 0, 1);

            glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

            QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
            glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

            glDrawArrays(GL_TRIANGLES, 0, 3*triangles_cnt);
        }

        // odczepiamy VAO
        glBindVertexArray(0);

        // swiatlo
        glBindVertexArray(VAO_light);

        m_matrix.setToIdentity();
        m_matrix.translate(light.pos[0], light.pos[1], light.pos[2]);
        m_matrix.scale(0.2);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        QMatrix3x3 norm_matrix = upperLeftMatrix3x3(m_matrix.inverted().transposed());
        glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

        attr_material = getUniformLocation(shaderProgram, "material.ambient");
        glUniform3fv(attr_material, 1, light.color);
        attr_material = getUniformLocation(shaderProgram, "material.useTex");
        glUniform1f(attr_material, 0.0);

        glDrawArrays(GL_TRIANGLES, 0, 3*triangles_light_cnt);

        // odczepiamy VAO_light
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
    p_matrix.perspective(45, r, 0.1, 100);
    p_matrix.translate(0, 0, -5);
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
