#pragma once

#include <cstdint>

namespace perlin{
    uint32_t hashA(uint32_t x) {
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }
    uint32_t hashB(uint32_t x){
        x ^= x >> 15;
        x *= 0x2c1b3c6dU;
        x ^= x >> 12;
        x *= 0x297a2d39U;
        x ^= x >> 15;
        return x;
    }
    float hash(int x, int y){
        return (float)(hashA(hashA(x) ^ hashB(y)))/(float)UINT32_MAX;
    }
    float lerp(float min, float max, float t){
        return min*(1-t) + max*(t);
    }
    float noise2d(float x, float y){
        int xmin = x, ymin = y;
        float c[2][2] = {{hash(xmin,ymin),hash(xmin+1,ymin)},{hash(xmin,ymin+1),hash(xmin+1,ymin+1)}};
        float dx = x-xmin, dy = y-ymin;
        return lerp(lerp(c[0][0],c[1][0],dy),lerp(c[0][1],c[1][1],dy),dx);
    }
};