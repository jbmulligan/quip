#include "quip_config.h"

/* plot subroutines using pixrect library */
#include <stdio.h>

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "getbuf.h"
#include "viewer.h"
#include "debug.h"
#include "data_obj.h"
#include "view_cmds.h"
#include "view_util.h"

#include "get_viewer.h"

#define std_type	float

static std_type sx1,sx2,sy1,sy2;

static int lat_long_hack=0;	/* BUG for plotting the globe we don't want to wrap
				 * around, but now we have no way to set this...
				 * Need a better solution for plotting the globe.
				 */


#define XYPLOT_LOOP( this_type, init_statements, loop_statements )	\
									\
	init_statements;						\
									\
	for(k=0;k<OBJ_FRAMES(dp);k++){					\
		for(i=0;i<OBJ_ROWS(dp);i++){				\
			p = (this_type *) OBJ_DATA_PTR(dp);		\
			p += i*OBJ_ROW_INC(dp) + k*OBJ_FRM_INC(dp);	\
			for(j=0;j<OBJ_COLS(dp);j++){			\
									\
				loop_statements;			\
									\
				p += OBJ_PXL_INC(dp);			\
									\
				if( j==0 ) xplot_fmove((float)x,(float)y);	\
									\
			/* remove the "else" here so that		\
			 * column vectors will draw as single points	\
			 */						\
									\
				xplot_fcont((float)x,(float)y);		\
			}						\
		}							\
	}

static COMMAND_FUNC( do_xp_arc )
{
	std_type cx,cy,x1,y1,x2,y2;

	cx=(std_type)how_much("center x");
	cy=(std_type)how_much("center y");
	x1=(std_type)how_much("arc start x");
	y1=(std_type)how_much("arc start y");
	x2=(std_type)how_much("arc end x");
	y2=(std_type)how_much("arc end y");
	xplot_farc(cx,cy,x1,y1,x2,y2);
}

static COMMAND_FUNC( do_xp_fill_arc )
{
	std_type cx,cy,x1,y1,x2,y2;

	cx=(std_type)how_much("center x");
	cy=(std_type)how_much("center y");
	x1=(std_type)how_much("arc start x");
	y1=(std_type)how_much("arc start y");
	x2=(std_type)how_much("arc end x");
	y2=(std_type)how_much("arc end y");
	xplot_ffill_arc(cx,cy,x1,y1,x2,y2);
}

static COMMAND_FUNC( do_xp_fill_polygon )
{
	float* x_vals=NULL, *y_vals = NULL;
	int num_points;
	int i;

	num_points = (int)how_many("number of polygon points");
	x_vals = (float *) getbuf(sizeof(float) * num_points);
	y_vals = (float *) getbuf(sizeof(float) * num_points);
	
	for (i=0; i < num_points; i++) {
		char s[100];
		sprintf(s, "point %d x value", i+1);
		x_vals[i] = (float)how_much(s);
		sprintf(s, "point %d y value", i+1);
		y_vals[i] = (float)how_much(s);
	}

	xplot_fill_polygon(num_points, x_vals, y_vals);

	givbuf(x_vals);
	givbuf(y_vals);
}

static COMMAND_FUNC( do_xp_space )
{
	sx1=(std_type)how_much("minimum x coord");
	sy1=(std_type)how_much("minimum y coord");
	sx2=(std_type)how_much("maximum x coord");
	sy2=(std_type)how_much("maximum y coord");

	/* There is no real reason to require this, except that
	 * we assumed it must have been a mistake if the user did this...
	 * But we often deal with situations where the y coordinate needs
	 * to be turned upside down to make various conventions mesh...
	 */
	/*
	if( sx2 <= sx1 ){
	warn("do_xp_space:  x max coord must be larger than x min coord");
		return;
	}
	if( sy2 <= sy1 ){
	warn("do_xp_space:  y max coord must be larger than y min coord");
		return;
	}
	*/
	if( sx2 == sx1 ){
	warn("do_xp_space:  x max coord must be different from x min coord");
		return;
	}
	if( sy2 == sy1 ){
	warn("do_xp_space:  y max coord must be different from y min coord");
		return;
	}

	xplot_fspace(sx1,sy1,sx2,sy2);
}

static COMMAND_FUNC( do_xp_move )
{
	std_type x,y;

	x=(std_type)how_much("x coord");
	y=(std_type)how_much("y coord");
	xplot_fmove(x,y);
}

static COMMAND_FUNC( do_xp_cont )
{
	std_type x,y;

	x=(std_type)how_much("x coord");
	y=(std_type)how_much("y coord");
	xplot_fcont(x,y);
}

static COMMAND_FUNC( do_xp_point )
{
	std_type x,y;

	x=(std_type)how_much("x coord");
	y=(std_type)how_much("y coord");
	xplot_fpoint(x,y);
}

static COMMAND_FUNC( do_xp_line )
{
	std_type x1,x2,y1,y2;

	x1=(std_type)how_much("first x coord");
	y1=(std_type)how_much("first y coord");
	x2=(std_type)how_much("second x coord");
	y2=(std_type)how_much("second y coord");
	xplot_fline(x1,y1,x2,y2);
}

static COMMAND_FUNC( do_xp_select )
{
	int color;
	
	color=(int)how_many("color index");
	xplot_select(color);
}

static COMMAND_FUNC( do_rdplot )
{
	FILE *fp;

	fp=try_open( nameof("filename"), "r") ;
	if( !fp ) return;
	rdplot(fp);
}

static COMMAND_FUNC( do_xp_erase )
{
	xplot_erase();
}

static COMMAND_FUNC( do_dumpdraw )
{
	Viewer *vp;

	/* BUG dump drawlist is implemented in xsupp (where the draw
	 * commands are remembered for the redraw function), but
	 * plot_vp is static to view/xplot.c ...
	 *
	 * the user shouldn't have to specify the viewer twice,
	 * or this command should be in a different menu.
	 */

	GET_VIEWER("do_dumpdraw")
	if( vp == NULL ) return;

	dump_drawlist(vp);
}

static int bad_plot_vec2(QSP_ARG_DECL Data_Obj *dp,dimension_t n_comps_expected,const char *funcname)
{
	if( dp==NULL ) return 1;

	if( OBJ_PREC(dp) != PREC_SP && OBJ_PREC(dp) != PREC_DP ){
		sprintf(ERROR_STRING,
			"%s:  data vector %s (%s) should have float or double precision",
			funcname,
			OBJ_NAME(dp),OBJ_MACH_PREC_NAME(dp));
		warn(ERROR_STRING);
		return 1;
	}
	if( OBJ_COMPS(dp) != n_comps_expected ){
		sprintf(ERROR_STRING,"%s:  data vector %s (%d) should have %d components",
			funcname,OBJ_NAME(dp),OBJ_COMPS(dp),n_comps_expected);
		warn(ERROR_STRING);
		return 1;
	}
	return 0;
}

static int bad_plot_vec(QSP_ARG_DECL Data_Obj *dp,dimension_t n_comps_expected,const char *funcname)
{
	if( dp==NULL ) return 1;

	if( OBJ_PREC(dp) != PREC_SP ){
		sprintf(ERROR_STRING,
			"%s:  data vector %s (%s) should have float precision",
			funcname,
			OBJ_NAME(dp),OBJ_MACH_PREC_NAME(dp));
		warn(ERROR_STRING);
		return 1;
	}
	if( OBJ_COMPS(dp) != n_comps_expected ){
		sprintf(ERROR_STRING,"%s:  data vector %s (%d) should have %d components",
			funcname,OBJ_NAME(dp),OBJ_COMPS(dp),n_comps_expected);
		warn(ERROR_STRING);
		return 1;
	}
	return 0;
}

#ifdef FOOBAR
static void get_data_params(Data_Obj *dp, u_long *np, long *incp)
{
	if( OBJ_COLS(dp)==1 ){	/* maybe this is a column vector? */
		*np=OBJ_ROWS(dp);
		*incp = OBJ_ROW_INC(dp);
	} else {
		*np=OBJ_COLS(dp);
		*incp = OBJ_PXL_INC(dp);
	}
}
#endif /* FOOBAR */

static COMMAND_FUNC( do_xplot )
{
	Data_Obj *dp;
	u_long i,j,k;

	std_type x,y,y0,dy,*p;

	dp=pick_obj("data vector");

	if( bad_plot_vec(QSP_ARG dp,1,"xplot") ) return;

	INSIST_RAM_OBJ(dp,"xplot")

	dy=1;

	XYPLOT_LOOP( std_type, y0=0, x = (std_type)*p; y = y0; y0+=dy )

}

static COMMAND_FUNC( do_yplot )
{
	Data_Obj *dp;
	u_long i,j,k;
	std_type x,y,dx,*p;
	std_type x0;

	dp=pick_obj("data vector");

	if( bad_plot_vec(QSP_ARG dp,1,"yplot") ) return;

	INSIST_RAM_OBJ(dp,"yplot")

	dx=1;

	XYPLOT_LOOP( std_type, x0=0, y = *p; x = x0; x0+=dx )

}

static COMMAND_FUNC( do_cyplot )
{
	Data_Obj *dp;
	Data_Obj *cdp;
	u_long i, np;		/* number of points */
	long inc;
	long cinc;
	std_type x,y,dx,*yp;
	char *cp;

	dp=pick_obj("data vector");
	cdp=pick_obj("color vector");

	if( dp==NULL ) return;
	if( cdp==NULL ) return;

	INSIST_RAM_OBJ(dp,"cyplot")
	INSIST_RAM_OBJ(cdp,"cyplot")

	if( OBJ_PREC(dp) != PREC_SP ){
		warn("do_cyplot:  data vector should be float");
		return;
	}
	if( OBJ_PREC(cdp) != PREC_BY ){
		warn("color vector should be byte");
		return;
	}
	if( !dp_same_size(dp,cdp,"do_cyplot") ){
		sprintf(ERROR_STRING,"data vector %s and color vector %s must have identical sizes",
			OBJ_NAME(dp),OBJ_NAME(cdp));
		warn(ERROR_STRING);
		return;
	}

	if( OBJ_COLS(dp)==1 ){	/* maybe this is a column vector? */
		np=OBJ_ROWS(dp);
		inc = OBJ_ROW_INC(dp);
		cinc = OBJ_ROW_INC(cdp);
	} else {
		np=OBJ_COLS(dp);
		inc = OBJ_PXL_INC(dp);
		cinc = OBJ_PXL_INC(cdp);
	}


	x=0;
	dx=1;
	i=np-1;
	yp = (std_type *) OBJ_DATA_PTR(dp);
	cp = (char *) OBJ_DATA_PTR(cdp);

	xplot_fmove(x,*yp);
	xplot_select(*cp);
	xplot_fcont(x,*yp);
	while(i--){
		yp += inc;
		cp += cinc;
		x += dx;
		y = *yp;
		xplot_select(*cp);
		xplot_fcont(x,y);
	}
}

#define double_xyplot(dp) _double_xyplot(QSP_ARG  dp)

static void _double_xyplot(QSP_ARG_DECL  Data_Obj *dp)
{
	u_long i,j,k;
	double x,y,*p;

	XYPLOT_LOOP( double, , x = *p; y = *(p+OBJ_COMP_INC(dp)) )
}

#define float_xyplot(dp) _float_xyplot(QSP_ARG  dp)

static void _float_xyplot(QSP_ARG_DECL  Data_Obj *dp)
{
	u_long i,j,k;
	float x,y,*p;

	XYPLOT_LOOP( float, , x = *p; y = *(p+OBJ_COMP_INC(dp)) )
}

static COMMAND_FUNC( do_xyplot )
{
	Data_Obj *dp;

	dp=pick_obj("data vector");
	if( dp==NULL ) return;

	INSIST_RAM_OBJ(dp,"xyplot")

	if( bad_plot_vec2(QSP_ARG dp,2,"xyplot") ) return;

	switch( OBJ_MACH_PREC(dp) ){
		case PREC_SP:
			float_xyplot(dp);
			break;
		case PREC_DP:
			double_xyplot(dp);
			break;
		default:
			sprintf(ERROR_STRING,"do_xyplot:  unhandled precision %s (object %s)",
				OBJ_PREC_NAME(dp),OBJ_NAME(dp));
			warn(ERROR_STRING);
			break;
	}
}

/*
 * This is like xyplot, except that we don't draw stuff with negative z coords...
 */

static COMMAND_FUNC( do_xyzplot )
{
	Data_Obj *dp;
	dimension_t i,j;
	std_type x,y,z,*p;
	std_type lastx,lasty;
#define DOWN	1
#define UP	2
	int pen_state=UP;

	dp=pick_obj("data vector");
	if( dp==NULL ) return;

	INSIST_RAM_OBJ(dp,"xyzplot")

	if( bad_plot_vec(QSP_ARG dp,3,"xyzplot") ) return;

	for(i=0;i<OBJ_ROWS(dp);i++){
		p = (std_type *) OBJ_DATA_PTR(dp);
		p += i*OBJ_ROW_INC(dp);
		pen_state=UP;
		for(j=0;j<OBJ_COLS(dp);j++){
			x = *p;
			y = *(p + OBJ_COMP_INC(dp));
			z = *(p + 2*OBJ_COMP_INC(dp));
			p += OBJ_PXL_INC(dp);

			/* removed else so that a single column data set will plot the pt */
			if( z < 0 ){
				if( pen_state == DOWN ){
					// can't be first point if pen is down
					xplot_fcont(lastx,lasty);
				}
				pen_state = UP;
			} else {	/* draw this point */
				/* This is a hack for plotting lat/long without
				 * wrap-around...  BUG
				 */
				if( pen_state == UP ){
					xplot_fmove(x,y);
					lastx=x;
					lasty=y;
					// draw from here
				}
				if( lat_long_hack ){
					if( fabs(lastx-x) < 180 &&
					   fabs(lasty-y) < 180 ){
						xplot_fcont(x,y);
					} else {
						xplot_move((int)x,(int)y);
					}
				} else {
					xplot_fcont(x,y);
				}
				pen_state=DOWN;
			}
			// remember this point
			lastx = x;
			lasty = y;
		}
	}
}

static COMMAND_FUNC( do_xp_circ )
{
	std_type rad;

	rad = (std_type)how_much("radius");
	xplot_circle(rad);
}

static COMMAND_FUNC( do_plot_string )
{
	const char *s;

	s=nameof("string");
	xplot_text(s);
}

static COMMAND_FUNC( do_update_plot )
{
#ifdef BUILD_FOR_IOS
	xplot_update();
#else /* ! BUILD_FOR_IOS */
	advise("update_plot does nothing (except on iOS)");
#endif /* ! BUILD_FOR_IOS */
}

static COMMAND_FUNC( do_tell_plot_space ){ tell_plot_space(); }

#define ADD_CMD(s,f,h)	ADD_COMMAND(xplot_menu,s,f,h)

MENU_BEGIN(xplot)
ADD_CMD( space,		do_xp_space,	define plotting space )
ADD_CMD( move,		do_xp_move,	move CAP )
ADD_CMD( cont,		do_xp_cont,	draw vector )
ADD_CMD( point,		do_xp_point,	draw point )
ADD_CMD( line,		do_xp_line,	draw line )
ADD_CMD( arc,		do_xp_arc,	draw arc )
ADD_CMD( circle,	do_xp_circ,	draw circle )
ADD_CMD( fill_arc,	do_xp_fill_arc,	draw and fill an arc )
ADD_CMD( fill_polygon,	do_xp_fill_polygon, draw and fill a polygon )
ADD_CMD( select,	do_xp_select,	select drawing color )
ADD_CMD( erase,		do_xp_erase,	erase )
ADD_CMD( yplot,		do_yplot,	graph a vector of data values )
ADD_CMD( xplot,		do_xplot,	plot a function along the y axis )
ADD_CMD( string,	do_plot_string,	draw a text string )
ADD_CMD( cyplot,	do_cyplot,	graph a vector of data values w/ colors )
ADD_CMD( xyplot,	do_xyplot,	graph an image of x-y pairs )
ADD_CMD( xyzplot,	do_xyzplot,	graph an image of x-y pairs for z>0 )
	/*
ADD_CMD( poly,		do_xp_poly,	draw filled polygon )
ADD_CMD( bitwise,	multiop,	do bit operations between images )
	*/
ADD_CMD( plot,		do_rdplot,	interpret plot(5) file )
ADD_CMD( dump,		do_dumpdraw,	dump viewer drawlist to stdout )
ADD_CMD( info,		do_tell_plot_space, report plotting space )
ADD_CMD( update,	do_update_plot,	force refresh of plot )
MENU_END(xplot)

COMMAND_FUNC( do_xp_menu )
{
	Viewer *vp;

	INSURE_X11_SERVER
	GET_VIEWER("xp_menu")
	if( vp != NULL )
		xplot_setup(vp);

	CHECK_AND_PUSH_MENU(xplot);
}

