#include "quip_config.h"

#include "quip_prot.h"
#include "viewer.h"
#include "xsupp.h"
#include "debug.h"

static Disp_Obj *current_dop=NULL;

ITEM_INTERFACE_DECLARATIONS(Disp_Obj,disp_obj,0)

#ifdef HAVE_X11

#include <X11/Xproto.h>	// the request codes
#include "xsupp_prot.h"

/* manipulate displays */

#ifdef HAVE_STDLIB_H
#include <stdlib.h>	/* atoi() */
#endif

static int window_sys_inited=0;

// BUG static vars not thread-safe...
// Also, should be one per display...
static XVisualInfo *	visualList = NULL;

#if defined(__cplusplus) || defined(c_plusplus)
#define xvi_class	c_class
#else
#define xvi_class	class
#endif

#ifdef THREAD_SAFE_QUERY
static Query_Stack *x_error_qsp=NULL;
#define SPECIFIED_QSP_ARG(p)	p,
#else // ! THREAD_SAFE_QUERY
#define SPECIFIED_QSP_ARG(p)
#endif // ! THREAD_SAFE_QUERY

void set_display( Disp_Obj *dop )
{
	current_dop = dop;
}

List *displays_list(SINGLE_QSP_ARG_DECL)
{
	if( disp_obj_itp == NULL ) return(NULL);
	return( item_list(disp_obj_itp) );
}

void info_do( Disp_Obj *dop )
{
	printf("Display %s:\n",DO_NAME(dop));
	printf("\tdpy = 0x%lx\n",(u_long)DO_DISPLAY(dop));
	printf("\tvis = 0x%lx\n",(u_long)DO_VISUAL(dop));
	printf("\tgc  = 0x%lx\n",(u_long)DO_GC(dop));
	printf("\tscreen = %d\n",DO_SCREEN(dop));
	printf("\trootw = %ld (0x%lx)\n",DO_ROOTW(dop),DO_ROOTW(dop));
	printf("\twidth = %d\n",DO_WIDTH(dop));
	printf("\theight = %d\n",DO_HEIGHT(dop));
	printf("\tdepth = %d\n",DO_DEPTH(dop));
}


#define dop_open(dop) _dop_open(QSP_ARG  dop)

static int _dop_open( QSP_ARG_DECL  Disp_Obj *dop )
{
	/* BUG - on Solaris, when we have DISPLAY set to :0,
	 * but are on a remote server, this call hangs...
	 * (e.g., run iview on vision from xterm on stiles,
	 * w/ DISPLAY erroneously set to :0 instead of stiles:0)
	 * We ought to put a watchdog timer here...
	 */

fprintf(stderr,"Calling XOpenDisplay...\n");
	if ( (SET_DO_DISPLAY(dop,XOpenDisplay(DO_NAME(dop)))) == NULL) {
		sprintf(ERROR_STRING,
			"dop_open:  Can't open display \"%s\"\n",DO_NAME(dop));
		warn(ERROR_STRING);
		/* remove the object */
		del_disp_obj(dop);
		return(-1);
	}
	return(0);
}

#define get_vis_list(dop,np) _get_vis_list(QSP_ARG  dop,np)

static XVisualInfo *_get_vis_list( QSP_ARG_DECL  Disp_Obj * dop, int *np )
{
	XVisualInfo		vTemplate;

	vTemplate.screen=DO_SCREEN(dop);

	if( visualList != NULL ) XFree(visualList);
	visualList = XGetVisualInfo(DO_DISPLAY(dop),VisualScreenMask,
		&vTemplate,np);

#ifdef QUIP_DEBUG
if( debug & xdebug ){
sprintf(ERROR_STRING,"%d visuals found",*np);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */

	return(visualList);
}

#define get_depth_list(dop,depth,np) _get_depth_list( QSP_ARG  dop,depth,np)

static XVisualInfo *_get_depth_list( QSP_ARG_DECL  Disp_Obj * dop, int depth, int *np )
{
	XVisualInfo		vTemplate;

	/* try to get an 8 bit psuedocolor visual */
	/* taken from Xlib prog manual p. 215 */

	vTemplate.depth=depth;
	vTemplate.screen=DO_SCREEN(dop);

	if( visualList != NULL ) XFree(visualList);
	visualList = XGetVisualInfo(DO_DISPLAY(dop),VisualScreenMask|VisualDepthMask,
		&vTemplate,np);
	if( visualList == NULL ){
		sprintf(ERROR_STRING,
			"get_depth_list(%d) got NULL from XGetVisualInfo!?",depth);
		warn(ERROR_STRING);
	}
#ifdef QUIP_DEBUG
if( debug & xdebug ){
sprintf(ERROR_STRING,"%d visuals found with depth %d",*np,depth);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */

	return(visualList);
}

#ifndef HAVE_OPENGL
static int find_visual_by_id( XVisualInfo *list, int n,unsigned int id )
{
	int i;

	/* find one which of the matching class */
	for(i=0;i<n;i++)
		if( list[i].visualid == id  ) return(i);
	return(-1);
}
#endif // ! HAVE_OPENGL

static int find_visual( XVisualInfo *list, int n, int cl, int depth )
{
	int i;

	/* find one which of the matching class */
	for(i=0;i<n;i++)
		if( list[i].xvi_class == cl  && list[i].depth >= depth) return(i);
	return(-1);
}

#define GetEightBitVisual(dop) _GetEightBitVisual(QSP_ARG  dop)

static Visual *_GetEightBitVisual( QSP_ARG_DECL  Disp_Obj * dop)
{
	int			visualsMatched=0;
	Visual *vis;
	XVisualInfo *vip;
	int i;

	vip = get_depth_list(dop,8,&visualsMatched);

	if( visualsMatched == 0 ) return(NULL);

	i=find_visual(vip,visualsMatched,PseudoColor,8);
	if( i < 0 ){
		warn("no pseudocolor visual found!?");
		return(vip[0].visual);
	}
#ifdef QUIP_DEBUG
if( debug & xdebug ){
sprintf(ERROR_STRING,"using visual %ld",vip[i].visualid);
advise(ERROR_STRING);
vis=DefaultVisual(DO_DISPLAY(dop),DO_SCREEN(dop));
sprintf(ERROR_STRING,"default visual is %ld",vis->visualid);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */

	return(vip[i].visual);
}

// According to the documentation, TrueColor has a fixed read-only colormap,
// while DirectColor has a programmable colormap...

#define PREFERRED_MODE TrueColor
#define ALTERNATE_MODE DirectColor
#define PREFERRED_NAME "TrueColor"
#define ALTERNATE_NAME "DirectColor"


#define GetSpecifiedVisual(dop,depth) _GetSpecifiedVisual(QSP_ARG  dop,depth)

static Visual *_GetSpecifiedVisual(QSP_ARG_DECL  Disp_Obj * dop, int depth )
{
#ifdef HAVE_OPENGL

	XVisualInfo *vi_p;
	Visual *vis_p;
fprintf(stderr,"GetSpecifiedVisual:  requested depth = %d\n",depth);
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, depth, GLX_DOUBLEBUFFER, None };
	vi_p = glXChooseVisual(DO_DISPLAY(dop),0,att);
	if( vi_p == NULL ){
		error1("glXChooseVisual failed!?");
	}
//fprintf(stderr,"glXChooseVisual returned visual %p\n",(void *)vi_p->visualid);
	vis_p = vi_p->visual;
	XFree(vi_p);

	return vis_p; 

#else // ! HAVE_OPENGL

	XVisualInfo *	vip;
	int		visualsMatched;
	int i;
	const char *	name;

	vip = get_depth_list(dop,depth,&visualsMatched);
	if( visualsMatched == 0 ) return(NULL);

	/* We need something more here, for openGL we have multiple visuals w/ 24 bit truecolor,
	 * but not all have a Z buffer...
	 * As a hack, we pass the desired visualID through the environment...
	 */
	{
		char *s;

		s=getenv("PREFERRED_VISUAL_ID");
		if( s != NULL ){
			int preferred_id;
sprintf(ERROR_STRING,"Checking for PREFERRED_VISUAL_ID %s",s);
advise(ERROR_STRING);
			preferred_id = atoi(s);	/* BUG decimal only, should parse hex too */
			i=find_visual_by_id(vip,visualsMatched,preferred_id);
			if( i >= 0 ){
sprintf(ERROR_STRING,"preferred visual id %d FOUND at index %d",preferred_id,i);
advise(ERROR_STRING);
				return(vip[i].visual);
			}
			sprintf(ERROR_STRING,"Unable to find requested visual id %d",preferred_id);
			warn(ERROR_STRING);
		}
	}
	i=find_visual(vip,visualsMatched,PREFERRED_MODE,depth);
	if( i < 0 ){
		sprintf(ERROR_STRING,"no %s visual found with depth %d!?",
			PREFERRED_NAME,depth);
		warn(ERROR_STRING);
		i=find_visual(vip,visualsMatched,ALTERNATE_MODE,depth);
		if( i < 0 ){
			sprintf(ERROR_STRING,"no %s visual found with depth %d!?",
				ALTERNATE_NAME,depth);
			warn(ERROR_STRING);
			return(vip[0].visual);
		} else {
			name = ALTERNATE_NAME;
		}
	} else {
		name = PREFERRED_NAME;
	}

//if( verbose ){
sprintf(ERROR_STRING,"i=%d, using visual %ld (%s, depth = %d)",
i, vip[i].visualid,name,depth);
advise(ERROR_STRING);
//}
	return(vip[i].visual);
#endif // ! HAVE_OPENGL
}

#define Get15BitVisual(dop) _Get15BitVisual(QSP_ARG  dop)
#define Get16BitVisual(dop) _Get16BitVisual(QSP_ARG  dop)
#define Get24BitVisual(dop) _Get24BitVisual(QSP_ARG  dop)

static Visual *_Get16BitVisual(QSP_ARG_DECL  Disp_Obj * dop)
{
	return( GetSpecifiedVisual(dop,16) );
}


static Visual *_Get24BitVisual(QSP_ARG_DECL  Disp_Obj * dop)
{
	return( GetSpecifiedVisual(dop,24) );
}

/* powerbook display */

static Visual *_Get15BitVisual(QSP_ARG_DECL  Disp_Obj * dop)
{
	return( GetSpecifiedVisual(dop,15) );
}

#define dop_setup(dop,desired_depth) _dop_setup(QSP_ARG   dop, desired_depth)

static int _dop_setup( QSP_ARG_DECL   Disp_Obj *dop, int desired_depth)
{
	XVisualInfo vinfo, *list;
	int n;

	SET_DO_SCREEN(dop, DefaultScreen(DO_DISPLAY(dop)));
	SET_DO_ROOTW(dop, RootWindow(DO_DISPLAY(dop),DO_SCREEN(dop)));
	SET_DO_CURRW(dop, RootWindow(DO_DISPLAY(dop),DO_SCREEN(dop)));

#ifdef HAVE_OPENGL
// This is per-display instead of per-viewer because it just depends on the visual
// use XGLMakeCurrent to select a window...
	SET_DO_OGL_CTX(dop, NULL);
#endif /* HAVE_OPENGL */

#ifdef QUIP_DEBUG
if( debug & xdebug ){
XWindowAttributes wa;
XGetWindowAttributes(DO_DISPLAY(dop),DO_ROOTW(dop),&wa);
sprintf(ERROR_STRING,"depth of root window = %d", wa.depth);
prt_msg(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
	SET_DO_GC(dop, DefaultGC(DO_DISPLAY(dop),DO_SCREEN(dop)));


	if( verbose ){
sprintf(ERROR_STRING,"desired depth is %d",desired_depth);
advise(ERROR_STRING);
	}

	if( desired_depth == 8 ){
		SET_DO_VISUAL(dop, GetEightBitVisual(dop) );
	} else if( desired_depth == 24 ){
//fprintf(stderr,"calling Get24BitVisual...\n");
		SET_DO_VISUAL(dop, Get24BitVisual(dop) );
//fprintf(stderr,"Using visual 0x%p\n",(void *)DO_VISUAL(dop)->visualid);
	} else if( desired_depth == 16 ){
		SET_DO_VISUAL(dop, Get16BitVisual(dop) );
	} else {
		SET_DO_VISUAL(dop, DefaultVisual(DO_DISPLAY(dop),DO_SCREEN(dop)) );
	}

	if( DO_VISUAL(dop) == 0 ){
		if( verbose )
			advise("initial attempt to get a visual failed");
	}

	/* BUG? can't we do something better here? */
	if( DO_VISUAL(dop) == 0 && desired_depth == 8 ){
		/* this works on powerbook */
		SET_DO_VISUAL(dop, Get15BitVisual(dop) );
	}
	if( DO_VISUAL(dop) == 0 && desired_depth == 8 ){
		/* this works on durer... */
		SET_DO_VISUAL(dop, Get16BitVisual(dop) );
	}
	if( DO_VISUAL(dop) == 0 ){	/* couldn't find anything !? */
		if( verbose ){
			int nvis,i;
			XVisualInfo *	vlp;

			vlp=get_vis_list(dop,&nvis);

			sprintf(ERROR_STRING,"%d visuals found:",nvis);
			advise(ERROR_STRING);

			for(i=0;i<nvis;i++){
				sprintf(ERROR_STRING,"class %d   depth %d",
			vlp[i].xvi_class,vlp[i].depth);
				advise(ERROR_STRING);
			}
		}

		return(-1);
	}

	/* remember the depth of this visual - do we still need to do this? */
	vinfo.visualid = XVisualIDFromVisual(DO_VISUAL(dop));
	list = XGetVisualInfo(DO_DISPLAY(dop),VisualIDMask,&vinfo,&n);

	if( n != 1 ){
		warn("more than one visual with specified ID!?");
		//SET_DO_DEPTH(dop, 8);	// why was this 8???
	} /*else {
		SET_DO_DEPTH(dop, list[0].depth);
	}*/
	SET_DO_DEPTH(dop,list[0].depth);
	XFree(list);

	SET_DO_WIDTH(dop, DisplayWidth(DO_DISPLAY(dop),DO_SCREEN(dop)) );
	SET_DO_HEIGHT(dop, DisplayHeight(DO_DISPLAY(dop),DO_SCREEN(dop)) );

#ifdef QUIP_DEBUG
if( debug & xdebug ){
sprintf(msg_str,"display %s, %d by %d, %d bits deep",
DO_NAME(dop),DO_WIDTH(dop),DO_HEIGHT(dop),DO_DEPTH(dop));
prt_msg(msg_str);
}
#endif /* QUIP_DEBUG */

	return(0);
}

static double get_dpy_size( QSP_ARG_DECL  Item *ip, int index)
{
	Disp_Obj *dop;

	dop = (Disp_Obj *)ip;

	switch( index ){
		case 0: return( (double) DO_DEPTH(dop) );
		case 1: return( (double) DO_WIDTH(dop) );
		case 2: return( (double) DO_HEIGHT(dop) );
		default:
			sprintf(ERROR_STRING,
	"get_dpy_size:  unsupported display size function (index = %d)",index);
			warn(ERROR_STRING);
			return(0.0);
	}
}

static Size_Functions dpy_sf={
	get_dpy_size,
	default_prec_name
};



/*
 * Open the named display
 */

Disp_Obj *_open_display(QSP_ARG_DECL  const char *name,int desired_depth)
{
	Disp_Obj *dop;
	static int siz_done=0;

	dop = new_disp_obj(name);
	if( dop == NULL ){
		sprintf(ERROR_STRING, "Couldn't create object for display %s",
					name);
		warn(ERROR_STRING);
		return(NULL);
	}

	if( dop_open(dop) < 0 ){
		return(NULL);
	}

	if( dop_setup(dop,desired_depth) < 0 ){
		/* Bug - XCloseDisplay?? */
		/* need to destroy object here */
		del_disp_obj(dop);
		return(NULL);
	}
	set_display(dop);

	if( ! siz_done ){
		siz_done++;
		add_sizable(disp_obj_itp,&dpy_sf, NULL );
	}

	return(dop);
}

#define check_for_desired_depth(dname) _check_for_desired_depth(QSP_ARG  dname)

static Disp_Obj *_check_for_desired_depth(QSP_ARG_DECL  const char *dname)
{
	const char *s;
	Disp_Obj *dop;
	int desired_depth;

	s=getenv("DESIRED_DEPTH");
	if( s == NULL ) return NULL;

	desired_depth=atoi(s);

	dop = open_display(dname,desired_depth);
	if( dop == NULL ){
		sprintf(ERROR_STRING,"Unable to open display %s with $DESIRED_DEPTH (%d)",
			dname,desired_depth);
		warn(ERROR_STRING);
	}
	return(dop);
}

#define MAX_DISPLAY_DEPTHS	4
static int possible_depths[MAX_DISPLAY_DEPTHS]={24,8,16,15};

#define check_possible_depth(d,dname) _check_possible_depth(QSP_ARG  d, dname)

static Disp_Obj *_check_possible_depth(QSP_ARG_DECL  int d, const char *dname)
{
	Disp_Obj *dop;

	dop = open_display(dname,d);
	if( dop != NULL && verbose ){
		sprintf(ERROR_STRING,
			"Using depth %d on display %s",
			d,dname);
		advise(ERROR_STRING);
	}
	return(dop);
}

#define check_possible_depths(dname) _check_possible_depths(QSP_ARG  dname)

static Disp_Obj *_check_possible_depths(QSP_ARG_DECL  const char *dname)
{
	Disp_Obj *dop;
	int i;

	for(i=0;i<MAX_DISPLAY_DEPTHS;i++){
		dop=check_possible_depth(possible_depths[i], dname);
		if( dop != NULL ) return dop;

		if( verbose ){
			sprintf(ERROR_STRING,
				"Couldn't get %d bit visual on device %s",
				possible_depths[i],dname);
			advise(ERROR_STRING);
		}
	}
	return NULL;
}

static Disp_Obj * default_x_display(SINGLE_QSP_ARG_DECL)
{
	const char *dname;
	Disp_Obj *dop;

	dname = check_display();

	/* these two lines added so this can be called more than once */
	dop = disp_obj_of(dname);
	if( dop != NULL ) return(dop);

	dop = check_for_desired_depth(dname);
	if( dop != NULL ) return dop;

	dop = check_possible_depths(dname);
	return dop;
}
/* end default_x_display */

static char *x_request_name[128];

// normal error codes go from 0 to 17 (see X.h)

#define N_X11_ERROR_CODES	18
static char *x_error_description[N_X11_ERROR_CODES];


#define INIT_NAME_ENTRY(code)	x_request_name[code] = #code
#define INIT_ERROR_ENTRY(code)	{ assert(code<N_X11_ERROR_CODES); x_error_description[code] = #code; }

static void init_error_names(void)
{
	bzero( x_request_name, 128 * sizeof(char *) );
	// don't bother to zero error names because there are no gaps

	INIT_NAME_ENTRY( X_CreateWindow );
	INIT_NAME_ENTRY( X_ChangeWindowAttributes );
	INIT_NAME_ENTRY( X_GetWindowAttributes );
	INIT_NAME_ENTRY( X_DestroyWindow );
	INIT_NAME_ENTRY( X_DestroySubwindows );
	INIT_NAME_ENTRY( X_ChangeSaveSet );
	INIT_NAME_ENTRY( X_ReparentWindow );
	INIT_NAME_ENTRY( X_MapWindow );
	INIT_NAME_ENTRY( X_MapSubwindows );
	INIT_NAME_ENTRY( X_UnmapWindow );
	INIT_NAME_ENTRY( X_UnmapSubwindows );
	INIT_NAME_ENTRY( X_ConfigureWindow );
	INIT_NAME_ENTRY( X_CirculateWindow );
	INIT_NAME_ENTRY( X_GetGeometry );
	INIT_NAME_ENTRY( X_QueryTree );
	INIT_NAME_ENTRY( X_InternAtom );
	INIT_NAME_ENTRY( X_GetAtomName );
	INIT_NAME_ENTRY( X_ChangeProperty );
	INIT_NAME_ENTRY( X_DeleteProperty );
	INIT_NAME_ENTRY( X_GetProperty );
	INIT_NAME_ENTRY( X_ListProperties );
	INIT_NAME_ENTRY( X_SetSelectionOwner );
	INIT_NAME_ENTRY( X_GetSelectionOwner );
	INIT_NAME_ENTRY( X_ConvertSelection );
	INIT_NAME_ENTRY( X_SendEvent );
	INIT_NAME_ENTRY( X_GrabPointer );
	INIT_NAME_ENTRY( X_UngrabPointer );
	INIT_NAME_ENTRY( X_GrabButton );
	INIT_NAME_ENTRY( X_UngrabButton );
	INIT_NAME_ENTRY( X_ChangeActivePointerGrab );
	INIT_NAME_ENTRY( X_GrabKeyboard );
	INIT_NAME_ENTRY( X_UngrabKeyboard );
	INIT_NAME_ENTRY( X_GrabKey );
	INIT_NAME_ENTRY( X_UngrabKey );
	INIT_NAME_ENTRY( X_AllowEvents );
	INIT_NAME_ENTRY( X_GrabServer );
	INIT_NAME_ENTRY( X_UngrabServer );
	INIT_NAME_ENTRY( X_QueryPointer );
	INIT_NAME_ENTRY( X_GetMotionEvents );
	INIT_NAME_ENTRY( X_TranslateCoords );
	INIT_NAME_ENTRY( X_WarpPointer );
	INIT_NAME_ENTRY( X_SetInputFocus );
	INIT_NAME_ENTRY( X_GetInputFocus );
	INIT_NAME_ENTRY( X_QueryKeymap );
	INIT_NAME_ENTRY( X_OpenFont );
	INIT_NAME_ENTRY( X_CloseFont );
	INIT_NAME_ENTRY( X_QueryFont );
	INIT_NAME_ENTRY( X_QueryTextExtents );
	INIT_NAME_ENTRY( X_ListFonts );
	INIT_NAME_ENTRY( X_ListFontsWithInfo );
	INIT_NAME_ENTRY( X_SetFontPath );
	INIT_NAME_ENTRY( X_GetFontPath );
	INIT_NAME_ENTRY( X_CreatePixmap );
	INIT_NAME_ENTRY( X_FreePixmap );
	INIT_NAME_ENTRY( X_CreateGC );
	INIT_NAME_ENTRY( X_ChangeGC );
	INIT_NAME_ENTRY( X_CopyGC );
	INIT_NAME_ENTRY( X_SetDashes );
	INIT_NAME_ENTRY( X_SetClipRectangles );
	INIT_NAME_ENTRY( X_FreeGC );
	INIT_NAME_ENTRY( X_ClearArea );
	INIT_NAME_ENTRY( X_CopyArea );
	INIT_NAME_ENTRY( X_CopyPlane );
	INIT_NAME_ENTRY( X_PolyPoint );
	INIT_NAME_ENTRY( X_PolyLine );
	INIT_NAME_ENTRY( X_PolySegment );
	INIT_NAME_ENTRY( X_PolyRectangle );
	INIT_NAME_ENTRY( X_PolyArc );
	INIT_NAME_ENTRY( X_FillPoly );
	INIT_NAME_ENTRY( X_PolyFillRectangle );
	INIT_NAME_ENTRY( X_PolyFillArc );
	INIT_NAME_ENTRY( X_PutImage );
	INIT_NAME_ENTRY( X_GetImage );
	INIT_NAME_ENTRY( X_PolyText8 );
	INIT_NAME_ENTRY( X_PolyText16 );
	INIT_NAME_ENTRY( X_ImageText8 );
	INIT_NAME_ENTRY( X_ImageText16 );
	INIT_NAME_ENTRY( X_CreateColormap );
	INIT_NAME_ENTRY( X_FreeColormap );
	INIT_NAME_ENTRY( X_CopyColormapAndFree );
	INIT_NAME_ENTRY( X_InstallColormap );
	INIT_NAME_ENTRY( X_UninstallColormap );
	INIT_NAME_ENTRY( X_ListInstalledColormaps );
	INIT_NAME_ENTRY( X_AllocColor );
	INIT_NAME_ENTRY( X_AllocNamedColor );
	INIT_NAME_ENTRY( X_AllocColorCells );
	INIT_NAME_ENTRY( X_AllocColorPlanes );
	INIT_NAME_ENTRY( X_FreeColors );
	INIT_NAME_ENTRY( X_StoreColors );
	INIT_NAME_ENTRY( X_StoreNamedColor );
	INIT_NAME_ENTRY( X_QueryColors );
	INIT_NAME_ENTRY( X_LookupColor );
	INIT_NAME_ENTRY( X_CreateCursor );
	INIT_NAME_ENTRY( X_CreateGlyphCursor );
	INIT_NAME_ENTRY( X_FreeCursor );
	INIT_NAME_ENTRY( X_RecolorCursor );
	INIT_NAME_ENTRY( X_QueryBestSize );
	INIT_NAME_ENTRY( X_QueryExtension );
	INIT_NAME_ENTRY( X_ListExtensions );
	INIT_NAME_ENTRY( X_ChangeKeyboardMapping );
	INIT_NAME_ENTRY( X_GetKeyboardMapping );
	INIT_NAME_ENTRY( X_ChangeKeyboardControl );
	INIT_NAME_ENTRY( X_GetKeyboardControl );
	INIT_NAME_ENTRY( X_Bell );
	INIT_NAME_ENTRY( X_ChangePointerControl );
	INIT_NAME_ENTRY( X_GetPointerControl );
	INIT_NAME_ENTRY( X_SetScreenSaver );
	INIT_NAME_ENTRY( X_GetScreenSaver );
	INIT_NAME_ENTRY( X_ChangeHosts );
	INIT_NAME_ENTRY( X_ListHosts );
	INIT_NAME_ENTRY( X_SetAccessControl );
	INIT_NAME_ENTRY( X_SetCloseDownMode );
	INIT_NAME_ENTRY( X_KillClient );
	INIT_NAME_ENTRY( X_RotateProperties );
	INIT_NAME_ENTRY( X_ForceScreenSaver );
	INIT_NAME_ENTRY( X_SetPointerMapping );
	INIT_NAME_ENTRY( X_GetPointerMapping );
	INIT_NAME_ENTRY( X_SetModifierMapping );
	INIT_NAME_ENTRY( X_GetModifierMapping );
	INIT_NAME_ENTRY( X_NoOperation );

	INIT_ERROR_ENTRY( Success );
	INIT_ERROR_ENTRY( BadRequest );
	INIT_ERROR_ENTRY( BadValue );
	INIT_ERROR_ENTRY( BadWindow );
	INIT_ERROR_ENTRY( BadPixmap );
	INIT_ERROR_ENTRY( BadAtom );
	INIT_ERROR_ENTRY( BadCursor );
	INIT_ERROR_ENTRY( BadFont );
	INIT_ERROR_ENTRY( BadMatch );
	INIT_ERROR_ENTRY( BadDrawable );
	INIT_ERROR_ENTRY( BadAccess );
	INIT_ERROR_ENTRY( BadAlloc );
	INIT_ERROR_ENTRY( BadColor );
	INIT_ERROR_ENTRY( BadGC );
	INIT_ERROR_ENTRY( BadIDChoice );
	INIT_ERROR_ENTRY( BadName );
	INIT_ERROR_ENTRY( BadLength );
	INIT_ERROR_ENTRY( BadImplementation );

}

#define identify_x11_request(code) _identify_x11_request(QSP_ARG  code)

static void _identify_x11_request( QSP_ARG_DECL  unsigned char code )
{
	const char *s;

	//assert( code < 128 );	// BUG use symbolic constant
	if( code >= 128 ){
		sprintf(ERROR_STRING,
	"identify_x11_request:  code %d out of range!?",code);
		advise(ERROR_STRING);
		return;
	}
	s = x_request_name[code];
	if( s == NULL )
		error1("identify_x11_request:  undefined code!?");
	sprintf(ERROR_STRING,"X11 error in %s:  ",s);
	advise(ERROR_STRING);
}

#define identify_x11_error(code) _identify_x11_error( QSP_ARG  code )

static void _identify_x11_error( QSP_ARG_DECL  unsigned char code )
{
	if( code >= N_X11_ERROR_CODES ){
		sprintf(ERROR_STRING,"Unexpected X11 error code %d!?",code);
		error1(ERROR_STRING);
	}
	sprintf(ERROR_STRING,"%s",x_error_description[code]);
	advise(ERROR_STRING);
}

static int quip_x_error_handler(Display *dpy_p, XErrorEvent *event_p)
{
	_identify_x11_request( SPECIFIED_QSP_ARG(x_error_qsp)  event_p->request_code );
	_identify_x11_error( SPECIFIED_QSP_ARG(x_error_qsp)  event_p->error_code );
	return -1;
}

void window_sys_init(SINGLE_QSP_ARG_DECL)
{
	char s[8];
	Variable *vp;
	int (*old_handler)(Display *,XErrorEvent*);

	if( window_sys_inited ) return;

#ifdef QUIP_DEBUG
	xdebug = add_debug_module("xsupp");
#endif /* QUIP_DEBUG */

//int (*XSetErrorHandler(handler))()
     // int (*handler)(Display *, XErrorEvent *)
	init_error_names();
#ifdef THREAD_SAFE_QUERY
	x_error_qsp = THIS_QSP;
#endif // THREAD_SAFE_QUERY
     	old_handler = XSetErrorHandler(quip_x_error_handler);
	if( verbose )
		fprintf(stderr,"XSetErrorHandler returned 0x%lx\n",(long)old_handler);


	add_event_func(i_loop);
	set_discard_func( discard_events );

	window_sys_inited=1;

	if( current_dop == NULL ){
		current_dop = default_x_display(SINGLE_QSP_ARG);
		if( current_dop == NULL ){
			warn("Couldn't open default display!?");
			return;
		}
	}
	// Make sure DISPLAY_WIDTH and DISPLAY_HEIGHT are set...
	// If these have been set in the environment, leave be.
	vp = var_of("DISPLAY_WIDTH");
	if( vp == NULL ){
		sprintf(s,"%d",DO_WIDTH(current_dop));
		assign_reserved_var("DISPLAY_WIDTH",s);
	}
	vp = var_of("DISPLAY_HEIGHT");
	if( vp == NULL ){
		sprintf(s,"%d",DO_HEIGHT(current_dop));
		assign_reserved_var("DISPLAY_HEIGHT",s);
	}

	init_reserved_vars();	// view_xpos etc.

	//window_sys_inited=1;
}

#define DEFAULT_DISPLAY_DEPTH	24
#define ALTERNATE_DISPLAY_DEPTH	8

int _display_depth(SINGLE_QSP_ARG_DECL)
{
	if( current_dop == NULL )
		current_dop = default_x_display(SINGLE_QSP_ARG);

	if( current_dop == NULL )
		return(0);

	return( DO_DEPTH(current_dop) );
}

void _show_visuals(QSP_ARG_DECL  Disp_Obj *dop )
{
	int nvis,i;
	XVisualInfo *	vlp;

	vlp=get_vis_list(dop,&nvis);

	sprintf(ERROR_STRING,"%d visuals found:",nvis);
	advise(ERROR_STRING);

	for(i=0;i<nvis;i++){
		sprintf(ERROR_STRING,"id %p    screen %d   class %d   depth %d    masks %ld %ld %ld   cmap siz %d   bits_per_rgb %d",
			(void *)vlp[i].visualid,
			vlp[i].screen,
			vlp[i].xvi_class,
			vlp[i].depth,
			vlp[i].red_mask,
			vlp[i].green_mask,
			vlp[i].blue_mask,
			vlp[i].colormap_size,
			vlp[i].bits_per_rgb
			);
		advise(ERROR_STRING);
	}
}

#else /* !HAVE_X11 */

/* dummy functions to allow linking w/o X11 */

int _display_depth(SINGLE_QSP_ARG_DECL)
{
	UNIMP_MSG(display_depth)
	return 0;
}

void window_sys_init(SINGLE_QSP_ARG_DECL)
{ UNIMP_MSG(window_sys_init) }

void _show_visuals(QSP_ARG_DECL  Disp_Obj *dop )
{ UNIMP_MSG(show_visuals) }

#endif /* !HAVE_X11 */

Disp_Obj *curr_dop(void)
{ return(current_dop); }

