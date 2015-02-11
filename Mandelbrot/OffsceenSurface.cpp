//
//  OffsceenSurface.cpp
//  Mandelbrot
//
//  Created by Shulga Nikita on 2015/02/08.
//  Copyright (c) 2015年 Home. All rights reserved.
//

#include "OffsceenSurface.h"
#include <random>
#include <assert.h>

void Palette::randomize()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0,255*255*255);
    for(unsigned i(0); i < data.size(); ++i) {
        auto val = dis(gen);
        data[i] = RGB<unsigned char>((val>>16)&255,(val>>8)&255,val&255);
    }
}

RGB<unsigned char >& Palette::operator[](size_t idx)
{
    assert (idx < data.size());
    return data[idx];
}


OffscreenSurface::OffscreenSurface(unsigned w, unsigned h):width(w),height(h)
{
    rgb = new unsigned char[width*height*3];
}

void OffscreenSurface::putPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
    auto offs = (y*width+x)*3;
    rgb[offs+0] = r;
    rgb[offs+1] = g;
    rgb[offs+2] = b;
}

void OffscreenSurface::putPixel(int x, int y, RGB<unsigned char> c)
{
    putPixel(x,y,c.getR(), c.getG(), c.getB());
}

void OffscreenSurface::putPixel(int x, int y, unsigned idx)
{
    putPixel(x,y, palette[idx]);
}

void OffscreenSurface::putPixel(int x, int y, float val)
{
    unsigned idx = unsigned(floor(val));
    if (idx >= palette.size()-1) {
        putPixel(x,y, palette[palette.size()-1]);
        return;
    }
    float a = val - idx;
    putPixel(x,y, (1-a)*palette[idx]+a*palette[idx+1]);
}


OffscreenSurface::~OffscreenSurface() {
    delete[] rgb;
}


void OffscreenSurface::clear()
{
    memset(rgb, 0, 3*width*height);
}