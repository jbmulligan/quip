
/* mtl_kern_call_defs.m4 BEGIN */

// These definitions are expanded during C compilation, but are not
// compiled - they are stored to strings and compiled on the fly.

// We have a problem because some gpu's can't handle double (e.g. Iris Pro)
// The typedefs here cause problems, even when the kernel doesn't use them.
// So we take them out and include only when needed...

// BUG the structure definitions have to be kept in sync with the definitions in the header files...
// How could we write this in a way that would ensure this?
// See shape_info.h

define(`KERNEL_FUNC_PRELUDE',`					\n\
								\n\
#include <metal_stdlib>						\n\
using namespace metal;						\n\
								\n\
typedef unsigned char u_char;					\n\
typedef unsigned short u_short;					\n\
typedef unsigned long uint64_t;					\n\
typedef unsigned int uint32_t;					\n\
typedef long int64_t;						\n\
typedef uint32_t dimension_t;					\n\
typedef int int32_t;						\n\
typedef int index_type;						\n\
/*typedef struct { int x; int y; int z; } dim3 ;*/		\n\
typedef struct { int d5_dim[5]; } dim5 ;			\n\
typedef unsigned long bitmap_word;				\n\
typedef struct {						\n\
	uint32_t	word_offset;				\n\
	uint32_t	first_index[5];				\n\
	uint64_t	first_bit_num;				\n\
	bitmap_word	valid_bits;				\n\
} Bitmap_GPU_Word_Info;						\n\
typedef struct {						\n\
	uint32_t			n_bitmap_words;		\n\
	uint32_t			total_size;		\n\
	int32_t				obj_size[5];		\n\
	Bitmap_GPU_Word_Info 		word_tbl[1];		\n\
} Bitmap_GPU_Info;						\n\
EXTRA_PRELUDE(type_code)					\n\
')

define(`EXTRA_PRELUDE',EXTRA_PRELUDE_$1)

define(`EXTRA_PRELUDE_by',`')
define(`EXTRA_PRELUDE_in',`')
define(`EXTRA_PRELUDE_di',`')
define(`EXTRA_PRELUDE_li',`')
define(`EXTRA_PRELUDE_uby',`')
define(`EXTRA_PRELUDE_uin',`')
define(`EXTRA_PRELUDE_udi',`')
define(`EXTRA_PRELUDE_uli',`')
define(`EXTRA_PRELUDE_ubyin',`')
define(`EXTRA_PRELUDE_uindi',`')
define(`EXTRA_PRELUDE_inby',`')
define(`EXTRA_PRELUDE_bit',`')
define(`EXTRA_PRELUDE_spdp',EXTRA_PRELUDE_sp EXTRA_PRELUDE_dp)

define(`EXTRA_PRELUDE_sp',`						\n\
									\n\
typedef struct { float re; float im; } SP_Complex;			\n\
typedef struct { float re; float _i; float _j; float _k; } SP_Quaternion;\n\
')

define(`EXTRA_PRELUDE_dp',`						\n\
									\n\
typedef struct { double re; double im; } DP_Complex;			\n\
typedef struct { double re; double _i; double _j; double _k; } DP_Quaternion;\n\
')

/* mtl_kern_call_defs.m4 END */

