#include <string.h>
#include "quip_config.h"
#include "quip_prot.h"
#include "history.h"
#include "query_stack.h"
#include "query_prot.h"

static List *mf_list=NULL;

#ifndef MAC
#ifdef HAVE_HISTORY
#ifdef TTY_CTL

static void stash_menu_commands(QSP_ARG_DECL  Menu *mp)
{
	// Make sure we have a history list for this menu,
	// and add the commands as default entries.
	List *lp;
	Node *np;

	assert( mp != NULL );

	lp = container_list( MENU_CONTAINER(mp) );
	assert( lp != NULL );

	np = QLIST_HEAD(lp);
	while( np != NULL ){
		Command *cp;
		cp = (Command *)NODE_DATA(np);
		// should we use the menu prompt or the full prompt?
		add_def(QS_CMD_PROMPT_STR(THIS_QSP),CMD_SELECTOR(cp));
		np = NODE_NEXT(np);
	}
}
#endif /* TTY_CTL */
#endif /* HAVE_HISTORY */
#endif /* ! MAC */

static void perform_callbacks(SINGLE_QSP_ARG_DECL)
{
	assert( QS_CALLBACK_LIST(THIS_QSP) != NULL );

	reset_return_strings();
	call_funcs_from_list(QS_CALLBACK_LIST(THIS_QSP) );
}

void qs_do_cmd( Query_Stack *qsp )
{
	const char *cmd;
	Menu *mp;
	Command *cp;

//advise("qs_do_cmd BEGIN");
//qdump(qsp);

	mp = TOP_OF_STACK( QS_MENU_STACK(qsp) );
	assert( mp != NULL );

#ifdef HAVE_HISTORY
#ifdef TTY_CTL
	// preload the history list
	//
	// The first time it doesn't get activated, because
	// we have been reading the startup files...
	// Need to lookahead!

	if( (! MENU_COMMANDS_STASHED(mp))  &&  intractive() ){
		stash_menu_commands(QSP_ARG  mp);
		stash_menu_commands(QSP_ARG  QS_BUILTIN_MENU(THIS_QSP));
		SET_MENU_FLAG_BITS(mp,MENU_FLAG_CMDS_STASHED);
	}
#endif /* TTY_CTL */
#endif /* HAVE_HISTORY */


	//SET_QRY_RETSTR_IDX(CURR_QRY(THIS_QSP),0);
	reset_return_strings();

	cmd = next_query_word(QS_CMD_PROMPT_STR(qsp));

	if( cmd == NULL || strlen(cmd) == 0 ){
		return;
	}
	cp = (Command *) container_find_match(MENU_CONTAINER(mp),cmd);

	if( cp == NULL )
		cp = (Command *) container_find_match( MENU_CONTAINER(THIS_QSP->qs_builtin_menu), cmd );

	if( cp == NULL )
		cp = (Command *) container_find_match( MENU_CONTAINER(THIS_QSP->qs_help_menu), cmd );

	if( cp == NULL ){

#ifdef HAVE_HISTORY
		/* make sure that a bad command doesn't get saved */
			/* intractive() calls lookahead... */
			/* what is/was HISTORY_FLAG? */
		if( intractive() /* && HISTORY_FLAG */ ){
			rem_def(QS_CMD_PROMPT_STR(qsp),cmd);	// erase from history list
		}
#endif /* HAVE_HISTORY */

		snprintf(ERROR_STRING,LLEN,"Command \"%s\" not found!?",cmd);
		warn(ERROR_STRING);
		// Show the menu here
		list_menu( TOP_OF_STACK( QS_MENU_STACK(THIS_QSP) ) );
		list_menu( QS_HELP_MENU(THIS_QSP) );

		return;
	}
	(*(cp->cmd_action))(SINGLE_QSP_ARG);

	if( IS_PROCESSING_CALLBACKS(THIS_QSP) ){
		perform_callbacks(SINGLE_QSP_ARG);
	}
	// test IS_HALTING(qsp) here???
}


void rls_mouthful(Mouthful *mfp)
{
	Node *np;

	/* we don't release the strings here, because
	 * a query structure might be holding a pointer to
	 * the terminating null.
	 * We put the struct on the free list, and release
	 * the strings when we retrieve it from the free list.
	 */

	if( mf_list == NULL ){
		mf_list=new_list();
	}
	np = mk_node(mfp);
	addHead(mf_list,np);
}

// Can we be sure that we are finished with the references to
// a mouthful's strings when we move it to the free list?
// push_text doesn't make its own copies...

Mouthful *new_mouthful(const char *text, const char *filename)
{
	Mouthful *mfp;
	if( mf_list != NULL ){
		Node *np;

		np=remHead(mf_list);
		if( np != NULL ){
			mfp= (Mouthful *)NODE_DATA(np);
			if( mfp->text != NULL ){
				rls_str(mfp->text);
				mfp->text = savestr(text);
			}
			if( mfp->filename != NULL ){
				rls_str(mfp->filename);
				mfp->filename=savestr(filename);
			}
			return mfp;
		}
	}
	mfp = (Mouthful *) getbuf( sizeof(*mfp) );
	mfp->text=savestr(text);
	mfp->filename=savestr(filename);
	return mfp;
}

static void store_mouthful(QSP_ARG_DECL  const char *text,
						const char *filename )
{
	Node *np;
	Mouthful *mfp;

	mfp = new_mouthful(text,filename);

	np = mk_node( (void *) mfp );
	if( CHEW_LIST == NULL ){
		CHEW_LIST = new_list();
		assert( CHEW_LIST != NULL );
	}
	addTail(CHEW_LIST,np);
}

// chew_mouthful is called by the iOS alarm handler...
// we need to push the top menu, but we have to make sure that
// we don't store this for later, we want to execute now!?

void chew_mouthful(Mouthful *mfp)
{
	_chew_text(DEFAULT_QSP_ARG  mfp->text, mfp->filename );
}

static void begin_digestion(QSP_ARG_DECL  const char *text, const char *filename)
{
	push_top_menu(SINGLE_QSP_ARG);	/* make sure at root menu */
	push_text(text,filename);	/* or fullpush? */
}

// Interpret this text

void swallow(QSP_ARG_DECL  const char *text, const char *filename)
{
	SET_QS_FLAG_BITS(THIS_QSP,QS_CHEWING);
	// Why do we need to remember the chew level?
	// Does it have something to do with returning
	// from alerts in iOS?
	SET_QS_CHEW_LEVEL(THIS_QSP,QLEVEL);
	begin_digestion(QSP_ARG  text,filename);
	finish_swallowing(SINGLE_QSP_ARG);
}

COMMAND_FUNC( suspend_chewing )
{
	CLEAR_QS_FLAG_BITS(THIS_QSP,QS_CHEWING);
}

COMMAND_FUNC( unsuspend_chewing )
{
	SET_QS_FLAG_BITS(THIS_QSP,QS_CHEWING);
}

static void finish_digestion(QSP_ARG_DECL  int level)
{
	exec_at_level(QSP_ARG  level);
	if( IS_HALTING(THIS_QSP) ){
		return;
	}
	pop_menu();
}


// We use digest when we call from the optimization package,
// because it doesn't mess with the chew flag...

void _digest(QSP_ARG_DECL  const char *text, const char *filename)
{
	/* set_busy_cursor(); */

	begin_digestion(QSP_ARG  text, filename );
	finish_digestion(QSP_ARG  QLEVEL); /* pops the menu */
}

static void chew_stored(SINGLE_QSP_ARG_DECL)
{
	if( CHEW_LIST != NULL ){
		Node *np;
		Mouthful *mfp;

		while( (np=remHead(CHEW_LIST)) != NULL ){
			mfp = (Mouthful *) NODE_DATA(np);
			swallow( QSP_ARG  mfp->text, mfp->filename );
			// BUG - we might not be done with the text released!?
			// if we're halting!?
			rls_mouthful(mfp);
			rls_node(np);
			if( IS_HALTING(THIS_QSP) ){
				return;
			}
		}
	}
	/* else { warn("no chew_list"); } */

	/* set_std_cursor(); */
} // end chew_stored

void finish_swallowing(SINGLE_QSP_ARG_DECL)
{
	finish_digestion(QSP_ARG  1+QS_CHEW_LEVEL(THIS_QSP) );
	if( IS_HALTING(THIS_QSP) ){
		return;
	}
	CLEAR_QS_FLAG_BITS(THIS_QSP,QS_CHEWING);
}

// eat this text, unless we are already chewing something else...
// In that case, save it for later.

void _chew_text(QSP_ARG_DECL  const char *text, const char *filename )
{
	if( text == NULL ) return;
	if( IS_CHEWING(THIS_QSP) ){
		store_mouthful(QSP_ARG  text, filename );
	} else {
		swallow(QSP_ARG  text, filename);
		chew_stored(SINGLE_QSP_ARG);
	}
}

static void resume_chewing(SINGLE_QSP_ARG_DECL)
{
	if( ! IS_CHEWING(THIS_QSP) ){
		return;
	}
	CLEAR_QS_FLAG_BITS(THIS_QSP,QS_HALTING);

	// first finish the interrupted swallow
	finish_swallowing(SINGLE_QSP_ARG);
	if( IS_HALTING(THIS_QSP) ){
		return;
	}

	chew_stored(SINGLE_QSP_ARG);

	if( IS_HALTING(THIS_QSP) ){
		return;
	}

	CLEAR_QS_FLAG_BITS(THIS_QSP,QS_CHEWING);
}

void resume_execution(SINGLE_QSP_ARG_DECL)
{
	CLEAR_QS_FLAG_BITS(THIS_QSP,QS_HALTING);
	exec_at_level(QSP_ARG  0);
}

// We use this in the iOS environment, when we need to temporarily yield
// to the system's dispatch queue, to carry out some previous requests.
// We have been doing that with alerts, where we have a callback function
// that can restart execution...

void _suspend_quip(SINGLE_QSP_ARG_DECL)
{
	SET_QS_FLAG_BITS(THIS_QSP, (QS_HALTING|QS_SUSPENDED) );
}

void resume_quip(SINGLE_QSP_ARG_DECL)
{
	if( IS_CHEWING(THIS_QSP) ){
		resume_chewing(SINGLE_QSP_ARG);
	} else {
		resume_execution(SINGLE_QSP_ARG);
	}
}

// BUG if we push a macro which contains only comments, then we could
// end up presenting an interactive prompt instead of popping back...

void exec_at_level(QSP_ARG_DECL  int level)
{
	int l;

	assert( level >= 0 );

	// We thought a lookahead here might help, but it didn't, probably
	// because lookahead does not skip comments?

	push_stop_level(level);

	//lookahead(SINGLE_QSP_ARG);	// in case an empty macro was pushed?
	while( QLEVEL >= level ){
		qs_do_cmd(THIS_QSP);
		if( IS_HALTING(THIS_QSP) ){
			// restore stop level here???  BUG?
			return;
		}

		/* The command may have disabled lookahead;
		 * We need to lookahead here to make sure
		 * the end of the text is properly detected.
		 */

		lookahead(SINGLE_QSP_ARG);
	}
	l = pop_stop_level();
	set_stop_level(l);

	// BUG?  what happens if we halt execution when an alert is delivered?
}

