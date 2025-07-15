
#include "quip_config.h"

#include "quip_prot.h"
#include "getbuf.h"
#include "data_obj.h"
#include "veclib/fftsupp.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif // ! TRUE

// BUG global vars not thread-safe!?
dimension_t bitrev_size=0;
dimension_t *bitrev_data;

static void mk_bitrev(dimension_t n)
{
	dimension_t i,j;
	dimension_t m;
		
	/*	bit reverse	*/

	/*		i	j
			0	0
			1	n/2
			2	n/4
			3	3n/4
	*/

	j = 0;
	for(i=0;i<n;i++){
		bitrev_data[i]=j;
		m = n/2;
		while (j>=m) {
			j -= m;
			m = (m+1)/2;
		}
		j += m;
	}
	bitrev_size = n;
}

void bitrev_init(dimension_t len)
{
	if( bitrev_size > 0 ){
		givbuf(bitrev_data);
	}
	bitrev_data = (dimension_t *)getbuf( sizeof(*bitrev_data) * len );
	mk_bitrev(len);
}

static int log_2(dimension_t n)
{
	dimension_t p;
	int w;

	p = 0;
	w = 0;	
	while (n >= 2 && p == 0) {
		++w;
		p = n % 2;
		n = n / 2;
	}
	if (p != 0) return(-1);
	else return(w);
}

#define fft_row_size_ok(dp, funcname) _fft_row_size_ok(QSP_ARG  dp, funcname)

static int _fft_row_size_ok(QSP_ARG_DECL  Data_Obj *dp, const char * funcname )
{
	if( log_2(OBJ_COLS(dp)) == -1 ){
		snprintf(ERROR_STRING,LLEN,
	"%s:  number of columns of image %s (%d) is not a power of two for FFT",
			funcname,OBJ_NAME(dp),OBJ_COLS(dp));
		WARN(ERROR_STRING);
		longlist(dp);
		return(-1);
	}

	return(0);
}

#define dim_is_power_of_two(dp, dim_idx, funcname) _dim_is_power_of_two( QSP_ARG  dp, dim_idx, funcname )

static int _dim_is_power_of_two( QSP_ARG_DECL  Data_Obj *dp, int dim_idx, const char *funcname )
{
	if( log_2( OBJ_DIMENSION(dp,dim_idx) ) == -1 ){
		snprintf(ERROR_STRING,LLEN,
	"%s:  Number of %ss of image %s (%d) is not a power of two!?", funcname,
			dimension_name[dim_idx],OBJ_NAME(dp),OBJ_DIMENSION(dp,dim_idx));
		WARN(ERROR_STRING);
		longlist(dp);
		return FALSE;
	}
	return TRUE;
}

// Some day we may want to relax the restriction of power-of-2...

#define fft_col_size_ok(dp, funcname) _fft_col_size_ok(QSP_ARG  dp, funcname)

static int _fft_col_size_ok(QSP_ARG_DECL  Data_Obj *dp, const char *funcname )
{
	return dim_is_power_of_two(dp, 2, funcname );
	/*
	if( log_2(OBJ_ROWS(dp)) == -1 ){
		snprintf(ERROR_STRING,LLEN,
	"Number of rows of image %s (%d) is not a power of two for FFT",
			OBJ_NAME(dp),OBJ_ROWS(dp));
		WARN(ERROR_STRING);
		longlist(dp);
		return(-1);
	}
	return(0);
	*/
}

#define fft_size_ok(dp, funcname) _fft_size_ok(QSP_ARG  dp, funcname)

static int _fft_size_ok(QSP_ARG_DECL  Data_Obj *dp, const char * funcname )
{
	if( ! fft_row_size_ok(dp, funcname ) ) return FALSE;
	if( ! fft_col_size_ok(dp, funcname ) ) return FALSE;
	return TRUE;
}

#define good_xform_size(real_dp,cpx_dp, dim_idx, funcname) _good_xform_size( QSP_ARG  real_dp,cpx_dp, dim_idx, funcname)

static int _good_xform_size( QSP_ARG_DECL  Data_Obj *real_dp,Data_Obj *cpx_dp, int dim_idx, const char *funcname)
{
	if( (OBJ_DIMENSION(cpx_dp,dim_idx)-1) != (OBJ_DIMENSION(real_dp,dim_idx)/2) ){
		snprintf(ERROR_STRING,LLEN,
"%s:  complex %s %ss (%d) should be 1 plus half real %s %ss (%d)",
			funcname,OBJ_NAME(cpx_dp),dimension_name[dim_idx],OBJ_DIMENSION(cpx_dp,dim_idx),
			         OBJ_NAME(real_dp),dimension_name[dim_idx],OBJ_DIMENSION(real_dp,dim_idx));
		WARN(ERROR_STRING);
		return FALSE;
	}
	return TRUE;
}

#define real_cpx_objs_ok(real_dp,cpx_dp, funcname) _real_cpx_objs_ok( QSP_ARG  real_dp,cpx_dp, funcname )

static int _real_cpx_objs_ok( QSP_ARG_DECL  Data_Obj *real_dp,Data_Obj *cpx_dp, const char *funcname )
{
	if( ! IS_COMPLEX(cpx_dp) ){
		snprintf(ERROR_STRING,LLEN,
			"%s:  %s must be complex",funcname,OBJ_NAME(cpx_dp));
		WARN(ERROR_STRING);
		return FALSE;
	}
	if( ! IS_REAL(real_dp) ){
		snprintf(ERROR_STRING,LLEN,
			"%s:  %s must be real",funcname,OBJ_NAME(real_dp));
		WARN(ERROR_STRING);
		return FALSE;
	}
	if( ! FLOATING_OBJ( cpx_dp ) ){
		snprintf(ERROR_STRING,LLEN,
			"%s:  precision must be float or double",funcname);
		WARN(ERROR_STRING);
		return FALSE;
	}

	if( !dp_same_mach_prec(cpx_dp,real_dp,funcname) ){
		snprintf(ERROR_STRING,LLEN,
	"%s:  complex object (%s,%s) and target (%s,%s) must have same precision",
			funcname,OBJ_NAME(cpx_dp),OBJ_MACH_PREC_NAME(cpx_dp),
			OBJ_NAME(real_dp),OBJ_MACH_PREC_NAME(real_dp));
		WARN(ERROR_STRING);
		return FALSE;
	}
	return TRUE;
}

int real_row_fft_ok(QSP_ARG_DECL  Data_Obj *real_dp,Data_Obj *cpx_dp,const char *funcname)
{
	if( ! good_xform_size( real_dp, cpx_dp, 1, funcname ) ) return FALSE;
	if( OBJ_ROWS(cpx_dp) != OBJ_ROWS(real_dp) ){
		snprintf(ERROR_STRING,LLEN,
			"%s:  row count mismatch, %s (%d) and %s (%d)",
			funcname,OBJ_NAME(cpx_dp),OBJ_ROWS(cpx_dp),
			OBJ_NAME(real_dp),OBJ_ROWS(real_dp));
		WARN(ERROR_STRING);
		return FALSE;
	}
	if( ! real_cpx_objs_ok( real_dp, cpx_dp, funcname ) ) return FALSE;
	if( ! fft_row_size_ok(real_dp, funcname ) ) return FALSE;
	return TRUE;
}

// Originally, we insisted that the row length of the transform be 1+N/2,
// and the column size be a power of two; but clFFT uses the opposite of that
// so to provide a compatible routine we now accept either.  We return 1 for the
// old convention, 2 for the new one, and -1 for error.

int real_fft_type(QSP_ARG_DECL  Data_Obj *real_dp,Data_Obj *cpx_dp,const char *funcname)
{
	// First make sure the objects match in precision and are of the correct type
	if( ! real_cpx_objs_ok( real_dp, cpx_dp, funcname ) ) return -1;


	if( OBJ_ROWS(real_dp) == OBJ_ROWS(cpx_dp) ){
		if( ! good_xform_size( real_dp, cpx_dp, 1, funcname ) ) return -1;
		if( ! dim_is_power_of_two(real_dp, 2, funcname ) ) return -1;
		return FFT2D_REAL_XFORM_ROWS;
	} else if( OBJ_COLS(real_dp) == OBJ_COLS(cpx_dp) ){
		if( ! good_xform_size( real_dp, cpx_dp, 2, funcname ) ) return -1;
		if( ! dim_is_power_of_two(real_dp, 1, funcname ) ) return -1;
		return FFT2D_REAL_XFORM_COLS;
	} else {
		snprintf(ERROR_STRING,LLEN,
"%s:  real data %s (%d x %d) and transform %s (%d x %d) must have one matching dimension!?",
			funcname,OBJ_NAME(real_dp),OBJ_ROWS(real_dp),OBJ_COLS(real_dp),
			         OBJ_NAME(cpx_dp),OBJ_ROWS(cpx_dp),OBJ_COLS(cpx_dp));
		WARN(ERROR_STRING);
		return -1;
	}
}

// We use this when we want to take the transform of just the rows of an image...
// But why insist that the image be complex?  BUG?

int row_fft_ok(QSP_ARG_DECL  Data_Obj *dp, const char * funcname )
{
	if( ! IS_COMPLEX(dp) ){
		snprintf(ERROR_STRING,LLEN,
			"%s:  image %s is not complex!?",funcname,OBJ_NAME(dp));
		WARN(ERROR_STRING);
		return FALSE;
	}

	if( fft_row_size_ok(dp, funcname ) < 0 )
		return FALSE;

	return TRUE;
}

int cpx_fft_ok(QSP_ARG_DECL  Data_Obj *dp, const char *funcname )
{
	if( ! IS_COMPLEX(dp) ){
		snprintf(ERROR_STRING,LLEN,
			"%s:  image %s is not complex",funcname,OBJ_NAME(dp));
		WARN(ERROR_STRING);
		return FALSE;
	}

	if( fft_size_ok(dp, funcname ) < 0 )
		return FALSE;

	return TRUE;
}

void show_fft_args(FFT_Args *fap )
{
	fprintf(stderr,"FFT args at 0x%lx\n",(long)fap);
	fprintf(stderr,"\tdest 0x%lx, inc = %d\n",(long)FFT_DST(fap),FFT_DINC(fap));
	fprintf(stderr,"\tsrc  0x%lx, inc = %d\n",(long)FFT_SRC(fap),FFT_SINC(fap));
	fprintf(stderr,"\tlen  %d, is_inverse = %d\n",FFT_LEN(fap),FFT_ISI(fap));
	fflush(stderr);
}


