
#include "quip_config.h"

#ifdef HAVE_METEOR

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>	/* strncasecmp() */
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>		/* kill */
#endif

//#ifdef HAVE_SYS_SIGNAL_H
//#include <sys/signal.h>
//#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

/* #include <sys/shm.h> */

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

/* #include "ioctl_meteor.h" */

#include "quip_prot.h"
#include "mmenu.h"
#include "pupfind.h"
#include "mmvi.h"
#include "gmovie.h"
#include "fio_api.h"
#include "rv_api.h"
#include "fg_routines.h"

#if defined(PAL)
#define MAXROWS 576
#define MAXCOLS 768
#define DEFAULT_FORMAT METEOR_FMT_PAL
#define MAXFPS 25
#elif defined(SECAM)
#define MAXROWS 576
#define MAXCOLS 768
#define DEFAULT_FORMAT METEOR_FMT_SECAM
#define MAXFPS 25
#else /* NTSC */
#define MAXROWS 480
#define MAXCOLS 640
#define DEFAULT_FORMAT METEOR_FMT_NTSC
#define MAXFPS 30
#endif
#define HIMEM_FG

char *mmbuf=NULL;

//static char DefaultName[]="/dev/meteor0";/*MRA changed device from mmetfgrab0 */

#ifdef USE_SIGS
static struct sigaction sigact = 
((struct sigaction){ sa_handler: gotframe,
		     sa_mask: 0
/*		     sa_flags: SA_RESTART */ });
#endif


static int grab_signal=SIGUSR1;	/* was SIGUSR2?? */


/* local prototypes */


Movie_Module meteor_movie_module = {
	"meteor",
	meteor_setup_movie,
	meteor_add_frame,
	meteor_end_assemble,
	meteor_record,
	meteor_monitor,

	do_meteor_menu,
	meteor_movie_info,
	meteor_init,

	meteor_open_movie,
	meteor_setup_play,
	meteor_play_movie,
	meteor_wait_play,
	meteor_reverse_movie,
	meteor_get_frame,
	meteor_get_field,
	meteor_get_framec,
	meteor_get_fieldc,
	meteor_close_movie,
	meteor_shuttle_movie
};

static void meteor_set_input(int which_input)
{
#ifdef HAVE_X11_EXT
	if( which_input == METEOR_INPUT_DEV_RCA )
		displaying_color=1;
#endif /* HAVE_X11_EXT */

#ifndef FAKE_METEOR_HARDWARE
	if (ioctl(meteor_fd, METEORSINPUT, &which_input) < 0)
		perror("ioctl Setinput failed");

	if(which_input==METEOR_INPUT_DEV_RGB && my_geo.oformat==METEOR_GEO_RGB16){
		/* set RGB section for 15 bit colour */
		u_short s = (0x4 << 4);
		if (ioctl(meteor_fd, METEORSBT254, &s))
			perror("ioctl SetBt254 failed");
	}
#endif
}

#define N_METEOR_INPUTS	7

static const char *input_names[N_METEOR_INPUTS]={
	"rgb",
	"rca",
	"svideo",
	"dev0",
	"dev1",
	"dev2",
	"dev3"
};

static COMMAND_FUNC( do_meteor_set_input )
{
	int i;

	i=WHICH_ONE("input source",N_METEOR_INPUTS,input_names);
	if( i < 0 ) return;

	switch(i){
		case 0:  meteor_set_input(METEOR_INPUT_DEV_RGB); break;
		case 1:  meteor_set_input(METEOR_INPUT_DEV_RCA); break;
		case 2:  meteor_set_input(METEOR_INPUT_DEV_SVIDEO); break;
		case 3:  meteor_set_input(METEOR_INPUT_DEV0); break;
		case 4:  meteor_set_input(METEOR_INPUT_DEV1); break;
		case 5:  meteor_set_input(METEOR_INPUT_DEV2); break;
		case 6:  meteor_set_input(METEOR_INPUT_DEV3); break;
		default:
			assert( ! "bad meteor input" );
			break;
	}
}

static int meteor_get_input()
{
	int which_input;

	if (ioctl(meteor_fd, METEORGINPUT, &which_input) < 0){
		perror("ioctl Getinput failed");
		return(-1);
	}
	return(which_input);
}

static COMMAND_FUNC( do_meteor_get_input )
{
	int w,i;

	w=meteor_get_input();
	switch(w){
		case METEOR_INPUT_DEV_RGB:  i=0; break;
		/* dev0 is the same as rca!? */
		/* case METEOR_INPUT_DEV0:  i=3; break; */
		case METEOR_INPUT_DEV_RCA:  i=1; break;
		case METEOR_INPUT_DEV_SVIDEO:  i=2; break;
		case METEOR_INPUT_DEV1:  i=4; break;
		case METEOR_INPUT_DEV2:  i=5; break;
		case METEOR_INPUT_DEV3:  i=6; break;
		default:
			assert( ! "invalid meteor input" );
			return;
	}

	snprintf(msg_str,LLEN,"Meteor input is %s",input_names[i]);
	prt_msg(msg_str);
}

static void setup_meteor_device(SINGLE_QSP_ARG_DECL)
{
	if ( fg_open(SOURCE_NTSC, METEOR_GEO_RGB24, KERNEL_RAM) < 0) {
		perror("fg_open()");
		exit(-1);
	}
}

static void meteor_set_iformat(int c)
{
#ifndef FAKE_METEOR_HARDWARE
	int error = 0;

	error = ioctl(meteor_fd, METEORSFMT, &c);
	if(error < 0)
		perror("ioctl SetFormat failed");
#endif
}

#define N_METEOR_FORMATS	4

static const char *fmt_list[N_METEOR_FORMATS]={
	"ntsc",
	"pal",
	"secam",
	"auto"
};

static COMMAND_FUNC( do_meteor_set_iformat )
{
	int i;

	i=WHICH_ONE("video format",N_METEOR_FORMATS,fmt_list);
	if( i < 0 ) return;
	switch(i){
		case 0:  meteor_set_iformat(METEOR_FMT_NTSC); break;
		case 1:  meteor_set_iformat(METEOR_FMT_PAL); break;
		case 2:  meteor_set_iformat(METEOR_FMT_SECAM); break;
		case 3:  meteor_set_iformat(METEOR_FMT_AUTOMODE); break;
		default:
			assert( ! "bad format selection");
			break;
	}
}

static COMMAND_FUNC( do_meteor_get_iformat )
{
	int c,i=(-1);

	if (ioctl(meteor_fd, METEORGFMT, &c) < 0){
		perror("ioctl GetFormat failed");
		return;
	}
	switch(c){
		case METEOR_FMT_NTSC:  i=0; break;
		case METEOR_FMT_PAL:  i=1; break;
		case METEOR_FMT_SECAM:  i=2; break;
		case METEOR_FMT_AUTOMODE:  i=3; break;
		default:
			assert( ! "unrecognized format code");
			break;
	}
	if( i < 0 ) return;

	snprintf(ERROR_STRING,LLEN,"Format is %s",fmt_list[i]);
	advise(ERROR_STRING);
}

	
#ifdef USE_SIGS
static void meteor_install_handler()
{
	if ( sigaction(grab_signal, &sigact, NULL) )
		perror("sigaction failed");
}
#endif


static COMMAND_FUNC( kill_sig )
{
	pid_t pid;

	pid=getpid();
advise("killing");
	kill(pid,grab_signal);
}

static COMMAND_FUNC( do_getc )
{
	FILE *fp;
	int c;
	int n_eof=0;

	fp=try_open("/dev/tty","r");
	if( !fp ) return;

	c=(-1);
	while( c!=EOF && c!='q' && c!=4 ){
		c=fgetc(fp);
		if( c!=EOF ){
			snprintf(ERROR_STRING,LLEN,"char is 0x%x",c);
			advise(ERROR_STRING);
			c &= 0xff;
			snprintf(ERROR_STRING,LLEN,"char is 0x%x",c);
			advise(ERROR_STRING);
			kill_sig(SINGLE_QSP_ARG);
			n_eof=0;
		} else n_eof++;
		c &= 0xff;
	}

	fclose(fp);
	snprintf(ERROR_STRING,LLEN,"%d EOF's detected\n",n_eof);
	advise(ERROR_STRING);
}

static COMMAND_FUNC( do_make_frame_obj )
{
	const char *s;
	int i;
	Data_Obj *dp;

	s=NAMEOF("name for this frame object");
	i=HOW_MANY("frame index");

	dp = make_frame_object(s,i);

	/* WHY do we do this here??? */
	/* Probably just to make things easy for the user? */
	/*
	setup_monitor_capture();
	*/
}

static COMMAND_FUNC( do_close_dev )
{
	if( mmbuf != NULL ){
		if( munmap(mmbuf,last_mm_size) < 0 ){
			perror("do_close_dev:  munmap");
		}
	}

	fg_close();
}

/* These timestamps are added by the driver, and make the frame a bit bigger on rv disk...
 */

void _enable_meteor_timestamps(QSP_ARG_DECL  uint32_t flag)
{
	uint32_t arg;

	/* first check previous status */

	if( ioctl(meteor_fd,METEORGTS,&arg) < 0 ){
		perror("ioctl METEORGTS");
		WARN("error determining meteor timestamp status");
		return;
	}

	if( arg ){
		if( flag ){
			if( verbose )
				advise("enable_meteor_timestamps:  timestamps are already enabled");
			return;
		} else if( verbose ){
			advise("enable_meteor_timestamps:  timestamps were disabled, enabling...");
		}
	} else {
		if( ! flag ){
			if( verbose )
				advise("enable_meteor_timestamps:  timestamps are already disabled");
			return;
		} else if( verbose ){
			advise("enable_meteor_timestamps:  timestamps were enabled, disabling...");
		}
	}

	if( ioctl(meteor_fd,METEORSTS,&flag) < 0 ){
		perror("ioctl METEORSTS");
		WARN("error setting meteor timestamp flag");
	}

	/* requesting timestamps may increase the size of the per frame buffer (if it pushed it
	 * over a block boundary), so at this point the safe thing to do is to redo all of our memory
	 * mapping.
	 */
	meteor_mmap(SINGLE_QSP_ARG);
}

static COMMAND_FUNC( do_set_timestamp )
{
	uint32_t arg;

	if( ioctl(meteor_fd,METEORGTS,&arg) < 0 ){
		perror("ioctl METEORGTS");
		WARN("error determining meteor timestamp status");
	} else if( verbose ){
		if( arg )
			advise("Meteor timestamps were enabled.");
		else
			advise("Meteor timestamps were NOT enabled.");
	}

	arg =  ASKIF("enable timestamping of meteor frames");
	enable_meteor_timestamps(arg);		/* enable timestamps in the driver */
}

static COMMAND_FUNC( do_compute_diff )
{
	int n,p,c;

	n=HOW_MANY("index of newest frame");
	p=HOW_MANY("index of previous frame");
	c=HOW_MANY("index of component");
	compute_diff_image(n,p,c);
}

static COMMAND_FUNC( do_compute_curv )
{
	int n;

	n=HOW_MANY("index of newest frame");
	compute_curvature(n);
}

static COMMAND_FUNC( do_setup_blur )
{
	Data_Obj *dp;

	dp = pick_obj("impulse response");
	if( dp == NULL ) return;

	setup_blur(dp);
}

#define ADD_CMD(s,f,h)	ADD_COMMAND(pupfind_menu,s,f,h)

MENU_BEGIN(pupfind)
ADD_CMD( setup_diff,		setup_diff_computation,	initialize internal variables )
ADD_CMD( compute_diff,		do_compute_diff,	compute difference image )
ADD_CMD( setup_curv,		setup_curv_computation,	initialize internal variables )
ADD_CMD( compute_curv,		do_compute_curv,	compute gaussian curvature )
ADD_CMD( setup_blur,		do_setup_blur,		specity impulse response )
ADD_CMD( blur_curvature,	blur_curvature,		blur curvature )
MENU_END(pupfind)

static COMMAND_FUNC( pf_menu )
{
	CHECK_AND_PUSH_MENU(pupfind);
}

#undef ADD_CMD
#define ADD_CMD(s,f,h)	ADD_COMMAND(meteor_menu,s,f,h)

MENU_BEGIN(meteor)
ADD_CMD( frame,		do_make_frame_obj,	make an object for a frame )
ADD_CMD( close,		do_close_dev,		close meteor devices )
ADD_CMD( geometry,	do_geometry,		geometry submenu )
ADD_CMD( video,		do_video_controls,	video controls )
ADD_CMD( capture,	do_capture,		capture submenu )
ADD_CMD( flow,		meteor_flow_menu,	image flow submenu )
ADD_CMD( set_input,	do_meteor_set_input,	select input source )
ADD_CMD( get_input,	do_meteor_get_input,	report current input source )
ADD_CMD( timestamp,	do_set_timestamp,	enable/disable timestamping )
#ifdef USE_SIGS
ADD_CMD( install_handler,	meteor_install_handler,	install signal handler )
#endif
ADD_CMD( set_iformat,	do_meteor_set_iformat,	set input format )
ADD_CMD( get_iformat,	do_meteor_get_iformat,	get input format )
ADD_CMD( kill,		kill_sig,		fake a capture signal )
ADD_CMD( getc,		do_getc,		try to get a character from the terminal file )
ADD_CMD( cap_tst,	do_captst,		capture test submenu )
ADD_CMD( pupil_finder,	pf_menu,		pupil finder submenu )
MENU_END(meteor)


void meteor_init(SINGLE_QSP_ARG_DECL)
{
	static int meteor_inited=0;

	if( meteor_inited ) return;

	//DevName = DefaultName;
	
	/* verbose=1; */
	setup_meteor_device(SINGLE_QSP_ARG);
	set_grab_depth(24);
	meteor_set_size(DEFAULT_METEOR_HEIGHT,DEFAULT_METEOR_WIDTH,DEFAULT_METEOR_FRAMES);
	meteor_set_iformat(DEFAULT_FORMAT);

	meteor_inited++;
}

COMMAND_FUNC( do_meteor_menu )
{
	static int inited=0;

	if( ! inited ){
advise("setting up default rawvol");
		if( insure_default_rv(SINGLE_QSP_ARG) >= 0 ){
			/* create movie structs for any existing rv files */
advise("initializing rawvol movies");
			init_rv_movies();
		} else {
			WARN("do_meteor_menu:  No default raw volume!?");
		}

advise("calling meteor init");
		meteor_init(SINGLE_QSP_ARG);
advise("back from meteor init");
		load_movie_module(&meteor_movie_module);

		inited=1;
	}

	CHECK_AND_PUSH_MENU(meteor);
}


#endif /* HAVE_METEOR */

