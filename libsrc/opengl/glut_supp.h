#ifndef GLUT_SUPP_H
#define GLUT_SUPP_H

#include "quip_config.h"
#include <stdio.h>
#include "quip_prot.h"


// This file is called glut_supp.h, but if we comment this out
// then we get no warnings, whereas if we include it we load
// the CUDA file and get a bunch of warnings!?

//#ifdef HAVE_GL_GLUT_H
//#include <GL/glut.h>
//#endif

#ifdef HAVE_GL_GL_H
#include <GL/gl.h>
#else // ! HAVE_GL_GL_H
#ifdef BUILD_FOR_OBJC
#include <OpenGL/gl.h>		// OBJC
#include <GLUT/glut.h>	// OBJC
#endif // BUILD_FOR_OBJC
#endif // ! HAVE_GL_GL_H


#ifdef HAVE_GL_GLU_H
#include <GL/glu.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

// Bool type is pre-defined in g++, but not in regular C???
//typedef int bool;
#define false 0
#define true 1

// Prototypes
extern void display(void);
extern void reshape(int, int);
extern void MouseMotion(int, int);
extern void Mouse(int, int, int, int);
extern void init(void);
extern void keyboard (unsigned char, int, int);

extern void setup_view(void);

/* glx_supp.c */
//extern void swap_buffers(void);
extern void _wait_video_sync(QSP_ARG_DECL  int n);
#define wait_video_sync(n) _wait_video_sync(QSP_ARG  n)

extern
#ifdef BUILD_FOR_MACOS
       GLboolean
#else
       int
#endif
                   check_extension( QSP_ARG_DECL  const char *extension );

#endif /* ! GLUT_SUPP_H */
