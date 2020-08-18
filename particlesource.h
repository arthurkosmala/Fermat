#ifndef PARTICLESOURCE_H
#define PARTICLESOURCE_H
#include <QVector>
#include "particle.h"
#include "potential.h"
#include <vector>

//For comments on this class, please refer to the .cpp file
class ParticleSource
{
public:
    ParticleSource(double, double, double, double, double, double, double);
    void propHistories(int, Potential, double, double, double[]);
    std::vector <Particle> myParticles;
};

#endif // PARTICLESOURCE_H
