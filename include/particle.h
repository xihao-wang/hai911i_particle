#ifndef PARTICLE_H
#define PARTICLE_H
#include <QVector3D>
#include <cmath>
struct Particle {
    QVector3D pos;
    QVector3D speed;
    float age = 0;
    float ageMax = 100;
    float g = -1.f;
    QVector3D padding; // pour remplir la taille à n*16
    // float MAX_DT = 0.05f;


    void init() {
        pos = QVector3D(0.0, 0.0, 0.0);
        float angle = 2.0 * M_PI * rand() / RAND_MAX;
        float norm = 0.04 * rand() / RAND_MAX;
        speed = QVector3D(norm * cos(angle), norm * sin(angle),
                   rand() / static_cast<float>(RAND_MAX));
        age = 0.0f;
        // ageMax = 50.0f + (100.0f * rand() / float(RAND_MAX));
        ageMax = 5.0f + (5.0f * rand() / float(RAND_MAX));

    }

    void animate(float dt) {
        // speed[2] -= 0.05f;
        // pos += 0.1f * speed;
        // if (dt>MAX_DT )
        //     dt = MAX_DT;
        speed.setZ(speed.z() + g * dt);
        pos += speed * dt;

        if (pos[2] < 0.0) {
            speed[2] = -0.8 * speed[2];
            pos[2] = 0.0;
        }
        age += dt;
        if(age >= ageMax) init();
    }
};
// static_assert(sizeof(Particle) % 16 == 0, "Particle must be 16-byte aligned");

struct ParticleGPU {
    QVector4D pos;
    QVector4D speed;
    float age;
    float ageMax;
    float pad0;
    float pad1;
};

#endif // PARTICLE_H
