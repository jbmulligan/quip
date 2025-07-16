/* ocl_port.m4 BEGIN */

include(`../../include/veclib/gen_port.m4')

suppress_if

define(`BUILD_FOR_GPU',`')
define(`BUILD_FOR_OPENCL',`')
define(`QUATERNION_SUPPORT',`')
ifdef(`QUATERNION_SUPPORT',`
#define QUATERNION_SUPPORT
',`')

define(`pf_str',`ocl')
define(`KERNEL_ARG_QUALIFIER',`__global')
define(`OS_ARG',GEN_SEP($1) $1`_'$2`_OFFSET')

define(`DECLARE_KERN_ARGS_DEST_OFFSET',`MTL_OFFSET_TYPE a_offset ')
define(`DECLARE_KERN_ARGS_SRC1_OFFSET',`MTL_OFFSET_TYPE b_offset')
define(`DECLARE_KERN_ARGS_BSRC1_OFFSET',`MTL_OFFSET_TYPE b_offset')
define(`DECLARE_KERN_ARGS_SSRC1_OFFSET',`MTL_OFFSET_TYPE b_offset')
define(`DECLARE_KERN_ARGS_SRC2_OFFSET',`MTL_OFFSET_TYPE c_offset')
define(`DECLARE_KERN_ARGS_MAP_OFFSET',`MTL_OFFSET_TYPE map_offset')
define(`DECLARE_KERN_ARGS_SRC3_OFFSET',`MTL_OFFSET_TYPE d_offset')
define(`DECLARE_KERN_ARGS_SRC4_OFFSET',`MTL_OFFSET_TYPE e_offset')

dnl define(`DECLARE_KERN_ARGS_DBM_OFFSET',`MTL_OFFSET_TYPE dbm_offset')
dnl define(`DECLARE_KERN_ARGS_SBM_OFFSET',`MTL_OFFSET_TYPE sbm_offset')

define(`OFFSET_A',`+ a_offset')
define(`OFFSET_B',`+ b_offset')
define(`OFFSET_C',`+ c_offset')
define(`OFFSET_D',`+ d_offset')
define(`OFFSET_E',`+ e_offset')

define(`THREAD_INDEX_X',`get_global_id(0)')
dnl define(`SET_INDEX',$1 = get_global_id(0);)
define(`MTL_OFFSET_TYPE',`int')

suppress_no

#include "platform.h"
extern void *TMPVEC_NAME`(QSP_ARG_DECL  Platform_Device *pdp, size_t size, size_t len, const char *whence);'
extern void FREETMP_NAME`(QSP_ARG_DECL  void *a, const char *whence);'
extern int get_max_threads_per_block(Data_Obj *odp);
extern int max_threads_per_block;

/* ocl_port.m4 END */

