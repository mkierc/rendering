#include "widgetopengl.h"
#include "model.h"

#include <QDebug>
#include <QFile>

#define MODEL "texcube" // texsphere texcube texrcube
#define TEXTURE "marble" // wood2 marble


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
        // Shadery
        ////////////////////////////////////////////////////////////////

        // ladujemy shadery, tworzymy program...
        GLuint vertexShader   = loadShader(GL_VERTEX_SHADER,   "inst_vertex.glsl");
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "inst_fragment.glsl");
        shaderProgram_inst = linkProgram(vertexShader, fragmentShader);

        vertexShader   = loadShader(GL_VERTEX_SHADER,   "light_vertex.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "light_fragment.glsl");
        shaderProgram_light = linkProgram(vertexShader, fragmentShader);

        ////////////////////////////////////////////////////////////////
        // Wczytanie modelu
        ////////////////////////////////////////////////////////////////

        Model model;
        model.readFile("../Modele/" MODEL ".obj", true, true, 0.2);
        triangles_cnt = model.getVertDataCount();

        ////////////////////////////////////////////////////////////////
        // Wczytanie tekstur
        ////////////////////////////////////////////////////////////////

        tex_cube_color = loadTexture2D("../Modele/" TEXTURE ".jpg", false);

        ////////////////////////////////////////////////////////////////
        // VBO dla instancji
        ////////////////////////////////////////////////////////////////

        int cnt = 0;
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++)
                for (int k = 0; k < 10; k++, cnt++)
                {
                    positions_and_colors[cnt].x = i - 4.5;
                    positions_and_colors[cnt].y = j - 4.5;
                    positions_and_colors[cnt].z = k - 4.5;

                    positions_and_colors[cnt].r = f_rand(0.2, 1.0);
                    positions_and_colors[cnt].g = f_rand(0.2, 1.0);
                    positions_and_colors[cnt].b = f_rand(0.2, 1.0);
                }

        GLuint VBO_positions_and_colors;
        glGenBuffers(1, &VBO_positions_and_colors);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_positions_and_colors);
        glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData)*1000, positions_and_colors, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        ////////////////////////////////////////////////////////////////
        // Vertex Buffer Object + Vertex Array Object
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
        GLint attr = getAttribLocation(shaderProgram_inst, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = getAttribLocation(shaderProgram_inst, "normal");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // wspolrzedne tekstury
        attr = getAttribLocation(shaderProgram_inst, "textureCoor");
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // polozenie dla instanced draw
        // !!!
        attr = getAttribLocation(shaderProgram_inst, "instancePosition");
        glBindBuffer(GL_ARRAY_BUFFER, VBO_positions_and_colors);
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
        glVertexAttribDivisor(attr, 1);
        glEnableVertexAttribArray(attr);

        attr = getAttribLocation(shaderProgram_inst, "instanceColor");
        glBindBuffer(GL_ARRAY_BUFFER, VBO_positions_and_colors);
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void *)(3*sizeof(GLfloat)));
        glVertexAttribDivisor(attr, 1);
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);




        ////////////////////////////////////////////////////////////////
        // Wczytanie modelu swiatla (sfera)
        ////////////////////////////////////////////////////////////////

        Model model_light;
        model_light.readFile("../Modele/sphere.obj", true, true, 0.4);
        triangles_light_cnt = model_light.getVertDataCount();

        light.setPos(1.0, 3.0, 0.0);


        ////////////////////////////////////////////////////////////////
        // Vertex Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // tworzymy VBO i przesylamy dane do serwera OpenGL
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, model_light.getVertDataSize(), model_light.getVertData(), GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO_light);
        glBindVertexArray(VAO_light);

        // wspolrzene wierzcholkow
        attr = getAttribLocation(shaderProgram_light, "position");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // Inne inicjalizacje OpenGL
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
        glUseProgram(shaderProgram_inst);
        glBindVertexArray(VAO);

        // udostepnienie tekstury
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_cube_color);
        int attr_tex = getUniformLocation(shaderProgram_inst, "textureBitmap");
        glUniform1i(attr_tex, 0);

        // macierze
        int attr_pm = getUniformLocation(shaderProgram_inst, "p_matrix");
        glUniformMatrix4fv(attr_pm, 1, GL_FALSE, p_matrix.data());

        int attr_vm = getUniformLocation(shaderProgram_inst, "v_matrix");
        glUniformMatrix4fv(attr_vm, 1, GL_FALSE, v_matrix.data());

        int attr_mm = getUniformLocation(shaderProgram_inst, "m_matrix");

        // obserwator
        int attr_eye = getUniformLocation(shaderProgram_inst, "eyePos");
        QVector4D eye = v_matrix.inverted()*QVector4D(0, 0, 5, 1);
        GLfloat eyefl[] = {eye.x(), eye.y(), eye.z()};
        glUniform3fv(attr_eye, 1, eyefl);

        // swiatlo
        int attr_light = getUniformLocation(shaderProgram_inst, "light.pos");
        glUniform3fv(attr_light, 1, light.pos);
        attr_light = getUniformLocation(shaderProgram_inst, "light.color");
        glUniform3fv(attr_light, 1, light.color);

        // slider
        int attr_s = getUniformLocation(shaderProgram_inst, "slider");
        glUniform1i(attr_s, slider);

        // material...
        Material material(0.2,      0.2,      0.2,        1.0, // ambient
                          1.0,      1.0,      1.0,        1.0, // diffuse
                          1.0,      1.0,      1.0,        1.0, // specular
                          0.4*128.0);

        int attr_material = getUniformLocation(shaderProgram_inst, "material.ambient");
        glUniform3fv(attr_material, 1, material.ambient);
        attr_material = getUniformLocation(shaderProgram_inst, "material.diffuse");
        glUniform3fv(attr_material, 1, material.diffuse);
        attr_material = getUniformLocation(shaderProgram_inst, "material.specular");
        glUniform3fv(attr_material, 1, material.specular);
        attr_material = getUniformLocation(shaderProgram_inst, "material.shininess");
        glUniform1f(attr_material, material.shininess);


        m_matrix.setToIdentity();
        m_matrix.rotate(30, 0, 1, 0);
        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        glDrawArraysInstanced(GL_TRIANGLES, 0, 3*triangles_cnt, 1000);

        // odczepiamy VAO
        glBindVertexArray(0);

        // swiatlo
        glUseProgram(shaderProgram_light);
        glBindVertexArray(VAO_light);

        m_matrix.setToIdentity();
        m_matrix.translate(light.pos[0], light.pos[1], light.pos[2]);
        m_matrix.scale(0.2);

        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        attr_pm = getUniformLocation(shaderProgram_light, "p_matrix");
        glUniformMatrix4fv(attr_pm, 1, GL_FALSE, p_matrix.data());

        attr_vm = getUniformLocation(shaderProgram_light, "v_matrix");
        glUniformMatrix4fv(attr_vm, 1, GL_FALSE, v_matrix.data());

        attr_mm = getUniformLocation(shaderProgram_light, "m_matrix");
        glUniformMatrix4fv(attr_mm, 1, GL_FALSE, m_matrix.data());

        attr_light = getUniformLocation(shaderProgram_light, "light.color");
        glUniform3fv(attr_light, 1, light.color);

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


float WidgetOpenGL::f_rand(float min, float max)
{
    return min + (max - min)*(float(qrand())/float(RAND_MAX));
}
