#include "quip_config.h"

/* more generalized dimensions & subscripts added 8-92 jbm */

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#include <inttypes.h>	// PRId64 etc

#include "quip_prot.h"
#include "data_obj.h"
#include "ascii_fmts.h"
#include "dobj_private.h"
#include "query_stack.h"	// like to eliminate this dependency...
#ifdef HAVE_POPEN
#include "pipe_support.h"
#endif // HAVE_POPEN
#include "veclib/obj_args.h"	// argset_prec

// BUG - put this in dai_ struct!
static int n_this_line=0;	// just for postscript

static void init_format_type_tbl(void);


/*
 * For printing out data with more than one number per line,
 * ret_dim is the index of the dimension where a return is forced
 */

#define DEFAULT_MAX_PER_LINE	12
#define ENFORCED_MAX_PER_LINE	64

/* We have different needs for formats;
 * When we are print a table of dobj values, we'd like to have fixed length fields,
 * but when we are constructing filenames or strings from object values we don't
 * want any space padding...
 */
#define DEFAULT_MIN_FIELD_WIDTH		10
#define DEFAULT_DISPLAY_PRECISION	6

#define PADDED_FLT_FMT_STR	"%10.24g"		// not used???
#define PLAIN_FLT_FMT_STR	"%g"

// BUG - use sym const for buffer size
#define UPDATE_PADDED_FLOAT_FORMAT_STRING						\
	snprintf(padded_flt_fmt_str,16,"%%%d.%dg",min_field_width,display_precision);

#define NORMAL_SEPARATOR	" "
#define POSTSCRIPT_SEPARATOR	""

ITEM_INTERFACE_DECLARATIONS(Integer_Output_Fmt,int_out_fmt,0)


// when we call this, we quote only plain_fmt_str - so we can use the macros like PRId64...

#define DECLARE_FMT_FUNC(type,format,member,fmt_str_prefix,padding_fmt_str,plain_fmt_str)		\
													\
static void _format_##type##_data_##format(QSP_ARG_DECL  char *buf,int buflen, Scalar_Value *svp, int pad_flag)	\
{													\
	if( pad_flag ){											\
		snprintf(buf,buflen,#fmt_str_prefix #padding_fmt_str plain_fmt_str, svp->member);		\
	} else {											\
		snprintf(buf,buflen,#fmt_str_prefix plain_fmt_str, svp->member);				\
	}												\
}

#define DECLARE_PS_FMT_FUNC(type,format,member)						\
											\
static void _format_##type##_data_##format(QSP_ARG_DECL  char *buf, int buflen,Scalar_Value *svp, int pad_flag)	\
{											\
	snprintf(buf,buflen,"%02x",svp->member);							\
}


#define DECLARE_INVALID_FMT_FUNC(type,format)						\
											\
static void _format_##type##_data_##format(QSP_ARG_DECL  char *buf,int buflen, Scalar_Value *svp, int pad_flag)	\
{											\
	snprintf(ERROR_STRING,LLEN,"CAUTIOUS:  can't apply %s format to %s!?",#format,#type);	\
	error1(ERROR_STRING);								\
}

DECLARE_FMT_FUNC(string,	decimal,	u_ub,	%,	,	"c")
DECLARE_FMT_FUNC(char,		decimal,	u_ub,	%,	,	"c")
DECLARE_FMT_FUNC(byte,		decimal,	u_b,	%,	4,	PRId8)
DECLARE_FMT_FUNC(u_byte,	decimal,	u_ub,	%,	4,	PRIu8)
DECLARE_FMT_FUNC(short,		decimal,	u_s,	%,	6,	PRId16)
DECLARE_FMT_FUNC(u_short,	decimal,	u_us,	%,	6,	PRIu16)
DECLARE_FMT_FUNC(int,		decimal,	u_l,	%,	10,	PRId32)
DECLARE_FMT_FUNC(u_int,		decimal,	u_ul,	%,	10,	PRIu32)
DECLARE_FMT_FUNC(long,		decimal,	u_ll,	%,	10,	PRId64)
DECLARE_FMT_FUNC(u_long,	decimal,	u_ull,	%,	10,	PRIu64)
// How many digits is a 64 bit integer?

DECLARE_FMT_FUNC(string,	hex,	u_b,	%,	,	"c")
DECLARE_FMT_FUNC(char,		hex,	u_b,	%,	,	"c")
DECLARE_FMT_FUNC(byte,		hex,	u_ub,	0x%,	-3,	PRIx8)	// cast to unsigned
DECLARE_FMT_FUNC(u_byte,	hex,	u_ub,	0x%,	-3,	PRIx8)
DECLARE_FMT_FUNC(short,		hex,	u_us,	0x%,	-5,	PRIx16)	// cast to unsigned
DECLARE_FMT_FUNC(u_short,	hex,	u_us,	0x%,	-5,	PRIx16)
DECLARE_FMT_FUNC(int,		hex,	u_l,	0x%,	-9,	PRIx32)
DECLARE_FMT_FUNC(u_int,		hex,	u_ul,	0x%,	-9,	PRIx32)
DECLARE_FMT_FUNC(long,		hex,	u_ll,	0x%,	-17,	PRIx64)
DECLARE_FMT_FUNC(u_long,	hex,	u_ull,	0x%,	-17,	PRIx64)

DECLARE_FMT_FUNC(string,	octal,	u_ub,	%,	,	"c")
DECLARE_FMT_FUNC(char,		octal,	u_ub,	%,	,	"c")
DECLARE_FMT_FUNC(byte,		octal,	u_ub,	0%,	-4,	PRIo8)	// cast to unsigned
DECLARE_FMT_FUNC(u_byte,	octal,	u_ub,	0%,	-4,	PRIo8)	// 0377
DECLARE_FMT_FUNC(short,		octal,	u_us,	0%,	-5,	PRIo16)	// cast to unsigned
DECLARE_FMT_FUNC(u_short,	octal,	u_us,	0%,	-5,	PRIo16)
DECLARE_FMT_FUNC(int,		octal,	u_l,	0%,	-9,	PRIo32)
DECLARE_FMT_FUNC(u_int,		octal,	u_ul,	0%,	-9,	PRIo32)
DECLARE_FMT_FUNC(long,		octal,	u_ll,	0%,	-17,	PRIo64)
DECLARE_FMT_FUNC(u_long,	octal,	u_ull,	0%,	-17,	PRIo64)

DECLARE_FMT_FUNC(string,	unsigned,	u_ub,	%,	,	"c")
DECLARE_FMT_FUNC(char,		unsigned,	u_ub,	%,	,	"c")
DECLARE_FMT_FUNC(byte,		unsigned,	u_ub,	%,	4,	PRIu8)
DECLARE_FMT_FUNC(u_byte,	unsigned,	u_ub,	%,	4,	PRIu8)
DECLARE_FMT_FUNC(short,		unsigned,	u_us,	%,	6,	PRIu16)
DECLARE_FMT_FUNC(u_short,	unsigned,	u_us,	%,	6,	PRIu16)
DECLARE_FMT_FUNC(int,		unsigned,	u_ul,	%,	10,	PRIu32)
DECLARE_FMT_FUNC(u_int,		unsigned,	u_ul,	%,	10,	PRIu32)
DECLARE_FMT_FUNC(long,		unsigned,	u_ll,	%,	10,	PRIu64)
DECLARE_FMT_FUNC(u_long,	unsigned,	u_ull,	%,	10,	PRIu64)

DECLARE_FMT_FUNC(string,	postscript,	u_ub,	%,	,	"c")
DECLARE_FMT_FUNC(char,		postscript,	u_ub,	%,	,	"c")

DECLARE_PS_FMT_FUNC(byte,	postscript,	u_ub )
DECLARE_PS_FMT_FUNC(u_byte,	postscript,	u_ub )

// for longer types, just revert to normal hex
DECLARE_FMT_FUNC(short,		postscript,	u_us,	0x%,	-5,	PRIx16)	// cast to unsigned
DECLARE_FMT_FUNC(u_short,	postscript,	u_us,	0x%,	-5,	PRIx16)
DECLARE_FMT_FUNC(int,		postscript,	u_ul,	0x%,	-9,	PRIx32)	// cast to unsigned
DECLARE_FMT_FUNC(u_int,		postscript,	u_ul,	0x%,	-9,	PRIx32)
DECLARE_FMT_FUNC(long,		postscript,	u_ll,	0x%,	-17,	PRIx64)
DECLARE_FMT_FUNC(u_long,	postscript,	u_ull,	0x%,	-17,	PRIx64)

/* get a pixel from the input stream, store data at *cp */

/* We often encounter situations where we have data stored with text, e.g.:
 *
 * FRAME 10 pup_x 10 pup_y 9
 * FRAME 11 pup_x 11 pup_y 8
 * ...
 *
 * The approach we have taken in the past is to use dm to pick out
 * the numeric fields...  We would like to add that capability here...
 * We allow the user to specify a format string:
 *
 * FRAME %d %s %f %s %d
 *
 * The rules are:
 * 	1) anything not starting with '%' is a literal string, read w/ nameof().
 *	2) literal strings must be matched, or a warning is printed
 *	3) %s strings are read w/ nameof() and discarded
 *	4) %d, %o, %x read w/ how_many()
 *	5) %f, %g read w/ how_much()
 */

#define INIT_OUTPUT_FORMAT(name,code,padded_fmt_str,plain_fmt_str,prefix)\
	iof_p = new_int_out_fmt(#name);					\
	assert(iof_p!=NULL);						\
	iof_p->iof_code = code;						\
	iof_p->iof_padded_fmt_str = padded_fmt_str;			\
	iof_p->iof_plain_fmt_str = plain_fmt_str;			\
	iof_p->iof_fmt_string_func = _format_string_data_##name;	\
	iof_p->iof_fmt_char_func = _format_char_data_##name;		\
	iof_p->iof_fmt_byte_func = _format_byte_data_##name;		\
	iof_p->iof_fmt_u_byte_func = _format_u_byte_data_##name;	\
	iof_p->iof_fmt_short_func = _format_short_data_##name;		\
	iof_p->iof_fmt_u_short_func = _format_u_short_data_##name;	\
	iof_p->iof_fmt_int_func = _format_int_data_##name;		\
	iof_p->iof_fmt_u_int_func = _format_u_int_data_##name;		\
	iof_p->iof_fmt_long_func = _format_long_data_##name;		\
	iof_p->iof_fmt_u_long_func = _format_u_long_data_##name;	\


#define init_output_formats() _init_output_formats(SINGLE_QSP_ARG)

static void _init_output_formats(SINGLE_QSP_ARG_DECL)
{
	Integer_Output_Fmt *iof_p;

	INIT_OUTPUT_FORMAT(decimal,	FMT_DECIMAL,	"%10ld",	"%ld",		dec)
	curr_output_int_fmt_p = iof_p;

	INIT_OUTPUT_FORMAT(hex,		FMT_HEX,	"0x%-10lx",	"0x%lx",	hex)
	INIT_OUTPUT_FORMAT(octal,	FMT_OCTAL,	"0%-10lo",	"0%lo",		oct)
	INIT_OUTPUT_FORMAT(unsigned,	FMT_UDECIMAL,	"%10lu",	"%lu",		uns)
	INIT_OUTPUT_FORMAT(postscript,	FMT_POSTSCRIPT,	"%02x",		"%02x",		pst)
}



// This global defines the format types used by all threads
// and is not modified once initialized.

static struct input_format_type input_format_type_tbl[N_INPUT_FORMAT_TYPES];

void _init_dobj_ascii_info(QSP_ARG_DECL  Dobj_Ascii_Info *dai_p)
{
	dai_p->dai_ascii_data_dp = NULL;
	dai_p->dai_ascii_warned = 0;
	dai_p->dai_dobj_max_per_line = DEFAULT_MAX_PER_LINE;
	dai_p->dai_min_field_width = DEFAULT_MIN_FIELD_WIDTH;
	dai_p->dai_display_precision = DEFAULT_DISPLAY_PRECISION;
	dai_p->dai_fmt_lp = NULL;

	UPDATE_PADDED_FLOAT_FORMAT_STRING

	if( int_out_fmt_itp == NULL ) init_output_formats();
}

static void show_input_format(SINGLE_QSP_ARG_DECL)
{
	if( ! HAS_FORMAT_LIST ){
		advise("no input format specified");
		return;
	}

	CURRENT_FORMAT_NODE = FIRST_INPUT_FORMAT_NODE;
	while( CURRENT_FORMAT_NODE != NULL ){
		if( ! IS_FIRST_FORMAT )
			prt_msg_frag(" ");
		CURRENT_FORMAT->fmt_type->display_format(QSP_ARG  CURRENT_FORMAT);
		CURRENT_FORMAT_NODE = NODE_NEXT(CURRENT_FORMAT_NODE);
	}
	prt_msg("");
}

static void display_int_format(QSP_ARG_DECL  Input_Format_Spec *fmt_p)
{
	prt_msg_frag("%d");
}

static void display_literal_format(QSP_ARG_DECL  Input_Format_Spec *fmt_p)
{
	prt_msg_frag(fmt_p->fmt_litstr);
}

static void display_string_format(QSP_ARG_DECL  Input_Format_Spec *fmt_p)
{
	prt_msg_frag("%s");
}

static void display_float_format(QSP_ARG_DECL  Input_Format_Spec *fmt_p)
{
	prt_msg_frag("%f");
}

static /*Input_Format_Type*/ struct input_format_type *format_type_for_code(Input_Format_Type_Code c)
{
	static int inited=0;

	if( ! inited ){
		init_format_type_tbl();
		inited=1;
	}
	assert( c >= 0 && c < N_INPUT_FORMAT_TYPES );
	return( & input_format_type_tbl[c] );
}

static Input_Format_Spec *new_format_spec(Input_Format_Type_Code c)
{
	Input_Format_Spec *fmt_p;

	fmt_p = getbuf(sizeof(*fmt_p));
	fmt_p->fmt_type = format_type_for_code(c);
	fmt_p->fmt_litstr = NULL;
	return fmt_p;
}

#define add_format_for_code(c) _add_format_for_code(QSP_ARG  c)

static void _add_format_for_code(QSP_ARG_DECL  Input_Format_Type_Code c)
{
	Input_Format_Spec *fmt_p;
	Node *np;

	fmt_p = new_format_spec(c);
	np = mk_node(fmt_p);
	if( ! HAS_FORMAT_LIST )
		INPUT_FORMAT_LIST = new_list();
	addTail( INPUT_FORMAT_LIST, np );
}

static void release_format_list(SINGLE_QSP_ARG_DECL)
{
	Node *np;
	Input_Format_Spec *fmt_p;

	np = QLIST_HEAD(INPUT_FORMAT_LIST);
	while(np!=NULL){
		fmt_p = (Input_Format_Spec *) NODE_DATA(np);
		(*(fmt_p->fmt_type->release))(fmt_p);
		np = NODE_NEXT(np);
	}
	dellist(INPUT_FORMAT_LIST);
	INPUT_FORMAT_LIST = NULL;
}

#define set_literal_format_string(s) _set_literal_format_string(QSP_ARG  s)

static void _set_literal_format_string(QSP_ARG_DECL  char *s)
{
	Input_Format_Spec *fmt_p;
	Node *np;

	np = QLIST_TAIL(INPUT_FORMAT_LIST);
	assert(np!=NULL);
	fmt_p = NODE_DATA(np);
	assert( fmt_p != NULL );
	assert( fmt_p->fmt_type->type_code == IN_FMT_LIT );
	fmt_p->fmt_litstr = savestr(s);
}

#define MAX_LIT_STR_LEN	255

#define read_literal_format_string(sptr) _read_literal_format_string(QSP_ARG  sptr)

static void _read_literal_format_string(QSP_ARG_DECL  const char **sptr)
{
	const char *s;
	int i=0;
	char lit_str[MAX_LIT_STR_LEN+1];

	s = *sptr;
	while( *s && !isspace(*s) && i < MAX_LIT_STR_LEN )
		lit_str[i++]=(*s++);
	lit_str[i] = 0;
	if( *s && !isspace(*s) )
		warn("literal string overflow in input format spec");

	*sptr = s;

	set_literal_format_string(lit_str);
}

#define process_format_char(sptr) _process_format_char(QSP_ARG  sptr )

static int _process_format_char(QSP_ARG_DECL  const char **sptr )
{
	const char *s = *sptr;
	int c = *s;

	switch(c){
		case 0:
			return -1;

		case 'd':
		case 'x':
		case 'o':
		case 'i':
			add_format_for_code(IN_FMT_INT);
			break;
		case 'f':
		case 'g':
			add_format_for_code(IN_FMT_FLT);
			break;
		case 's':
			add_format_for_code(IN_FMT_STR);
			break;
	}
	s++;

	if( *s && !isspace(*s) ){
		snprintf(ERROR_STRING,LLEN,
			"white space should follow format descriptor!?");
		warn(ERROR_STRING);
	}

	*sptr = s;
	return 0;
}

#define process_format_string_char(sptr) _process_format_string_char(QSP_ARG  sptr)

static int _process_format_string_char(QSP_ARG_DECL  const char **sptr)
{
	const char *s = *sptr;
	int c;

	c = *s++;
	if( isspace(c) ){
		while( *s && isspace(*s) )
			s++;
	} else if( c == '%' ){
		if( process_format_char(&s) < 0 ){
			*sptr = s;
			return -1;
		}
	} else {	/* literal string */
		s--;
		add_format_for_code(IN_FMT_LIT);
		read_literal_format_string(&s);
	}
	*sptr = s;
	return 0;
}

void _set_input_format_string( QSP_ARG_DECL  const char *s )
{
	const char *orig_str=s;
	//Input_Format_Spec *fmt_p;

	if( HAS_FORMAT_LIST ) release_format_list(SINGLE_QSP_ARG);
	/* parse the string */

	while( *s ){
		if( process_format_string_char(&s) < 0 ){
			snprintf(ERROR_STRING,LLEN,
		"Poorly formed input format string \"%s\"" , orig_str);
			warn(ERROR_STRING);
			// BUG?  clean up by releasing format?
			return;
		}
	}
}

static void literal_format_release(Input_Format_Spec *fmt_p)
{
	rls_str( (char *) fmt_p->fmt_litstr );
}

static void default_format_release(Input_Format_Spec *fmt_p)
{ /* nop */ }

static int float_format_read_long(QSP_ARG_DECL  int64_t *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	if( !ascii_warned ){
		snprintf(ERROR_STRING,LLEN,
			"Float format data assigned to integer object %s!?",
			OBJ_NAME( ascii_data_dp) );
		warn(ERROR_STRING);
		ascii_warned=1;
	}

	*result = (int64_t) how_much(pmpt);
	return 1;
}

static int int_format_read_long(QSP_ARG_DECL  int64_t *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	*result = how_many(pmpt);
	return 1;
}

#define consume_format_line(prec_p) _consume_format_line(QSP_ARG  prec_p)

static void _consume_format_line(QSP_ARG_DECL  Precision *prec_p)
{
	Input_Format_Spec *fmt_p;

	do {
		if( QLEVEL != ASCII_LEVEL ){
			warn("Incomplete formatted input line!?");
			return;
		}
		fmt_p = CURRENT_FORMAT;
		fmt_p->fmt_type->consume(QSP_ARG  prec_p);
		lookahead_til(ASCII_LEVEL-1);
	} while( CURRENT_FORMAT_NODE != FIRST_INPUT_FORMAT_NODE );
}

static int is_numeric_prec( Precision *prec_p )
{
	return (*(prec_p->is_numeric_func))();
}

#define consume_literal_string(fmt_p) _consume_literal_string(QSP_ARG  fmt_p)

static inline void _consume_literal_string(QSP_ARG_DECL  Input_Format_Spec *fmt_p)
{
	const char *s;
	s=nameof(fmt_p->fmt_litstr);
	if( strcmp(s,fmt_p->fmt_litstr) ){
		snprintf(ERROR_STRING,LLEN,
	"expected literal string \"%s\", saw string \"%s\"",
			fmt_p->fmt_litstr,s);
		warn(ERROR_STRING);
	}
}

#define RESET_INPUT_FORMAT_FIELD	CURRENT_FORMAT_NODE = QLIST_HEAD(INPUT_FORMAT_LIST);

#define advance_format() _advance_format(SINGLE_QSP_ARG)

static inline void _advance_format(SINGLE_QSP_ARG_DECL)
{
	assert(HAS_FORMAT_LIST);
	if( CURRENT_FORMAT_NODE == NULL )
		CURRENT_FORMAT_NODE = FIRST_INPUT_FORMAT_NODE;
	else {
		CURRENT_FORMAT_NODE = NODE_NEXT(CURRENT_FORMAT_NODE);
		if( CURRENT_FORMAT_NODE == NULL )
			CURRENT_FORMAT_NODE = FIRST_INPUT_FORMAT_NODE;
	}
	assert(CURRENT_FORMAT_NODE!=NULL);
}

/*
 * get the next number (int or float format),
 * skipping strings and literals
 *
 * 
 */

int64_t _next_input_int_with_format(QSP_ARG_DECL   const char *pmpt)
{
	int64_t l=0;
	int done=0;

	do {
		assert(INPUT_FORMAT_LIST != NULL);
		assert(CURRENT_FORMAT_NODE != NULL);
		assert(CURRENT_FORMAT != NULL);
		done = CURRENT_FORMAT->fmt_type->read_long(QSP_ARG  &l, pmpt, CURRENT_FORMAT);
		advance_format();
	} while(!done);

	return(l);
}

/*
 * Read input fields until a number is encountered
 */

double _next_input_flt_with_format(QSP_ARG_DECL  const char *pmpt)
{
	int done=0;
	double d=0.0;

	do {
		assert(CURRENT_FORMAT_NODE != NULL);
		assert(CURRENT_FORMAT != NULL);
		done = CURRENT_FORMAT->fmt_type->read_double(QSP_ARG  &d, pmpt, CURRENT_FORMAT);
		advance_format();
	} while(!done);

	return(d);
}

// The "consume" methods read and discard a field, UNLESS the precision code
// is appropriate for that format.

static void int_format_consume(QSP_ARG_DECL  Precision *prec_p)
{
	if( is_numeric_prec(prec_p) ){
		RESET_INPUT_FORMAT_FIELD
		return;
	} else {
		howmany_type l;
		l=how_many("dummy integer");
        if( verbose ){      // this was added just to suppress a compiler warning about never-read value l
            snprintf(ERROR_STRING,LLEN,"Ignoring integer value %lld",l);  // BUG? use format macro
            advise(ERROR_STRING);
        }
		advance_format();
	}
}

static void float_format_consume(QSP_ARG_DECL  Precision *prec_p)
{
	if( is_numeric_prec(prec_p) ){
		RESET_INPUT_FORMAT_FIELD
		return;
	} else {
		double d;
		d=how_much("dummy float");
        if( verbose ){      // this was added just to suppress a compiler warning about never-read value
            snprintf(ERROR_STRING,LLEN,"Ignoring float value %g",d);
            advise(ERROR_STRING);
        }

		advance_format();
	}
}

static void string_format_consume(QSP_ARG_DECL  Precision *prec_p)
{
	if( PREC_CODE(prec_p) == PREC_STR ){
		RESET_INPUT_FORMAT_FIELD
		return;
	} else {
		const char *s;
		s=nameof("dummy string");
        if( verbose ){      // this was added just to suppress a compiler warning about never-read value
            snprintf(ERROR_STRING,LLEN,"Ignoring string value \"%s\"",s);
            advise(ERROR_STRING);
        }

		advance_format();
	}
}

static void literal_format_consume(QSP_ARG_DECL  Precision *prec_p)
{
	consume_literal_string(CURRENT_FORMAT);
	advance_format();
}


static void consume_variable_string(SINGLE_QSP_ARG_DECL)
{
	/*s=*/nameof("don't-care string");
}

static int string_format_read_long(QSP_ARG_DECL  int64_t *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	consume_variable_string(SINGLE_QSP_ARG);
	return 0;
}

static int literal_format_read_long(QSP_ARG_DECL  int64_t *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	consume_literal_string(fmt_p);
	return 0;
}

static int int_format_read_double(QSP_ARG_DECL  double *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	*result = how_many(pmpt);
	return 1;
}

static int float_format_read_double(QSP_ARG_DECL  double *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	*result = how_much(pmpt);
	return 1;
}

static int string_format_read_double(QSP_ARG_DECL  double *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	consume_variable_string(SINGLE_QSP_ARG);
	return 0;
}

static int literal_format_read_double(QSP_ARG_DECL  double *result, const char *pmpt, Input_Format_Spec *fmt_p)
{
	consume_literal_string(fmt_p);
	return 0;
}

#define next_input_str(pmpt) _next_input_str(QSP_ARG  pmpt)

static const char * _next_input_str(QSP_ARG_DECL  const char *pmpt)
{
	const char *s = NULL;	/* quiet compiler to elim possibly used w/o init warning */
	int done=0;

	do {
		assert(CURRENT_FORMAT != NULL);
		done = CURRENT_FORMAT->fmt_type->read_string(QSP_ARG  &s, pmpt, CURRENT_FORMAT);
		advance_format();
	} while(!done);

	return(s);
}

static int int_format_read_string(QSP_ARG_DECL  const char **sptr, const char *pmpt, Input_Format_Spec *fmt_p)
{
	howmany_type l;
	l=how_many("dummy integer value");
    if( verbose ){      // this was added just to suppress a compiler warning about never-read value
        snprintf(ERROR_STRING,LLEN,"Ignoring integer value %lld",l);  // BUG use format macro
        advise(ERROR_STRING);
    }

	return 0;
}

static int float_format_read_string(QSP_ARG_DECL  const char **sptr, const char *pmpt, Input_Format_Spec *fmt_p)
{
	double d;
	d=how_much("dummy float value");
    if( verbose ){      // this was added just to suppress a compiler warning about never-read value
        snprintf(ERROR_STRING,LLEN,"Ignoring double value %g",d);
        advise(ERROR_STRING);
    }

	return 0;
}

static int string_format_read_string(QSP_ARG_DECL  const char **sptr, const char *pmpt, Input_Format_Spec *fmt_p)
{
	*sptr = nameof(pmpt);
	return 1;
}

static int literal_format_read_string(QSP_ARG_DECL  const char **sptr, const char *pmpt, Input_Format_Spec *fmt_p)
{
	consume_literal_string(fmt_p);
	return 0;
}

#define check_input_level() _check_input_level(SINGLE_QSP_ARG)

static inline int _check_input_level(SINGLE_QSP_ARG_DECL)
{
	if( QLEVEL != ASCII_LEVEL ){
		if( verbose ){
			snprintf(ERROR_STRING,LLEN,
				"check_input_level (ascii):  input depth is %d, expected %d!?",
				QLEVEL,ASCII_LEVEL);
			advise(ERROR_STRING);
		}
		snprintf(ERROR_STRING,LLEN,"premature end of data (%d elements read so far)",dobj_n_gotten);
		warn(ERROR_STRING);

		if( HAS_FORMAT_LIST ){
			prt_msg_frag("input_format:  ");
			show_input_format(SINGLE_QSP_ARG);
		}
		return(-1);
	}
	return 0;
}

#define get_a_string(dp,datap,dim) _get_a_string(QSP_ARG  dp,datap,dim)

static int _get_a_string(QSP_ARG_DECL  Data_Obj *dp,char *datap,int dim)
{
	const char *s, *orig;
	char *t;
	dimension_t i;

	assert( dim >= 0 );

	if( check_input_level() < 0 ) return(-1);

	/* see if we need to look at the input format string */
	if( ! HAS_FORMAT_LIST )
		s = nameof("string data");
	else
		s = next_input_str("string data");

	t=datap;

	// Old (deleted) code assumed strings were in image rows...
	// This new code takes the dimension passed in the arg.
	i=0;
	orig=s;
	while( *s && i < DIMENSION(OBJ_TYPE_DIMS(dp),dim) ){
		*t = *s;
		t += INCREMENT(OBJ_TYPE_INCS(dp),dim);
		s++;
		i++;
	}
	if( i >= DIMENSION(OBJ_TYPE_DIMS(dp),dim) ){
		t -= INCREMENT(OBJ_TYPE_INCS(dp),dim);
		snprintf(ERROR_STRING,LLEN,
"get_a_string:  input string (%ld chars) longer than data buffer (%ld chars)",
			(long)(strlen(orig)+1),
			(long)DIMENSION(OBJ_TYPE_DIMS(dp),dim));
		warn(ERROR_STRING);
	}
	*t = 0;	// add null terminator

	/* now lookahead to pop the file if it is empty */
	lookahead_til(ASCII_LEVEL-1);

	return(0);
} // get_a_string

#ifdef HAVE_ANY_GPU
int _object_is_in_ram(QSP_ARG_DECL  Data_Obj *dp, const char *op_str)
{
	static Data_Obj *warned_dp=NULL;

	if( ! OBJ_IS_RAM(dp) ){
		if( dp != warned_dp ){
			snprintf(ERROR_STRING,LLEN,
		"Object %s is not in host ram, cannot %s!?",
				OBJ_NAME(dp), op_str);
			warn(ERROR_STRING);
			warned_dp=dp;
		}
		return 0;
	}
	return 1;
}
#endif // HAVE_ANY_GPU

#define get_next_element(dp,datap) _get_next_element(QSP_ARG   dp,datap)

static int _get_next_element(QSP_ARG_DECL   Data_Obj *dp,void *datap)
{
	Precision *prec_p;
	char *prompt;

	if( check_input_level() < 0 ) return(-1);

	// should the old test have been >= instead of > ???
	assert( OBJ_MACH_PREC(dp) < N_MACHINE_PRECS );
	

#ifdef QUIP_DEBUG
if( debug & debug_data ){
snprintf(ERROR_STRING,LLEN,"get_next_element:  getting a %s value for address 0x%lx",
OBJ_MACH_PREC_NAME(dp),(u_long)datap);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */

	prec_p = OBJ_MACH_PREC_PTR(dp);
	prompt = msg_str;	// use this buffer...
	snprintf(prompt,LLEN,"%s data",PREC_NAME(prec_p));
	(*(prec_p->set_value_from_input_func))(QSP_ARG  datap, prompt);

	dobj_n_gotten++;

	/* now lookahead to pop the file if it is empty */
	lookahead_til(ASCII_LEVEL-1);

	return(0);
} /* end get_next_element() */

#define bit_set_value_from_input(wp, i_bit ) _bit_set_value_from_input(QSP_ARG  wp, i_bit )

static void _bit_set_value_from_input(QSP_ARG_DECL  bitmap_word *wp, bitnum_t i_bit )
{
	bitmap_word val;
	bitmap_word bit;

	if( ! HAS_FORMAT_LIST )
		val = (bitmap_word) how_many("bit value");
	else
		val = (bitmap_word) next_input_int_with_format("bit value");

	if( /* val < 0 || */ val > 1 ){     // bitmap_word is an unsigned type
		warn("Truncation error converting bit");
	}
	// 1ull forces this constant to be 64 bits, unsigned!
	bit = BITMAP_ONE << (i_bit % BITS_PER_BITMAP_WORD);

	if( val == 0 ){
		*( wp + i_bit/BITS_PER_BITMAP_WORD ) &= ~bit;
	} else {
		*( wp + i_bit/BITS_PER_BITMAP_WORD ) |=  bit;
	}
}

#define get_next_bit(ptr, bit0) _get_next_bit(QSP_ARG  ptr, bit0)

static int _get_next_bit(QSP_ARG_DECL  void *ptr, bitnum_t bit0)
{
	if( ptr != NULL )
		bit_set_value_from_input((bitmap_word *) ptr, bit0 );
	return 0;
}

#ifdef BITMAP_FOOBAR
static int64_t get_bit_from_bitmap(Data_Obj *dp, void *data)
{
	int which_bit;
	int64_t l;

	/* if it's a bitmap, we don't really have the address of
	 * the data, we need to get the bit offset...
	 */

	/* We encode the bit in the data address - it's not the real address. */
	which_bit = ((u_char *)data) - ((u_char *)OBJ_DATA_PTR(dp));
snprintf(ERROR_STRING,LLEN,"get_bit_from_bitmap:  which_bit = %d",which_bit);
advise(ERROR_STRING);
	which_bit += OBJ_BIT0(dp);

	/* now we know which bit it is, but we still need to figure
	 * out which word...  If this is a subobject of a bit image,
	 * we are ok, because the ultimate ancestor will be contiguous,
	 * but we can get in trouble if it is an equivalence of
	 * a non-contiguous object...  not sure how to handle this?
	 */

	data = ((BITMAP_DATA_TYPE *)OBJ_DATA_PTR(dp)) +
		(which_bit>>LOG2_BITS_PER_BITMAP_WORD);
	l= (int64_t)(* (BITMAP_DATA_TYPE *) data );
	which_bit &= BIT_NUMBER_MASK;

	if( l & (1L<<which_bit) )
		l=1;
	else
		l=0;
	return(l);
}
#endif /* BITMAP_FOOBAR */


/*
 * Format one (possibly multidimensional) pixel value
 *
 * It seems we are confused about what to do about bitmaps - BUG?
 */

void _format_scalar_obj(QSP_ARG_DECL  char *buf,int buflen,Data_Obj *dp,void *data, int pad_flag)
{
	//int64_t l;
	int c;

	if( OBJ_PREC(dp) == PREC_CHAR || OBJ_PREC(dp) == PREC_STR ){
		c=(*(unsigned char *)data);
		// BUG we don't check against buflen here, but should be OK
		if( isalnum(c) || ispunct(c) )
			snprintf(buf,buflen," '%c'",c);	// add a space to be the same as 4 octal chars
		else
			snprintf(buf,buflen,"0%03o",c);
		return;
	}

	if( ! IS_BITMAP(dp) ){
		// Why do we pad by default???
		format_scalar_value(buf,buflen,data,OBJ_PREC_PTR(dp),/*PAD_FOR_EVEN_COLUMNS*/ pad_flag);
	}
	else {
		warn("format_scalar_obj:  don't know what to do with bitmaps!?");
		/*
		l = get_bit_from_bitmap(dp,data);
		snprintf(buf,buflen,ifmtstr,l);
		*/
	}
}

// BUG?
// We cast the data to the largest possible size, but do we want sign extension?
// Yes, but not for display in hex and octal...
// This is tricky, because the print function doesn't know the source type of the data...

void _format_scalar_value(QSP_ARG_DECL  char *buf,int buflen,void *data,Precision *prec_p, int pad_flag)
{
	// BUG - buflen is ignored?
	(*(PREC_MACH_PREC_PTR(prec_p)->format_func))(QSP_ARG  buf,buflen,data,pad_flag);
}

char * string_for_scalar(QSP_ARG_DECL  void *data,Precision *prec_p )
{
	static char buf[64];

	format_scalar_value(buf,64,data,prec_p,PAD_FOR_EVEN_COLUMNS);
	return buf;
}

Precision *src_prec_for_argset_prec(Argset_Prec * ap_p, argset_type at)
{
	int code=PREC_NONE;

	switch(ARGSPREC_CODE(ap_p)){
		case BY_ARGS:	code=PREC_BY; break;
		case IN_ARGS:	code=PREC_IN; break;
		case DI_ARGS:	code=PREC_DI; break;
		case LI_ARGS:	code=PREC_LI; break;
		case SP_ARGS:
			switch(at){
				case REAL_ARGS: 	code=PREC_SP; break;
				case MIXED_ARGS:
				case COMPLEX_ARGS:	code=PREC_CPX; break;
				case QMIXED_ARGS:
				case QUATERNION_ARGS:	code=PREC_QUAT; break;
				default:
					assert( AERROR("bad argset type in src_prec_for_argset_prec()") );
					break;
			}
			break;
		case DP_ARGS:
			switch(at){
				case REAL_ARGS: 	code=PREC_DP; break;
				case MIXED_ARGS:
				case COMPLEX_ARGS:	code=PREC_DBLCPX; break;
				case QMIXED_ARGS:
				case QUATERNION_ARGS:	code=PREC_DBLQUAT; break;
				default:
					assert( AERROR("bad argset type in src_prec_for_argset_prec()") );
					break;
			}
			break;
		case UBY_ARGS:	code=PREC_UBY; break;
		case UIN_ARGS:	code=PREC_UIN; break;
		case UDI_ARGS:	code=PREC_UDI; break;
		case ULI_ARGS:	code=PREC_ULI; break;
		case BYIN_ARGS:	code=PREC_BY; break;
		case INBY_ARGS:	code=PREC_IN; break;
		case INDI_ARGS:	code=PREC_IN; break;
		case SPDP_ARGS:	code=PREC_SP; break;
		default:
			assert( AERROR("bad argset prec in src_prec_for_argset_prec()") );
			break;
	}

	return( PREC_FOR_CODE(code) );
}

/*
 * Print one (possibly multidimensional) pixel value
 *
 * BUG - if we want it to go to a redirected output (prt_msg_vec),
 * it won't happen, because this doesn't use prt_msg!?
 *
 * We don't necessarily want to use prt_msg, because this can be
 * called both from a display function and a write function...
 */

#define pnt_one( fp, dp,  data ) _pnt_one(QSP_ARG  fp, dp,  data )

static void _pnt_one(QSP_ARG_DECL  FILE *fp, Data_Obj *dp,  u_char *data )
{
	char buf[128];

	if( OBJ_MACH_DIM(dp,0) > 1 ){	/* not real, complex or higher */
		if( IS_STRING(dp) ){
			fprintf(fp,"%s",data);
			n_this_line++;
		} else {
			incr_t inc;
			dimension_t j;

			inc = (ELEMENT_SIZE(dp) * OBJ_MACH_INC(dp,0));
			for(j=0;j<OBJ_MACH_DIM(dp,0);j++){
				/* WHen this code was written, we didn't anticipate
				 * having too many components, so ret_dim=0 means
				 * print a return after each pixel.  But if
				 * we have many many components, then the resulting
				 * lines can be too long!?  Fixing this here is
				 * kind of a hack...
				 */
				if( j>0 && OBJ_MACH_DIM(dp,0) > dobj_max_per_line ){
					fprintf(fp,"\n");
					n_this_line = 0;
				}
				format_scalar_obj(buf,128,dp,data,PAD_FOR_EVEN_COLUMNS);
				// put a space here, because format_scalar_obj does not include a separator
				fprintf(fp," %s",buf);
				data += inc;
				n_this_line++;
			}
		}
	} else {
		format_scalar_obj(buf,128,dp,data,PAD_FOR_EVEN_COLUMNS);
		fprintf(fp," %s",buf);
		n_this_line++;
	}
} /* end pnt_one */

#define pnt_dim( fp, dp, data, dim ) _pnt_dim( QSP_ARG  fp, dp, data, dim )

static void _pnt_dim( QSP_ARG_DECL  FILE *fp, Data_Obj *dp, unsigned char *data, int dim )
{
	dimension_t i;
	incr_t inc;

	assert( ! IS_BITMAP(dp) );

	// Old code assumed that strings were the rows
	// of an image, but other code seemed to insist
	// that strings are multi-dimensional pixels...
	// By using OBJ_MINDIM, we handle both cases.

	if( dim==OBJ_MINDIM(dp) && OBJ_PREC(dp) == PREC_STR ){
		/* here we use .* to put the max number of chars
		 * to print in the next arg (is this a gcc
		 * extension or standard?).
		 * We do this because we are not guaranteed
		 * a null terminator char.
		 */
		fprintf(fp,"%.*s\n",(int)OBJ_DIMENSION(dp,OBJ_MINDIM(dp)),data);
		n_this_line = 0;
		return;
	}

	if( dim > 0 ){
		inc=(ELEMENT_SIZE(dp)*OBJ_MACH_INC(dp,dim));
#ifdef QUIP_DEBUG
if( debug & debug_data ){
snprintf(ERROR_STRING,LLEN,"pntdim: dim=%d, n=%d, inc=%d",dim,OBJ_MACH_DIM(dp,dim),
inc);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */

		for(i=0;i<OBJ_MACH_DIM(dp,dim);i++){
			// have to cast i to prevent inc from being cast to unsigned!!!
			pnt_dim(fp,dp,data+((incr_t)i)*inc,dim-1);
		}
	} else {
		pnt_one(fp,dp,data);
	}
//	// For postscript, don't worry about dimensions, just print a fixed number per line...
//	if( ascii_fmt_code == FMT_POSTSCRIPT ){
//		if( n_this_line >= 32 ){
//			fprintf(fp,"\n");
//			n_this_line=0;
//		}
//	} else {
		if( dim == ret_dim ){
			fprintf(fp,"\n");
			n_this_line = 0;
		}
//	}
}

/* This is a speeded-up version for floats - do we need it? */

#define sp_pntvec( dp, fp ) _sp_pntvec( QSP_ARG  dp, fp )

static void _sp_pntvec( QSP_ARG_DECL  Data_Obj *dp, FILE *fp )
{
	float *base, *fbase, *rbase, *pbase;
	dimension_t i3,i2,i1,i0;
	const char *ffmtstr;

	ffmtstr = padded_flt_fmt_str;

	base = (float *) OBJ_DATA_PTR(dp);

	for(i3=0;i3<OBJ_FRAMES(dp);i3++){
	    fbase=base+i3*OBJ_MACH_INC(dp,3);
	    for(i2=0;i2<OBJ_ROWS(dp);i2++){
		rbase=fbase+i2*OBJ_MACH_INC(dp,2);
		for(i1=0;i1<OBJ_COLS(dp);i1++){
		    pbase=rbase+i1*OBJ_MACH_INC(dp,1);
		    for(i0=0;i0<OBJ_MACH_DIM(dp,0);i0++){
			fprintf(fp," ");
			fprintf(fp,ffmtstr,*(pbase+i0*OBJ_MACH_INC(dp,0)));
			n_this_line++;
		        if( ret_dim == 0 && OBJ_MACH_DIM(dp,0) > dobj_max_per_line ){
				fprintf(fp,"\n");
				n_this_line = 0;
			}
		    }
		    if( ret_dim == 0 && OBJ_MACH_DIM(dp,0) <= dobj_max_per_line ){
		    	fprintf(fp,"\n");
			n_this_line = 0;
		    }
		}
		if( ret_dim > 0 ){
			fprintf(fp,"\n");
			n_this_line = 0;
		}
	    }
	}
}


#ifdef NOT_USED
void set_min_field_width(int fw)
{
	min_field_width=fw;
	UPDATE_PADDED_FLOAT_FORMAT_STRING
}
#endif /* NOT_USED */

void _set_display_precision(QSP_ARG_DECL  int digits)
{
	display_precision=digits;
	UPDATE_PADDED_FLOAT_FORMAT_STRING
}

#define MAX_BITS_PER_LINE 32

#define display_bitmap(dp, fp) _display_bitmap(QSP_ARG  dp, fp)

static void _display_bitmap(QSP_ARG_DECL  Data_Obj *dp, FILE *fp)
{
	int i,j,k,l,m;
	bitmap_word *bwp,val;
	bitnum_t which_bit;
	bitnum_t bit_index, word_offset;
	int bits_this_line;

	bwp = (bitmap_word *)OBJ_DATA_PTR(dp);

	bits_this_line=0;
	for(i=0;i<OBJ_SEQS(dp);i++){
		for(j=0;j<OBJ_FRAMES(dp);j++){
			for(k=0;k<OBJ_ROWS(dp);k++){
				for(l=0;l<OBJ_COLS(dp);l++){
					for(m=0;m<OBJ_COMPS(dp);m++){
						which_bit = OBJ_BIT0(dp)
							+ m * OBJ_TYPE_INC(dp,0)
							+ l * OBJ_TYPE_INC(dp,1)
							+ k * OBJ_TYPE_INC(dp,2)
							+ j * OBJ_TYPE_INC(dp,3)
							+ i * OBJ_TYPE_INC(dp,4);
						bit_index = which_bit % BITS_PER_BITMAP_WORD;
						word_offset = which_bit/BITS_PER_BITMAP_WORD;
						val = *(bwp + word_offset) & NUMBERED_BIT(bit_index); 
						if( bits_this_line >= MAX_BITS_PER_LINE ){
							bits_this_line=0;
							fputc('\n',fp);
							//prt_msg("");
						} else if( bits_this_line > 0 )
							fputc(' ',fp);
							//prt_msg_frag(" ");
						//prt_msg_frag(val?"1":"0");
						fputc(val?'1':'0',fp);
						bits_this_line++;
					}
				}
				if( bits_this_line > 0 ){
					bits_this_line=0;
					fputc('\n',fp);
					//prt_msg("");
				}
			}
		}
	}
}

void _pntvec(QSP_ARG_DECL  Data_Obj *dp,FILE *fp)			/**/
{
//	const char *save_ifmt;
//	const char *save_ffmt;
	/* first let's figure out when to print returns */
	/* ret_dim == 0 means a return is printed after every pixel */
	/* ret_dim == 1 means a return is printed after every row */

#ifdef THREAD_SAFE_QUERY
	// This is done for the main thread in dataobj_init()
	INSURE_QS_DOBJ_ASCII_INFO(THIS_QSP)
#endif // THREAD_SAFE_QUERY

// where is ret_dim declared?  part of qsp?
	if( OBJ_MACH_DIM(dp,0) == 1 && OBJ_COLS(dp) <= dobj_max_per_line)
		ret_dim=1;
	else ret_dim=0;

	/* We pad with spaces only if we are printing more than one element */

	/* BUG should set format based on desired radix !!! */

	// // BUG should only do this at init time, and when changed...
	// snprintf(padded_flt_fmt_str,"%%%d.%dg",min_field_width,display_precision);

	if( OBJ_MACH_PREC(dp) == PREC_SP ){
		sp_pntvec(dp,fp);
	} else if( OBJ_PREC(dp) == PREC_BIT )
		display_bitmap(dp,fp);
	else {
		/* the call to pnt_dim() was commented out,
		 * and the following lines calling dobj_iterate
		 * were there instead - why?
		 * Perhaps pnt_dim does not handle non-contig data
		 * correctly???  Or perhaps it was deemed better
		 * to use a general mechanism and dump the
		 * special purpose code???
		 *
		 * For now we reinstate pnt_dim to regain column
		 * format printing!
		 */

		n_this_line=0;
		pnt_dim(fp,dp,(u_char *)OBJ_DATA_PTR(dp),N_DIMENSIONS-1);
		if( n_this_line > 0 ){	// postscript only???
			fprintf(fp,"\n");
			n_this_line=0;
		}
	}
	fflush(fp);

//	ffmtstr = save_ffmt ;
//	ifmtstr = save_ifmt ;
} // pntvec

#define shp_trace(name,shpp) _shp_trace(QSP_ARG  name,shpp)

static void _shp_trace(QSP_ARG_DECL  const char *name,Shape_Info *shpp)
{
	snprintf(ERROR_STRING,LLEN,
		"%s: mindim = %d,  maxdim = %d",
		name, SHP_MINDIM(shpp), SHP_MAXDIM(shpp));
	advise(ERROR_STRING);

	snprintf(ERROR_STRING,LLEN,
		"%s dim:  %u %u %u %u %u",
		name,
		SHP_TYPE_DIM(shpp,0),
		SHP_TYPE_DIM(shpp,1),
		SHP_TYPE_DIM(shpp,2),
		SHP_TYPE_DIM(shpp,3),
		SHP_TYPE_DIM(shpp,4));
	advise(ERROR_STRING);
}

void _dptrace( QSP_ARG_DECL  Data_Obj *dp )
{
	shp_trace(OBJ_NAME( dp) ,OBJ_SHAPE(dp) );

	snprintf(ERROR_STRING,LLEN,
		// why %u format when increment can be negative???
		"%s inc:  %u %u %u %u %u  (%u %u %u %u %u)",
		OBJ_NAME( dp) ,
		OBJ_TYPE_INC(dp,0),
		OBJ_TYPE_INC(dp,1),
		OBJ_TYPE_INC(dp,2),
		OBJ_TYPE_INC(dp,3),
		OBJ_TYPE_INC(dp,4),
		OBJ_MACH_INC(dp,0),
		OBJ_MACH_INC(dp,1),
		OBJ_MACH_INC(dp,2),
		OBJ_MACH_INC(dp,3),
		OBJ_MACH_INC(dp,4));
	advise(ERROR_STRING);
}

/* read an array of strings... */

#define get_strings(dp,data,dim) _get_strings(QSP_ARG  dp,data,dim)

static int _get_strings(QSP_ARG_DECL  Data_Obj *dp,char *data,int dim)
{
	int status=0;

	if( dim == OBJ_MINDIM(dp) ){
		return( get_a_string(dp,data,dim) );
	} else {
		dimension_t i;
		long offset;

		offset = ELEMENT_SIZE( dp);
		offset *= OBJ_MACH_INC(dp,dim);
		for(i=0;i<OBJ_MACH_DIM(dp,dim);i++){
			status = get_strings(dp,data+i*offset,dim-1);
			if( status < 0 ) return status;
		}
	}
	return status;
}

#define get_bits(dp, ptr, dim, bit0 ) _get_bits(QSP_ARG  dp, ptr, dim, bit0 )

static int _get_bits(QSP_ARG_DECL  Data_Obj *dp, void *ptr, int dim, int bit0 )
{
	dimension_t i;
	long offset;

	if( dim < 0 ){
		return( get_next_bit(ptr,bit0) );
	}

	offset = OBJ_TYPE_INC(dp,dim);
	for(i=0;i<OBJ_TYPE_DIM(dp,dim);i++){
		int status;
		status = get_bits(dp,ptr,dim-1,bit0+(int)(i*offset));
		if( status < 0 ) return status;
	}
	return 0;
}

#define get_sheets(dp,data,dim) _get_sheets(QSP_ARG  dp,data,dim)

static int _get_sheets(QSP_ARG_DECL  Data_Obj *dp,unsigned char *data,int dim)
{
	dimension_t i;
	long offset;
	int status=0;

	assert( ! IS_BITMAP(dp) );

	if( dim < 0 ){	/* get a component */
		return( get_next_element(dp,data) );
	}
	
	offset = ELEMENT_SIZE(dp);
	offset *= OBJ_MACH_INC(dp,dim);
	for(i=0;i<OBJ_MACH_DIM(dp,dim);i++){
		// if data is NULL, that means we aren't really writing...
		status = get_sheets(dp,
			data == NULL ? data : data+i*offset,
			dim-1);
		if( status < 0 ) return status;
	}
	return status;
}

/*
 * We check qlevel, so that if the file is bigger than
 * the data object, the file will be closed so that
 * the succeeding data will not be read as a command.
 * This logic cannot deal with too *little* data in the
 * file, that has to be taken care of in read_obj()
 */

#define read_object_with_check(dp, expect_exact_count, filename) _read_object_with_check(QSP_ARG  dp, expect_exact_count, filename)

static void _read_object_with_check(QSP_ARG_DECL  Data_Obj *dp, int expect_exact_count, const char *filename)
{
	int level;

	level = QLEVEL;

	read_obj(dp);

	if( level == QLEVEL ){
		if( expect_exact_count ){
			snprintf(ERROR_STRING,LLEN,
				"Needed %d values for object %s, file %s has more!?",
				OBJ_N_MACH_ELTS(dp),OBJ_NAME( dp) ,filename);
			warn(ERROR_STRING);
		}
		pop_file();
	}
}

#ifdef HAVE_POPEN
void _read_ascii_data_from_pipe(QSP_ARG_DECL  Data_Obj *dp, Pipe *pp, const char *filename, int expect_exact_count)
{
	const char *orig_filename;

	assert(!strncmp(filename,PIPE_PREFIX_STRING,strlen(PIPE_PREFIX_STRING)));
	orig_filename = savestr(filename);	/* with input formats, we might lose it */
	redir_from_pipe(pp, orig_filename);	// BUG?  Do we really need to store a copy of the filename?
	read_object_with_check(dp, expect_exact_count, filename);
	rls_str( orig_filename);
}
#endif // ! HAVE_POPEN

void _read_ascii_data_from_file(QSP_ARG_DECL  Data_Obj *dp, FILE *fp, const char *filename, int expect_exact_count)
{
	const char *orig_filename;

	// We allow a filename to begin with the pipe prefix???
	//assert(strncmp(filename,PIPE_PREFIX_STRING,strlen(PIPE_PREFIX_STRING)));
	orig_filename = savestr(filename);	/* with input formats, we might lose it */
	redir(fp, orig_filename);
	read_object_with_check(dp, expect_exact_count, filename);
	rls_str( orig_filename);
}

void _read_obj(QSP_ARG_DECL   Data_Obj *dp)
{
	void *data_ptr;

	ASCII_LEVEL = QLEVEL;
	dobj_n_gotten = 0;

	if( ! OBJ_IS_RAM(dp) ){
		snprintf(ERROR_STRING,LLEN,
	"read_obj:  object %s must be in RAM for assignment!?",
			OBJ_NAME(dp));
		warn(ERROR_STRING);
		data_ptr = NULL;
	} else {
		data_ptr = OBJ_DATA_PTR(dp);
	}

	if( HAS_FORMAT_LIST ){
		CURRENT_FORMAT_NODE = FIRST_INPUT_FORMAT_NODE;
	}

	if( dp != ascii_data_dp ){
		/* We store the target object so we can print its name if we need to... */
		ascii_data_dp = dp;		/* if this is global, then this is not thread-safe!? */
		ascii_warned=0;
	}

	if( OBJ_PREC(dp) == PREC_CHAR || OBJ_PREC(dp) == PREC_STR ){
		if( get_strings(dp,(char *)data_ptr,N_DIMENSIONS-1) < 0 ){
			snprintf(ERROR_STRING,LLEN,"error reading strings for object %s",OBJ_NAME( dp) );
			warn(ERROR_STRING);
		}
	} else if( IS_BITMAP(dp) ){
		if( get_bits(dp,data_ptr,N_DIMENSIONS-1,OBJ_BIT0(dp)) < 0){
			snprintf(ERROR_STRING,LLEN,"expected %d bits for bitmap object %s",
				OBJ_N_TYPE_ELTS(dp),OBJ_NAME( dp) );
			warn(ERROR_STRING);
		}
	} else {	// normal object
		if( get_sheets(dp,(u_char *)data_ptr,N_DIMENSIONS-1) < 0 ){
			snprintf(ERROR_STRING,LLEN,"expected %d elements for object %s",
				OBJ_N_MACH_ELTS(dp),OBJ_NAME( dp) );
			warn(ERROR_STRING);
		}
	}

	// If we are reading formatted input, there may be some irrelavant fields
	// that could trigger a "too many values" warning...
	if( HAS_FORMAT_LIST ){
		if( CURRENT_FORMAT_NODE != FIRST_INPUT_FORMAT_NODE ){
			consume_format_line(OBJ_PREC_PTR(dp));
		}
	}

	// Input file will not be popped if reading from stdin,
	// OR flag set...
} // read_obj

void _set_integer_print_fmt(QSP_ARG_DECL  Integer_Output_Fmt *iof_p )
{
	curr_output_int_fmt_p = iof_p;
}

void _set_max_per_line(QSP_ARG_DECL  int n )
{
	if( n < 1 )
		warn("max_per_line must be positive");
	else if( n > ENFORCED_MAX_PER_LINE ){
		snprintf(ERROR_STRING,LLEN,"Requested max_per_line (%d) exceeds hard-coded maximum (%d)",
				n,ENFORCED_MAX_PER_LINE);
		warn(ERROR_STRING);
	} else
		dobj_max_per_line = n;
}

static void int_format_release(Input_Format_Spec *fmt_p) { default_format_release(fmt_p); }
static void float_format_release(Input_Format_Spec *fmt_p) { default_format_release(fmt_p); }
static void string_format_release(Input_Format_Spec *fmt_p) { default_format_release(fmt_p); }

#define INIT_FORMAT_TYPE(code,fmt_name,stem)			\
								\
	{							\
	struct input_format_type *ft_p;				\
	assert( code >= 0 && code < N_INPUT_FORMAT_TYPES );	\
	ft_p = &input_format_type_tbl[code];			\
	ft_p->name = #fmt_name;					\
	ft_p->type_code = code;					\
	ft_p->display_format = display_##stem;			\
	ft_p->release = stem##_release;				\
	ft_p->consume = stem##_consume;				\
	ft_p->read_long = stem##_read_long;			\
	ft_p->read_double = stem##_read_double;			\
	ft_p->read_string = stem##_read_string;			\
	}


static void init_format_type_tbl(void)
{
	INIT_FORMAT_TYPE(IN_FMT_INT,integer,int_format)
	INIT_FORMAT_TYPE(IN_FMT_FLT,float,float_format)
	INIT_FORMAT_TYPE(IN_FMT_STR,string,string_format)
	INIT_FORMAT_TYPE(IN_FMT_LIT,literal,literal_format)
}

