#ifndef _LIST_H_
#define _LIST_H_

#include "quip_fwd.h"
#include "node.h"
#include "typedefs.h"
#include "item_obj.h"

// list.c
extern void report_node_data(SINGLE_QSP_ARG_DECL);
extern count_t eltcount( List * lp );
#define NEW_LIST		new_list()
extern List *new_list(void);

extern void rls_list_nodes(List *lp);
extern void _zap_list(QSP_ARG_DECL List *lp);
extern Node *mk_node(void * ip );
#define zap_list(lp) _zap_list(QSP_ARG lp)
extern Node *remHead(List *lp);
extern Node *remTail(List *lp);
extern Node * remNode(List *lp, Node *np);
extern Node *remData(List *lp, void * data);
extern void rls_list(List *lp);
//extern void _rls_list(QSP_ARG_DECL  List *lp);
//#define rls_list(lp) _rls_list(QSP_ARG  lp)

extern void rls_nodes_from_list(List *lp);
extern void addTail(List *lp, Node *np);
extern void addHead( List *lp, Node* np );
extern void _dellist(QSP_ARG_DECL  List *lp);
#define dellist(lp) _dellist(QSP_ARG  lp)

extern Node *nodeOf( List *lp, void * ip );
extern Node * list_find_named_item(List *lp, const char *name);

extern void p_sort(List *lp);
extern Node *nth_elt(List *lp, count_t k);
extern Node *nth_elt_from_tail(List *lp, count_t k);



extern void advance_list_enumerator(List_Enumerator *lep);
extern Item *list_enumerator_item(List_Enumerator *lep);
extern List_Enumerator *_new_list_enumerator(QSP_ARG_DECL  List *lp);
#define new_list_enumerator(lp) _new_list_enumerator(QSP_ARG  lp)

extern void rls_list_enumerator(List_Enumerator *lp);

extern Node *list_head(List *lp);
extern Node *list_tail(List *lp);
#define QLIST_HEAD(lp)	list_head(lp)
#define QLIST_TAIL(lp)	list_tail(lp)

#endif /* ! _LIST_H_ */

