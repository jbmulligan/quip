#include "quip_config.h"

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "quip_prot.h"
#include "debug.h"
#include "hash.h"
#include "list.h"
#include "getbuf.h"

#ifdef QUIP_DEBUG
static u_long hash_debug=HASH_DEBUG_MASK;
#endif

#define MAX_SILENT_COLL_RATE	2.5

#define N_PRIMES	16
static u_long prime[N_PRIMES]={
	31L,
	61L,
	121L,
	241L,
	479L,
	953L,
	1901L,
	3797L,
	7591L,
	15173L,
	30341L,
	60679L,
	121357L,
	242713L,
	485423L,
	970829L
};

/* local prototypes */
static void clear_entries(void **entry,u_long size);

static void clear_entries(void **entry, u_long size)
{
	u_long i;

	for(i=0;i<size;i++) entry[i] = NULL ;
}



#define setup_ht(htp,size) _setup_ht(QSP_ARG  htp,size)

static void _setup_ht(QSP_ARG_DECL  Hash_Tbl *htp, u_long size)
{
	u_long i;

	for(i=0;i<N_PRIMES;i++)
		if( prime[i] > size ){
			size=prime[i];
			i=N_PRIMES+2;
		}
	if( i == N_PRIMES ){
		snprintf(ERROR_STRING,LLEN,"setup_ht( tbl = %s, size = %ld (0x%lx) )",htp->ht_name,size,size);
		advise(ERROR_STRING);

		error1("need to enlarge prime number table in hash.c");
		IOS_RETURN
	}

	htp->ht_size=size;
	htp->ht_entries = (void**) getbuf(size * sizeof(void *));
	//if( htp->ht_entries==NULL ) mem_err("setup_ht");
	if( htp->ht_entries==NULL ) warn("setup_ht memory allocation error");

	clear_entries(htp->ht_entries,size);

	htp->ht_n_entries=0;
	htp->ht_n_removals=0;
	htp->ht_n_moved=0;
	htp->ht_n_checked=0;

	htp->ht_flags=0;

	htp->ht_item_list = NULL;

#ifdef MONITOR_COLLISIONS
	htp->ht_searches=0;
	htp->ht_collisions=0;
#endif
}

Hash_Tbl * _enlarge_ht(QSP_ARG_DECL  Hash_Tbl *htp)
{
	u_long newsize;
	u_long i;
	void **oldents;
	void **oe;
	u_long oldsize;

if( verbose ){
snprintf(ERROR_STRING,LLEN,"doubling size of hash table \"%s\" (old size = %ld)",
htp->ht_name,htp->ht_size);
advise(ERROR_STRING);
}


	oldsize = htp->ht_size;
	newsize = 2*oldsize;
	oldents=htp->ht_entries;

	setup_ht(htp,newsize);

	/* now copy all of the entries */
	oe= oldents;
	for(i=0;i<oldsize;i++)
		if( oe[i] != NULL ){
			if( insert_hash(oe[i],htp) < 0 ){
				error1("error growing hash table");
				IOS_RETURN_VAL(NULL)
			}
		}

	/* now free the old entries */
	givbuf(oldents);

	return(htp);
}

/*
 * Allocate and initialize a hash table with the given name and size entries
 */

Hash_Tbl *_ht_init(QSP_ARG_DECL  const char *name)
		/* name for hash table */
{
	register Hash_Tbl *htp;

	htp= (Hash_Tbl*) getbuf( sizeof(*htp) );
	//if( htp == NULL ) mem_err("ht_init");
	if( htp == NULL ) {
		error1("ht_init memory allocation failure");
		IOS_RETURN_VAL(NULL)
	}
//fprintf(stderr,"ht_init:  name = 0x%lx  (\"%s\")\n",name,name==NULL?"<null>":name);
	if( name != NULL )
		htp->ht_name=savestr(name);
	else
		htp->ht_name=NULL;

	setup_ht(htp,prime[0]);

	return(htp);
}

void _zap_hash_tbl(QSP_ARG_DECL  Hash_Tbl *htp)
{
	givbuf(htp->ht_entries);
	if( htp->ht_name != NULL )
		rls_str(htp->ht_name);
	if( HT_ITEM_LIST(htp) != NULL )
		zap_list( HT_ITEM_LIST(htp) );
}


#define KEY_MULT	127

#define compute_key(keyvar,name,table)					\
				keyvar = 0;				\
				while(*name){				\
					keyvar +=(*name++);		\
					keyvar *= KEY_MULT;		\
					keyvar %= table->ht_size;	\
				}

/*
 * Insert the item pointed to by ptr into hash table.
 * ptr must point to a structure whose first element is a
 * pointer to a string with the item's name.
 *
 * If the node pointer is not null, the node is added to the
 * table list;  This routine may be called with a null node
 * pointer because the list already exists and doesn't need
 * to be updated, or because we don't care about the list...
 *
 * Returns 0 on success, -1 on failure.
 */

int _insert_hash(QSP_ARG_DECL  void *ptr,Hash_Tbl *htp)
			/* item to insert */
			/* hash table */
{
	u_long key;
	register char *name;
	u_long start;
	void **entry;

	/* check load factor */
	if( ((float)htp->ht_n_entries/(float)htp->ht_size) > 0.7 ){

if( verbose ){
snprintf(ERROR_STRING,LLEN,
"enlarging hash table %s: %ld of %ld entries filled",
htp->ht_name,htp->ht_n_entries,
htp->ht_size);
advise(ERROR_STRING);
}

		htp=enlarge_ht(htp);
	}

	/*
	 *	we assume that ptr points to a structure
	 *	whose first member is a char *
	 */

	name = * (char **) ptr;

#ifdef QUIP_DEBUG
if( debug & hash_debug ){
snprintf(ERROR_STRING,LLEN,"insert_hash: table %s at 0x%lx, item %s",
htp->ht_name,(long)htp,name);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
	compute_key(key,name,htp);	/* a macro for setting key */

	start = key;

	entry = htp->ht_entries;

	while( entry[key] != NULL ){
		key++;
		if( key >= htp->ht_size ) key=0;
		if( key==start )	/* hash table full */
			return(-1);
	}

	/* now we've found an empty slot */

	entry[key] = ptr;
	htp->ht_n_entries++;

	MARK_DIRTY(htp);

	return(0);
}

void _show_ht(QSP_ARG_DECL  Hash_Tbl *htp)	/* print out the contents of the given hash table */
{
	unsigned long i;
	register char *name;
	char *s;
	void **entry;
	u_long key;

	entry = htp->ht_entries;
	for(i=0;i<htp->ht_size;i++)
		if( entry[i] != NULL ){
			name = * ( char ** ) entry[i];
			s=name;
			compute_key(key,s,htp);	/* a macro for setting key */
			snprintf(ERROR_STRING,LLEN,"%ld\t%s, key = %ld",i,name,key);
			advise(ERROR_STRING);
		}
}

/*
 * Search the given table for an item with the given name.
 * Returns a pointer to the item if successful, or a null pointer.
 */

void *_fetch_hash(QSP_ARG_DECL  const char *name,Hash_Tbl* htp)
		/* name = target name */
		/* htp = table to search */
{
	u_long key;
	const char *s;
	u_long start;
	void **entry;

	if( HASH_TBL_WARNED(htp) ){
		if( verbose ){
snprintf(ERROR_STRING,LLEN,
"Enlarging hash table %s due to collisions, load factor is %g",
htp->ht_name,
((float)htp->ht_n_entries/(float)htp->ht_size) );
advise(ERROR_STRING);
		}
		htp=enlarge_ht(htp);
		CLEAR_HT_FLAG_BITS(htp,HT_WARNED);
	}

	entry = htp->ht_entries;
		
	s=name;
	compute_key(key,s,htp);	/* a macro for setting key */

	start = key;
#ifdef MONITOR_COLLISIONS
	htp->ht_searches++;
#endif /* MONITOR_COLLISIONS */

	while( entry[key] != NULL ){
		/* test this one */

		/* the entries are typically structure pointers */
		/* we assume that the name is the first field */

		s = *((char **)entry[key]);
		assert( s != NULL );

		if( !strcmp(s,name) ) return(entry[key]);

#ifdef MONITOR_COLLISIONS
		if( htp->ht_collisions >
			MAX_SILENT_COLL_RATE * htp->ht_searches
			&& !HASH_TBL_WARNED(htp) ){
			if( verbose ){
				snprintf(ERROR_STRING,LLEN,
"High collision rate (%f collisions/search), hash table \"%s\"",
					(float) htp->ht_collisions /
					(float) htp->ht_searches, htp->ht_name);
				advise(ERROR_STRING);
			}
			SET_HT_FLAG_BITS(htp,HT_WARNED);
		}

		htp->ht_collisions++;
#endif

		key++;
		if( key >= htp->ht_size ) key=0;
		if( key==start ) return(NULL);
	}
	return(NULL);
}

/*
 * was called remove_hash...
 *
 * Remove the pointed to item from the given hash table.
 * Returns 0 on success, -1 if the item is not found.
 */

int _remove_item_from_hash(QSP_ARG_DECL  const Item *ptr,Hash_Tbl *htp)
			/* pointer to the item to remove */
		/* table to search */
{
	const char *name;

//	name = * (char **) ptr;
	name = ITEM_NAME(ptr);
	return remove_name_from_hash(name,htp);
}

int _remove_name_from_hash(QSP_ARG_DECL  const char * name, Hash_Tbl *htp )
{
	u_long key;
	const char *s;
	u_long start;
	u_long k2;
	void **entry;

	entry = htp->ht_entries;
		
	s=name;
	compute_key(key,s,htp);	/* a macro for setting key */

	start = key;
#ifdef MONITOR_COLLISIONS
	htp->ht_searches++;
#endif /* MONITOR_COLLISIONS */

#ifdef QUIP_DEBUG
if( debug & hash_debug ){
snprintf(ERROR_STRING,LLEN,"key:  %ld",key);
advise(ERROR_STRING);
}
#endif

	while( entry[key] != NULL ){
		/* test this one */

		/* the entries are typically structure pointers */
		/* we assume that the name is the first field */

		s = *((char **)entry[key]);
		if( !strcmp(s,name) ){
#ifdef QUIP_DEBUG
if( debug & hash_debug ){
snprintf(ERROR_STRING,LLEN,"removing item at location %ld",key);
advise(ERROR_STRING);
}
#endif
			/* found the one to remove */
			entry[key] = NULL;
			start=key;
			MARK_DIRTY(htp);

			/* now check for collisions */
			while(1){
				char **sp;

				key++;
				if( key >= htp->ht_size ) key=0;

				if( entry[key] == NULL ){
					htp->ht_n_removals++;
					htp->ht_n_entries--;
					return(0);
				}

				sp=(char **)entry[key];
				s=(*sp);
#ifdef QUIP_DEBUG
if( debug & hash_debug ){
snprintf(ERROR_STRING,LLEN,"considering shifting item %s",s);
advise(ERROR_STRING);
}
#endif
				compute_key(k2,s,htp);	/* a macro for setting key */

				/* move if k2 is outside the range */
				if( ( k2 <= start &&
					( key > start || k2 > key ) )
				|| ( k2 > key &&  key > start ) ){
#ifdef QUIP_DEBUG
if( debug & hash_debug ){
snprintf(ERROR_STRING,LLEN,"shifting at %ld",key);
advise(ERROR_STRING);
}
#endif
					entry[start]
					  = entry[key];
					htp->ht_n_moved++;
					entry[key] = NULL;
					start=key;
				}
				htp->ht_n_checked++;
			}
		}

#ifdef MONITOR_COLLISIONS
		htp->ht_collisions++;
#endif

		key++;
		if( key >= htp->ht_size ) key=0;
		if( key==start ) goto not_found;
	}
not_found:
	snprintf(ERROR_STRING,LLEN,"word \"%s\" not found in hash table \"%s\"",
		name,htp->ht_name);
	advise(ERROR_STRING);
	return(-1);
}

/*
 * Print the statistics for the given hash table to stdout
 */

void tell_hash_stats(QSP_ARG_DECL  Hash_Tbl *htp)
		/* hash table */
{
	snprintf(DEFAULT_MSG_STR,LLEN,"\tStatistics for hash table \"%s\":",
		htp->ht_name);
	prt_msg(MSG_STR);
	snprintf(DEFAULT_MSG_STR,LLEN,"\tsize %ld, %ld entries",
		htp->ht_size,htp->ht_n_entries);
	prt_msg(DEFAULT_MSG_STR);

#ifdef MONITOR_COLLISIONS
	/*
	 * This was the old version when the var was u_long
	snprintf(DEFAULT_MSG_STR,LLEN,"\t%ld searches",htp->ht_searches);
	*/
	snprintf(DEFAULT_MSG_STR,LLEN,"\t%g searches",htp->ht_searches);
	prt_msg(DEFAULT_MSG_STR);

	if( htp->ht_searches > 0 ){
		double d;

		snprintf(DEFAULT_MSG_STR,LLEN,"\t%g collisions",htp->ht_collisions);
		prt_msg(DEFAULT_MSG_STR);
		d=(double)htp->ht_collisions;
		d /= (double) htp->ht_searches;
		snprintf(DEFAULT_MSG_STR,LLEN,"\t%f collisions/search",d);
		prt_msg(DEFAULT_MSG_STR);
	}
#endif
	if( htp->ht_n_removals > 0 ){
		snprintf(DEFAULT_MSG_STR,LLEN,"\t%ld removals",htp->ht_n_removals);
		prt_msg(DEFAULT_MSG_STR);
		snprintf(DEFAULT_MSG_STR,LLEN,"\t%ld items checked",htp->ht_n_checked);
		prt_msg(DEFAULT_MSG_STR);
		snprintf(DEFAULT_MSG_STR,LLEN,"\t%ld items moved",htp->ht_n_moved);
		prt_msg(DEFAULT_MSG_STR);
	}
	if( verbose ) show_ht(htp);
}


/* Build a list of all the items in this hash table */

List *_ht_list(QSP_ARG_DECL  Hash_Tbl *htp)
{
	void **entry;
	unsigned int i;
	List *lp;

	lp=new_list();
	entry = htp->ht_entries;
	for(i=0;i<htp->ht_size;i++){
		Item *ip;

		ip = (Item*) entry[i];
		if( ip != NULL ){
			Node *np;
			np=mk_node(ip);
			addTail(lp,np);
		}
	}
	return(lp);
}

void advance_ht_enumerator(Hash_Tbl_Enumerator *htep)
{
	void **entry;

	entry = htep->current_entry;
	if( entry == NULL ) return;

	while( ++entry < htep->htp->ht_entries+htep->htp->ht_size ){
		if( *entry != NULL ){
			htep->current_entry = entry;
			return;
		}
	}
	// end of table reached
	htep->current_entry = NULL;
}

Item *ht_enumerator_item(Hash_Tbl_Enumerator *htep)
{
	if( htep->current_entry == NULL ) return NULL;
	return (Item *) *(htep->current_entry);
}

void rls_hash_tbl_enumerator(Hash_Tbl_Enumerator *ep)
{
	givbuf(ep);	// We might save a bit by keeping a pool of these?
}

Hash_Tbl_Enumerator *_new_hash_tbl_enumerator(QSP_ARG_DECL  Hash_Tbl *htp)
{
	Hash_Tbl_Enumerator *htep;

	htep = getbuf( sizeof(*htep) );
	htep->htp = htp;	// needed!  we will need to know the size...
	htep->current_entry = htp->ht_entries;

	// advance to the first non-null entry
	while( *(htep->current_entry) == NULL && htep->current_entry < htep->htp->ht_entries+htep->htp->ht_size ){
		htep->current_entry ++;
	}
	if( htep->current_entry == htep->htp->ht_entries+htep->htp->ht_size ){
		htep->current_entry = NULL;	// nothing there.
	}
	return htep;
}

List *_hash_tbl_list(QSP_ARG_DECL  Hash_Tbl *htp)
{
	if( HT_ITEM_LIST(htp) == NULL ){
		SET_HT_ITEM_LIST(htp, ht_list(htp) );	// allocates and populates a new list
		MARK_CURRENT(htp);
	} else {
		if( ! HASH_TBL_LIST_IS_CURRENT(htp) ){
			zap_list( HT_ITEM_LIST(htp) );
			SET_HT_ITEM_LIST(htp, ht_list(htp) );	// allocates and populates a new list
			MARK_CURRENT(htp);
		}
	}
	return HT_ITEM_LIST(htp);
}

