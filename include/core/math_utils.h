#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cmath>

inline void computeInverseViewMatrix(float eyeX, float eyeY, float eyeZ,
                                     float cX, float cY, float cZ,
                                     float upX, float upY, float upZ,
                                     float* outMat) {
    float fx = cX - eyeX;
    float fy = cY - eyeY;
    float fz = cZ - eyeZ;
    float flen = std::sqrt(fx*fx + fy*fy + fz*fz);
    fx /= flen; fy /= flen; fz /= flen;
    
    // Up isn't usually perfectly orthogonal to f
    float rx = fy * upZ - fz * upY;
    float ry = fz * upX - fx * upZ;
    float rz = fx * upY - fy * upX;
    float rlen = std::sqrt(rx*rx + ry*ry + rz*rz);
    rx /= rlen; ry /= rlen; rz /= rlen;
    
    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;
    
    outMat[0] = rx;   outMat[4] = ux;   outMat[8] = -fx;  outMat[12] = eyeX;
    outMat[1] = ry;   outMat[5] = uy;   outMat[9] = -fy;  outMat[13] = eyeY;
    outMat[2] = rz;   outMat[6] = uz;   outMat[10] = -fz; outMat[14] = eyeZ;
    outMat[3] = 0.0f; outMat[7] = 0.0f; outMat[11] = 0.0f; outMat[15] = 1.0f;
}

#endif
