
divert(-1)	dnl suppress output

include(`veclib/mtl_port.m4')

my_include(`veclib/mtl_veclib_prot.m4')

suppress_no
#include "veclib/fftsupp.h"
#include "veclib/mtl_veclib_prot.h"
suppress_if

dnl first include the kernels...


define(`BUILDING_KERNELS',`')


suppress_no

my_include(`mtl_kern_call_defs.m4')

my_include(`veclib/sp_defs.m4')
my_include(`mtl_typed_fft_funcs.m4')
my_include(`veclib/dp_defs.m4')
my_include(`mtl_typed_fft_funcs.m4')

undefine(`BUILDING_KERNELS')

my_include(`mtl_host_call_defs.m4')

my_include(`veclib/sp_defs.m4')
my_include(`mtl_typed_fft_funcs.m4')
my_include(`veclib/dp_defs.m4')
my_include(`mtl_typed_fft_funcs.m4')

