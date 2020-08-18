#include "potential.h"

Potential::Potential()
{
    AddConstant("pi", 3.14159265358979323846);
    AddConstant("e", 2.71828182845904523536);
}

/* This function returns the potential gradient at a given point (x, y). It is calculated numerically by discretizing
   the gradient into a difference quotient with finite stepsize */

double * Potential::get_force(double x, double y, double step_size)
{
    static double f[2];
    double posxplus[2] = {x+step_size, y};
    double posxminus[2] = {x-step_size, y};
    double posyplus[2] = {x, y+step_size};
    double posyminus[2] = {x, y-step_size};

    f[0] = (Eval(posxminus) - Eval(posxplus))/(2*step_size);
    f[1] = (Eval(posyminus) - Eval(posyplus))/(2*step_size);
    return f;
}
