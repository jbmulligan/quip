#include "quip_config.h"

#include <stdio.h>

/* these next two includes used to be ifdef SGI */
/* For the old sgi system, we used iopen(), and O_DIRECT ... */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* close()  (on mac) */
#endif

#ifdef HAVE_STRING_H
#include <string.h>		/* strcmp() */
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "fio_prot.h"
#include "quip_prot.h"
#include "query_bits.h"	// LLEN - get rid of this!  BUG
#include "debug.h"
#include "getbuf.h"
#include "item_prot.h"
//#include "filetype.h"
#include "function.h"	/* prototype for add_sizable() */
#include "img_file.h"

#ifdef HAVE_PNG
#include "img_file/fio_png.h"
#else /* !HAVE_PNG */

#ifdef BUILD_FOR_IOS
#include "img_file/fio_png.h"
#endif // BUILD_FOR_IOS

#endif /* !HAVE_PNG */

#ifdef HAVE_TIFF
#include "img_file/fiotiff.h"
#endif /* HAVE_TIFF */

#include "img_file/wav.h"
//#include "fiojpeg.h"
#include "item_type.h"
//#include "raw.h"
//#include "uio.h"
#include "fileck.h"
#include "warn.h"
//#include "pathnm.h"

#ifdef HAVE_AVI_SUPPORT
#include "my_avi.h"
#endif /* HAVE_AVI_SUPPORT */

#ifdef HAVE_IOPEN
#include "glimage.h"			/* prototype for iopen() */
#endif /* HAVE_IOPEN */

#ifdef QUIP_DEBUG
debug_flag_t debug_fileio=0;
#endif /* QUIP_DEBUG */

static Filetype *curr_ftp=NULL;	// BUG set to default (was HIPS1)
static int no_clobber=1;		// BUG not thread-safe!?

static const char *iofile_directory=NULL;

ITEM_INTERFACE_DECLARATIONS(Image_File,img_file,0)


static int direct_flag=1;

#ifdef HAVE_GETTIMEOFDAY

static double get_if_seconds(QSP_ARG_DECL  Item *ip,dimension_t frame);
static double get_if_milliseconds(QSP_ARG_DECL  Item *ip,dimension_t frame);
static double get_if_microseconds(QSP_ARG_DECL  Item *ip,dimension_t frame);

static Timestamp_Functions if_tsf={
	{	get_if_seconds,
		get_if_milliseconds,
		get_if_microseconds
	}
};
#endif /* HAVE_GETTIMEOFDAY */

static Item_Type *file_type_itp=NULL;
ITEM_INIT_FUNC(Filetype,file_type,0)
ITEM_NEW_FUNC(Filetype,file_type)
ITEM_CHECK_FUNC(Filetype,file_type)
ITEM_PICK_FUNC(Filetype,file_type)
ITEM_ENUM_FUNC(Filetype,file_type)
//ITEM_LIST_FUNC(Filetype,file_type)	// added for debugging...

void set_direct_io(int flag)
{
	direct_flag=flag;
}

List *image_file_list(SINGLE_QSP_ARG_DECL)
{
	if( img_file_itp==NULL ) return(NULL);

	return( item_list(img_file_itp) );
}

static void update_pathname(Image_File *ifp)
{
	// BUG - should do nothing for rawvol files!?

	if( ifp->if_pathname != ifp->if_name ){
		rls_str((char *)ifp->if_pathname);
	}

	/* BUG? don't require UNIX delimiters... */

	if( iofile_directory != NULL && *ifp->if_name != '/' ){
		char str[LLEN];
		snprintf(str,LLEN,"%s/%s",iofile_directory,ifp->if_name);
		ifp->if_pathname = savestr(str);
	} else {
		ifp->if_pathname = ifp->if_name;
	}
}

void _set_iofile_directory(QSP_ARG_DECL  const char *dirname)
{
	if( !directory_exists(dirname) ){
		snprintf(ERROR_STRING,LLEN,
	"Directory %s does not exist or is not a directory", dirname);
		WARN(ERROR_STRING);
		return;
	}

	if( iofile_directory != NULL ){
		rls_str(iofile_directory);
	}

	iofile_directory = savestr(dirname);
}

static FIO_WT_FUNC( dummy )
{
	return(0);
}

static FIO_INFO_FUNC( null )
{}

static FIO_CONV_FUNC(null)
{ return(-1); }

static FIO_UNCONV_FUNC(null)
{ return(-1); }

static FIO_RD_FUNC( null ) {}
static FIO_OPEN_FUNC( null ) { return(NULL); }
static FIO_CLOSE_FUNC( null ) {}

static FIO_SEEK_FUNC(null)
{
	snprintf(ERROR_STRING,LLEN,"Sorry, can't seek on file %s",ifp->if_name);
	WARN(ERROR_STRING);
	return(-1);
}

#define DECLARE_FILETYPE(stem,code,flags)			\
								\
	ftp = new_file_type(#stem);				\
	SET_FT_CODE(ftp,code);					\
	SET_FT_FLAGS(ftp,flags);				\
	SET_FT_OPEN_FUNC(ftp,FIO_OPEN_FUNC_NAME(stem));		\
	SET_FT_READ_FUNC(ftp,FIO_RD_FUNC_NAME(stem));		\
	SET_FT_WRITE_FUNC(ftp,FIO_WT_FUNC_NAME(stem));		\
	SET_FT_CLOSE_FUNC(ftp,FIO_CLOSE_FUNC_NAME(stem));	\
	SET_FT_CONV_FUNC(ftp,FIO_CONV_FUNC_NAME(stem));		\
	SET_FT_UNCONV_FUNC(ftp,FIO_UNCONV_FUNC_NAME(stem));	\
	SET_FT_SEEK_FUNC(ftp,FIO_SEEK_FUNC_NAME(stem));		\
	SET_FT_INFO_FUNC(ftp,FIO_INFO_FUNC_NAME(stem));

#define network_open	null_open
#define network_rd	null_rd
#define network_wt	dummy_wt
#define network_close	null_close
#define _network_conv	_null_conv
#define _network_unconv	_null_unconv
#define network_seek	null_seek
#define network_info_func	null_info_func
#define network_seek_frame	null_seek_frame

#define raw_close	_generic_imgfile_close
#define raw_seek	uio_seek
#define raw_info_func	null_info_func
#define raw_seek_frame	uio_seek_frame

#define hips1_rd		raw_rd
#define hips1_info_func		null_info_func
#define hips1_seek_frame	uio_seek_frame

#define hips2_info_func		null_info_func
#define hips2_seek_frame	_std_seek_frame

#define bmp_wt		dummy_wt
#define bmp_seek_frame	null_seek_frame

static void create_filetypes(SINGLE_QSP_ARG_DECL)
{
	Filetype *ftp;

	//init_filetypes(SINGLE_QSP_ARG);		// init the item type

	DECLARE_FILETYPE(network,IFT_NETWORK,0)
	DECLARE_FILETYPE(raw,IFT_RAW,USE_UNIX_IO|CAN_DO_FORMAT)
	DECLARE_FILETYPE(hips1,IFT_HIPS1,USE_UNIX_IO|CAN_DO_FORMAT)
	DECLARE_FILETYPE(hips2,IFT_HIPS2,USE_STDIO|CAN_DO_FORMAT)
	DECLARE_FILETYPE(ppm,IFT_PPM,USE_STDIO|CAN_DO_FORMAT)
	DECLARE_FILETYPE(ascii,IFT_ASC,USE_STDIO|CAN_DO_FORMAT)
	DECLARE_FILETYPE(wav,IFT_WAV,USE_STDIO|CAN_DO_FORMAT)

#ifdef HAVE_RAWVOL
	DECLARE_FILETYPE(rvfio,IFT_RV,CAN_DO_FORMAT)
#endif // HAVE_RAWVOL

#ifdef HAVE_JPEG_SUPPORT
	DECLARE_FILETYPE(jpeg,IFT_JPEG,USE_STDIO|CAN_DO_FORMAT)
	DECLARE_FILETYPE(lml,IFT_LML,USE_STDIO|CAN_DO_FORMAT)
#endif /* HAVE_JPEG_SUPPORT */

#ifdef HAVE_PNG
	DECLARE_FILETYPE(pngfio,IFT_PNG,USE_STDIO|CAN_DO_FORMAT)
#else /* !HAVE_PNG */
#ifdef BUILD_FOR_IOS
	DECLARE_FILETYPE(pngfio,IFT_PNG,CAN_DO_FORMAT)

#endif // BUILD_FOR_IOS
#endif /* !HAVE_PNG */

#ifdef HAVE_TIFF
	DECLARE_FILETYPE(tiff,IFT_TIFF,CAN_DO_FORMAT)
#endif /* HAVE_TIFF */

#ifdef HAVE_MATIO
	DECLARE_FILETYPE(mat,IFT_MATLAB,CAN_DO_FORMAT)
#endif /* HAVE_MATIO */

#ifdef HAVE_AVI_SUPPORT
	DECLARE_FILETYPE(avi,IFT_AVI,CAN_DO_FORMAT)
#endif /* HAVE_AVI_SUPPORT */

	DECLARE_FILETYPE(bmp,IFT_BMP,USE_STDIO|CAN_READ_FORMAT)
}

static double get_if_size(QSP_ARG_DECL  Item *ip,int index)
{
	Image_File *ifp;

	ifp = (Image_File *)ip;
	assert( ifp->if_dp != NULL );

	return( get_dobj_size(ifp->if_dp,index) );
}

static const char * get_if_prec_name(QSP_ARG_DECL  Item *ip)
{
	Image_File *ifp;

	ifp = (Image_File *)ip;
	assert( ifp != NULL );
	assert( ifp->if_dp != NULL );

	return( get_dobj_prec_name(ifp->if_dp) );
}

static Size_Functions imgfile_sf={
		get_if_size,
		get_if_prec_name
};

static double get_if_il_flg(QSP_ARG_DECL  Item *ip)
{
	Image_File *ifp;

	ifp = (Image_File *)ip;
	assert( ifp->if_dp != NULL );

	return( get_dobj_il_flg(ifp->if_dp) );
}

static Interlace_Functions imgfile_if={
		get_if_il_flg
};


// image_file_init initializes the fileio subsystem

void image_file_init(SINGLE_QSP_ARG_DECL)
{
	static int inited=0;

	if( inited ) return;

#ifdef QUIP_DEBUG
	debug_fileio = add_debug_module("fileio");
#endif /* QUIP_DEBUG */

	/* This may have already been called - e.g. mmvi */
	if( img_file_itp == NULL )
		//img_file_init(SINGLE_QSP_ARG);
		init_img_files();

	create_filetypes(SINGLE_QSP_ARG);	// used to be a static table

	// Set the default filetype
	curr_ftp = FILETYPE_FOR_CODE(IFT_HIPS2);

#ifdef HAVE_GETTIMEOFDAY
	add_tsable( img_file_itp, &if_tsf, (Item * (*)(QSP_ARG_DECL  const char *))_img_file_of);
#endif

	add_sizable(img_file_itp,&imgfile_sf,NULL);
	add_interlaceable(img_file_itp,&imgfile_if,NULL);

	//setstrfunc("iof_exists",iof_exists);
	DECLARE_STR1_FUNCTION(	iof_exists,	iof_exists )

	define_port_data_type(P_IMG_FILE,"image_file","name of image file",
		_recv_img_file,
		/* null_proc, */
		(const char *(*)(QSP_ARG_DECL  const char *))_pick_img_file,
		(void (*)(QSP_ARG_DECL  Port *,const void *,int)) _xmit_img_file);

	inited=1;
}

/*
 * Delete the file struct, assume the file itself has already
 * been closed, if necessary.
 */

void _delete_image_file(QSP_ARG_DECL  Image_File *ifp)
{
	// BUG - this can fail for a raw volume file because
	// of permissions mismatch...
	// We need to check BEFORE we get here...

	if( ifp->if_dp != NULL ){
		/* Do we know that the other resources are already freed? */
		/* If we are reading from a file, then this is a dummy object
		 * and should be freed... but what about writing???
		 */
fprintf(stderr,"releasing data object\n");
		givbuf(ifp->if_dp);
		ifp->if_dp = NULL;
	}
	if( ifp->if_extra_p != NULL ){
fprintf(stderr,"releasing extra\n");
		givbuf(ifp->if_extra_p);
		ifp->if_extra_p = NULL;
	}
	if( ifp->if_pathname != ifp->if_name ){
fprintf(stderr,"releasing pathname string\n");
		rls_str((char *)ifp->if_pathname);
	}
fprintf(stderr,"releasing ifp\n");
	del_img_file(ifp);	// frees the name

	/* don't free the struct pointer, it's marked available
	 * for reuse by del_item (called from del_img_file)...
	 */
}

/*
 * Close the file associated with this image file structure.
 * Also delete the image file structure.  May appear
 * as tabled close routine for simple filetypes, also
 * may be called from filetype-specific close routine.
 */

void _generic_imgfile_close(QSP_ARG_DECL  Image_File *ifp)
{
	if( USES_STDIO(ifp) ){
		if( ifp->if_fp != NULL ) {
			fclose(ifp->if_fp);
		}
	} else if( USES_UNIX_IO(ifp) ){
		if( ifp->if_fd != -1 ){
			close(ifp->if_fd);
		}
	}

	if( HAD_ERROR(ifp) && IS_WRITABLE(ifp) ){
		/* BUG this should only apply to file-system files */
		if( USES_STDIO(ifp) || USES_UNIX_IO(ifp) )
			unlink(ifp->if_pathname);	/* remove file */
	}
	delete_image_file(ifp);
}

static Data_Obj *new_temp_dobj(void)
{
	Data_Obj *dp;

	dp = getbuf(sizeof(Data_Obj));
	SET_OBJ_SHAPE( dp, ALLOC_SHAPE );

	return dp;
}

static void rls_temp_dobj(Data_Obj *dp)
{
	givbuf( OBJ_TYPE_INCS(dp) );
	givbuf( OBJ_MACH_INCS(dp) );
	givbuf( OBJ_TYPE_DIMS(dp) );
	givbuf( OBJ_MACH_DIMS(dp) );
	givbuf( OBJ_SHAPE(dp) );
	givbuf(dp);
}

void setup_dummy(Image_File *ifp)
{
	assert(ifp!=NULL);
	assert( ifp->if_dp == NULL );
	assert( ifp->if_extra_p == NULL );

	/* these are not in the database... */

	ifp->if_dp = new_temp_dobj();
	// BUG?  do we need to lock this object?

	SET_OBJ_NAME(ifp->if_dp, ifp->if_name);
	ifp->if_dp->dt_ap = NULL;
	ifp->if_dp->dt_parent = NULL;
	ifp->if_dp->dt_children = NULL;
	SET_OBJ_DATA_PTR(ifp->if_dp, NULL);
	SET_OBJ_FLAGS(ifp->if_dp, 0);

	SET_OBJ_PREC_PTR(ifp->if_dp,NULL);
}

int open_fd(QSP_ARG_DECL  Image_File *ifp)
{
	int o_direct=0;

#ifdef HAVE_RGB
	if(  FT_CODE(IF_TYPE(ifp))  == IFT_RGB )
		return(0);
#endif /* HAVE_RGB */

#ifdef HAVE_DIRECT_IO

	/* for SGI video disk files,
	 * use sgi extension "direct i/o"
	 * which causes the disk driver to dma directly to user space.
	 *
	 * This only works for a local disk, however,
	 * so we need to stat() the file first and find if it is local.
	 *
	 * BUG - this works ok for read files,
	 * but for writing a file, you can't stat()
	 * a file that doesn't exist!?
	 *
	 * We should get the directory and stat it...
	 */

	if(  FT_CODE(IF_TYPE(ifp))  == IFT_DISK ){
		if( IS_READABLE(ifp) ){
			struct statvfs vfsbuf;

			if( statvfs(ifp->if_pathname,&vfsbuf)< 0 ){
				snprintf(ERROR_STRING,LLEN,"statvfs (%s):",
					ifp->if_pathname);
				tell_sys_error(ERROR_STRING);
				warn("Couldn't determine fs type, not using O_DIRECT");
			} else {
				if( vfsbuf.f_flag & ST_LOCAL ){
					o_direct = O_DIRECT;
				} else {
					o_direct = 0;
				}
			}
		} else {
			/* BUG - should stat the directory... */
			if( direct_flag ){
advise("writing file using DIRECT_IO based on flag (NEED TO FIX!)");
advise(ifp->if_pathname);
				o_direct = O_DIRECT;
			}
		}
	}
		
retry:

#endif /* HAVE_DIRECT_IO */


	if( IS_READABLE(ifp) )
		ifp->if_fd = open(ifp->if_pathname,O_RDONLY|o_direct);
	else

		/* open read-write so can rewrite header nframes */

		ifp->if_fd = open(ifp->if_pathname,
					O_RDWR|O_CREAT|O_TRUNC|o_direct,0644);

	if( ifp->if_fd < 0 ){

#ifdef HAVE_DIRECT_IO
		/* can't do O_DIRECT across NFS */
		if( o_direct && errno==EINVAL ){
			o_direct = 0;
snprintf(ERROR_STRING,LLEN,"Couldn't open file \"%s\" with direct i/o.",
ifp->if_pathname);
warn(ERROR_STRING);
advise("retrying to open write file w/o DIRECT_IO");
			goto retry;
		}
#endif /* HAVE_DIRECT_IO */

		tell_sys_error("open");
		snprintf(ERROR_STRING,LLEN,
			"open_fd:  error getting descriptor for %s file %s",
			IS_READABLE(ifp)?"read":"write",ifp->if_pathname);
		warn(ERROR_STRING);
		return(-1);
	}
	return(0);
}

#ifdef HAVE_IOPEN
int ifp_iopen(Image_File *ifp)
{
snprintf(ERROR_STRING,LLEN,"name %s  rows %d cols %d",
ifp->if_name,OBJ_ROWS(ifp->if_dp),OBJ_COLS(ifp->if_dp));
advise(ERROR_STRING);

	/* BUG maybe this 3 can be set to 2 if tdim==1 */
	ifp->if_hd.rgb_ip = iopen(ifp->if_pathname,"w",VERBATIM(1),3,
		OBJ_ROWS(ifp->if_dp),OBJ_COLS(ifp->if_dp),OBJ_COMPS(ifp->if_dp));

	if( ifp->if_hd.rgb_ip == NULL ){
		snprintf(ERROR_STRING,LLEN,"Error iopening file %s",ifp->if_pathname);
		warn(ERROR_STRING);
		return(-1);
	} else return(0);
}
#endif /* HAVE_IOPEN */

int _open_fp(QSP_ARG_DECL  Image_File *ifp)
{
#ifndef HAVE_RGB

	if( IS_READABLE(ifp) ){
		ifp->if_fp = try_open(ifp->if_pathname,"r");
	} else {
		/* open read-write so we can read back
		 * the header if necessary...  (see hips2.c)
		 */
		ifp->if_fp = try_open(ifp->if_pathname,"w+");
	}
	if( ! ifp->if_fp ) return(-1);
	return(0);

#else /* HAVE_RGB */

	if( IS_READABLE(ifp) ){
		if(  FT_CODE(IF_TYPE(ifp))  == IFT_RGB ){
			ifp->if_hd.rgb_ip = iopen(ifp->if_pathname,"r");
			if( ifp->if_hd.rgb_ip == NULL ){
				snprintf(ERROR_STRING,LLEN,
			"open_fp:  error getting RGB descriptor for file %s",
					ifp->if_pathname);
				warn(ERROR_STRING);
				return(-1);
			} else return(0);
		} else {
			ifp->if_fp = try_open(ifp->if_pathname,"r");
		}
	} else {
		/* if .rgb defer the iopen until we know the image size */
		if(  FT_CODE(IF_TYPE(ifp))  != IFT_RGB ){
			/* open read-write so we can read back
			 * the header if necessary...  (see hips2.c)
			 */
			ifp->if_fp = try_open(ifp->if_pathname,"w+");
		}
	}
	if(  FT_CODE(IF_TYPE(ifp))  != IFT_RGB ){
		if( ! ifp->if_fp ) return(-1);
	}
	return(0);

#endif /* HAVE_RGB */
}

static int check_clobber(QSP_ARG_DECL  Image_File *ifp)
{
	const char *dir;

	dir=parent_directory_of(ifp->if_pathname);


	/* now see if the file already exists, and if
	 * our application is permitting clobbers, then
	 * check the file system permissions
	 */

	if( file_exists(ifp->if_pathname) ){
		if( no_clobber ){
			snprintf(ERROR_STRING,LLEN,
				"Not clobbering existing file \"%s\"",
				ifp->if_pathname);
			warn(ERROR_STRING);
			return(-1);
		} else if( !can_write_to(ifp->if_pathname) )
			return(-1);
	} else {
		if( !file_exists(dir) ){
			snprintf(ERROR_STRING,LLEN, "No directory \"%s\"!?", dir);
			warn(ERROR_STRING);
			return(-1);
		}
		/* We may have write permissions to the file even if we don't have
		 * write permissions on the directory, e.g. /dev/null
		 */
		if( !can_write_to(dir) ){
			snprintf(ERROR_STRING,LLEN, "Can't write to directory \"%s\"!?", dir);
			warn(ERROR_STRING);
			return(-1);
		}
	}

	return(0);
}

void image_file_clobber(int flag)
{
	if( flag ) no_clobber=0;
	else no_clobber=1;
}

/* Allocate a new file struct and initialize the basic fields.
 * This used to be done as part if img_file_creat, but equivalent
 * code in rv.c had omitted initialization of if_extra_p.  Better
 * to have a single routine to handle the initialization!
 */

Image_File * _init_new_img_file(QSP_ARG_DECL  const char *name, int rw){
	Image_File *ifp;
	ifp = new_img_file(name);

	ifp->if_flags = (short) rw;
	ifp->if_nfrms = 0;

	ifp->if_pathname = ifp->if_name;	/* default */

	ifp->if_dp = NULL;
fprintf(stderr,"Setting extra_p to NULL, ifp = %s\n",ifp->if_name);
	ifp->if_extra_p = NULL;	// most filetypes don't use this...

	return ifp;
}

/*
 * This routine creates and initializes an image file struct,
 * and then opens the file using a method appropriate to the file type.
 *
 * Typically called from module open routines.
 *
 * It used to be called image_file_open, but that was too confusing
 * given the existence of a different function called open_image_file...
 */

Image_File *_img_file_creat(QSP_ARG_DECL  const char *name,int rw,Filetype * ftp)
{
	Image_File *ifp;
	int had_error=0;

	if( rw == FILE_READ && CANNOT_READ(ftp) ){
		snprintf(ERROR_STRING,LLEN,"Sorry, don't know how to read %s files",
			FT_NAME(ftp));
		warn(ERROR_STRING);
		return(NULL);
	} else if( rw == FILE_WRITE && CANNOT_WRITE(ftp) ){
		snprintf(ERROR_STRING,LLEN,"Sorry, don't know how to write %s files",
			FT_NAME(ftp));
		warn(ERROR_STRING);
		return(NULL);
	}

	ifp = init_new_img_file(name,rw);
	if( ifp==NULL ) return(ifp);

	update_pathname(ifp);

	// what is the "dummy" used for, and when do we release it?
	if( IS_READABLE(ifp) ) setup_dummy(ifp);

	if( IS_WRITABLE(ifp) ){
		if( check_clobber(QSP_ARG  ifp) < 0 ){
			had_error=1;
			goto dun;
		}
	}

	ifp->if_ftp = ftp;

	if( USES_STDIO(ifp) ){
		if( open_fp(ifp) < 0 ) had_error=1;
	} else if( USES_UNIX_IO(ifp) ){
		if( open_fd(QSP_ARG  ifp) < 0 ) had_error=1;
	} else {
		// This used to be a CAUTIOUS error, but some
		// filetypes (tiff, matlab) have their own
		// open routines...  So we do - nothing?
	}

dun:
	if( had_error ){
		if( IS_READABLE(ifp) ){
			//givbuf(ifp->if_dp);
			rls_temp_dobj(ifp->if_dp);
		}
		del_img_file(ifp);
		/* BUG? should also rls_str(name) here???
		 * Or does del_item release the naem???
		 */
		return(NULL);
	}

	return(ifp);
}

int same_size(QSP_ARG_DECL  Data_Obj *dp,Image_File *ifp)
{
	if( ifp->if_dp == NULL ){
		snprintf(ERROR_STRING,LLEN,"No size/prec info about image file %s",
			ifp->if_name);
		WARN(ERROR_STRING);
		return(0);
	}

	if(
		( OBJ_ROWS(ifp->if_dp) != 0 && OBJ_ROWS(ifp->if_dp) != OBJ_ROWS(dp) )	||
		( OBJ_COLS(ifp->if_dp) != 0 && OBJ_COLS(ifp->if_dp) != OBJ_COLS(dp) )
		){

		snprintf(ERROR_STRING,LLEN,"size mismatch, object %s and file %s",
			OBJ_NAME(dp),ifp->if_name);
		WARN(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"      %-24s %-24s",
			OBJ_NAME(dp),ifp->if_name);
		advise(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"rows: %-24d %-24d",OBJ_ROWS(dp),
			OBJ_ROWS(ifp->if_dp));
		advise(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"cols: %-24d %-24d",OBJ_COLS(dp),
			OBJ_COLS(ifp->if_dp));
		advise(ERROR_STRING);
		return(0);
	}
	return(1);
}

int _same_type(QSP_ARG_DECL  Data_Obj *dp,Image_File *ifp)
{
	int retval=1;

	if( ifp->if_dp == NULL ){
		snprintf(ERROR_STRING,LLEN,"No size/prec info about image file %s",
			ifp->if_name);
		WARN(ERROR_STRING);
		return(0);
	}

	if( OBJ_PREC(dp) != OBJ_PREC(ifp->if_dp) ){
		/* special case for unsigned (hips doesn't record this) */
		if(
		    (OBJ_PREC(dp) == PREC_UDI && OBJ_PREC(ifp->if_dp) == PREC_DI) ||
		    (OBJ_PREC(dp) == PREC_UIN && OBJ_PREC(ifp->if_dp) == PREC_IN) ||
		    (OBJ_PREC(dp) == PREC_UBY && OBJ_PREC(ifp->if_dp) == PREC_BY)
		){
			/* it's ok */
		} else {
			snprintf(ERROR_STRING,LLEN,
	"Pixel format (%s) for file %s\n\tdoes not match object %s precision (%s)",
	PREC_NAME(OBJ_PREC_PTR(ifp->if_dp)),ifp->if_name,
	OBJ_NAME(dp),PREC_NAME(OBJ_PREC_PTR(dp)));
			WARN(ERROR_STRING);
			retval=0;
		}
	}

	if( OBJ_COMPS(dp) != OBJ_COMPS(ifp->if_dp) ){
		snprintf(ERROR_STRING,LLEN,
	"Pixel dimension (%d) for file %s\n\tdoes not match pixel dimension (%d) for object %s",
	OBJ_COMPS(ifp->if_dp),ifp->if_name,OBJ_COMPS(dp),OBJ_NAME(dp));
		WARN(ERROR_STRING);
		retval=0;
	}
	return(retval);
}

void copy_dimensions(Data_Obj *dpto,Data_Obj *dpfr)	/* used by write routines... */
{
	int i;

	for(i=0;i<N_DIMENSIONS;i++){
		SET_OBJ_TYPE_DIM(dpto,i,OBJ_TYPE_DIM(dpfr,i));
		SET_OBJ_TYPE_INC(dpto,i,OBJ_TYPE_INC(dpfr,i));
	}
	SET_OBJ_PREC_PTR(dpto, OBJ_PREC_PTR(dpfr));
	SET_OBJ_FLAGS(dpto, OBJ_FLAGS(dpfr));
	SET_OBJ_MAXDIM(dpto, OBJ_MAXDIM(dpfr) );
	SET_OBJ_MINDIM(dpto, OBJ_MINDIM(dpfr) );
	SET_OBJ_N_TYPE_ELTS(dpto,OBJ_N_TYPE_ELTS(dpfr) );
}

void _if_info(QSP_ARG_DECL  Image_File *ifp)
{
	snprintf(msg_str,LLEN,"File %s:",ifp->if_name);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tpathname:  %s:",ifp->if_pathname);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\t%s format (%d)",FT_NAME(IF_TYPE(ifp)),FT_CODE(IF_TYPE(ifp)) );
	prt_msg(msg_str);
	if( ifp->if_dp != NULL ){
		snprintf(msg_str,LLEN,"\t%s pixels",PREC_NAME(OBJ_PREC_PTR(ifp->if_dp)));
		prt_msg(msg_str);
		if( OBJ_SEQS(ifp->if_dp) > 1 ){
			snprintf(msg_str,LLEN,
			"\t%d sequences, ",OBJ_SEQS(ifp->if_dp));
			prt_msg_frag(msg_str);
		} else {
			snprintf(msg_str,LLEN,"\t");
			prt_msg_frag(msg_str);
		}

#define INFO_ARGS( n )	n , n==1?"":"s"

		snprintf(msg_str,LLEN,
		"%d frame%s, %d row%s, %d column%s, %d component%s",
			INFO_ARGS( OBJ_FRAMES(ifp->if_dp) ),
			INFO_ARGS( OBJ_ROWS(ifp->if_dp) ),
			INFO_ARGS( OBJ_COLS(ifp->if_dp) ),
			INFO_ARGS( OBJ_COMPS(ifp->if_dp) )
			);
		prt_msg(msg_str);
	}
	if( ifp->if_extra_p != NULL ){
		snprintf(msg_str,LLEN,"Extra info at 0x%lx",(long)ifp->if_extra_p);
		prt_msg(msg_str);
	}
else {
fprintf(stderr,"if_extra_p is NULL for %s\n",ifp->if_name);
}
	if( IS_READABLE(ifp) ){
		prt_msg("\topen for reading");
		snprintf(msg_str,LLEN,"\t%d frame%s already read",
			INFO_ARGS(ifp->if_nfrms));
		prt_msg(msg_str);
	} else if( IS_WRITABLE(ifp) ){
		prt_msg("\topen for writing");
		snprintf(msg_str,LLEN,"\t%d frame%s already written",
			INFO_ARGS(ifp->if_nfrms));
		prt_msg(msg_str);
	}
	else
		assert( AERROR("Wacky RW mode!?") );

	/* print format-specific info, if any */
	(*FT_INFO_FUNC(IF_TYPE(ifp)))(QSP_ARG  ifp);
}

/* typical usage:
 *	dump_image_file("foo.viff",IFT_VIFF,buffer,width,height,PREC_BY);
 */

void dump_image_file(QSP_ARG_DECL  const char *filename,Filetype *ftp,void *data,dimension_t width,dimension_t height,Precision *prec_p)
{
	Data_Obj *dp;
	Image_File *ifp;

	dp = new_temp_dobj();

	SET_OBJ_DATA_PTR(dp, data);
	SET_OBJ_NAME(dp, "dump_image");

	SET_OBJ_COMPS(dp,1);
	SET_OBJ_COMP_INC(dp,1);
	SET_OBJ_COLS(dp,width);
	SET_OBJ_PXL_INC(dp,1);
	SET_OBJ_ROWS(dp,height);
	SET_OBJ_ROW_INC(dp,(incr_t)width);
	SET_OBJ_FRAMES(dp,1);
	SET_OBJ_FRM_INC(dp,(incr_t)(width*height));
	SET_OBJ_SEQS(dp,1);
	SET_OBJ_SEQ_INC(dp,(incr_t)(width*height));

	SET_OBJ_N_TYPE_ELTS(dp,width*height);

	SET_OBJ_PREC_PTR(dp,prec_p);

	ifp=(*FT_OPEN_FUNC(ftp))(QSP_ARG  filename,FILE_WRITE);
	if( ifp == NULL ){
		rls_temp_dobj(dp);
		return;
	}
	write_image_to_file(ifp,dp);
	rls_temp_dobj(dp);
}

#ifdef FOOBAR

/*
 * this one should allocate it's own storage
 */

void *load_image_file(char *name,filetype_code input_file_type,filetype_code desired_hdr_type)
{
	Image_File *ifp;
	Data_Obj *dp;
	void *new_hdp;

	ifp=open_image_file(name,"r");
	if( ifp == NULL ) return(NULL);

	/*
	dp = make_dobj(name,ifp->if_dp->dt_type_dim,OBJ_PREC(ifp->if_dp));
	*/
	/* originally we gave this the filename, but use localname() to
	 * work with Carlo's hack
	 */
	dp = make_dobj(localname(),
		ifp->if_dp->dt_type_dim,OBJ_PREC(ifp->if_dp));

	if( dp == NULL ) return(NULL);

	/* read the data */
	(*FT_READ_FUNC(ftp))(dp,ifp,0,0,0);

	/* convert to desired type */
	if((*FT_UNCONV_FUNC(ftp))(&new_hdp,dp) < 0 )
		return(NULL);

	return(new_hdp);
}
#endif /* FOOBAR */

/* filetype independent stuff lifted from fiomenu.c */

Filetype * current_filetype(void)
{
	return(curr_ftp);
}

void _set_filetype(QSP_ARG_DECL  Filetype *ftp)
{
	curr_ftp=ftp;
}

static Item_Type *suffix_itp=NULL;

typedef struct known_suffix {
	const char *	sfx_name;
	Filetype *	sfx_ftp;
} Known_Suffix;

static ITEM_INIT_FUNC(Known_Suffix,suffix,0)
static ITEM_NEW_FUNC(Known_Suffix,suffix)
static ITEM_CHECK_FUNC(Known_Suffix,suffix)

#define init_suffixs()	_init_suffixs(SINGLE_QSP_ARG)
#define new_suffix(s)	_new_suffix(QSP_ARG  s)
#define suffix_of(s)	_suffix_of(QSP_ARG  s)

Filetype *filetype_for_code(QSP_ARG_DECL  filetype_code code)
{
	List *lp;
	Node *np;
	Filetype *ftp;

	lp = file_type_list();
	np=QLIST_HEAD(lp);
	while(np!=NULL){
		ftp = (Filetype *) NODE_DATA(np);
		if( FT_CODE(ftp) == code ) return ftp;
		np = NODE_NEXT(np);
	}

	snprintf(ERROR_STRING,LLEN,"filetype_for_code:  no filetype defined for code %d!?",code);
	WARN(ERROR_STRING);

	return NULL;
}

static void init_suffix( QSP_ARG_DECL  const char *name, filetype_code code )
{
	Known_Suffix *sfx_p;
	Filetype *ftp;

	ftp = filetype_for_code(QSP_ARG  code);
	assert( ftp != NULL );

	sfx_p = new_suffix(name);
	assert( sfx_p != NULL );

	/* Now file the filetype struct with this code... */
	sfx_p->sfx_ftp = ftp;
}

#define INIT_SUFFIX(name,code)	init_suffix(QSP_ARG  name, code)

static void init_suffixes(SINGLE_QSP_ARG_DECL)
{
	INIT_SUFFIX( "hips2",	IFT_HIPS2);
	INIT_SUFFIX( "raw",	IFT_RAW	);
#ifdef HAVE_KHOROS
	INIT_SUFFIX( "viff",	IFT_VIFF);
#endif /* HAVE_KHOROS */
#ifdef HAVE_TIFF
	INIT_SUFFIX( "tiff",	IFT_TIFF);
	INIT_SUFFIX( "tif",	IFT_TIFF);
#endif /* HAVE_TIFF */
#ifdef HAVE_RGB
	INIT_SUFFIX( "rgb",	IFT_RGB	);
#endif /* HAVE_RGB */
	INIT_SUFFIX( "ppm",	IFT_PPM	);
#ifdef HAVE_JPEG_SUPPORT
	INIT_SUFFIX( "jpg",	IFT_JPEG);
	INIT_SUFFIX( "JPG",	IFT_JPEG);
	INIT_SUFFIX( "lml",	IFT_LML	);
	INIT_SUFFIX( "jpeg",	IFT_JPEG);
	INIT_SUFFIX( "mjpg",	IFT_JPEG);
	INIT_SUFFIX( "mjpeg",	IFT_JPEG);
#endif /* HAVE_JPEG_SUPPORT */

	// this was in the ifdef HAVE_JPEG_SUPPORT - why?
	INIT_SUFFIX( "asc",	IFT_ASC	);

#ifdef HAVE_PNG
	INIT_SUFFIX( "png",	IFT_PNG	);
#else /* !HAVE_PNG */
#ifdef BUILD_FOR_IOS
	INIT_SUFFIX( "png",	IFT_PNG	);
#endif // BUILD_FOR_IOS
#endif /* !HAVE_PNG */

#ifdef HAVE_MATIO
	INIT_SUFFIX( "mat",	IFT_MATLAB);
#endif /* HAVE_MATIO */

#ifdef HAVE_MPEG
	INIT_SUFFIX( "mpeg",	IFT_MPEG);
	INIT_SUFFIX( "mpg",	IFT_MPEG);
#endif /* HAVE_MPEG */

#ifdef HAVE_QUICKTIME
	INIT_SUFFIX( "mov",	IFT_QT	);
#endif
	INIT_SUFFIX( "hips1",	IFT_HIPS1);

#ifdef HAVE_AVI_SUPPORT
	INIT_SUFFIX( "avi",	IFT_AVI	);
#endif /* HAVE_AVI_SUPPORT */

	INIT_SUFFIX( "bmp",	IFT_BMP	);
	INIT_SUFFIX( "BMP",	IFT_BMP	);
	INIT_SUFFIX( "wav",	IFT_WAV	);

#ifdef NOT_YET

	INIT_SUFFIX( "vst",	IFT_VISTA);
	INIT_SUFFIX( "dsk",	IFT_DISK);
	INIT_SUFFIX( "vl",	IFT_VL	);
	INIT_SUFFIX( "ras",	IFT_SUNRAS);
	INIT_SUFFIX( "WAV",	IFT_WAV	);
	INIT_SUFFIX( "bdf",	IFT_BDF	);


#endif /* NOT_YET */

}

static Filetype* infer_filetype_from_name(QSP_ARG_DECL  const char *name)
{
	Known_Suffix *sfx_p;
	const char *suffix=NULL;
	const char *s;
	static int suffixes_inited=0;

	/* First make sure that we've initialized the table of suffixes */
	if( ! suffixes_inited ){
		init_suffixes(SINGLE_QSP_ARG);
		suffixes_inited=1;
	}

	/* set the suffix to the string following the last '.' */

	s=name;
	while(*s!=0){
		if( *s == '.' ) suffix = s+1;
		s++;
	}
	if( suffix == NULL || *suffix == 0 ) return(NULL);

	sfx_p = suffix_of(suffix);
	if( sfx_p == NULL ){
		// Print an advisory
		snprintf(ERROR_STRING,LLEN,"File suffix \"%s\" is not known!?",
			suffix);
		WARN(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"Using current file type %s",
			FT_NAME(curr_ftp) );
		advise(ERROR_STRING);
		return(NULL);
	}
	return( sfx_p->sfx_ftp );
}

/*
 * Call type-specific function to open the file
 */

Image_File *_read_image_file(QSP_ARG_DECL  const char *name)
{
	Image_File *ifp;
	Filetype * ftp;

	ftp = infer_filetype_from_name(QSP_ARG  name);
	if( ftp == NULL ){
		assert( curr_ftp != NULL );
		ftp=curr_ftp;	/* use default if can't figure it out */
	} else if( verbose && ftp!=curr_ftp ){
		snprintf(ERROR_STRING,LLEN,"Inferring filetype %s from filename %s, overriding default %s",
			FT_NAME(ftp),name,FT_NAME(curr_ftp));
		advise(ERROR_STRING);
	}

	if( CANNOT_READ(ftp) ){
		snprintf(ERROR_STRING,LLEN,"Sorry, can't read files of type %s",
			FT_NAME(ftp));
		warn(ERROR_STRING);
		return(NULL);
	}

	/* pathname hasn't been set yet... */
fprintf(stderr,"Calling type-specific open func for %s\n",name);
	ifp=(*ftp->op_func)( QSP_ARG  name, FILE_READ );

	if( ifp == NULL ) {
		snprintf(ERROR_STRING,LLEN,
			"error reading %s file \"%s\"",FT_NAME(ftp),name);
		warn(ERROR_STRING);
	}
	return(ifp);
}

/* Open a file for writing */

Image_File *_write_image_file(QSP_ARG_DECL  const char *filename,dimension_t n)
{
	Image_File *ifp;
	Filetype * ftp;

	ftp = infer_filetype_from_name(QSP_ARG  filename);
	if( ftp == NULL ) {	/* use default if can't figure it out */
		if( curr_ftp == NULL ) image_file_init(SINGLE_QSP_ARG);
		//assert( curr_ftp != NULL );
		ftp=curr_ftp;	/* use default if can't figure it out */
	} else if( ftp != curr_ftp ){
		snprintf(ERROR_STRING,LLEN,"Inferring filetype %s from filename %s, overriding default %s",
			FT_NAME(ftp),filename,FT_NAME(curr_ftp));
		advise(ERROR_STRING);
		// Should we make this filetype the new current default???
	}

	if( CANNOT_WRITE(ftp) ){
		snprintf(ERROR_STRING,LLEN,"Sorry, can't write files of type %s",
			FT_NAME(ftp));
		warn(ERROR_STRING);
		return(NULL);
	}

	ifp = (*FT_OPEN_FUNC(ftp))( QSP_ARG  filename, FILE_WRITE ) ;
	if( ifp != NULL )
		ifp->if_frms_to_wt = n;

	return(ifp);
}

/* Should we impose that the objects have the same size?? */

void _read_object_from_file(QSP_ARG_DECL  Data_Obj *dp,Image_File *ifp)
{
	if( dp == NULL ) return;
	if( ifp == NULL ) return;

	if( !IS_READABLE(ifp) ){
		snprintf(ERROR_STRING,LLEN,"File %s is not readable",ifp->if_name);
		WARN(ERROR_STRING);
		return;
	}

	if( OBJ_ROWS(ifp->if_dp) != 0 && OBJ_ROWS(dp) != OBJ_ROWS(ifp->if_dp) ){
		snprintf(ERROR_STRING,LLEN,"Row count mismatch, object %s (%d) and file %s (%d)",
				OBJ_NAME(dp),OBJ_ROWS(dp),ifp->if_name,OBJ_ROWS(ifp->if_dp));
		WARN(ERROR_STRING);
	}

	if( OBJ_COLS(ifp->if_dp) != 0 && OBJ_COLS(dp) != OBJ_COLS(ifp->if_dp) ){
		snprintf(ERROR_STRING,LLEN,"Column count mismatch, object %s (%d) and file %s (%d)",
				OBJ_NAME(dp),OBJ_COLS(dp),ifp->if_name,OBJ_COLS(ifp->if_dp));
		WARN(ERROR_STRING);
	}

	/* was this nfrms a BUG, or was there a reason??? */
	
	(*FT_READ_FUNC(IF_TYPE(ifp)))(QSP_ARG  dp,ifp,0,0,/* ifp->hf_nfrms */ 0 );
}

/*
 * Filetype independent way to close an image file.
 * Calls routine from table.
 */

void _close_image_file(QSP_ARG_DECL  Image_File *ifp)
{
	if( ifp == NULL ) return;
	(*FT_CLOSE_FUNC(IF_TYPE(ifp)))(QSP_ARG  ifp);
}

/*
 * Open for reading or writing
 *
 * High level routine, calls r/w specific routine, which may
 * call vectored module-specific routine...
 */

Image_File * _open_image_file(QSP_ARG_DECL  const char *filename,const char *rw)
{
	Image_File *ifp;

snprintf(ERROR_STRING,LLEN,"open_image_file %s",filename);
advise(ERROR_STRING);
	if( *rw == 'r' )
		ifp = read_image_file(filename);

	/* BUG 4096 is an arbitrary big number.  Originally we
	 * passed the number of frames to write to the open routine
	 * so we could write the header; Now for hips1 we can go
	 * back and edit it later...  need to support this feature
	 * for hips2, viff and ???
	 */

	else if( *rw == 'w' )
		ifp = write_image_file(filename,4096);

	else {
		assert( AERROR("bad r/w string passed to open_image_file") );
	}

	return(ifp);
}

/* put an image out to a writable file */

void _write_image_to_file(QSP_ARG_DECL  Image_File *ifp,Data_Obj *dp)
{
	/* take filetype from image file */
	if( dp == NULL ) return;
	if( ifp == NULL ) return;

	if( ! IS_WRITABLE(ifp) ){
		snprintf(ERROR_STRING,LLEN,"File %s is not writable",ifp->if_name);
		WARN(ERROR_STRING);
		return;
	}

	(*FT_WRITE_FUNC(IF_TYPE(ifp)))(QSP_ARG  dp,ifp);
}

static off_t fio_seek_setup(Image_File *ifp, index_t n)
{
	off_t frms_to_seek;
	index_t frmsiz;

	// if_nfrms holds the number of frames already read...
	// index_t is unsigned, so we have to cast this to be sure to get
	// the correct sign...
	frms_to_seek = ((off_t)n) - ifp->if_nfrms;

	/* figure out frame size */

	frmsiz = OBJ_COLS(ifp->if_dp) * OBJ_ROWS(ifp->if_dp)
		* OBJ_COMPS(ifp->if_dp) * PREC_SIZE(OBJ_PREC_PTR(ifp->if_dp));

	if( FT_CODE(IF_TYPE(ifp)) == IFT_DISK ){
		/* round up to block size */
		frmsiz += 511;
		frmsiz &= ~511;
		/* BUG? add a 1?? */
	}

	/* although IFT_RV frames are also rounded up to blocksize,
	 * we pass the frame number to rv_seek_frame() and let it worry
	 * about it...
	 */

	return( frms_to_seek * frmsiz );
}

int uio_seek_frame(QSP_ARG_DECL  Image_File *ifp, index_t n)
{
	off_t offset;

	offset = fio_seek_setup(ifp,n);

	if( lseek(ifp->if_fd,offset,SEEK_CUR) < 0 ){
		WARN("lseek error");
		return(-1);
	}
	return(0);
}

int _std_seek_frame(QSP_ARG_DECL  Image_File *ifp, index_t n)
{
	off_t offset;

	offset = fio_seek_setup(ifp,n);

	if( offset == 0 ){
		if( verbose ) advise("Seek to current location requested!?");
		return(0);	/* nothing to do */
	}
	if( fseek(ifp->if_fp,(long)offset,/*1*/ SEEK_CUR) != 0 ){
		WARN("fseek error");
		return(-1);
	}
	return(0);
}

int _image_file_seek(QSP_ARG_DECL  Image_File *ifp,dimension_t n)
{
	/* BUG?  off_t is long long on new sgi!? */

#ifdef QUIP_DEBUG
if( debug & debug_fileio ){
snprintf(ERROR_STRING,LLEN,"image_file_seek %s %d",
		ifp->if_name,n);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
	if( ! IS_READABLE(ifp) ){
		snprintf(ERROR_STRING,LLEN,"File %s is not readable, can't seek",
			ifp->if_name);
		WARN(ERROR_STRING);
		return(-1);
	}
	if( n >= OBJ_FRAMES(ifp->if_dp) ){
		snprintf(ERROR_STRING,LLEN,
	"Frame index %d is out of range for file %s (%d frames)",
			n,ifp->if_name,OBJ_FRAMES(ifp->if_dp));
		WARN(ERROR_STRING);
		return(-1);
	}

	/* how do we figure out what frame we are at currently?
	 * It looks like if_nfrms holds the index of the next frame
	 * we will read...
	 *
	 * For most of our formats, the frame size is fixed so we can
	 * just calculate the offset.  For jpeg, we have to use a seek
	 * table that we construct when we first scan the file for frames.
	 *
	 * This would be cleaner if we vectored the seek routine...
	 * rv is a special case, since we have multiple disks to worry about.
	 */

	if( (*FT_SEEK_FUNC(IF_TYPE(ifp)))(QSP_ARG  ifp,n) < 0 ){
		snprintf(ERROR_STRING,LLEN,"Error seeking frame %d on file %s",n,ifp->if_name);
		WARN(ERROR_STRING);
		return(-1);
	}
	ifp->if_nfrms = n;
	return(0);
}

void _check_auto_close(QSP_ARG_DECL  Image_File *ifp)
{
	if( ifp->if_nfrms >= ifp->if_frms_to_wt ){
		if( verbose ){
	snprintf(ERROR_STRING,LLEN, "closing file \"%s\" after writing %d frames",
			ifp->if_name,ifp->if_nfrms);
			advise(ERROR_STRING);
		}
		close_image_file(ifp);
	}
}

double iof_exists(QSP_ARG_DECL  const char *s)
{
	Image_File *ifp;

	ifp=img_file_of(s);
	if( ifp==NULL ) return(0.0);
	else return(1.0);
}

static double get_if_seconds(QSP_ARG_DECL  Item *ip,dimension_t frame)
{
	Image_File *ifp;

	ifp = (Image_File *) ip;

	switch( FT_CODE(IF_TYPE(ifp)) ){

#ifdef HAVE_JPEG_SUPPORT
		case IFT_LML: return( get_lml_seconds(QSP_ARG  ifp,frame) ); break;
#endif /* HAVE_JPEG_SUPPORT */

#ifdef HAVE_RAWVOL
		case IFT_RV: return( get_rv_seconds(QSP_ARG  ifp,frame)); break;
#endif /* HAVE_RAWVOL */

		default:
			WARN("Timestamp functions are only supported for file types LML and RV");
			snprintf(ERROR_STRING,LLEN,"(file %s is type %s)",ifp->if_name,
					FT_NAME(IF_TYPE(ifp)));
			advise(ERROR_STRING);
			return(-1);
	}
}

static double get_if_milliseconds(QSP_ARG_DECL  Item *ip,dimension_t frame)
{
	Image_File *ifp;

	ifp = (Image_File *) ip;

	switch( FT_CODE(IF_TYPE(ifp)) ){

#ifdef HAVE_JPEG_SUPPORT
		case IFT_LML: return( get_lml_milliseconds(QSP_ARG  ifp,frame) ); break;
#endif /* HAVE_JPEG_SUPPORT */

#ifdef HAVE_RAWVOL
		case IFT_RV: return( get_rv_milliseconds(QSP_ARG  ifp,frame) ); break;
#endif /* HAVE_RAWVOL */

		default:
			WARN("Timestamp functions are only supported for file types LML and RV");
			snprintf(ERROR_STRING,LLEN,"(file %s is type %s)",ifp->if_name,
					FT_NAME(IF_TYPE(ifp)));
			advise(ERROR_STRING);
			return(-1);
	}
}

static double get_if_microseconds(QSP_ARG_DECL  Item *ip,dimension_t frame)
{
	Image_File *ifp;

	ifp = (Image_File *) ip;

	switch( FT_CODE(IF_TYPE(ifp)) ){
#ifdef HAVE_JPEG_SUPPORT
		case IFT_LML: return( get_lml_microseconds(QSP_ARG  ifp,frame) ); break;
#endif /* HAVE_JPEG_SUPPORT */

#ifdef HAVE_RAWVOL
		case IFT_RV: return( get_rv_microseconds(QSP_ARG  ifp,frame) ); break;
#endif /* HAVE_RAWVOL */

		default:
			WARN("Timestamp functions are only supported for file types LML and RV");
			snprintf(ERROR_STRING,LLEN,"(file %s is type %s)",ifp->if_name,
					FT_NAME(IF_TYPE(ifp)) );
			advise(ERROR_STRING);
			return(-1);
	}
}

