#include "widgetopengl.h"

#include <QDebug>


// vertex shader
const GLchar *vertexShaderSource =
    "#version 450 \n"
    "in vec3 position;"
    "void main() "
    "{ "
    " gl_Position = vec4(position, 1.0); "
    "} ";

// fragment shader
const GLchar *fragmentShaderSource =
    "#version 450 \n"
    "out vec4 color; "
    "void main() "
    "{ "
    "  color = vec4(1.0, 1.0, 0.0, 1.0); "
    "} ";


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

        // tworzymy Vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        // czy kompilacja ok?
        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            throw QString("Blad vertex shadera: %1").arg(infoLog);
        }


        // tworzymy Fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        // czy kompilacja ok?
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            throw QString("Blad fragment shadera: %1").arg(infoLog);
        }


        // tworzymy i linkujemy program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // czy kompilacja ok?
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
        // CZ 2. Vertex Buffer Object + Vertex Array Object
        ////////////////////////////////////////////////////////////////

        // trzy wierzolki trojkata (prawie rownoboczny)
        GLfloat vertices[] =
        {
            -0.5, -0.43, 0.0,
             0.5, -0.43, 0.0,
             0.0,  0.43, 0.0
        };


        // tworzymy VBO i przesylamy dane do serwera OpenGL
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // tworzymy VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // program ma jeden parametr
        GLint attr = glGetAttribLocation(shaderProgram, "position");
        if (attr < 0) throw QString("Nieprawidlowy parametr 'position'");
        glVertexAttribPointer(attr, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(attr);

        // zapodajemy VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // odczepiamy VAO, aby sie nic juz nie zmienilo
        glBindVertexArray(0);


        ////////////////////////////////////////////////////////////////
        // CZ 3. Inne inicjalizacje OpenGL
        ////////////////////////////////////////////////////////////////

        glClearColor(0, 0.5, 0, 1);

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
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // odczepiamy VAO
        glBindVertexArray(0);
    }
    catch (QString msg)
    {
        qDebug() << "BLAD w paintGL():" << msg;
    }
}
