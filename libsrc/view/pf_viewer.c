// Based on old cuda_viewer.cpp, this file provides
// a platform-independent linkage between gpu and openGL

// OpenGL appears not to have access to GLEW...
// So the port from Cuda may not be so simple
// as implementing REGBUF_FN etc...

#include "quip_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef BUILD_FOR_OBJC
#ifdef HAVE_GL_GLEW_H
#include <GL/glew.h>
#endif // HAVE_GL_GLEW_H
#else // BUILD_FOR_OBJC
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif // BUILD_FOR_OBJC

#ifdef HAVE_GL_GL_H
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

// used to include GL/glut.h and rendercheck_gl.h...

#ifdef HAVE_GL_GLX_H
#include <GL/glx.h>	// jbm added for glXSwapBuffers()
#endif

#define NO_OGL_MSG	warn("Sorry, no openGL support in this build!?");

#include "quip_prot.h"
#include "gl_viewer.h"		/* select_gl_viewer() */
#include "gl_info.h"

#include "platform.h"
#include "pf_viewer.h"
#include "opengl_utils.h"

static IOS_Item_Type *pf_vwr_itp=NULL;
static IOS_ITEM_INIT_FUNC(Platform_Viewer,pf_vwr,0)
#ifdef BUILD_FOR_OBJC
static IOS_ITEM_CHECK_FUNC(Platform_Viewer,pf_vwr)
#endif // BUILD_FOR_OBJC
static IOS_ITEM_NEW_FUNC(Platform_Viewer,pf_vwr)
static IOS_ITEM_PICK_FUNC(Platform_Viewer,pf_vwr)

#define pick_pf_vwr(p)	_pick_pf_vwr(QSP_ARG  p)
#define new_pf_vwr(s)	_new_pf_vwr(QSP_ARG  s)

static void init_pf_viewer(Platform_Viewer *pvp)
{
#ifdef HAVE_OPENGL
//	pvp->pv_pbo_buffer = 0;
//	pvp->pv_texid = 0;
	SET_PFVWR_BUFFER(pvp,0);
	SET_PFVWR_TEXID(pvp,0);
#endif // HAVE_OPENGL
}

// This is the normal display path
#define update_pf_viewer(pvp, dp) _update_pf_viewer(QSP_ARG  pvp, dp) 

static void _update_pf_viewer(QSP_ARG_DECL  Platform_Viewer *pvp, Data_Obj *dp) 
{
#ifdef HAVE_OPENGL
	int t;
	//cudaError_t e;

	// unmap buffer before using w/ GL
	if( BUF_IS_MAPPED(dp) ){
		if( (*PF_UNMAPBUF_FN(PFDEV_PLATFORM(OBJ_PFDEV(dp))))
				(QSP_ARG  dp) < 0 ) {
			warn("update_pf_viewer:  buffer unmap error!?");
		}
		CLEAR_OBJ_FLAG_BITS(dp, DT_BUF_MAPPED);
		// propagate change to children and parents
		propagate_flag(dp,DT_BUF_MAPPED);

	}

	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, OBJ_TEX_ID(dp));
	// is glBindBuffer REALLY part of libGLEW???
//#ifdef HAVE_LIBGLEW
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, OBJ_BUF_ID(dp));
//#endif // HAVE_LIBGLEW

	t=gl_pixel_type(dp);
	glTexSubImage2D(GL_TEXTURE_2D, 0,	// target, level
		0, 0,				// x0, y0
		OBJ_COLS(dp), OBJ_ROWS(dp), 	// dx, dy
		t,
		GL_UNSIGNED_BYTE,		// type
		OFFSET(0));			// offset into PIXEL_UNPACK_BUFFER

//#ifdef HAVE_LIBGLEW
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
//#endif // HAVE_LIBGLEW

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1.0, -1.0);
	glTexCoord2f(0, 0); glVertex2f(-1.0, 1.0);
	glTexCoord2f(1, 0); glVertex2f(1.0, 1.0);
	glTexCoord2f(1, 1); glVertex2f(1.0, -1.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	if( (*PF_MAPBUF_FN(PFDEV_PLATFORM(OBJ_PFDEV(dp))))(QSP_ARG  dp) < 0 ){
		warn("update_pf_viewer:  Error mapping buffer!?");
	}


	SET_OBJ_FLAG_BITS(dp, DT_BUF_MAPPED);
	// propagate change to children and parents
	propagate_flag(dp,DT_BUF_MAPPED);
#else // ! HAVE_OPENGL
	NO_OGL_MSG
#endif // ! HAVE_OPENGL
}

static int pf_viewer_subsystem_inited=0;

#define init_pf_viewer_subsystem() _init_pf_viewer_subsystem(SINGLE_QSP_ARG)

static void _init_pf_viewer_subsystem(SINGLE_QSP_ARG_DECL)
{
	const char *pn;
	int n;

	pn = tell_progname();

	if( pf_viewer_subsystem_inited ){
		warn("Platform viewer subsystem already initialized!?");
		return;
	}

	// First initialize OpenGL context, so we can properly set
	// the GL for CUDA.
	// This is necessary in order to achieve optimal performance
	// with OpenGL/CUDA interop.

	//glutInit( &argc, argv);		/* BUG?  where should this be done? */
	n=1;
#ifdef HAVE_GLUT
	glutInit( &n, (char **)&pn);		/* BUG?  where should this be done? */

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
#endif // HAVE_GLUT

	pf_viewer_subsystem_inited=1;
}

#define new_pf_viewer(vp) _new_pf_viewer(QSP_ARG  vp)

static Platform_Viewer * _new_pf_viewer(QSP_ARG_DECL  Viewer *vp)
{
	Platform_Viewer *pvp;

	if( !pf_viewer_subsystem_inited ){
		if( verbose )
	advise("new_pf_viewer:  initializing cuda viewer subsys");
		init_pf_viewer_subsystem();
	}

	pvp = new_pf_vwr(VW_NAME(vp));
	if( pvp == NULL ) return(pvp);

#ifndef BUILD_FOR_OBJC
	pvp->pv_vp = vp;
#endif // BUILD_FOR_OBJC
	
	init_pf_viewer(pvp);

	return(pvp);
}

COMMAND_FUNC( do_new_pf_vwr )
{
	Viewer *vp;


	vp = pick_vwr("name of existing viewer to use with Cuda/OpenCL");
	if( vp == NULL ) return;

	if( ! READY_FOR_GLX(vp) ) {
		snprintf(ERROR_STRING,LLEN,"do_new_pf_vwr:  Existing viewer %s must be initialized for GL before using!?",VW_NAME(vp) );
		warn(ERROR_STRING);
		return;
	}

#ifdef HAVE_OPENGL
	glew_check(SINGLE_QSP_ARG);	/* without this, we get a segmentation violation on glGenBuffers??? */
#endif // HAVE_OPENGL

	if( new_pf_viewer(vp) == NULL ){
		snprintf(ERROR_STRING,LLEN,"Error making %s a cuda viewer!?",VW_NAME(vp));
		warn(ERROR_STRING);
	}
} // do_new_pf_viewer

#ifndef BUILD_FOR_IOS
COMMAND_FUNC( do_load_pf_vwr )
{
	Platform_Viewer *pvp;
	Data_Obj *dp;

	pvp = pick_pf_vwr("platform viewer");
	dp = pick_obj("GL buffer object");

	if( pvp == NULL || dp == NULL ) return;

#ifdef HAVE_OPENGL
//fprintf(stderr,"do_load_pf_vwr:  calling select_gl_viewer %s\n",
//VW_NAME(PFVWR_VIEWER(pvp)));
	select_gl_viewer( QSP_ARG  /*pvp->pv_vp*/ PFVWR_VIEWER(pvp) );

	if( ! IS_GL_BUFFER(dp) ){
		snprintf(ERROR_STRING,LLEN,"Object %s is not a GL buffer object.",OBJ_NAME(dp));
		warn(ERROR_STRING);
		return;
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, OBJ_TEX_ID(dp) );
#else // ! HAVE_OPENGL
	warn("do_load_pf_viewer:  Sorry, no OpenGL support in this build!?");
#endif // ! HAVE_OPENGL
		
	update_pf_viewer(pvp,dp);
}
#endif // BUILD_FOR_IOS

#ifdef BUILD_FOR_OBJC

@implementation Platform_Viewer

@synthesize pv_vp;
#ifdef HAVE_OPENGL
@synthesize pv_pbo_buffer;
@synthesize pv_texid;
#endif // HAVE_OPENGL

+(void) initClass
{
	pf_vwr_itp = new_ios_item_type(DEFAULT_QSP_ARG  "Platform_Viewer");
}

@end

#endif // BUILD_FOR_OBJC

