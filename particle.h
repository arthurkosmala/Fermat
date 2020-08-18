#ifndef PARTICLE_H
#define PARTICLE_H
#include <QVector>
#include "potential.h"

//For comments on this class, please refer to the .cpp file
class Particle
{
public:
    double pos[2], vel[2];
    double m;
    bool inBounds;
    QVector <double> histx, histy;

    Particle(double[2], double[2], double);
    void iter_rk4(Potential, double, double);
};

#endif // PARTICLE_H
