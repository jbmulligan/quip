#ifndef _ITEM_TYPE_H_
#define _ITEM_TYPE_H_

#include "quip_fwd.h"
#include "item_obj.h"
#include "item_prot.h"

struct qrb_node;
struct item_context;

typedef struct {
	struct qrb_node *		curr_n_p;
	struct qrb_node *		first_n_p;
	struct qrb_node *		last_n_p;
} rbtree_frag_match_info;

typedef struct {
	Node *		curr_np;
	Node *		first_np;
	Node *		last_np;
} list_frag_match_info;

typedef struct {
	List *			hti_lp;
} ht_frag_match_info;

// frag_match_info - 
// We store information for each partial match, including the fragment,
// the item context, and container-specific match info
//
// When there is a stack of contexts, we can have a stack of these?
// ans:  match_cycle ...

struct frag_match_info {
	Item				it;		// the partial string - stored in their own context
	struct item_context *		fmi_icp;	// the item context that these fragments refer to?
	u_long				fmi_item_serial;

	//int type;	// LIST_CONTAINER or RB_TREE_CONTAINER
	// Now we get container type from the context...

	void *				fmi_curr_p;
	void *				fmi_first_p;
	void *				fmi_last_p;

	union {
		rbtree_frag_match_info	rbti;
		list_frag_match_info	li;
		ht_frag_match_info	hti;
	} fmi_u;
} ;

#define FMI_STRING(fmi_p)	(fmi_p)->it.item_name
#define FMI_ITEM_SERIAL(fmi_p)		(fmi_p)->fmi_item_serial
#define SET_FMI_ITEM_SERIAL(fmi_p,c)	(fmi_p)->fmi_item_serial = c

#define FMI_CONTAINER(fmi_p)		CTX_CONTAINER(FMI_CTX(fmi_p))
#define FMI_CTX(fmi_p)			(fmi_p)->fmi_icp
#define SET_FMI_CTX(fmi_p,icp)		(fmi_p)->fmi_icp = icp

#define CURR_FRAG(fmi_p)	(fmi_p)->fmi_curr_p
#define FIRST_FRAG(fmi_p)	(fmi_p)->fmi_first_p
#define LAST_FRAG(fmi_p)	(fmi_p)->fmi_last_p

#define SET_CURR_FRAG(fmi_p,v)	(fmi_p)->fmi_curr_p = v
#define SET_FIRST_FRAG(fmi_p,v)	(fmi_p)->fmi_first_p = v
#define SET_LAST_FRAG(fmi_p,v)	(fmi_p)->fmi_last_p = v

#define CURR_RBT_FRAG(fmi_p)	(qrb_node *)CURR_FRAG(fmi_p)
#define FIRST_RBT_FRAG(fmi_p)	(qrb_node *)FIRST_FRAG(fmi_p)
#define LAST_RBT_FRAG(fmi_p)	(qrb_node *)LAST_FRAG(fmi_p)

#define CURR_LIST_FRAG(fmi_p)	(Node *)CURR_FRAG(fmi_p)
#define FIRST_LIST_FRAG(fmi_p)	(Node *)FIRST_FRAG(fmi_p)
#define LAST_LIST_FRAG(fmi_p)	(Node *)LAST_FRAG(fmi_p)

/*
#define SET_CURR_RBT_FRAG(fmi_p,v)	(fmi_p)->fmi_u.rbti.curr_n_p = v
#define SET_FIRST_RBT_FRAG(fmi_p,v)	(fmi_p)->fmi_u.rbti.first_n_p = v
#define SET_LAST_RBT_FRAG(fmi_p,v)	(fmi_p)->fmi_u.rbti.last_n_p = v

#define SET_CURR_LIST_FRAG(fmi_p,v)	(fmi_p)->fmi_u.li.curr_np = v
#define SET_FIRST_LIST_FRAG(fmi_p,v)	(fmi_p)->fmi_u.li.first_np = v
#define SET_LAST_LIST_FRAG(fmi_p,v)	(fmi_p)->fmi_u.li.last_np = v

#define SET_CURR_HT_FRAG(fmi_p,v)	(fmi_p)->fmi_u.hti.hti_li.curr_np = v
#define SET_FIRST_HT_FRAG(fmi_p,v)	(fmi_p)->fmi_u.hti.hti_li.first_np = v
#define SET_LAST_HT_FRAG(fmi_p,v)	(fmi_p)->fmi_u.hti.hti_li.last_np = v
*/

#define FMI_HT_LIST(fmp_p)	(fmi_p)->fmi_u.hti.hti_lp

// What is a match_cycle?
//
struct match_cycle {
	Item it;				// name of the fragment
	Item_Type *	mc_itp;			// the relevant item type
	u_long		mc_stack_serial;	// need to redo if doesn't match itci
	u_long		mc_item_serial;		// need to redo at least one fmp if doesn't match itci
	List *		mc_fmi_lp;		// list of frag_match_info's
	Node *		mc_curr_np;		// points to a node in fmi_lp, for cycling
};

#define MATCH_CYCLE_STRING(mc_p)	(mc_p)->it.item_name

#define MATCH_CYCLE_LIST(mc_p)		(mc_p)->mc_fmi_lp
#define SET_MATCH_CYCLE_LIST(mc_p,lp)	(mc_p)->mc_fmi_lp = lp

#define MATCH_CYCLE_CURR_NODE(mc_p)		(mc_p)->mc_curr_np
#define SET_MATCH_CYCLE_CURR_NODE(mc_p,np)	(mc_p)->mc_curr_np = np

#define MATCH_CYCLE_IT(mc_p)		(mc_p)->mc_itp
#define SET_MATCH_CYCLE_IT(mc_p,itp)	(mc_p)->mc_itp = itp

#define MC_STACK_SERIAL(mc_p)		(mc_p)->mc_stack_serial
#define MC_ITEM_SERIAL(mc_p)		(mc_p)->mc_stack_serial
#define SET_MC_STACK_SERIAL(mc_p,c)	(mc_p)->mc_stack_serial = c
#define SET_MC_ITEM_SERIAL(mc_p,c)	(mc_p)->mc_stack_serial = c

struct item_context {
	Item			ic_item;

	// We used to use a "dictionary" to store the items; traditionally,
	// this has been a hash table, but to support partial name
	// lookup (for more efficient completion), we might prefer
	// to use a red-black tree...
	Container *		ic_cnt_p;	// the primary container
	u_long			ic_item_serial;	// count changes to the container

	List *			ic_lp;		// list of items in this context
	u_long			ic_list_serial;	// when list was last updated

	Item_Type *		ic_itp;		// points to the owner of this context
	struct item_context *	ic_frag_icp;	// fragment match database just for this context
} ;

#define CTX_IS_NEEDY(icp)	( (icp)->ic_list_serial != (icp)->ic_item_serial )

/* Item_Context */
#define CTX_NAME(icp)			(icp)->ic_item.item_name
//#define CONTEXT_NAME(icp)		(icp)->ic_item.item_name
//#define CTX_DICT(icp)			(icp)->ic_dict_p
#define CTX_CONTAINER(icp)		(icp)->ic_cnt_p
#define CTX_FLAGS(icp)			(icp)->ic_flags
#define CTX_IT(icp)			(icp)->ic_itp

#define CTX_ITEM_LIST(icp)		(icp)->ic_lp
#define SET_CTX_ITEM_LIST(icp,lp)	(icp)->ic_lp = lp

#define SET_CTX_NAME(icp,s)		(icp)->ic_item.item_name = s
#define SET_CTX_IT(icp,itp)		(icp)->ic_itp = itp
#define SET_CTX_FLAGS(icp,f)		(icp)->ic_flags = f
//#define SET_CTX_DICT(icp,dict_p)	(icp)->ic_dict_p = dict_p
#define SET_CTX_CONTAINER(icp,cnt_p)	(icp)->ic_cnt_p = cnt_p
#define SET_CTX_FLAG_BITS(icp,f)	(icp)->ic_flags |= f
#define CLEAR_CTX_FLAG_BITS(icp,f)	(icp)->ic_flags &= ~(f)

#define CTX_FRAG_ICP(icp)		(icp)->ic_frag_icp
#define SET_CTX_FRAG_ICP(icp,icp2)	(icp)->ic_frag_icp = icp2

#define CTX_ITEM_SERIAL(icp)		(icp)->ic_item_serial
#define SET_CTX_ITEM_SERIAL(icp,c)	(icp)->ic_item_serial = c
#define INC_CTX_ITEM_SERIAL(icp)	(icp)->ic_item_serial ++

#define CTX_LIST_SERIAL(icp)		(icp)->ic_list_serial
#define SET_CTX_LIST_SERIAL(icp,c)	(icp)->ic_list_serial = c
#define INC_CTX_LIST_SERIAL(icp)	(icp)->ic_list_serial ++

#define NEW_ITEM_CONTEXT(icp)	icp=((Item_Context *)getbuf(sizeof(Item_Context)))

#ifdef THREAD_SAFE_QUERY
#define MAX_QUERY_STACKS	51	// why do we need to have a limit?
					// Because we have fixed size arrays
					// of per-query stack ptrs...
#else // ! THREAD_SAFE_QUERY
#define MAX_QUERY_STACKS	1
#endif // ! THREAD_SAFE_QUERY

// This struct contains all of the things that we need to have copied
// for each thread...
// The main thing is the context stack

typedef struct item_type_context_info {
	Stack *			itci_context_stack;	// need to have one per thread
	u_long			itci_stack_serial;	// increment this when the stack is changed (push/pop)
	u_long			itci_items_serial;	// increment this when any context is changed

	int			itci_flags;

	List *			itci_item_lp;		// list of all of the items -
	u_long			itci_list_items_serial;	// update when list is updated
	u_long			itci_list_stack_serial;	// update when list is updated

	Item_Context *		itci_icp;		// current context
	Match_Cycle *		itci_mc_p;		// why remember one???
	struct item_context *	itci_mc_icp;		// match cycle database for this context stack
} Item_Type_Context_Info; // ITCI

// Flag bits
#define ITCI_CTX_RESTRICTED_FLAG		1

#define NEEDS_NEW_LIST(itp)	(											\
					(ITCI_LIST_ITEMS_SERIAL(THIS_ITCI(itp)) != ITCI_ITEMS_SERIAL(THIS_ITCI(itp)))	\
					|| 										\
					(ITCI_LIST_STACK_SERIAL(THIS_ITCI(itp)) != ITCI_STACK_SERIAL(THIS_ITCI(itp)))	\
				)

#define ITCI_ITEMS_LIST(itci_p)			(itci_p)->itci_item_lp
#define SET_ITCI_ITEMS_LIST(itci_p,lp)		(itci_p)->itci_item_lp = lp

#define ITCI_CSTK(itci_p)			(itci_p)->itci_context_stack
#define SET_ITCI_CSTK(itci_p,sp)		(itci_p)->itci_context_stack = sp

#define ITCI_CTX(itci_p)			(itci_p)->itci_icp
#define SET_ITCI_CTX(itci_p,icp)		(itci_p)->itci_icp = icp

#define ITCI_FLAGS(itci_p)			(itci_p)->itci_flags
#define SET_ITCI_FLAGS(itci_p,val)		(itci_p)->itci_flags = val
#define SET_ITCI_FLAG_BITS(itci_p,val)		(itci_p)->itci_flags |= val
#define CLEAR_ITCI_FLAG_BITS(itci_p,val)	(itci_p)->itci_flags &= ~(val)

#define ITCI_CTX_RESTRICTED(itci_p)		(ITCI_FLAGS(itci_p) & ITCI_CTX_RESTRICTED_FLAG)

#define ITCI_MATCH_CYCLE(itci_p)		(itci_p)->itci_mc_p
#define SET_ITCI_MATCH_CYCLE(itci_p,p)		(itci_p)->itci_mc_p = p

#define ITCI_MC_CONTEXT(itci_p)			(itci_p)->itci_mc_icp
#define SET_ITCI_MC_CONTEXT(itci_p,icp)		(itci_p)->itci_mc_icp = icp

#define INC_ITCI_ITEMS_CHANGE_COUNT(itci_p)	(itci_p)->itci_items_serial ++
#define INC_ITCI_STACK_CHANGE_COUNT(itci_p)	(itci_p)->itci_stack_serial ++

#define ITCI_ITEMS_SERIAL(itci_p)		(itci_p)->itci_items_serial
#define SET_ITCI_ITEMS_SERIAL(itci_p,c)		(itci_p)->itci_items_serial = c

#define ITCI_STACK_SERIAL(itci_p)		(itci_p)->itci_stack_serial
#define SET_ITCI_STACK_SERIAL(itci_p,c)		(itci_p)->itci_stack_serial = c

#define ITCI_LIST_ITEMS_SERIAL(itci_p)		(itci_p)->itci_list_items_serial
#define ITCI_LIST_STACK_SERIAL(itci_p)		(itci_p)->itci_list_stack_serial
#define SET_ITCI_LIST_ITEMS_SERIAL(itci_p,c)	(itci_p)->itci_list_items_serial = c
#define SET_ITCI_LIST_STACK_SERIAL(itci_p,c)	(itci_p)->itci_list_stack_serial = c


struct item_type {
	Item		it_item;
	int		it_flags;	// don't need?
	List *		it_free_lp;
	int		it_container_type;
	void		(*it_del_method)(QSP_ARG_DECL  Item *);
	List *		it_class_lp;

	// If we can have multiple interpreter threads, then each thread
	// needs its own context stack...
	//
	// WHY?  Well, if we have multiple threads running the expression language,
	// then they will be pushing and popping contexts...
	// But for many things, like variables and macros,
	// we don't really need multiple contexts???

#ifdef THREAD_SAFE_QUERY
#ifdef HAVE_PTHREADS
	pthread_mutex_t	it_mutex;
#endif /* HAVE_PTHREADS */
#endif /* ! THREAD_SAFE_QUERY */

	Item_Type_Context_Info	it_itci[MAX_QUERY_STACKS];

	//Frag_Match_Info *	it_fmi_p;			// only one???
};

#define it_name	it_item.item_name
#define ITEM_TYPE_NAME(itp)	((itp)->it_name)

// "Restricted" means that only the top context will be searched.
// This feature was introduced in the expression language to support
// scoping of variables to subroutines.  To allow multiple instances
// to run in parallel, this has to be a per-thread flag...
//#define RESTRICTED	8	// what does restricted mean???


/* Item_Type */
#define IT_NAME(itp)			(itp)->it_item.item_name
#define IT_FREE_LIST(itp)		(itp)->it_free_lp

#define IT_CLASS_LIST(itp)		(itp)->it_class_lp
#define IT_CONTAINER_TYPE(itp)		(itp)->it_container_type
#define IT_DEL_METHOD(itp)		(itp)->it_del_method
#define SET_IT_DEL_METHOD(itp,f)	(itp)->it_del_method = f
#define SET_IT_FREE_LIST(itp,lp)	(itp)->it_free_lp = lp
#define SET_IT_CTX_IT(itp,citp)		(itp)->it_ctx_itp = citp
#define SET_IT_CLASS_LIST(itp,lp)	(itp)->it_class_lp = lp
#define SET_IT_CONTAINER_TYPE(itp,t)	(itp)->it_container_type = t

#define IT_MATCH_CYCLE(itp)		ITCI_MATCH_CYCLE( THIS_ITCI(itp) )
#define SET_IT_MATCH_CYCLE(itp,mc_p)	SET_ITCI_MATCH_CYCLE( THIS_ITCI(itp), mc_p )

#define IT_MC_CONTEXT(itp)		ITCI_MC_CONTEXT( THIS_ITCI(itp) )
#define SET_IT_MC_CONTEXT(itp,icp)	SET_ITCI_MC_CONTEXT( THIS_ITCI(itp), icp )

#define ITEMS_CHANGE_COUNT(itp)		ITCI_ITEMS_SERIAL( THIS_ITCI(itp) )
#define STACK_CHANGE_COUNT(itp)		ITCI_STACK_SERIAL( THIS_ITCI(itp) )
#define INC_ITEMS_CHANGE_COUNT(itp)	INC_ITCI_ITEMS_CHANGE_COUNT( THIS_ITCI(itp) )
#define INC_STACK_CHANGE_COUNT(itp)	INC_ITCI_STACK_CHANGE_COUNT( THIS_ITCI(itp) )

// The context field may be null, so we have a function to do the checking
// and initialization if necessary...
extern Item_Context *_current_context(QSP_ARG_DECL  Item_Type *itp);
#define current_context(itp)		_current_context(QSP_ARG  itp)
#define SET_CURRENT_CONTEXT(itp,icp)	SET_ITCI_CTX(THIS_ITCI(itp),icp)
#define ITCI_AT_INDEX(itp,idx)		(&((itp)->it_itci[idx]))
#ifdef THREAD_SAFE_QUERY
#define THIS_ITCI(itp)			ITCI_AT_INDEX(itp,_QS_SERIAL(THIS_QSP))
#else // ! THREAD_SAFE_QUERY
#define THIS_ITCI(itp)			ITCI_AT_INDEX(itp,0)
#endif // ! THREAD_SAFE_QUERY

// this is the list of contexts,
// not the list of items in a context
#define LIST_OF_CONTEXTS(itp)		_context_stack(QSP_ARG  itp)
#define SET_LIST_OF_CONTEXTS(itp,lp)	SET_ITCI_CSTK( THIS_ITCI(itp), lp )

#define FIRST_LIST_OF_CONTEXTS(itp)	ITCI_CSTK( ITCI_AT_INDEX(itp,0) )

#define IS_RESTRICTED(itp)		ITCI_CTX_RESTRICTED( THIS_ITCI(itp) )

// We use ITCI_AT_INDEX instead of THIS_ITCI() when we can't see the qsp internals...
#define RESTRICT_ITEM_CONTEXT(itp,yesno)								\
	{												\
		if( yesno ) SET_ITCI_FLAG_BITS(ITCI_AT_INDEX(itp,QS_SERIAL),ITCI_CTX_RESTRICTED_FLAG);	\
		else CLEAR_ITCI_FLAG_BITS(ITCI_AT_INDEX(itp,QS_SERIAL),ITCI_CTX_RESTRICTED_FLAG);	\
	}


#define SET_IT_NAME(itp,s)		(itp)->it_item.item_name=s


struct item_class {
	Item		icl_item;
	List *		icl_lp;		// list of itp's
	int		icl_flags;
} ;

/* Item_Class */
#define CL_NAME(icp)			(icp)->icl_item.item_name
#define SET_CL_FLAG_BITS(iclp,f)	(iclp)->icl_flags |= f

// flag bits
#define NEED_CLASS_CHOICES	1	// BUG item_type items are per-thread, so class choices should be too!?

struct member_info {
	Item_Type *	mi_itp;
	void *		mi_data;
	Item *		(*mi_lookup)(QSP_ARG_DECL  const char *);
} ;

#define MBR_DATA(mp)	(mp)->mi_data

#define ITEM_INTERFACE_CONTAINER(stem,type)

#define ITEM_INTERFACE_DECLARATIONS(type,stem,container_type)	IIF_DECLS(type,stem,,container_type)

#define ITEM_INTERFACE_DECLARATIONS_STATIC(type,stem,container_type)		\
						IIF_DECLS(type,stem,static,container_type)

#define IIF_DECLS(type,stem,storage,container_type)		\
								\
static Item_Type *stem##_itp=NULL;				\
storage ITEM_INIT_FUNC(type,stem,container_type)		\
storage ITEM_NEW_FUNC(type,stem)				\
storage ITEM_CHECK_FUNC(type,stem)				\
storage ITEM_GET_FUNC(type,stem)				\
storage ITEM_LIST_FUNC(type,stem)				\
storage ITEM_ENUM_FUNC(type,stem)				\
storage ITEM_DEL_FUNC(type,stem)				\
storage ITEM_PICK_FUNC(type,stem)


// BUG we should use new_item to take advantage of item free lists...

#define ITEM_NEW_FUNC(type,stem)				\
								\
type *_new_##stem(QSP_ARG_DECL  const char *name)		\
{								\
	type * stem##_p;					\
								\
	assert(name!=NULL);					\
								\
	if( *name == 0 ){					\
		WARN("Can't create " #stem " item with null name!?");	\
		return NULL;					\
	}							\
	if( stem##_itp == NULL )			\
		_init_##stem##s(SINGLE_QSP_ARG);			\
								\
	stem##_p = (type *) new_item(stem##_itp, name, sizeof(type));		\
	if( stem##_p == NULL ){					\
		snprintf(ERROR_STRING,LLEN,			\
	"Error creating item %s!?",name);			\
		WARN(ERROR_STRING);				\
		/* BUG release name here */			\
	}							\
	return stem##_p;					\
}

#define ITEM_INIT_FUNC(type,stem,container_type)		\
								\
void _init_##stem##s(SINGLE_QSP_ARG_DECL)			\
{								\
	stem##_itp = new_item_type(#type, container_type);	\
}

#define ITEM_CHECK_FUNC(type,stem)				\
								\
type *_##stem##_of(QSP_ARG_DECL  const char *name)			\
{								\
	if( stem##_itp == NULL )			\
		_init_##stem##s(SINGLE_QSP_ARG);			\
	return (type *) item_of(stem##_itp, name );	\
}

#define ITEM_GET_FUNC(type,stem)				\
								\
type *_get_##stem(QSP_ARG_DECL  const char *name)		\
{								\
	if( stem##_itp == NULL )			\
		_init_##stem##s(SINGLE_QSP_ARG);			\
	return (type *) get_item(stem##_itp, name );	\
}

#define ITEM_PICK_FUNC(type,stem)				\
								\
type *_pick_##stem(QSP_ARG_DECL  const char *pmpt)		\
{								\
	if( stem##_itp == NULL )			\
		_init_##stem##s(SINGLE_QSP_ARG);			\
	return (type *) pick_item(stem##_itp, pmpt );	\
}

#define ITEM_LIST_FUNC(type,stem)				\
								\
void _list_##stem##s(QSP_ARG_DECL  FILE *fp)			\
{								\
	if( stem##_itp == NULL )			\
		_init_##stem##s(SINGLE_QSP_ARG);			\
	list_items(stem##_itp, fp );			\
}

#define ITEM_ENUM_FUNC(type,stem)				\
								\
List * _##stem##_list(SINGLE_QSP_ARG_DECL)				\
{								\
	if( stem##_itp == NULL )			\
		_init_##stem##s(SINGLE_QSP_ARG);			\
	return item_list(stem##_itp);			\
}

#define ITEM_DEL_FUNC(type,stem)				\
								\
void _del_##stem(QSP_ARG_DECL  type *ip)				\
{								\
	del_item(stem##_itp, (Item *)ip);		\
}

extern ITEM_INIT_PROT(Item_Type,ittyp)
extern int _add_item( QSP_ARG_DECL  Item_Type *itp, void *ip );
extern int _remove_from_item_free_list(QSP_ARG_DECL  Item_Type *itp, void *ip);
extern const char *_find_partial_match( QSP_ARG_DECL  Item_Type *itp, const char *s );
extern List *_alpha_sort(QSP_ARG_DECL  List *lp);
extern List *_context_stack(QSP_ARG_DECL  Item_Type *itp);

#define add_item(itp,ip)	_add_item(QSP_ARG  itp, ip )
#define remove_from_item_free_list(itp,ip)	_remove_from_item_free_list(QSP_ARG  itp, ip )
#define find_partial_match(itp,frag)	_find_partial_match(QSP_ARG  itp, frag )
#define alpha_sort(lp)	_alpha_sort(QSP_ARG  lp)
#define context_stack(itp)	_context_stack(QSP_ARG  itp)

#endif /* ! _ITEM_TYPE_H_ */

