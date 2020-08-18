#include "particlesource.h"
#include "math.h"
#include <vector>
#include <iostream>

/* Upon construction, a ParticleSource creates a number n of Particle objects and stores them in its MyParticles vector.
   These Particles are then associated with the ParticleSource and will (after starting the ray animation) ...
   ---------
   ... emanate out of the source position (x, y), with ...
   ... velocity vectors spreading evenly around a mean direction defined by the angle dir ...
   ... with the spread defined by the angular sector angsec (symmetric around dir), ...
   ... masses defined by m, ...
   ... and kinetic energies (hence, initial velocities) defined by E.
   ---------
*/

ParticleSource::ParticleSource(double x, double y, double dir, double angsec, double m, double E, double n){
    // associate n particles with the source
    for (int i=0; i<n; i++){
        // this formula ensures even spread around mean direction dir
        double ang = dir + angsec*(i/(n-1) - 0.5);
        // comes from E = 1/2*m*v^2
        double v = sqrt(2*E/m);
        double pos [2] = {x,y};
        double vel [2] = {v*cos(ang), v*sin(ang)};
        Particle p_i = Particle(pos, vel, m);
        myParticles.push_back(p_i);
    }
}

/* This function propagates the position histories of the particles in the potential V by a number (steps) of
   RK4 iterations (timestep dt, gradient discretization step dx), as long as they are within the coordinate bounds
   defined by the range array (format: {x_min, x_max, y_min, y_max})
*/

void ParticleSource::propHistories(int steps, Potential V, double dt, double dx, double range[4]){
    std::vector<Particle>::iterator it;

    for (int step = 0; step < steps; step++){

        std::vector<Particle>::iterator it;
        for(it = myParticles.begin(); it != myParticles.end(); ++it){
            if(it->inBounds == true){
                // update particle positions and velocities by a single RK4 step
                it->iter_rk4(V, dt, dx);
                if((it->pos[0] < range[0])||(it->pos[0] > range[1])||(it->pos[1] < range[2])||(it->pos[1] > range[3])){
                    it->inBounds = false;
                }
            }
        }
    }
}
