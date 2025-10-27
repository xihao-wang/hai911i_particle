#include "systemwindow.h"
#include <QMatrix4x4>
#include <QOpenGLShader>
#include <QScreen>
#include <QtMath>
#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>
#include <QDateTime>
#include <QOpenGLTexture>

GLuint tex = 0; 
GLint  uSamplerLoc = -1;
qint64 m_lastTickMs = -1;

GLuint ssbo = 0;



void ParticleSystemWindow::initialize() {
    m_lastTickMs = -1;

    m_context = new QOpenGLContext(this);
    m_context->create();

    //Switch to OpenGL context
    m_context->makeCurrent(this);
    gl43 = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_3_Core>();
    if (!gl43) {
        qFatal("Impossible to initialize OpenGLFunction 4.3 Core.");
    }
    gl43->initializeOpenGLFunctions();
    srand(static_cast<unsigned int>(QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFF));
    /////////////////////////////////////////////////// Compute shader
    gl43->glGenVertexArrays(1, &vao);
    gl43->glBindVertexArray(vao);

    N = numParticles;
    gl43->glGenBuffers(1, &ssbo);
    gl43->glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    gl43->glBufferData(GL_SHADER_STORAGE_BUFFER, N * sizeof(ParticleGPU), nullptr, GL_DYNAMIC_DRAW);

    std::vector<ParticleGPU> initData(N);
    for (int i = 0; i < N; ++i) {
        float angle = 2.0 * M_PI * rand() / RAND_MAX;
        initData[i].pos = QVector4D(0.0, 0.0, 0.0, 0.0);
        float norm = 0.04 * rand() / RAND_MAX;
        initData[i].speed = QVector4D(norm * cos(angle), norm * sin(angle),
                   rand() / static_cast<float>(RAND_MAX),0.f);
        initData[i].age   = 0.0f;
        initData[i].ageMax = 5.0f + (5.0f * rand() / float(RAND_MAX));
        initData[i].pad0 = 0.0f;
        initData[i].pad1 = 0.0f;
    }

    gl43->glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, N * sizeof(ParticleGPU), initData.data());

    gl43->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    gl43->glBindBuffer(GL_ARRAY_BUFFER, ssbo);
    gl43->glEnableVertexAttribArray(0);
    gl43->glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(ParticleGPU),
        reinterpret_cast<void*>(0)
    );

    programCS = new QOpenGLShaderProgram(this);
    programCS->addShaderFromSourceFile(QOpenGLShader::Compute, "../shaders/particle.comp");
    programCS->link();
    programCS->bind();
    u_dt   = programCS->uniformLocation("dt");
    u_gz   = programCS->uniformLocation("gz");
    programCS->release();
    /////////////////////////////////////////////////// Compute shader





    particles.resize(numParticles);
    for (auto& p : particles) p.init();

    vbo.create();
    /*vbo.bind();
    vbo.allocate(numParticles * 3 * sizeof(GLfloat));
*/
    m_camera.setAspectRatio(width() / float(height()));

    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/particle.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Geometry, "../shaders/particle.geom");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/particle.frag");

    program->link();
    program->bind();

    matrixUniform = program->uniformLocation("mvp");
    uSamplerLoc = program->uniformLocation("texture2D");

    QImage img("../data/smoke.png");
    img = img.convertToFormat(QImage::Format_RGBA8888).mirrored(); // changer format de la texture pour ajuster le format d'openGl
    gl43->glGenTextures(1, &tex);
    gl43->glActiveTexture(GL_TEXTURE0);
    gl43->glBindTexture(GL_TEXTURE_2D, tex);

    gl43->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    gl43->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl43->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl43->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl43->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl43->glTexImage2D(
        GL_TEXTURE_2D,
        0,  
        GL_RGBA8, 
        img.width(),
        img.height(),
        0, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        img.constBits() 
    );
    program->setUniformValue(uSamplerLoc, 0);

    gl43->glBindTexture(GL_TEXTURE_2D, 0);
    program->release();


    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(5.0f);
}

void ParticleSystemWindow::render() {
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.f, 1.0f, 1.0f, 1.0f);

    qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    float dt = 0.0f;
    if (m_lastTickMs < 0) {
        dt = 0.0f; 
    } else {
        dt = float(nowMs - m_lastTickMs) / 1000.0f;
    }
    m_lastTickMs = nowMs;

    programCS->bind();
    programCS->setUniformValue(u_dt,   dt);
    programCS->setUniformValue(u_gz,  -1.f);
    programCS->setUniformValue("timeSeed", float(nowMs % 100000) * 0.001f);

    gl43->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    const GLuint LOCAL_SIZE = 256;
    GLuint groups = (N + LOCAL_SIZE - 1) / LOCAL_SIZE;
    gl43->glDispatchCompute(groups, 1, 1);

    gl43->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                        GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    programCS->release();


    // particlePositions.clear();
    // particlePositions.reserve(numParticles * 3);
    // for (auto& p : particles) {
    //     p.animate(dt);
    //     particlePositions.push_back(p.pos.x());
    //     particlePositions.push_back(p.pos.y());
    //     particlePositions.push_back(p.pos.z());
    // }

    // vbo.bind();
    // vbo.allocate(particlePositions.data(), particlePositions.size() * sizeof(GLfloat) * 3);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    // glEnableVertexAttribArray(0);

    program->bind();

    QMatrix4x4 V = m_camera.viewMatrix();
    QMatrix4x4 Vinv = V.inverted(); 

    QVector3D camRightWS = QVector3D(Vinv(0,0), Vinv(1,0), Vinv(2,0)).normalized(); 
    QVector3D camUpWS    = QVector3D(Vinv(0,1), Vinv(1,1), Vinv(2,1)).normalized(); 

    program->setUniformValue("camRight", camRightWS);
    program->setUniformValue("camUp",    camUpWS);

    
    QMatrix4x4 mvp = m_camera.projectionMatrix() * m_camera.viewMatrix();
    program->setUniformValue(matrixUniform, mvp);

    gl43->glBindVertexArray(vao);
    gl43->glActiveTexture(GL_TEXTURE0);
    gl43->glBindTexture(GL_TEXTURE_2D, tex);

    gl43->glEnable(GL_BLEND);
    gl43->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl43->glDepthMask(GL_FALSE);
    gl43->glDrawArrays(GL_POINTS, 0, N);
    gl43->glDepthMask(GL_TRUE);

    // glDrawArrays(GL_POINTS, 0, numParticles);

    program->release();
}


