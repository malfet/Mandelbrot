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
#include <OpenGL/gl.h>
#include <thread>
#include <sstream>
#include "vgapalette.h"

Palette BuildVGAPalette()
{
    Palette rc;
    for (unsigned i=0;i<256;++i)
        rc[i] =  RGB<unsigned char>((vga_palette[i]>>16)&0xff, (vga_palette[i]>>8)&0xff,vga_palette[i]&0xff);
    return rc;
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
        x.re = x.im = 0;
    }
};

template<typename T> class Julia:  public PolynomialDynamicalSystem<T> {
    using PolynomialDynamicalSystem<T>::x;
public:
    void init(std::complex<T> _x) { x = _x; }
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
    glOrtho (0,1, 0, 1, .2, 999999.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}


void glInit() {
    glClearColor (.0, .0, .0, .0);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_TEXTURE_2D);
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
            renderer = new EscapeTimeRenderer<float>(surface, getFactory());
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

int main(int argc, const char *argv[]) {
    GLUTWrapper wrapper(&argc, (char **)argv);
    MultibrotDemo<float> demo(&wrapper);
    
    wrapper.init(1080, 1080);
    wrapper.run();
    return 0;
}
