#pragma once

#include "graphicsLibrary/include/texture.h"

namespace filter{
    void circle(graphics::Image &img, uint32_t fill, uint32_t stroke, uint32_t strokeWeight, int8_t noise, bool bg, uint32_t bgColor = 0){
        uint32_t *buf = img.buffer32;
        double fs = 0.5f*(double)strokeWeight/std::sqrt(img.width*img.height);
        for(int i=0;i<img.width;++i){
            for(int j=0;j<img.height;++j){
                double fw = 2.0f*(((double)i/(double)img.width) - 0.5f);
                double fh = 2.0f*(((double)j/(double)img.height) - 0.5f);
                double r = fw*fw + fh*fh;
                if(r <= 1.0f){
                    if(r < 1.0-fs){
                        buf[i+j*img.width] = fill;
                    }else{
                        buf[i+j*img.width] = stroke;
                    }
                }else{
                    buf[i+j*img.width] = bgColor;
                }
            }
        }
    }
}