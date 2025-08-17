
include(`../../include/veclib/mtl_port.m4')
my_include(`../../include/veclib/mtl_veclib_prot.m4')
my_include(`mtl_kern_args.m4')

void h_mtl_sp_vuni(HOST_CALL_ARG_DECLS)
{
	if( ! mtl_sp_vuni_inited ){
		if( mtl_sp_vuni_init( OBJ_PFDEV(OA_DEST(oap)) ) < 0 ) return;
	}

	fprintf(stderr,"Sorry, h_mtl_sp_vuni not implemented yet\n");

#ifdef FOOBAR
	size_t global_work_size[3] = {1, 1, 1};
	int ki_idx=0;

	// BUG should make sure destination is contiguous!

	cl_int status;

	global_work_size[0] = OBJ_N_MACH_ELTS(OA_DEST(oap));

	_SET_KERNEL_ARG( mtl_sp_vuni_kernel, void *, &(OBJ_DATA_PTR( OA_DEST(oap))) )
	_SET_KERNEL_ARG( mtl_sp_vuni_kernel, int, &mtl_sp_vuni_counter )
	mtl_sp_vuni_counter ++;

	fprintf(stderr,"Sorry, metal rand not implemented yet\n");
	/*
	status = clEnqueueNDRangeKernel(				
		OCLDEV_QUEUE( OBJ_PFDEV(OA_DEST(oap)) ),
		mtl_sp_vuni_kernel,
		1,	/* work_dim, 1-3 */
		NULL,
		global_work_size,
		/*local_work_size*/ NULL,
		0,	/* num_events_in_wait_list */
		NULL,	/* event_wait_list */
		NULL	/* event */
		);
	if( status != CL_SUCCESS )
		report_mtl_error(status, "clEnqueueNDRangeKernel" );
		*/
#endif // FOOBAR
}

void h_mtl_dp_vuni(HOST_CALL_ARG_DECLS)
{
	warn("Sorry, dp_vuni not implemented for OpenCL");
}

