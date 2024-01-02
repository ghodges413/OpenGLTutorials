/*
 *  Graphics.h
 *
 */
#pragma once

#ifdef WINDOWS
#include <GL/glew.h>
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#endif

#ifndef GL_RGB32F
#define GL_RGB32F   GL_RGB32F_ARB
#endif

#ifndef GL_RGB16F
#define GL_RGB16F   GL_RGB16F_ARB
#endif

#ifndef glFramebufferTexture
#define glFramebufferTexture    glFramebufferTextureEXT
#endif

void myglGetError();
void myglClearErrors();

template < class T >
void Swap( T & a, T & b ) {
	T tmp = b;
	b = a;
	a = tmp;
}
