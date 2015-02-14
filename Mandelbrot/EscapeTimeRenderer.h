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
#include <future>
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
        numIterations = 200;
    }
    
    T computeEscapeTime(DynamicalSystem<T> *sys, const Complex<T> &c) {
        sys->init(c);
        for (unsigned steps(0); steps < numIterations; ++steps) {
            auto x = sys->step();
            if (x.mod2() > 4.0) {
                if (steps == 0) return 0;
                return steps + 1 - (log (log (x.mod2()))/log(2));
            }
            if (x.mod2() < .03) return numIterations;
        }
        return numIterations;
        
    }
    
    T renderSection(unsigned sx, unsigned sy, unsigned ex, unsigned ey) {
        auto w = surface->getWidth();
        auto h = surface->getHeight();
        Complex<T> stepx((bottomright.re-topleft.re)/w,0);
        Complex<T> stepy(0, (bottomright.im-topleft.im)/h);
        DynamicalSystem<T> *sys = factory();
        auto pixelArea = stepx.re*stepy.im;
        T rc = 0;
        for(unsigned y(sy);y<ey; ++y)
            for(unsigned x(sx); x<ex; ++x) {
                T c = computeEscapeTime(sys, topleft + y*stepy + x*stepx);
                if (c >= numIterations) {
                    rc += pixelArea;
                    surface->putPixel(x, y, 0, 0, 0);
                    continue;
                }
                surface->putPixel(x, y, c);
            }
        delete sys;
        return rc;
        
    }
    
    void render(void) {
        
        auto start = std::chrono::steady_clock::now();
#if 0
        T area = renderSection(0, 0, surface->getWidth(), surface->getHeight());
#else
        unsigned w = surface->getWidth();
        unsigned h = surface->getHeight();
        unsigned midw = w/2;
        unsigned midh = h/2;

        auto tl = std::async(std::launch::deferred, &EscapeTimeRenderer::renderSection, this, 0, 0, midw, midh);
        auto tr = std::async(std::launch::async, &EscapeTimeRenderer::renderSection, this, midw, 0, w, midh);
        auto bl = std::async(std::launch::async, &EscapeTimeRenderer::renderSection, this, 0, midh, midw, h);
        auto br = std::async(std::launch::async, &EscapeTimeRenderer::renderSection, this, midw, midh, w, h);

        T area = tl.get();
        area += bl.get();
        area += tr.get();
        area += br.get();
#endif
        auto stop = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
        std::cout<<"area is "<<area<<" duration is "<<duration<<" ms"<<std::endl;
    }
    void updateFactory(std::function<DynamicalSystem<T> *()> f) { factory = f; }
    void setSurface(OffscreenSurface *s) { surface = s; }
private:
    Complex<T> topleft,bottomright;
    OffscreenSurface *surface;
    unsigned numIterations;
    std::function< DynamicalSystem<T> *()> factory;
};

#endif /* defined(__Mandelbrot__EscapeTimeRenderer__) */
