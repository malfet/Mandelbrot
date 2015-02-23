/*
 * Bitmap surface primitives and palette handling classes
 *
 * Copyright (c) 2015 Nikita Shulga
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
    RGB<unsigned char >& operator[](size_t idx);
    void save(const std::string &name);
    void load(const std::string &name);
    void randomize();
    /* Generate a palette with fixed luma and minimum distance between UV components*/
    void randomizeUV(unsigned char Y, unsigned distance=80);

private:
    std::vector< RGB<unsigned char> > data;
};

class OffscreenSurface {
public:
    void saveToPNG(const std::string &name);
    OffscreenSurface(unsigned, unsigned);
    OffscreenSurface(unsigned, unsigned, Palette &p);
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
