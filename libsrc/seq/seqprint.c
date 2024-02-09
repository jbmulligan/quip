/** seqprint.c	routines to print out sequence information */

#include "quip_config.h"

#include <stdio.h>

#include "quip_prot.h"
#include "data_obj.h"
#include "seq.h"

void pseq(QSP_ARG_DECL  Seq *seqptr)		/** print out a sequence */
{
	snprintf(msg_str,LLEN,"%s: ",seqptr->seq_name);
	prt_msg_frag(msg_str);
	pfull(QSP_ARG  seqptr);
	prt_msg("");
}

void pframe(QSP_ARG_DECL   Seq *seqptr)
{
	Item *ip;

	ip = (Item *)seqptr->seq_data;
	snprintf(msg_str,LLEN,"%d * %s",seqptr->seq_count,ip->item_name);
	prt_msg_frag(msg_str);
}

void pfull(QSP_ARG_DECL   Seq *seqptr )
{
	if( seqptr->seq_first == NULL && seqptr->seq_next==NULL )	/* frame */
		pframe(QSP_ARG  seqptr);
	else {						/* concatentation */
		if( seqptr->seq_count == -1 ){	/* reversal */
			prt_msg_frag("reverse( ");
			pone(QSP_ARG  seqptr->seq_first);
			prt_msg_frag(" )");
			return;
		}
		if( seqptr->seq_count != 1 ){
			snprintf(msg_str,LLEN,"%d ( ",seqptr->seq_count);
			prt_msg_frag(msg_str);
		}
		pone(QSP_ARG  seqptr->seq_first);
		if( seqptr->seq_next != NULL ){
			prt_msg_frag(" + ");
			pone(QSP_ARG  seqptr->seq_next);
		}
		if( seqptr->seq_count != 1 )
			prt_msg_frag(" ) ");
	}
}

void pone(QSP_ARG_DECL   Seq *seqptr )
{
	if( seqptr==NULL ){ /* end of the chain */
		/* warn("pone:  null sequence"); */
		return;
	}
	if( seqptr->seq_name!=NULL ){	/* frames don't have seqnames ... */
		snprintf(msg_str,LLEN,"%s",seqptr->seq_name);
		prt_msg_frag(msg_str);
	} else if( seqptr->seq_first == NULL && seqptr->seq_next==NULL ) /* frame */
		pframe(QSP_ARG  seqptr);
	else {						/* concatentation */
		if( seqptr->seq_count == -1 ){
			prt_msg_frag("reverse( ");
			pone(QSP_ARG  seqptr->seq_first);
			prt_msg_frag(" )");
			return;
		}
		if( seqptr->seq_count != 1 ){
			snprintf(msg_str,LLEN,"%d ( ",seqptr->seq_count);
			prt_msg_frag(msg_str);
		}
		pone(QSP_ARG  seqptr->seq_first);
		if( seqptr->seq_next != NULL ){
			prt_msg_frag(" + ");
			pone(QSP_ARG  seqptr->seq_next);
		}
		if( seqptr->seq_count != 1 )
			prt_msg_frag(" ) ");
	}
}

