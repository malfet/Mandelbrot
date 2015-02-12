//
//  main.cpp
//  Mandelbrot
//
//  Created by Shulga Nikita on 2015/02/08.
//  Copyright (c) 2015年 Home. All rights reserved.
//

#include "GLUTWrapper.h"
#include "EscapeTimeRenderer.h"
#include <OpenGL/gl.h>
#include <thread>

template<typename T> class PolynomialDynamicalSystem: public DynamicalSystem<T> {
public:
    PolynomialDynamicalSystem(): x(0,0), c(0,0) {}
    PolynomialDynamicalSystem(Complex<T> _c): x(0,0), c(_c) {}
    PolynomialDynamicalSystem(T re, T im): x(0,0), c(re,im) {}

    Complex<T> step() {
        return x = x*x + c;
    }
    Complex<T> getVal() { return x; }
protected:
    Complex<T> x,c;
};

template<typename T> class Mandelbrot:  public PolynomialDynamicalSystem<T> {
    using PolynomialDynamicalSystem<T>::c;
    using PolynomialDynamicalSystem<T>::x;

public:
    void init(Complex<T> _c) {
        c = _c;
        x.re = x.im = 0;
    }
};

template<typename T> class Julia:  public PolynomialDynamicalSystem<T> {
    using PolynomialDynamicalSystem<T>::x;
public:
    void init(Complex<T> _x) { x = _x; }
};

template<typename T> class MandelPower: public DynamicalSystem<T> {
public:
    MandelPower(T _p):x(0,0),c(0,0),p(_p) {}

    void init(Complex<T> _c) { c = _c; x.re = x.im = 0;}
    
    Complex<T> step() {
        return x = pow(x,p) + c;
        
    }
    Complex<T> getVal() { return x; }
private:
    Complex<T> x,c;
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


class MandelPowerDemo {
public:
    MandelPowerDemo(GLUTWrapper *w): wrapper(w), surface(NULL), renderer(NULL), p(2.0),dp(.01) {
        wrapper->setDisplayFunc(std::bind(&MandelPowerDemo::display,this));
        wrapper->setReshapeFunc(std::bind(&MandelPowerDemo::reshape, this, std::placeholders::_1, std::placeholders::_2));
    }
private:
    std::function<DynamicalSystem<float> *()> getFactory() { return [&] { return new MandelPower<float>(p); }; }

    void updatePower() {
        p += dp;
        if (p < 1.1 || p > 3.9) dp *= -1;
    }
    
    void display() {
        if (!surface || !renderer) return;
        renderer->render();
        updatePower();
        copyToTexture(surface,13);
        drawQuad();
        wrapper->redisplay();
    }
    void reshape(int w, int h) {
        glInit();
        glConfigureCamera(w, h);
        if (surface) delete surface;
        surface = new OffscreenSurface(w,h, palette);
        if (renderer == NULL)
            renderer = new EscapeTimeRenderer<float>(surface, getFactory());
        else
            renderer->setSurface(surface);
    }
    Palette palette;
    GLUTWrapper *wrapper;
    OffscreenSurface *surface;
    EscapeTimeRenderer<float> *renderer;
    float p, dp;
};

int main(int argc, const char *argv[]) {
    GLUTWrapper wrapper(&argc, (char **)argv);
    MandelPowerDemo demo(&wrapper);
    
    wrapper.init(800,600);
    wrapper.run();
    return 0;
}
