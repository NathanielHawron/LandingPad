#pragma once

#include "graphicsLibrary/include/texture.h"
#include "perlin.h"

namespace filter{
    void noise(graphics::Image &img, uint32_t mag){
        uint8_t *buf = img.buffer;
        uint8_t *magCmp = (uint8_t*)&mag;
        for(int x=0;x<img.width;++x){
            for(int y=0;y<img.height;++y){
                int offset = 4*(x*img.height + y);
                for(int i=0;i<4;++i){
                    int index = offset + i;
                    int8_t d = magCmp[i]*(perlin::noise2d(5*i + x/100.0f,11 * i + y/100.0f) + 0.5f*perlin::noise2d(7*i + 50+x/200.0f,17*i + 50+y/200.0f));
                    uint8_t current = buf[index];
                    if(d<0){
                        if(abs(d)>=current){
                            buf[index] = 0;
                        }else{
                            buf[index] += d;
                        }
                    }else{
                        if(d+current < current){
                            buf[index] = 255;
                        }else{
                            buf[index] += d;
                        }
                    }
                }
            }
        }
    }
}