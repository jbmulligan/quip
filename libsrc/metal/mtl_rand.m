
/* Use threefry parallel random number generator from Random123 package */

#include "quip_config.h"

#ifdef HAVE_METAL

#include "quip_prot.h"
#include "my_mtl.h"
#include "mtl_platform.h"
#include <string.h>	// strcmp - for debugging
#include <sys/time.h>	// gettimeofday

#include "vuni.metal"	// kernel source string

// The generator has no random seed currently, so it
// will always produce the same "random" data!?
static int mtl_sp_vuni_counter=0;
static int mtl_sp_vuni_seeded=0;
static int mtl_sp_vuni_inited=0;
//static cl_kernel mtl_sp_vuni_kernel = NULL;

// BUG the kernel is created for the device,
// so this code is broken in the case that we have multiple devices

#define mtl_sp_vuni_init(pdp) _mtl_sp_vuni_init(QSP_ARG  pdp)

static int _mtl_sp_vuni_init(QSP_ARG_DECL  Platform_Device *pdp)
{
	fprintf(stderr,"Sorry, mtl_sp_vuni_init not implemented\n");
	return -1;
#ifdef FOOBAR
	struct timeval tv;
	//cl_kernel kernel;

	mtl_sp_vuni_inited=1;
	// BUG need to be able to name the source string!?
	if( (kernel=mtl_make_kernel(opencl_src,"g_mtl_fast_sp_vuni",pdp)) == NULL ){
		warn("Error creating kernel for g_mtl_fast_sp_vuni!?");
		return -1;
	}
	//mtl_sp_vuni_kernel = kernel;

	if( !mtl_sp_vuni_seeded ){
		// "randomize" the seed so that we don't get the same numbers every time
		if( gettimeofday(&tv,NULL) < 0 ){
			warn("mtl_sp_vuni_init:  error calling gettimeofday, not setting random seed!?");
		} else {
			mtl_sp_vuni_counter = 100*(tv.tv_usec / 100);
			mtl_sp_vuni_counter += tv.tv_sec % 100;
		}
	}
	return 0;
#endif // FOOBAR
}

void h_mtl_set_seed(int seed)
{
	mtl_sp_vuni_counter = seed;
	mtl_sp_vuni_seeded = 1;
}

#include "mtl_rand_expanded.c"

#endif // HAVE_METAL

