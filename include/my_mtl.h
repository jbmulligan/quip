#ifndef _MY_MTL_H_
#define _MY_MTL_H_


#include "platform.h"	// MAX_DEVICES_PER_PLATFORM

#ifdef HAVE_METAL

#define MAX_METAL_DEVICES	4

#import <Metal/Metal.h>

#include "item_type.h"
#include "veclib/vec_func.h"
#include "veclib/obj_args.h"

#include "veclib/mtl_veclib_prot.h"	// BUILD_FOR_GPU, BUILD_FOR_CUDA
//#include "platform.h"

#include "veclib_api.h"

extern Vec_Func_Array mtl_vfa_tbl[N_VEC_FUNCS];

/* prototypes here */


extern void _ensure_mtl_device( QSP_ARG_DECL   Data_Obj *dp );

extern id<MTLDevice> _get_metal_dev(SINGLE_QSP_ARG_DECL);
#define get_metal_dev()	_get_metal_dev(SINGLE_QSP_ARG)

#define NO_METAL_MSG(whence)						\
									\
	snprintf(ERROR_STRING,LLEN,					\
		"%s:  Sorry, no Metal support in this build.",#whence);	\
	advise(ERROR_STRING);

#define index_type	int32_t	// for vmaxi etc
#define INDEX_PREC	PREC_DI	// for vmaxi etc

extern void * _mtl_make_kernel( QSP_ARG_DECL  const char *src, const char *name, Platform_Device *pdp );
#define mtl_make_kernel(src,name,pdp) _mtl_make_kernel(QSP_ARG  src,name,pdp )

extern void h_mtl_set_seed(int seed);

#endif // HAVE_METAL
#endif // ! _MY_MTL_H_

