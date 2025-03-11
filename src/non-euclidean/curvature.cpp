#include "curvature.h"
#include <stdexcept>
#include <execinfo.h>
#include <cxxabi.h>
#include <iostream>

float Curvature::curvature = EUC;

float Curvature::getCurvature()
{   
    return curvature;
}

bool Curvature::isHyperbolic() {
    return curvature < 0.0f;
}

bool Curvature::isSpherical() {
    return curvature > 0.0f;
}

bool Curvature::isEuclidean() {
    return curvature == 0.0f;
}

void Curvature::setHyperbolic() {
    curvature = -1.0f;
}

void Curvature::setSpherical() {
    curvature =  1.0f;
}

void Curvature::setEuclidean() {
    curvature =  0.0f;
}