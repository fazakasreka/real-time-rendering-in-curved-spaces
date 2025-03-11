#include "curvature.h"
#include <stdexcept>
#include <execinfo.h>
#include <cxxabi.h>
#include <iostream>

float Curvature::curvature = EUC;

void Curvature::setCurvature(float newCurvature) {
    curvature = newCurvature;
}

float Curvature::getCurvature()
{   
    return curvature;
}
