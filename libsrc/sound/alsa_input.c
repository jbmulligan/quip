#include "quip_config.h"

#ifdef HAVE_ALSA

#ifndef USE_OSS_SOUND

#include <stdio.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>		/* localtime() */
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ALSA_ASOUNDLIB_H
#include <alsa/asoundlib.h>
#endif

#include "quip_prot.h"
#include "sound.h"

/*
#define MIXER_NAME "/dev/mixer"
static int audio_stream_fd=(-1);
*/

/* #define N_SAMPS_PER_BUF 0x4000*/				/* 16K, about 1 second's worth */
/* #define N_SAMPS_PER_BUF	2048 */					/* 2K, about 12 msec second's worth */
/* #define N_SAMPS_PER_BUF	256 */

/* assume 40kHz sample rate, 400 samples is 10msec worth... */
/* #define N_SAMPS_PER_BUF		512 */
#define N_SAMPS_PER_BUF		2048

/* If 1 buffer is around 12 msec, then 80 buffers would be about 1 second */
/* #define N_BUFFERS	256 */		/* 3 sec? */
#define N_BUFFERS	(0x20000/N_SAMPS_PER_BUF)		/* 3 sec? */

static pthread_t audio_thr;
static pthread_t writer_thr;
static int n_record_channels=0;
static int streaming=0;

/* audio_reader sets newest to a nonnegative integer when
 * data is ready; we want to have enough buffers available that
 * the reader can get a bit ahead.  oldest gets set the first
 * time newest is set, and thereafter is updated only by disk_writer.
 */
static int active_buf=0;		/* currently receiving audion data */
static int wait_usecs=1;
static int newest=(-1), oldest=(-1);

static int halting=0;
static Data_Obj *audio_stream_dp=NULL;
static int audio_stream_fd=(-1);
static int timestamp_stream_fd=(-1);
static struct timeval tv_tbl[N_BUFFERS];

typedef struct sound_device {
	char *			sd_name;
	snd_pcm_t *		sd_capture_handle;
	snd_pcm_hw_params_t *	sd_hw_params;
} Sound_Device;

typedef struct {
	Query_Stack *	ara_qsp;
} Audio_Reader_Args;

static Sound_Device *the_sdp=NULL;
#define DEFAULT_SOUND_DEVICE		"default"	/* capture device */

//ITEM_INTERFACE_DECLARATIONS_STATIC( Sound_Device, snddev )
static Item_Type *snddev_itp=NULL;
static ITEM_INIT_FUNC(Sound_Device,snddev,0)
static ITEM_CHECK_FUNC(Sound_Device,snddev)
static ITEM_NEW_FUNC(Sound_Device,snddev)

#define init_snddevs()	_init_snddevs(SINGLE_QSP_ARG)
#define snddev_of(s)	_snddev_of(QSP_ARG  s)
#define new_snddev(s)	_new_snddev(QSP_ARG  s)



static Sound_Device * _init_sound_device(QSP_ARG_DECL  const char *devname);
#define init_sound_device(devname) _init_sound_device(QSP_ARG  devname)

static int _record_sound_to_obj(QSP_ARG_DECL  Data_Obj *dp, Sound_Device *sdp);
#define record_sound_to_obj(dp,sdp) _record_sound_to_obj(QSP_ARG  dp,sdp)

static int _init_sound_hardware(QSP_ARG_DECL  Sound_Device *sdp);
#define init_sound_hardware(sdp) _init_sound_hardware(QSP_ARG  sdp)

void set_sound_gain(QSP_ARG_DECL  int g)
{
	int recsrc;
	recsrc = 0;

#ifdef FOOBAR
	CHECK_AUDIO(AUDIO_RECORD);

	ioctl(mfd, SOUND_MIXER_READ_RECSRC, &recsrc);
	if(recsrc & SOUND_MASK_LINE) {
advise("sound recording souce is line...");
		ioctl(mfd, SOUND_MIXER_WRITE_LINE, &g);
	} else {
advise("sound recording souce is the microphone...");
		ioctl(mfd, SOUND_MIXER_WRITE_MIC, &g);
	}
#else
	warn("set_sound_gain:  not implemented for ALSA");
#endif /* FOOBAR */
}

#define select_input(mask ) _select_input(QSP_ARG  mask )

static void _select_input(QSP_ARG_DECL  int mask )
{
	int recsrc;
	recsrc = 0;

#ifdef FOOBAR
	CHECK_AUDIO(AUDIO_RECORD);

	if( ioctl(mfd, SOUND_MIXER_READ_RECSRC, &recsrc) < 0 ){
		tell_sys_error("ioctl(SOUND_MIXER_READ_RECSRC)");
		warn("error fetching input source");
		return;
	}
	recsrc &= ~(SOUND_MASK_LINE|SOUND_MASK_MIC);
	recsrc |= mask;
	if( ioctl(mfd, SOUND_MIXER_WRITE_RECSRC, &recsrc) < 0 ){
		tell_sys_error("ioctl(SOUND_MIXER_WRITE_RECSRC)");
		warn("error selecting microphone input");
	}
#else
	warn("select_input:  not yet implemented for ALSA");
#endif /* FOOBAR */
}

void select_mic_input(SINGLE_QSP_ARG_DECL)
{
	select_input(SOUND_MASK_MIC);
}

void select_line_input(SINGLE_QSP_ARG_DECL)
{
	select_input(SOUND_MASK_LINE);
}


double get_sound_seconds(QSP_ARG_DECL  Item *ip,dimension_t frame)
{
	Data_Obj *dp;
	u_long sec;
	struct tm tm1;
	Timestamp_Data *tm_p;

	dp = (Data_Obj *)ip;

	if( ! object_is_sound(dp) ) return(-1.0);

	/* convert time stamp to broken-down time */

	tm_p = (Timestamp_Data *)OBJ_DATA_PTR(dp);

	tm1.tm_sec = tm_p->tsd_sec;
	tm1.tm_min = tm_p->tsd_min;
	tm1.tm_hour = tm_p->tsd_hour;
	tm1.tm_mday = tm_p->tsd_mday;
	tm1.tm_mon = tm_p->tsd_mon;
	tm1.tm_year = tm_p->tsd_year;

	sec = mktime(&tm1);

	return((double)sec);
}

double get_sound_microseconds(QSP_ARG_DECL  Item *ip,dimension_t frame)
{
	Data_Obj *dp;
	u_long usec;
	Timestamp_Data *tm_p;

	dp = (Data_Obj *)ip;

	if( ! object_is_sound(dp) ) return(-1.0);

	tm_p = (Timestamp_Data *)OBJ_DATA_PTR(dp);

	usec = tm_p->tsd_csec;
	usec *= 100;
	usec += tm_p->tsd_ccsec;
	usec *= 100;
	usec += tm_p->tsd_usec;

	return((double)usec);
}

double get_sound_milliseconds(QSP_ARG_DECL  Item *ip,dimension_t frame)
{
	if( ! object_is_sound((Data_Obj *)ip) ) return(-1.0);
	return( get_sound_microseconds(QSP_ARG  ip,frame) / 1000.0 );
}

void halt_rec_stream(SINGLE_QSP_ARG_DECL)
{
	if( halting ){
		warn("halt_rec_stream:  already halting!?");
		return;
	}

	if( !streaming ){
		warn("halt_rec_stream:  not streaming!?");
		return;
	}

	halting=1;

	/* wait for disk writer to finish - should call pthread_join! */

	/*
	while( streaming && audio_stream_fd != (-1) )
		usleep(100000);
		*/

	if( pthread_join(audio_thr,NULL) < 0 )
		warn("halt_rec_stream:  error in pthread_join");

	streaming = 0;
	halting = 0;
}

void set_stereo_input(QSP_ARG_DECL  int is_stereo) { warn("unimplemented for ALSA:  set_stereo_input"); }

void _record_sound(QSP_ARG_DECL  Data_Obj *dp)
{
	CHECK_AUDIO(AUDIO_RECORD);

	if( the_sdp == NULL ){
		the_sdp = init_sound_device(DEFAULT_SOUND_DEVICE);
		if( the_sdp == NULL ) return;
	}
	record_sound_to_obj(dp,the_sdp);
}

#define FRAMES_PER_CHUNK	128		/* how should we set this?? */

#define setup_record(sdp) _setup_record(QSP_ARG  sdp)

static int _setup_record(QSP_ARG_DECL  Sound_Device *sdp)
{
	snd_pcm_uframes_t n_frames;
	int dir=0;
	int err;

	/* Set period size to 32 frames. */
	n_frames = FRAMES_PER_CHUNK;
	snd_pcm_hw_params_set_period_size_near(sdp->sd_capture_handle,
		sdp->sd_hw_params, &n_frames, &dir);

	if((err = snd_pcm_hw_params(sdp->sd_capture_handle, sdp->sd_hw_params)) < 0) {
		snprintf(ERROR_STRING,LLEN, "setup_record:  cannot set parameters (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	if((err = snd_pcm_prepare(sdp->sd_capture_handle)) < 0) {
		snprintf(ERROR_STRING,LLEN,"setup_record:  cannot prepare audio interface %s for use (%s)\n",
			sdp->sd_name,snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}
	return(0);
}

#define read_sound_frames(sdp,ptr,frames_remaining,frame_size) _read_sound_frames(QSP_ARG  sdp,ptr,frames_remaining,frame_size)

static int _read_sound_frames(QSP_ARG_DECL  Sound_Device *sdp, char *ptr, snd_pcm_uframes_t frames_remaining, int frame_size )
{
	snd_pcm_uframes_t frames_requested;
	int err;

//fprintf(stderr,"read_sound_frames: ptr = 0x%lx, frames_remaining = %d\n",(u_long)ptr,frames_remaining);
	frames_requested = 64;

	while(frames_remaining){
		frames_requested = frames_requested < frames_remaining ? frames_requested : frames_remaining;

//fprintf(stderr,"read_sound_frames: ptr = 0x%lx, frames_requested = %d\n",(u_long)ptr,frames_requested);
		if((err = snd_pcm_readi(sdp->sd_capture_handle, ptr, frames_requested)) != (int) frames_requested) {
			if( err == EPIPE ){		/* short read */
				warn("audio read overrun");
				return(-1);
			} else if( errno == EINTR ){
				if( verbose ){
					snprintf(ERROR_STRING,LLEN,"audio read error:  %s",snd_strerror(err));
					warn(ERROR_STRING);
				}
				frames_requested = 0;
			} else if( err < 0 ){
				snprintf(ERROR_STRING,LLEN,"audio read error:  %s",snd_strerror(err));
				warn(ERROR_STRING);
				return(-1);
			} else {
				if( verbose ){
					snprintf(ERROR_STRING,LLEN,"audio read, expected %d frames but got %d",(int)frames_requested,err);
					warn(ERROR_STRING);
				}
				frames_requested = err;
			}
		}

		ptr += frames_requested * frame_size;
		frames_remaining -= frames_requested;
	}
//fprintf(stderr,"read_sound_frames: ptr = 0x%lx  DONE\n",(u_long)ptr);
	return(0);
}

static int _record_sound_to_obj(QSP_ARG_DECL  Data_Obj *dp, Sound_Device *sdp)
{
	snd_pcm_uframes_t n;
	char *ptr;
	
	if( setup_record(sdp) < 0 ) return(-1);

	n = OBJ_N_TYPE_ELTS(dp)/OBJ_COMPS(dp);		/* assume tdim =2 if stereo... */
snprintf(ERROR_STRING,LLEN,"_record_sound_to_obj:  n_frames = %ld, tdim = %ld, size = %d",
n, (long)OBJ_COMPS(dp), PREC_SIZE(OBJ_MACH_PREC_PTR(dp)));
advise(ERROR_STRING);

	ptr = (char *)OBJ_DATA_PTR(dp);
	return read_sound_frames(sdp,ptr,n,OBJ_COMPS(dp)*PREC_SIZE(OBJ_MACH_PREC_PTR(dp)));
}

static int _init_sound_hardware(QSP_ARG_DECL  Sound_Device *sdp)
{
	int err,dir;
	u_int desired_rate;

	if((err = snd_pcm_hw_params_malloc(&sdp->sd_hw_params)) < 0) {
		snprintf(ERROR_STRING,LLEN, "cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	if((err = snd_pcm_hw_params_any(sdp->sd_capture_handle, sdp->sd_hw_params)) < 0) {
		snprintf(ERROR_STRING,LLEN, "cannot initialize hardware parameter structure (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	if((err = snd_pcm_hw_params_set_access(sdp->sd_capture_handle, sdp->sd_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		snprintf(ERROR_STRING,LLEN, "cannot set access type (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	if((err = snd_pcm_hw_params_set_format(sdp->sd_capture_handle, sdp->sd_hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		snprintf(ERROR_STRING,LLEN, "cannot set sample format (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	desired_rate=44100;
	dir=0;
	if((err = snd_pcm_hw_params_set_rate_near(sdp->sd_capture_handle, sdp->sd_hw_params, &desired_rate, &dir)) < 0) {
		snprintf(ERROR_STRING,LLEN, "cannot set sample rate (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	n_record_channels = 2;

	if((err = snd_pcm_hw_params_set_channels(sdp->sd_capture_handle, sdp->sd_hw_params, n_record_channels)) < 0) {
		snprintf(ERROR_STRING,LLEN, "cannot set channel count (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	if((err = snd_pcm_hw_params(sdp->sd_capture_handle, sdp->sd_hw_params)) < 0) {
		snprintf(ERROR_STRING,LLEN, "cannot set parameters (%s)\n",
			snd_strerror(err));
		warn(ERROR_STRING);
		return(-1);
	}

	/* snd_pcm_hw_params_free(hw_params); */

	return(0);
}

static Sound_Device * _init_sound_device(QSP_ARG_DECL  const char *devname)
{
	Sound_Device *sdp;
	int err;

	sdp = snddev_of(devname);
	if( sdp != NULL ){
		snprintf(ERROR_STRING,LLEN,"init_sound_device:  device %s is already initialized",devname);
		warn(ERROR_STRING);
		return(sdp);
	}

	sdp = new_snddev(devname);
	if( sdp == NULL ){
		snprintf(ERROR_STRING,LLEN,"init_sound_device:  unable to create struct for device %s",devname);
		warn(ERROR_STRING);
		return(NULL);
	}

	if((err = snd_pcm_open(&sdp->sd_capture_handle, devname, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		snprintf(ERROR_STRING,LLEN, "init_sound_device:  cannot open audio device %s (%s)\n", 
			devname,
			snd_strerror(err));
		warn(ERROR_STRING);
		return(NULL);
	}

	if( init_sound_hardware(sdp) < 0 ){
		snprintf(ERROR_STRING,LLEN,"init_sound_device:  Unable to initialize sound hardware for device %s",sdp->sd_name);
		warn(ERROR_STRING);
		/* BUG cleanup here */
		return(NULL);
	}

	/* snd_pcm_close(capture_handle); */

	return(sdp);
}

/* If we are streaming to disk, we assume that we will record asynchronously until a halt command
 * is given.  We check to see if we are stereo or mono, and create a pair of sound vectors accordingly.
 */

static Data_Obj * init_stream_obj(SINGLE_QSP_ARG_DECL)
{
	Dimension_Set ds1;

	ds1.ds_dimension[0] = n_record_channels;
	ds1.ds_dimension[1] = N_SAMPS_PER_BUF;
	ds1.ds_dimension[2] = N_BUFFERS;	/* for double buffering */
	ds1.ds_dimension[3] = 1;
	ds1.ds_dimension[4] = 1;

	audio_stream_dp = make_dobj("_audio_stream_obj",&ds1,PREC_FOR_CODE(PREC_IN));
	return( audio_stream_dp );
}

// disk writer is a separate thread, but should inherit a qsp from the caller

static  void *disk_writer(void *arg)
{
	short *ptr;
	int n_want, n_written;
	struct timeval *tvp;
	Query_Stack *qsp;

	qsp = arg;

	n_want = OBJ_COMPS(audio_stream_dp) * OBJ_COLS(audio_stream_dp) * sizeof(short);
	while(streaming){
		/* wait for first buffer */
		while( oldest == (-1) ){
			if( !streaming ) goto writer_done;
			usleep(wait_usecs);		/* one frame of data */
		}
		/* wait for an extra buffer to complete */
		while( oldest == newest ){
			if( !streaming ) goto writer_done;
			usleep(wait_usecs);		/* one frame of data */
		}


		ptr = (short *)OBJ_DATA_PTR(audio_stream_dp);
		ptr += oldest * OBJ_ROW_INC(audio_stream_dp);

		if( (n_written=write(audio_stream_fd,ptr,n_want)) < 0 ){
			tell_sys_error("write");
			warn("error writing audio stream file");
		} else if( n_written != n_want ){
			snprintf(ERROR_STRING,LLEN,"disk_writer:  %d audio bytes requested, %d actually written",
					n_want,n_written);
			warn(ERROR_STRING);
		}

		tvp = &tv_tbl[oldest];
		if( (n_written=write(timestamp_stream_fd,tvp,sizeof(*tvp))) < 0 ){
			tell_sys_error("write");
			warn("error writing audio timestamp stream file");
		} else if( n_written != sizeof(*tvp) ){
	snprintf(ERROR_STRING,LLEN,"disk_writer:  %d timestamp bytes requested, %d actually written",
				n_want,n_written);
			warn(ERROR_STRING);
		}

		oldest++;
		if( oldest >= N_BUFFERS )
			oldest = 0;
	}
writer_done:

	close(audio_stream_fd);
	delvec(audio_stream_dp);
	audio_stream_fd=(-1);

	close(timestamp_stream_fd);
	timestamp_stream_fd=(-1);

	return(NULL);
}

static void *audio_reader(void *arg)
{
	int warned_once=0;
	struct timeval tv;
	struct timezone tz;
	int framesize;
	snd_pcm_uframes_t frames_requested;
	Audio_Reader_Args *ara_p;
#ifdef THREAD_SAFE_QUERY
	Query_Stack *qsp;
#endif

	ara_p = arg;
#ifdef THREAD_SAFE_QUERY
	qsp = ara_p->ara_qsp;
#endif
	framesize = OBJ_COMPS(audio_stream_dp)*PREC_SIZE(OBJ_MACH_PREC_PTR(audio_stream_dp));
	frames_requested = OBJ_COLS(audio_stream_dp);

	while( !halting ){
		short *ptr;

		ptr = (short *) OBJ_DATA_PTR(audio_stream_dp);
		ptr += active_buf * OBJ_ROW_INC(audio_stream_dp);

		/* now fill this buffer with data */
		if( read_sound_frames(the_sdp,(char *)ptr,frames_requested,framesize) < 0 ){
			warn("error reading audio data");
			halting=1;
		}

		/* get a timestamp for this bufferful */
		if( gettimeofday(&tv,&tz) < 0 && ! warned_once ){
			perror("gettimeofday");
			warn("error getting time stamp for sound stream");
			warned_once++;
		}
		tv_tbl[ active_buf ] = tv;

		/* we hope that read doesn't return asynchronously!? */

		newest = active_buf;
		if( oldest < 0 ) oldest=newest;

		active_buf ++;
		if( active_buf >= N_BUFFERS ) active_buf=0;

		while( active_buf == oldest ){
			snprintf(ERROR_STRING,LLEN,"audio_reader:  disk writer not keeping up (active_buf = %d, oldest = %d)!?",active_buf,oldest);
			warn(ERROR_STRING);
			usleep(wait_usecs);	/* wait one buffer */
		}

		/* yield the processor here */
		usleep(10);

	}

	/* Wait for disk writer to finish */
	usleep(500000);

	/* We close the file here, otherwise the sound driver
	 * continues to record and buffer data.
	 */

	audio_init(QSP_ARG  AUDIO_UNINITED);

	return(NULL);

} /* end audio_reader */

static int stream_record_init(SINGLE_QSP_ARG_DECL)
{
	if( streaming ){
		warn("stream_record_init:  already streaming, need to halt before initiating another recording");
		return(-1);
	}

	if( init_stream_obj(SINGLE_QSP_ARG) == NULL ){
		warn("stream_record_init:  error creating audio stream object");
		return(-1);
	}
	streaming=1;
	return(0);
}


void record_stream(QSP_ARG_DECL  int sound_fd, int timestamp_fd)
{
	pthread_attr_t attr1;

	audio_stream_fd = sound_fd;
	timestamp_stream_fd = timestamp_fd;

	if( the_sdp == NULL ){
		the_sdp = init_sound_device(DEFAULT_SOUND_DEVICE);
		if( the_sdp == NULL ) return;
	}

	if( setup_record(the_sdp) < 0 ) return;

	if( stream_record_init(SINGLE_QSP_ARG) < 0 ){
		warn("error initializing stream recorder");
		return;
	}

	wait_usecs = 25 /* usecs per sample @ 40 kHz */ * N_SAMPS_PER_BUF;


	pthread_attr_init(&attr1);	/* initialize to default values */
	pthread_attr_setinheritsched(&attr1,PTHREAD_INHERIT_SCHED);
	pthread_create(&audio_thr,&attr1,audio_reader,NULL);
	pthread_create(&writer_thr,&attr1,disk_writer,THIS_QSP);
}


#endif /* ! USE_OSS_SOUND */

#endif /* HAVE_ALSA */
