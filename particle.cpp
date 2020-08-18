#include "particle.h"

/* This class is the physical and numerical core part of this project.
   -----------------
   Its variables are ...
   ... m : double, the particle mass, ...
   ... pos : double[2], its position vector (format: {x,y}), ...
   ... vel : double[2], its velocity vector (format: {v_x, v_y}), ...
   ... inBounds : bool, true if the particle trajectory has not yet left the displayed coordinate range, ...
   ... QVector <double> histx, histy : contain the saved x and y position values of previous RK4 iteration steps
       (used to draw the particle trajectories with the MainWindow::animateHistories() method)
   -----------------
   Its iter_rk4 function...
   ... updates the positions and velocities in a single fourth-order Runge-Kutta iteration step, ...
   ... applied to Newton's second low in the potential V: ddot_x = 1/m * grad V, ...
   ... with timestep dt and gradient discretization step dx).
*/

// upon construction, define the initial conditions for the RK4 integrator
Particle::Particle(double pos0 [2], double vel0 [2], double mass)
{
    pos[0] = pos0[0]; pos[1] = pos0[1];
    vel[0] = vel0[0]; vel[1] = vel0[1];
    m = mass;
    inBounds = true;
}

void Particle::iter_rk4(Potential V, double dt, double dx)
{
    // standard fourth-order Runge Kutta (with Newton's second law cast into a system of 2x2 first-order ODEs)
    double k11, k12, k21, k22, k13, k23, k14, k24;
    double& x = pos[0]; double& y = pos[1];
    double& vx = vel[0]; double& vy = vel[1];

    //first, update the x positions and velocities
    k11 = dt*vx;
    k21 = dt*( *(V.get_force(x,y,dx))/m );
    k12 = dt*(vx+0.5*k21);
    k22 = dt*( *(V.get_force(x+0.5*k11,y,dx))/m );
    k13 = dt*(vx+0.5*k22);
    k23 = dt*( *(V.get_force(x+0.5*k12,y,dx))/m );
    k14 = dt*(vx+k23);
    k24 = dt*( *(V.get_force(x+k13,y,dx))/m );
    x += (k11+2*k12+2*k13+k14)/6;
    vx += (k21+2*k22+2*k23+k24)/6;

    //next, update the y positions and velocities
    k11 = dt*vy;
    k21 = dt*( *(V.get_force(x,y,dx)+1)/m );
    k12 = dt*(vy+0.5*k21);
    k22 = dt*( *(V.get_force(x,y+0.5*k11,dx)+1)/m );
    k13 = dt*(vy+0.5*k22);
    k23 = dt*( *(V.get_force(x,y+0.5*k12,dx)+1)/m );
    k14 = dt*(vy+k23);
    k24 = dt*( *(V.get_force(x,y+k13,dx)+1)/m );
    y += (k11+2*k12+2*k13+k14)/6;
    vy += (k21+2*k22+2*k23+k24)/6;

    // append updated position values to the history
    histx.append(x); histy.append(y);
}

