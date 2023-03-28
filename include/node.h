#ifndef _NODE_H_
#define _NODE_H_

#include "quip_fwd.h"

struct node {
	void *		n_data;
	struct node *	n_next;
	struct node *	n_prev;
	int		n_pri;
} ;

extern void rls_node(Node *np);
extern void init_node(Node *np,void* dp);

#define NODE_DATA(np)		(np)->n_data
#define SET_NODE_DATA(np,d)	(np)->n_data=d
#define NODE_NEXT(np)		(np)->n_next
#define NODE_PREV(np)		(np)->n_prev
#define SET_NODE_NEXT(np,_np)	(np)->n_next = _np
#define SET_NODE_PREV(np,_np)	(np)->n_prev = _np

#endif /* !  _NODE_H_ */

