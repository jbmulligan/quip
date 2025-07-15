#include "quip_config.h"

#include <stdio.h>	// NULL

#include "quip_prot.h"
#include "query_stack.h"
#include "item_type.h"
#include "item_prot.h"
#include "node.h"
#include "warn.h"
#include "getbuf.h"
#include "debug.h"

int quip_verbose=0;

#ifdef QUIP_DEBUG
/*
 * Because the debug modules use the item package, subsystems
 * which support this (getbuf, hash) need to have their bit masks
 * pre-defined...
 */


debug_flag_t debug=0;
static int  n_debug_modules=(-1);

/* local prototypes */
#define init_dbm()	_init_dbm(SINGLE_QSP_ARG)

static Debug_Module *	add_auto_module(QSP_ARG_DECL  const char *, debug_flag_t mask);

static Item_Type *debug_itp=NULL;

ITEM_INIT_FUNC(Debug_Module,debug,0)
ITEM_NEW_FUNC(Debug_Module,debug)
ITEM_CHECK_FUNC(Debug_Module,debug)
ITEM_GET_FUNC(Debug_Module,debug)
ITEM_PICK_FUNC(Debug_Module,debug)
ITEM_LIST_FUNC(Debug_Module,debug)

static int bit_count(debug_flag_t word)
{
	int i,n_total_bits;
	debug_flag_t mask;
	int n_set_bits;

	n_total_bits=sizeof(mask)*8;

	n_set_bits=0;
	for(i=0;i<n_total_bits;i++){
		mask = 1 << i;
		if( word & mask )
			n_set_bits++;
	}
	return n_set_bits;
}

static Debug_Module * add_auto_module(QSP_ARG_DECL  const char *name, debug_flag_t mask)
{
	Debug_Module *dmp;

	/* user of savestr() eliminates a warning, but wastes some memory... */
	//auto_dbm_tbl[n_debug_modules].db_name = (char *) name;

	dmp = new_debug(name);
	assert( dmp != NULL );

	SET_DEBUG_MASK(dmp, mask );
	SET_DEBUG_FLAGS(dmp, DEBUG_SET);	// default

	// Don't increase the number of modules for the special choices yes/no all/none
	if( bit_count(mask) == 1 ){
		if( mask != (1<<n_debug_modules) ){
			snprintf(ERROR_STRING,LLEN,
		"Debug module %s (mask = 0x%x) added out of sequence (n_debug_modules = %d)!?",
				name,mask,n_debug_modules);
			warn(ERROR_STRING);
		}
		n_debug_modules++;
	}

	return dmp;
}

#define AUTO_MODULE( code, mask )				\
								\
	dmp = add_auto_module(QSP_ARG  code, mask );		\
	if( dmp == NULL ){					\
		error1("init_dbm:  Error adding automatic debug module!?");	\
		return;						\
	}

static void _init_dbm(SINGLE_QSP_ARG_DECL)
{
	Debug_Module *dmp;

	n_debug_modules=0;

#ifdef QUIP_DEBUG
	/* These have to be called in the correct order...
	 * To make sure everything is consistent, we define
	 * these in debug.h
	 */

	/* We enclosed this in ifdef USE_GETBUF, but that messed up the CAUTIOUS check below,
	 * so we just leave it here even when we're using malloc.
	 */
	AUTO_MODULE(  AUTO_MODULE_GETBUF,	GETBUF_DEBUG_MASK	)

	AUTO_MODULE(  AUTO_MODULE_FREEL,	FREEL_DEBUG_MASK	)
	AUTO_MODULE(  AUTO_MODULE_HASH,		HASH_DEBUG_MASK		)
	AUTO_MODULE(  AUTO_MODULE_DICT,		DICT_DEBUG_MASK		)
	AUTO_MODULE(  AUTO_MODULE_NODES,	NODE_DEBUG_MASK		)
	AUTO_MODULE(  AUTO_MODULE_ITEMS,	ITEM_DEBUG_MASK		)
	AUTO_MODULE(  AUTO_MODULE_CONTEXTS,	CTX_DEBUG_MASK		)

	assert( n_debug_modules == N_AUTO_DEBUG_MODULES );
	// what about ios return?  will abort do it right away?
#endif /* QUIP_DEBUG */

	AUTO_MODULE(  "yes",			ALL_DEBUG_MODULES	)
	AUTO_MODULE(  "no",			ALL_DEBUG_MODULES	)
	CLEAR_DEBUG_FLAG_BITS(dmp,DEBUG_SET);

	AUTO_MODULE(  "all",			ALL_DEBUG_MODULES	)
	AUTO_MODULE(  "none",			ALL_DEBUG_MODULES	)
	CLEAR_DEBUG_FLAG_BITS(dmp,DEBUG_SET);
}

void _set_debug(QSP_ARG_DECL  Debug_Module *dbmp)
{
	snprintf(ERROR_STRING,LLEN,"enabling debugging messages for %s module",
		DEBUG_NAME(dbmp));
	advise(ERROR_STRING);
//snprintf(ERROR_STRING,LLEN,"mask = 0x%x",DEBUG_MASK(dbmp));
//advise(ERROR_STRING);

	if( DEBUG_FLAGS(dbmp) & DEBUG_SET )
		debug |= DEBUG_MASK(dbmp);
	else
		debug &= ~DEBUG_MASK(dbmp);

//snprintf(ERROR_STRING,LLEN,"debug = 0x%x",debug);
//advise(ERROR_STRING);
}

debug_flag_t _add_debug_module(QSP_ARG_DECL  const char *name)
{
	Debug_Module *dmp;

	dmp = new_debug(name);
	if( dmp == NULL ) return 0;

	if( n_debug_modules < 0 ) init_dbm();

	if( n_debug_modules >= MAX_DEBUG_MODULES ){
		snprintf(ERROR_STRING,LLEN,"Can't add debug module %s",name);
		warn(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"n is %d",n_debug_modules);
		advise(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"Max is %ld",(long)MAX_DEBUG_MODULES);
		advise(ERROR_STRING);
		advise("Modules:");
		list_debugs(tell_errfile());
		return(0);
	}

	SET_DEBUG_MASK(dmp, 1 << n_debug_modules++ );
	SET_DEBUG_FLAGS(dmp, DEBUG_SET);	// default
	return(DEBUG_MASK(dmp));
}

#endif // QUIP_DEBUG

// We don't need to call assign_var (or assign_reserved_var)
// here, because $verbose is a dynamic variable...

void clr_verbose(SINGLE_QSP_ARG_DECL)
{
	if( verbose )  advise("suppressing verbose messages");
	quip_verbose=0;
}

void set_verbose(SINGLE_QSP_ARG_DECL)
{
	advise("printing verbose messages");
	quip_verbose=1;
}

void verbtog(SINGLE_QSP_ARG_DECL)
{
	if( quip_verbose ){
		clr_verbose(SINGLE_QSP_ARG);
	} else {
		set_verbose(SINGLE_QSP_ARG);
	}
}

