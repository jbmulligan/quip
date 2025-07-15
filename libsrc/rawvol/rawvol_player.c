#include "quip_config.h"

#ifdef HAVE_RAWVOL
#ifdef HAVE_X11_EXT

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>		/* these two are for getpriority */
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "quip_prot.h"
#include "xsupp.h"
#include "rawvol.h"
#include "rv_api.h"
#include "fio_api.h"

#include "data_obj.h"

/* globals */

/* disk reader stuff */
static int32_t n_to_read;			/* number of bytes per write op */
static pthread_t dr_thr[MAX_DISKS];
static int read_frame_want;		/* index if the highest buf we want to fill */

#define MAX_VIEWERS	3
static int n_displayed_components=3;
static int display_component[MAX_VIEWERS]={0,1,2};

/* local prototypes */
static void start_dr_threads(QSP_ARG_DECL  int32_t nf,RV_Inode *inp,int ndisks, int *fd_arr);
static void * disk_reader(void *argp);

/* status codes */

#define DR_INIT		0
#define DR_WAIT		1
#define DR_READ		2
#define DR_BOT		3
#define DR_EXIT		4

#define RM_INIT		0
#define RM_TOP		1
#define RM_READY	2
#define RM_BOT		3
#define RM_EXIT		4


#define TRACE_FLOW

#ifdef TRACE_FLOW

static int m_status=0;
static char rstatstr[]="iwrbx";
static char rmstatstr[]="itrbx";

static char estring[MAX_DISKS][LLEN];

#define RSTATUS(code)						\
pip->ppi_status = code;						\
if( verbose ){							\
snprintf(estring[pip->ppi_index],LLEN,				\
"%c %c\t%d\t%c %d\t%c %d\t%c %d\t%c %d",			\
'0'+pip->ppi_index,rmstatstr[m_status],				\
read_frame_want,						\
rstatstr[ppi[0].ppi_status],					\
ppi[0].ppi_newest,					\
rstatstr[ppi[1].ppi_status],					\
ppi[1].ppi_newest,					\
rstatstr[ppi[2].ppi_status],					\
ppi[2].ppi_newest,					\
rstatstr[ppi[3].ppi_status],					\
ppi[3].ppi_newest);					\
NADVISE(estring[pip->ppi_index]);				\
}

#define RMSTATUS(code)						\
m_status = code;						\
if( verbose ){							\
snprintf(ERROR_STRING,LLEN,						\
"M %c\t%d\t%c %d\t%c %d\t%c %d\t%c %d",				\
rmstatstr[m_status],						\
read_frame_want,						\
rstatstr[ppi[0].ppi_status],					\
ppi[0].ppi_newest,					\
rstatstr[ppi[1].ppi_status],					\
ppi[1].ppi_newest,					\
rstatstr[ppi[2].ppi_status],					\
ppi[2].ppi_newest,					\
rstatstr[ppi[3].ppi_status],					\
ppi[3].ppi_newest);					\
NADVISE(ERROR_STRING);						\
}
#else /* ! TRACE_FLOW */

#define MSTATUS(code)		/* nop */
#define STATUS(code)		/* nop */
#define RSTATUS(code)		/* nop */
#define RMSTATUS(code)		/* nop */

#endif /* ! TRACE_FLOW */

#ifdef ALLOW_RT_SCHED
extern int rt_is_on;
#define YIELD_PROC(time)	{ if( rt_is_on ) sched_yield(); else usleep(time); }
#else /* ! ALLOW_RT_SCHED */
#define YIELD_PROC(time)	usleep(time);
#endif /* ALLOW_RT_SCHED */



#define MAXCAPT 100000
#define MAX_RINGBUF_FRAMES	300

/* These are the data to be passed to each disk reader thread */

typedef struct per_proc_info {
	int	ppi_index;		/* 0-MAXDISKS */
	int	ppi_fd;			/* file descriptor */
	int32_t	ppi_nf;			/* number of frames (not really per-proc) */
	int	ppi_flags;
	unsigned int	ppi_ndisks;
	int	ppi_newest;
	int	ppi_ccount[MAXCAPT];	/* capture count after each write */
	RV_Inode *	ppi_inp;
#ifdef THREAD_SAFE_QUERY
	Query_Stack *	ppi_qsp;
#endif // THREAD_SAFE_QUERY
#ifdef TRACE_FLOW
	int	ppi_status;		/* holds disk_writer state */
#endif /* TRACE_FLOW */
} Proc_Info;


static Viewer *rawvol_vp=NULL;
static int rawvol_increment=4;	// BUG this should be a movie parameter!?

static Proc_Info ppi[MAX_DISKS];

#define RAWVOL_VIEWER_NAME	"rawvol_viewer"

#define N_READ_BUFFERS	8
static char *rdfrmptr[N_READ_BUFFERS];

// This code was written for the meteor RGB, that's kind of irrelevant now...
int displaying_color=0;

void _play_rawvol_movie(QSP_ARG_DECL  Image_File *ifp)
{
	dimension_t frame_index;
	RV_Inode *inp;
	int ndisks;
	int fd_arr[MAX_DISKS];
	int height, width, depth;

	if( FT_CODE(IF_TYPE(ifp)) != IFT_RV ){
		WARN("Sorry, image files must be type rv for display in rawvol window");
		return;
	}
	inp = (RV_Inode *)ifp->if_hdr_p;
	ndisks = queue_rv_file(inp,fd_arr);

	/* get real size */
	width = OBJ_COLS(ifp->if_dp);
	height = OBJ_ROWS(ifp->if_dp);
	depth = OBJ_COMPS(ifp->if_dp);

	/* make sure that the rawvol window is available */

	rawvol_vp = init_shm_viewer(RAWVOL_VIEWER_NAME,width, height, depth);

	/* allocate two frame buffers */

	rdfrmptr[0] = (char*) getbuf( N_READ_BUFFERS * (depth * width * height ) );
	for(frame_index=1;frame_index<N_READ_BUFFERS;frame_index++)
		rdfrmptr[frame_index] = rdfrmptr[0] + frame_index*(depth*width*height);

	n_to_read = depth*width*height;	

	read_frame_want=N_READ_BUFFERS-1;	/* do some pre-fill */

RMSTATUS(RM_INIT);

	/* start disk reader threads */
	start_dr_threads(QSP_ARG  OBJ_FRAMES(ifp->if_dp),inp,ndisks,fd_arr);
	usleep(N_READ_BUFFERS*33000);		/* let buffers fill up */

	for(frame_index=0;frame_index<OBJ_FRAMES(ifp->if_dp);frame_index++){
		int which_buf;
		int disk_index;

RMSTATUS(RM_TOP);
		/* wait for this frame */
		disk_index = frame_index % ndisks;
		while( ppi[disk_index].ppi_newest < (int32_t) frame_index ){
			YIELD_PROC(100)
		}
RMSTATUS(RM_READY);

		which_buf = frame_index % N_READ_BUFFERS;

		/* BUG we need to have a non-shm version that will
		 * work over the network...
		 */

		update_shm_viewer(rawvol_vp,
			rdfrmptr[which_buf],
			rawvol_increment,0,
			rawvol_vp->vw_width,rawvol_vp->vw_height,
			0,0);

		read_frame_want++;
RMSTATUS(RM_BOT);
	}
RMSTATUS(RM_EXIT);
	givbuf(rdfrmptr[0]);

	/* now we need to update the seek pointer in the i/o file struct, so that seeking works! */
	ifp->if_nfrms = OBJ_FRAMES(ifp->if_dp);
} /* end play_rawvol_movie */

void _play_rawvol_frame(QSP_ARG_DECL  Image_File *ifp, uint32_t frame)
{
	int j;
	RV_Inode *inp;
	int ndisks;
	int fd_arr[MAX_DISKS];
	int disk_index = 0;
	int width, height, depth;
	int n_read;		/* number of bytes read */

advise("play_rawvol_frame");
	/* get real size */
	width = OBJ_COLS(ifp->if_dp);
	height = OBJ_ROWS(ifp->if_dp);
	depth = OBJ_COMPS(ifp->if_dp);


	if( FT_CODE(IF_TYPE(ifp)) != IFT_RV ){
		WARN("Sorry, image files must be type rv for display in rawvol window");
		return;
	}
	inp = (RV_Inode *) ifp->if_hdr_p;
	ndisks = queue_rv_file(inp,fd_arr);

	/* make sure that the rawvol window is available */

	rawvol_vp = init_shm_viewer(RAWVOL_VIEWER_NAME,width, height, depth);

	/* seek to the correct frame */

	/*
	rv_frame_seek(inp, frame);
	*/
advise("calling image_file_seek...");
	image_file_seek(ifp,frame);

	/* allocate the frame buffer */

	rdfrmptr[0] = (char*) getbuf( depth * width * height );

	n_to_read = depth*width*height;

	read_frame_want=frame;

	disk_index = frame % ndisks;

RMSTATUS(RM_INIT);

        /* why start threads when it's just one frame? */ 
        if( (n_read = read(fd_arr[disk_index],rdfrmptr[0],n_to_read)) != n_to_read ){
	  char tmpstr[LLEN];
	  snprintf(tmpstr,LLEN,
		  "read (frm %d, buf=0x%"PRIxPTR", fd=%d)",
		  frame,(uintptr_t)rdfrmptr[0],disk_index);
	  perror(tmpstr);
	  snprintf(tmpstr,LLEN, "%d requested, %d read",
		  n_to_read,n_read);
	  WARN(tmpstr);
	  return;
	}

	snprintf(ERROR_STRING,LLEN,"play_rawvol_frame:  playing frame: %d of %d",frame + 1, OBJ_FRAMES(ifp->if_dp));
	advise(ERROR_STRING);

	RMSTATUS(RM_TOP);
	/* wait for this frame */
	RMSTATUS(RM_READY);

	for(j=0;j<n_displayed_components;j++){
		update_shm_viewer(rawvol_vp, rdfrmptr[0]+display_component[j],
				rawvol_increment,0,
				rawvol_vp->vw_width,rawvol_vp->vw_height, 0,0);
	}
	
	RMSTATUS(RM_BOT);

	RMSTATUS(RM_EXIT);
	givbuf(rdfrmptr[0]);
} /* end play_rawvol_frame */



static void start_dr_threads(QSP_ARG_DECL  int32_t nf, RV_Inode *inp, int ndisks, int *fd_arr )
{
	int i;
	pthread_attr_t attr1;
	//struct rawvol_geomet gp;

	/* get the geometry values */
	//meteor_get_geometry(&gp);

	/* we used to set real time scheduling priority here...
	 * but the x server needs to run too!
	 * So we try making only the disk readers real time,
	 * and putting in long wait delays...
	 */

	pthread_attr_init(&attr1);	/* initialize to default values */
	pthread_attr_setinheritsched(&attr1,PTHREAD_INHERIT_SCHED);

	for(i=0;i<ndisks;i++){
		ppi[i].ppi_index=i;

		ppi[i].ppi_nf = nf;
		ppi[i].ppi_flags = 0;
		ppi[i].ppi_inp = inp;
		ppi[i].ppi_fd = fd_arr[i];
		ppi[i].ppi_newest= -1;
		ppi[i].ppi_ndisks= ndisks;
#ifdef THREAD_SAFE_QUERY
		ppi[i].ppi_qsp= THIS_QSP;
#endif // THREAD_SAFE_QUERY
#ifdef TRACE_FLOW
		ppi[i].ppi_status = DR_INIT;
#endif

		pthread_create(&dr_thr[i],&attr1,disk_reader,&ppi[i]);
	}
}

static void * disk_reader(void* argp)
{
	Proc_Info *pip;
	int fd;
	dimension_t j;
	/* cant use a statically allocated string,
	 * because each thread needs its own.
	 */
	char tmpstr[LLEN];
#ifdef THREAD_SAFE_QUERY
	Query_Stack *qsp;
#endif // THREAD_SAFE_QUERY

#ifdef ALLOW_RT_SCHED
	pid_t pid;
	struct sched_param p;

	pid = getpid();
	p.sched_priority = 1;
	if( sched_setscheduler(pid,SCHED_FIFO,&p) < 0 ){
		rt_is_on = 0;
	} else {
		rt_is_on = 1;
	}
#endif /* ALLOW_RT_SCHED */



	pip = (Proc_Info*) argp;
#ifdef THREAD_SAFE_QUERY
	qsp = pip->ppi_qsp;
#endif // THREAD_SAFE_QUERY

	fd = pip->ppi_fd;

	for(j=pip->ppi_index;(int32_t) j<pip->ppi_nf;j+=pip->ppi_ndisks){
		int n_read;		/* number of bytes read */
		char *buf;

		/* Wait until we are ready for this frame */

 		while( read_frame_want < (int32_t) j ){
RSTATUS(DR_WAIT);
			/* We don't use YIELD_PROC here, because
			 * we really want a delay!  The X server will
			 * not be running w/ realtime priority, so we
			 * can't hog the processor...
			 */

			usleep(10000);
		}

RSTATUS(DR_READ);
			

		buf = rdfrmptr[(j%N_READ_BUFFERS)];

		/* read in the next frame */
		/* We put this seek here in case there is timestamp info at the end of the last frame */
		rv_frame_seek(pip->ppi_inp,j);

		if( (n_read = read(fd,buf,n_to_read)) != n_to_read ){
			snprintf(tmpstr,LLEN,
				"read (frm %d, buf=0x%"PRIxPTR", fd=%d)",
				j,(uintptr_t)buf,fd);
			perror(tmpstr);
			snprintf(tmpstr,LLEN, "%d requested, %d read",
				n_to_read,n_read);
			NWARN(tmpstr);
			return(NULL);
		}

		/* Now we've read a frame.
		 * Let the parent know, so that when all threads
		 * have so signalled the frame can be released to
		 * the ring buffer.
		 */
		pip->ppi_newest = j;
RSTATUS(DR_BOT);
	}
RSTATUS(DR_EXIT);
	return(NULL);
}

Viewer *rawvol_viewer(void)
{
	return rawvol_vp;
}

#endif /* HAVE_X11_EXT */

#endif // HAVE_RAWVOL
