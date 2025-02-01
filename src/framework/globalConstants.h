#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#define SPH 1
#define EUC 0
#define HYP -1

extern float curvature;
extern int windowWidth;
extern int windowHeight;

void changeCurvature(int newCurvature);

#endif // GLOBAL_CONSTANTS_H