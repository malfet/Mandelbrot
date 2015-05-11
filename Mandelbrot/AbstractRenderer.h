/*
 * Abstract Renderer template
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

#ifndef __Mandelbrot__AbstractRenderer__
#define __Mandelbrot__AbstractRenderer__
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

template<typename T> class AbstractRenderer {
public:
    AbstractRenderer(OffscreenSurface *s, std::function<DynamicalSystem<T> *()> f): surface(s), factory(f)
    {
        topleft = std::complex<T>(-2,-2);
        bottomright = std::complex<T>(2,2);
        numIterations = 256;
    }

    void updateFactory(std::function<DynamicalSystem<T> *()> f) { factory = f; }
    void setSurface(OffscreenSurface *s) { surface = s; }
    void setBounds(std::complex<T> tl, std::complex<T> br) { topleft = tl; bottomright = br; }
    void setIterations(unsigned it) { numIterations = it; }
    unsigned getIterations() { return numIterations;}

protected:
    /* Bounding box*/
    std::complex<T> topleft,bottomright;

    OffscreenSurface *surface;
    /* Renderer parameters*/
    unsigned numIterations;
    /* The system itself*/
    std::function< DynamicalSystem<T> *()> factory;
};

#endif /* defined(__Mandelbrot__AbstractRenderer__) */

