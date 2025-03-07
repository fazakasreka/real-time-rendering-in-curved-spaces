#include "globalConstants.h"
#include <stdexcept>
#include <execinfo.h>
#include <cxxabi.h>
#include <iostream>

float curvature = SPH;
int windowWidth = 800;
int windowHeight = 600;

void setCurvature(float newCurvature) {
    curvature = newCurvature;
}

float getCurvature()
{   
    return curvature;
}