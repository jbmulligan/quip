// These macros are used to build the host-side function calls, which typically
// involve a speed test, then a branch to a speed-specific host function that
// calls the appropriate kernel.
//
// This file contains "special" definitions that don't follow the usual pattern...

define(`INSIST_CONTIG',`

	if( ! is_contiguous($1) ){
		snprintf(ERROR_STRING,LLEN,
	"Sorry, object %s must be contiguous for %s (gpu only?).",
			OBJ_NAME($1),$2);
		warn(ERROR_STRING);
		return;
	}
')



define(`INSIST_LENGTH',`

	if( ($1) == 1 ){
		snprintf(ERROR_STRING,LLEN,
	"Oops, kind of silly to do %s of 1-len vector %s!?",$2,$3);
		warn(ERROR_STRING);
		return;
	}
')


define(`_VEC_FUNC_2V_PROJ_IDX',`H_CALL_PROJ_2V_IDX($1)')
define(`_VEC_FUNC_MM_NOCC',`H_CALL_MM_NOCC($1)')

define(`_VEC_FUNC_MM',`H_CALL_MM($1)')
define(`_VEC_FUNC_CPX_2V_PROJ',`H_CALL_PROJ_2V( $1, dest_cpx, std_cpx )')

define(`_VEC_FUNC_QUAT_2V_PROJ',`H_CALL_PROJ_2V( $1, dest_quat, std_quat )')

// This was H_CALL_MM - ???
// The statements are used in the kernels, this just declares the function that fixes the args
// and then calls the kernel...
define(`_VEC_FUNC_2V_PROJ',`H_CALL_PROJ_2V( $1, dest_type, std_type )')

define(`_VEC_FUNC_3V_PROJ',`H_CALL_PROJ_3V( $1, dest_type, std_type, rvmul, rvsum)')
define(`_VEC_FUNC_CPX_3V_PROJ',`H_CALL_PROJ_3V( $1, dest_cpx, std_cpx, cvmul, cvsum)')

dnl define(`_VEC_FUNC_CPX_LUTMAP_B',`H_CALL_LUTMAP_B( $1, dest_cpx, std_cpx, cvlutmapb)')
dnl define(`_VEC_FUNC_QUAT_LUTMAP_B',`H_CALL_LUTMAP_B( $1, dest_quat, std_quat, qvlutmapb)')


