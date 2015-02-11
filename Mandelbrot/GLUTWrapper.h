//
//  GLUTWrapper.h
//  Mandelbrot
//
//  Created by Shulga Nikita on 2015/02/08.
//  Copyright (c) 2015年 Home. All rights reserved.
//

#ifndef Mandelbrot_GLUTWrapper_h
#define Mandelbrot_GLUTWrapper_h

#include <functional>

class GLUTWrapper {
public:
    GLUTWrapper(int *argc, char **argv);
    void printDisplayDimentions();
    void init(int, int);
    void run();
    void setDisplayFunc(std::function<void()> f) { displayFunc = f;}
    void setReshapeFunc(std::function<void(int,int)> f) { reshapeFunc = f;}
    void redisplay();
private:
    static void display();
    static void reshape(int w, int h) { self->reshapeFunc(w,h);}
    std::function<void()> displayFunc;
    std::function<void(int,int)> reshapeFunc;
    static GLUTWrapper *self;
    int winWidth, winHeight;
};

#endif
