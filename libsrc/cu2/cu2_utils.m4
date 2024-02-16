dnl	This is an original source file, which may be edited.
dnl	It is used to generate other source files, which should NOT be edited...

dnl	include(`../../include/veclib/cu2_veclib_prot.m4')
include(`../../include/veclib/cu2_port.m4')

#define MAX_SOURCE_SIZE (0x100000)

//In general Intel CPU and NV/AMD's GPU are in different platforms
//But in Mac OSX, all the OpenCL devices are in the platform "Apple"

#define MAX_PARAM_SIZE	128

#define ERROR_CASE(code,string)	case code: msg = string; break;

#ifdef CAUTIOUS
#define INSURE_CURR_ODP(whence)					\
	if( curr_pdp == NULL ){					\
		snprintf(ERROR_STRING,LLEN,"CAUTIOUS:  %s:  curr_pdp is null!?",#whence);	\
		warn(ERROR_STRING);				\
	}
#else // ! CAUTIOUS
#define INSURE_CURR_ODP(whence)
#endif // ! CAUTIOUS

// init_dev_memory

void PF_FUNC_NAME(shutdown)(void)
{
	//cl_int status;

	/*
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	*/
	warn("shutdown_cu2_platform NOT implemented!?");

	// Need to iterate over all devices...
}

void PF_FUNC_NAME(alloc_data)(QSP_ARG_DECL  Data_Obj *dp, dimension_t size)
{
	warn("PF_FUNC_NAME(alloc_data) not implemented!?");
}

void PF_FUNC_NAME(sync)(SINGLE_QSP_ARG_DECL)
{
	warn("PF_FUNC_NAME(sync):  not implemented!?");
}

void PF_FUNC_NAME(set_device)( QSP_ARG_DECL  Platform_Device *pdp )
{
#ifdef HAVE_CUDA
	cudaError_t e;
#endif // HAVE_CUDA

	if( curr_pdp == pdp ){
		snprintf(ERROR_STRING,LLEN,"%s:  current device is already %s!?",
			STRINGIFY(HOST_CALL_NAME(set_device)),PFDEV_NAME(pdp));
		warn(ERROR_STRING);
		return;
	}
	if( PFDEV_PLATFORM_TYPE(pdp) != PLATFORM_CUDA ){
		snprintf(ERROR_STRING,LLEN,"%s:  device %s is not a CUDA device!?",
			STRINGIFY(HOST_CALL_NAME(set_device)),PFDEV_NAME(pdp));
		warn(ERROR_STRING);
		return;
	}

#ifdef HAVE_CUDA
	e = cudaSetDevice( PFDEV_CUDA_DEV_INDEX(pdp) );
	if( e != cudaSuccess )
		describe_cuda_driver_error2(STRINGIFY(HOST_CALL_NAME(set_device)),"cudaSetDevice",e);
	else
		curr_pdp = pdp;
#else // ! HAVE_CUDA
	NO_CUDA_MSG(set_device)
#endif // ! HAVE_CUDA
}

PF_COMMAND_FUNC( list_devs )
{
	list_pfdevs(QSP_ARG  tell_msgfile(SINGLE_QSP_ARG));
}

void insure_cu2_device(QSP_ARG_DECL  Data_Obj *dp )
{
	Platform_Device *pdp;

	if( AREA_FLAGS(OBJ_AREA(dp)) & DA_RAM ){
		snprintf(ERROR_STRING,LLEN,
	"insure_cu2_device:  Object %s is a host RAM object!?",OBJ_NAME(dp));
		warn(ERROR_STRING);
		return;
	}

	pdp = AREA_PFDEV(OBJ_AREA(dp));

#ifdef CAUTIOUS
	if( pdp == NULL )
		NERROR1("CAUTIOUS:  null cuda device ptr in data area!?");
#endif /* CAUTIOUS */

	if( curr_pdp != pdp ){
snprintf(ERROR_STRING,LLEN,"insure_cu2_device:  curr_pdp = 0x%"PRIxPTR"  pdp = 0x%"PRIxPTR,
(uintptr_t)curr_pdp,(uintptr_t)pdp);
advise(ERROR_STRING);

snprintf(ERROR_STRING,LLEN,"insure_cu2_device:  current device is %s, want %s",
PFDEV_NAME(curr_pdp),PFDEV_NAME(pdp));
advise(ERROR_STRING);
		PF_FUNC_NAME(set_device)(QSP_ARG  pdp);
	}

}

void *TMPVEC_NAME `(QSP_ARG_DECL  Platform_Device *pdp, size_t size,size_t len,const char *whence)'
{
	// Why is this commented out???
/*
	void *cuda_mem;
	cudaError_t drv_err;

	drv_err = cudaMalloc(&cuda_mem, size * len );
	if( drv_err != cudaSuccess ){
		snprintf(DEFAULT_MSG_STR,LLEN,"tmpvec (%s)",whence);
		describe_cuda_driver_error2(DEFAULT_MSG_STR,"cudaMalloc",drv_err);
		NERROR1("CUDA memory allocation error");
	}

//snprintf(ERROR_STRING,LLEN,"tmpvec:  %d bytes allocated at 0x%"PRIxPTR,len,(uintptr_t)cuda_mem);
//advise(ERROR_STRING);

//snprintf(ERROR_STRING,LLEN,"tmpvec %s:  0x%"PRIxPTR,whence,(uintptr_t)cuda_mem);
//advise(ERROR_STRING);
	return(cuda_mem);
	*/
	return NULL;
}

void FREETMP_NAME `(QSP_ARG_DECL  void *ptr,const char *whence)'
{
	/*
	cudaError_t drv_err;

//snprintf(ERROR_STRING,LLEN,"freetmp %s:  0x%"PRIxPTR,whence,(uintptr_t)ptr);
//advise(ERROR_STRING);
	drv_err=cudaFree(ptr);
	if( drv_err != cudaSuccess ){
		snprintf(DEFAULT_MSG_STR,LLEN,"freetmp (%s)",whence);
		describe_cuda_driver_error2(DEFAULT_MSG_STR,"cudaFree",drv_err);
	}
	*/
}

typedef struct {
	const char *	ckpt_tag;
#ifdef FOOBAR
	cudaEvent_t	ckpt_event;
#endif // FOOBAR
} CU2_Checkpoint;

static CU2_Checkpoint *ckpt_tbl=NULL;
//static int max_cu2_ckpts=0;	// size of checkpoit table
static int n_cu2_ckpts=0;	// number of placements

static void init_cu2_ckpts(int n)
{
	/*
	//CUresult e;
	cudaError_t drv_err;
	int i;

	if( max_cu2_ckpts > 0 ){
		snprintf(ERROR_STRING,LLEN,
"init_cu2_ckpts (%d):  already initialized with %d checpoints",
			n,max_cu2_ckpts);
		warn(ERROR_STRING);
		return;
	}
	ckpt_tbl = (Cuda_Checkpoint *) getbuf( n * sizeof(*ckpt_tbl) );
	if( ckpt_tbl == NULL ) NERROR1("failed to allocate checkpoint table");

	max_cu2_ckpts = n;

	for(i=0;i<max_cu2_ckpts;i++){
		drv_err=cudaEventCreate(&ckpt_tbl[i].ckpt_event);
		if( drv_err != cudaSuccess ){
			describe_cuda_driver_error2("init_cu2_ckpts",
				"cudaEventCreate",drv_err);
			NERROR1("failed to initialize checkpoint table");
		}
		ckpt_tbl[i].ckpt_tag=NULL;
	}
	*/
}


PF_COMMAND_FUNC( init_ckpts )
{
	int n;

	n = HOW_MANY("maximum number of checkpoints");
	init_cu2_ckpts(n);
}


PF_COMMAND_FUNC( set_ckpt )
{
	/*
	//cudaError_t e;
	cudaError_t drv_err;
	const char *s;

	s = NAMEOF("tag for this checkpoint");

	if( max_cu2_ckpts == 0 ){
		warn("do_place_ckpt:  checkpoint table not initialized, setting to default size");
		init_cu2_ckpts(256);
	}

	if( n_cu2_ckpts >= max_cu2_ckpts ){
		snprintf(ERROR_STRING,LLEN,
	"do_place_ckpt:  Sorry, all %d checkpoints have already been placed",
			max_cu2_ckpts);
		warn(ERROR_STRING);
		return;
	}

	ckpt_tbl[n_cu2_ckpts].ckpt_tag = savestr(s);

	// use default stream (0) for now, but will want to introduce
	// more streams later?
	drv_err = cudaEventRecord( ckpt_tbl[n_cu2_ckpts++].ckpt_event, 0 );
	CUDA_DRIVER_ERROR_RETURN( "do_place_ckpt","cudaEventRecord")
	*/
}

PF_COMMAND_FUNC( show_ckpts )
{
	/*
	CUresult e;
	cudaError_t drv_err;
	float msec, cum_msec;
	int i;

	if( n_cu2_ckpts <= 0 ){
		warn("do_show_cu2_ckpts:  no checkpoints placed!?");
		return;
	}

	drv_err = cudaEventSynchronize(ckpt_tbl[n_cu2_ckpts-1].ckpt_event);
	CUDA_DRIVER_ERROR_RETURN("do_show_cu2_ckpts", "cudaEventSynchronize")

	drv_err = cudaEventElapsedTime( &msec, ckpt_tbl[0].ckpt_event, ckpt_tbl[n_cu2_ckpts-1].ckpt_event);
	CUDA_DRIVER_ERROR_RETURN("do_show_cu2_ckpts", "cudaEventElapsedTime")
	snprintf(msg_str,LLEN,"Total GPU time:\t%g msec",msec);
	prt_msg(msg_str);

	// show the start tag
	snprintf(msg_str,LLEN,"GPU  %3d  %12.3f  %12.3f  %s",1,0.0,0.0,
		ckpt_tbl[0].ckpt_tag);
	prt_msg(msg_str);
	cum_msec =0.0;
	for(i=1;i<n_cu2_ckpts;i++){
		drv_err = cudaEventElapsedTime( &msec, ckpt_tbl[i-1].ckpt_event,
			ckpt_tbl[i].ckpt_event);
		CUDA_DRIVER_ERROR_RETURN("do_show_cu2_ckpts", "cudaEventElapsedTime")

		cum_msec += msec;
		snprintf(msg_str,LLEN,"GPU  %3d  %12.3f  %12.3f  %s",i+1,msec,
			cum_msec, ckpt_tbl[i].ckpt_tag);
		prt_msg(msg_str);
	}
	*/
}

PF_COMMAND_FUNC( clear_ckpts )
{
	int i;

	for(i=0;i<n_cu2_ckpts;i++){
		rls_str(ckpt_tbl[i].ckpt_tag);
		ckpt_tbl[i].ckpt_tag=NULL;
	}
	n_cu2_ckpts=0;
}

