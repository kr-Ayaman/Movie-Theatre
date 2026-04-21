#include "core/camera.h"

#include <cmath>

namespace {
const float kPi = 3.14159265f;
const float kDegToRad = kPi / 180.0f;

float clampf(float value, float minVal, float maxVal) {
    if (value < minVal) {
        return minVal;
    }
    if (value > maxVal) {
        return maxVal;
    }
    return value;
}

void normalizeVec3(float& x, float& y, float& z) {
    float length = std::sqrt(x * x + y * y + z * z);
    if (length < 1e-5f) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        return;
    }
    x /= length;
    y /= length;
    z /= length;
}

void crossVec3(
    float ax,
    float ay,
    float az,
    float bx,
    float by,
    float bz,
    float& rx,
    float& ry,
    float& rz
) {
    rx = ay * bz - az * by;
    ry = az * bx - ax * bz;
    rz = ax * by - ay * bx;
}

void computeForward(float yawDeg, float pitchDeg, float& x, float& y, float& z) {
    float yawRad = yawDeg * kDegToRad;
    float pitchRad = pitchDeg * kDegToRad;
    float cosPitch = std::cos(pitchRad);

    x = std::sin(yawRad) * cosPitch;
    y = std::sin(pitchRad);
    z = std::cos(yawRad) * cosPitch;
    normalizeVec3(x, y, z);
}

void computeBasis(
    float yawDeg,
    float pitchDeg,
    float& forwardX,
    float& forwardY,
    float& forwardZ,
    float& rightX,
    float& rightY,
    float& rightZ,
    float& upX,
    float& upY,
    float& upZ
) {
    computeForward(yawDeg, pitchDeg, forwardX, forwardY, forwardZ);

    const float worldUpX = 0.0f;
    const float worldUpY = 1.0f;
    const float worldUpZ = 0.0f;

    crossVec3(
        forwardX,
        forwardY,
        forwardZ,
        worldUpX,
        worldUpY,
        worldUpZ,
        rightX,
        rightY,
        rightZ
    );
    normalizeVec3(rightX, rightY, rightZ);
    if (rightX == 0.0f && rightY == 0.0f && rightZ == 0.0f) {
        rightX = 1.0f;
        rightY = 0.0f;
        rightZ = 0.0f;
    }

    crossVec3(
        rightX,
        rightY,
        rightZ,
        forwardX,
        forwardY,
        forwardZ,
        upX,
        upY,
        upZ
    );
    normalizeVec3(upX, upY, upZ);
}
}  // namespace

Camera::Camera() {
    reset();
}

void Camera::reset() {
    posX = 0.0f;
    posY = 11.9f;
    posZ = 18.0f;
    yaw = 180.0f;
    pitch = -10.0f;
    projectionMode = PROJECTION_PERSPECTIVE;
    orthoScale = 30.0f;  // Default orthographic view height
}

void Camera::clamp() {
    pitch = clampf(pitch, -85.0f, 85.0f);
}

void Camera::computeLookAt(
    float& eyeX,
    float& eyeY,
    float& eyeZ,
    float& centerX,
    float& centerY,
    float& centerZ,
    float& upX,
    float& upY,
    float& upZ
) const {
    float forwardX, forwardY, forwardZ;
    float rightX, rightY, rightZ;
    computeBasis(
        yaw,
        pitch,
        forwardX,
        forwardY,
        forwardZ,
        rightX,
        rightY,
        rightZ,
        upX,
        upY,
        upZ
    );

    eyeX = posX;
    eyeY = posY;
    eyeZ = posZ;
    centerX = posX + forwardX;
    centerY = posY + forwardY;
    centerZ = posZ + forwardZ;
}

void Camera::moveLocal(float forwardDelta, float rightDelta, float upDelta) {
    float forwardX, forwardY, forwardZ;
    float rightX, rightY, rightZ;
    float upX, upY, upZ;
    computeBasis(
        yaw,
        pitch,
        forwardX,
        forwardY,
        forwardZ,
        rightX,
        rightY,
        rightZ,
        upX,
        upY,
        upZ
    );

    posX += forwardX * forwardDelta + rightX * rightDelta + upX * upDelta;
    posY += forwardY * forwardDelta + rightY * rightDelta + upY * upDelta;
    posZ += forwardZ * forwardDelta + rightZ * rightDelta + upZ * upDelta;
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    yaw += yawDelta;
    pitch += pitchDelta;
    clamp();
}

void Camera::zoom(float amount) {
    moveLocal(amount, 0.0f, 0.0f);
}

void Camera::pan(int dx, int dy) {
    const float panScale = 0.02f;
    moveLocal(0.0f, -dx * panScale, dy * panScale);
}

void Camera::toggleProjectionMode() {
    if (projectionMode == PROJECTION_PERSPECTIVE) {
        projectionMode = PROJECTION_ORTHOGRAPHIC;
    } else {
        projectionMode = PROJECTION_PERSPECTIVE;
    }
}

void Camera::setProjectionMode(ProjectionMode mode) {
    projectionMode = mode;
}

void Camera::adjustOrthoScale(float delta) {
    orthoScale += delta;
    if (orthoScale < 1.0f) {
        orthoScale = 1.0f;
    }
    if (orthoScale > 100.0f) {
        orthoScale = 100.0f;
    }
}
