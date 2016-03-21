#ifndef _QUERY_H_
#define _QUERY_H_

//#include "fragment.h"
#include "list.h"
#include "macro.h"
#include "query_bits.h"

#define QBUF_LEN	256
#define N_QRY_RETSTRS	32	// BUG shouldn't be a fixed number,
				// But as long as this is larger than the max number
				// of command args we should be OK... ?

typedef struct query {
	char *			(*q_readfunc)(QSP_ARG_DECL  void *buf, int size, void *stream);
	int			q_idx;
	char *			q_databuf;	// was NSString - do we need?
	// BUG - we'd like to be able to grow this table dynamically...
	// It is kind of wasteful for every query in the stack to have
	// the max number of ret_strs!?
	String_Buf *		q_retstr_tbl[N_QRY_RETSTRS];
	int			q_which_retstr;
	int			q_count;
	FILE *			q_fp;
	FILE *			q_dup_fp;
	char *			q_text;
	int			q_txtsiz;
	int			q_txtfree;
	const char *		q_filename;
	int			q_rdlineno;
	int			q_lineno;
	uint32_t		q_flags;
	const char *		q_lbptr;
	char *			q_buffer;
	const char **		q_args;
	Macro *			q_mp;
	Foreach_Loop *		q_forloop;
} Query;

#define NO_QUERY	((Query *)NULL)

#define QRY_FLAGS(qp)			(qp)->q_flags
#define SET_QRY_FLAGS(qp,f)		(qp)->q_flags = f
#define SET_QRY_FLAG_BITS(qp,f)		(qp)->q_flags |= f
#define CLEAR_QRY_FLAG_BITS(qp,f)	(qp)->q_flags &= ~(f)
#define SET_QRY_FORLOOP(qp,f)		(qp)->q_forloop = f
#define QRY_FORLOOP(qp)			(qp)->q_forloop
#define QRY_RETSTR_AT_IDX(qp,idx)	(qp)->q_retstr_tbl[idx]
#define SET_QRY_RETSTR_AT_IDX(qp,idx,sbp)	(qp)->q_retstr_tbl[idx] = sbp
#define QRY_RETSTR_IDX(qp)			(qp)->q_which_retstr
#define SET_QRY_RETSTR_IDX(qp,idx)		(qp)->q_which_retstr = idx

#define QRY_IDX(qp)			(qp)->q_idx
#define SET_QRY_IDX(qp,v)		(qp)->q_idx = v
// This used to be (qp-1) when they were an array...
#define QRY_AT_LEVEL(qsp,l)		query_at_level(QSP_ARG_FOR(qsp)  l )
#define UNDER_QRY(qp)			QRY_AT_LEVEL(THIS_QSP,QRY_IDX(qp)-1)

#define QRY_MACRO(qp)			(qp)->q_mp
#define SET_QRY_MACRO(qp,mp)		(qp)->q_mp =  mp
#define SET_QRY_ARGS(qp,p)		(qp)->q_args = p
#define QRY_ARGS(qp)			(qp)->q_args
#define QRY_ARG_AT_IDX(qp,idx)		(qp)->q_args[idx]
#define SET_QRY_ARG_AT_IDX(qp,idx,s)	(qp)->q_args[idx]=s
#define QRY_FILE_PTR(qp)		(qp)->q_fp
#define SET_QRY_FILE_PTR(qp,fp)		(qp)->q_fp=fp
#define QRY_HAS_FILE_PTR(qp)		(QRY_FILE_PTR(qp) != NULL && ! QRY_IS_SOCKET(qp) )
#define QRY_READFUNC(qp)		(qp)->q_readfunc
#define SET_QRY_READFUNC(qp,f)		(qp)->q_readfunc = f
#define QRY_BUFFER(qp)			(qp)->q_buffer
#define SET_QRY_BUFFER(qp,s)		(qp)->q_buffer = s
#define QRY_FILENAME(qp)		(qp)->q_filename
#define SET_QRY_FILENAME(qp,s)						\
	{								\
	if( QRY_FILENAME(qp) != NULL ) rls_str(QRY_FILENAME(qp));	\
	(qp)->q_filename = savestr(s);					\
	}

#define QRY_DUPFILE(qp)			(qp)->q_dup_fp
#define SET_QRY_DUPFILE(qp,fp)		(qp)->q_dup_fp = fp
#define QRY_LINE_PTR(qp)		(qp)->q_lbptr
#define SET_QRY_LINE_PTR(qp,s)		(qp)->q_lbptr = s
#define QRY_COUNT(qp)			(qp)->q_count
#define SET_QRY_COUNT(qp,n)		(qp)->q_count = n
#define QRY_HAS_TEXT(qp)		(QRY_FLAGS(qp) & Q_HAS_SOMETHING)
#define QRY_IS_SAVING(qp)		(QRY_FLAGS(qp) & Q_SAVING)
#define QRY_LINENO(qp)			(qp)->q_lineno
#define SET_QRY_LINENO(qp,n)		(qp)->q_lineno = n
#define QRY_LBPTR(qp)			(qp)->q_lbptr
#define SET_QRY_LBPTR(qp,p)		(qp)->q_lbptr = p
#define QRY_TEXT(qp)			(qp)->q_text
#define SET_QRY_TEXT(qp,s)		(qp)->q_text = s
#define CLEAR_QRY_TEXT(qp)		*(QRY_TEXT(qp)) = 0
#define QRY_TXTFREE(qp)			(qp)->q_txtfree
#define SET_QRY_TXTFREE(qp,n)		(qp)->q_txtfree = n
#define QRY_TXTSIZE(qp)			(qp)->q_txtsiz
#define SET_QRY_TXTSIZE(qp,n)		(qp)->q_txtsiz = n
#define QRY_RDLINENO(qp)		(qp)->q_rdlineno
#define SET_QRY_RDLINENO(qp,n)		(qp)->q_rdlineno = n

#define QRY_IS_SOCKET(qp)		(QRY_FLAGS(qp) & Q_SOCKET)


/* external API */
/* BUG?  are these really external??? */

extern Query *new_query(void);
extern void rls_query(Query *);

#endif /* ! _QUERY_H_ */
