//
//  OffsceenSurface.h
//  Mandelbrot
//
//  Created by Shulga Nikita on 2015/02/08.
//  Copyright (c) 2015年 Home. All rights reserved.
//

#ifndef __Mandelbrot__OffsceenSurface__
#define __Mandelbrot__OffsceenSurface__
#include <vector>

template<typename T> class RGB {
public:
    RGB() { col[0] = col[1] = col[2] = 0;}
    RGB(T r,T g, T b) {
        col[0] = r;
        col[1] = g;
        col[2] = b;
    }
    const inline T getR() const { return col[0];}
    const inline T getG() const { return col[1];}
    const inline T getB() const { return col[2];}
    RGB operator+(const RGB &x) {
        return RGB(col[0]+x.col[0], col[1]+x.col[1], col[2]+x.col[2]);
    }
private:
    T col[3];
};

template<typename T> RGB<T> operator*(float x, const RGB<T> &y) {
    return RGB<T>(x*y.getR(),x*y.getG(), x*y.getB());
}


class Palette {
public:
    Palette(unsigned size=256):data(size) {randomize();}
    size_t size() { return data.size();}
    void randomize(void);
    RGB<unsigned char >& operator[](size_t idx);
private:
    std::vector< RGB<unsigned char> > data;
};

class OffscreenSurface {
public:
    OffscreenSurface(unsigned, unsigned);
    ~OffscreenSurface();
    inline unsigned getWidth() { return width; }
    inline unsigned getHeight() { return height; }
    inline unsigned char *getData() { return rgb;}
    void clear();
    void putPixel(int x, int y, RGB<unsigned char> c);
    void putPixel(int,int, unsigned char, unsigned char, unsigned char);
    void putPixel(int, int, unsigned);
    void putPixel(int, int, float);
    void setPalette(const Palette &p) {palette = p;}
private:
    unsigned width,height;
    Palette palette;
    unsigned char *rgb;
};
#endif /* defined(__Mandelbrot__OffsceenSurface__) */
