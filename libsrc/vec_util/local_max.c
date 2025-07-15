#include "quip_config.h"

/* find local maxima - used on output of hough transform.
 * This version is naive and doesn't worry about wrap-around...
 * How should we handle this?
 *
 * For now, we assume that the hough transform is padded so we don't need to check
 * the boundary pixels or deal with wrap-around.  This simplifies things considerably...
 */

#include "vec_util.h"
#include "quip_prot.h"

long _local_maxima(QSP_ARG_DECL  Data_Obj *val_dp, Data_Obj *coord_dp, Data_Obj *src_dp )
{
	float *nw_p, *w_p, *sw_p;
	float v,*src_p;
	float *base_p;
	u_long *coord_p;
	u_long x,y;
	dimension_t n_maxima;
	float *val_p;

	VINSIST_RAM_OBJ(val_dp,local_maxima,0);
	VINSIST_RAM_OBJ(coord_dp,local_maxima,0);
	VINSIST_RAM_OBJ(src_dp,local_maxima,0);

	/* For now, we only support contiguous float */
	if( OBJ_PREC(src_dp) != PREC_SP ){
		snprintf(ERROR_STRING,LLEN,"local_maxima:  source image %s (%s) should have %s precision!?",
			OBJ_NAME(src_dp),OBJ_PREC_NAME(src_dp),NAME_FOR_PREC_CODE(PREC_SP));
		advise(ERROR_STRING);
		return(-1);
	}
	if( OBJ_PREC(val_dp) != PREC_SP ){
		snprintf(ERROR_STRING,LLEN,"local_maxima:  max value vector %s (%s) should have %s precision!?",
			OBJ_NAME(val_dp),OBJ_PREC_NAME(val_dp),NAME_FOR_PREC_CODE(PREC_SP));
		advise(ERROR_STRING);
		return(-1);
	}
	if( ! IS_CONTIGUOUS(src_dp) ){
		snprintf(ERROR_STRING,LLEN,"local_maxima:  source image %s should be contiguous!?",
			OBJ_NAME(src_dp));
		advise(ERROR_STRING);
		return(-1);
	}
	if( OBJ_PREC(coord_dp) != PREC_UDI ){
		snprintf(ERROR_STRING,LLEN,"local_maxima:  coordinate array %s (%s) should have %s precision!?",
			OBJ_NAME(coord_dp),OBJ_PREC_NAME(coord_dp),NAME_FOR_PREC_CODE(PREC_UDI));
		advise(ERROR_STRING);
		return(-1);
	}
	if( ! IS_CONTIGUOUS(coord_dp) ){
		snprintf(ERROR_STRING,LLEN,"local_maxima:  coordinate vector %s should be contiguous!?",
			OBJ_NAME(coord_dp));
		advise(ERROR_STRING);
		return(-1);
	}
	if( ! IS_CONTIGUOUS(val_dp) ){
		snprintf(ERROR_STRING,LLEN,"local_maxima:  max value vector %s should be contiguous!?",
			OBJ_NAME(val_dp));
		advise(ERROR_STRING);
		return(-1);
	}

	base_p = (float *)OBJ_DATA_PTR(src_dp);
	coord_p = (u_long *)OBJ_DATA_PTR(coord_dp);
	val_p = (float *)OBJ_DATA_PTR(val_dp);

	n_maxima=0;

	for(y=1;y<(OBJ_ROWS(src_dp)-1);y++){
		nw_p = base_p;
		w_p = nw_p + OBJ_COLS(src_dp);
		sw_p = w_p + OBJ_COLS(src_dp);
		src_p = w_p + 1;
		for(x=1;x<(OBJ_COLS(src_dp)-1);x++){
			v=(*src_p);
//snprintf(ERROR_STRING,LLEN,"%g at %d %d",v,x,y);
//prt_msg(ERROR_STRING);
			if(
				   v >  *(nw_p  )
				&& v >  *(nw_p+1)
				&& v >  *(nw_p+2)
				&& v >  *( w_p  )
				&& v >= *( w_p+2)
				&& v >= *(sw_p  )
				&& v >= *(sw_p+1)
				&& v >= *(sw_p+2)
								){

//snprintf(ERROR_STRING,LLEN,"local max %g at %d %d",v,x,y);
//advise(ERROR_STRING);
				/* this is a local max! */
				n_maxima++;
				if( n_maxima > OBJ_COLS(coord_dp) ){
					snprintf(ERROR_STRING,LLEN,"local_maxima:  coord vector %s (%d columns) needs to be enlarged!?",
						OBJ_NAME(coord_dp),OBJ_COLS(coord_dp));
					warn(ERROR_STRING);
					return(OBJ_COLS(coord_dp));
				}
				*val_p++ = v;
				*coord_p++ = x;
				*coord_p++ = y;
			}

			src_p++;
			nw_p++;
			w_p++;
			sw_p++;
		}
		base_p += OBJ_COLS(src_dp);
	}
	return(n_maxima);
}

