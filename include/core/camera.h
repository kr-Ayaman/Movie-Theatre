#ifndef CAMERA_H
#define CAMERA_H

struct Camera {
    float posX;
    float posY;
    float posZ;
    float yaw;
    float pitch;

    Camera();
    void reset();
    void clamp();
    void computeLookAt(
        float& eyeX,
        float& eyeY,
        float& eyeZ,
        float& centerX,
        float& centerY,
        float& centerZ,
        float& upX,
        float& upY,
        float& upZ
    ) const;
    void moveLocal(float forwardDelta, float rightDelta, float upDelta);
    void rotate(float yawDelta, float pitchDelta);
    void zoom(float amount);
    void pan(int dx, int dy);
};

#endif
