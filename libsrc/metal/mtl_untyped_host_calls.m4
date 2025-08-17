
dnl	Put fft stuff here??

suppress_no



dnl	 FP_PREC_SWITCH(sw_dp,func)
define(`FP_PREC_SWITCH',`

	switch( OBJ_MACH_PREC($1) ){
		case PREC_SP:
			HOST_TYPED_CALL_NAME($2,sp)(HOST_CALL_ARGS);
			break;
		case PREC_DP:
			HOST_TYPED_CALL_NAME($2,dp)(HOST_CALL_ARGS);
			break;
		default:
			fprintf(stderr,"Unexpected destination precision!?\n");
			break;
	}
')

dnl	FP_PREC_SWITCH_ISINV(sw_dp,func,isinv)
define(`FP_PREC_SWITCH_ISINV',`

	switch( OBJ_MACH_PREC($1) ){
		case PREC_SP:
			HOST_TYPED_CALL_NAME($2,sp)(HOST_CALL_ARGS,$3);
			break;
		case PREC_DP:
			HOST_TYPED_CALL_NAME($2,dp)(HOST_CALL_ARGS,$3);
			break;
		default:
			fprintf(stderr,"Unexpected destination precision!?\n");
			break;
	}
')

dnl	 RC_SWITCH(sw_dp,rfunc,cfunc,isinv)
define(`RC_SWITCH',`
	Vec_Obj_Args oa1, *oap=(&oa1);
fprintf(stderr,"%s BEGIN, oap = 0x%lx\n",STRINGIFY(HOST_CALL_NAME($2)),(u_long)oap);
	setvarg2(oap,_dst_dp,src_dp);
	SET_OA_PFDEV(oap,OBJ_PFDEV(OA_DEST(oap)));
	if( IS_REAL($1) ){
		FP_PREC_SWITCH(_dst_dp,`r'$2)
	} else {
		FP_PREC_SWITCH_ISINV(_dst_dp,`c'$3,$4)
	}
')

