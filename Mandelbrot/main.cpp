/*
 * Mandelbrot-like renderer demoes
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

#include "GLUTWrapper.h"
#include "EscapeTimeRenderer.h"
#include "AttractionPointRenderer.h"
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <thread>
#include <sstream>
#include <iostream>
#include "vgapalette.h"
#include "Polynomial.h"

Palette BuildVGAPalette()
{
    Palette rc;
    for (unsigned i=0;i<256;++i)
        rc[i] =  RGB<unsigned char>((vga_palette[i]>>16)&0xff, (vga_palette[i]>>8)&0xff,vga_palette[i]&0xff);
    return rc;
}

template<typename T> Polynomial<T> buildMisiurewiczPolynomial(unsigned k, unsigned n) {
    Polynomial<T> c(0);
    for(auto i(0);i<k;++i)
        c = c*c + Polynomial<T>::x;
    auto pk = c;
    for (auto i(0); i<n;++i)
        c = c*c + Polynomial<T>::x;
    return c-pk;
}


template<typename T> class PolynomialDynamicalSystem: public DynamicalSystem<T> {
public:
    PolynomialDynamicalSystem(): x(0,0), c(0,0) {}
    PolynomialDynamicalSystem(std::complex<T> _c): x(0,0), c(_c) {}
    PolynomialDynamicalSystem(T re, T im): x(0,0), c(re,im) {}

    std::complex<T> step() {
        return x = x*x + c;
    }
    std::complex<T> getVal() { return x; }
protected:
    std::complex<T> x,c;
};

template<typename T> class Mandelbrot:  public PolynomialDynamicalSystem<T> {
    using PolynomialDynamicalSystem<T>::c;
    using PolynomialDynamicalSystem<T>::x;

public:
    void init(std::complex<T> _c) {
        c = _c;
        x = 0;
    }
};

template<typename T> class Julia:  public PolynomialDynamicalSystem<T> {
    using PolynomialDynamicalSystem<T>::x;
public:
    Julia(T re, T im): PolynomialDynamicalSystem<T>(re,im) {}
    void init(std::complex<T> _x) { x = _x; }
};

template<typename T> class Newton:public DynamicalSystem<T> {
public:
    Newton(const Polynomial<T> &p): poly(p), derPoly(p.derivative()), x(0,0) {}
    std::complex<T> step() {
        return x -= poly(x)/derPoly(x);
    }
    std::complex<T> getVal() { return x;}
    void init(std::complex<T> x0) {x = x0;}

private:
    Polynomial<T> poly, derPoly;
    std::complex<T> x;
};

template<typename T> class Multibrot: public DynamicalSystem<T> {
public:
    Multibrot(T _p):x(0,0),c(0,0),p(_p) {}

    void init(std::complex<T> _c) { c = _c; x = 0; }

    std::complex<T> step() {
        return x = pow(x,p) + c;

    }
    std::complex<T> getVal() { return x; }
private:
    std::complex<T> x,c;
    T p;
};

void glConfigureCamera(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0,1, 1, 0, .2, 999999.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}


void glInit() {
    glClearColor (.0, .0, .0, .0);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_TEXTURE_2D);
    glLineWidth(5.f);
}

void copyToTexture(OffscreenSurface *s, int texture)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->getWidth(), s->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, s->getData());
}


void drawQuad(float x=0.0,float y=0.0, float w=1.0, float h=1.0, float z = -10.0)
{
    glBegin(GL_QUADS);
    glTexCoord2f (x, y);
    glVertex3f (x, y, z);
    glTexCoord2f (x, y+h);
    glVertex3f (x, y+h, z);
    glTexCoord2f (x+w, y+h);
    glVertex3f (x+w, y+h, z);
    glTexCoord2f (x+w, y);
    glVertex3f (x+w, y, z);
    glEnd();
}

typedef std::pair<float,float> point;

void drawRect(float sx, float sy, float ex, float ey, float z = -9.0)
{
    glBegin(GL_LINE_LOOP);
    glVertex3f (sx, sy, z);
    glVertex3f (ex, sy, z);
    glVertex3f (ex, ey, z);
    glVertex3f (sx, ey, z);
    glEnd();
}
void drawRect(point start, point end, float z = -9.0)
{
    drawRect(start.first, start.second, end.first, end.second);
}

std::string getHomeFolder() {
    const char *homeDir = getenv("HOME");
    return std::string(homeDir);
}



template<typename T>
class MultibrotDemo {
public:
    MultibrotDemo(GLUTWrapper *w): wrapper(w), surface(NULL), renderer(NULL), p(1.0),dp(.005) {
        palette = BuildVGAPalette();
        wrapper->setDisplayFunc(std::bind(&MultibrotDemo::display,this));
        wrapper->setReshapeFunc(std::bind(&MultibrotDemo::reshape, this, std::placeholders::_1, std::placeholders::_2));
    }
private:
    std::function<DynamicalSystem<T> *()> getFactory() { return [&] { return new Multibrot<T>(p); }; }

    void startRenderer() {
        updatePower();
        //std::packaged_task<std::pair<float,float>()> tsk(std::bind(&EscapeTimeRenderer<float>::render, renderer));
        std::packaged_task<std::pair<float,float>(EscapeTimeRenderer<T> *)> tsk(&EscapeTimeRenderer<T>::render);

        renderResult = tsk.get_future();
        std::thread taskThread(std::move(tsk), renderer);
        taskThread.detach();
    }

    void updatePower() {
        p += dp;
        if (p < 1.0 || p > 5.0) dp *= -1;
        if (p>5.0) wrapper->quit();
    }

    void saveImage() {
        std::ostringstream ss;
        ss<<getHomeFolder()<<"/Mandel-results/pow(x,"<<p<<").jpg";
        //surface->saveToPNG(ss.str());
        surface->saveToJPEG(ss.str());

    }
    void savePalette() {
        std::ostringstream ss;
        ss<<getHomeFolder()<<"/Mandel-results/palette.dat";
        palette.save(ss.str());
    }

    void updateTitle(float area, float time) {
        std::ostringstream ss;
        ss<<"x=pow(x,"<<p<<")+c area="<<area<<" time="<<time<<" ms";
        wrapper->setWindowTitle(ss.str());
    }

    void display() {
        if (!surface || !renderer) return;
        copyToTexture(surface,13);
        drawQuad();
        wrapper->redisplay();

        if (renderResult.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
        auto rc = renderResult.get();
        updateTitle(rc.first, rc.second);
        saveImage();
        startRenderer();
    }

    void reshape(int w, int h) {

        glInit();
        glConfigureCamera(w, h);

        OffscreenSurface *oldSurface = surface;
        surface = new OffscreenSurface(w,h, palette);
        if (renderer == NULL) {
            renderer = new EscapeTimeRenderer<T>(surface, getFactory());
            startRenderer();
        } else {
            renderResult.wait();
            renderer->setSurface(surface);
        }
        delete oldSurface;
    }

    std::future<std::pair<float,float> > renderResult;
    Palette palette;
    GLUTWrapper *wrapper;
    OffscreenSurface *surface;
    EscapeTimeRenderer<T> *renderer;
    T p, dp;
};

template<typename T,typename Renderer>
class ZoomInViewer {
public:
    ZoomInViewer(GLUTWrapper *w): wrapper(w), surface(NULL), renderer(NULL) {
        topLeft = std::complex<T>(-2,-2);
        bottomRight = std::complex<T>(2,2);
        numIterations = 256;
        palette = BuildVGAPalette();
        mouseDown = false;
        wrapper->setDisplayFunc(std::bind(&ZoomInViewer::display,this));
        wrapper->setReshapeFunc(std::bind(&ZoomInViewer::reshape, this, std::placeholders::_1, std::placeholders::_2));
        wrapper->setMouseFunc(std::bind(&ZoomInViewer::mouse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    }

    //std::function<DynamicalSystem<T> *()> getFactory() { return [&] { return new Julia<T>((T)-0.77568377, (T)0.13646737); }; }
    //std::function<DynamicalSystem<T> *()> getFactory() { return [&] { return new Mandelbrot<T>(); }; }
    //std::function<DynamicalSystem<T> *()> getFactory() { return [&] { return new Newton<T>((Polynomial<T>::x^3)-1); }; }
    std::function<DynamicalSystem<T> *()> getFactory() { return [&] { return new Newton<T>(buildMisiurewiczPolynomial<T>(4,2)); }; }


    void reshape(int w, int h) {

        glInit();
        glConfigureCamera(w, h);

        OffscreenSurface *oldSurface = surface;
        surface = new OffscreenSurface(w,h, palette);
        if (renderer == NULL) {
            renderer = new Renderer(surface, getFactory());
        } else {
            if (renderResult.valid())
                renderResult.wait();
            renderer->setSurface(surface);
        }
        delete oldSurface;
        startRenderer();

    }

    void display() {
        if (!surface || !renderer) return;
        copyToTexture(surface,13);
        glColor3f(1.0f,1.0f, 1.0f);
        drawQuad();
        if (mouseDown) {
            glColor3f(1.0f,1.0f, 0.f);
            drawRect(start, end);
        }

        if (!renderResult.valid())
            return;
        if (renderResult.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            wrapper->redisplay();
            return;
        }

        auto rc = renderResult.get();
        updateTitle(rc.first, rc.second);
        if (numIterations < 10000) {
            numIterations *= 1.1;
            startRenderer();
        }
    }

    std::complex<T> pointToSurface(point p) {
        T stepX = (bottomRight.real()-topLeft.real());
        T stepY = (bottomRight.imag()-topLeft.imag());
        return topLeft + std::complex<T>(stepX*p.first,stepY*p.second);
    }

    point screenToPoint(unsigned x, unsigned y) {
        return point(((float)x)/surface->getWidth(), ((float)y)/surface->getHeight());
    }

    void mouse(int x, int y, unsigned button)
    {
        if (!mouseDown) {
            end = start = screenToPoint(x,y);
        } else {
            float ratio = (float)surface->getWidth()/surface->getHeight();
            end = screenToPoint(x,y);
            float w = end.first-start.first;
            float h = end.second-start.second;
            float rw = std::max(w,h*ratio);
            end.first = start.first+rw;
            end.second = start.second+(rw/ratio);
        }

        if (mouseDown && button == 0) {
            auto tl  = pointToSurface(start);
            auto br = pointToSurface(end);
            topLeft = tl;
            bottomRight = br;
            numIterations = 256;
            startRenderer();
        }
        mouseDown = button != 0;

        wrapper->redisplay();

    }

    void startRenderer() {
        std::packaged_task<std::pair<float,float>(Renderer *)> tsk(&Renderer::render);

        if (renderResult.valid())
            renderResult.wait();

        renderer->setBounds(topLeft, bottomRight);
        renderer->setIterations(numIterations);

        renderResult = tsk.get_future();
        std::thread taskThread(std::move(tsk), renderer);
        taskThread.detach();
        wrapper->redisplay();

    }

    void updateTitle(float area, float time) {
        std::ostringstream ss;
        ss<<"Mandelbrot "<<topLeft<<"-"<<bottomRight<<": iterations="<<numIterations<<" area="<<area<<" time="<<time<<" ms";
        wrapper->setWindowTitle(ss.str());
    }

private:
    std::future<std::pair<float,float> > renderResult;
    Palette palette;
    GLUTWrapper *wrapper;
    OffscreenSurface *surface;
    Renderer *renderer;
    std::complex<T> topLeft, bottomRight;
    unsigned numIterations;
    point start, end;
    bool mouseDown;

};

template<typename T> std::vector<std::complex<T> > findMisiurewiczRootsBairstow(unsigned k, unsigned n) {
    std::vector<std::complex<T> > rc;
    auto c = buildMisiurewiczPolynomial<T>(k, n);
    /* Eliminate 0 as root */
    while (c.isRoot(0))
       c = c.deflate(0);
    while (c.degree()>1) {
        std::pair<T,T> factors;
        try {
            factors = findQuadraticFactorBairstow(c);
        } catch (DoNotConvergeException<std::pair<T, T> > const &e) {
            factors = e.getValue();
            std::cerr<<"Could not converge to a root around factors "<<factors.first<<" and "<<factors.second<<std::endl;
        }
        auto roots = solveQuadratic(factors.first, factors.second);
        rc.push_back(roots.first);
        rc.push_back(roots.second);
        c = c.deflate(factors.first, factors.second);
    }
    return rc;
}

template<typename T> std::vector<std::complex<T> > findMisiurewiczRootsLaguerre(unsigned k, unsigned n) {
    std::vector<std::complex<T> > rc;
    auto c = buildMisiurewiczPolynomial<std::complex<T>>(k, n);
    /* Eliminate 0 as root */
    while (c.isRoot(std::complex<T>(0)))
       c = c.deflate(std::complex<T>(0));

    try {
        while (c.degree()>1) {
            auto r = findRootLaguerre(c);
            rc.push_back(r);
            c = c.deflate(r);
        }
    } catch (DoNotConvergeException<std::complex<T>> const &e) {
        std::cerr<<"Could not converge to a root around "<<e.getValue()<<std::endl;
    }
    return rc;
}



int main(int argc, const char *argv[]) {
    if (argc > 1) {
        int k = atoi(argv[1]);
        unsigned n = argc>2 ? atoi(argv[2]) : 2;
        if (k<= 0) k = 4;
        auto pol = buildMisiurewiczPolynomial<double>(k,n);
        std::cout<<"Misiurewicz("<<k<<","<<n<<") polynomial is "<<pol<<std::endl;
        auto roots = findMisiurewiczRootsBairstow<double>(k, n);
        //auto roots = findMisiurewiczRootsLaguerre<double>(k, n);
        std::cout<<"Roots are ";
        for (auto r: roots) std::cout<<" "<<r<<" (error="<<std::abs(pol(r))<<")";
        std::cout<<std::endl;
        return 0;

    }

    GLUTWrapper wrapper(&argc, (char **)argv);
    ZoomInViewer<double,AttractionPointRenderer<double>> demo(&wrapper);

    wrapper.init(1080, 1080);
    wrapper.run();
    return 0;
}
