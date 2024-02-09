#include "quip_config.h"

/*
 * Data areas were introduced back when the sky warrior had
 * to have it's objects living in special VME memory.
 * The construct is pretty much obsolete nowadays, but it
 * is not worth the trouble to get rid of it at present...
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "quip_prot.h"
#include "getbuf.h"
#include "stack.h"
#include "data_obj.h"
#include "shape_info.h"
#include "debug.h"
#include "platform.h"

Data_Area *curr_ap=NULL, *ram_area_p=NULL;

static int n_areas=0;

ITEM_INTERFACE_DECLARATIONS(Data_Area,data_area,0)

// BUG not thread-safe...
static Stack *data_area_stack_p=NULL;

// set_data_area needs to replace the top of the stack, so we can restore it if something
// else is pushed and popped...

void set_data_area(Data_Area *ap)
{
	curr_ap = ap;	// set_data_area
//fprintf(stderr,"Current data area set to %s\n",AREA_NAME(curr_ap));
}

void push_data_area(Data_Area *ap)
{
	if( data_area_stack_p == NULL )
		data_area_stack_p = new_stack();

	/* We used to check for stack to big here, mainly because
	 * the "stack" was a fixed size array...
	 * Here is the old comment:
	 * This test is not an assertion (CAUTIOUS)
	 * because we could encounter
	 * infinite recursion in a script, or something similar...
	 */

//fprintf(stderr,"push_data_area:  pushing %s\n",AREA_NAME(curr_ap));
	push_item(data_area_stack_p,(Item *)curr_ap);
	curr_ap=ap;	// push_data_area
}

void pop_data_area(void)
{
	Data_Area *ap;

	assert( data_area_stack_p != NULL );

	ap = (Data_Area *) pop_item(data_area_stack_p);

	// This used to be an error, but it is OK to push and pop a data
	// area before any area has been set...
	// assert( ap != NULL );

//fprintf(stderr,"pop_data_area:  popped %s\n",AREA_NAME(ap));
	curr_ap = ap;	// pop_data_area
}

void a_init(void)
{
	if( n_areas==0 ){
		curr_ap=NULL;
	}
}

Data_Area *_default_data_area(SINGLE_QSP_ARG_DECL)
{
	if( curr_ap == NULL ) dataobj_init();
	assert(curr_ap!=NULL);
	return(curr_ap);
}

// Now use pf_area_init please.

#define area_init( name, buffer, siz, n_chunks, flags ) _area_init( QSP_ARG  name, buffer, siz, n_chunks, flags )

static Data_Area *_area_init( QSP_ARG_DECL  const char *name, u_char *buffer, uint32_t siz, unsigned int n_chunks, uint32_t flags )
{
	Data_Area *ap;

	a_init();

	ap = new_data_area(name);
	if( ap == NULL ) return(ap);
	//ap = [[DataArea alloc] initWithName : name ];

	if( buffer == NULL ){
		ap->da_ma_p = NULL;
	} else {
		ap->da_ma_p =
			(Memory_Area *) getbuf(sizeof(Memory_Area));
		ap->da_base = buffer;
		ap->da_memsiz=siz;
		ap->da_memfree=siz;

	/* n_chunks can be less than the number of objects we need as
	 * long as there's not too much fragmentation...
	 */
		freeinit(&ap->da_freelist,(count_t)n_chunks,siz);
	}

	ap->da_flags = flags;

	ap->da_dp=NULL;
	// Should we have the platform device first???
	ap->da_pdp=NULL;

	return(ap);

}	// end area_init

Data_Area *			/**/
_pf_area_init( QSP_ARG_DECL  const char *name, u_char *buffer, uint32_t siz, int n_chunks, uint32_t flags,   Platform_Device *pdp )
{
	Data_Area *ap;

	ap = area_init(name, buffer, siz, n_chunks, flags );
	if( ap == NULL ) return ap;

	SET_AREA_PFDEV( ap , pdp );

	return ap;
}

Data_Area *
_new_area( QSP_ARG_DECL  const char *s, uint32_t siz, unsigned int n )
{
	u_char *buf;
	Data_Area *ap;

	buf=(u_char *)getbuf(siz);

	// our own implementation of getbuf never returns a failure
	// code; but we sometimes compile with getbuf #define'd to be malloc
	assert( buf != NULL );
	assert( n > 0 );

	ap=area_init(s,buf,siz,n,DA_RAM);
	if( ap==NULL ) givbuf((char *)buf);

	return(ap);
}

void _data_area_info(QSP_ARG_DECL  Data_Area *ap )
{
	snprintf(MSG_STR,LLEN,"AREA %s:%-30s",PFDEV_NAME(AREA_PFDEV(ap)),ap->da_name);
	prt_msg_frag(MSG_STR);
	switch(ap->da_flags & DA_TYPE_MASK ){
		case DA_RAM:
			prt_msg("   ram");
			break;
		// BUG - cuda memory should per-device!
		case DA_CUDA_GLOBAL:
			prt_msg("   CUDA device global memory");
			break;
		case DA_CUDA_HOST:
			prt_msg("   CUDA device host memory");
			break;
		case DA_CUDA_HOST_MAPPED:
			prt_msg("   CUDA device host mapped  memory");
			break;
		case DA_OCL_GLOBAL:
			prt_msg("   OpenCL device global memory");
			break;
		case DA_OCL_HOST:
			prt_msg("   OpenCL device host memory");
			break;
		case DA_OCL_HOST_MAPPED:
			prt_msg("   OpenCL device host mapped  memory");
			break;
#ifdef DA_CUDA_CONSTANT
		case DA_CUDA_CONSTANT:
			prt_msg("   CUDA device constant memory");
			break;
#endif /* defined DA_CUDA_CONSTANT */

		default:
			assert( AERROR("Bad data area type!?") );
			break;
	}
}

void _list_area(QSP_ARG_DECL  Data_Area *ap )
{
	snprintf(MSG_STR,LLEN,"AREA %-10s",ap->da_name);
	prt_msg_frag(MSG_STR);
	if( ap->da_memsiz != 0 ){
		snprintf(MSG_STR,LLEN,"   0x%-10x total bytes, 0x%-10x remaining",
			ap->da_memsiz,
			ap->da_memfree);
		prt_msg(MSG_STR);
	} else prt_msg("");
}

/* This const business causes a lot of warnings if not done right...
 * The way it is here doesn't complain on the newest gcc, but complains
 * on a slightly older version, while taking the const out of the typedef
 * and putting down in the cast shuts up the older version but makes
 * the newer one complain...  We assume the newer one is correct, this
 * seems to make sense as best as I can figure out the semantics
 * of the const keyword...  jbm
 */

typedef const Data_Obj * DataPtr;

int dp_addr_cmp( const void *dpp1, const void *dpp2 )
{
	const Data_Obj *dp1,*dp2;

	dp1= *(const DataPtr *)dpp1;

	dp2= *(const DataPtr *)dpp2;

	if( OBJ_DATA_PTR(dp1) > OBJ_DATA_PTR(dp2) ) return(1);
	else if( OBJ_DATA_PTR(dp1) < OBJ_DATA_PTR(dp2) ) return(-1);
	else return(0);
}

void _show_area_space( QSP_ARG_DECL  Data_Area *ap )
{
	List *lp;
	Node *np;
	Data_Obj **dp_list;
	Data_Obj *dp;
	int n,i;

	if( data_area_itp == NULL ) init_data_areas();

	lp=dobj_list();
	if( lp==NULL ) return;

	n=eltcount(lp);
	if( n == 0 ){
		snprintf(MSG_STR,LLEN,"Area %s has no objects.",ap->da_name);
		prt_msg(MSG_STR);
	} else {
		snprintf(MSG_STR,LLEN,"Area %s:  %d object%s:",ap->da_name,n,n==1?"":"s");
		prt_msg(MSG_STR);
	}

	dp_list = (Data_Obj **) getbuf( n * sizeof(Data_Obj *) );
	if( dp_list == NULL ) {
		mem_err("show_area_space");
		return;	// NOTREACHED - silence static analyzer
	}
	
	np=QLIST_HEAD(lp);
	i=0;
	while( np != NULL ){
		dp = (Data_Obj *) np->n_data;
		if( OBJ_AREA(dp) == ap ){
			dp_list[i] = dp;
			i++;
		}
		np=np->n_next;
	}
	if( i == 0 ) return;

#ifdef PC
	qsort((void *)dp_list,(size_t)i,sizeof(Data_Obj *),dp_addr_cmp);
#else /* PC */
	qsort((char *)dp_list,(int)i,sizeof(Data_Obj *),dp_addr_cmp);
#endif /* PC */

	for(i=0;i<n;i++)
		show_space_used(dp_list[i]);
	
	givbuf(dp_list);
}



/* init_scratch_scalar - make up a two scalar objects
 * to get the return values of vmaxv & vminv
 */

#define MAX_NAME_LEN	80

#define init_scratch_scalar(ap)	_init_scratch_scalar(QSP_ARG  ap)

static void _init_scratch_scalar(QSP_ARG_DECL  Data_Area *ap)
{
	char name[MAX_NAME_LEN];

	assert( AREA_SCALAR_OBJ(ap) == NULL );

	if( strlen(AREA_NAME(ap))+strlen(".scratch_scalar")+1 > MAX_NAME_LEN )
		error1("init_scratch_scalar:  need to increase MAX_NAME_LEN!?");

	/* Can we make this not hashed??? */

	// The scalar should be the largest type...
	// Do we need quaternion, complex, etc?

	//set_data_area(ap);	// init_scratch_scalar
	push_data_area(ap);	// init_scratch_scalar
	snprintf(name,MAX_NAME_LEN,"%s.scratch_scalar",AREA_NAME(ap));
	ap->da_dp = mk_scalar(name, PREC_FOR_CODE(PREC_DP) );
	pop_data_area();	// init_scratch_scalar
}

Data_Obj *_area_scalar(QSP_ARG_DECL  Data_Area *ap)
{
	if( AREA_SCALAR_OBJ(ap) == NULL )
		init_scratch_scalar(ap);
	return AREA_SCALAR_OBJ(ap);
}


