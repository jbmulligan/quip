#include "quip_config.h"

#include "quip_prot.h"
#include "vectree.h"
#include "vt_native.h"
#include "vec_util.h"
#include "veclib_api.h"				/* setvarg1() etc */
#include "veclib/vl2_veclib_prot.h"		/* h_vl2_xform_list() */
#ifdef HAVE_NUMREC
#ifdef USE_NUMREC
#include "nrm_api.h"
#endif /* USE_NUMREC */
#endif /* HAVE_NUMREC */
#include "debug.h"		/* verbose */
#include "platform.h"		/* dp_convert - should be declared elsewhere!? */

#define EVAL_VT_NATIVE_WORK(enp)		eval_vt_native_work(QSP_ARG enp)

Keyword vt_native_func_tbl[N_VT_NATIVE_FUNCS+1]={
//#ifdef HAVE_NUMREC
//#ifdef USE_NUMREC
{	"svdcmp",	NATIVE_SVDCMP	},
{	"svbksb",	NATIVE_SVBKSB	},
{	"jacobi",	NATIVE_JACOBI	},
{	"eigsrt",	NATIVE_EIGSRT	},
{	"choldc",	NATIVE_CHOLDC	},
//#endif /* USE_NUMREC */
//#endif /* HAVE_NUMREC */
{	"system",	NATIVE_SYSTEM	},
{	"render",	NATIVE_RENDER	},
{	"xform_list",	NATIVE_XFORM_LIST	},
{	"invert",	NATIVE_INVERT	},
{	"cumsum",	NATIVE_CUMSUM	},
{	"foobar",	-1		}		/* must be last */
};

const char *_eval_vt_native_string(QSP_ARG_DECL  Vec_Expr_Node *enp)
{
	warn("eval_vt_native_string:  not implemented for vt!?");
	return("");
}

float _eval_vt_native_flt(QSP_ARG_DECL  Vec_Expr_Node *enp)
{
	warn("eval_vt_native_flt:  not implemented for vt!?");
	return(0.0);
}

void _eval_vt_native_assignment(QSP_ARG_DECL  Data_Obj *dp, Vec_Expr_Node *enp )
{
	switch(VN_INTVAL(enp)){
		default:
			assert( AERROR("eval_vt_native_assignment:  unhandled keyword!?") );
			break;
	}
}

#define CHECK_ARGLIST(enp,name)						\
									\
	if( enp == NULL ){						\
		snprintf(ERROR_STRING,LLEN,"missing arg list for native function %s",name);	\
		warn(ERROR_STRING);					\
		return;							\
	}								\
	if( VN_CODE(enp) != T_ARGLIST ){				\
		snprintf(ERROR_STRING,LLEN,"Oops, missing arglist for native function %s!?",name);	\
		warn(ERROR_STRING);					\
		dump_tree(enp);						\
		return;							\
	}

void _eval_vt_native_work(QSP_ARG_DECL  Vec_Expr_Node *enp )
{
	Vec_Expr_Node *arg_enp;
	int vf_code=(-1);

	/* BUG? we need to define a template for the args somewhere,
	 * so that we can make sure that the args are correct during tree-building...
	 */

	switch(VN_INTVAL(enp)){
		case NATIVE_CUMSUM:
			{
			Data_Obj *dst_dp,*src_dp;

			if( arg_count(VN_CHILD(enp,0)) != 2 ){
				node_error(enp);
				warn("cumsum function requires 2 args");
				return;
			}

			arg_enp = nth_arg(VN_CHILD(enp,0),0);
			dst_dp = eval_obj_ref(arg_enp);

			arg_enp = nth_arg(VN_CHILD(enp,0),1);
			src_dp = eval_obj_exp(arg_enp,NULL);

			if( dst_dp == NULL || src_dp == NULL ){
				node_error(enp);
				warn("problem with cumsum args");
				break;
			}

			war_cumsum(dst_dp, src_dp );
			break;
			}
		case NATIVE_RENDER:
			{
			Data_Obj *dst_dp,*coord_dp,*src_dp;

			if( arg_count(VN_CHILD(enp,0)) != 3 ){
				node_error(enp);
				warn("render function requires 3 args");
				return;
			}

			arg_enp = nth_arg(VN_CHILD(enp,0),0);
			dst_dp = eval_obj_ref(arg_enp);

			arg_enp = nth_arg(VN_CHILD(enp,0),1);
			coord_dp = eval_obj_exp(arg_enp,NULL);

			arg_enp = nth_arg(VN_CHILD(enp,0),2);
			src_dp = eval_obj_exp(arg_enp,NULL);

			if( dst_dp == NULL || coord_dp == NULL || src_dp == NULL ){
				node_error(enp);
				warn("problem with render args");
				break;
			}
			if( IS_FLOATING_PREC_CODE(OBJ_PREC(coord_dp)) )
				render_samples2(dst_dp,coord_dp,src_dp);
			else
				render_samples(dst_dp,coord_dp,src_dp);
			}
			break;

		case NATIVE_INVERT:
			{
			Data_Obj *dst_dp, *src_dp;

			if( arg_count(VN_CHILD(enp,0)) != 2 ){
				node_error(enp);
				warn("invert function requires 2 args");
				return;
			}

			arg_enp = nth_arg(VN_CHILD(enp,0),0);
			dst_dp = eval_obj_ref(arg_enp);

			arg_enp = nth_arg(VN_CHILD(enp,0),1);
			src_dp = eval_obj_exp(arg_enp,NULL);

			/* BUG I use convert() here because I am lazy */
			/* should just use vmov... */
			//setvarg2(oap,dst_dp,src_dp);
			//h_vl2_convert(HOST_CALL_ARGS);
			dp_convert(dst_dp,src_dp);
			/* dtinvert operates inplace */
			dt_invert(dst_dp);
			/* set value set flag??? */
			}
			break;


		case NATIVE_SYSTEM:
			{
			const char *s;
			int status;
			char stat_str[32];
			Variable *vp;

			s=eval_string(VN_CHILD(enp,0));
#ifndef BUILD_FOR_IOS
			status = system(s);
#else // BUILD_FOR_IOS
            warn("Sorry, system() is temporarily unavailable for iOS!?");
            snprintf(ERROR_STRING,LLEN,"Can't run command '%s'",s);
            advise(ERROR_STRING);
            status=(-1);
#endif // BUILD_FOR_IOS
                
			snprintf(stat_str,32,"%d",status);	// BUG?  protect against buffer overflow?
			vp=_assign_reserved_var(DEFAULT_QSP_ARG  "exit_status",stat_str);
			assert( vp != NULL );
				
			}
			break;

		case NATIVE_CHOLDC:
			{
			Data_Obj *inmat_dp, *diag_dp;

advise("evaluating choldc...");
			if( arg_count(VN_CHILD(enp,0)) != 2) {
				node_error(enp);
				warn("choldc requires 2 args");
				return;
			}

			/* first arg is the input matrix, second arg is for the diagonal elements... */
			arg_enp  = nth_arg(VN_CHILD(enp,0),0);
			inmat_dp = eval_obj_ref(arg_enp);
		
			arg_enp = nth_arg(VN_CHILD(enp,0),1);
			diag_dp = eval_obj_ref(arg_enp);

			if ( inmat_dp == NULL || diag_dp == NULL )
				return;

#ifdef HAVE_NUMREC
#ifdef USE_NUMREC
			dp_choldc(inmat_dp,diag_dp);
#else // ! USE_NUMREC	
			warn("Program not configured to use numerical recipes library, can't compute CHOLESKY");
#endif // ! USE_NUMREC	

#else // ! HAVE_NUMREC
			warn("No numerical recipes library, can't compute CHOLESKY");
#endif // ! HAVE_NUMREC

			}

			break;
		case NATIVE_SVDCMP:
			{
			Data_Obj *umat_dp, *vmat_dp, *ev_dp;
			/* We need to get three args... */
			if( arg_count(VN_CHILD(enp,0)) != 3 ){
				node_error(enp);
				warn("svdcmp requires 3 args");
				return;
			}
			/* v matrix on the second branch */
			arg_enp = nth_arg(VN_CHILD(enp,0),0);
			umat_dp = eval_obj_ref(arg_enp);

			arg_enp = nth_arg(VN_CHILD(enp,0),1);
			ev_dp = eval_obj_ref(arg_enp);

			arg_enp = nth_arg(VN_CHILD(enp,0),2);
			vmat_dp = eval_obj_ref(arg_enp);

			if( ev_dp == NULL || umat_dp == NULL || vmat_dp == NULL )
				return;
				
			/*
			snprintf(ERROR_STRING,LLEN,"Ready to compute SVD, umat = %s, vmat= %s, ev = %s",
				OBJ_NAME(umat_dp),OBJ_NAME(vmat_dp),OBJ_NAME(ev_dp));
			advise(ERROR_STRING);
			*/

#ifdef HAVE_NUMREC
#ifdef USE_NUMREC
			dp_svd(umat_dp,ev_dp,vmat_dp);
#else // USE_NUMREC
			warn("Program not configured to use numerical recipes library, can't compute SVD! - try GSL?");
#endif // USE_NUMREC
#else
			warn("Numerical recipes library not present, can't compute SVD");
#endif
			}
			break;
		case NATIVE_SVBKSB:
			{
			/* svbksb(x,u,w,v,b); */
			Data_Obj *umat_dp, *vmat_dp, *ev_dp;
			Data_Obj *x_dp, *b_dp;

			enp=VN_CHILD(enp,0);		/* the arg list */
			CHECK_ARGLIST(enp,"svbksb")
			b_dp = eval_obj_ref(VN_CHILD(enp,1));
			enp=VN_CHILD(enp,0);
			CHECK_ARGLIST(enp,"svbksb")
			vmat_dp = eval_obj_ref(VN_CHILD(enp,1));
			enp=VN_CHILD(enp,0);
			CHECK_ARGLIST(enp,"svbksb")
			ev_dp = eval_obj_ref(VN_CHILD(enp,1));
			enp=VN_CHILD(enp,0);
			CHECK_ARGLIST(enp,"svbksb")
			umat_dp = eval_obj_ref(VN_CHILD(enp,1));
			x_dp = eval_obj_ref(VN_CHILD(enp,0));

			if( x_dp == NULL || umat_dp == NULL || ev_dp == NULL ||
				vmat_dp == NULL || b_dp == NULL )
				return;

#ifdef HAVE_NUMREC
#ifdef USE_NUMREC
			dp_svbksb(x_dp,umat_dp,ev_dp,vmat_dp,b_dp);
#else // ! USE_NUMREC
			warn("Program not configured to use numerical recipes library, can't compute SVBKSB");
#endif // ! USE_NUMREC
#else // ! HAVE_NUMREC
			warn("No numerical recipes library, can't compute SVBKSB");
#endif // ! HAVE_NUMREC
			}
			break;
		case NATIVE_JACOBI:			/* eval_vt_native_work */
			{
			/* jacobi(eigenvectors,eigenvalues,input_matrix,nrot) */
			/* do we use nrot?? */
			Data_Obj *a_dp,*d_dp,*v_dp;
#ifdef HAVE_NUMREC
#ifdef USE_NUMREC
			int nrot;
#endif  // USE_NUMREC
#endif // HAVE_NUMREC

			enp=VN_CHILD(enp,0);		/* the arg list */
			CHECK_ARGLIST(enp,"jacobi")
			a_dp = eval_obj_ref(VN_CHILD(enp,1));

			enp=VN_CHILD(enp,0);
			CHECK_ARGLIST(enp,"jacobi")
			d_dp = eval_obj_ref(VN_CHILD(enp,1));
			v_dp = eval_obj_ref(VN_CHILD(enp,0));

			if( a_dp == NULL || d_dp == NULL || v_dp == NULL )
				return;

#ifdef HAVE_NUMREC
#ifdef USE_NUMREC
			dp_jacobi(v_dp,d_dp,a_dp,&nrot);
			if( verbose ){
				snprintf(ERROR_STRING,LLEN,"jacobi(%s,%s,%s) done after %d rotations",
					OBJ_NAME(v_dp),OBJ_NAME(d_dp),OBJ_NAME(a_dp),nrot);
				advise(ERROR_STRING);
			}
			//SET_OBJ_FLAG_BITS(v_dp, DT_ASSIGNED);
			note_assignment(v_dp);
#else // ! USE_NUMREC
			warn("Program not configured to use numerical recipes library, can't compute JACOBI");
#endif // ! USE_NUMREC

#else // ! HAVE_NUMREC
			warn("No numerical recipes library, can't compute JACOBI");
#endif // ! HAVE_NUMREC
			}
			break;
		case NATIVE_EIGSRT:
			{
			Data_Obj *d_dp,*v_dp;
			/* eigsrt(eigenvectors,eigenvalues) */
			enp=VN_CHILD(enp,0);
			CHECK_ARGLIST(enp,"eigsrt")
			d_dp = eval_obj_ref(VN_CHILD(enp,1));
			v_dp = eval_obj_ref(VN_CHILD(enp,0));

			if( d_dp == NULL || v_dp == NULL )
				return;

#ifdef HAVE_NUMREC
#ifdef USE_NUMREC
			dp_eigsrt(v_dp,d_dp);
#else // ! USE_NUMREC
			warn("Program not configured to use numerical recipes library, can't compute EIGSRT");
#endif // ! USE_NUMREC

#else // ! HAVE_NUMREC
			warn("No numerical recipes library, can't compute EIGSRT");
#endif // ! HAVE_NUMREC
			}
			break;

		case NATIVE_XFORM_LIST:
			{
			Vec_Obj_Args oa1,*oap=&oa1;
			Data_Obj *dst_dp, *src_dp, *mat_dp;
			/* xform_list(&dst,&src,&mat); */
			enp=VN_CHILD(enp,0);
			CHECK_ARGLIST(enp,"xform_list")
			/* left child is an arglist */
			assert( VN_CODE(VN_CHILD(enp,0)) == T_ARGLIST );
				
			dst_dp = eval_obj_ref(VN_CHILD(VN_CHILD(enp,0),0));
			src_dp = eval_obj_ref(VN_CHILD(VN_CHILD(enp,0),1));
			mat_dp = eval_obj_ref(VN_CHILD(enp,1));

			if( dst_dp == NULL || src_dp == NULL || mat_dp == NULL )
				return;

			// OLD
			// h_vl2_xform_list(dst_dp,src_dp,mat_dp);
			setvarg3(oap,dst_dp,src_dp,mat_dp);
			h_vl2_xform_list(HOST_CALL_ARGS);
			//SET_OBJ_FLAG_BITS(dst_dp, DT_ASSIGNED);
			note_assignment(dst_dp);
			}
			break;

#ifdef CAUTIOUS
		default:
			snprintf(ERROR_STRING,LLEN,
	"CAUTIOUS:  eval_vt_native_work (vt):  unhandled keyword %s (%"PRId64")",vt_native_func_tbl[VN_INTVAL(enp)].kw_token,VN_INTVAL(enp));
			warn(ERROR_STRING);
//			assert( AERROR("eval_vt_native_work:  unhandled keyword!?") );
			break;
#endif /* CAUTIOUS */
	}
}

void _update_vt_native_shape(QSP_ARG_DECL  Vec_Expr_Node *enp)
{
	switch(VN_INTVAL(enp)){
		default:
			assert( AERROR("update_native_shape:  unhandled keyword!?") );
			break;
	}
}

void _prelim_vt_native_shape(QSP_ARG_DECL  Vec_Expr_Node *enp)
{

	// All of these have no shape, so there's not
	// really much of a point in switching on the function.
	// But maybe in the future we will allow native funcs
	// to return a value?

	switch(VN_INTVAL(enp)){
		case  NATIVE_SYSTEM:
		case  NATIVE_RENDER:
		case  NATIVE_XFORM_LIST:
		case  NATIVE_INVERT:
		case  NATIVE_CUMSUM:
//#ifdef HAVE_NUMREC
//#ifdef USE_NUMREC
		case  NATIVE_CHOLDC:
		case  NATIVE_SVDCMP:
		case  NATIVE_SVBKSB:
		case  NATIVE_JACOBI:
		case  NATIVE_EIGSRT:
//#endif // USE_NUMREC
//#endif /* HAVE_NUMREC */
			/* no shape, do nothing */
			assert( VN_SHAPE(enp) == NULL );
			break;

		default:
			assert( AERROR("prelim_vt_native_shape:  unhandled native func!?") );
			break;
	}
}

