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
}  // namespace

Camera::Camera() {
    reset();
}

void Camera::reset() {
    targetX = 0.0f;
    targetY = 6.0f;
    targetZ = -7.0f;
    distance = 34.0f;
    yaw = 0.0f;
    pitch = 10.0f;
}

void Camera::clamp() {
    distance = clampf(distance, 10.0f, 80.0f);
    pitch = clampf(pitch, -8.0f, 80.0f);
    targetY = clampf(targetY, 1.0f, 18.0f);
}

void Camera::computePosition(float& x, float& y, float& z) const {
    float yawRad = yaw * kDegToRad;
    float pitchRad = pitch * kDegToRad;
    float cosPitch = std::cos(pitchRad);

    x = targetX + distance * cosPitch * std::sin(yawRad);
    y = targetY + distance * std::sin(pitchRad);
    z = targetZ + distance * cosPitch * std::cos(yawRad);
}

void Camera::pan(int dx, int dy) {
    float camX, camY, camZ;
    computePosition(camX, camY, camZ);

    float forwardX = targetX - camX;
    float forwardY = targetY - camY;
    float forwardZ = targetZ - camZ;
    normalizeVec3(forwardX, forwardY, forwardZ);

    float rightX = -forwardZ;
    float rightY = 0.0f;
    float rightZ = forwardX;
    normalizeVec3(rightX, rightY, rightZ);

    float upX = rightY * forwardZ - rightZ * forwardY;
    float upY = rightZ * forwardX - rightX * forwardZ;
    float upZ = rightX * forwardY - rightY * forwardX;
    normalizeVec3(upX, upY, upZ);

    float panScale = distance * 0.0045f;
    targetX += (-dx * panScale) * rightX + (dy * panScale) * upX;
    targetY += (-dx * panScale) * rightY + (dy * panScale) * upY;
    targetZ += (-dx * panScale) * rightZ + (dy * panScale) * upZ;
}
