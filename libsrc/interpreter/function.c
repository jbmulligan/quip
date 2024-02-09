#include <string.h>
#include <ctype.h>
#include "quip_config.h"
#include "quip_prot.h"
#include "function.h"
#include "func_helper.h"
#include "variable.h"
#include "item_type.h"
#include "warn.h"
#include "rn.h"
#include "data_obj.h"
#include "item_prot.h"
#include "veclib/vecgen.h"
#include "sizable.h"
#include "fileck.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_MATH_H
#include <math.h>
#endif /* HAVE_MATH_H */

#include <time.h>

//#ifdef HAVE_GSL
//#include "gsl/gsl_sf_gamma.h"
//#endif /* HAVE_GSL */

//#import "nexpr_func.h"


//void addFunctionWithCode(Quip_Function *f,int code);
//const char *function_name(Quip_Function *f);

static Item_Type *function_itp=NULL;

ITEM_INIT_FUNC(Quip_Function,function,0)
ITEM_NEW_FUNC(Quip_Function,function)
ITEM_CHECK_FUNC(Quip_Function,function)

#define DECLARE_CHAR_FUNC( fname )					\
									\
static int c_##fname( char c )						\
{									\
	if( fname(c) ) return 1;					\
	else return 0;							\
}

DECLARE_CHAR_FUNC( isprint )
DECLARE_CHAR_FUNC( islower )
DECLARE_CHAR_FUNC( isupper )
DECLARE_CHAR_FUNC( isalpha )
DECLARE_CHAR_FUNC( isalnum )
DECLARE_CHAR_FUNC( isdigit )
DECLARE_CHAR_FUNC( isspace )
DECLARE_CHAR_FUNC( iscntrl )
DECLARE_CHAR_FUNC( isblank )

/*
static int c_tolower( char c ) { return tolower(c); }
static int c_toupper( char c ) { return toupper(c); }
*/

#define DECLARE_STRINGMAP_FUNCTION( funcname, test_macro, map_macro )	\
									\
static const char * funcname(QSP_ARG_DECL  const char *s )				\
{									\
	char *t, *p;							\
									\
	t = getbuf(strlen(s)+1);					\
	p=t;								\
	while( *s ){							\
		if( test_macro(*s) )					\
			*p = (char) map_macro(*s);			\
		else							\
			*p = *s;					\
		p++;							\
		s++;							\
	}								\
	*p = 0;								\
	return t;							\
}

DECLARE_STRINGMAP_FUNCTION(s_tolower,isupper,tolower)
DECLARE_STRINGMAP_FUNCTION(s_toupper,islower,toupper)

static const char *_format_time(QSP_ARG_DECL  long secs)
{
	char *s, *t;
	struct tm tm1, *tm_p;

	s=getbuf(64);
	tm_p = gmtime_r(&secs,&tm1);
	if( tm_p != &tm1 ){
		strcpy(s,"gmtime_r error!?\n");
	} else {
		t = asctime_r( &tm1, s );
		if( t != s ){
			strcpy(s,"asctime_r error!?\n");
		}
	}
	// remove trailing newline...
	assert( s[ strlen(s)-1 ] == '\n' );
	s[ strlen(s)-1 ] = 0;
	return s;
}

// When are these allocated strings freed???

static const char *_ascii_to_string(QSP_ARG_DECL  long value)
{
	char *s;
	s=getbuf(2);
	s[0] = value & 0xff;
	s[1] = 0;
	return s;
}

static double modtimefunc(QSP_ARG_DECL  const char *s)
{
#ifdef HAVE_SYS_STAT_H
	struct stat statb;

	if( stat(s,&statb) < 0 ){
		tell_sys_error(s);
		return 0.0;
	}
	return (double) statb.st_mtime;
#else /* ! HAVE_SYS_STAT_H */

	return 0.0;

#endif /* ! HAVE_SYS_STAT_H */
}

#ifndef BUILD_FOR_IOS
int is_portrait(void)
{
	NADVISE("is_portrait() called in non-tablet environment!?");

	return 0;
}
#endif // ! BUILD_FOR_IOS

static double is_landscape_dbl(void)
{
	if( is_portrait() )
		return 0.0;
	else
		return 1.0;
}

static double is_portrait_dbl(void)
{
	return (double) is_portrait();
}

static double rn_uni(double arg)		/* arg is not used... */
{
	double d;
	_rninit(SGL_DEFAULT_QSP_ARG);
#ifdef HAVE_DRAND48
	d=drand48();
#else
	_warn(DEFAULT_QSP_ARG  "rn_uni:  no drand48!?");
	d=1.0;
#endif // ! HAVE_DRAND48
	return d;
}

static double rn_number(double dlimit)
{
	double dret;
	int ilimit, iret;

	ilimit=(int)dlimit;
	iret=(int)rn(ilimit);
	dret=iret;
	return dret;
}

static double dstrstr(const char *s1,const char *s2)
{
	char *s;
	s=strstr(s1,s2);
	if( s == NULL ) return -1;
	else return (double)(s - s1);
}

static double dstrcmp(const char *s1,const char *s2)
{
	double d;
	d=strcmp(s1,s2);
	return d;
}

static double dstrncmp(const char *s1,const char *s2,int n)
{
	double d;
	d=strncmp(s1,s2,n);
	return d;
}

static double dstrlen(QSP_ARG_DECL  const char *s1)
{
	double d;
	d=strlen(s1);
	return d;
}

static double ascii_val(QSP_ARG_DECL  const char *s)
{
	unsigned long l;
	if( (l=strlen(s)) == 0 ){
		warn("ascii() passed an empty string!?");
		return 0;
	}
	if( l > 1 ){
		snprintf(ERROR_STRING,LLEN,"ascii() passed a string of length %lu, returning value of 1st char.",l);
		warn(ERROR_STRING);
	}
	return (double) s[0];
}

static double dvarexists(QSP_ARG_DECL  const char *s)
{
	Variable *vp;

	vp = var_of(s);
	if( vp == NULL ) return 0.0;
	return 1.0;
}

static double dmacroexists(QSP_ARG_DECL  const char *s)
{
	Macro *mp;

	mp = macro_of(s);
	if( mp == NULL ) return 0.0;
	return 1.0;
}

static double dexists(QSP_ARG_DECL  const char *fname)
{
	if( path_exists(fname) )
		return 1.0;
	else	return 0.0;
}

static double disdir(QSP_ARG_DECL  const char *path)
{
	if( directory_exists(path) )
		return 1.0;
	else	return 0.0;
}

static double disreg(QSP_ARG_DECL  const char *path)
{
	if( regfile_exists(path) )
		return 1.0;
	else	return 0.0;
}

static void set_day_of_week(struct tm *tm_p, const char *date_string)
{
	int d;
	switch(date_string[0]){
		case 'S':
			if( date_string[1] == 'u' ) d=0;
			else if( date_string[1] == 'a' ) d=6;
			else {
				NWARN("bad day of week!?");
				return;
			}
			break;
		case 'M':	d=1; break;
		case 'T':
			if( date_string[1] == 'u' ) d=2;
			else if( date_string[1] == 'h' ) d=4;
			else {
				NWARN("bad day of week!?");
				return;
			}
			break;
		case 'W':	d=3; break;
		case 'F':	d=5; break;
		default:
			NWARN("bad day of week!?");
			return;
	}
	tm_p->tm_wday = d;
}

static void set_month(struct tm *tm_p, const char *date_string)
{
	int m;
	switch(date_string[0]){
		case 'J':
			if( date_string[1] == 'a' ) m=0;
			else if( date_string[1] == 'u' ){
				if( date_string[2] == 'n' ) m=5;
				else if( date_string[2] == 'l' ) m=6;
				else {
					NWARN("bad month!?");
					return;
				}
			} else {
				NWARN("bad month!?");
				return;
			}
			break;
		case 'F':	m=1; break;
		case 'M':
			if( date_string[2] == 'r' ) m=2;
			else if( date_string[2] == 'y' ) m=4;
			else {
				NWARN("bad month!?");
				return;
			}
			break;
		case 'A':
			if( date_string[1] == 'p' ) m=3;
			else if( date_string[1] == 'u' ) m=7;
			else {
				NWARN("bad month!?");
				return;
			}
			break;
		case 'S':	m=8; break;
		case 'O':	m=9; break;
		case 'N':	m=10; break;
		case 'D':	m=11; break;
		default:
			NWARN("bad month!?");
			return;
	}
	tm_p->tm_mon = m;
}

static int two_digit_number(const char *s)
{
	int n;
	n = s[0]-'0';
	n *= 10;
	n += s[1]-'0';
	return n;
}

static void set_day_of_month(struct tm *tm_p, const char *date_string)
{
	int d;
	d = two_digit_number(date_string);
	if( d > 31 ){
		NWARN("Bad day of month!?");
		return;
	}
	tm_p->tm_mday = d;
}

static void set_hour(struct tm *tm_p, const char *date_string)
{
	int h;
	h = two_digit_number(date_string);
	if( h > 23 ){
		NWARN("Bad hour!?");
		return;
	}
	tm_p->tm_hour = h;
}

static void set_minute(struct tm *tm_p, const char *date_string)
{
	int m;
	m = two_digit_number(date_string);
	if( m > 59 ){
		NWARN("Bad hour!?");
		return;
	}
	tm_p->tm_min = m;
}

static void set_second(struct tm *tm_p, const char *date_string)
{
	int s;
	s = two_digit_number(date_string);
	if( s > 59 ){
		NWARN("Bad second!?");
		return;
	}
	tm_p->tm_sec = s;
}

static void set_year(struct tm *tm_p, const char *date_string)
{
	int y;
	y = two_digit_number(date_string);
	y *= 100;
	y += two_digit_number(&date_string[2]);
	if( y < 1970 || y > 2038 ){
		snprintf(DEFAULT_ERROR_STRING,LLEN,"Bad year %d!? (date_string = \"%s\"",y,date_string);
		NWARN(DEFAULT_ERROR_STRING);
		return;
	}
	tm_p->tm_year = y - 1900;
}

// unix_time() expects a 24 character string of this form:
// Thu Nov 24 18:22:48 1986

#ifdef LIKE_A_COMMENT
struct tm {
        int     tm_sec;         /* seconds after the minute [0-60] */
        int     tm_min;         /* minutes after the hour [0-59] */
        int     tm_hour;        /* hours since midnight [0-23] */
        int     tm_mday;        /* day of the month [1-31] */
        int     tm_mon;         /* months since January [0-11] */
        int     tm_year;        /* years since 1900 */
        int     tm_wday;        /* days since Sunday [0-6] */
        int     tm_yday;        /* days since January 1 [0-365] */
        int     tm_isdst;       /* Daylight Savings Time flag */
        long    tm_gmtoff;      /* offset from CUT in seconds */
        char    *tm_zone;       /* timezone abbreviation */
};
#endif // LIKE_A_COMMENT


static double unix_time(QSP_ARG_DECL  const char *date_string)
{
	struct tm tm1;
	time_t t;

	set_day_of_week(&tm1,date_string);
	set_month(&tm1,&date_string[4]);
	set_day_of_month(&tm1,&date_string[8]);
	set_hour(&tm1,&date_string[11]);
	set_minute(&tm1,&date_string[14]);
	set_second(&tm1,&date_string[17]);
	set_year(&tm1,&date_string[20]);

	tm1.tm_yday = (-1);
	tm1.tm_isdst = 0;
	tm1.tm_gmtoff = 0;
	tm1.tm_zone = NULL;

	//t = mktime(&tm1);	// mktime uses local timezone settings...
	t = timegm(&tm1);	// UTC

	return (double) t;
}

static double bitsum(double num)
{
	long l;
	long bit;
	int i,sum;

	sum=0;
	l=(long)num;
	bit=1;
	for(i=0;i<32;i++){
		if( l & bit ) sum++;
		bit <<= 1;
	}
	num = sum;
	return num;
}

static double maxfunc(double a1,double a2)
{
	if( a1 >= a2 ) return a1;
	else return a2;
}

static double minfunc(double a1,double a2)
{
	if( a1 <= a2 ) return a1;
	else return a2;
}

static Item_Class *sizable_icp=NULL;
static Item_Class *interlaceable_icp=NULL;
static Item_Class *positionable_icp=NULL;
static Item_Class *tsable_icp=NULL;
static Item_Class *camera_icp=NULL;
static Item_Class *subscriptable_icp=NULL;

#define DECLARE_CLASS_INITIALIZER(type_stem)				\
									\
static void init_##type_stem##_class(SINGLE_QSP_ARG_DECL)		\
{									\
	if( type_stem##_icp != NULL ){				\
		snprintf(ERROR_STRING,LLEN,					\
	"Redundant call to %s class initializer",#type_stem);		\
		warn(ERROR_STRING);					\
		return;							\
	}								\
	type_stem##_icp = new_item_class(#type_stem );		\
}

DECLARE_CLASS_INITIALIZER(sizable)
DECLARE_CLASS_INITIALIZER(tsable)
DECLARE_CLASS_INITIALIZER(subscriptable)
DECLARE_CLASS_INITIALIZER(interlaceable)
DECLARE_CLASS_INITIALIZER(positionable)
DECLARE_CLASS_INITIALIZER(camera)


// We would like to be able to use named IOS items here too...
// how can we handle that, when one our our Items can't point to an IOS
// object?  This is really ugly, but we could make up another item
// type that provides regular item name lookup, and then handles
// the size functions...

#define DECLARE_ADD_FUNCTION(type_stem,func_type)			\
									\
void _add_##type_stem(QSP_ARG_DECL  Item_Type *itp,			\
		func_type *func_str_ptr,				\
		Item *(*lookup)(QSP_ARG_DECL  const char *))		\
{									\
	if( type_stem##_icp == NULL )					\
		init_##type_stem##_class(SINGLE_QSP_ARG);		\
	add_items_to_class(type_stem##_icp,itp,func_str_ptr,lookup);	\
}

DECLARE_ADD_FUNCTION(sizable,Size_Functions)
DECLARE_ADD_FUNCTION(tsable,Timestamp_Functions)
DECLARE_ADD_FUNCTION(interlaceable,Interlace_Functions)
DECLARE_ADD_FUNCTION(positionable,Position_Functions)
// For the most part, the subscript functions aren't really functions with names...
// But originally, they were packed into the size function struct, so here they
// are...
DECLARE_ADD_FUNCTION(subscriptable,Subscript_Functions)
DECLARE_ADD_FUNCTION(camera,Camera_Functions)

#define DECLARE_FIND_FUNCTION(type_stem)				\
									\
Item *_find_##type_stem(QSP_ARG_DECL  const char *name )		\
{									\
	if( type_stem##_icp == NULL )					\
		init_##type_stem##_class(SINGLE_QSP_ARG);		\
									\
	return get_member(type_stem##_icp,name);			\
}

DECLARE_FIND_FUNCTION(sizable)
DECLARE_FIND_FUNCTION(tsable)
DECLARE_FIND_FUNCTION(interlaceable)
DECLARE_FIND_FUNCTION(positionable)
DECLARE_FIND_FUNCTION(subscriptable)


#ifdef BUILD_FOR_OBJC
#define OBJC_CHECK(type_stem)						\
	if( ip == NULL )						\
		ip = (__bridge Item *) check_ios_##type_stem(QSP_ARG  name);
#else // ! BUILD_FOR_OBJC
#define OBJC_CHECK(type_stem)
#endif // ! BUILD_FOR_OBJC

#define DECLARE_CHECK_FUNC(type_stem)					\
Item *check_##type_stem(QSP_ARG_DECL  const char *name )		\
{									\
	Item *ip;							\
	if( type_stem##_icp == NULL )				\
		init_##type_stem##_class(SINGLE_QSP_ARG);		\
									\
	ip = check_member(type_stem##_icp, name );		\
	OBJC_CHECK(type_stem)						\
	return ip;							\
}

DECLARE_CHECK_FUNC(sizable)

//DECLARE_CHECK_FUNC(tsable)

#define DECLARE_GETFUNCS_FUNC(type_stem,func_type)			\
									\
func_type *_get_##type_stem##_functions(QSP_ARG_DECL  Item *ip)		\
{									\
	Member_Info *mip;						\
	if( type_stem##_icp == NULL )					\
		init_##type_stem##_class(SINGLE_QSP_ARG);		\
	mip = get_member_info(type_stem##_icp,ip->item_name);		\
	/*MEMBER_CAUTIOUS_CHECK(type_stem)*/				\
	assert( mip != NULL );						\
	return (func_type *) mip->mi_data;				\
}

DECLARE_GETFUNCS_FUNC(sizable,Size_Functions)		// get_sizable_functions
DECLARE_GETFUNCS_FUNC(tsable,Timestamp_Functions)	// get_tsable_functions
DECLARE_GETFUNCS_FUNC(interlaceable,Interlace_Functions)	// get_interlaceable_functions
DECLARE_GETFUNCS_FUNC(positionable,Position_Functions)	// get_positionable_functions
DECLARE_GETFUNCS_FUNC(subscriptable,Subscript_Functions)	// get_subscriptable_functions

#ifndef BUILD_FOR_OBJC

// If we are building for IOS, use the version in ios_sizable, which handles
// both types of object...
//
// The initial approach didn't work, because we can't cast void * to IOS_Item...
// A cleaner approach would be for items to carry around their item type ptr,
// and for the item type struct to have a pointer to the size functions
// (when they exist).
// That approach burns a bit more memory, but probably insignificant?

const char *_get_object_prec_string(QSP_ARG_DECL  Item *ip )	// non-iOS
{
	Size_Functions *sfp;

	if( ip == NULL ) return "u_byte";

	sfp = get_sizable_functions(ip);
	assert( sfp != NULL );

	return (*sfp->prec_func)(QSP_ARG  ip);
}

double _get_object_size(QSP_ARG_DECL  Item *ip,int d_index)
{
	Size_Functions *sfp;

	if( ip == NULL ) return 0.0;

	sfp = get_sizable_functions(ip);
	assert( sfp != NULL );

	return (*sfp->sz_func)(QSP_ARG  ip,d_index);
}

#endif /* ! BUILD_FOR_OBJC */

static double get_posn(QSP_ARG_DECL  Item *ip, int index)
{
	Position_Functions *pfp;
	if( ip == NULL ) return 0.0;
	pfp = get_positionable_functions(ip);
	assert( pfp != NULL );
	assert( index >= 0 && index <= 1 );

	return (*pfp->posn_func)(QSP_ARG  ip,index);
}

static double get_interlace_flag(QSP_ARG_DECL  Item *ip)
{
	Interlace_Functions *ifp;

	if( ip == NULL ) return 0.0;

	ifp = get_interlaceable_functions(ip);
	assert( ifp != NULL );
	assert( ifp->ilace_func != NULL );

	return (*ifp->ilace_func)(QSP_ARG  ip);
}

static double get_timestamp(QSP_ARG_DECL  Item *ip,int func_index,dimension_t frame)
{
	Timestamp_Functions *tsfp;
	Member_Info *mip;
	double d;

	if( ip == NULL ) return 0.0;

	if( tsable_icp == NULL )
		init_tsable_class(SINGLE_QSP_ARG);

	mip = get_member_info(tsable_icp,ip->item_name);
	assert( mip != NULL );

	tsfp = (Timestamp_Functions *) mip->mi_data;

	d = (*(tsfp->timestamp_func[func_index]))(QSP_ARG  ip,frame);
	return d;
}

static double is_capturing(QSP_ARG_DECL  Item *ip)
{
	Camera_Functions *cfp;
	Member_Info *mip;
	double d;

	if( ip == NULL ) return(0.0);

	if( camera_icp == NULL )
		init_camera_class(SINGLE_QSP_ARG);

	mip = get_member_info(camera_icp,ip->item_name);
	assert( mip != NULL );

	cfp = (Camera_Functions *) mip->mi_data;

	d = (*(cfp->is_capturing_func))(QSP_ARG  ip);
	return( d );
}

Item *sub_sizable(QSP_ARG_DECL  Item *ip,index_t index)
{
	Subscript_Functions *sfp;
	Member_Info *mip;

	/* currently data objects are the only sizables
		which can be subscripted */

	if( ip == NULL ) return ip;

	if( subscriptable_icp == NULL )
		init_subscriptable_class(SINGLE_QSP_ARG);

	mip = get_member_info(subscriptable_icp,ip->item_name);
	assert( mip != NULL );

	sfp = (Subscript_Functions *) mip->mi_data;

	if( sfp->subscript == NULL ){
		snprintf(ERROR_STRING,LLEN,"Can't subscript object %s!?",
			ip->item_name);
		warn(ERROR_STRING);
		return NULL;
	}

	return (*sfp->subscript)(QSP_ARG  ip,index);
}

Item *csub_sizable(QSP_ARG_DECL  Item *ip,index_t index)
{
	Subscript_Functions *sfp;
	Member_Info *mip;

	/* currently data objects are the only sizables
		which can be subscripted */

	if( ip == NULL ) return ip;

	if( subscriptable_icp == NULL )
		init_subscriptable_class(SINGLE_QSP_ARG);

	mip = get_member_info(subscriptable_icp,ip->item_name);
	assert( mip != NULL );

	sfp = (Subscript_Functions *) mip->mi_data;

	if( sfp->csubscript == NULL ){
		snprintf(ERROR_STRING,LLEN,"Can't subscript object %s",
			ip->item_name);
		warn(ERROR_STRING);
		return NULL;
	}

	return (*sfp->csubscript)(QSP_ARG  ip,index);
}

// We want to be able to pass these functions any type
// of object, but we can't cast from void * to IOS_Object...

static double _ilfunc(QSP_ARG_DECL  Item *ip)
{ return get_interlace_flag(QSP_ARG  ip); }

static double _x_func(QSP_ARG_DECL  Item *ip)
{ return get_posn(QSP_ARG  ip,0); }

static double _y_func(QSP_ARG_DECL  Item *ip)
{ return get_posn(QSP_ARG  ip,1); }

static double _dpfunc(QSP_ARG_DECL  Item *ip)
{ return get_object_size(ip,0); }

static double _colfunc(QSP_ARG_DECL  Item *ip)
{ return get_object_size(ip,1); }

static double _rowfunc(QSP_ARG_DECL  Item *ip)
{ return get_object_size(ip,2); }

static double _frmfunc(QSP_ARG_DECL  Item *ip)
{ return get_object_size(ip,3); }

static double _seqfunc(QSP_ARG_DECL  Item *ip)
{ return get_object_size(ip,4); }

static const char *_precfunc(QSP_ARG_DECL  const char *s)
{
	Item *ip;
	ip = _find_sizable( DEFAULT_QSP_ARG  s );
	return get_object_prec_string(ip);
}

static const char *strcat_func(QSP_ARG_DECL  const char *s1, const char *s2 )
{
	char *s;
	s = getbuf( strlen(s1) + strlen(s2) + 1 );
	strcpy(s,s1);
	strcat(s,s2);
	return s;
}

static double _nefunc(QSP_ARG_DECL  Item *ip)
{
	int i;
	double d;

	d=1;
	for(i=0;i<N_DIMENSIONS;i++)
		d *= get_object_size(ip,i);

	return d;
}

#define SIGN(x)		(x<0?-1.0:1.0)
#define SIGNF(x)	(x<0?-1.0f:1.0f)

// This approximation to erfinv comes from wikipedia, which cites:
// Winitzki, Sergei (6 February 2008). "A handy approximation for
// the error function and its inverse" (PDF). Retrieved 2011-10-03.

double erfinv(double x)
{
	double y;
	static double pi=0.0;
	static double a=0.0;

	if( pi == 0.0 ){
		pi = 4*atan(1);
		a = 8 * ( pi - 3 ) / ( 3 * pi * ( 4 - pi ) );
	}

	y = SIGN(x) *
	    sqrt(
	    sqrt( pow( 2/(pi*a) + log(1-x*x)/2, 2 ) - log(1-x*x)/a )
	     - ( 2/(pi*a) + log(1-x*x)/2 )
	     );

	return y;
}

float erfinvf(float x)
{
	float y;
	static float pi=0.0;
	static float a=0.0;

	if( pi == 0.0 ){
		pi = 4*atanf(1);
		a = 8 * ( pi - 3 ) / ( 3 * pi * ( 4 - pi ) );
	}

	y = SIGNF(x) *
	    sqrtf(
	    sqrtf( powf( 2.0f/(pi*a) + logf(1.0f-x*x)/2.0f, 2.0f ) - logf(1.0f-x*x)/a )
	     - ( 2.0f/(pi*a) + logf(1.0f-x*x)/2.0f )
	     );

	return y;
}


static double _captfunc(QSP_ARG_DECL  Item *ip )
{ return( is_capturing(QSP_ARG  ip)); }

static double _secfunc(QSP_ARG_DECL  Item *ip, dimension_t frame )
{ return get_timestamp(QSP_ARG  ip,0,frame); }

static double _msecfunc(QSP_ARG_DECL  Item *ip, dimension_t frame )
{ return get_timestamp(QSP_ARG  ip,1,frame); }

static double _usecfunc(QSP_ARG_DECL  Item *ip, dimension_t frame )
{ return get_timestamp(QSP_ARG  ip,2,frame); }

static double signfunc(double x)
{ if( x > 0 ) return 1.0; else if( x < 0 ) return -1.0; else return 0.0; }

static int isnanfunc(double x)
{ return isnan(x); }

static int isinffunc(double x)
{ return isinf(x); }

static int isnormalfunc(double x)
{ return isnormal(x); }

#ifndef HAVE_ROUND
double round(double arg) { return floor(arg+0.5); }
#endif /* ! HAVE_ROUND */


/*
 * ANAL BUG?  doesn't compile with strict prototyping
 * of the function pointer...  jbm 3/17/95
 */

/* Even though uni() doesn't need an arg, we'd like to be able to
 * give it a vector arg to indicate that we want a vector value,
 * not a scalar value:
 *
 * x = uni();		# same as:  tmp_scalar = uni(); x = tmp_scalar;
 * vs.  x = uni(x)	# every pixel of x set to a different value...
 */

int func_serial=0;

void declare_functions( SINGLE_QSP_ARG_DECL )
{

DECLARE_D0_FUNCTION(	is_portrait,	is_portrait_dbl,		INVALID_VFC,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D0_FUNCTION(	is_landscape,	is_landscape_dbl,		INVALID_VFC,	INVALID_VFC,	INVALID_VFC	)

DECLARE_D1_FUNCTION(	uni,	rn_uni,		FVUNI,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	sqrt,	sqrt,		FVSQRT,		INVALID_VFC,	INVALID_VFC	)
#ifdef HAVE_ERF
DECLARE_D1_FUNCTION(	erf,	erf,		FVERF,		INVALID_VFC,	INVALID_VFC	)
#endif // HAVE_ERF
DECLARE_D1_FUNCTION(	erfinv,	erfinv,		FVERFINV,	INVALID_VFC,	INVALID_VFC	)
#ifdef SINE_TBL
DECLARE_D1_FUNCTION(	sin,	t_sin,		FVSIN,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	cos,	t_cos,		FVCOS,		INVALID_VFC,	INVALID_VFC	)
#else /* ! SINE_TBL */
DECLARE_D1_FUNCTION(	sin,	sin,		FVSIN,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	cos,	cos,		FVCOS,		INVALID_VFC,	INVALID_VFC	)
#endif /* ! SINE_TBL */
DECLARE_D1_FUNCTION(	gamma,	tgamma,		FVGAMMA,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	lngamma, lgamma,	FVLNGAMMA,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	exp,	exp,		FVEXP,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	log,	log,		FVLOG,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	log10,	log10,		FVLOG10,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	random,	rn_number,	FVRAND,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	abs,	fabs,		FVABS,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	sign,	signfunc,	FVSIGN,		INVALID_VFC,	INVALID_VFC	)
DECLARE_I1_FUNCTION(	isnan,	isnanfunc,	FVISNAN,	INVALID_VFC,	INVALID_VFC	)
DECLARE_I1_FUNCTION(	isinf,	isinffunc,	FVISINF,	INVALID_VFC,	INVALID_VFC	)
DECLARE_I1_FUNCTION(	isnormal, isnormalfunc,	FVISNORM,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	atan,	atan,		FVATAN,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	acos,	acos,		FVACOS,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	asin,	asin,		FVASIN,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	floor,	floor,		FVFLOOR,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	trunc,	trunc,		FVTRUNC,	INVALID_VFC,	INVALID_VFC	)
#ifdef HAVE_ROUND
DECLARE_D1_FUNCTION(	round,	round,		FVROUND,	INVALID_VFC,	INVALID_VFC	)
#endif // HAVE_ROUND
DECLARE_D1_FUNCTION(	ceil,	ceil,		FVCEIL,		INVALID_VFC,	INVALID_VFC	)
#ifdef HAVE_RINT
DECLARE_D1_FUNCTION(	rint,	rint,		FVRINT,		INVALID_VFC,	INVALID_VFC	)
#endif // HAVE_RINT
#ifndef MAC
#ifndef _WINDOWS
DECLARE_D1_FUNCTION(	j0,	j0,		INVALID_VFC,	INVALID_VFC,	INVALID_VFC	)
#endif /* !_WINDOWS */
#endif /* !MAC */
DECLARE_D1_FUNCTION(	ptoz,	ptoz,		INVALID_VFC,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	ztop,	ztop,		INVALID_VFC,	INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	tan,	tan,		FVTAN,		INVALID_VFC,	INVALID_VFC	)
DECLARE_D1_FUNCTION(	bitsum,	bitsum,		INVALID_VFC,	INVALID_VFC,	INVALID_VFC	)

DECLARE_D2_FUNCTION(	atan2,	atan2,		FVATAN2,	FVSATAN2,	FVSATAN22	)
DECLARE_D2_FUNCTION(	pow,	pow,		FVPOW,		FVSPOW,		FVSPOW2		)
DECLARE_D2_FUNCTION(	max,	maxfunc,	FVMAX,		FVSMAX,		FVSMAX		)
DECLARE_D2_FUNCTION(	min,	minfunc,	FVMIN,		FVSMIN,		FVSMIN		)

// STR1 functions take one string as input and return double
DECLARE_STR1_FUNCTION(	strlen,		dstrlen		)
DECLARE_STR1_FUNCTION(	var_exists,	dvarexists	)
DECLARE_STR1_FUNCTION(	macro_exists,	dmacroexists	)
DECLARE_STR1_FUNCTION(	exists,		dexists		)
DECLARE_STR1_FUNCTION(	file_exists,	dexists	 	)
DECLARE_STR1_FUNCTION(	mod_time,	modtimefunc	)
DECLARE_STR1_FUNCTION(  ascii,		ascii_val	)
DECLARE_STR1_FUNCTION(	is_directory,	disdir	 )
DECLARE_STR1_FUNCTION(	is_regfile,	disreg	 )
DECLARE_STR1_FUNCTION(	unix_time,	unix_time	 )

DECLARE_STR2_FUNCTION(	strcmp,	dstrcmp		)
DECLARE_STR2_FUNCTION(	strstr,	dstrstr		)
DECLARE_STR3_FUNCTION(	strncmp,dstrncmp	)

// shouldn't these be string valued functions that translat a whole string?
// But what about the mapping functions???

// STRV functions take a string as input and produce a string as output
DECLARE_STRV_FUNCTION(	tolower, s_tolower,	FVTOLOWER, INVALID_VFC,	INVALID_VFC	)
DECLARE_STRV_FUNCTION(	toupper, s_toupper,	FVTOUPPER, INVALID_VFC,	INVALID_VFC	)

// STRV3 takes a long integer as input and produces a string
DECLARE_STRV3_FUNCTION(	format_time, _format_time )
DECLARE_STRV3_FUNCTION(	ascii_char, _ascii_to_string )

DECLARE_CHAR_FUNCTION(	isprint, c_isprint,	FVISPRINT, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	islower, c_islower,	FVISLOWER, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	isupper, c_isupper,	FVISUPPER, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	isalpha, c_isalpha,	FVISALPHA, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	isdigit, c_isdigit,	FVISDIGIT, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	isalnum, c_isalnum,	FVISALNUM, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	isspace, c_isspace,	FVISSPACE, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	iscntrl, c_iscntrl,	FVISCNTRL, INVALID_VFC, INVALID_VFC	)
DECLARE_CHAR_FUNCTION(	isblank, c_isblank,	FVISBLANK, INVALID_VFC, INVALID_VFC	)

// This was called prec_name, but that was defined by a C macro to be prec_item.item_name!?
// string in, string out, but input string should be an object name
DECLARE_STRV_FUNCTION(	precision,	_precfunc, INVALID_VFC, INVALID_VFC, INVALID_VFC	)

// two strings in, one string out
DECLARE_STRV2_FUNCTION(	strcat,		strcat_func	)

DECLARE_SIZE_FUNCTION(	depth,	_dpfunc,	0	)
DECLARE_SIZE_FUNCTION(	ncols,	_colfunc,	1	)
DECLARE_SIZE_FUNCTION(	nrows,	_rowfunc,	2	)
DECLARE_SIZE_FUNCTION(	nframes,_frmfunc,	3	)
DECLARE_SIZE_FUNCTION(	nseqs,	_seqfunc,	4	)
DECLARE_SIZE_FUNCTION(	ncomps,	_dpfunc,	0	)
DECLARE_SIZE_FUNCTION(	nelts,	_nefunc,	-1	)

//DECLARE_SIZE_FUNCTION(	is_interlaced,_ilfunc,	-1	)
DECLARE_ILACE_FUNCTION(	is_interlaced,	_ilfunc		)

DECLARE_POSITION_FUNCTION(	x_position,	_x_func,	0	)
DECLARE_POSITION_FUNCTION(	y_position,	_y_func,	1	)

DECLARE_TS_FUNCTION(	seconds,	_secfunc	)
DECLARE_TS_FUNCTION(	milliseconds,	_msecfunc	)
DECLARE_TS_FUNCTION(	microseconds,	_usecfunc	)

DECLARE_CAM_FUNCTION(	is_capturing,	_captfunc	)
}


#ifdef NOT_NEEDED
void assign_func_ptr(const char *name,double (*func)(void))
{
	Quip_Function *func_p;

	func_p = function_of(DEFAULT_QSP_ARG  name);
	assert( func_p != NULL );

	func_p->fn_u.d0_func = func;
}
#endif /* NOT_NEEDED */

double evalD0Function( Quip_Function *func_p )
{
	return (*(func_p->fn_u.d0_func))();
}

double evalD1Function( Quip_Function *func_p, double arg )
{
	return (*(func_p->fn_u.d1_func))(arg);
}

double evalD2Function( Quip_Function *func_p, double arg1, double arg2 )
{
	return (*(func_p->fn_u.d2_func))(arg1,arg2);
}

int evalI1Function( Quip_Function *func_p, double arg )
{
	return (*(func_p->fn_u.i1_func))(arg);
}

double evalStr1Function( QSP_ARG_DECL  Quip_Function *func_p, const char *s )
{
	return (*(func_p->fn_u.str1_func))(QSP_ARG  s);
}

int evalCharFunction( Quip_Function *func_p, char c )
{
	return (*(func_p->fn_u.char_func))(c);
}

double evalStr2Function( Quip_Function *func_p, const char *s1, const char *s2 )
{
	return (*(func_p->fn_u.str2_func))(s1,s2);
}

double evalStr3Function( Quip_Function *func_p, const char *s1, const char *s2, int n )
{
	return (*(func_p->fn_u.str3_func))(s1,s2,n);
}

#ifndef BUILD_FOR_OBJC
const char *default_prec_name(QSP_ARG_DECL  Item *ip)
{
	return "u_byte";	// could point to precision struct?  to save 7 bytes!?
}
#endif // ! BUILD_FOR_OBJC

