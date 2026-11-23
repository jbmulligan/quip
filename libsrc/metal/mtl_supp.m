#include "quip_config.h"
#include "my_mtl.h"
#include "platform.h"
#include "mtl_platform.h"

#ifdef HAVE_METAL

static int n_mtl_devs = 0;

id<MTLDevice> mtl_device = NULL;
id <MTLHeap> mtl_heap = NULL;

#define DEFAULT_METAL_DEVICE_NAME	"default"

#define MTL_BUF_HASH_SIZE	7919	// 1000th prime
typedef struct mtl_buf_info {
	id <MTLBuffer>	mbi_id;
	const void *	mbi_addr;
} Mtl_Buf_Info;
static int mtl_buf_info_inited = 0;

static Mtl_Buf_Info mtl_buf_info_tbl[MTL_BUF_HASH_SIZE];

static void init_mtl_buf_info(void)
{
	int i;
	for(i=0;i<MTL_BUF_HASH_SIZE;i++){
		mtl_buf_info_tbl[i].mbi_addr = NULL;
	}
	mtl_buf_info_inited = 1;
}

static int hash_for(const void *addr)
{
	unsigned long idx;
	idx = (unsigned long) addr;

	idx %= MTL_BUF_HASH_SIZE;

	return (int) idx;
}

id <MTLBuffer> mtl_buf_for(const void *addr)
{
	const void *a;

	if( ! mtl_buf_info_inited ){ init_mtl_buf_info(); }

	int i=hash_for(addr);

	while(1){
		a = mtl_buf_info_tbl[i].mbi_addr;
		if( a == addr ){
			return mtl_buf_info_tbl[i].mbi_id;
		} else if( a == NULL ){
			q_error1(DEFAULT_QSP_ARG
				"Failed to find metal buffer for address");
			return 0;
		} else {	// a collision
			i++;
			if( i >= MTL_BUF_HASH_SIZE ){
				i = 0;
			}
		}
		// possible infinite loop if table becomes full!?
		// Need to check when table entries are created!
	}
}

static void init_mtl_dev_memory(QSP_ARG_DECL  Platform_Device *pdp)
{
	char area_name[MAX_AREA_NAME_LEN+1];
	Data_Area *ap;

	//strcpy(area_name,PFDEV_NAME(pdp));
	// make sure names will fit - longest name is %s.%s_host_mapped
	if( strlen(PLATFORM_NAME(PFDEV_PLATFORM(pdp)))+strlen(PFDEV_NAME(pdp))+strlen("._host_mapped") > MAX_AREA_NAME_LEN )
		error1("init_mtl_dev_memory:  area name too large for buffer, increase MAX_AREA_NAME_LEN!?");

	snprintf(area_name,MAX_AREA_NAME_LEN+1,"%s.%s",
		PLATFORM_NAME(PFDEV_PLATFORM(pdp)),PFDEV_NAME(pdp));

	// what should the name for the memory area be???

	// address set to NULL says use custom allocator;
	// see dobj/makedobj.c

	ap = pf_area_init(area_name,NULL,0, MAX_MTL_GLOBAL_OBJECTS,
							DA_MTL_GLOBAL,pdp);
	if( ap == NULL ){
		snprintf(ERROR_STRING,LLEN,
	"init_mtl_dev_memory:  error creating global data area %s",
			area_name);
		warn(ERROR_STRING);
	}
	// g++ won't take this line!?
	SET_AREA_PFDEV(ap,pdp);

	// BUG should be per-device, not global table...
	pdp->pd_ap[PF_GLOBAL_AREA_INDEX] = ap;

	/* We used to declare a heap for constant memory here,
	 * but there wasn't much of a point because:
	 * Constant memory can't be allocated, rather it is declared
	 * in the .cu code, and placed by the compiler as it sees fit.
	 * To have objects use this, we would have to declare a heap and
	 * manage it ourselves...
	 * There's only 64k, so we should be sparing...
	 * We'll try this later...
	 */


	/* Make up another area for the host memory
	 * which is locked and mappable to the device.
	 * We don't allocate a pool here, but do it as needed...
	 */

	//strcat(cname,"_host");
	snprintf(area_name,MAX_AREA_NAME_LEN+1,"%s.%s_host",
		PLATFORM_NAME(PFDEV_PLATFORM(pdp)),PFDEV_NAME(pdp));

	ap = pf_area_init(area_name,(u_char *)NULL,0,MAX_MTL_MAPPED_OBJECTS,
							DA_MTL_HOST,pdp);
	if( ap == NULL ){
		snprintf(ERROR_STRING,LLEN,
	"init_mtl_dev_memory:  error creating host data area %s",area_name);
		error1(ERROR_STRING);
	}
	SET_AREA_PFDEV(ap, pdp);
	pdp->pd_ap[PF_HOST_AREA_INDEX] = ap;

	/* Make up another psuedo-area for the mapped host memory;
	 * This is the same memory as above, but mapped to the device.
	 * In the current implementation, we create objects in the host
	 * area, and then automatically create an alias on the device side.
	 * There is a BUG in that by having this psuedo area in the data
	 * area name space, a user could select it as the data area and
	 * then try to create an object.  We will detect this in make_dobj,
	 * and complain.
	 */

	//strcpy(cname,dname);
	//strcat(cname,"_host_mapped");
	snprintf(area_name,MAX_AREA_NAME_LEN+1,"%s.%s_host_mapped",
		PLATFORM_NAME(PFDEV_PLATFORM(pdp)),PFDEV_NAME(pdp));

	ap = pf_area_init(area_name,(u_char *)NULL,0,MAX_MTL_MAPPED_OBJECTS,
						DA_MTL_HOST_MAPPED,pdp);
	if( ap == NULL ){
		snprintf(ERROR_STRING,LLEN,
	"init_mtl_dev_memory:  error creating host-mapped data area %s",area_name);
		error1(ERROR_STRING);
	}
	SET_AREA_PFDEV(ap,pdp);
	pdp->pd_ap[PF_HOST_MAPPED_AREA_INDEX] = ap;

	if( verbose ){
		snprintf(ERROR_STRING,LLEN,"init_mtl_dev_memory DONE");
		advise(ERROR_STRING);
	}
}

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
	id<MTLBuffer> buffer = [mtl_device newBufferWithLength:size
					options:MTLResourceStorageModeShared];
	return buffer;
}

#define mtl_obj_alloc(dp,size,align) _mtl_mem_alloc(QSP_ARG  dp,size,align)

static int _mtl_obj_alloc(QSP_ARG_DECL  Data_Obj *dp, dimension_t size,
								int align )
{
	id<MTLBuffer> buf = mtl_mem_alloc(curr_pdp,size,align);
	SET_OBJ_EXTRA(dp,buf);
	void *p = [buf contents];
	SET_OBJ_DATA_PTR(dp,p);
	return 0;
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
	init_mtl_dev_memory(QSP_ARG  pdp);
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
	NSError *err_p;

fprintf(stderr,"mtl_make_kernel %s BEGIN\n",name);
fprintf(stderr,"src:\n%s\n",src);
	ensure_mtl_device(SINGLE_QSP_ARG);

	NSString *kernel_str = [NSString stringWithUTF8String:src];
	// BUG - should get device from pdp arg!?
	id<MTLLibrary> lib = [mtl_device newLibraryWithSource:kernel_str
						options:NULL error:&err_p];
	// BUG check compile error here!
	if( err_p == NULL ){
		fprintf(stderr,"no errors\n");
	} else {
		NSInteger code;
		code = [err_p code];
		fprintf(stderr,"error code: %ld\n",code);
		NSString *ns;
		ns = [err_p domain];
		const char *s;
		s = [ns UTF8String];
		fprintf(stderr,"error domain: %s\n",s);

		ns = [err_p localizedDescription];
		fprintf(stderr,"error description: %s\n",[ns UTF8String]);

	}
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

	SET_PF_FUNC_TBL(cpp,mtl_vfa_tbl);


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
