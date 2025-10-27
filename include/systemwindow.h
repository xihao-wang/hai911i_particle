#ifndef SYSTEMWINDOW_H
#define SYSTEMWINDOW_H

#include "openglwindow.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include "particle.h"

class ParticleSystemWindow : public OpenGLWindow {
public:
    using OpenGLWindow::OpenGLWindow;

    void initialize() override;
    void render() override;
    ~ParticleSystemWindow()
    {
        delete program;
    }


private:
    std::vector<Particle> particles;
    std::vector<GLfloat> particlePositions;

    QOpenGLBuffer vbo;
    GLuint vao = 0; 
    GLuint ssbo = 0; 
    int    N   = 0; 

    QOpenGLShaderProgram* programCS = nullptr;
    GLint u_dt   = -1;
    GLint u_gz   = -1;
    GLint u_rest = -1;

    QOpenGLShaderProgram* program = nullptr;

    GLint matrixUniform = -1;

    const int numParticles = 2000;

};


#endif // SYSTEMWINDOW_H
