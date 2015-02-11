//
//  EscapeTimeRenderer.h
//  Mandelbrot
//
//  Created by Shulga Nikita on 2015/02/09.
//  Copyright (c) 2015年 Home. All rights reserved.
//

#ifndef __Mandelbrot__EscapeTimeRenderer__
#define __Mandelbrot__EscapeTimeRenderer__
#include <functional>
#include <chrono>
#include "complex.h"
#include "OffsceenSurface.h"



template<typename T> class DynamicalSystem {
public:
    virtual ~DynamicalSystem() {}
    virtual void init(Complex<T> c) = 0;
    virtual Complex<T> step() = 0;
    virtual Complex<T> getVal() = 0;
};

template<typename T> class EscapeTimeRenderer {
public:
    EscapeTimeRenderer(OffscreenSurface *s, std::function<DynamicalSystem<T> *()> f): surface(s), factory(f)
    {
        topleft = Complex<T>(-2,-2);
        bottomright = Complex<T>(2,2);
        numIterations = 256;
    }
    
    void render(void) {
        
        surface->clear();
        
        auto w = surface->getWidth();
        auto h = surface->getHeight();
        Complex<T> stepx((bottomright.re-topleft.re)/w,0);
        Complex<T> stepy(0, (bottomright.im-topleft.im)/h);
        DynamicalSystem<T> *sys = factory();
        
        auto start = std::chrono::steady_clock::now();
        for(unsigned y(0);y<h; ++y)
            for(unsigned x(0); x<w; ++x) {
                sys->init(topleft+y*stepy+x*stepx);
                for(unsigned steps(0);steps<numIterations;steps++) {
                    auto x_ = sys->step();
                    if (x_.mod2()>4.0) {
                        float c =steps+1-(log(log(x_.mod2()))/log(2));
                        surface->putPixel(x, y, c);
                        break;
                    }
                }
            }
        auto stop = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
        std::cout<<"duration is "<<duration<<" ms"<<std::endl;
        delete sys;
    }
private:
    Complex<T> topleft,bottomright;
    OffscreenSurface *surface;
    unsigned numIterations;
    std::function< DynamicalSystem<T> *()> factory;
};

#endif /* defined(__Mandelbrot__EscapeTimeRenderer__) */
