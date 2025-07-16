
all:	libmtl.a
	cp libmtl.a ../../libs

noinst_LIBRARIES = libmtl.a

libmtl_a_CPPFLAGS = -I$(top_srcdir)/include -Wall -Wmissing-prototypes

# BUG this is mac only...
#AM_CFLAGS= -framework OpenGL
AM_CFLAGS= -I/System/Library/Frameworks/OpenGL.framework/Headers

GENERATED_SRCS=	mtl_rand_expanded.c mtl_veclib_expanded.c mtl_fft_expanded.c

clean-local:
	rm -rf $(GENERATED_SRCS) vuni.i

#libmvimenu_a_CPPFLAGS = -I$(top_srcdir)/include -Wall -Wmissing-prototypes

#	mtl_viewer.c

#	mtl_veclib.c
#	mtl_fft.c

M4OPTS=	-I $(top_srcdir)/include -I $(srcdir)

mtl_rand_expanded.c:	mtl_rand.m4 $(LOCAL_M4FILES) $(INC_M4FILES)
	m4 $(M4OPTS) $(srcdir)/mtl_rand.m4 > $(builddir)/mtl_rand_expanded.c

mtl_veclib_expanded.c:	mtl_veclib.m4 $(LOCAL_M4FILES) $(INC_M4FILES)
	m4 $(M4OPTS) $(srcdir)/mtl_veclib.m4 > $(builddir)/mtl_veclib_expanded.c

mtl_fft_expanded.c:	mtl_fft_funcs.m4 $(LOCAL_M4FILES) $(INC_M4FILES)
	m4 $(M4OPTS) $(srcdir)/mtl_fft_funcs.m4 > $(builddir)/mtl_fft_expanded.c

libmtl_a_SOURCES =			\
			mtl_veclib.c

#			mtl_streams.c	\
#			mtl.c		\
#			mtl_menu.c	\
#			mtl_rand.c	\
#			mtl_utils.c	\
#			mtl_fft.c	\
#			mtl_veclib.c

$(srcdir)/mtl_veclib.c:	mtl_veclib_expanded.c
	touch $(srcdir)/mtl_veclib.c

$(srcdir)/mtl_fft.c:	mtl_fft_expanded.c
	touch $(srcdir)/mtl_fft.c

#../../include/veclib/vecgen.m4:

LOCAL_M4FILES=						\
			mtl_typed_fft_funcs.m4		\
			mtl_untyped_fft_funcs.m4	\
			mtl_host_call_defs.m4		\
			mtl_untyped_host_calls.m4	\
			mtl_host_funcs.m4		\
			mtl_kern_args.m4		\
			mtl_kern_call_defs.m4		\
			mtl_kernel_src.m4		\
			mtl_kernels.m4			\
			mtl_typtbl.m4

#			mtl_rand.m4			\
#			mtl_veclib.m4 

M4_INCDIR=$(srcdir)/../../include/veclib
INC_M4FILES=							\
			$(M4_INCDIR)/all_bit_kernels.m4		\
			$(M4_INCDIR)/all_float_kernels.m4		\
			$(M4_INCDIR)/all_int_kernels.m4		\
			$(M4_INCDIR)/all_mixed_float_kernels.m4	\
			$(M4_INCDIR)/all_mixed_int_kernels.m4	\
			$(M4_INCDIR)/all_mixed_uint_kernels.m4	\
			$(M4_INCDIR)/all_same_prec_vec.m4		\
			$(M4_INCDIR)/all_uint_kernels.m4		\
			$(M4_INCDIR)/all_vec.m4			\
			$(M4_INCDIR)/bit_defs.m4			\
			$(M4_INCDIR)/by_defs.m4			\
			$(M4_INCDIR)/cpx_args.m4			\
			$(M4_INCDIR)/cpx_vec.m4			\
			$(M4_INCDIR)/di_defs.m4			\
			$(M4_INCDIR)/dp_defs.m4			\
			$(M4_INCDIR)/dp_func_defs.m4		\
			$(M4_INCDIR)/eqsp_defs.m4			\
			$(M4_INCDIR)/eqsp_incs.m4			\
			$(M4_INCDIR)/fast_defs.m4			\
			$(M4_INCDIR)/fast_eqsp_defs.m4		\
			$(M4_INCDIR)/fast_test.m4			\
			$(M4_INCDIR)/fft_funcs.m4			\
			$(M4_INCDIR)/flen_defs.m4			\
			$(M4_INCDIR)/flush_output.m4		\
			$(M4_INCDIR)/gen_bit_calls.m4		\
			$(M4_INCDIR)/gen_float_calls.m4		\
			$(M4_INCDIR)/gen_func_array.m4		\
			$(M4_INCDIR)/gen_gpu_calls.m4		\
			$(M4_INCDIR)/gen_host_calls.m4		\
			$(M4_INCDIR)/gen_int_calls.m4		\
			$(M4_INCDIR)/gen_kern_args.m4		\
			$(M4_INCDIR)/gen_kernel_calls.m4		\
			$(M4_INCDIR)/gen_mixed_float_calls.m4	\
			$(M4_INCDIR)/gen_mixed_int_calls.m4	\
			$(M4_INCDIR)/gen_mixed_uint_calls.m4	\
			$(M4_INCDIR)/gen_port.m4			\
			$(M4_INCDIR)/gen_typtbl.m4			\
			$(M4_INCDIR)/gen_uint_calls.m4		\
			$(M4_INCDIR)/gpu_args.m4			\
			$(M4_INCDIR)/gpu_call_utils.m4		\
			$(M4_INCDIR)/gpu_special_defs.m4		\
			$(M4_INCDIR)/cpu_host_call_defs.m4		\
			$(M4_INCDIR)/gpu_host_call_defs.m4		\
			$(M4_INCDIR)/host_calls_special.m4		\
			$(M4_INCDIR)/host_typed_call_defs.m4	\
			$(M4_INCDIR)/in_defs.m4			\
			$(M4_INCDIR)/inby_defs.m4			\
			$(M4_INCDIR)/int_bit_vec.m4		\
			$(M4_INCDIR)/intvec.m4			\
			$(M4_INCDIR)/li_defs.m4			\
			$(M4_INCDIR)/math_funcs.m4			\
			$(M4_INCDIR)/new_conv.m4			\
			$(M4_INCDIR)/mtl_port.m4			\
			$(M4_INCDIR)/mtl_veclib_prot.m4		\
			$(M4_INCDIR)/quat_args.m4			\
			$(M4_INCDIR)/real_args.m4			\
			$(M4_INCDIR)/report_args.m4		\
			$(M4_INCDIR)/signed_vec.m4			\
			$(M4_INCDIR)/slow_defs.m4			\
			$(M4_INCDIR)/slow_incs.m4			\
			$(M4_INCDIR)/slow_len.m4			\
			$(M4_INCDIR)/slow_vars.m4			\
			$(M4_INCDIR)/sp_defs.m4			\
			$(M4_INCDIR)/sp_func_defs.m4		\
			$(M4_INCDIR)/spdp_defs.m4			\
			$(M4_INCDIR)/uby_defs.m4			\
			$(M4_INCDIR)/ubyin_defs.m4			\
			$(M4_INCDIR)/udi_defs.m4			\
			$(M4_INCDIR)/uin_defs.m4			\
			$(M4_INCDIR)/uindi_defs.m4			\
			$(M4_INCDIR)/uli_defs.m4			\
			$(M4_INCDIR)/unsigned_vec.m4		\
			$(M4_INCDIR)/vecgen.m4			\
			$(M4_INCDIR)/xfer_args.m4


$(srcdir)/mtl_rand.c:	vuni.i mtl_rand_expanded.c
	touch $(srcdir)/mtl_rand.c

$(srcdir)/vuni.i:	$(srcdir)/vuni.mtl
	$(srcdir)/gencl.sh $(srcdir)/vuni.mtl vuni.i

#			mtl_func_tbl.c

#noinst_HEADERS = my_mtl.h

#mtl_veclib_expanded.c:	_mtl_veclib.c
#	gcc -E -I../.. -I$(top_srcdir)/include `test -f _mtl_veclib.c || echo $(srcdir)/`_mtl_veclib.c > mtl_veclib_expanded.c || rm mtl_veclib_expanded.c
#
#mtl_veclib_stripped.c:		mtl_veclib_expanded.c
#	./strip_it.sh
#
#mtl_veclib.c:	mtl_veclib_stripped.c

#./strip_it2.sh
#			mtl_api.h

#			conversions.h
#			enum_menu_calls.h
#			host_conv_prot.h
#			kern_src.h
#			mtl_host_call_defs.h
#			mtl_host_untyped_call_defs.h
#			mtl_kern_args.h
#			mtl_kern_call_defs.h
#			mtl_kernel_src.h
#			type_undefs.h
#			undefs.h
#			mtl_host_funcs.c
#			mtl_kernels.c
#			mtl_typed_fft.c			\
#			mtl_typtbl.c

noinst_HEADERS =					\
			mtl_veclib.m4			\
			mtl_rand.m4			\
			$(LOCAL_M4FILES)		\
							\
			gencl.sh			\
			vuni.mtl


#	-rm -rf mtl_veclib_expanded.c mtl_veclib_stripped.c vuni.i

