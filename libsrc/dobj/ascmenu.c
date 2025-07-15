#include "quip_config.h"

#include <stdio.h>
#include <string.h>
#include "data_obj.h"
//#include "dataprot.h"
#include "debug.h"
#include "query_stack.h"
#include "variable.h"
#include "ascii_fmts.h"
#include "warn.h"
#include "quip_prot.h"
#include "getbuf.h"
#include "veclib_api.h"	// BUG should integrate into data_obj.h...
#include "platform.h"
#ifdef HAVE_POPEN
#include "pipe_support.h"
#endif // HAVE_POPEN


// BUG should be per-thread variable...
static int expect_exact_count=1;

#ifndef HAVE_ANY_GPU

#define DECL_RAM_DATA_OBJ
#define ram_dp	dp
#define INSURE_OK_FOR_READING(dp)
#define INSURE_OK_FOR_WRITING(dp)
#define RELEASE_RAM_OBJ_FOR_READING_IF(dp)
#define RELEASE_RAM_OBJ_FOR_WRITING_IF(dp)

#else // ! HAVE_ANY_GPU

#define DECL_RAM_DATA_OBJ	Data_Obj *ram_dp;

#define INSURE_OK_FOR_READING(dp)					\
									\
	ram_dp = ensure_ram_obj_for_reading(dp);			\
	assert( ram_dp != NULL );

#define INSURE_OK_FOR_WRITING(dp)					\
	ram_dp = ensure_ram_obj_for_writing(dp);			\
	assert(ram_dp!=NULL);

#define RELEASE_RAM_OBJ_FOR_READING_IF(dp)				\
	release_ram_obj_for_reading(ram_dp, dp);

#define RELEASE_RAM_OBJ_FOR_WRITING_IF(dp)				\
	release_ram_obj_for_writing(ram_dp, dp);

#define DNAME_PREFIX "downloaded_"
#define CNAME_PREFIX "contiguous_"

#define IS_SUBSCRIPT_DELIMITER(c)	( (c)=='[' || (c)=='{' || (c)==']' || c=='}' )

static char *get_temp_name(const char *prefix, const char *name )
{
	char *s, *buf;

	int l = strlen(name) + strlen(prefix) + 1 ;
	buf = getbuf(l);
	snprintf(buf,l,"%s%s",prefix,name);

	// replace index delimiters with underscores...
	s=buf;
	while(*s){
		if( IS_SUBSCRIPT_DELIMITER(*s) )
			*s = '_';
		s++;
	}
	return buf;
}

void _release_ram_obj_for_reading(QSP_ARG_DECL  Data_Obj *ram_dp, Data_Obj *dp)
{
	if( ram_dp == dp ) return;
	delvec(ram_dp);
}

#define create_ram_copy(dp) _create_ram_copy(QSP_ARG  dp)

static Data_Obj *_create_ram_copy(QSP_ARG_DECL  Data_Obj *dp)
{
	Data_Area *save_ap;
	Data_Obj *tmp_dp;
	char *tmp_name;

	tmp_name = get_temp_name(DNAME_PREFIX,OBJ_NAME(dp));

	save_ap = curr_ap;
	curr_ap = ram_area_p;
	tmp_dp = dup_obj(dp, tmp_name);
	curr_ap = save_ap;

	givbuf(tmp_name);

	return tmp_dp;
}

// for host-device tranfers, we need a contiguous object.

#define create_platform_copy(dp) _create_platform_copy(QSP_ARG  dp)

static Data_Obj *_create_platform_copy(QSP_ARG_DECL  Data_Obj *dp)
{
	Data_Area *save_ap;
	Data_Obj *contig_dp;
	const char *tname;

	tname = get_temp_name(CNAME_PREFIX,OBJ_NAME(dp));
	assert(tname!=NULL);

	save_ap = curr_ap;
	curr_ap = OBJ_AREA( dp );
	contig_dp = dup_obj(dp, tname );
	curr_ap = save_ap;

	givbuf(tname);

	return contig_dp;
}

// Assume that the two objects are matched in shape

#define copy_platform_data(dst_dp, src_dp) _copy_platform_data(QSP_ARG  dst_dp, src_dp)

static void _copy_platform_data(QSP_ARG_DECL  Data_Obj *dst_dp, Data_Obj *src_dp)
{
	Vec_Obj_Args oa1, *oap=&oa1;

	setvarg2(oap,dst_dp,src_dp);
	if( IS_BITMAP(src_dp) ){
		SET_OA_SBM(oap,src_dp);
		// SRC1 gets referenced???
		//SET_OA_SRC1(oap,NULL);
	}

	if( IS_REAL(src_dp) ) /* BUG case for QUAT too? */
		OA_ARGSTYPE(oap) = REAL_ARGS;
	else if( IS_COMPLEX(src_dp) ) /* BUG case for QUAT too? */
		OA_ARGSTYPE(oap) = COMPLEX_ARGS;
	else if( IS_QUAT(src_dp) ) /* BUG case for QUAT too? */
		OA_ARGSTYPE(oap) = QUATERNION_ARGS;
	else
		assert( AERROR("copy_platform_data:  bad argset type!?") );

	call_vfunc( FIND_VEC_FUNC(FVMOV), oap );
}

#define contig_obj(dp) _contig_obj(QSP_ARG  dp)

static Data_Obj *_contig_obj(QSP_ARG_DECL  Data_Obj *dp)
{
	Data_Obj *copy_dp;

	if( IS_CONTIGUOUS(dp) ) return dp;
	if( HAS_CONTIGUOUS_DATA(dp) ) return dp;

//advise("object is not contiguous, and does not have contiguous data, creating temp object for copy...");
//longlist(dp);

	copy_dp = create_platform_copy(dp);
//longlist(copy_dp);
	return copy_dp;
}

#define contig_obj_with_data(dp) _contig_obj_with_data(QSP_ARG  dp)

static Data_Obj *_contig_obj_with_data(QSP_ARG_DECL  Data_Obj *dp)
{
	Data_Obj *contig_dp;

	contig_dp = contig_obj(dp);
	if( contig_dp == dp ) return dp;
	copy_platform_data(contig_dp, dp );
	return contig_dp;
}

#define download_platform_data(ram_dp, pf_dp) _download_platform_data(QSP_ARG  ram_dp, pf_dp)

static void _download_platform_data(QSP_ARG_DECL  Data_Obj *ram_dp, Data_Obj *pf_dp)
{
	Data_Obj *contig_dp;

	// We can't download if the source data is not contiguous...

	contig_dp = contig_obj_with_data(pf_dp);
	assert( IS_CONTIGUOUS(ram_dp) );

	gen_obj_dnload(ram_dp, contig_dp);

	if( contig_dp != pf_dp )
		delvec(contig_dp);
}

#define upload_platform_data(pf_dp, ram_dp) _upload_platform_data(QSP_ARG  pf_dp, ram_dp)

static void _upload_platform_data(QSP_ARG_DECL  Data_Obj *pf_dp, Data_Obj *ram_dp)
{
	Data_Obj *contig_dp;

	// We can't upload if the destination data is not contiguous...
	assert( IS_CONTIGUOUS(ram_dp) );

	contig_dp = contig_obj(pf_dp);

	gen_obj_upload(contig_dp, ram_dp );

	if( contig_dp != pf_dp ){
		copy_platform_data(pf_dp,contig_dp);
		delvec(contig_dp);
	}
}


// To write a platform object, we need to have a ram copy to stick the values in,
// that we then transfer en-mass.  The copy must have the correct shape,
// but doesn't need to contain the data, as we will be over-writing it anyway.

Data_Obj *_ensure_ram_obj_for_writing(QSP_ARG_DECL  Data_Obj *dp)
{
	if( OBJ_IS_RAM(dp) ) return dp;
	return create_ram_copy(dp);
}

// To read a platform object, the copies need to have the data copied along!

Data_Obj *_ensure_ram_obj_for_reading(QSP_ARG_DECL  Data_Obj *dp)
{
	Data_Obj *ram_dp;

	if( OBJ_IS_RAM(dp) ) return dp;

	// This object lives on a different platform.
	// We create a copy in RAM, and download the data
	// using the platform download function.

	ram_dp = create_ram_copy(dp);

	if( ram_dp == NULL ){
		// This can happen if the object is subscripted,
		// as the bracket characters are illegal in names
		return NULL;
	}

	download_platform_data(ram_dp, dp);

	return ram_dp;
}

#define release_ram_obj_for_writing(ram_dp, dp) _release_ram_obj_for_writing(QSP_ARG  ram_dp, dp)

static void _release_ram_obj_for_writing(QSP_ARG_DECL  Data_Obj *ram_dp, Data_Obj *dp)
{
	if( ram_dp == dp ) return;	// nothing to do

	upload_platform_data(dp,ram_dp);
	delvec(ram_dp);
}
#endif /* ! HAVE_ANY_GPU */

/*
 * BUG do_read_obj will not work correctly for subimages
 * (or will it with the new revision?
 */

static COMMAND_FUNC( do_read_obj )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ
	FILE *fp;
	const char *s;

	dp=pick_obj("");
	s=nameof("input file");

	if( dp == NULL ) return;

#ifdef QUIP_DEBUG
//if( debug ) dptrace(dp);
#endif /* QUIP_DEBUG */

	// reading is tricker for non-ram, because
	// we must create the copy, then read into
	// the copy, then xfer to the device...

	INSURE_OK_FOR_WRITING(dp)

	if( strcmp(s,"-") && strcmp(s,"stdin") ){
		fp=try_open( s, "r" );
		if( !fp ) return;

		read_ascii_data_from_file(ram_dp,fp,s,expect_exact_count);
	} else {
		/* read from stdin, no problem... */

		read_obj(ram_dp);
	}

	RELEASE_RAM_OBJ_FOR_WRITING_IF(dp)
}

static COMMAND_FUNC( do_read_obj_from_stream )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ

	dp=pick_obj("");
	if( dp == NULL ) return;

	// reading is tricker for non-ram, because
	// we must create the copy, then read into
	// the copy, then xfer to the device...

	INSURE_OK_FOR_WRITING(dp)

	lookahead_til(QLEVEL-1);

	read_obj(ram_dp);

	RELEASE_RAM_OBJ_FOR_WRITING_IF(dp)

	// We may have called this from a macro, in which case we want to go back to the top menu...
	pop_to_top_menu();

}

// Do we need to test HAVE_POPEN here???  BUG???

static COMMAND_FUNC( do_pipe_obj )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ
	Pipe *pp;
	char cmdbuf[LLEN];

	dp=pick_obj("");
	pp=pick_pipe("readable pipe");

	if( dp == NULL ) return;
	if( pp == NULL ) return;

#ifdef HAVE_POPEN
	// reading is tricker for non-ram, because
	// we must create the copy, then read into
	// the copy, then xfer to the device...

	INSURE_OK_FOR_WRITING(dp)

	// BUG  a symbolic constant should be used here - this has to match
	// test string in read_ascii_data!!!

	snprintf(cmdbuf,LLEN,"%s:  %s",PIPE_PREFIX_STRING,pp->p_cmd);
	read_ascii_data_from_pipe(ram_dp,pp,cmdbuf,expect_exact_count);
	/* If there was just enough data, then the pipe
	 * will have been closed already... */

	// pipe should be closed when input is exhausted!

	/* check qlevel to make sure that the pipe was popped... */
	if( ASCII_LEVEL != QLEVEL + 1 ){	// expected
		snprintf(ERROR_STRING,LLEN,
	"do_pipe_obj:  final level %d is not one less than ascii level %d!?",
			QLEVEL,ASCII_LEVEL);
		warn(ERROR_STRING);
		// close pipe???
	}

	RELEASE_RAM_OBJ_FOR_WRITING_IF(dp)
#else // ! HAVE_POPEN
	warn("Sorry, no support for UNIX pipes in this build...");
#endif // ! HAVE_POPEN
}

static COMMAND_FUNC( do_set_var_from_obj )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ
	const char *s;

	s=nameof("variable");
	dp=pick_obj("");

	if( dp == NULL ) return;

	if( ! IS_STRING(dp) ){
		snprintf(ERROR_STRING,LLEN,"do_set_var_from_obj:  object %s (%s) does not have string precision",
			OBJ_NAME(dp),OBJ_PREC_NAME(dp));
		warn(ERROR_STRING);
		return;
	}

	INSURE_OK_FOR_READING(dp)

	// This is NOT OK if the object is a substring
	if( OBJ_N_MACH_ELTS(ram_dp) == strlen((char *)OBJ_DATA_PTR(ram_dp)) ){
		assign_var(s,(char *)OBJ_DATA_PTR(ram_dp));
	} else {
		char *tmpstr;
		dimension_t n;

		n = OBJ_N_MACH_ELTS(ram_dp);
		tmpstr = getbuf( n + 1 );
		strncpy(tmpstr,(char *)OBJ_DATA_PTR(ram_dp),n);
		tmpstr[n]=0;
		assign_var(s,tmpstr);
		givbuf(tmpstr);
	}

	RELEASE_RAM_OBJ_FOR_READING_IF(dp)
}

static COMMAND_FUNC( do_set_obj_from_var )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ
	const char *src_str;
	char *dst_str;
	dimension_t dst_size;

	dp=pick_obj("");
	src_str=nameof("string");

	if( dp == NULL ) return;

#ifdef QUIP_DEBUG
//if( debug ) dptrace(dp);
#endif /* QUIP_DEBUG */

	if( ! IS_STRING(dp) ){
		snprintf(ERROR_STRING,LLEN,"do_set_obj_from_var:  object %s (%s) does not have string precision",
			OBJ_NAME(dp),OBJ_PREC_NAME(dp));
		warn(ERROR_STRING);
		return;
	}

	dst_size = OBJ_COMPS(dp);

	if( strlen(src_str) >= dst_size ){
		snprintf(ERROR_STRING,LLEN,
	"Truncating string of length %d to fit string object %s (%d)...",
			(int)strlen(src_str), OBJ_NAME(dp), dst_size );
		warn(ERROR_STRING);
	}

	INSURE_OK_FOR_WRITING(dp)

	if( ! IS_CONTIGUOUS(ram_dp) ){
		snprintf(ERROR_STRING,LLEN,"Sorry, object %s must be contiguous for string reading",
			OBJ_NAME(ram_dp));
		warn(ERROR_STRING);
		assert(ram_dp==dp);
		return;
	}

	dst_str = (char *) OBJ_DATA_PTR(ram_dp);
	strncpy(dst_str,src_str,dst_size-1);
	dst_str[dst_size-1] = 0;	// guarantee string termination

	RELEASE_RAM_OBJ_FOR_WRITING_IF(dp)
}

static COMMAND_FUNC( do_disp_obj )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ
	FILE *fp;

	dp=pick_obj("");
	if( dp==NULL ) return;

	// We used to insist that the object be in RAM,
	// but we make life easier by automatically creating
	// a temporary object...

	INSURE_OK_FOR_READING(dp)

	fp = tell_msgfile();
	if( fp == stdout ){
		if( IS_IMAGE(ram_dp) || IS_SEQUENCE(ram_dp) )
			if( !confirm(
		"are you sure you want to display an image/sequence in ascii") )
				return;
		list_dobj(ram_dp);
	}
	pntvec(ram_dp,fp);
	fflush(fp);

	RELEASE_RAM_OBJ_FOR_READING_IF(dp)
}

/* BUG wrvecd will not work correctly for subimages */

/* Now that we can redirect the output file, and do_disp_obj prints
 * to msgfile, we really don't need this... maybe delete it later.
 */

static COMMAND_FUNC( do_wrt_obj )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ
	FILE *fp;
	/* BUG what if pathname is longer than 256??? */
	const char *filename;

	dp=pick_obj("");
	filename = nameof("output file");

	if( dp==NULL ) return;

	if( strcmp(filename,"-") && strcmp(filename,"stdout") ){
		// BUG? we don't check append flag here,
		// but there is a separate append command...

		fp=try_nice( filename, "w" );
		if( !fp ) return;
	} else {
		// If the invoking script has redirected stdout,
		// then use that
		if( QS_MSG_FILE(THIS_QSP)!=NULL ){
			fp = QS_MSG_FILE(THIS_QSP);
		} else
			fp = stdout;
	}

	if( IS_IMAGE(dp) || IS_SEQUENCE(dp) )
		if( !confirm(
		"are you sure you want to write an image/sequence in ascii") ){
			fclose(fp);
			return;
		}

	INSURE_OK_FOR_READING(dp)

	pntvec(ram_dp,fp);
	if( fp != stdout && QS_MSG_FILE(THIS_QSP)!=NULL && fp != QS_MSG_FILE(THIS_QSP) ) {
		if( verbose ){
			snprintf(MSG_STR,LLEN,"closing file %s",filename);
			prt_msg(MSG_STR);
		}
		fclose(fp);
	}

	RELEASE_RAM_OBJ_FOR_READING_IF(dp)
}

static COMMAND_FUNC( do_append )
{
	Data_Obj *dp;
	DECL_RAM_DATA_OBJ
	FILE *fp;

	dp=pick_obj("");
	if( dp==NULL ) return;

	if( IS_IMAGE(dp) || IS_SEQUENCE(dp) )
		if( !confirm(
		"are you sure you want to write an image/sequence in ascii") )
			return;
	fp=try_nice( nameof("output file"), "a" );
	if( !fp ) return;

	INSURE_OK_FOR_READING(dp)

	pntvec(ram_dp,fp);
	fclose(fp);

	RELEASE_RAM_OBJ_FOR_READING_IF(dp)
}

static COMMAND_FUNC( do_set_fmt )
{
	Integer_Output_Fmt *iof_p;

	iof_p = pick_int_out_fmt("format");
	if( iof_p == NULL ) return;
	set_integer_print_fmt(iof_p);
}

static COMMAND_FUNC( do_set_max )
{
	set_max_per_line( (int) HOW_MANY("max number of items per line") );
}

static COMMAND_FUNC( do_set_in_fmt )
{
	const char *s;

	s=nameof("input line format string");
	set_input_format_string(s);
}

static COMMAND_FUNC( do_exact )
{
	expect_exact_count = ASKIF("expect ascii files to contain exactly the right number of elements");
}

static COMMAND_FUNC( do_set_digits )
{
	int d;
	d = (int)HOW_MANY("number of significant digits to print");
	set_display_precision(d);
}

#define ADD_CMD(s,f,h)	ADD_COMMAND(ascii_menu,s,f,h)

MENU_BEGIN(ascii)

ADD_CMD( display,	do_disp_obj,	display data	)
ADD_CMD( read,		do_read_obj,	read vector data from ascii file	)
ADD_CMD( read_from_input_stream,	do_read_obj_from_stream,	read vector data from input stream (exits menu) )
ADD_CMD( pipe_read,	do_pipe_obj,	read vector data from named pipe	)
ADD_CMD( set_string,	do_set_obj_from_var,	read vector data from a string	)
ADD_CMD( get_string,	do_set_var_from_obj,	set a script variable to the value of a stored string	)
ADD_CMD( write,		do_wrt_obj,	write vector data to ascii file	)
ADD_CMD( append,	do_append,	append vector data to ascii file	)
ADD_CMD( exact,		do_exact,	enable/disable warnings for file/vector length mismatch	)
ADD_CMD( output_fmt,	do_set_fmt,	set data print format	)
ADD_CMD( input_fmt,	do_set_in_fmt,	specify format of input line	)
ADD_CMD( max_per_line,	do_set_max,	set maximum number of items per line	)
ADD_CMD( digits,	do_set_digits,	set number of significant digits to print	)

MENU_END(ascii)

COMMAND_FUNC( asciimenu )			/** ascii object submenu */
{
	CHECK_AND_PUSH_MENU(ascii);
}

