#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#define SPH 1.0f
#define EUC 0.0f
#define HYP -1.0f

class Curvature {
    private:
        static float curvature;
        
    public:
        static void setCurvature(float newCurvature);
        static float getCurvature();
};

#endif // GLOBAL_CONSTANTS_H