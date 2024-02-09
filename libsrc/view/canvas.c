#include "quip_config.h"

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "quip_prot.h"
#include "xsupp.h"
#include "viewer.h"
#include "view_prot.h"

#ifndef BUILD_FOR_IOS

int add_image( Viewer *vp, Data_Obj *dp, int x, int y )
{
	Window_Image *wip;
	Node *np;

	/* Don't add the image if it is already on the list... */
	np = QLIST_HEAD( VW_IMAGE_LIST(vp) );
	while( np != NULL ){
		wip = NODE_DATA(np);
		if( wip->wi_dp == dp ) {
			return 0;
		}
		np = NODE_NEXT(np);
	}
		
	wip=(Window_Image *)getbuf(sizeof(*wip));
	wip->wi_dp = dp;
	wip->wi_x = x;
	wip->wi_y = y;
	np = mk_node(wip);
	addTail(VW_IMAGE_LIST(vp),np);
	return 1;
}
#endif /* ! BUILD_FOR_IOS */

// This is really a sequence viewer, and is not currently used...

void _old_load_viewer( QSP_ARG_DECL  Viewer *vp, Data_Obj *dp )
{
	dimension_t i;

	/* We don't need to care if the size isn't an exact match */
	/*
	if( OBJ_COLS(dp) != VW_WIDTH(vp) ||
		OBJ_ROWS(dp) != VW_HEIGHT(vp) ){
		snprintf(ERROR_STRING,LLEN,
		"Size mismatch between viewer %s and image %s",
			vp->vw_name, OBJ_NAME(dp));
		warn(ERROR_STRING);
		return;
	}
	*/

	/*
	if( (8*OBJ_COMPS(dp)) != vp->vw_depth ){
		snprintf(ERROR_STRING,LLEN,
		"Depth mismatch between image %s (%ld) and viewer %s (%d)",
			OBJ_NAME(dp),8*OBJ_COMPS(dp),vp->vw_name,vp->vw_depth);
		warn(ERROR_STRING);
		return;
	}
	*/

	if( ! IS_CONTIGUOUS(dp) ){
		snprintf(ERROR_STRING,LLEN,
		"Can't display non-contiguous image %s (viewer %s)",
			OBJ_NAME(dp),VW_NAME(vp));
		warn(ERROR_STRING);
		longlist(dp);
		return;
	}
	if( OBJ_MACH_PREC(dp) != PREC_BY && OBJ_MACH_PREC(dp) != PREC_UBY ){
		snprintf(ERROR_STRING,LLEN,"Bad pixel format (%s), image %s",
				OBJ_PREC_NAME(dp),OBJ_NAME(dp));
		warn(ERROR_STRING);
		advise("Only byte images may be displayed in viewers");
		return;
	}
	if( IS_DRAGSCAPE(vp) ){
		//zap_image_list(vp);
#ifndef BUILD_FOR_OBJC
		rls_list_nodes(vp->vw_image_list);
		add_image(vp,dp,0,0);
#endif // BUILD_FOR_OBJC
	} else {
		/* If we are holding an image, release it */
		if( VW_OBJ(vp) != NULL )
			release_image(VW_OBJ(vp));
		SET_VW_OBJ(vp,dp);
		/* make sure this image doesn't get deleted out from under us */
		SET_OBJ_REFCOUNT(dp,
			OBJ_REFCOUNT(dp)+1 );
	}

	for(i=0;i<OBJ_FRAMES(dp);i++){
		SET_VW_FRAMENO(vp,i);
		//refresh_image(QSP_ARG  vp);
		usleep(16000);	/* approx 16 msec */
	}
	select_viewer(vp);
} // end old_load_viewer

#ifndef BUILD_FOR_IOS
void _bring_image_to_front(QSP_ARG_DECL  Viewer *vp, Data_Obj *dp, int x, int y )
{
	Node *np;
	Window_Image *wip;
	int n_tried=0, n_possible;
	List *lp;

	lp = VW_IMAGE_LIST(vp);

	assert(lp!=NULL);
	assert(QLIST_HEAD(lp)!=NULL);

	n_possible = eltcount(lp);
	while( n_tried < n_possible ){
		np = QLIST_HEAD(lp);
		wip = NODE_DATA(np);
		if( wip->wi_dp == dp ){
			// image is first in the list now
			wip->wi_x = x;
			wip->wi_y = y;
			embed_image(vp,dp,wip->wi_x,wip->wi_y);
			return;
		}
		// rotate the list
		np = remHead( lp );
		addTail(lp,np);
		n_tried ++;
	}
	snprintf(ERROR_STRING,LLEN,"bring_image_to_front:  image %s not found attached to viewer %s!?",OBJ_NAME(dp),VW_NAME(vp));
	warn(ERROR_STRING);
} // end bring_image_to_front

void _clear_queue( QSP_ARG_DECL  Viewer *vp )
{
	warn("clear_queue not implemented!?");
}

void _queue_frame( QSP_ARG_DECL  Viewer *vp, Data_Obj *dp )
{
	warn("queue_frame not implemented!?");
}

#endif // ! BUILD_FOR_IOS

// queue_frame is supposed to load without displaying,
// but we'll do that another way...

void _load_viewer( QSP_ARG_DECL  Viewer *vp, Data_Obj *dp )
{
	if( OBJ_FRAMES(dp) != 1 ){
#ifdef NOT_USED_NOW
		snprintf(ERROR_STRING,LLEN,"load_viewer:  Object %s has %d frames, calling old_load_viewer().",
		OBJ_NAME(dp),OBJ_FRAMES(dp));
		advise(ERROR_STRING);
		old_load_viewer(vp,dp);
#else /* ! NOT_USED_NOW */
		warn("load_viewer:  don't know what to do with sequence object...");
#endif /* ! NOT_USED_NOW */
		return;
	}

	/* This caused a memory leak bug - add a zap_image_list()
	 * if we want this here...
	 */

	/* BUG we should have a mode that specifies whether images
	 * should be added to the list or not
	 */
#ifdef BUILD_FOR_IOS
	embed_image(vp,dp,0,0);
#else /* ! BUILD_FOR_IOS */

	if( add_image(vp,dp,0,0) ){
//fprintf(stderr,"load_viewer:  redrawing image %s\n",OBJ_NAME(dp));
		embed_image(vp,dp,0,0);
	} else {
		// This image has already been displayed...
//fprintf(stderr,"load_viewer:  bringing old image %s to front\n",OBJ_NAME(dp));
		bring_image_to_front(vp,dp,0,0);
	}

#endif /* ! BUILD_FOR_IOS */
} // end load_viewer


