#ifndef HYPERBOLIC_CAMERA_H
#define HYPERBOLIC_CAMERA_H

#include "framework.h"
#include "nonEuclideanMath.h"

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    NONE,
};

class GeomCamera { 
	vec4 eucPosition = vec4(0, 0.2, 2.0, 1.0f);
	vec4 velocity = vec4(0, 0, 0, 0);
	vec4 lookAt = vec4(0, 0, -1, 0);
	vec4 up = vec4(0, 1, 0, 0);
	float fov, asp, fp, bp;	  

public:
    GeomCamera();
    void updateAspectRatio(int windowWidth, int windowHeight);
    vec4 getPosition();
    void pan(float deltaX, float deltaY);
    void move(float dt, Direction move_direction);
    mat4 V();
    mat4 P();
};

#endif // HYPERBOLIC_CAMERA_H