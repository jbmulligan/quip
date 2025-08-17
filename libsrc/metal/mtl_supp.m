#include "quip_config.h"
#include "my_mtl.h"
#include "platform.h"
#include "mtl_platform.h"

#ifdef HAVE_METAL

static int n_mtl_devs = 0;

id<MTLDevice> mtl_device = NULL;
id <MTLHeap> mtl_heap = NULL;

#define DEFAULT_METAL_DEVICE_NAME	"default"

static void mtl_mem_upload(QSP_ARG_DECL  void *dst, void *src, size_t siz,
					index_t offset, Platform_Device *pdp )
{
	fprintf(stderr,"Sorry, mtl_mem_upload not implemented\n");
}

static void mtl_mem_dnload(QSP_ARG_DECL  void *dst, void *src, size_t siz,
					index_t offset, Platform_Device *pdp )
{
	fprintf(stderr,"Sorry, mtl_mem_dnload not implemented\n");
}

#define mtl_mem_alloc(pdp,size,align) _mtl_mem_alloc(QSP_ARG  pdp,size,align)

static void *_mtl_mem_alloc(QSP_ARG_DECL  Platform_Device *pdp,
						dimension_t size, int align )
{
	fprintf(stderr,"Sorry, mtl_mem_alloc not implemented\n");
	return NULL;
}

#define mtl_obj_alloc(dp,size,align) _mtl_mem_alloc(QSP_ARG  dp,size,align)

static int _mtl_obj_alloc(QSP_ARG_DECL  Data_Obj *dp, dimension_t size,
								int align )
{
	fprintf(stderr,"Sorry, mtl_obj_alloc not implemented\n");
	return -1;
}

#define mtl_mem_free(ptr) _mtl_mem_free(QSP_ARG  ptr)

static void _mtl_mem_free(QSP_ARG_DECL  void *ptr)
{
	fprintf(stderr,"Sorry, mtl_mem_free not implemented\n");
}

static void _mtl_obj_free(QSP_ARG_DECL  Data_Obj *dp)
{
	fprintf(stderr,"Sorry, mtl_obj_free not implemented\n");
}

static void _mtl_offset_data(QSP_ARG_DECL  Data_Obj *dp, index_t offset)
{
	fprintf(stderr,"Sorry, mtl_offset_data not implemented\n");
}

static void _mtl_update_offset(QSP_ARG_DECL  Data_Obj *dp )
{
	fprintf(stderr,"Sorry, mtl_update_offset not implemented\n");
}

static int mtl_register_buf(QSP_ARG_DECL  Data_Obj *dp)
{
	fprintf(stderr,"Sorry, mtl_register_buf not implemented\n");
	return -1;
}

static int mtl_map_buf(QSP_ARG_DECL  Data_Obj *dp)
{
	fprintf(stderr,"Sorry, mtl_map_buf not implemented\n");
	return -1;
}

static int mtl_unmap_buf(QSP_ARG_DECL  Data_Obj *dp)
{
	fprintf(stderr,"Sorry, mtl_unmap_buf not implemented\n");
	return -1;
}

static void mtl_dev_info(QSP_ARG_DECL  Platform_Device *pdp)
{
	fprintf(stderr,"Sorry, mtl_dev_info not implemented\n");
}

static void mtl_info(QSP_ARG_DECL  Compute_Platform *cpp)
{
	fprintf(stderr,"Sorry, mtl_info not implemented\n");
}

static const char *mtl_kernel_string(QSP_ARG_DECL
					Platform_Kernel_String_ID which )
{
	fprintf(stderr,"Sorry, mtl_kernel_string not implemented\n");
	return NULL;
}

static void mtl_store_kernel(QSP_ARG_DECL  Kernel_Info_Ptr *kip_p,
					void *kp, Platform_Device *pdp)
{
	fprintf(stderr,"Sorry, mtl_store_kernel not implemented\n");
}

static void * mtl_fetch_kernel(QSP_ARG_DECL  Kernel_Info_Ptr kip,
						Platform_Device *pdp)
{
	fprintf(stderr,"Sorry, mtl_fetch_kernel not implemented\n");
	return NULL;
}

static void mtl_run_kernel(QSP_ARG_DECL  void *kp, Vec_Expr_Node *arg_enp,
							Platform_Device *pdp)
{
	fprintf(stderr,"Sorry, mtl_run_kernel not implemented\n");
}

static void mtl_set_kernel_arg(QSP_ARG_DECL  /*cl_kernel*/ void * kp,
				int *idx_p, void *vp, Kernel_Arg_Type arg_type)
{
	fprintf(stderr,"Sorry, mtl_set_kernel_arg not implemented\n");
}

static int _init_mtl_devices(QSP_ARG_DECL  Compute_Platform *cpp )
{
	Platform_Device *pdp;

	pdp = new_pfdev(DEFAULT_METAL_DEVICE_NAME);
	SET_PFDEV_PLATFORM(pdp,cpp);
	SET_PFDEV_SERIAL(pdp,n_mtl_devs++);
	SET_PFDEV_MAX_DIMS(pdp,DEFAULT_PFDEV_MAX_DIMS);
	SET_PFDEV_MDI(pdp,getbuf(sizeof(*PFDEV_MDI(pdp))));
	// BUG - initialize mdi fields!
	curr_pdp = pdp;

	mtl_device = MTLCreateSystemDefaultDevice();
	pdp->pd_dev_info.u_mdi_p->mdi_device = mtl_device;
	return 0;
}

static void ensure_mtl_device(SINGLE_QSP_ARG_DECL)
{
	if( mtl_device == NULL ){
		_init_mtl_devices(QSP_ARG  NULL);
	}
}

/*id<MTLFunction>*/ void * _mtl_make_kernel(QSP_ARG_DECL  const char *src,
				const char *name, Platform_Device *pdp)
{
	ensure_mtl_device(SINGLE_QSP_ARG);

	NSString *kernel_str = [NSString stringWithUTF8String:src];
	// BUG - should get device from pdp arg!?
	id<MTLLibrary> lib = [mtl_device newLibraryWithSource:kernel_str
						options:NULL error:NULL];
	NSString *name_str = [NSString stringWithUTF8String:name];
	id<MTLFunction> kernelFunction = [lib newFunctionWithName:name_str];
	return kernelFunction;
}

static void ensure_metal_heap()
{
	if( mtl_heap != NULL ){ return; }

	int heapSize = 256 * 1024 * 1024;	// 256M
	MTLHeapDescriptor* descriptor = [MTLHeapDescriptor new];
	//descriptor.type = MTLHeapTypeSparse;
	// default type is MTLHeapType.automatic
	descriptor.storageMode = MTLStorageModePrivate;
	descriptor.size = heapSize;
	mtl_heap = [mtl_device newHeapWithDescriptor: descriptor];
}

void * _mtl_tmp_vec(QSP_ARG_DECL  Platform_Device *pdp, size_t size,size_t len,
							const char *whence)
{
	id<MTLBuffer> buf;
	MTLResourceOptions opts;	// if no initialization,
					// do we get default values?
	opts = MTLResourceCPUCacheModeDefaultCache;	// the default?
	ensure_metal_heap();
	buf = [mtl_heap newBufferWithLength: size*len options: opts];
	return buf;
}

void _mtl_free_tmp(QSP_ARG_DECL  void *ptr,const char *whence)
{
	// In Metal, there is nothing to do, the memory will be
	// released automatically when the last reference is
	// gone (garbage collection).  We just have to be sure
	// that the references do go away!
}

id<MTLDevice> _get_metal_dev(SINGLE_QSP_ARG_DECL){
	if( mtl_device == NULL ){
		error1("Metal device is not initialized!?");
	}
	return mtl_device;
}

void mtl_init_platform(SINGLE_QSP_ARG_DECL)
{
fprintf(stderr,"mtl_init_platform BEGIN\n");
	Compute_Platform *cpp;
	static int inited=0;

	if( inited ){
		advise("Redundant call to mtl_init_platform!?");
		return;
	}
	inited=1;

	cpp = creat_platform(QSP_ARG  "Metal", PLATFORM_METAL );
	assert( cpp != NULL );

	SET_PLATFORM_FUNCTIONS(cpp,mtl)

	push_pfdev_context(PF_CONTEXT(cpp) );
	if( _init_mtl_devices(QSP_ARG  cpp) < 0 ){
		inited = -1;
	}
	if( pop_pfdev_context() == NULL ){
		error1("init_mtl_platform:  Failed to pop platform device context!?");
	}

	check_vfa_tbl(mtl_vfa_tbl, N_VEC_FUNCS);

	if( inited < 0 ){
		// problem above
		delete_platform(QSP_ARG  cpp);
	}
}

#endif // HAVE_METAL
