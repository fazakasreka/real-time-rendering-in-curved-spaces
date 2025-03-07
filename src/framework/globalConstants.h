#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#define SPH 1.0f
#define EUC 0.0f
#define HYP -1.0f

extern float curvature;
extern int windowWidth;
extern int windowHeight;

void setCurvature(float newCurvature);
float getCurvature();

#endif // GLOBAL_CONSTANTS_H