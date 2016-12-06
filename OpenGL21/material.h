#ifndef MATERIAL_H
#define MATERIAL_H

class Material
{
public:
    GLfloat ambient[3];
    GLfloat diffuse[3];
    GLfloat specular[3];
    GLfloat shininess;
    GLfloat useTexture; // !!!

    Material(GLfloat ambient_r, GLfloat ambient_g, GLfloat ambient_b, GLfloat ambient_f,
             GLfloat diffuse_r, GLfloat diffuse_g, GLfloat diffuse_b, GLfloat diffuse_f,
             GLfloat specular_r, GLfloat specular_g, GLfloat specular_b, GLfloat specular_f,
             GLfloat sh, GLfloat useTex)
    {
        ambient[0] = ambient_r*ambient_f;
        ambient[1] = ambient_g*ambient_f;
        ambient[2] = ambient_b*ambient_f;

        diffuse[0] = diffuse_r*diffuse_f;
        diffuse[1] = diffuse_g*diffuse_f;
        diffuse[2] = diffuse_b*diffuse_f;

        specular[0] = specular_r*specular_f;
        specular[1] = specular_g*specular_f;
        specular[2] = specular_b*specular_f;

        shininess = sh;
        useTexture = useTex; // !!!
    }
};


#endif // MATERIAL_H
