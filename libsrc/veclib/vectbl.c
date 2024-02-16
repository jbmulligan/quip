
#include "quip_config.h"

/* this table defines all of the functions.
 * It gives their name, a mask that tells the supported precisions,
 * and an entry point...
 */

#include <stdlib.h>	/* qsort */
#include "quip_prot.h"
#include "nvf.h"
#include "rn.h"		/* set_random_seed */
#include "platform.h"

static Item_Type *vec_func_itp=NULL;
static ITEM_INIT_PROT(Vector_Function, vec_func)
#define init_vec_funcs()	_init_vec_funcs(SINGLE_QSP_ARG)

static ITEM_INIT_FUNC(Vector_Function, vec_func, 0)
ITEM_LIST_FUNC(Vector_Function, vec_func)
ITEM_PICK_FUNC(Vector_Function, vec_func)

/* This used to be an initialized table,
 * but in Objective C we have to do it differently...
 *
 * This table contains information about the functions, but isn't
 * specific to a particular platform.  The elements are the name,
 * the code, a code indicating the argument types, a mask indicating
 * what precisions (and combinations) are allowed, and a mask
 * indicating what types (real/complex/quaternion/mixed/etc)
 * are allowed.
 */

BEGIN_VFUNC_DECLS

CREAT_VEC_FUNC( vset,		FVSET,		S_UNARY,	M_ALL,	RCQ_MASK	)
CREAT_VEC_FUNC( vmov,		FVMOV,		V_UNARY,	M_ALL,	RCQ_MASK	)
CREAT_VEC_FUNC( vadd,		FVADD,		VV_BINARY,	M_ALLMM,RCMQP_MASK	)
CREAT_VEC_FUNC( vsub,		FVSUB,		VV_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vmul,		FVMUL,		VV_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vdiv,		FVDIV,		VV_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vneg,		FVNEG,		V_UNARY,	M_ALL,	RCQ_MASK	)
CREAT_VEC_FUNC( vsqr,		FVSQR,		V_UNARY,	M_ALL,	RCQ_MASK	)
CREAT_VEC_FUNC( vramp1d,	FVRAMP1D,	SS_RAMP,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vramp2d,	FVRAMP2D,	SSS_RAMP,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsadd,		FVSADD,		VS_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vssub,		FVSSUB,		VS_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vsmul,		FVSMUL,		VS_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vsdiv,		FVSDIV,		VS_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vsdiv2,		FVSDIV2,	VS_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vssub2,		FVSSUB2,	VS_BINARY,	M_ALL,	RCMQP_MASK	)
CREAT_VEC_FUNC( vabs,		FVABS,		V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsign,		FVSIGN,		V_UNARY,	M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vsqrt,		FVSQRT,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsin,		FVSIN,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vcos,		FVCOS,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vatan,		FVATAN,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vtan,		FVTAN,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vatan2,		FVATAN2,	VV_BINARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsatan2,	FVSATAN2,	VS_BINARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsatan22,	FVSATAN22,	VS_BINARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vlog,		FVLOG,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vlog10,		FVLOG10,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
//CREAT_VEC_FUNC( vexp,		FVEXP,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vexp,		FVEXP,		V_UNARY,	M_BP,	RC_MASK	)
CREAT_VEC_FUNC( verf,		FVERF,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( verfinv,	FVERFINV,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vpow,		FVPOW,		VV_BINARY,	M_BP,	RC_MASK	)
CREAT_VEC_FUNC( vspow,		FVSPOW,		VS_BINARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vspow2,		FVSPOW2,	VS_BINARY,	M_BP,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vmin,		FVMIN,		VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmax,		FVMAX,		VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vminm,		FVMINM,		VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmaxm,		FVMAXM,		VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsmin,		FVSMIN,		VS_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsmax,		FVSMAX,		VS_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsmnm,		FVSMNM,		VS_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsmxm,		FVSMXM,		VS_BINARY,	M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vminv,		FVMINV,		V_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmaxv,		FVMAXV,		V_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmnmv,		FVMNMV,		V_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmxmv,		FVMXMV,		V_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmini,		FVMINI,		V_INT_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmaxi,		FVMAXI,		V_INT_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmnmi,		FVMNMI,		V_INT_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmxmi,		FVMXMI,		V_INT_PROJECTION,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vming,		FVMING,		V_SCALRET2,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmaxg,		FVMAXG,		V_SCALRET2,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmnmg,		FVMNMG,		V_SCALRET2,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmxmg,		FVMXMG,		V_SCALRET2,	M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vfloor,		FVFLOOR,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vtrunc,		FVTRUNC,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vround,		FVROUND,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vceil,		FVCEIL,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vrint,		FVRINT,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_CPU_VEC_FUNC( vj0,	FVJ0,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_CPU_VEC_FUNC( vj1,	FVJ1,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_CPU_VEC_FUNC( vgamma,	FVGAMMA,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_CPU_VEC_FUNC( vlngamma,	FVLNGAMMA,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vacos,		FVACOS,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vasin,		FVASIN,		V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vatn2,		FVATN2,		V_UNARY_C,	M_BP,	RC_MASK	)
CREAT_VEC_FUNC( vuni,		FVUNI,		V_NO_ARGS,	M_BP,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vand,		FVAND,		VV_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vnand,		FVNAND,		VV_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vor,		FVOR,		VV_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vxor,		FVXOR,		VV_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vnot,		FVNOT,		V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vmod,		FVMOD,		VV_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsmod,		FVSMOD,		VS_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsmod2,		FVSMOD2,	VS_BINARY,	M_AI,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vsand,		FVSAND,		VS_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsor,		FVSOR,		VS_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsxor,		FVSXOR,		VS_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vshr,		FVSHR,		VV_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsshr,		FVSSHR,		VS_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsshr2,		FVSSHR2,	VS_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vshl,		FVSHL,		VV_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsshl,		FVSSHL,		VS_BINARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsshl2,		FVSSHL2,	VS_BINARY,	M_AI,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vconv2by,	FVCONV2BY,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2in,	FVCONV2IN,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2di,	FVCONV2DI,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2li,	FVCONV2LI,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2uby,	FVCONV2UBY,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2uin,	FVCONV2UIN,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2udi,	FVCONV2UDI,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2uli,	FVCONV2ULI,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2sp,	FVCONV2SP,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vconv2dp,	FVCONV2DP,	V_UNARY,	M_ALL,	REAL_ARG_MASK	)


/* max mag,	min max changed to M_ALL to allow long destination... */
CREAT_VEC_FUNC( vsum,		FVSUM,		V_PROJECTION,	M_ALL,	RCQ_MASK	)
CREAT_VEC_FUNC( vdot,		FVDOT,		V_PROJECTION2,	M_ALL,	RC_MASK	)
CREAT_CPU_VEC_FUNC( vrand,	FVRAND,		V_UNARY,	M_ALL,	RC_MASK	)

CREAT_VEC_FUNC( vsm_lt,		FVSMLT,		VS_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsm_gt,		FVSMGT,		VS_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsm_le,		FVSMLE,		VS_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsm_ge,		FVSMGE,		VS_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsm_ne,		FVSMNE,		VS_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vsm_eq,		FVSMEQ,		VS_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vvm_lt,		FVVMLT,		VV_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vvm_gt,		FVVMGT,		VV_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vvm_le,		FVVMLE,		VV_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vvm_ge,		FVVMGE,		VV_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vvm_ne,		FVVMNE,		VV_TEST,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vvm_eq,		FVVMEQ,		VV_TEST,	M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vvv_slct,	FVVVSLCT,	VV_SELECT,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vvs_slct,	FVVSSLCT,	VS_SELECT,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vss_slct,	FVSSSLCT,	SS_SELECT,	M_ALL,	REAL_ARG_MASK	)

/* New conditionals */
CREAT_VEC_FUNC( vv_vv_lt,	FVV_VV_LT,	VVVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vv_gt,	FVV_VV_GT,	VVVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vv_le,	FVV_VV_LE,	VVVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vv_ge,	FVV_VV_GE,	VVVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vv_eq,	FVV_VV_EQ,	VVVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vv_ne,	FVV_VV_NE,	VVVVCA,		M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vv_vs_lt,	FVV_VS_LT,	VVVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vs_gt,	FVV_VS_GT,	VVVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vs_le,	FVV_VS_LE,	VVVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vs_ge,	FVV_VS_GE,	VVVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vs_eq,	FVV_VS_EQ,	VVVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vv_vs_ne,	FVV_VS_NE,	VVVSCA,		M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vs_vv_lt,	FVS_VV_LT,	VSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vv_gt,	FVS_VV_GT,	VSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vv_le,	FVS_VV_LE,	VSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vv_ge,	FVS_VV_GE,	VSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vv_eq,	FVS_VV_EQ,	VSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vv_ne,	FVS_VV_NE,	VSVVCA,		M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vs_vs_lt,	FVS_VS_LT,	VSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vs_gt,	FVS_VS_GT,	VSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vs_le,	FVS_VS_LE,	VSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vs_ge,	FVS_VS_GE,	VSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vs_eq,	FVS_VS_EQ,	VSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vs_vs_ne,	FVS_VS_NE,	VSVSCA,		M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( ss_vv_lt,	FSS_VV_LT,	SSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vv_gt,	FSS_VV_GT,	SSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vv_le,	FSS_VV_LE,	SSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vv_ge,	FSS_VV_GE,	SSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vv_eq,	FSS_VV_EQ,	SSVVCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vv_ne,	FSS_VV_NE,	SSVVCA,		M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( ss_vs_lt,	FSS_VS_LT,	SSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vs_gt,	FSS_VS_GT,	SSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vs_le,	FSS_VS_LE,	SSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vs_ge,	FSS_VS_GE,	SSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vs_eq,	FSS_VS_EQ,	SSVSCA,		M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( ss_vs_ne,	FSS_VS_NE,	SSVSCA,		M_ALL,	REAL_ARG_MASK	)

CREAT_CPU_VEC_FUNC( visnan,		FVISNAN,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_CPU_VEC_FUNC( visinf,		FVISINF,	V_UNARY,	M_BP,	REAL_ARG_MASK	)
CREAT_CPU_VEC_FUNC( visnorm,	FVISNORM,	V_UNARY,	M_BP,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vmgsq,		FVMGSQ,		V_UNARY_C,	M_ALL,	CPX_ARG_MASK	)
CREAT_VEC_FUNC( vcmul,		FVCMUL,		VV_BINARY,	M_ALL,	CPX_ARG_MASK	)
CREAT_VEC_FUNC( vscml,		FVSCML,		VS_BINARY,	M_BP,	CPX_ARG_MASK	)
CREAT_VEC_FUNC( vconj,		FVCONJ,		V_UNARY,	M_ALL,	CPX_ARG_MASK	)
CREAT_VEC_FUNC( vfft,		FVFFT,		V_FWD_FFT,	M_BP,	RCM_MASK	)
CREAT_VEC_FUNC( vift,		FVIFT,		V_INV_FFT,	M_BP,	RCM_MASK	)

CREAT_VEC_FUNC( vfft2d,		FVFFT2D,	V_FWD_FFT,	M_BP,	RCM_MASK	)
CREAT_VEC_FUNC( vift2d,		FVIFT2D,	V_INV_FFT,	M_BP,	RCM_MASK	)
CREAT_VEC_FUNC( vfftrows,	FVFFTROWS,	V_FWD_FFT,	M_BP,	RCM_MASK	)
CREAT_VEC_FUNC( viftrows,	FVIFTROWS,	V_INV_FFT,	M_BP,	RCM_MASK	)

CREAT_VEC_FUNC( vbnd,		FVBND,		VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vibnd,		FVIBND,		VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vclip,		FVCLIP,		VS_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( viclp,		FVICLP,		VS_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vcmp,		FVCMP,		VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vscmp,		FVSCMP,		VS_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vscmp2,		FVSCMP2,	VS_BINARY,	M_ALL,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vcomp,		FVCOMP,		V_UNARY,	M_AI,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vtolower,	FVTOLOWER,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vtoupper,	FVTOUPPER,	V_UNARY,	M_AI,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( visprint,	FVISPRINT,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vislower,	FVISLOWER,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( visupper,	FVISUPPER,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( visalnum,	FVISALNUM,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( visalpha,	FVISALPHA,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( visdigit,	FVISDIGIT,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( visspace,	FVISSPACE,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( visblank,	FVISBLANK,	V_UNARY,	M_AI,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( viscntrl,	FVISCNTRL,	V_UNARY,	M_AI,	REAL_ARG_MASK	)

CREAT_VEC_FUNC( vlutmapb,	FVLUTMAPB,	VV_BINARY,	M_ALL,	REAL_ARG_MASK	)
CREAT_VEC_FUNC( vlutmaps,	FVLUTMAPS,	VV_BINARY,	M_ALL,	REAL_ARG_MASK	)

END_VFUNC_DECLS


// In the old coding, we created items and copied their values from the table...
// But we don't need to have two copies - we just have to insert the names

#define N_NVFS		(sizeof(vec_func_tbl)/sizeof(Vector_Function))


static void create_vfs(SINGLE_QSP_ARG_DECL)
{
	u_int i;

	init_vec_funcs();	// init item type

	for(i=0;i<N_NVFS;i++){
		add_item(vec_func_itp, &vec_func_tbl[i] );
	}
}

static int vf_cmp(const void *vfp1,const void *vfp2)
{
	if( ((const Vector_Function *)vfp1)->vf_code > ((const Vector_Function *)vfp2)->vf_code ) return(1);
	else return(-1);
}

static int vfa_cmp(const void *vfp1,const void *vfp2)
{
	if( ((const Vec_Func_Array *)vfp1)->vfa_code > ((const Vec_Func_Array *)vfp2)->vfa_code ) return(1);
	else return(-1);
}

int _check_vfa_tbl_size(QSP_ARG_DECL  Vec_Func_Array vfa_tbl[], size_t size)
{
	assert( size == N_VEC_FUNCS );
	return 0;
}

void _check_vfa_tbl(QSP_ARG_DECL  Vec_Func_Array *vfa_tbl, size_t size)
{
	int i;
	if( size != N_VEC_FUNCS ){
		snprintf(ERROR_STRING,LLEN,"check_vfa_tbl:  size (%ld) not equal to N_VEC_FUNCS (%d)!?",
			size,N_VEC_FUNCS);
		warn(ERROR_STRING);
	}

	qsort(vfa_tbl,size,sizeof(Vec_Func_Array),vfa_cmp);

#ifdef CAUTIOUS
	/* make sure the table is complete */
	for(i=0;i<size;i++){

/*
if( verbose ){
snprintf(ERROR_STRING,LLEN,"check_vfa_tbl:  vfa_tbl[%d] (%s):  code = %d (%s)",
i, VF_NAME(&vec_func_tbl[i]),
vfa_tbl[i].vfa_code,
VF_NAME(&vec_func_tbl[ vfa_tbl[i].vfa_code ])
);
advise(ERROR_STRING);
}
*/

		if( vfa_tbl[i].vfa_code != i ){
			snprintf(ERROR_STRING,LLEN,
	"CAUTIOUS:  check_vfa_tbl:  Vec_Func_Array table entry %d (%s) has code %d (%s)!?",
		i,VF_NAME(&vec_func_tbl[i]),
		vfa_tbl[i].vfa_code,VF_NAME(&vec_func_tbl[ vfa_tbl[i].vfa_code ]) );
			warn(ERROR_STRING);
		}

		assert( vfa_tbl[i].vfa_code == i );
	}
#endif /* CAUTIOUS */
}

void vl_init(SINGLE_QSP_ARG_DECL)
{
	static int inited=0;
	int i;

	if( inited ){
		// Don't warn, could be called from warmenu & vl_menu...
		/*warn("vl_init:  already initialized"); */
		return;
	}

#ifdef QUIP_DEBUG
	if( veclib_debug == 0 )
		veclib_debug = add_debug_module("veclib");
#endif // QUIP_DEBUG

	/* sort the table to insure that each entry is at the location of its code */
#ifdef CAUTIOUS
	if( N_VEC_FUNCS != N_NVFS ){
		snprintf(ERROR_STRING,LLEN,"CAUTIOUS:  vl_init:  Vector function table is missing %zd entries!?",
			N_VEC_FUNCS-N_NVFS);
		error1(ERROR_STRING);
	}
#endif // CAUTIOUS

	assert( N_VEC_FUNCS == N_NVFS );

	qsort(vec_func_tbl,N_NVFS,sizeof(Vector_Function),vf_cmp);

#ifdef CAUTIOUS
	/* make sure the table is complete */
	for(i=0;i<N_NVFS;i++){

if( verbose ){
snprintf(ERROR_STRING,LLEN,"vl_init:  vec_func_tbl[%d] (%s):  code %d (%s)",
i, VF_NAME(&vec_func_tbl[i]),
VF_CODE(&vec_func_tbl[i]), VF_NAME(&vec_func_tbl[ VF_CODE(&vec_func_tbl[i]) ])
);
advise(ERROR_STRING);
}

		assert( VF_CODE(&vec_func_tbl[i]) == i );
	}
#endif /* CAUTIOUS */

	init_argset_objects(SINGLE_QSP_ARG);

	// Initialize the platforms
	//init_all_platforms(SINGLE_QSP_ARG);
	vl2_init_platform(SINGLE_QSP_ARG);

	/* now create some items */

	create_vfs(SINGLE_QSP_ARG);

	declare_vector_functions();

	/* We used to have these in a table, indexed by their code, for fast lookup...
	 * Would that be a good idea still?
	 */

	set_random_seed();	/* use low order bits of microsecond clock */

	inited++;
}

