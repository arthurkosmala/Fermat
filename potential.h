#ifndef POTENTIAL_H
#define POTENTIAL_H
#include <3rdparty/fparser4/fparser.hh>

/*
  This object parses the text input from the refractive index text field: it is derived from the FunctionParser object (defined in
  in the third party fparser4 library) and allows pointwise evaluation of the parsed function using the Eval() method. Its get_force
  method numerically differentiates the function at a given point.
*/

class Potential : public FunctionParser
{
public:
    Potential();
    double * get_force (double, double, double);
};

#endif // POTENTIAL_H
