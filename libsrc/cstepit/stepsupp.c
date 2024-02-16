#include "quip_config.h"

#include <math.h>

#include "quip_prot.h"
//#include "fitsine.h"
#include "optimize.h"
#include "item_type.h"
#include "list.h"

ITEM_INTERFACE_DECLARATIONS(Opt_Param,opt_param,0)

const char *opt_func_string=NULL;

Opt_Pkg *curr_opt_pkg=NULL;

void _opt_param_info(QSP_ARG_DECL  Opt_Param *opp)
{
	snprintf(msg_str,LLEN,"Parameter %s:",opp->op_name);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tvalue:\t%f",opp->ans);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tmin:\t%f",opp->minv);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tmax:\t%f",opp->maxv);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tdelta:\t%f",opp->delta);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tmindel:\t%f",opp->mindel);
	prt_msg(msg_str);
}

/* C language interface */

void delete_opt_params(SINGLE_QSP_ARG_DECL)
{
	List *lp;
	Node *np;

	lp = opt_param_list();
	if( lp == NULL ) return;
	np=QLIST_HEAD(lp);
	while( np!= NULL ){
		Opt_Param *opp;
		Node *next;

		opp = (Opt_Param *)(np->n_data);
		next=np->n_next;
		del_opt_param(opp);
		np=next;
	}
}

Opt_Param * _add_opt_param(QSP_ARG_DECL  Opt_Param *opp)
{
	Opt_Param *new_opp;

	new_opp = new_opt_param(opp->op_name);
	if( new_opp != NULL ){
		new_opp->ans = opp->ans;
		new_opp->maxv = opp->maxv;
		new_opp->minv = opp->minv;
		new_opp->delta = opp->delta;
		new_opp->mindel = opp->mindel;
	}
	return(new_opp);
}

void _optimize(QSP_ARG_DECL  float (*opt_func)(SINGLE_QSP_ARG_DECL))
{
	insure_opt_pkg(SINGLE_QSP_ARG);
	(*curr_opt_pkg->pkg_c_func)(QSP_ARG  opt_func);
}

float _get_opt_param_value(QSP_ARG_DECL  const char *name)
{
	Opt_Param *opp;

	opp=get_opt_param(name);
	if( opp==NULL ){
		snprintf(ERROR_STRING,LLEN,"No optimization parameter \"%s\"",name);
		warn(ERROR_STRING);
		return(-1.0);
	}
	return(opp->ans);
}

