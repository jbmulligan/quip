
#include "quip_config.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "debug.h"
#include "getbuf.h"
#include "node.h"
#include "quip_prot.h"
#include "warn.h"

typedef enum {
	JSON_TYPE_INVALID,	// 0
	JSON_TYPE_OBJECT,	// 1
	JSON_TYPE_NUMBER,	// 2
	JSON_TYPE_STRING,	// 3
	JSON_TYPE_ID,		// 4
	JSON_TYPE_NULL,		// 5
	N_JSON_TYPES		// 6
} JSON_Thing_Type;

typedef enum {
	JSON_OBJ_TYPE_INVALID,	// 0
	JSON_OBJ_TYPE_PAIRS,	// 1
	JSON_OBJ_TYPE_ARRAY,	// 2
	N_JSON_OBJ_TYPES	// 3
} JSON_Obj_Type;

struct json_array;
struct json_obj;

typedef struct json_thing {
	union {
		struct json_array *	u_array_p;
		struct json_obj *	u_obj_p;
		const char *		u_string;
		double			u_number;
	} jt_u;
	JSON_Thing_Type	jt_type;
} JSON_Thing;

/*
typedef struct json_array {
	JSON_Thing **	ja_thing_tbl;
	int		ja_len;
} JSON_Array;
*/

typedef struct json_pair {
	const char *	jp_key;
	JSON_Thing *	jp_thing_p;
} JSON_Pair;

typedef struct json_obj {
	union {
		JSON_Pair **	u_pair_tbl;
		JSON_Thing **	u_thing_tbl;
	} jo_u;
	JSON_Obj_Type	jo_type;
	int		jo_len;
} JSON_Obj;

#define jo_thing_tbl	jo_u.u_thing_tbl
#define jo_pair_tbl	jo_u.u_pair_tbl

extern JSON_Obj *_parse_json(SINGLE_QSP_ARG_DECL);
extern void _dump_json_obj(QSP_ARG_DECL  JSON_Obj *obj_p);

extern COMMAND_FUNC(do_json_menu);

#define parse_json()	_parse_json(SINGLE_QSP_ARG)
#define dump_json_obj(obj)	_dump_json_obj(QSP_ARG  obj)

