#include "quip_config.h"

/* draggable objects for xlib !? */

#include "quip_prot.h"
#include "data_obj.h"
#include "getbuf.h"
#include "viewer.h"
#include "item_type.h"
#include "item_prot.h"

ITEM_INTERFACE_DECLARATIONS(Draggable,dragg,0)

void _make_dragg(QSP_ARG_DECL  const char *name,Data_Obj *bm,Data_Obj *dp)
{
	Draggable *dgp;

	if( !dp_same_size(bm,dp,"make_dragg") ){
		warn("image/bitmap size mismatch");
		return;
	}
	if( OBJ_PREC(bm) != PREC_BIT ){
		snprintf(ERROR_STRING,LLEN,"Object %s has precision %s, should be %s",
			OBJ_NAME(bm),PREC_NAME(OBJ_PREC_PTR(bm)),
			PREC_NAME(PREC_FOR_CODE(PREC_BIT)));
		warn(ERROR_STRING);
		return;
	}
	if( OBJ_PREC(dp) != PREC_BY && OBJ_PREC(dp) != PREC_UBY ){
		snprintf(ERROR_STRING,LLEN,"Image %s (for draggable object) has %s precision, should be %s or %s",
			OBJ_NAME(dp),PREC_NAME(OBJ_PREC_PTR(dp)),
			PREC_NAME(PREC_FOR_CODE(PREC_BY)),
			PREC_NAME(PREC_FOR_CODE(PREC_UBY)) );
		warn(ERROR_STRING);
		return;
	}

	dgp = new_dragg(name);
	if( dgp == NULL ) return;

	dgp->dg_width = (int) OBJ_COLS(dp);
	dgp->dg_height = (int) OBJ_ROWS(dp);
	dgp->dg_bitmap = bm;
	dgp->dg_image = dp;
	dgp->dg_np = mk_node(dgp);
}

