//#include "quip_config.h"
//#include "quip_prot.h"
//#include "veclib/vecgen.h"
//#include "veclib/obj_args.h"
#include "veclib/bitmap_ops.h"


/* Bitmaps are normally represented in u_longs, which can be 32 or 64 bits,
 * depending on the host architecture.
 * That makes it most efficient for doing bitwise ops on the host.
 * But for GPU computations, where we do each bit of the word
 * sequentially, and all of the words in parallel, it might be more
 * efficient to use a shorter word...
 *
 * The necessary constants are defined in data_obj.h, and include:
 * BITS_PER_BITMAP_WORD			32 or 64
 */

/* 1/6/15	bug fix
 *
 * We can't advance the bit after the last operation, because
 * that can cause a read outside of the data object.
 * This only showed up as a seg violation with 1024x1024 images,
 * maybe because their data fell on a page boundary...
 */

#define ADVANCE_BIT( bit, lp, l )					\
									\
		if( bit == BITMAP_WORD_MSB ){				\
			bit = 1;					\
			lp++;						\
			l = *lp;					\
		} else {						\
			bit <<= 1;					\
		}

#define ADVANCE_DEST_BIT( bit, lp )					\
									\
		if( bit == BITMAP_WORD_MSB ){				\
			bit = 1;					\
			lp++;						\
		} else {						\
			bit <<= 1;					\
		}

#define BITMAP_OBJ_BINARY_FUNC( funcname, statement )				\
void funcname( HOST_CALL_ARG_DECLS )						\
{										\
	bitmap_word *src1_ptr,*src2_ptr,*dst_ptr;				\
	bitmap_word l1,l2/*,l3*/;							\
	bitmap_word src1_bit,src2_bit,dst_bit;					\
	dimension_t n;								\
										\
	n = OBJ_N_TYPE_ELTS( OA_DEST(oap) );					\
										\
	src1_ptr=(bitmap_word *)OBJ_DATA_PTR( OA_SRC_OBJ(oap,0) );		\
	src2_ptr=(bitmap_word *)OBJ_DATA_PTR( OA_SRC_OBJ(oap,1) );		\
	dst_ptr=(bitmap_word *)OBJ_DATA_PTR( OA_DEST(oap) );			\
										\
	src1_bit = 1 << OBJ_BIT0( OA_SRC_OBJ(oap,0) );				\
	src2_bit = 1 << OBJ_BIT0( OA_SRC_OBJ(oap,1) );				\
	dst_bit = 1 << OBJ_BIT0( OA_DEST(oap) );				\
										\
	l1=(*src1_ptr);								\
	l2=(*src2_ptr);								\
	/*l3=(*dst_ptr);*/							\
	n--;									\
	while(n--){								\
		statement							\
		ADVANCE_BIT(src1_bit,src1_ptr,l1)				\
		ADVANCE_BIT(src2_bit,src2_ptr,l2)				\
		ADVANCE_DEST_BIT(dst_bit,dst_ptr)				\
	}									\
	statement								\
}


#define BITMAP_OBJ_VS_FUNC( funcname, statement )				\
void funcname( HOST_CALL_ARG_DECLS )						\
{										\
	bitmap_word *src1_ptr,*dst_ptr;						\
	bitmap_word l1/*,l3*/;							\
	bitmap_word src1_bit,dst_bit;						\
	bitmap_word scalar_bit; /* really just a boolean */			\
	dimension_t n;								\
										\
	n = OBJ_N_TYPE_ELTS( OA_DEST(oap) );					\
										\
	src1_ptr=(bitmap_word *)OBJ_DATA_PTR( OA_SRC_OBJ(oap,0) );		\
	scalar_bit=OA_SVAL(oap,0)->u_bit;					\
	dst_ptr=(bitmap_word *)OBJ_DATA_PTR( OA_DEST(oap) );			\
										\
	src1_bit = 1 << OBJ_BIT0( OA_SRC_OBJ(oap,0) );				\
	dst_bit = 1 << OBJ_BIT0( OA_DEST(oap) );				\
										\
	l1=(*src1_ptr);								\
	/*l3=(*dst_ptr);*/							\
	n--;									\
	while(n--){								\
		statement							\
										\
		ADVANCE_BIT(src1_bit,src1_ptr,l1)				\
		ADVANCE_DEST_BIT(dst_bit,dst_ptr)				\
	}									\
	statement								\
}

#define BITMAP_OBJ_UNARY_FUNC( funcname, statement )				\
										\
void funcname( HOST_CALL_ARG_DECLS )						\
{										\
	bitmap_word *src1_ptr,*dst_ptr;						\
	bitmap_word l1/*,l2*/;							\
	bitmap_word src1_bit,dst_bit;						\
	dimension_t n;								\
										\
	n = OBJ_N_TYPE_ELTS( OA_SRC_OBJ(oap,0) );				\
										\
	src1_ptr=(bitmap_word *)OBJ_DATA_PTR( OA_SRC_OBJ(oap,0) );		\
	dst_ptr=(bitmap_word *)OBJ_DATA_PTR( OA_DEST(oap) );			\
										\
	src1_bit = 1 << OBJ_BIT0( OA_SRC_OBJ(oap,0) );				\
	dst_bit = 1 << OBJ_BIT0( OA_DEST(oap) );				\
										\
										\
	l1=(*src1_ptr);								\
	/*l2=(*dst_ptr);*/							\
	n--;									\
	while(n--){								\
		statement							\
										\
		ADVANCE_BIT(src1_bit,src1_ptr,l1)				\
		ADVANCE_DEST_BIT(dst_bit,dst_ptr)				\
	}									\
	statement								\
}


BITMAP_OBJ_UNARY_FUNC( HOST_TYPED_CALL_NAME(vnot,bit), 
		if( l1 & src1_bit ) *dst_ptr &= ~dst_bit;
		else *dst_ptr |= dst_bit; )

BITMAP_OBJ_BINARY_FUNC( HOST_TYPED_CALL_NAME(vand,bit), 
	if( (l1 & src1_bit) && (l2 & src2_bit) ) *dst_ptr |= dst_bit; else *dst_ptr &= ~dst_bit; )

BITMAP_OBJ_BINARY_FUNC( HOST_TYPED_CALL_NAME(vnand,bit),
	if( (l1 & src1_bit) && (l2 & src2_bit) ) *dst_ptr &= ~dst_bit; else *dst_ptr |= dst_bit; )

BITMAP_OBJ_BINARY_FUNC( HOST_TYPED_CALL_NAME(vor,bit),
	if( (l1 & src1_bit) || (l2 & src2_bit) ) *dst_ptr |= dst_bit; else *dst_ptr &= ~dst_bit; )

BITMAP_OBJ_BINARY_FUNC( HOST_TYPED_CALL_NAME(vxor,bit),
	if( ( (l1 & src1_bit) && !(l2 & src2_bit) ) ||
	    (!(l1 & src1_bit) &&  (l2 & src2_bit) ) ) *dst_ptr |= dst_bit; else *dst_ptr &= ~dst_bit; )

BITMAP_OBJ_VS_FUNC( HOST_TYPED_CALL_NAME(vsand,bit),
	if( (l1 & src1_bit) && scalar_bit ) *dst_ptr |= dst_bit; else *dst_ptr &= ~dst_bit; )

BITMAP_OBJ_VS_FUNC( HOST_TYPED_CALL_NAME(vsor,bit),
	if( (l1 & src1_bit) || scalar_bit ) *dst_ptr |= dst_bit; else *dst_ptr &= ~dst_bit; )

BITMAP_OBJ_VS_FUNC( HOST_TYPED_CALL_NAME(vsxor,bit),
	if( l1 & src1_bit ){ if( ! scalar_bit ) *dst_ptr |= dst_bit; else *dst_ptr &= ~dst_bit; } else { if( scalar_bit ) *dst_ptr |= dst_bit; else *dst_ptr &= ~dst_bit; } )

/* This is the same as vnot - much more efficient to complement the long words! */

BITMAP_OBJ_UNARY_FUNC( HOST_TYPED_CALL_NAME(vcomp,bit), 
		if( l1 & src1_bit ) *dst_ptr &= ~dst_bit;
		else *dst_ptr |= dst_bit; )

