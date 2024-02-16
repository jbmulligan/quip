
include(`../../include/veclib/slow_len.m4')
include(`../../include/veclib/slow_incs.m4')
include(`../../include/veclib/eqsp_incs.m4')
include(`../../include/veclib/slow_vars.m4')

// cuda uses dim3...

dnl include(`../../include/veclib/dim3.m4')


// cudaGetLastError not available before 5.0 ...

dnl	CHECK_CUDA_ERROR(whence)

define(`CHECK_CUDA_ERROR',`

	e = cudaGetLastError();
	if( e != cudaSuccess ){
		describe_cuda_driver_error("$1",e);
	}
')


dnl	CLEAR_CUDA_ERROR(name)
define(`CLEAR_CUDA_ERROR',`

	e = cudaGetLastError();
	if( e != cudaSuccess ){
		describe_cuda_driver_error("$1",e);
	}
')

dnl	CLEAR_CUDA_ERROR2(name)

define(`CLEAR_CUDA_ERROR2',`

	e = cudaGetLastError();
	if( e != cudaSuccess ){
		describe_cuda_driver_error2("HOST_CALL_NAME($1)",
			"GPU_CALL_NAME($1)",e);
	}
')

define(`HELPER_FUNC_PRELUDE',`
DECLARE_PLATFORM_VARS
')

// What is the point of this - where does it occur?
dnl	SET_MAX_THREADS_FROM_OBJ(dp)

define(`SET_MAX_THREADS_FROM_OBJ',`
dnl	/*max_threads_per_block = PFDEV_CUDA_MAX_THREADS_PER_BLOCK(OBJ_PFDEV(dp));*/
')


define(`BLOCK_VARS_DECLS',`

	DIM3 n_blocks, n_threads_per_block;
	DIM3 extra;
')

define(`DECLARE_PLATFORM_VARS_2',`
DECLARE_PLATFORM_VARS
')

define(`DECLARE_PLATFORM_VARS',`
	cudaError_t e;
BLOCK_VARS_DECLS
')

define(`DECLARE_PLATFORM_FAST_VARS',`
DECLARE_PLATFORM_VARS
	/*dimension_t len;*/
')


define(`DECLARE_PLATFORM_SLOW_VARS',`
DECLARE_PLATFORM_VARS
	/*DIM3 xyz_len;*/
')


dnl	These are no-op's for CUDA
define(`SETUP_KERNEL_FAST_CALL',`')
define(`SETUP_KERNEL_FAST_CALL_CONV',`')

define(`SETUP_KERNEL_EQSP_CALL',`')
define(`SETUP_KERNEL_EQSP_CALL_CONV',`')

define(`SETUP_KERNEL_SLOW_CALL',`')
define(`SETUP_KERNEL_SLOW_CALL_CONV',`')

dnl	CALL_FAST_KERNEL(name,bitmap,typ,scalars,vectors)

/* call_fast_kernel */
define(`CALL_FAST_KERNEL',`

CLEAR_CUDA_ERROR(GPU_FAST_CALL_NAME($1))
SETUP_BLOCKS_X(VA_LENGTH(vap))
	/* call_fast_kernel  get_threads_per_ /$2/ block */
	GET_THREAD_COUNT($2,VA_PFDEV(vap),VA_LENGTH(vap))
	if (extra.x != 0) {
		n_blocks.x++;
		REPORT_THREAD_INFO
		GPU_FLEN_CALL_NAME($1)<<< NN_GPU >>> 
			(KERN_ARGS_FLEN($2,$3,$4,$5));
    		CHECK_CUDA_ERROR(flen $1:  kernel launch failure);
	} else {
		REPORT_THREAD_INFO
		GPU_FAST_CALL_NAME($1)<<< NN_GPU >>>
			(KERN_ARGS_FAST($2,$3,$4,$5));
    		CHECK_CUDA_ERROR(fast $1:  kernel launch failure);
	}
')

/* call_fast_kernel defn DONE */



dnl	CALL_FAST_CONV_KERNEL(name,bitmap,typ,type)

define(`CALL_FAST_CONV_KERNEL',`

	CLEAR_CUDA_ERROR(GPU_FAST_CALL_NAME($1))
	SETUP_BLOCKS_X(VA_LENGTH(vap))
	/* call_fast_conv_kernel  get_threads_per_ /$2/ block */
	GET_THREAD_COUNT($2,VA_PFDEV(vap),VA_LENGTH(vap))
	if (extra.x != 0) {
		n_blocks.x++;
		REPORT_THREAD_INFO
		GPU_FLEN_CALL_NAME($1)<<< NN_GPU >>> 
			(KERN_ARGS_FLEN_CONV($4) );
    		CHECK_CUDA_ERROR(flen $1:  kernel launch failure);
	} else {
		REPORT_THREAD_INFO
		GPU_FAST_CALL_NAME($1)<<< NN_GPU >>>
			(KERN_ARGS_FAST_CONV($4));
		/* BUG?  should we put this check everywhere? */
    		CHECK_CUDA_ERROR(fast $1:  kernel launch failure);
	}
')



dnl	CALL_EQSP_KERNEL(name,bitmap,typ,scalars,vectors)

define(`CALL_EQSP_KERNEL',`

	CLEAR_CUDA_ERROR(GPU_EQSP_CALL_NAME($1))
	/* call_eqsp_kernel  get_threads_per_ /$2/ block */
	GET_THREAD_COUNT($2,VA_PFDEV(vap),VA_LENGTH(vap))
	SETUP_BLOCKS_X(VA_LENGTH(vap))
	/* BUG? shoudl this be commented out??? */
	/*SETUP_SIMPLE_INCS_##vectors*/
	/*GET_EQSP_INCR_##bitmap##vectors*/
	if (extra.x != 0) {
		n_blocks.x++;
		REPORT_THREAD_INFO
		GPU_ELEN_CALL_NAME($1)<<< NN_GPU >>> 
			(KERN_ARGS_ELEN($2,$3,$4,$5));
    		CHECK_CUDA_ERROR(elen $1:  kernel launch failure);
	} else {
		REPORT_THREAD_INFO
		GPU_EQSP_CALL_NAME($1)<<< NN_GPU >>>
			(KERN_ARGS_EQSP($2,$3,$4,$5));
		/* BUG?  should we put this check everywhere? */
    		CHECK_CUDA_ERROR(eqsp $1:  kernel launch failure);
	}
')


dnl	CALL_EQSP_CONV_KERNEL(name,bitmap,typ,type)

define(`CALL_EQSP_CONV_KERNEL',`

	CLEAR_CUDA_ERROR(GPU_EQSP_CALL_NAME($1))
	/* call_eqsp_conv_kernel  get_threads_per_ /$2/ block */
	GET_THREAD_COUNT($2,VA_PFDEV(vap),VA_LENGTH(vap))
	SETUP_BLOCKS_X(VA_LENGTH(vap))
	/* BUG? shoudl this be commented out??? */
	/*SETUP_SIMPLE_INCS_##vectors*/
	/*GET_EQSP_INCR_##bitmap##vectors*/
	if (extra.x != 0) {
		n_blocks.x++;
		REPORT_THREAD_INFO
		GPU_ELEN_CALL_NAME($1)<<< NN_GPU >>> 
			( KERN_ARGS_ELEN_CONV($4) );
    		CHECK_CUDA_ERROR(elen $1:  kernel launch failure);
	} else {
		REPORT_THREAD_INFO
		GPU_EQSP_CALL_NAME($1)<<< NN_GPU >>>
			( KERN_ARGS_EQSP_CONV($4) );
		/* BUG?  should we put this check everywhere? */
    		CHECK_CUDA_ERROR(eqsp $1:  kernel launch failure);
	}
')



dnl	CALL_SLOW_KERNEL(name,bitmap,typ,scalars,vectors)
define(`CALL_SLOW_KERNEL',`

	CLEAR_CUDA_ERROR(GPU_SLOW_CALL_NAME($1))
	SETUP_BLOCKS($2,VA_PFDEV(vap))	/* using len - was _XY */
	if( extra.x > 0 || extra.y > 0 || extra.z > 0 ){
		GPU_SLEN_CALL_NAME($1)<<< NN_GPU >>>
			(KERN_ARGS_SLEN($2,$3,$4,$5));
		CHECK_CUDA_ERROR(slen $1:  kernel launch failure);
	} else {
		GPU_SLOW_CALL_NAME($1)<<< NN_GPU >>>
			(KERN_ARGS_SLOW($2,$3,$4,$5));
		CHECK_CUDA_ERROR(slow $1:  kernel launch failure);
	}
')



dnl	CALL_SLOW_CONV_KERNEL(name,bitmap,typ,type)
define(`CALL_SLOW_CONV_KERNEL',`

	CLEAR_CUDA_ERROR(GPU_SLOW_CALL_NAME($1))
	SETUP_BLOCKS($2,VA_PFDEV(vap))
	REPORT_THREAD_INFO
	if( extra.x > 0 || extra.y > 0 || extra.z > 0 ){
		GPU_SLEN_CALL_NAME($1)<<< NN_GPU >>>
			( KERN_ARGS_SLEN_CONV($4) );
		CHECK_CUDA_ERROR(slen $1:  kernel launch failure);
	} else {
		GPU_SLOW_CALL_NAME($1)<<< NN_GPU >>>
			( KERN_ARGS_SLOW_CONV($4) );
		CHECK_CUDA_ERROR(slow $1:  kernel launch failure);
	}
')


/* For slow loops, we currently only iterate over two dimensions (x and y),
 * although in principle we should be able to handle 3...
 * We need to determine which 2 by examining the dimensions of the vectors.
 */

dnl	SVAL_FLOAT(svp)
define(`SVAL_FLOAT',`($1)->u_f')
define(`SVAL_STD',`($1)->std_scalar')
define(`SVAL_STDCPX',`($1)->std_cpx_scalar')
define(`SVAL_STDQUAT',`($1)->std_quat_scalar')
define(`SVAL_BM',`($1)->bitmap_scalar')



// this is 80 columns
//345678901234567890123456789012345678901234567890123456789012345678901234567890


define(`NN_GPU',`n_blocks, n_threads_per_block')


// PORT - insure_gpu_device ???

// We should have an intelligent way of designing blocks...
// On wheatstone, the card can have a maximum of 512 threads/block.
// But this doesn't divide typical video image dimensions
// of 640x480.  But 640 is 128x5, so we could have blocks
// that are 128x4 and not have any leftover.  Using our dumb
// strategy, we have blocks that are 16x32... 480/32=15,
// so we are OK here.
//
// We found a bug doing interlaces of 640/480, the 240 line half-frames
// came out to 7.5 32 row blocks in height.  In that case it's better
// to have the blocks wider and less tall.
//
// It might be nice to adjust things like this automatically, but
// it's kind of tricky!?

// This is kind of kludgy, but let's try it
dnl	should be max_threads_per_block???
define(`MAX_THREADS_PER_ROW',`32')

dnl	SETUP_BLOCKS(bitmaps,pdp)
define(`SETUP_BLOCKS',`SETUP_BLOCKS_XYZ_$1($2)')

dnl	SETUP_BLOCKS_XYZ_(pdp)
define(`SETUP_BLOCKS_XYZ_',`SETUP_BLOCKS_XYZ($1)')
define(`SETUP_BLOCKS_XYZ_SBM_',`SETUP_BLOCKS_XYZ($1)')

dnl	SETUP_BLOCKS_XYZ(pdp)
define(`SETUP_BLOCKS_XYZ',`

dnl /*fprintf(stderr,"SETUP_BLOCKS_XYZ_\n");*/
	SETUP_BLOCKS_X(VA_ITERATION_TOTAL(vap))
	SETUP_BLOCKS_Y($1)
	SETUP_BLOCKS_Z($1)
')


dnl	If we have a destination bitmap, we handle all the bits in one word
dnl	in a single thread.
dnl	
dnl	BUG - here we ignore bit0 ???
dnl	
dnl	MAX_THREADS_PER_ROW is 32...  for a 512 pixel wide image, the nmber
dnl	of bitmap words is either 8 (64 bit words) or 16 (32 bit words).
dnl	So we need to ...

dnl	using the number of bitmap words of the total number of pixel ops
dnl	is correct only when the destination bitmap is contiguous!?
dnl	BUG!?
dnl	SETUP_BLOCKS_X( N_BITMAP_WORDS(VA_ITERATION_TOTAL(vap)) )

dnl	SETUP_BLOCKS_XYZ_DBM_(pdp)
define(`SETUP_BLOCKS_XYZ_DBM_',`
	SETUP_BLOCKS_X( VA_ITERATION_TOTAL(vap) )
	SETUP_BLOCKS_Y($1)
	SETUP_BLOCKS_Z($1)
')

dnl	SETUP_BLOCKS_XYZ_DBM_SBM(pdp)
define(`SETUP_BLOCKS_XYZ_DBM_SBM',`
	SETUP_BLOCKS_XYZ_DBM_($1)
')

define(`SETUP_BLOCKS_XYZ_DBM_2SBM',`
	SETUP_BLOCKS_XYZ_DBM_($1)
')

define(`SETUP_BLOCKS_XYZ_DBM_1SBM',`
	SETUP_BLOCKS_XYZ_DBM_($1)
')


dnl	SETUP_BLOCKS_X(w)
define(`SETUP_BLOCKS_X',`

dnl /*snprintf(ERROR_STRING,LLEN,"SETUP_BLOCKS_X:  len = %d",$1);
dnl advise(ERROR_STRING);*/
	if( ($1) < MAX_THREADS_PER_ROW ) {
		n_threads_per_block.x = $1;
		n_blocks.x = 1;
		extra.x = 0;
	} else {
		n_blocks.x = ($1) / MAX_THREADS_PER_ROW;
		n_threads_per_block.x = MAX_THREADS_PER_ROW;
		extra.x = ($1) % MAX_THREADS_PER_ROW;
	}
')


dnl	SETUP_BLOCKS_Y(pdp)
define(`OLD_SETUP_BLOCKS_Y',`

	assert(n_threads_per_block.x>0);
	n_threads_per_block.y = PFDEV_CUDA_MAX_THREADS_PER_BLOCK($1) 
				/ n_threads_per_block.x;
	if( VA_LEN_Y(vap) < n_threads_per_block.y ){
		n_threads_per_block.y = VA_LEN_Y(vap);
		n_blocks.y = 1;
		extra.y = 0;
	} else {
		assert(n_threads_per_block.y>0);
		n_blocks.y = VA_LEN_Y(vap) / n_threads_per_block.y;
		extra.y = VA_LEN_Y(vap) % n_threads_per_block.y;
	}
	if( extra.x > 0 ) n_blocks.x++;
	if( extra.y > 0 ) n_blocks.y++;
')
define(`SETUP_BLOCKS_Y',`
	n_threads_per_block.y = 1;
	extra.y=0;
')

dnl	SETUP_BLOCKS_Z(pdp)
define(`OLD_SETUP_BLOCKS_Z',`

	assert(n_threads_per_block.x>0);
	assert(n_threads_per_block.y>0);
	n_threads_per_block.z = PFDEV_CUDA_MAX_THREADS_PER_BLOCK($1) 
		/ (n_threads_per_block.x*n_threads_per_block.y);
	if( VA_LEN_Z(vap) < n_threads_per_block.z ){
		n_threads_per_block.z = VA_LEN_Z(vap);
		n_blocks.z = 1;
		extra.z = 0;
	} else {
		assert(n_threads_per_block.z>0);
		n_blocks.z = VA_LEN_Z(vap) / n_threads_per_block.z;
		extra.z = VA_LEN_Z(vap) % n_threads_per_block.z;
	}
	if( extra.z > 0 ) n_blocks.z++;
')

define(`SETUP_BLOCKS_Z',`
	n_threads_per_block.z = 1;
	extra.z=0;
')

dnl define(`MORE_DEBUG',`')

ifdef(`MORE_DEBUG',`

define(`REPORT_THREAD_INFO',`

snprintf(ERROR_STRING,LLEN,"Blocks:  %d x %d x %d    Threads:  %d x %d x %d",
n_blocks.x,n_blocks.y,n_blocks.z,
n_threads_per_block.x,n_threads_per_block.y,n_threads_per_block.z);
advise(ERROR_STRING);
snprintf(ERROR_STRING,LLEN,"Length:  %d x %d x %d    Extra:  %d x %d x %d",
VA_ITERATION_TOTAL(vap),VA_LEN_Y(vap),VA_LEN_Z(vap),extra.x,extra.y,extra.z);
advise(ERROR_STRING);
')

define(`REPORT_THREAD_INFO2',`

snprintf(ERROR_STRING,LLEN,"Blocks:  %d x %d    Threads:  %d x %d",
n_blocks.x,n_blocks.y,n_threads_per_block.x,n_threads_per_block.y);
advise(ERROR_STRING);
snprintf(ERROR_STRING,LLEN,"Len1:  %ld   Len2:  %ld   Extra:  %d x %d",
len1,len2,extra.x,extra.y);
advise(ERROR_STRING);
')

',` dnl else /* ! MORE_DEBUG */

define(`REPORT_THREAD_INFO',`')
define(`REPORT_THREAD_INFO2',`')

') dnl endif /* ! MORE_DEBUG */


define(`DEFAULT_YZ',`

	n_threads_per_block.y = n_threads_per_block.z =
	n_blocks.y = n_blocks.z = 1;
	extra.y = extra.z = 0;
')


dnl	SET_BLOCKS_FROM_LEN( pdp, n_tot )
define(`SET_BLOCKS_FROM_LEN',`

	if( $2 < PFDEV_CUDA_MAX_THREADS_PER_BLOCK($1) ) {
		n_threads_per_block.x = $2;
		n_blocks.x = 1;
		extra.x = 0;
	} else {
		n_blocks.x = $2 / PFDEV_CUDA_MAX_THREADS_PER_BLOCK($1);
		n_threads_per_block.x = PFDEV_CUDA_MAX_THREADS_PER_BLOCK($1);
		extra.x = $2 % PFDEV_CUDA_MAX_THREADS_PER_BLOCK($1);
	}
')

dnl	GET_THREAD_COUNT(bitmaps,pdp,len_var)
define(`GET_THREAD_COUNT',`
	DEFAULT_YZ
	SET_BLOCKS_FROM_LEN($2,$3)
	SET_BLOCKS_$1($2,$3)
')

define(`SET_BLOCKS_',`')

define(`SET_BLOCKS',`
SET_BLOCKS_FROM_LEN($1,$2)
')

/* get_threads_per_block defn */
dnl	GET_THREADS_PER_BLOCK(pdp,len_var)

define(`GET_THREADS_PER_BLOCK',`

	DEFAULT_YZ
	SET_BLOCKS_FROM_LEN($1,$2)
')
/* get_threads_per_block defn DONE */
dnl	GET_THREADS_PER_SBM_BLOCK(pdp,len_var)
define(`GET_THREADS_PER_SBM_BLOCK',`GET_THREADS_PER_BLOCK($1,$2)')

// This used to be called GET_THREADS_PER_BITMAP_BLOCK

dnl	GET_THREADS_PER_DBM_BLOCK(pdp,len_var)
define(`GET_THREADS_PER_DBM_BLOCK',`

dnl /*fprintf(stderr,"GET_THREADS_PER_DBM_BLOCK:  len = %d\n",len_var);*/
	DEFAULT_YZ
	if( (VA_DBM_BIT0(vap)+$2) < BITS_PER_BITMAP_WORD ) {
		n_threads_per_block.x = 1;
		n_blocks.x = 1;
	} else {
		int nw;
		nw = N_BITMAP_WORDS(VA_DBM_BIT0(vap)+$2);
		SET_BLOCKS_FROM_LEN($1,nw)
	}
')

dnl	SET_BLOCKS_DBM_(pdp,len_var)
define(`SET_BLOCKS_DBM_',`
	if( (VA_DBM_BIT0(vap)+$2) < BITS_PER_BITMAP_WORD ) {
		n_threads_per_block.x = 1;
		n_blocks.x = 1;
	} else {
		int nw;
		nw = N_BITMAP_WORDS(VA_DBM_BIT0(vap)+$2);
		SET_BLOCKS_FROM_LEN($1,nw)
	}
')

define(`SET_BLOCKS_DBM_SBM',`SET_BLOCKS_DBM_($1,$2)')
define(`SET_BLOCKS_DBM_2SBM',`SET_BLOCKS_DBM_($1,$2)')
define(`SET_BLOCKS_DBM_1SBM',`SET_BLOCKS_DBM_($1,$2)')

dnl	SET_BLOCKS_SBM_(pdp,len_var)
define(`SET_BLOCKS_SBM_',`
	/* set_blocks_sbm_ is a no-op */
dnl	if( (VA_SBM_BIT0(vap)+$2) < BITS_PER_BITMAP_WORD ) {
dnl		n_threads_per_block.x = 1;
dnl		n_blocks.x = 1;
dnl	} else {
dnl		int nw;
dnl		nw = N_BITMAP_WORDS(VA_SBM_BIT0(vap)+$2);
dnl		SET_BLOCKS_FROM_LEN($1,nw)
dnl	}
')

dnl	GET_THREADS_PER_DBM_SBMBLOCK(pdp,len_var)
define(`GET_THREADS_PER_DBM_SBMBLOCK',`

	DEFAULT_YZ
	if( (VA_DBM_BIT0(vap)+$2) < BITS_PER_BITMAP_WORD ) {
		n_threads_per_block.x = 1;
		n_blocks.x = 1;
	} else {
		int nw;
		nw = N_BITMAP_WORDS(VA_DBM_BIT0(vap)+$2);
		SET_BLOCKS_FROM_LEN($1,nw)
	}
')




define(`MAX_THREADS_PER_BITMAP_ROW',`MIN(MAX_THREADS_PER_ROW,BITS_PER_BITMAP_WORD)')

dnl dnl	INSIST_LENGTH( n , msg , name )
dnl define(`INSIST_LENGTH',`
dnl 
dnl 	if( ($1) == 1 ){
dnl 		snprintf(ERROR_STRING,LLEN,
dnl 	"Oops, kind of silly to do %s of 1-len vector %s!?",$2,$3);
dnl 		warn(ERROR_STRING);
dnl 		return;
dnl 	}
dnl ')


ifdef(`MORE_DEBUG',`

dnl	REPORT_VECTORIZATION1( host_func_name )
define(`REPORT_VECTORIZATION1',`

	snprintf(ERROR_STRING,LLEN,
"%s:  ready to vectorize:\txyz_len.x = %ld, inc1.x = %ld, inc1.y = %ld",
		"$1",VA_ITERATION_TOTAL(vap),inc1.x,inc1.y);
	advise(ERROR_STRING);
')

dnl	REPORT_VECTORIZATION2( host_func_name )
define(`REPORT_VECTORIZATION2',`

	REPORT_VECTORIZATION1($1)
	snprintf(ERROR_STRING,LLEN, "\t\t\t\tinc2.x = %ld, inc2.y = %ld",
		inc2.x,inc2.y);
	advise(ERROR_STRING);
')

dnl	REPORT_VECTORIZATION3( host_func_name )
define(`REPORT_VECTORIZATION3',`

	REPORT_VECTORIZATION2($1)
	snprintf(ERROR_STRING,LLEN, "\t\t\t\tinc3.x = %ld, inc3.y = %ld",
		inc3.x,inc3.y);
	advise(ERROR_STRING);
')

dnl	REPORT_VECTORIZATION4( host_func_name )
define(`REPORT_VECTORIZATION4',`

	REPORT_VECTORIZATION3($1)
	snprintf(ERROR_STRING,LLEN, "\t\t\t\tinc4.x = %ld, inc4.y = %ld",
		inc4.x,inc4.y);
	advise(ERROR_STRING);
')

dnl	REPORT_VECTORIZATION5( host_func_name )
define(`REPORT_VECTORIZATION5',`

	REPORT_VECTORIZATION4($1)
	snprintf(ERROR_STRING,LLEN, "\t\t\t\tinc5.x = %ld, inc5.y = %ld",
		inc5.x,inc5.y);
	advise(ERROR_STRING);
')

',` dnl else /* ! MORE_DEBUG */

define(`REPORT_VECTORIZATION1',`')
define(`REPORT_VECTORIZATION2',`')
define(`REPORT_VECTORIZATION3',`')
define(`REPORT_VECTORIZATION4',`')
define(`REPORT_VECTORIZATION5',`')

') dnl endif /* ! MORE_DEBUG */





// MM_IND vmaxi etc

// CUDA definitions
// BUG we probably want the passed vap to have constant data...

// BUG use symbolic constant for kernel args!
dnl	CALL_GPU_FAST_NOCC_SETUP_FUNC(name)
define(`CALL_GPU_FAST_NOCC_SETUP_FUNC',`
	CLEAR_GPU_ERROR(SETUP_NAME($1))
	REPORT_THREAD_INFO2
	/* call_gpu_fast_nocc_setup_func /$1/ */
	GPU_FAST_CALL_NAME(SETUP_NAME($1))<<< NN_GPU >>>
		dnl BUG use macro for these args!?
		(dst_values, dst_counts, orig_src_values, indices, len1, len2);
	CHECK_GPU_ERROR(SETUP_NAME($1))
')


// BUG use symbolic constant for kernel args!
dnl	CALL_GPU_FAST_NOCC_HELPER_FUNC(name)
define(`CALL_GPU_FAST_NOCC_HELPER_FUNC',`
	CLEAR_GPU_ERROR(HELPER_NAME($1))
	REPORT_THREAD_INFO2
	GPU_FAST_CALL_NAME(HELPER_NAME($1))<<< NN_GPU >>>
		(dst_values, dst_counts,src_values,src_counts, indices,len1,len2,stride); 
	CHECK_GPU_ERROR(HELPER_NAME($1))
')



// CUDA only!
dnl	CALL_GPU_FAST_PROJ_3V_SETUP_FUNC(name)
define(`CALL_GPU_FAST_PROJ_3V_SETUP_FUNC',`
/* call_gpu_fast_proj_2v_setup_func */
	CLEAR_GPU_ERROR($1)
	REPORT_THREAD_INFO2
fprintf(stderr,"call_gpu_fast_proj_3v_setup_func(%s):  dst_values = 0x%lx, orig_src1_values = 0x%lx, orig_src2_values = 0x%lx, len1 = %d, len2 = %d\n",
"$1",(long)dst_values,(long)orig_src1_values,(long)orig_src2_values,len1,len2);
	GPU_FAST_CALL_NAME($1`_setup')<<< NN_GPU >>>( dst_values, orig_src1_values,orig_src2_values, len1, len2 );
	CHECK_GPU_ERROR($1)
')

dnl	CALL_GPU_FAST_PROJ_3V_HELPER_FUNC(name) /* CUDA only */
define(`CALL_GPU_FAST_PROJ_3V_HELPER_FUNC',`
/* call_gpu_fast_proj_3v_helper_func */
	CLEAR_GPU_ERROR($1)
	REPORT_THREAD_INFO2
dnl fprintf(stderr,"CALL_GPU_FAST_PROJ_3V_HELPER_FUNC(%s):  dst_values = 0x%lx, src_values = 0x%lx, len1 = %d, len2 = %d\n",
dnl "$1",(long)dst_values,(long)src_values,len1,len2);
	GPU_FAST_CALL_NAME($1`_helper')<<< NN_GPU >>>( dst_values, src1_values, src2_values, len1, len2 );
	CHECK_GPU_ERROR($1)
')



// CUDA only!
dnl	CALL_GPU_FAST_PROJ_2V_SETUP_FUNC(name)
define(`CALL_GPU_FAST_PROJ_2V_SETUP_FUNC',`
/* call_gpu_fast_proj_2v_setup_func */
	CLEAR_GPU_ERROR($1)
	REPORT_THREAD_INFO2
fprintf(stderr,"call_gpu_fast_proj_2v_setup_func(%s):  dst_values = 0x%lx, orig_src_values = 0x%lx, len1 = %d, len2 = %d\n",
"$1",(long)dst_values,(long)orig_src_values,len1,len2);
	GPU_FAST_CALL_NAME($1`_setup')<<< NN_GPU >>>( dst_values, orig_src_values, len1, len2 );
	CHECK_GPU_ERROR($1)
')

dnl	CALL_GPU_FAST_PROJ_2V_HELPER_FUNC(name) /* CUDA only */
define(`CALL_GPU_FAST_PROJ_2V_HELPER_FUNC',`
/* call_gpu_fast_proj_2v_helper_func */
	CLEAR_GPU_ERROR($1)
	REPORT_THREAD_INFO2
dnl fprintf(stderr,"CALL_GPU_FAST_PROJ_2V_HELPER_FUNC(%s):  dst_values = 0x%lx, src_values = 0x%lx, len1 = %d, len2 = %d\n",
dnl "$1",(long)dst_values,(long)src_values,len1,len2);
	GPU_FAST_CALL_NAME($1`_helper')<<< NN_GPU >>>( dst_values, src_values, len1, len2 );
	CHECK_GPU_ERROR($1)
')

dnl	CALL_GPU_FAST_INDEX_SETUP_FUNC(name)
define(`CALL_GPU_FAST_INDEX_SETUP_FUNC',`
	CLEAR_GPU_ERROR(GPU_FAST_CALL_NAME(SETUP_NAME($1)))
	REPORT_THREAD_INFO2
	GPU_FAST_CALL_NAME(SETUP_NAME($1))<<< NN_GPU >>>
		(indices,src1_values,src2_values,len1,len2);
	CHECK_GPU_ERROR(GPU_FAST_CALL_NAME(SETUP_NAME($1)))
')


dnl	CALL_GPU_FAST_INDEX_HELPER_FUNC(name)
define(`CALL_GPU_FAST_INDEX_HELPER_FUNC',`
	CLEAR_GPU_ERROR(GPU_FAST_CALL_NAME(HELPER_NAME($1)))
	REPORT_THREAD_INFO2
	GPU_FAST_CALL_NAME(HELPER_NAME($1))<<< NN_GPU >>>
		(indices,idx1_values,idx2_values,orig_src_values,len1,len2);
	CHECK_GPU_ERROR(GPU_FAST_CALL_NAME(HELPER_NAME($1)))
')


