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

template<typename T> class Mandelbrot: public DynamicalSystem<T> {
public:
    void init(Complex<T> _c) {
        c = _c;
        x.re = x.im = 0;
    }
    void step() {
        x = x*x + c;
    }
    Complex<T> getVal() { return x; }
private:
    Complex<T> x,c;
};

template<typename T> class Julia: public DynamicalSystem<T> {
public:
    Julia(Complex<T> _c): x(0,0), c(_c) {}
    Julia(T re, T im): x(0,0), c(re,im) {}
    void init(Complex<T> _x) { x = _x; }
    
    Complex<T> step() {
        return x = x*x + c;
    }
    Complex<T> getVal() { return x; }
private:
    Complex<T> x, c;
};

void reshape(int width, int height) {
    glClearColor (.0, .0, .0, .0);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_TEXTURE_2D);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0,1, 0, 1, .2, 999999.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
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

int main(int argc, const char * argv[]) {
    
    OffscreenSurface *surface = new OffscreenSurface(800,600);
    EscapeTimeRenderer<float> renderer(surface, [] { return new Julia<float>(.10101,.9562);});
//    EscapeTimeRenderer<float> renderer(surface, [] { return new Mandelbrot<float>();});
    renderer.render();
    
    
    GLUTWrapper wrapper(&argc, (char **)argv);
    
    auto display = [&]() {
        copyToTexture(surface,13);
        glBegin (GL_QUADS);
        glTexCoord2f (0.0, 0.0);
        glVertex3f (0.0, 0.0, -10.0);
        glTexCoord2f (0.0, 1.0);
        glVertex3f (0.0, 1.0, -10.0);
        glTexCoord2f (1.0, 1.0);
        glVertex3f (1.0, 1.0, -10.0);
        glTexCoord2f (1.0, 0.0);
        glVertex3f (1.0, 0.0, -10.0);
        glEnd();
    };
    wrapper.setDisplayFunc(display);
    wrapper.setReshapeFunc(reshape);
    wrapper.init(800,600);
    wrapper.run();
    return 0;
}
