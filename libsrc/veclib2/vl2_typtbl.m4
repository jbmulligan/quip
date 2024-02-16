// Now that the typed functions are static, we include this file at the end
// of vl2_veclib.c...

/* We have a table of functions that take the following arg types:
 *
 *	byte, short, long, float, double, u_byte,
 *	u_short, u_long, u_byte/short, short/byte, u_short/long, float/double, bitmap
 *
 * This is one row of the table - 
 *  then the table has separate rows for:
 *		real
 *		complex
 *		mixed (r/c)
 *		quaternion
 *		mixed (r/q)
 *
 */

// vl2_typtbl.m4 including gen_typtbl.m4 BEGIN
my_include(`veclib/gen_typtbl.m4')
// vl2_typtbl.m4 DONE including gen_typtbl.m4

static void nullobjf(HOST_CALL_ARG_DECLS)
{
	snprintf(ERROR_STRING,LLEN,
"CAUTIOUS:  function %s is not implemented for precision %s!?",
		VF_NAME(&vec_func_tbl[vf_code]),
		ARGSPREC_NAME(OA_ARGSPREC_PTR(oap)) );
	warn(ERROR_STRING);
	show_obj_args(oap);
	/* no more global this_vfp... */
	/*
	advise("nullobjf:");
	snprintf(ERROR_STRING,LLEN,
		"Oops, function %s has not been implemented for %s %s precision (functype = %d)",
		VF_NAME(this_vfp), type_strings[OA_FUNCTYPE(oap)%N_ARGSET_PRECISIONS],
		argset_type_name[(OA_FUNCTYPE(oap)/N_ARGSET_PRECISIONS)+1],OA_FUNCTYPE(oap));
	warn(ERROR_STRING);
	*/
	advise("Need to add better error checking!");
	abort();
}

// Do these have to be in order, or do they get sorted???
// They get sorted, but we don't have an easy test
// to make sure that everything is here that should be!?

Vec_Func_Array vl2_vfa_tbl[]={

// vl2_typtbl.m4 including gen_func_array.m4 BEGIN
my_include(`veclib/gen_func_array.m4')
// vl2_typtbl.m4 DONE including gen_func_array.m4

};

#define N_VL2_ARRAYED_VEC_FUNCS (sizeof(vl2_vfa_tbl)/sizeof(Vec_Func_Array))

void check_vl2_vfa_tbl(SINGLE_QSP_ARG_DECL)
{
	if( N_VL2_ARRAYED_VEC_FUNCS != N_VEC_FUNCS ){
		snprintf(ERROR_STRING,LLEN,
	"vl2_vfa_tbl has %zd entries, expected %d!?",
			N_VL2_ARRAYED_VEC_FUNCS, N_VEC_FUNCS );
		WARN(ERROR_STRING);
//		return -1;
	}
	assert( N_VL2_ARRAYED_VEC_FUNCS <= N_VEC_FUNCS );
	check_vfa_tbl(vl2_vfa_tbl, N_VL2_ARRAYED_VEC_FUNCS);
	assert( N_VL2_ARRAYED_VEC_FUNCS == N_VEC_FUNCS );
}

