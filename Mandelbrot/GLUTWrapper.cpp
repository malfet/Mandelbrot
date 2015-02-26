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

#include <GLUT/glut.h>
#include <iostream>
#include <assert.h>
#include "GLUTWrapper.h"

GLUTWrapper *GLUTWrapper::self = NULL;

void GLUTWrapper::printDisplayDimentions(void)
{
    auto displayWidth	= glutGet(GLUT_SCREEN_WIDTH);
    auto displayHeight	= glutGet(GLUT_SCREEN_HEIGHT);
    std::cout <<"default display dimentions are "<<displayWidth<<"x"<<displayHeight<<std::endl;
}

GLUTWrapper::GLUTWrapper(int *argc, char **argv) {
    assert (self == NULL);
    self = this;
    displayFunc = [] {};
    reshapeFunc = [](int w,int h) {};
    mouseFunc = [](int x, int y, unsigned b) {};
    glutInit (argc, argv);
}

void GLUTWrapper::init(int width, int height) {
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    winId = glutCreateWindow("Window");
    glutDisplayFunc(GLUTWrapper::display);
    glutReshapeFunc(GLUTWrapper::reshape);
    glutMouseFunc(GLUTWrapper::mouse);
    glutMotionFunc(GLUTWrapper::motion);
}

void GLUTWrapper::run()
{
    glutMainLoop();
}


void GLUTWrapper::setWindowTitle(const std::string &s)
{
    glutSetWindowTitle(s.c_str());
}

void GLUTWrapper::redisplay()
{
    glutPostRedisplay();
}

void GLUTWrapper::display()
{
    self->displayFunc();
    glutSwapBuffers();
}

void GLUTWrapper::quit()
{
    glutDestroyWindow(winId);
    exit(0);
}


void GLUTWrapper::mouse(int b, int s, int x, int y)
{
    if (b == GLUT_LEFT_BUTTON && s == GLUT_UP) self->mouseButtons &= ~1;
    if (b == GLUT_LEFT_BUTTON && s == GLUT_DOWN) self->mouseButtons |= 1;
    if (b == GLUT_RIGHT_BUTTON && s == GLUT_DOWN) self->mouseButtons |= 2;
    if (b == GLUT_RIGHT_BUTTON && s == GLUT_UP) self->mouseButtons &= ~2;

    self->mouseFunc(x,y, self->mouseButtons);
}

void GLUTWrapper::motion(int x, int y)
{
    self->mouseFunc(x,y, self->mouseButtons);
}