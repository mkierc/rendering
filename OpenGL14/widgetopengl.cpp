#include "widgetopengl.h"
#include "model.h"

#include <QDebug>
#include <QFile>


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

        // ladujemy shadery
        GLuint vertexShader   = loadShader(GL_VERTEX_SHADER,   "vertex.glsl");
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment.glsl");

        // tworzymy i linkujemy program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // czy kompilacja ok?
        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            throw QString("Blad linkowania: %1").arg(infoLog);
        }

        // shadery staly sie czescia programu, mozna skasowac
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);


        ////////////////////////////////////////////////////////////////
        // CZ 2. Wczytanie modelu
        ////////////////////////////////////////////////////////////////

        Model model;
        model.readFile("../Modele/rcube.obj", true, false, 0.4);
        triangles_cnt = model.getVertDataCount();


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
        GLint attr = glGetAttribLocation(shaderProgram, "position");
        if (attr < 0) throw QString("Nieprawidlowy parametr 'position'");

        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // normalne
        attr = glGetAttribLocation(shaderProgram, "normal");
        if (attr < 0) throw QString("Nieprawidlowy parametr 'normal'");

        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, model.getVertDataStride()*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 4. Inne inicjalizacje OpenGL
        ////////////////////////////////////////////////////////////////

        glClearColor(0, 0.3, 0, 1);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        v_transform(0, 0, 0, 1);

        init_ok = true;
    }
    catch (QString msg)
    {
        qDebug() << msg;
    }
}


QMatrix3x3 WidgetOpenGL::invertTranspose(QMatrix4x4 const &A)
{
    QMatrix3x3 R;
    float det =    +A(0,0)*(A(1,1)*A(2,2)-A(2,1)*A(1,2))
                   -A(0,1)*(A(1,0)*A(2,2)-A(1,2)*A(2,0))
                   +A(0,2)*(A(1,0)*A(2,1)-A(1,1)*A(2,0));

    float invdet = 1/det;
    R(0,0) =  (A(1,1)*A(2,2)-A(2,1)*A(1,2))*invdet;
    R(1,0) = -(A(0,1)*A(2,2)-A(0,2)*A(2,1))*invdet;
    R(2,0) =  (A(0,1)*A(1,2)-A(0,2)*A(1,1))*invdet;
    R(0,1) = -(A(1,0)*A(2,2)-A(1,2)*A(2,0))*invdet;
    R(1,1) =  (A(0,0)*A(2,2)-A(0,2)*A(2,0))*invdet;
    R(2,1) = -(A(0,0)*A(1,2)-A(1,0)*A(0,2))*invdet;
    R(0,2) =  (A(1,0)*A(2,1)-A(2,0)*A(1,1))*invdet;
    R(1,2) = -(A(0,0)*A(2,1)-A(2,0)*A(0,1))*invdet;
    R(2,2) =  (A(0,0)*A(1,1)-A(1,0)*A(0,1))*invdet;

    return R;
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

        // macierz
        int attr = glGetUniformLocation(shaderProgram, "pvm_matrix");
        if (attr < 0) throw QString("Nieprawidlowy parametr 'pvm_matrix'");

        // macierz dla normalnych
        // !!!
        int attr_n = glGetUniformLocation(shaderProgram, "norm_matrix");
        if (attr < 0) throw QString("Nieprawidlowy parametr 'norm_matrix'");

        for (int i = 0; i < 10; i++)
        {
            m_matrix.setToIdentity();
            m_matrix.rotate(36*i, 0, 1, 0);
            m_matrix.translate(0, 0, -2);
            m_matrix.scale(0.5);

            QMatrix4x4 pvm_matrix = p_matrix*v_matrix*m_matrix;
            glUniformMatrix4fv(attr, 1, GL_FALSE, pvm_matrix.data());

            QMatrix3x3 norm_matrix = invertTranspose(m_matrix); // !!!
            glUniformMatrix3fv(attr_n, 1, GL_FALSE, norm_matrix.data());

            glDrawArrays(GL_TRIANGLES, 0, 3*triangles_cnt);
        }

        // odczepiamy VAO
        glBindVertexArray(0);
    }
    catch (QString msg)
    {
        qDebug() << msg;
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
