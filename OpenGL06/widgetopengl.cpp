#include "widgetopengl.h"

#include <QDebug>
#include <QFile>


// !!!
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
        // CZ 2. Vertex Buffer Object + Element Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // trzy wierzolki trojkata (prawie rownoboczny)
        GLfloat vertData[] =
        {
            -0.5, -0.43, 0.0,  1.0, 0.0, 0.0,
             0.5, -0.43, 0.0,  0.0, 1.0, 0.0,
             0.0,  0.43, 0.0,  0.0, 0.0, 1.0
        };

        // !!!
        GLuint indices[] =
        {
             0, 1, 2
        };


        // tworzymy VBO i przesylamy dane do serwera OpenGL
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);

        // tworzymy EBO i przesylamy dane do serwera OpenGL
        // !!!
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // program ma teraz 2 parametry
        GLint attr = glGetAttribLocation(shaderProgram, "position");
        if (attr < 0) throw QString("Nieprawidlowy parametr 'position'");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        attr = glGetAttribLocation(shaderProgram, "color");
        if (attr < 0) throw QString("Nieprawidlowy parametr 'color'");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO i EBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  // !!!

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3. Inne inicjalizacje OpenGL
        ////////////////////////////////////////////////////////////////

        glClearColor(0, 0.3, 0, 1);

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
        // czyscimy ekran
        glClear(GL_COLOR_BUFFER_BIT);

        // rysujemy
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0); // !!!

        // odczepiamy VAO
        glBindVertexArray(0);
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w paintGL():" << msg;
    }
}
