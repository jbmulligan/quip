#include "quip_config.h"
#include "quip_prot.h"
#include "spink.h"
#include "data_obj.h"
#include "query_bits.h"	// LLEN - BUG

static Spink_Cam *the_cam_p=NULL;	// should this be per-thread?
				// no need yet...

// local prototypes

#define UNIMP_MSG(whence)						\
									\
	snprintf(ERROR_STRING,LLEN,					\
		"%s:  function not implemented yet!?",whence);		\
	WARN(ERROR_STRING);

#define NO_LIB_MSG(whence)						\
									\
	snprintf(ERROR_STRING,LLEN,					\
		"%s:  program built without libspinnaker support!?",whence);	\
	error1(ERROR_STRING);

#define EAT_ONE_DUMMY(whence)						\
									\
	const char *s;							\
	s=NAMEOF("dummy word");						\
	if( s != NULL ) s=NULL; /* silence compiler warning */		\
	NO_LIB_MSG(whence)


#define CHECK_CAM(whence)						\
									\
	if( the_cam_p == NULL ){					\
		snprintf(ERROR_STRING,LLEN,				\
				"%s:  No spink_cam selected.",#whence);	\
		warn(ERROR_STRING); 					\
		return;							\
	}

static Spink_Map *curr_map_p=NULL;

static COMMAND_FUNC(do_list_spink_maps)
{
	list_spink_maps( tell_msgfile() );
}

void _select_spink_map(QSP_ARG_DECL  Spink_Map *skm_p)
{
	if( curr_map_p == skm_p ) return;	// nothing to do

	if( curr_map_p != NULL ){
		pop_map_contexts();
	}

	curr_map_p = skm_p;

	if( skm_p != NULL ){
		push_map_contexts(skm_p);
		insure_current_camera(skm_p->skm_skc_p);
	}
}

static COMMAND_FUNC(do_select_spink_map)
{
	Spink_Map *skm_p;

	skm_p = pick_spink_map("");
	if( skm_p == NULL ) return;

	select_spink_map(skm_p);
}

#define CHECK_CURRENT_MAP			\
	if( curr_map_p == NULL ){		\
		warn("No map selected!?");	\
		return;				\
	}

static COMMAND_FUNC(do_list_spink_nodes)
{
	CHECK_CURRENT_MAP

	snprintf(MSG_STR,LLEN,"\nNodes from %s:\n",curr_map_p->skm_name);
	list_nodes_from_map(curr_map_p);
}

static COMMAND_FUNC(do_all_nodes_info)
{
	CHECK_CURRENT_MAP

	print_map_tree(curr_map_p);
}

static COMMAND_FUNC(do_spink_node_info)
{
	Spink_Node *skn_p;

	CHECK_CURRENT_MAP

	skn_p = pick_spink_node("");
	if( skn_p == NULL ) return;

	snprintf(MSG_STR,LLEN,"Map %s, Node %s:",skn_p->skn_skm_p->skm_name,skn_p->skn_name);
	prt_msg(MSG_STR);

	print_spink_node_info(skn_p,0);
}

static COMMAND_FUNC(do_spink_node_terse_info)
{
	Spink_Node *skn_p;

	CHECK_CURRENT_MAP

	skn_p = pick_spink_node("");
	if( skn_p == NULL ) return;

	print_spink_node_info(skn_p,0);
}

#define get_dummy_input() _get_dummy_input(SINGLE_QSP_ARG)

static void _get_dummy_input(SINGLE_QSP_ARG_DECL)
{
	const char *s;
	s = nameof("input value (will not be used)");
	if( s != NULL ) s=NULL;	// silence compiler
}

#define set_node_from_user_input(skn_p) _set_node_from_user_input(QSP_ARG  skn_p)

static void _set_node_from_user_input(QSP_ARG_DECL  Spink_Node *skn_p)
{
	if( ! NODE_IS_WRITABLE(skn_p) ){
		snprintf(ERROR_STRING,LLEN,"Node %s is not writable!?",skn_p->skn_name);
		warn(ERROR_STRING);
		get_dummy_input();
		return;
	}
	(*(skn_p->skn_type_p->snt_set_func))(QSP_ARG  skn_p);
}

static COMMAND_FUNC(do_set_node)
{
	Spink_Node *skn_p;

	skn_p = pick_spink_node("");
	if( skn_p == NULL ){
		get_dummy_input();
		return;
	}
	set_node_from_user_input(skn_p);
}

static COMMAND_FUNC(do_list_cats)
{
	list_spink_cats( tell_msgfile() );
}

static COMMAND_FUNC(do_print_cat)
{
	Spink_Category *sct_p;

	sct_p = pick_spink_cat("");
	if( sct_p == NULL ) return;

	print_cat_tree(sct_p);
}



#define ADD_CMD(s,f,h)	ADD_COMMAND(node_menu,s,f,h)
MENU_BEGIN(node)
ADD_CMD(list_maps,do_list_spink_maps, list all node maps)
ADD_CMD(select_map,do_select_spink_map, select default map for node operations)
ADD_CMD(list_nodes,do_list_spink_nodes, list all nodes from current map)
ADD_CMD(info,do_spink_node_info, print information about a node)
ADD_CMD(terse_info,do_spink_node_terse_info, print node state with no header)
ADD_CMD(set,do_set_node, set the value of a node)
ADD_CMD(info_all,do_all_nodes_info, print information about a all nodes in current map)
ADD_CMD(list_categories,do_list_cats, list node categories in current map)
ADD_CMD(print_category,do_print_cat, print nodes in a category)
MENU_END(node)
#undef ADD_CMD

static COMMAND_FUNC(do_node_menu)
{
	CHECK_AND_PUSH_MENU(node);
}

static COMMAND_FUNC( do_init )
{
#ifdef HAVE_LIBSPINNAKER
	if( the_cam_p != NULL ){
		WARN("Firewire system already initialized!?");
		return;
	}

	if( init_spink_cam_system() < 0 )
		WARN("Error initializing Spinnaker system.");
#endif
}

#ifdef NOT_USED
static COMMAND_FUNC( do_list_spink_interfaces )
{
	prt_msg("Spinnaker interfaces:");
	list_spink_interfaces(tell_msgfile());
	prt_msg("");
}
#endif // NOT_USED

static COMMAND_FUNC( do_list_spink_cams )
{
	prt_msg("Spinnaker cameras:");
	list_spink_cams(tell_msgfile());
	prt_msg("");
}

static COMMAND_FUNC( do_cam_info )
{
	Spink_Cam *skc_p;

	skc_p = pick_spink_cam("camera");
	if( skc_p == NULL ) return;

	if( skc_p == the_cam_p ){
		snprintf(MSG_STR,LLEN,"%s is selected as current camera.",skc_p->skc_name);
		prt_msg(MSG_STR);
	}
#ifdef HAVE_LIBSPINNAKER
	print_spink_cam_info(skc_p);
#else
	NO_LIB_MSG("do_list_spink_cam");
#endif
}

static COMMAND_FUNC( do_select_cam )
{
	Spink_Cam *skc_p;

	skc_p = pick_spink_cam("camera");
	if( skc_p == NULL ) return;

	if( the_cam_p != NULL ) deselect_spink_cam(the_cam_p);
	the_cam_p = select_spink_cam(skc_p);
}

static COMMAND_FUNC( do_start )
{
	CHECK_CAM(start)
	spink_start_capture(the_cam_p);
}

static COMMAND_FUNC( do_grab )
{
	Data_Obj *dp;

	CHECK_CAM(grab)
	if( (dp=grab_spink_cam_frame(the_cam_p )) == NULL ){
		/* any error */
		// We might fail because we need to release a frame...
		// Don't shut down in that case.
		WARN("do_grab:  failed.");
	} else {
		char num_str[32];

		// BUG - we appear to be doing this twice...
		snprintf(num_str,32,"%d",the_cam_p->skc_newest);
		assign_reserved_var("newest",num_str);
	}

}

static COMMAND_FUNC( do_grab_newest )
{
	UNIMP_MSG("do_grab_newest");
}

static COMMAND_FUNC( do_stop )
{
	CHECK_CAM(stop)
	spink_stop_capture(the_cam_p );
}

static COMMAND_FUNC(do_reset)
{
	warn("reset not implemented!?");
}

// conflict started here???
static COMMAND_FUNC( do_release )
{
#ifdef HAVE_LIBSPINNAKER
	CHECK_CAM(release)
	release_oldest_spink_frame(the_cam_p);
#endif
}

static COMMAND_FUNC( do_close )
{
	//CHECK_CAM(close)
#ifdef HAVE_LIBSPINNAKER
	release_spink_cam_system();
#endif
	the_cam_p=NULL;
}

static COMMAND_FUNC( do_show_n_bufs )
{
	CHECK_CAM(show_n_buffers)

	show_n_buffers(the_cam_p);
}

static COMMAND_FUNC( do_set_n_bufs )
{
	int n;

	n=HOW_MANY("number of buffers");

	CHECK_CAM(set_n_buffers)

	if( n < MIN_N_BUFFERS ){
		snprintf(ERROR_STRING,LLEN,"do_set_n_bufs:  n (%d) must be >= %d",n,MIN_N_BUFFERS);
		WARN(ERROR_STRING);
	} else if ( n > MAX_N_BUFFERS ){
		snprintf(ERROR_STRING,LLEN,"do_set_n_bufs:  n (%d) must be <= %d",n,MAX_N_BUFFERS);
		WARN(ERROR_STRING);
	} else {
#ifdef HAVE_LIBSPINNAKER
		set_n_spink_buffers(the_cam_p, n);
#endif // HAVE_LIBSPINNAKER
	}
}

#undef ADD_CMD
#define ADD_CMD(s,f,h)	ADD_COMMAND(properties_menu,s,f,h)

#define MAX_CAMERAS	4

static COMMAND_FUNC( do_record )
{
	const char *s;
	int nf,nc;
	Image_File *ifp;
	Spink_Cam *skc_p_tbl[MAX_CAMERAS];
	int i;

	s=NAMEOF("name for raw volume recording");
	nf=HOW_MANY("total number of frames");
	nc=HOW_MANY("number of cameras");
	if( nc < 1 || nc > MAX_CAMERAS ){
		snprintf(ERROR_STRING,LLEN,"Number of cameras (%d) must be greater than zero and less than or equal to %d",
			nc,MAX_CAMERAS);
		warn(ERROR_STRING);
		return;
	}
	for(i=0;i<nc;i++){
		skc_p_tbl[i] = pick_spink_cam("");
	}
	for(i=0;i<nc;i++){
		if( skc_p_tbl[i] == NULL ) return;
		if( skc_p_tbl[i]->skc_n_buffers <= 0 ){
			snprintf(ERROR_STRING,LLEN,"record:  number of buffers for %s (%d) must be positive!?",
				skc_p_tbl[i]->skc_name, skc_p_tbl[i]->skc_n_buffers);
			warn(ERROR_STRING);
			return;
		}
	}
	// BUG here we should insist that all cameras are distinct!

	// For now, we write all data to a single file - but how to disentangle
	// data from multiple cameras???
	ifp = get_file_for_recording(s,nf*nc,skc_p_tbl[0]);
	if( ifp == NULL ) return;
	
	//CHECK_CAM(record)		// No! this does not use the_cam_p...

#ifdef HAVE_LIBSPINNAKER
	spink_stream_record(ifp, nf, nc, skc_p_tbl );
#else // ! HAVE_LIBSPINNAKER
	UNIMP_MSG("stream_record");
#endif // ! HAVE_LIBSPINNAKER
}

static COMMAND_FUNC(do_set_n_capture)
{
	int n;

	n = HOW_MANY("Number of frames to capture");

	CHECK_CAM(set_n_capture)	// insure the_cam_p is valid...

	if( n <= 0 ){
		snprintf(ERROR_STRING,LLEN,"set_n_capture:  count must be positive!?");
		warn(ERROR_STRING);
		return;
	}

	the_cam_p->skc_event_info.ei_next_frame = 0;
	the_cam_p->skc_event_info.ei_n_frames = n;
}

#undef ADD_CMD
#define ADD_CMD(s,f,h)	ADD_COMMAND(capture_menu,s,f,h)

MENU_BEGIN(capture)
//ADD_CMD( set_buffer_obj,	do_set_bufs,	specify sequence object to use for capture )
ADD_CMD( set_n_buffers,		do_set_n_bufs,		specify number of frames in the ring buffer )
ADD_CMD( show_n_buffers,	do_show_n_bufs,		show number of frames in the ring buffer )
ADD_CMD( set_n_capture,		do_set_n_capture,	set the number of frames to capture)
ADD_CMD( start,			do_start,	start capture )
ADD_CMD( grab,			do_grab,	grab a frame )
ADD_CMD( grab_newest,		do_grab_newest,	grab the newest frame )
ADD_CMD( release,		do_release,	release a frame )
ADD_CMD( record,		do_record,	record frames to disk )
ADD_CMD( stop,			do_stop,	stop capture )
MENU_END(capture)

static COMMAND_FUNC( do_captmenu )
{
	CHECK_AND_PUSH_MENU( capture );
}

#define CAM_P	the_cam_p->skc_cam_p

static COMMAND_FUNC( do_fmt7_list )
{
	UNIMP_MSG("fmt7_list");
}

static COMMAND_FUNC( do_fmt7_setsize )
{
	uint32_t w,h;

	w=HOW_MANY("width");
	h=HOW_MANY("height");

	CHECK_CAM(fmt7_setsize)

	/* Don't try to set the image size if capture is running... */

	if( IS_RUNNING(the_cam_p) ){
		WARN("can't set image size while camera is running!?");
		return;
	}
	UNIMP_MSG("set_fmt7_size");
	snprintf(ERROR_STRING,LLEN,"Can't set image size to %d x %d",w,h);
	advise(ERROR_STRING);
}

static COMMAND_FUNC( do_fmt7_setposn )
{
	uint32_t h,v;

	h=HOW_MANY("horizontal position (left)");
	v=HOW_MANY("vertical position (top)");

	CHECK_CAM(fmt7_setposn)

	// What are the constraints as to what this can be???
	// At least on the flea, the position has to be even...

	if( h & 1 ){
		snprintf(ERROR_STRING,LLEN,"Horizontal position (%d) should be even, rounding down to %d.",h,h&(~1));
		advise(ERROR_STRING);
		h &= ~1;
	}

	if( v & 1 ){
		snprintf(ERROR_STRING,LLEN,"Vertical position (%d) should be even, rounding down to %d.",v,v&(~1));
		advise(ERROR_STRING);
		v &= ~1;
	}

	UNIMP_MSG("fmt7_posn");
}

static COMMAND_FUNC( do_fmt7_select )
{
	UNIMP_MSG("fmt7_select");
}

#undef ADD_CMD
#define ADD_CMD(s,f,h)	ADD_COMMAND(format7_menu,s,f,h)

MENU_BEGIN(format7)
ADD_CMD( mode,		do_fmt7_select,		select format7 mode for get/set )
ADD_CMD( list,		do_fmt7_list,		list format7 settings )
ADD_CMD( set_image_size, do_fmt7_setsize,	set image size )
ADD_CMD( position,	do_fmt7_setposn,	set image position )
MENU_END(format7)

static COMMAND_FUNC( fmt7menu )
{
	CHECK_AND_PUSH_MENU( format7 );
}

static COMMAND_FUNC(do_quit_spinnaker)
{
	/*
	if( the_cam_p != NULL )
		pop_spink_cam_context(SINGLE_QSP_ARG);
		*/

	do_pop_menu(SINGLE_QSP_ARG);
}

static COMMAND_FUNC(do_get_cams)
{
	Data_Obj *dp;

	dp = pick_obj("string table");
	if( dp == NULL ) return;

	/*
	if( get_spink_cam_names( dp ) < 0 )
		WARN("Error getting camera names!?");
		*/
	warn("get_spink_cam_names not implemented!?");
}

#undef ADD_CMD
#define ADD_CMD(s,f,h)	ADD_COMMAND(chunks_menu,s,f,h)

static COMMAND_FUNC(do_list_chunks)
{
	list_chunk_datas( tell_msgfile() );
}

static COMMAND_FUNC(do_fetch_chunk)
{
	Chunk_Data *cd_p;
	Data_Obj *dp;
	const char *s;
	char buf[64];

	s = nameof("variable name");

	cd_p = pick_chunk_data("");
	dp = pick_obj("camera buffer");

	if( cd_p == NULL || dp == NULL ) return;

	fetch_chunk_data(cd_p,dp);
	//display_chunk_data(cd_p);
	format_chunk_data(buf,cd_p);
	assign_var(s,buf);
}

static COMMAND_FUNC(do_disp_chunk)
{
	Chunk_Data *cd_p;
	Data_Obj *dp;
	char buf[64];

	cd_p = pick_chunk_data("");
	dp = pick_obj("camera buffer");

	if( cd_p == NULL || dp == NULL ) return;

	fetch_chunk_data(cd_p,dp);
	format_chunk_data(buf,cd_p);

	snprintf(MSG_STR,LLEN,"\t%s:  ",cd_p->cd_name);
	prt_msg_frag(MSG_STR);
	prt_msg(buf);
}

static COMMAND_FUNC(do_enable_chunk)
{
	Chunk_Data *cd_p;

	cd_p = pick_chunk_data("");
	if( cd_p == NULL ) return;

	CHECK_CAM(enable_chunk)

	enable_chunk_data(the_cam_p,cd_p);
}


MENU_BEGIN(chunks)
ADD_CMD( list,		do_list_chunks,		list chunk data types)
ADD_CMD( enable,	do_enable_chunk,	enable chunk data)
ADD_CMD( fetch,		do_fetch_chunk,		fetch chunk data)
ADD_CMD( display,	do_disp_chunk,		display chunk data)
MENU_END(chunks)

static COMMAND_FUNC(do_chunk_menu)
{
	CHECK_AND_PUSH_MENU( chunks );
}

#undef ADD_CMD
#define ADD_CMD(s,f,h)	ADD_COMMAND(spinnaker_menu,s,f,h)

MENU_BEGIN(spinnaker)
ADD_CMD( init,		do_init,	initialize subsystem )
//ADD_CMD( list_interfaces,	do_list_spink_interfaces,	list interfaces )
ADD_CMD( list_cams,	do_list_spink_cams,	list cameras )
ADD_CMD( info,		do_cam_info,	print camera info )
ADD_CMD( nodes,		do_node_menu,	node submenu )
ADD_CMD( chunks,	do_chunk_menu,	chunk data submenu )
ADD_CMD( select,	do_select_cam,	select camera )
ADD_CMD( get_cameras,	do_get_cams,	copy camera names to an array )
ADD_CMD( capture,	do_captmenu,	capture submenu )
ADD_CMD( format7,	fmt7menu,	format7 submenu )
ADD_CMD( reset,		do_reset,	reset camera )
ADD_CMD( close,		do_close,	shutdown camera subsystem )
ADD_CMD( quit,		do_quit_spinnaker,	exit submenu )
MENU_SIMPLE_END(spinnaker)	// doesn't add quit command automatically

COMMAND_FUNC( do_spink_menu )
{
#ifdef FOOBAR
	if( the_cam_p != NULL )
		push_spink_cam_context(QSP_ARG  the_cam_p);
#endif // FOOBAR

	CHECK_AND_PUSH_MENU( spinnaker );
}

