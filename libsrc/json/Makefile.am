
all:	libjson.a
	cp libjson.a ../../libs

noinst_LIBRARIES = libjson.a
#noinst_PROGRAMS = test_json

libjson_a_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include -Wall -Wmissing-prototypes
#test_json_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include -Wall -Wmissing-prototypes
# -D__STRICT_ANSI__  this breaks u_long in sys/types.h...

libjson_a_SOURCES =	json_menu.c json.y
#test_json_SOURCES =	test.c
#test_json_LDADD = ../../libs/libinterpreter.a ../../libs/libdobj.a ../../libs/libvec.a

#noinst_HEADERS =	subrt.h

#noinst_HEADERS = vectree.h vt_native.h \
#			ctx_pair.h	\
#			identifier.h	\
#			pointer.h	\
#			reference.h	\
#			subrt.h		\
#			undef_sym.h	\
#			vec_expr_node.h

AM_YFLAGS=-v

