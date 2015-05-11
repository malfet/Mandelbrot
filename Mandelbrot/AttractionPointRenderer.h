/*
 * Attraction point renderer template
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

#ifndef Mandelbrot_AttractionPointRenderer_h
#define Mandelbrot_AttractionPointRenderer_h
#include <complex>
#include <iostream>
#include "AbstractRenderer.h"

template<typename T> class AttractionPointRenderer: public AbstractRenderer<T> {
private:
    std::pair<std::complex<T>, float>  computeAttractionTime(DynamicalSystem<T> *sys, const std::complex<T> &x0) {
        auto px = x0;
        sys->init(x0);
        for (unsigned steps(0); steps < numIterations; ++steps) {
            auto x = sys->step();
            auto diff = x-px;
            px = x;
            if (norm(diff) < 1e-8) {
                return std::pair<std::complex<T>, float> (x, steps);
            }
        }
        return std::pair<std::complex<T>, float> (x0, numIterations);
    }

public:

    AttractionPointRenderer(OffscreenSurface *s, std::function<DynamicalSystem<T> *()> f): AbstractRenderer<T>(s,f) {}

    unsigned getAttractionPointIndex(const std::complex<T> &point) {
        unsigned idx=0;
        for(auto p: attractionPoints)
            if (norm(p-point)<1e-6)
                break;
            else idx++;
        if (idx >= attractionPoints.size()) {
            std::cout<<"Adding "<<(idx+1)<<" attraction point"<<point<<std::endl;
            attractionPoints.push_back(point);
        }
        return idx;
    }

    /*Return area and time in milliseconds */
    std::pair<T,T> render(void) {
        auto start = std::chrono::steady_clock::now();
        float invIterations = 1.f/numIterations;
        auto width = surface->getWidth();
        auto height = surface->getHeight();
        std::complex<T> stepx((bottomright.real()-topleft.real())/width,0);
        std::complex<T> stepy(0, (bottomright.imag()-topleft.imag())/height);
        DynamicalSystem<T> *sys = factory();
        for(auto y(0); y<height;y++)
            for(auto x(0); x<width;x++) {
                auto c = computeAttractionTime(sys, topleft + ((T)y)*stepy + ((T)x)*stepx);
                if (c.second >= numIterations)
                    surface->putPixel(x, y, 0, 0, 0);
                else {
                    auto idx = getAttractionPointIndex(c.first);
                    surface->putPixel(x, y, (float(idx)/attractionPoints.size())+c.second*invIterations);
                }
            }
        delete sys;
        auto stop = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
        return std::pair<T,T>(T(attractionPoints.size()), duration);
    }

private:
    /* Attraction points */
    std::vector<std::complex<T>> attractionPoints;

    /* Bounding box*/
    using AbstractRenderer<T>::topleft;
    using AbstractRenderer<T>::bottomright;

    /*Surface*/
    using AbstractRenderer<T>::surface;

    /* Renderer parameters*/
    using AbstractRenderer<T>::numIterations;
    /* The system itself*/
    using AbstractRenderer<T>::factory;
};


#endif
