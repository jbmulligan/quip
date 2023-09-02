
#include "quip_config.h"

#include <stdio.h>
#include "quip_prot.h"
#include "data_obj.h"
#include "nrm_api.h"

#ifdef HAVE_NUMREC

#include "numrec.h"


#ifdef DEBUG
static void numrec_debug_init(SINGLE_QSP_ARG_DECL);
#endif /* DEBUG */


static COMMAND_FUNC( do_choldc )
{
	Data_Obj *a_dp, *p_dp;

	a_dp=pick_obj( "input/output matrix" );
	p_dp=pick_obj( "elements diagonal matrix" );

	if ( a_dp == NULL || p_dp == NULL )
		return;

	printf("nrmenu:nrmenu.c:pickobj %f\n", *((float *)OBJ_DATA_PTR(a_dp)));

	dp_choldc(a_dp,p_dp);
}


static COMMAND_FUNC( do_svd )
{
	Data_Obj *a_dp, *w_dp, *v_dp;

	a_dp=pick_obj( "input matrix" );
	w_dp=pick_obj( "vector for singular values" );
	v_dp=pick_obj( "output v matrix" );

	if( a_dp == NULL || w_dp == NULL || v_dp == NULL )
		return;

	dp_svd(a_dp,w_dp,v_dp);
}

static COMMAND_FUNC( do_svbksb )
{
	Data_Obj *u_dp, *w_dp, *v_dp;
	Data_Obj *x_dp, *b_dp;

	x_dp=pick_obj( "result vector for coefficients" );
	u_dp=pick_obj( "U matrix" );
	w_dp=pick_obj( "vector of singular values" );
	v_dp=pick_obj( "V matrix" );
	b_dp=pick_obj( "data vector" );

	if( u_dp == NULL || w_dp == NULL || v_dp == NULL ||
		x_dp == NULL || b_dp == NULL )
		return;

	dp_svbksb(x_dp,u_dp,w_dp,v_dp,b_dp);
}

static COMMAND_FUNC( do_jacobi )
{
	Data_Obj *a_dp,*d_dp,*v_dp;
	int nrot;

	v_dp = pick_obj("destination matrix for eigenvectors");
	d_dp = pick_obj("destination vector for eigenvalues");
	a_dp = pick_obj("input matrix");

	if( v_dp == NULL || d_dp == NULL || a_dp == NULL ) return;

	dp_jacobi(v_dp,d_dp,a_dp,&nrot);

	// sprintf(msg_str,"%d rotations performed",nrot);
	// prt_msg(msg_str);
}

static COMMAND_FUNC( do_eigsrt )
{
	Data_Obj *v_dp, *d_dp;
	
	v_dp = pick_obj("matrix of eigenvectors from Jacobi");
	d_dp = pick_obj("vector of eigenvalues from Jacobi");

	if( v_dp == NULL || d_dp == NULL ) return;

	dp_eigsrt(v_dp,d_dp);
}

static COMMAND_FUNC( do_moment )
{
	Data_Obj *d_dp;
	
	d_dp = pick_obj("array of data");
	/* How to get the values of ave and sdev??? */
	dp_moment(d_dp);
}

static COMMAND_FUNC( do_plgndr )
{
	float x,r;
	int l,m;

	l=(int)how_many("l");
	m=(int)how_many("m");
	x=(float)how_much("x");

	if( m < 0 || m > l ){
		sprintf(ERROR_STRING,"parameter m (%d) must be between 0 and l (%d)",m,l);
		warn(ERROR_STRING);
		return;
	}
	if( x < -1 || x > 1 ){
		sprintf(ERROR_STRING,"parameter x (%g) must be between -1 and 1",x);
		warn(ERROR_STRING);
		return;
	}

	r = plgndr(l,m,x);

	sprintf(msg_str,"P_%d,%d(%g) = %g",l,m,x,r);
	prt_msg(msg_str);
}

static int polish_roots=1;

static COMMAND_FUNC( do_set_polish )
{
	polish_roots = ASKIF( "polish roots with laguerre method" );
}

static COMMAND_FUNC( do_zroots )
{
	Data_Obj *a_dp, *r_dp;

	r_dp=pick_obj( "complex destination vector for roots" );
	a_dp=pick_obj( "polynomial coefficient vector" );

	if( r_dp == NULL || a_dp == NULL ) return;

	dp_zroots(r_dp,a_dp,polish_roots);
}

static COMMAND_FUNC(do_nr_version){
	const char *msg = get_numrec_version();
	sprintf(MSG_STR,"Numrec version: %s",msg);
	prt_msg(MSG_STR);
}

#define ADD_CMD(s,f,h)	ADD_COMMAND(numrec_menu,s,f,h)

MENU_BEGIN(numrec)
ADD_CMD( nr_version,	do_nr_version,	report library version )
ADD_CMD( svd,		do_svd,		singular value decomposition )
ADD_CMD( svbk,		do_svbksb,	back substitute into SVD )
ADD_CMD( jacobi,	do_jacobi,	compute eigenvectors & eigenvalues )
ADD_CMD( eigsrt,	do_eigsrt,	sort the eigenvalues into descending order )
ADD_CMD( moment,	do_moment,	mean value & standard deviation of a vector )
ADD_CMD( plgndr,	do_plgndr,	compute legendre polynomial )
ADD_CMD( zroots,	do_zroots,	compute roots of a polynomial )
ADD_CMD( polish,	do_set_polish,	enable (default) or disable root polishing in zroots() )
ADD_CMD( choldc,	do_choldc,	cholesky decomposition )
MENU_END(numrec)

#ifdef DEBUG
int numrec_debug=0;

static void numrec_debug_init(SINGLE_QSP_ARG_DECL)
{
	if( ! numrec_debug ){
		numrec_debug = add_debug_module("numrec");
	}
}
#endif /* DEBUG */


COMMAND_FUNC( do_nr_menu )
{
	static int inited=0;


	if( !inited ){
#ifdef DEBUG
		numrec_debug_init(SINGLE_QSP_ARG);
#endif /* DEBUG */
		inited=1;
	}

	CHECK_AND_PUSH_MENU(numrec);
}

#endif /* HAVE_NUMREC */


