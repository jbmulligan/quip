
// support for the scalar expression parser

// This is kind of messed-up right now, because there is some ambiguity about
// when a string should be converted to a data object...
// maybe we should not convert objects to strings unless we put them
// inside a function string_obj() ?

#include <string.h>
#include "quip_prot.h"
#include "nexpr.h"
#include "function.h"
#include "identifier.h"
#include "dobj_private.h"

#define eval_szbl_expr( enp ) _eval_szbl_expr( QSP_ARG  enp )

static Item * _eval_szbl_expr( QSP_ARG_DECL  Scalar_Expr_Node *enp )
{
	Item *szp=NULL,*szp2;
	index_t index;
	const char *s;

	switch(enp->sen_code){
		case N_LITSTR:
		case N_QUOT_STR:
			s = eval_scalexp_string(enp);
			szp = check_sizable( DEFAULT_QSP_ARG  s );
			/*
			if( szp == NULL ){
				Data_Obj *dp;
				dp = obj_for_string(s);
				szp = (Item *)dp;
			}
			*/
			if( szp == NULL ){
				snprintf(ERROR_STRING,LLEN,
					"No sizable object \"%s\"!?",s);
				warn(ERROR_STRING);
				return NULL;
			}
			break;

		//case N_SIZABLE:
		case N_OBJNAME:
			// Not necessarily a data object!?
			s = eval_scalexp_string(enp);
			szp = _find_sizable( DEFAULT_QSP_ARG  s );
			if( szp == NULL ){
				snprintf(ERROR_STRING,LLEN,
					"No sizable object \"%s\"!?",s);
				warn(ERROR_STRING);
				return NULL;
			}
			break;
		//case N_SUBSIZ:
		case N_SUBSCRIPT:
			szp2=eval_szbl_expr(enp->sen_child[0]);
			if( szp2 == NULL )
				return NULL;
			index = index_for_scalar( eval_expr(enp->sen_child[1]) );
			szp = sub_sizable(DEFAULT_QSP_ARG  szp2,index);
			break;
		//case N_CSUBSIZ:
		case N_CSUBSCRIPT:
			szp2=eval_szbl_expr(enp->sen_child[0]);
			if( szp2 == NULL )
				return NULL;
			index = index_for_scalar( eval_expr(enp->sen_child[1]) );
			szp = csub_sizable(DEFAULT_QSP_ARG  szp2,index);
			break;
		case N_DOBJV_STR_ARG_FUNC:	// eval_szbl_expr
			s = eval_scalexp_string(enp->sen_child[0]);
			if( s == NULL ){
				warn("Error evaluating arg for object-valued function!?");
				return NULL;
			}
			szp = (Item *) (*enp->sen_func_p->fn_u.dobjv_str_arg_func)( QSP_ARG  s );
			break;
		default:
			snprintf(ERROR_STRING,LLEN,
		"unexpected case in eval_szbl_expr %d",enp->sen_code);
			warn(ERROR_STRING);
			assert(0);
			break;
	}
	return(szp);
}

/*
 * This is the default data object locater.
 * If the data module were assumed to be always included
 * with the support library, then we wouldn't need this
 * here, but doing this allows us to run the parser
 * without the data module, but has the same grammer...
 */

static Data_Obj * _def_obj(QSP_ARG_DECL  const char *name)
{
	snprintf(ERROR_STRING,LLEN,"can't search for object \"%s\"; ",name);
	warn(ERROR_STRING);

	warn("data module not linked");
	return NULL;
}

static Data_Obj *_def_sub(QSP_ARG_DECL  Data_Obj *object,index_t index)
{
	warn("can't get subobject; data module not linked");
	return NULL;
}

Data_Obj * (*obj_get_func)(QSP_ARG_DECL  const char *)=_def_obj;
Data_Obj * (*exist_func)(QSP_ARG_DECL  const char *)=_def_obj;
Data_Obj * (*sub_func)(QSP_ARG_DECL  Data_Obj *,index_t)=_def_sub;
Data_Obj * (*csub_func)(QSP_ARG_DECL  Data_Obj *,index_t)=_def_sub;


void set_obj_funcs(
			Data_Obj *(*ofunc)(QSP_ARG_DECL  const char *),
			Data_Obj *(*efunc)(QSP_ARG_DECL  const char *),
			Data_Obj *(*sfunc)(QSP_ARG_DECL  Data_Obj *,index_t),
			Data_Obj *(*cfunc)(QSP_ARG_DECL  Data_Obj *,index_t))
{
	obj_get_func=ofunc;
	exist_func=efunc;
	sub_func=sfunc;
	csub_func=cfunc;
}

#ifdef SCALARS_NOT_OBJECTS

#define scalar_obj_for_id(idp) _scalar_obj_for_id(QSP_ARG  idp)

static Data_Obj * _scalar_obj_for_id(QSP_ARG_DECL  Identifier *idp)
{
	Data_Obj *dp;

	dp = temp_scalar("temp_scalar",ID_PREC_PTR(idp));

	(*((ID_PREC_PTR(idp))->copy_value_func))
		(OBJ_DATA_PTR(dp),ID_SVAL_PTR(idp));

	return dp;
}

#endif // SCALARS_NOT_OBJECTS

/* Evaluate a parsed expression */

#define eval_dobj_expr( enp ) _eval_dobj_expr( QSP_ARG  enp )

static Data_Obj *_eval_dobj_expr( QSP_ARG_DECL  Scalar_Expr_Node *enp )
{
	Data_Obj *dp=NULL,*dp2;
	Typed_Scalar *tsp;
	index_t index;
	const char *s;

	switch(enp->sen_code){
		case N_QUOT_STR:
			s = eval_scalexp_string(enp->sen_child[0]);
			/* first try object lookup... */
			/* we don't want a warning if does not exist... */
			dp = (*exist_func)( QSP_ARG  s );
			/* We have a problem here with indexed objects,
			 * since the indexed names aren't in the database...
			 */
			/*
			if( dp == NULL ){
				// treat the string like a rowvec of chars
				dp = obj_for_string(s);
				return(dp);
			}
			*/
			break;

		case N_SCALAR_OBJ:
			dp = eval_dobj_expr(enp->sen_child[0]);
			if( IS_SCALAR(dp) ) return(dp);
			return NULL;
			break;
		case N_OBJNAME:
			s = eval_scalexp_string(enp->sen_child[0]);
			dp = (*obj_get_func)( QSP_ARG  s );
#ifdef SCALARS_NOT_OBJECTS
			dp = (*exist_func)( QSP_ARG  s );
			if( dp == NULL ){
				Identifier *idp;
				idp = id_of(s);
				if( idp == NULL ){
					snprintf(ERROR_STRING,LLEN,
	"No object or identifier %s!?",s);
					warn(ERROR_STRING);
					return NULL;
				}
				if( ID_TYPE(idp) == ID_SCALAR ){
					dp = scalar_obj_for_id(idp);
				} else {
					snprintf(ERROR_STRING,LLEN,
	"Identifier %s is not a scalar!?",s);
					warn(ERROR_STRING);
					return NULL;
				}
			}
#endif // SCALARS_NOT_OBJECTS
			break;
		case N_SUBSCRIPT:
			dp2=eval_dobj_expr(enp->sen_child[0]);
			tsp = eval_expr(enp->sen_child[1]);
			index=index_for_scalar( tsp );
			RELEASE_SCALAR(tsp)
			dp=(*sub_func)( QSP_ARG  dp2, index );
			break;
		case N_CSUBSCRIPT:
			dp2=eval_dobj_expr(enp->sen_child[0]);
			tsp=eval_expr(enp->sen_child[1]);
			index=index_for_scalar(tsp);
			RELEASE_SCALAR(tsp)
			dp=(*csub_func)( QSP_ARG  dp2, index );
			break;
		case N_DOBJV_STR_ARG_FUNC:	// eval_szbl_expr
			s = eval_scalexp_string(enp->sen_child[0]);
			if( s == NULL ){
				warn("Error evaluating arg for object-valued function!?");
				return NULL;
			}
			dp = (*enp->sen_func_p->fn_u.dobjv_str_arg_func)( QSP_ARG  s );
			break;

		default:
			snprintf(ERROR_STRING,LLEN,
		"unexpected case (%d) in eval_dobj_expr",enp->sen_code);
			warn(ERROR_STRING);
			assert(0);
			break;
	}
	return(dp);
} // end eval_dobj_expr

void init_dobj_expr_funcs(SINGLE_QSP_ARG_DECL)
{
	set_eval_dobj_func( _eval_dobj_expr );
	set_eval_szbl_func( _eval_szbl_expr );
}

