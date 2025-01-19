
dnl	the subsidiary defns need to come first!

/* slow_defs defining gpu_index_type */
define(`GPU_INDEX_TYPE',`SLOW_GPU_INDEX_TYPE')
define(`RAMP_IDX',`INDEX_SUM(ramp_idx)')
dnl	define(`DECLARE_KERN_ARGS_DBM',`KERNEL_ARG_QUALIFIER bitmap_word *dbm, DECLARE_KERN_ARGS_DBM_GPU_INFO')

define(`DECLARE_DBM_INDEX',`')

define(`SCALE_INDEX',`					\
	$1.d5_dim[0] *= $2.d5_dim[0];					\
	$1.d5_dim[1] *= $2.d5_dim[1];					\
	$1.d5_dim[2] *= $2.d5_dim[2];					\
	$1.d5_dim[3] *= $2.d5_dim[3];					\
	$1.d5_dim[4] *= $2.d5_dim[4];					\
')


define(`dst',`slow_dst')
define(`src1',`slow_src1')
define(`src2',`slow_src2')
define(`src3',`slow_src3')
define(`src4',`slow_src4')

define(`cdst',`slow_cdst')
define(`csrc1',`slow_csrc1')
define(`csrc2',`slow_csrc2')
define(`csrc3',`slow_csrc3')
define(`csrc4',`slow_csrc4')

define(`qdst',`slow_qdst')
define(`qsrc1',`slow_qsrc1')
define(`qsrc2',`slow_qsrc2')
define(`qsrc3',`slow_qsrc3')
define(`qsrc4',`slow_qsrc4')

dnl	SET_INDEX(this_index)

dnl	What if we have to have blocks in 2 or more dims?
dnl	The current approach is that we always ask for a 1-D array
dnl	of kernel threads, and let the kernel logic sort out x and y.
dnl	BUT we need to ensure that we don't have a problem with CUDA
dnl	or OpenCL regarding the maximum thread array size???

define(`SET_INDEX',`							\
									\
	$1.d5_dim[0] = THREAD_INDEX_X;					\
	$1.d5_dim[1] = $1.d5_dim[0] / szarr.d5_dim[0];			\
	$1.d5_dim[2] = $1.d5_dim[1] / szarr.d5_dim[1];			\
	$1.d5_dim[3] = $1.d5_dim[2] / szarr.d5_dim[2];			\
	$1.d5_dim[4] = $1.d5_dim[3] / szarr.d5_dim[3];			\
	$1.d5_dim[0] %= szarr.d5_dim[0];				\
	$1.d5_dim[1] %= szarr.d5_dim[1];				\
	$1.d5_dim[2] %= szarr.d5_dim[2];				\
	$1.d5_dim[3] %= szarr.d5_dim[3];				\
	$1.d5_dim[4] %= szarr.d5_dim[4];				\
')

dnl	this is for bitmaps with info...
define(`SET_INDICES_1SRC',`index2.d5_dim[0] = tbl_idx; /* BUG need to initialize index2! */')
define(`SET_INDICES_2SRCS',`SET_INDICES_1SRC index3 = index2;')


dnl	Slow versions of these not implemented for GPU yet...
dnl	BUT fast version not for CPU!?
dnl	need to provide stubs in platform-specific files

define(`_VEC_FUNC_MM_NOCC',`_VEC_FUNC_SLOW_MM_NOCC($1,$2,$3,$4,$5,$6)')
define(`_VEC_FUNC_2V_PROJ',`_VEC_FUNC_SLOW_2V_PROJ($1,$2,$3,$4)')
define(`_VEC_FUNC_CPX_2V_PROJ',`_VEC_FUNC_SLOW_CPX_2V_PROJ($1,$2,$3,$4,$5)')
define(`_VEC_FUNC_QUAT_2V_PROJ',`_VEC_FUNC_SLOW_QUAT_2V_PROJ($1,$2,$3,$4,$5)')
define(`_VEC_FUNC_2V_PROJ_IDX',`_VEC_FUNC_SLOW_2V_PROJ_IDX($1,$2,$3,$4,$5)')
dnl	define(`_VEC_FUNC_3V_PROJ',`__VEC_FUNC_3V_PROJ($1,`',$2,$3,$4,$5)')
define(`_VEC_FUNC_3V_PROJ',`_VEC_FUNC_SLOW_3V_PROJ($1,`',$2,$3,$4,$5)')
define(`_VEC_FUNC_CPX_3V_PROJ',`_VEC_FUNC_SLOW_3V_PROJ($1,`CPX_',$2,$3,$4,$5)')

dnl	GPU-only stuff???
define(`IDX1',`(INDEX_SUM(index1))')
define(`IDX1_1',`(index1.d5_dim[1])')
define(`IDX1_2',`(index1.d5_dim[2])')
define(`INC1_1',`inc1.d5_dim[1]')
define(`INC1_2',`inc1.d5_dim[2]')

define(`BITMAP_ROW_IDX',`(i_dbm_word/words_per_row)')
define(`IDX_WITHIN_ROW',`((i_dbm_word % words_per_row)*BITS_PER_BITMAP_WORD)')
define(`WORDS_PER_FRAME',`(words_per_row * szarr.d5_dim[2])')
define(`WORDS_PER_SEQ',`(words_per_row * szarr.d5_dim[2] * szarr.d5_dim[3])')

define(`SET_INDICES_DBM_1S_',`SET_BASIC_INDICES_DBM')

dnl	Backslashes required for openCL (source strings)?

define(`SET_INDICES_DBM',`				\
	SET_BASIC_INDICES_DBM				\
')

// For slow bitmap-to-bitmap operations, we need to know the x y etc coordinates
// in order to compute the source bit.  sbm1_bit_offset etc
// This is complicated because each thread handles a bitmap word,
// which can span different rows - so the index has to be updated
// for each iteration.  This may required adding more fields to bitmap_gpu_info:
// namely the dimensions of the whole thing, and the starting point (coordinates) for each word.


define(`SET_BASIC_INDICES_DBM',`				\
	tbl_idx = THREAD_INDEX_X;				\
  	i_dbm_word = dbm_info_p->word_tbl[tbl_idx].word_offset;				\
')

define(`DECL_BASIC_INDICES_DBM',`				\
	unsigned int i_dbm_bit;					\
	int i_dbm_word; bitmap_word dbm_bit;			\
	int tbl_idx;						\
')

dnl	If we are using sbm1 or sbm2, then we assume that we have
dnl	a destination bitmap...
dnl
dnl	Each thread needs to know the coordinates of the source
dnl	bit in order to calculate the bit number.  To do that
dnl	we need to know the dimensions of the target image.
dnl	(At the moment we are not worrying about outer ops!)
dnl	The dimensions of the object are passed in dbm_bitmap_gpu_info
dnl	(or whatever it is called?), while each word has
dnl	an array first_indices.  After we obtain the word index,
dnl	we initialize sbm1_bit_offset from first indices;
dnl	as we advance through the loop, we need to increment,
dnl	(the lowest non-1 dimension) and possibly carry
dnl	to the next highest non-1 dimension...  messy!
dnl
dnl	We would like to assume that the initialization is
dnl	happening AFTER dbm initialization, so we can use
dnl	i_dbm_word and tbl_idx...

define(`SET_INDICES_1SBM',`							\
	sbm1_bit_offset.d5_dim[0] = dbm_info_p->word_tbl[tbl_idx].first_index[0];		\
	sbm1_bit_offset.d5_dim[1] = dbm_info_p->word_tbl[tbl_idx].first_index[1];		\
	sbm1_bit_offset.d5_dim[2] = dbm_info_p->word_tbl[tbl_idx].first_index[2];		\
	sbm1_bit_offset.d5_dim[3] = dbm_info_p->word_tbl[tbl_idx].first_index[3];		\
	sbm1_bit_offset.d5_dim[4] = dbm_info_p->word_tbl[tbl_idx].first_index[4];		\
')

define(`SET_INDICES_2SBM',`							\
	sbm2_bit_offset.d5_dim[0] = dbm_info_p->word_tbl[tbl_idx].first_index[0];		\
	sbm2_bit_offset.d5_dim[1] = dbm_info_p->word_tbl[tbl_idx].first_index[1];		\
	sbm2_bit_offset.d5_dim[2] = dbm_info_p->word_tbl[tbl_idx].first_index[2];		\
	sbm2_bit_offset.d5_dim[3] = dbm_info_p->word_tbl[tbl_idx].first_index[3];		\
	sbm2_bit_offset.d5_dim[4] = dbm_info_p->word_tbl[tbl_idx].first_index[4];		\
')

define(`DECL_INDICES_SBM1',`GPU_INDEX_TYPE sbm1_bit_offset; int i; int need_carry;')
define(`DECL_INDICES_SBM2',`GPU_INDEX_TYPE sbm2_bit_offset;')


dnl	 We need to know if we should do this bit...
dnl	 From these definitions, it is not clear whether the rows are padded to be an 
dnl	 integral number of words...
dnl	
dnl	 We assume that i_dbm_word is initilized to dbm_bit_idx.x, before upscaling to the bit index.
dnl	 Here we add the row offset
dnl	 But when adjust is called, the y increment has already been scaled.
dnl	 should dbm_bit_idx have more than one dimension or not???

define(`SET_SBM_WORD_IDX',`i_sbm_word=(sbm_bit_offset.d5_dim[1]+sbm_bit_offset.d5_dim[2])/BITS_PER_BITMAP_WORD;')

dnl	This was old GPU defn???
ifdef(`BUILD_FOR_GPU',`
define(`srcbit',`(sbm_ptr[(INDEX_SUM(sbm_bit_offset)+sbm_bit0)>>LOG2_BITS_PER_BITMAP_WORD] & NUMBERED_BIT((INDEX_SUM(sbm_bit_offset)+sbm_bit0)&(BITS_PER_BITMAP_WORD-1)))')
define(`srcbit1',`(sbm1_ptr[(INDEX_SUM(sbm1_bit_offset)+sbm1_bit0)>>LOG2_BITS_PER_BITMAP_WORD] & NUMBERED_BIT((INDEX_SUM(sbm1_bit_offset)+sbm1_bit0)&(BITS_PER_BITMAP_WORD-1)))')
define(`srcbit2',`(sbm2_ptr[(INDEX_SUM(sbm2_bit_offset)+sbm2_bit0)>>LOG2_BITS_PER_BITMAP_WORD] & NUMBERED_BIT((INDEX_SUM(sbm2_bit_offset)+sbm2_bit0)&(BITS_PER_BITMAP_WORD-1)))')
',`	dnl	else ! build_for_gpu
dnl	For cpu, bit0 is already in the base...
dnl	define(`srcbit',`(sbm_ptr[sbm_bit_offset>>LOG2_BITS_PER_BITMAP_WORD] & NUMBERED_BIT((sbm_bit_offset)&(BITS_PER_BITMAP_WORD-1)))')
dnl	Do we need separate GPU definitions?
define(`srcbit',`((*(sbm_ptr + (sbm_bit_offset/BITS_PER_BITMAP_WORD))) & NUMBERED_BIT(sbm_bit_offset))')
define(`srcbit1',`((*(sbm1_ptr + (sbm1_bit_offset/BITS_PER_BITMAP_WORD))) & NUMBERED_BIT(sbm1_bit_offset))')
define(`srcbit2',`((*(sbm2_ptr + (sbm2_bit_offset/BITS_PER_BITMAP_WORD))) & NUMBERED_BIT(sbm2_bit_offset))')

')

dnl	This defn was !BUILD_FOR_CUDA, why???
dnl	_VEC_FUNC_1V_3SCAL(name,s1,s2,s3)
define(`_VEC_FUNC_1V_3SCAL',`_VEC_FUNC_SLOW_1V_3SCAL($1,$2,$3,$4)')


ifdef(`BUILD_FOR_CUDA',`

define(`GENERIC_GPU_FUNC_CALL',`
	GENERIC_SLOW_VEC_FUNC($1,$2,$3,$4,$5,$6,$7)
	GENERIC_SLEN_VEC_FUNC($1,$2,$3,$4,$5,$6,$7)
')

define(`SLOW_GPU_FUNC_CALL',`
	GENERIC_SLOW_VEC_FUNC($1,$2,$3,$4,$5,$6,$7)
	GENERIC_SLEN_VEC_FUNC($1,$2,$3,$4,$5,$6,$7)
')


define(`GENERIC_VEC_FUNC_DBM',`
	GENERIC_SLOW_VEC_FUNC_DBM($1,$2,$3,$4,$5)
	GENERIC_SLEN_VEC_FUNC_DBM($1,$2,$3,$4,$5)
')

dnl	_VEC_FUNC_2V_CONV(name,dest_type)
define(`_VEC_FUNC_2V_CONV',`
	/* cuda vec_func_2v_conv */
	_GENERIC_SLOW_CONV_FUNC($1,$2)
	_GENERIC_SLEN_CONV_FUNC($1,$2)
')

',`	dnl	else // ! BUILD_FOR_CUDA

// Why is it that only CUDA needs the len versions???

define(`GENERIC_GPU_FUNC_CALL',`
	GENERIC_SLOW_VEC_FUNC($1,$2,$3,$4,$5,$6,$7)
')

define(`SLOW_GPU_FUNC_CALL',`
	GENERIC_SLOW_VEC_FUNC($1,$2,$3,$4,$5,$6,$7)
')

define(`GENERIC_VEC_FUNC_DBM',`
	GENERIC_SLOW_VEC_FUNC_DBM($1,$2,$3,$4,$5)
')

define(`_VEC_FUNC_2V_CONV',`
	_GENERIC_SLOW_CONV_FUNC($1,$2)
')

define(`GENERIC_FUNC_DECLS',`
	GENERIC_SF_DECL($1,$2,$3,$4,$5,$6,$7)
')

')	dnl	endif // ! BUILD_FOR_CUDA
