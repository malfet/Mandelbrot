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
#include <complex>
#include "OffsceenSurface.h"



template<typename T> class DynamicalSystem {
public:
    virtual ~DynamicalSystem() {}
    virtual void init(std::complex<T> c) = 0;
    virtual std::complex<T> step() = 0;
    virtual std::complex<T> getVal() = 0;
};


template<typename T> class EscapeTimeRenderer {
public:
    
    EscapeTimeRenderer(OffscreenSurface *s, std::function<DynamicalSystem<T> *()> f): surface(s), factory(f)
    {
        topleft = std::complex<T>(-2,-2);
        bottomright = std::complex<T>(2,2);
        numIterations = 256;
    }
    
    float computeEscapeTime(DynamicalSystem<T> *sys, const std::complex<T> &c) {
        sys->init(c);
        for (unsigned steps(0); steps < numIterations; ++steps) {
            auto x = sys->step();
            if (norm(x) > 4.0) {
                if (steps == 0) return 0;
                return steps + 1 - (log (log (norm(x)))/log(2));
            }
        }
        return numIterations;
        
    }

private:
    T renderSection(unsigned sx, unsigned sy, unsigned ex, unsigned ey) {
        auto w = surface->getWidth();
        auto h = surface->getHeight();
        std::complex<T> stepx((bottomright.real()-topleft.real())/w,0);
        std::complex<T> stepy(0, (bottomright.imag()-topleft.imag())/h);
        float invIterations = 1.f/numIterations;
        DynamicalSystem<T> *sys = factory();
        auto pixelArea = stepx.real()*stepy.imag();
        T rc = 0;
        for(unsigned y(sy);y<ey; ++y)
            for(unsigned x(sx); x<ex; ++x) {
                float c = computeEscapeTime(sys, topleft + ((T)y)*stepy + ((T)x)*stepx);
                if (c >= numIterations) {
                    rc += pixelArea;
                    surface->putPixel(x, y, 0, 0, 0);
                    continue;
                }
                surface->putPixel(x, y, c*invIterations);
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
    void setBounds(std::complex<T> tl, std::complex<T> br) { topleft = tl; bottomright = br; }
    void setIterations(unsigned it) { numIterations = it; }
    unsigned getIterations() { return numIterations;}
    
private:
    /* Bounding box*/
    std::complex<T> topleft,bottomright;
    
    OffscreenSurface *surface;
    /* Renderer parameters*/
    unsigned numIterations;
    /* The system itself*/
    std::function< DynamicalSystem<T> *()> factory;
};

#endif /* defined(__Mandelbrot__EscapeTimeRenderer__) */
