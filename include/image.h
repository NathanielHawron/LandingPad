#pragma once

#include <cstdint>
#include <cmath>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

struct rgba{
    uint8_t r, g, b, a;
    operator const uint32_t() const {return *(const uint32_t*)this;}
    bool equals(const rgba &col, uint8_t tr, uint8_t tg, uint8_t tb, uint8_t ta = 255) const {
        uint8_t dr = std::abs((int16_t)this->r - (int16_t)col.r), dg = std::abs((int16_t)this->g - (int16_t)col.g), db = std::abs((int16_t)this->b - (int16_t)col.b), da = std::abs((int16_t)this->a - (int16_t)col.a);
        return dr < tr && dg < tg && db < tb && da < ta;
    }
};
struct direction{
    int dx, dy;
    float magSq(){
        return this->dx*this->dx + this->dy*this->dy;
    }
};
struct point{
    uint32_t x, y;
    point operator+(direction d){
        return {(uint32_t)std::max(((int)this->x)+d.dx,0),(uint32_t)std::max(((int)this->y)+d.dy,0)};
    }
    direction operator-(point p){
        return {(int)this->x-(int)p.x,(int)this->y-(int)p.y};
    }
};

class Image{
public:
    rgba *pixels;
    rgba *annotated;
    int width, height;
    Image(rgba *pixels, point p1, point p2, int srcWidth){
        int minX = std::min(p1.x,p2.x);
        int minY = std::min(p1.y,p2.y);
        int maxX = std::max(p1.x,p2.x);
        int maxY = std::max(p1.y,p2.y);
        int dx = maxX-minX;
        int dy = maxY-minY;

        this->width = dx;
        this->height = dy;

        this->pixels = new rgba[dx*dy];
        this->annotated = new rgba[dx*dy];

        for(int y=0;y<dy;++y){
            memcpy(this->pixels+y*dx,pixels+(y+minY)*srcWidth+minX,dx*4);
        }
        memcpy(this->annotated,this->pixels,dx*dy*sizeof(rgba));
    }
    ~Image(){
        delete[] this->pixels;
        delete[] this->annotated;
    }
    rgba getPixel(point p){
        return this->pixels[p.x + this->width * p.y];
    }
    bool pixelEq(point p,rgba color, int tolerance){
        return this->getPixel(p).equals(color,tolerance,tolerance,tolerance);
    }
    void annotatePixel(point p, rgba color){
        if(color.a > 0){
            this->annotated[p.x + this->width * p.y] = color;
        }
    }
    bool inRange(point p){
        return p.x < this->width && p.y < this->height;
    }
    bool linearSearch(point start, direction dPos, point &res, rgba color, int tolerance, rgba annotationColor = {255,0,0,255}){
        res = start;
        while(!getPixel(res).equals(color,tolerance,tolerance,tolerance)){
            this->annotatePixel(res,annotationColor);
            res = res + dPos;
            if(!this->inRange(res)){
                return false;
            }
        }
        return true;
    }
    bool xDirOfMax(point p, bool &pxIsMax, rgba padColor, rgba borderColor, int tolerance, rgba annotateColor = {255,255,0,255}){
        // Cannot be determined if too close to edge
        if(p.y <= 1 || p.y >= this->height-2){
            return false;
        }
        uint32_t y = p.y;
        for(uint32_t x=p.x+1;x<this->width;++x){
            if(this->getPixel({x,y}).equals(borderColor,tolerance,tolerance,tolerance)){
                pxIsMax = false;
                return true;
            }else if(this->getPixel({x,y+1}).equals(padColor,tolerance,tolerance,tolerance) && this->getPixel({x,y-1}).equals(padColor,tolerance,tolerance,tolerance)){
                pxIsMax = true;
                return true;
            }
            this->annotatePixel({x,y},annotateColor);
        }
        return false;
    }
    bool traceBorder(point start, point center, bool ccw, bool findMin, uint32_t checks, point &res, float &radiusSq, rgba borderColor, int tolerance, rgba annotateColor = {255,0,255,255}){
        if(checks == 0 || !this->inRange(center)){
            return false;
        }
        if(findMin){
            radiusSq = this->width*this->width*this->height*this->height;
        }else{
            radiusSq = 0;
        }
        for(int count = 0;count <= checks;++count){
            if(!this->inRange(start)){
                return false;
            }
            this->annotatePixel(start,annotateColor);
            //Check current point
            direction delta = start-center;
            float rSq = delta.magSq();
            if((rSq <= radiusSq) == findMin){
                count = 0;
                radiusSq = rSq;
                res = start;
            }
            //Find next point
            if(std::abs(delta.dx)*this->height > std::abs(delta.dy)*this->width){
                // Left or right of screen
                start.y -= ((delta.dx>0)==ccw)?-1:1;
                if(this->pixelEq(start,borderColor,tolerance)){
                    do{
                        start.x += delta.dy>0 ? -1:1;
                    }while(this->pixelEq(start,borderColor,tolerance));
                }else{
                    do{
                        start.x -= delta.dy>0 ? -1:1;
                    }while(!this->pixelEq(start,borderColor,tolerance));
                    start.x += delta.dy>0 ? -1:1;
                }
            }else{
                // Top or bottom of screen
                start.x += ((delta.dy>0)==ccw)?-1:1;
                if(this->pixelEq(start,borderColor,tolerance)){
                    do{
                        start.y += delta.dy>0 ? -1:1;
                    }while(this->pixelEq(start,borderColor,tolerance));
                }else{
                    do{
                        start.y -= delta.dy>0 ? -1:1;
                    }while(!this->pixelEq(start,borderColor,tolerance));
                    start.y += delta.dy>0 ? -1:1;
                }
            }
        }
        return true;
    }
    void save(std::string fname){
        stbi_write_png((fname+".png").c_str(),this->width,this->height,4,this->pixels,4*this->width);
        stbi_write_png((fname+"_annotated.png").c_str(),this->width,this->height,4,this->annotated,4*this->width);
    }
};