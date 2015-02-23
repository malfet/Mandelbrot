/*
 * C++esuqe wrapper of the core GLUT functionality
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

#ifndef Mandelbrot_GLUTWrapper_h
#define Mandelbrot_GLUTWrapper_h

#include <functional>

class GLUTWrapper {
public:
    GLUTWrapper(int *argc, char **argv);
    void printDisplayDimentions();
    void init(int, int);
    void run();
    void quit();
    void setWindowTitle(const std::string &);
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
    int winId;
};

#endif
