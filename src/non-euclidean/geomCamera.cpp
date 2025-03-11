#include "geomCamera.h"


GeomCamera::GeomCamera() {
    updateAspectRatio(1200, 800);
    fov = 90.0f * (float)M_PI / 180.0f;
    fp = 0.01f;
}

void GeomCamera::updateAspectRatio(int windowWidth, int windowHeight) {
    asp = (float)windowWidth / windowHeight;
}

vec4 GeomCamera::getPosition() {
    return eucPosition;
}

void GeomCamera::setPosition(vec4 position) {
    eucPosition = position;
}

void GeomCamera::pan(float deltaX, float deltaY){ //x and y are in the range of -1 to 1
    vec3 lookAt3 = vec3(lookAt.x, lookAt.y, lookAt.z);
    vec3 up3 = vec3(up.x, up.y, up.z);
    vec3 right = euclideanCross(lookAt3, up3);

    lookAt3 = euclideanNormalize(lookAt3 + deltaX * right + deltaY * up3);

    lookAt = vec4(lookAt3.x, lookAt3.y, lookAt3.z, 0);
}

void GeomCamera::move(float dt, Direction move_direction){

    vec4 direction = vec4(0, 0, 0, 0);
    if (move_direction == LEFT)
    {
        vec3 left = euclideanCross(vec3(up.x, up.y, up.z), vec3(lookAt.x, lookAt.y, lookAt.z));
        direction = vec4(left.x, left.y, left.z, 0);
    }
    else if (move_direction == RIGHT)
    {
        vec3 right = euclideanCross(vec3(lookAt.x, lookAt.y, lookAt.z), vec3(up.x, up.y, up.z));
        direction = vec4(right.x, right.y, right.z, 0);
    }
    else if (move_direction == UP)
        direction = vec4(0, 1, 0, 0);
    else if (move_direction == DOWN)
        direction = vec4(0, -1, 0, 0);
    else if (move_direction == FORWARD)
        direction = lookAt;
    else if (move_direction == BACKWARD)
        direction = -lookAt;

    eucPosition = eucPosition + direction * dt * 1.0f;

    if (Curvature::isSpherical())
    { // we walked around int the spherical world
        vec4 sphPosition = transformPointToCurrentSpace(eucPosition);
        if (sphPosition.w < 0)
        {
            eucPosition = -eucPosition;
            up = -up;
        }
    }
}

mat4 GeomCamera::V() { // view matrix: translates the center to the origin
    if (Curvature::isEuclidean()) {
        vec3 wVup = vec3(0, 1, 0);

        vec3 k_ = euclideanNormalize(vec3(-lookAt.x, -lookAt.y, -lookAt.z));
        vec3 i_ = euclideanNormalize(euclideanCross(wVup, k_));
        vec3 j_ = euclideanNormalize(euclideanCross(k_, i_));

        return TranslateMatrix(eucPosition * oppositeVector()) * mat4(i_.x, j_.x, k_.x, 0,
            i_.y, j_.y, k_.y, 0,
            i_.z, j_.z, k_.z, 0,
            0, 0, 0, 1);
    
    }
    else {
        vec4 geomPosition = transformPointToCurrentSpace(eucPosition);

        vec4 lookAtTransformed = transformVectorToCurrentSpace(lookAt, geomPosition);
        vec4 wVup = transformVectorToCurrentSpace(up, geomPosition);


        float alpha = Curvature::getCurvature();

        vec4 k_ = smartNormalize(-lookAtTransformed);
        vec4 i_ = smartNormalize(smartCross(geomPosition, wVup, k_)) * alpha;
        vec4 j_ = smartNormalize(smartCross(geomPosition, k_, i_)) * alpha;

        return mat4(i_.x, j_.x, k_.x, alpha * geomPosition.x,
            i_.y, j_.y, k_.y, alpha * geomPosition.y,
            i_.z, j_.z, k_.z, alpha * geomPosition.z,
            alpha * i_.w, alpha * j_.w, alpha * k_.w, geomPosition.w);
    }

}

mat4 GeomCamera::P() { // projection matrix: transforms the view frustum to the canonical view volume
    if (Curvature::isSpherical()) 
        bp = 3.14f; 
    else 
        bp = 10.f;

    float A, B;

    if (Curvature::isEuclidean()) {
        A = -(fp + bp) / (bp - fp);
        B = -2 *fp*bp / (bp - fp);
    }
    else {
        A = -smartSin(fp + bp) / smartSin(bp - fp);
        B = -2 * smartSin(fp)*smartSin(bp) / smartSin(bp - fp);
    }

    return mat4(1 / (tan(fov / 2)*asp), 0, 0, 0,
        0, 1 / tan(fov / 2), 0, 0,
        0, 0, A, -1,
        0, 0, B, 0);
    
}