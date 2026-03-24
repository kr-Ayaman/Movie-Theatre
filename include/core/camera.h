#ifndef CAMERA_H
#define CAMERA_H

struct Camera {
    float targetX;
    float targetY;
    float targetZ;
    float distance;
    float yaw;
    float pitch;

    Camera();
    void reset();
    void clamp();
    void computePosition(float& x, float& y, float& z) const;
    void pan(int dx, int dy);
};

#endif
