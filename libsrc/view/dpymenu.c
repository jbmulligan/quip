#include "quip_config.h"

/* manipulate displays */


#include <stdio.h>

#include "data_obj.h"
#include "getbuf.h"
#include "viewer.h"
#include "view_cmds.h"
#include "view_util.h"
#include "item_type.h"
#include "quip_menu.h"
#include "xsupp.h"

#include "dispobj.h"
#include "debug.h"	/* verbose */

static COMMAND_FUNC( do_new_do )
{
	const char *s;

	s=nameof("display");
#ifdef HAVE_X11
	if( open_display(s,8) == NULL ){
		snprintf(ERROR_STRING,LLEN,"unable to open %s",s);
		warn(ERROR_STRING);
	}
#endif /* HAVE_X11 */
}

static COMMAND_FUNC( do_open_do )
{
	const char *s;
	int d;

	s=nameof("display");
	d=how_many("desired bit depth");
#ifdef HAVE_X11
	if( open_display(s,d) == NULL ){
		snprintf(ERROR_STRING,LLEN,"unable to open %s",s);
		warn(ERROR_STRING);
	}
#endif /* HAVE_X11 */
}

static COMMAND_FUNC( set_do )
{
	Disp_Obj *dop;

	dop = pick_disp_obj("");
	if( dop == NULL ) return;
#ifdef HAVE_X11
	set_display(dop);
#endif /* HAVE_X11 */
}

static COMMAND_FUNC( do_tell_dpy )
{
	Disp_Obj *dop;
	const char *s;

	s=nameof("name of variable in which to deposit name of current display");

	dop = curr_dop();
	if( dop == NULL ){
		warn("do_tell_dpy:  no current display!?");
		return;
	}

	assign_var(s,DO_NAME(dop));
}

static COMMAND_FUNC( do_info_do )
{
	Disp_Obj *dop;

	dop= pick_disp_obj("");
	if( dop == NULL ) return;
#ifdef HAVE_X11
	info_do(dop);
#endif /* HAVE_X11 */
}

static COMMAND_FUNC( do_show_visuals )
{
	Disp_Obj *dop;

	dop= pick_disp_obj("");
	if( dop == NULL ) return;
#ifdef HAVE_X11
	show_visuals(dop);
#endif /* HAVE_X11 */
}

static COMMAND_FUNC( do_list_dos )
{ list_disp_objs(tell_msgfile()); }



#define ADD_CMD(s,f,h)	ADD_COMMAND(displays_menu,s,f,h)

MENU_BEGIN(displays)
ADD_CMD( new,	do_new_do,		open new display )
ADD_CMD( open,	do_open_do,		open new display w/ specified depth )
ADD_CMD( info,	do_info_do,		give information about a display )
ADD_CMD( visuals,	do_show_visuals,	list available visuals for a display )
ADD_CMD( list,	do_list_dos,		list displays )
ADD_CMD( set,	set_do,			select display for succeeding operations )
ADD_CMD( tell,	do_tell_dpy,		report current default display )
MENU_END(displays)

COMMAND_FUNC( dpymenu )
{
#ifdef HAVE_X11
	ensure_x11_server(SINGLE_QSP_ARG);
#endif /* HAVE_X11 */

	CHECK_AND_PUSH_MENU(displays);
}

