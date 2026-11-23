/* mtl_port.m4 BEGIN */

include(`../../include/veclib/gen_port.m4')

suppress_if

define(`BUILD_FOR_GPU',`')
define(`BUILD_FOR_OPENCL',`')
define(`QUATERNION_SUPPORT',`')
ifdef(`QUATERNION_SUPPORT',`
#define QUATERNION_SUPPORT
',`')

define(`pf_str',`mtl')
define(`KERNEL_ARG_QUALIFIER',`device')
define(`OS_ARG',GEN_SEP($1) $1`_'$2`_OFFSET')

dnl  A metal-ism...
define(`DECLARE_THREAD_ID',`, uint id [[thread_position_in_grid]]')

define(`DECL_OFFSETS_',`')
define(`SET_BITMAP_OFFSETS1_',`')
define(`SET_BITMAP_OFFSETS2_',`')
define(`DECL_OFFSETS_1',`				\
	uint offset1;					\n\
')

define(`SET_OFFSETS_',`')
define(`SET_OFFSETS_1',`				\
	offset1 = *offset1_p;				\n\
')

define(`INIT_SCALARS',`DECL_SCALARS($1) SET_SCALARS($1)')
define(`DECL_SCALARS',`_DECL_SCALARS$1')

define(`_DECL_SCALARS',`')

define(`_DECL_SCALARS_1S',`				\
	std_type scalar1_val;				\
')

define(`_DECL_SCALARS_2S',`				\
	_DECL_SCALARS_1S					\
	std_type scalar2_val;				\
')

define(`_DECL_SCALARS_3S',`				\
	_DECL_SCALARS_2S					\
	std_type scalar3_val;				\
')

define(`SET_SCALARS',`_SET_SCALARS$1')

define(`_SET_SCALARS',`')

define(`_SET_SCALARS_1S',`				\
	scalar1_val = *scalar1_val_p;			\
')

define(`_SET_SCALARS_2S',`				\
	_SET_SCALARS_1S					\
	scalar2_val = *scalar2_val_p;			\
')

define(`_SET_SCALARS_3S',`				\
	_SET_SCALARS_2S					\
	scalar3_val = *scalar3_val_p;			\
')

dnl  It would be nice for these to be defined algorithmically?
define(`MTL_DO_BUF_IDX',`0')
define(`MTL_SO1_BUF_IDX',`1')
define(`MTL_SO2_BUF_IDX',`2')
define(`MTL_SO3_BUF_IDX',`3')
define(`MTL_SO4_BUF_IDX',`4')
define(`MTL_MO_BUF_IDX',`5')
define(`MTL_DEST_BUF_IDX',`6')
define(`MTL_SRC1_BUF_IDX',`7')
define(`MTL_SRC2_BUF_IDX',`8')
define(`MTL_SRC3_BUF_IDX',`9')
define(`MTL_SRC4_BUF_IDX',`10')
define(`MTL_SCLR1_BUF_IDX',`11')
define(`MTL_SCLR2_BUF_IDX',`12')
define(`MTL_SCLR3_BUF_IDX',`13')

define(`DEST_BUF_SPEC',`[[ buffer(MTL_DEST_BUF_IDX) ]]')
define(`SRC1_BUF_SPEC',`[[ buffer(MTL_SRC1_BUF_IDX) ]]')
define(`SRC2_BUF_SPEC',`[[ buffer(MTL_SRC2_BUF_IDX) ]]')
define(`SRC3_BUF_SPEC',`[[ buffer(MTL_SRC3_BUF_IDX) ]]')
define(`SRC4_BUF_SPEC',`[[ buffer(MTL_SRC4_BUF_IDX) ]]')
define(`SCLR1_BUF_SPEC',`[[ buffer(MTL_SCLR1_BUF_IDX) ]]')
define(`SCLR2_BUF_SPEC',`[[ buffer(MTL_SCLR2_BUF_IDX) ]]')
define(`SCLR3_BUF_SPEC',`[[ buffer(MTL_SCLR3_BUF_IDX) ]]')

dnl define(`DECL_INDICES_SRC1',`GPU_INDEX_TYPE index2;')
dnl define(`DECL_INDICES_SRC2',`GPU_INDEX_TYPE index3;')
dnl define(`DECL_INDICES_SRC3',`GPU_INDEX_TYPE index4;')
dnl define(`DECL_INDICES_SRC4',`GPU_INDEX_TYPE index5;')
dnl define(`DECL_INDICES_SBM',`GPU_INDEX_TYPE sbm_bit_offset;')

define(`DECLARE_KERN_ARGS_DEST_OFFSET',`MTL_OFFSET_TYPE offset1_p [[ buffer(MTL_DO_BUF_IDX) ]] ')
define(`DECLARE_KERN_ARGS_SRC1_OFFSET',`MTL_OFFSET_TYPE offset2_p [[ buffer(MTL_SO1_BUF_IDX) ]]')
define(`DECLARE_KERN_ARGS_BSRC1_OFFSET',`MTL_OFFSET_TYPE offset2_p [[ buffer(MTL_SO1_BUF_IDX) ]]')
define(`DECLARE_KERN_ARGS_SSRC1_OFFSET',`MTL_OFFSET_TYPE offset2_p [[ buffer(MTL_SO1_BUF_IDX) ]]')
define(`DECLARE_KERN_ARGS_SRC2_OFFSET',`MTL_OFFSET_TYPE offset3_p [[ buffer(MTL_SO2_BUF_IDX) ]]')
define(`DECLARE_KERN_ARGS_MAP_OFFSET',`MTL_OFFSET_TYPE map_offset_p [[ buffer(MTL_MO_BUF_IDX) ]]')
define(`DECLARE_KERN_ARGS_SRC3_OFFSET',`MTL_OFFSET_TYPE offset4_p [[ buffer(MTL_SO3_BUF_IDX) ]]')
define(`DECLARE_KERN_ARGS_SRC4_OFFSET',`MTL_OFFSET_TYPE offset5_p [[ buffer(MTL_SO4_BUF_IDX) ]]')

dnl define(`DECLARE_KERN_ARGS_DBM_OFFSET',`MTL_OFFSET_TYPE dbm_offset MTL_DBM_BUF_IDX')
dnl define(`DECLARE_KERN_ARGS_SBM_OFFSET',`MTL_OFFSET_TYPE sbm_offset MTL_SBM_BUF_IDX')

define(`DECLARE_KERN_ARGS_1S',`device std_type *scalar1_val_p SCLR1_BUF_SPEC')
define(`DECLARE_KERN_ARGS_2S',`DECLARE_KERN_ARGS_1S, device std_type *scalar2_val_p SCLR2_BUF_SPEC')
define(`DECLARE_KERN_ARGS_3S',`DECLARE_KERN_ARGS_2S, device std_type *scalar3_val_p SCLR3_BUF_SPEC')

define(`OFFSET_A',`+ offset1')
define(`OFFSET_B',`+ offset2')
define(`OFFSET_C',`+ offset3')
define(`OFFSET_D',`+ offset4')
define(`OFFSET_E',`+ offset5')

define(`THREAD_INDEX_X',`id')
define(`MTL_OFFSET_TYPE',`device int *')

suppress_no

#include "platform.h"
extern void *TMPVEC_NAME`(QSP_ARG_DECL  Platform_Device *pdp, size_t size, size_t len, const char *whence);'
extern void FREETMP_NAME`(QSP_ARG_DECL  void *a, const char *whence);'
extern int get_max_threads_per_block(Data_Obj *odp);
extern int max_threads_per_block;
extern id<MTLDevice> mtl_device;
extern id<MTLBuffer> mtl_buf_for(const void *);

/* mtl_port.m4 END */

