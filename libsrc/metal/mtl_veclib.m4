
include(`../../include/veclib/mtl_port.m4')

suppress_if dnl suppress output

/* `suppress_if' = suppress_if */
/* mtl_veclib.m4 BEGIN */

define(`BUILD_FOR_GPU',`')

my_include(`../../include/veclib/vecgen.m4')

define(`BUILDING_KERNELS',`')
my_include(`mtl_kernels.m4')
undefine(`BUILDING_KERNELS')

// That declares the kernels - now the host-side functions

void _ensure_mtl_device(QSP_ARG_DECL  Data_Obj *dp)
{
	warn("ensure_mtl_device:  not implemented!?");
}

my_include(`mtl_host_funcs.m4')

my_include(`mtl_typtbl.m4')

/* mtl_veclib.m4 END */


