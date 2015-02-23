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
    glutInit (argc, argv);
}

void GLUTWrapper::init(int width, int height) {
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    winId = glutCreateWindow("Window");
    glutDisplayFunc(GLUTWrapper::display);
    glutReshapeFunc(GLUTWrapper::reshape);
    
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
}