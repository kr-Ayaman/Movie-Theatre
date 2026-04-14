#include <cmath>
#include <iostream>

int main() {
    auto sign = [](float val) { return (val > 0.0f) ? 1.0f : ((val < 0.0f) ? -1.0f : 0.0f); };
    auto spow = [sign](float val, float p) { return sign(val) * std::pow(std::fabs(val), p); };

    float hw = 0.86f * 0.5f;
    float hh = 0.28f * 0.5f;
    float hd = 0.82f * 0.5f;
    float n = 0.25f;
    int res = 2; // small test

    for (int i = 0; i < res; ++i) {
        float u = -M_PI / 2.0f + (float)i / res * M_PI;
        for (int j = 0; j <= res * 2; ++j) {
            float v = -M_PI + (float)j / (res * 2) * 2.0f * M_PI;
            float cu = std::cos(u); float su = std::sin(u);
            float cv = std::cos(v); float sv = std::sin(v);
            
            float nx = spow(cu, 2.0f - n) * spow(cv, 2.0f - n) / hw;
            float ny = spow(su, 2.0f - n) / hh;
            float nz = spow(cu, 2.0f - n) * spow(sv, 2.0f - n) / hd;
            
            float len = std::sqrt(nx*nx + ny*ny + nz*nz);
            float flnx = nx/len; float flny = ny/len; float flnz = nz/len;
            
            float px = hw * spow(cu, n) * spow(cv, n);
            float py = hh * spow(su, n);
            float pz = hd * spow(cu, n) * spow(sv, n);
            std::cout << "N: " << flnx << " " << flny << " " << flnz << " | P: " << px << " " << py << " " << pz << std::endl;
        }
    }
    return 0;
}
