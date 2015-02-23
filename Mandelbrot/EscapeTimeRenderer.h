/*
 * Escape time rendered template
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
        numIterations = 256;
    }
    
    T computeEscapeTime(DynamicalSystem<T> *sys, const Complex<T> &c) {
        sys->init(c);
        for (unsigned steps(0); steps < numIterations; ++steps) {
            auto x = sys->step();
            if (x.mod2() > 4.0) {
                if (steps == 0) return 0;
                return steps + 1 - (log (log (x.mod2()))/log(2));
            }
        }
        return numIterations;
        
    }

private:
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
    
    typedef std::pair<unsigned,unsigned> point;
    
    point make_point(unsigned x, unsigned y) {return std::pair<unsigned,unsigned>(x,y);}
    
    /* Partition area into (numSection+1)*(numSection+1) equally sized squares */
    std::vector<std::pair<point, point> > partitionArea(unsigned numSections=1) {
        unsigned width = surface->getWidth();
        unsigned height = surface->getHeight();
        unsigned stepW = width/(numSections+1);
        unsigned stepH = height/(numSections+1);
        std::vector<std::pair<point, point> > rc;
        for (unsigned x(0); x<numSections+1;++x)
            for(unsigned y(0); y<numSections+1;++y) {
                auto tl = make_point(x*stepW,y*stepH);
                auto br = make_point(x==numSections ? width : (x+1)*stepW, y == numSections? height : (y+1)*stepH);
                rc.push_back(std::pair<point,point>(tl,br));
            }
        return rc;
    }
    
    std::future<T> renderAsync(point tl, point br) {
        std::future<T> rc(std::async(std::launch::async, &EscapeTimeRenderer::renderSection, this, tl.first, tl.second, br.first, br.second));
        return rc;
    }
    
public:
    /*Return area and time in miliseconds */
    std::pair<T,T> render(void) {
        
        auto start = std::chrono::steady_clock::now();
        
        std::vector<std::future<T> > rc;
        for(auto& reg: partitionArea(3))
            rc.push_back(renderAsync(reg.first, reg.second));

        T area = 0;
        for (auto &res: rc)
            area += res.get();
        auto stop = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
        return std::pair<T,T>(area, duration);
    }
    void updateFactory(std::function<DynamicalSystem<T> *()> f) { factory = f; }
    void setSurface(OffscreenSurface *s) { surface = s; }
    
private:
    /* Bounding box*/
    Complex<T> topleft,bottomright;
    
    OffscreenSurface *surface;
    /* Renderer parameters*/
    unsigned numIterations;
    /* The system itself*/
    std::function< DynamicalSystem<T> *()> factory;
};

#endif /* defined(__Mandelbrot__EscapeTimeRenderer__) */
