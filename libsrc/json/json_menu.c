#include "quip_config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <stdio.h>

#include "debug.h"
//#include "menuname.h"
//#include "version.h"
#include "quip_prot.h"
#include "quip_menu.h"
#include "warn.h"
#include "getbuf.h"


#include "json.h"

static COMMAND_FUNC(do_read_json)
{
	FILE *fp;
	const char *s;
	JSON_Obj *obj;

	s = nameof("input JSON file");
	fp = try_open(s,"r");
	if( ! fp ) return;

	redir(fp,s);
	obj = parse_json();
	dump_json_obj(obj);
}

#define ADD_CMD(s,f,h)	ADD_COMMAND(json_menu,s,f,h)
MENU_BEGIN(json)
ADD_CMD(	read,		do_read_json,	read JSON file)
MENU_END(json)

COMMAND_FUNC(do_json_menu)
{
	CHECK_AND_PUSH_MENU(json);
}

	
