#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#define SPH 1.0f
#define EUC 0.0f
#define HYP -1.0f

class Curvature {
    private:
        static float curvature;
        
    public:
        static float getCurvature();
        static bool isHyperbolic();
        static bool isSpherical();
        static bool isEuclidean();
        static void setHyperbolic();
        static void setSpherical();
        static void setEuclidean();
};

#endif // GLOBAL_CONSTANTS_H