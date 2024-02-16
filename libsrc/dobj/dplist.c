#include "quip_config.h"

/* information display functions */

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "quip_prot.h"
#include "data_obj.h"
#include "dobj_prot.h"
#include "debug.h"
#include "platform.h"

// BUG prec_for_code should use table lookup instead of list search

// missing precision 53?

Precision *prec_for_code(prec_t prec)
{
	// find the precision struct using the code
	List *lp;
	Node *np;
	Precision *prec_p;

	lp = prec_list(SGL_DEFAULT_QSP_ARG);
	assert( lp != NULL );

	np = QLIST_HEAD(lp);

	while( np != NULL ){
		prec_p = (Precision *) NODE_DATA(np);
		if( prec_p->prec_code == prec )
			return( prec_p );
		np=NODE_NEXT(np);
	}
	// shouldn't happen!

	assert( AERROR("Missing precision code in list of precisions!?") );

	return NULL;
}

void _describe_shape(QSP_ARG_DECL  Shape_Info *shpp)
{
	assert( SHP_PREC_PTR(shpp) != NULL );
	
	if( SHP_PREC(shpp) == PREC_VOID ){
		prt_msg("void (no shape)");
		return;
	}
	assert( SHP_TYPE_DIMS(shpp) != NULL );
	assert( SHP_MACH_DIMS(shpp) != NULL );

	if( HYPER_SEQ_SHAPE(shpp) )
		snprintf(MSG_STR,LLEN,"hyperseq, %3u sequences          ",
			SHP_SEQS(shpp));
	else if( SEQUENCE_SHAPE(shpp) )
		snprintf(MSG_STR,LLEN,"sequence, %3u %dx%d frames             ",
			SHP_FRAMES(shpp),SHP_ROWS(shpp),SHP_COLS(shpp));
	else if( IMAGE_SHAPE(shpp) ){
		/* used to have a special case for bitmaps here, but
		 * apparently no longer needed...
		 */
		snprintf(MSG_STR,LLEN,"image, %4u rows   %4u columns  ",
			SHP_ROWS(shpp),SHP_COLS(shpp));
	} else if( ROWVEC_SHAPE(shpp) )
		snprintf(MSG_STR,LLEN,"row vector, %4u elements        ",SHP_COLS(shpp));
	else if( COLVEC_SHAPE(shpp) )
		snprintf(MSG_STR,LLEN,"column vector, %4u elements     ",SHP_ROWS(shpp));
	else if( PIXEL_SHAPE(shpp) )
		snprintf(MSG_STR,LLEN,"scalar                           ");
	else if( UNKNOWN_SHAPE(shpp) )
		snprintf(MSG_STR,LLEN,"shape unknown at this time       ");
	else if( VOID_SHAPE(shpp) )
		snprintf(MSG_STR,LLEN,"void shape                            ");
	else {
fprintf(stderr,"no categorization of shape at 0x%lx!?\n",(long)shpp);
		assert( AERROR("describe_shape:  bad object type flag!?") );
	}
	prt_msg_frag(MSG_STR);

	if( BITMAP_PRECISION(SHP_PREC(shpp)) ){
		assert( (SHP_PREC(shpp) & MACH_PREC_MASK) == BITMAP_MACH_PREC );
		
		prt_msg_frag("     bit");
	} else if( STRING_PRECISION(SHP_PREC(shpp)) || CHAR_PRECISION(SHP_PREC(shpp)) ){
		assert( (SHP_PREC(shpp) & MACH_PREC_MASK) == PREC_BY );

		if( STRING_PRECISION(SHP_PREC(shpp)) )
			prt_msg("     string");
		else if( CHAR_PRECISION(SHP_PREC(shpp)) )
			prt_msg("       char");
		return;
	} else {
		snprintf(MSG_STR,LLEN,"     %s",PREC_NAME( SHP_MACH_PREC_PTR(shpp) ) );
		prt_msg_frag(MSG_STR);
		if( COMPLEX_PRECISION(SHP_PREC(shpp)) ){
			if( (SHP_PREC(shpp) & MACH_PREC_MASK) == PREC_SP )
				snprintf(MSG_STR,LLEN,", complex");
			else if( (SHP_PREC(shpp) & MACH_PREC_MASK) == PREC_DP )
				snprintf(MSG_STR,LLEN,", dblcpx");
			else {
				assert( AERROR("Unexpected complex machine precision!?") );
			}
		} else if( QUAT_PRECISION(SHP_PREC(shpp)) ){
			if( (SHP_PREC(shpp) & MACH_PREC_MASK) == PREC_SP )
				snprintf(MSG_STR,LLEN,", quaternion");
			else if( (SHP_PREC(shpp) & MACH_PREC_MASK) == PREC_DP )
				snprintf(MSG_STR,LLEN,", dblquat");
			else {
				assert( AERROR("unexpected quaternion machine precision!?") );
			}
		} else {
			snprintf(MSG_STR,LLEN,", real");
		}
		prt_msg_frag(MSG_STR);
	}

	if( SHP_COMPS(shpp) > 1 ){
		snprintf(MSG_STR,LLEN,", %d components",SHP_COMPS(shpp));
		prt_msg_frag(MSG_STR);
	}

	if( INTERLACED_SHAPE(shpp) ){
		snprintf(MSG_STR,LLEN,", interlaced");
		prt_msg_frag(MSG_STR);
	}

	prt_msg("");
}

void _dump_shape(QSP_ARG_DECL  Shape_Info *shpp)
{
	int i;

	snprintf(MSG_STR,LLEN,"shpp = 0x%"PRIxPTR,(uintptr_t)shpp);
	prt_msg(MSG_STR);

	describe_shape(shpp);
	snprintf(MSG_STR,LLEN,"prec = 0x%"PREC_FMT_X,SHP_PREC(shpp));
	prt_msg(MSG_STR);
	for(i=0;i<N_DIMENSIONS;i++){
		snprintf(MSG_STR,LLEN,"dim[%d] = %d (%d), ",i,SHP_TYPE_DIM(shpp,i),SHP_MACH_DIM(shpp,i));
		if( i == N_DIMENSIONS-1 )
			prt_msg(MSG_STR);
		else
			prt_msg_frag(MSG_STR);
	}

	snprintf(MSG_STR,LLEN,"n_elts = 0x%x (0x%x)",SHP_N_TYPE_ELTS(shpp),SHP_N_MACH_ELTS(shpp));
	prt_msg(MSG_STR);
	snprintf(MSG_STR,LLEN,"mindim = 0x%x",SHP_MINDIM(shpp));
	prt_msg(MSG_STR);
	snprintf(MSG_STR,LLEN,"maxdim = 0x%x",SHP_MAXDIM(shpp));
	prt_msg(MSG_STR);
	snprintf(MSG_STR,LLEN,"flags = 0x%llx",
			(long long unsigned int)SHP_FLAGS(shpp));
	prt_msg(MSG_STR);
	/*
	snprintf(MSG_STR,LLEN,"last_subi = 0x%x",SHP_LAST_SUBI(shpp));
	prt_msg(MSG_STR);
	*/
}

void _list_dobj(QSP_ARG_DECL  Data_Obj *dp)
{
	char string[128];

	if( OBJ_AREA(dp) == NULL )
		snprintf(string,128,"(no data area):%s", OBJ_NAME(dp) );
	else
		snprintf(string,128,"%s:%s", AREA_NAME( OBJ_AREA(dp) ), OBJ_NAME(dp) );
	snprintf(MSG_STR,LLEN,"%-40s",string);
	prt_msg_frag(MSG_STR);
	describe_shape(OBJ_SHAPE(dp) );

	/*
	if( dp->dt_extra != NULL ){
		snprintf(MSG_STR,LLEN,"Decl node has addr 0x%"PRIxPTR"\n",
			(uintptr_t)dp->dt_extra);
		prt_msg(MSG_STR);
	}
	*/
}

/*
 * Print out information about a data object
 */

struct _flagtbl {
	const char *flagname;
	shape_flag_t flagmask;
} flagtbl[N_DP_FLAGS]={
	{	"sequence",		DT_SEQUENCE		},
	{	"image",		DT_IMAGE		},
	{	"row vector",		DT_ROWVEC		},
	{	"column vector",	DT_COLVEC		},
	{	"scalar",		DT_SCALAR		},
	{	"hypersequence",	DT_HYPER_SEQ		},
	{	"unknown",		DT_UNKNOWN_SHAPE	},
	{	"string",		DT_STRING		},

	{	"char",			DT_CHAR			},
	{	"quaternion",		DT_QUAT			},
	{	"complex",		DT_COMPLEX		},
	{	"multidimensional",	DT_MULTIDIM		},

	{	"bitmap",		DT_BIT			},
	{	"not data owner",	DT_NO_DATA		},
	{	"zombie",		DT_ZOMBIE		},

	{	"contiguous",		DT_CONTIG		},
	{	"contiguity checked",	DT_CHECKED		},
	{	"evenly spaced",	DT_EVENLY		},

	{	"data aligned",		DT_ALIGNED		},
	{	"locked",		DT_LOCKED		},
	{	"assigned",		DT_ASSIGNED		},

	{	"temporary object",	DT_TEMP			},
	{	"void type",		DT_VOID			},
	{	"exported",		DT_EXPORTED		},
	{	"read-only",		DT_RDONLY		},
	{	"volatile",		DT_VOLATILE		},
	{	"interlaced",		DT_INTERLACED		},
	{	"obj_list",		DT_OBJ_LIST		},
	{	"static",		DT_STATIC		},
	{	"GL buffer",		DT_GL_BUF		},
	{	"GL buffer is mapped",	DT_BUF_MAPPED		},
	{	"shape checked",	DT_SHAPE_CHECKED	},
	{	"partially assigned",	DT_PARTIALLY_ASSIGNED	},
	{	"contiguous bitmap data",	DT_CONTIG_BITMAP_DATA	},
	{	"bitmap GPU info present",	DT_HAS_BITMAP_GPU_INFO	},
};

#define list_dp_flags(dp) _list_dp_flags(QSP_ARG  dp)

static void _list_dp_flags(QSP_ARG_DECL  Data_Obj *dp)
{
	int i;
	shape_flag_t flags;

	snprintf(MSG_STR,LLEN,"\tflags (0x%x):",(unsigned) OBJ_FLAGS(dp));
	prt_msg(MSG_STR);

	/* We keep a copy of flags, and clear each bit as we display its
	 * description...  then if there are any bits left at the end, we know
	 * something has been left out of the table.
	 */

	flags = OBJ_FLAGS(dp);
	for(i=0;i<N_DP_FLAGS;i++){
		assert( flagtbl[i].flagmask != 0 );
		if( flags & flagtbl[i].flagmask ){
			snprintf(MSG_STR,LLEN,"\t\t%s (0x%llx)",flagtbl[i].flagname,
				(long long unsigned int)flagtbl[i].flagmask);
			prt_msg(MSG_STR);

			flags &= ~flagtbl[i].flagmask;
		}
	}
	fflush(stdout);
	assert( flags == 0 );
}

void _show_obj_dimensions(QSP_ARG_DECL  Data_Obj *dp, Dimension_Set *dsp, Increment_Set *isp)
{
	int i;
	char dn[32];

	/* this makes the singularity check somewhat superfluous,
	 * but we'll leave it in for now...
	 */
#ifdef QUIP_DEBUG
if( debug & debug_data ){

	for(i=N_DIMENSIONS-1;i>OBJ_MAXDIM(dp);i--){
		strcpy(dn,dimension_name[i]);
		if( DIMENSION(dsp,i) > 1 ) strcat(dn,"s");
		snprintf(MSG_STR,LLEN,"\t%4u %12s   inc=%d", DIMENSION(dsp,i),
			dn, INCREMENT(isp,i));
		prt_msg(MSG_STR);
	}
}
#endif /* QUIP_DEBUG */

	/* show only the dimensions which are between mindim and maxdim */

	for(i=OBJ_MAXDIM(dp);i>=OBJ_MINDIM(dp);i--){
		strcpy(dn,dimension_name[i]);
		if( DIMENSION(dsp,i) > 1 ) strcat(dn,"s");
		snprintf(MSG_STR,LLEN,"\t%4u %12s   inc=%d", DIMENSION(dsp,i),
			dn, INCREMENT(isp,i));
		prt_msg(MSG_STR);
	}

#ifdef QUIP_DEBUG
if( debug & debug_data ){
	for(i=OBJ_MINDIM(dp)-1;i>=0;i--){
		strcpy(dn,dimension_name[i]);
		if( DIMENSION(dsp,i) > 1 ) strcat(dn,"s");
		snprintf(MSG_STR,LLEN,"\t%4u %12s   inc=%d", DIMENSION(dsp,i),
			dn, INCREMENT(isp,i));
		prt_msg(MSG_STR);
	}
}
#endif /* QUIP_DEBUG */

} // show_obj_dimensions

#define list_sizes(dp) _list_sizes(QSP_ARG  dp)

static void _list_sizes(QSP_ARG_DECL  Data_Obj *dp)
{
	snprintf(MSG_STR,LLEN,"\tmindim = %d, maxdim = %d",
		OBJ_MINDIM(dp),OBJ_MAXDIM(dp));
	prt_msg(MSG_STR);

	snprintf(MSG_STR,LLEN,"\trange_mindim = %d, range_maxdim = %d",
		OBJ_RANGE_MINDIM(dp),OBJ_RANGE_MAXDIM(dp));
	prt_msg(MSG_STR);

	show_obj_dimensions(dp,OBJ_TYPE_DIMS(dp),OBJ_TYPE_INCS(dp));
#ifdef QUIP_DEBUG
	if( debug & debug_data ){
		prt_msg("machine type dimensions:");
		show_obj_dimensions(dp,OBJ_MACH_DIMS(dp),OBJ_MACH_INCS(dp));
	}
#endif // QUIP_DEBUG
}

#define list_relatives(dp) _list_relatives(QSP_ARG  dp)

static void _list_relatives(QSP_ARG_DECL  Data_Obj *dp)
{
	if( OBJ_PARENT(dp) != NULL ){
		snprintf(MSG_STR,LLEN,"\tparent data object:  %s",
			OBJ_NAME(OBJ_PARENT( dp) ));
		prt_msg(MSG_STR);

		snprintf(MSG_STR,LLEN,"\tdata offset:  0x%x", OBJ_OFFSET(dp));
		prt_msg(MSG_STR);
	}
	if( OBJ_CHILDREN(dp) != NULL &&
		QLIST_HEAD( OBJ_CHILDREN(dp) ) != NULL ){

		Node *np;
		Data_Obj *dp2;

		snprintf(MSG_STR,LLEN,"\tsubobjects:");
		prt_msg(MSG_STR);
		np = QLIST_HEAD( OBJ_CHILDREN(dp) );
		while( np != NULL ){
			dp2=(Data_Obj *) NODE_DATA(np);
			snprintf(MSG_STR,LLEN,"\t\t%s",OBJ_NAME(dp2));
			prt_msg(MSG_STR);
			np=NODE_NEXT(np);
		}
	}
}

#define list_device(dp) _list_device(QSP_ARG  dp)

static void _list_device(QSP_ARG_DECL  Data_Obj *dp)
{
	snprintf(MSG_STR,LLEN,"\tdevice:  %s",PFDEV_NAME(OBJ_PFDEV(dp)));
	prt_msg(MSG_STR);
}

// Show the context of a data object.
// A "context" is a namespace, these can be stacked...
// This was introduced to support function-base scope
// in the expression language.
//
// Contexts can be pushed and popped, and the context for
// a subroutine is popped when we call another subroutine
// from the first.  If an object from the context of the first
// subroutine is passed by reference to the second, then its
// context will not be active, and so this will fail.
// This was originally written to be a CAUTIOUS error,
// but in fact this seems like the correct behavior.
// The alternative would be for the objects to keep a pointer
// to their context (BETTER SOLUTION, BUG), but
// for now it's not worth the trouble.
// Another solution would be to search ALL of the contexts, not
// just those currently on the stack...

#define show_dobj_context(dp) _show_dobj_context(QSP_ARG  dp)

static void _show_dobj_context(QSP_ARG_DECL  Data_Obj *dp)
{
	Item_Context *icp;
	Node *np;
	Item *ip;
	const char *cname="not found";

	/* objects don't keep a ptr to their context,
	 * so we search all the contexts until we find it.
	 *
	 * But subscripted objects won't show up,
	 * so if the object has a parent, list the context
	 * of the parent instead.
	 */

	if( OBJ_PARENT(dp) != NULL ){
		show_dobj_context(OBJ_PARENT( dp) );
		return;
	}

	/* BUG this is the list of the current context stack,
	 * not ALL the contexts!?
	 */
	np=QLIST_HEAD( LIST_OF_DOBJ_CONTEXTS );
	assert( np != NULL );

	while(np!=NULL){
		icp=(Item_Context *)NODE_DATA(np);
		/* can we search this context only? */
/*
snprintf(ERROR_STRING,LLEN,
"Searching context %s for object %s",CTX_NAME(icp),OBJ_NAME(dp));
advise(ERROR_STRING);
*/
		ip=FETCH_OBJ_FROM_CONTEXT( dp, icp );
		if( ((Data_Obj *)ip) == dp ){	/* found it! */
			cname=CTX_NAME(icp);
			goto show_context;
		}
		np=NODE_NEXT(np);
	}
	// fall-through if not found, use default string
	//
	// Why isn't it an error not to find an object's context?
	// Is it possible for an object to persist when its context is popped?
show_context:
	snprintf(MSG_STR,LLEN,"\titem context:  %s",cname);
	prt_msg(MSG_STR);
	return;

} // list context

#define list_data(dp) _list_data(QSP_ARG  dp)

static void _list_data(QSP_ARG_DECL  Data_Obj *dp)
{
	bitnum_t n;

	if( IS_BITMAP(dp) )
		n = bitmap_obj_word_count(dp);
	else
		n = OBJ_N_MACH_ELTS(dp);
// BUG fix format strings!!!
#ifdef BITNUM_64
	snprintf(MSG_STR,LLEN,"\t%llu %s element%s",n,OBJ_MACH_PREC_NAME(dp),n==1?"":"s");
#else // ! BITNUM_64
    snprintf(MSG_STR,LLEN,"\t%u %s element%s",n,OBJ_MACH_PREC_NAME(dp),n==1?"":"s");
#endif // ! BITNUM_64
	prt_msg(MSG_STR);

	snprintf(MSG_STR,LLEN,"\tdata at   0x%"PRIxPTR,(uintptr_t)OBJ_DATA_PTR(dp));
	prt_msg(MSG_STR);
	if( IS_BITMAP(dp) ){
#ifdef BITNUM_64
		snprintf(MSG_STR,LLEN,"\t\tbit0 = %llu",OBJ_BIT0(dp));
#else
		snprintf(MSG_STR,LLEN,"\t\tbit0 = %u",OBJ_BIT0(dp));
#endif
		prt_msg(MSG_STR);
	}
}

#ifdef QUIP_DEBUG

#define list_increments(dp) _list_increments(QSP_ARG  dp)

static void _list_increments(QSP_ARG_DECL  Data_Obj *dp)
{
	int i;

	for(i=0;i<N_DIMENSIONS;i++){
		snprintf(MSG_STR,LLEN,"\tincr[%d] = %d (%d)",i,OBJ_TYPE_INC(dp,i),OBJ_MACH_INC(dp,i));
		prt_msg(MSG_STR);
	}
}
#endif /* QUIP_DEBUG */

void _longlist(QSP_ARG_DECL  Data_Obj *dp)
{
	list_dobj(dp);
	list_device(dp);
	show_dobj_context(dp);
	list_sizes(dp);
	list_data(dp);
	list_relatives(dp);
	list_dp_flags(dp);
#ifdef QUIP_DEBUG
if( debug & debug_data ){
list_increments(dp);
dump_shape(OBJ_SHAPE(dp) );
}
#endif /* QUIP_DEBUG */
}

void info_area(QSP_ARG_DECL  Data_Area *ap)
{
	List *lp;
	Node *np;
	Data_Obj *dp;

	lp=dobj_list();
	if( lp==NULL ) return;
	np=QLIST_HEAD( lp );
	while( np != NULL ){
		dp = (Data_Obj *)NODE_DATA(np);
		if( OBJ_AREA(dp) == ap )
			list_dobj(dp);
		np=NODE_NEXT(np);
	}
}

void info_all_dps(SINGLE_QSP_ARG_DECL)
{
	List *lp;
	Node *np;

	lp=data_area_list();
	if( lp==NULL ) return;
	np=QLIST_HEAD( lp );
	while( np != NULL ){
		info_area( QSP_ARG  (Data_Area *) NODE_DATA(np) );
		np=NODE_NEXT(np);
	}
}

void _show_space_used(QSP_ARG_DECL  Data_Obj *dp)
{
	snprintf(MSG_STR,LLEN,"%s:\t\t0x%"PRIxPTR,OBJ_NAME(dp),(uintptr_t)OBJ_DATA_PTR(dp));
	prt_msg(MSG_STR);
}


