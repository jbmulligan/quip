
#include "quip_config.h"

#include <stdio.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "quip_prot.h"
#include "query_bits.h"	// LLEN - BUG
#include "fio_prot.h"
#include "nports_api.h"
#include "quip_prot.h"
#include "img_file.h"
//#include "filetype.h"

/* flag ignored, for arg compatibility w/ xmit_data */
void _xmit_img_file(QSP_ARG_DECL  Port *mpp,Image_File *ifp,int flag)	/** send a file header */
{
	int32_t len;
	int32_t code;

#ifdef SIGPIPE
	// We used to call a handler (if_pipe) that performed
	// a fatal error exit.  Now we would like to recover
	// a little more gracefully.  That means that put_port_int32 etc
	// can return after the port is closed.
	signal(SIGPIPE,SIG_IGN);
#endif /* SIGPIPE */

	code=P_IMG_FILE;
	if( put_port_int32(mpp,code) == (-1) ){
		warn("xmit_file:  error sending code");
		return;
	}

	len=(int32_t)strlen(ifp->if_name)+1;


	/* we don't send the file data, just the header data */
	/* We need to send:
	 *
	 *	name
	 *	if_nfrms	(frms written/read)
	 *	if_type		(file format)
	 *	(file type specific header?)
	 *	if_flags
	 *	(dp ptr to obj with dimensions?)
	 *	(if_pathname)	(don't really care since file is on remot sys)
	 */

	if( put_port_int32(mpp,len) == -1 ){
		warn("xmit_file:  error writing name length word");
		return;
	}

	if( put_port_int32(mpp, ifp->if_nfrms) == -1 ||
	    put_port_int32(mpp, FT_CODE(IF_TYPE(ifp)) ) == -1 ||
	    put_port_int32(mpp, ifp->if_flags ) == -1 ){
		warn("error sending image file header data");
		return;
	}
	    
	if( write_port(mpp,ifp->if_name,len) == (-1) ){
		warn("xmit_file:  error writing image file name");
		return;
	}

	/* now send the associated data_obj header... */

	assert( ifp->if_dp != NULL );

	xmit_obj(mpp,ifp->if_dp,0);
}

/** recv_img_file - recieve a new data object */

long _recv_img_file(QSP_ARG_DECL  Port *mpp, /* char **bufp */ Packet *pkp )
{
	long len;
	Image_File *old_ifp, *new_ifp;
	char namebuf[LLEN];
	Image_File imgf, *ifp;
	long code;

	ifp=(&imgf);
	len=get_port_int32(mpp);
	if( len <= 0 ) goto error_return;

#ifdef QUIP_DEBUG
if( debug & debug_data ){
snprintf(ERROR_STRING,LLEN,
"recv_file:  want %ld name bytes",len);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */

	if( (ifp->if_nfrms = get_port_int32(mpp)) == BAD_PORT_LONG ||
	    (ifp->if_ftp = filetype_for_code( QSP_ARG  (filetype_code) get_port_int32(mpp))) == NULL ||
	    (ifp->if_flags = (short) get_port_int32(mpp)) == (short)BAD_PORT_LONG ){
		warn("error getting image file data");
		goto error_return;
	}

	if( len > LLEN ){
		warn("more than LLEN name chars!?");
		goto error_return;
	}
	if( read_port(mpp,namebuf,len) != len ){
		warn("recv_file:  error reading data object name");
		goto error_return;
	}

	/* where does the string get null-terminated? */

	if( (long)strlen( namebuf ) != len-1 ){
		u_int i;

		snprintf(ERROR_STRING,LLEN,"name length %ld, expected %ld",
			(long)strlen(namebuf), len-1);
		advise(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"name:  \"%s\"",namebuf);
		advise(ERROR_STRING);
		for(i=0;i<strlen(namebuf);i++){
			snprintf(ERROR_STRING,LLEN,"name[%d] = '%c' (0%o)",
				i,namebuf[i],namebuf[i]);
			advise(ERROR_STRING);
		}
		error1("choked");
	}

	old_ifp=img_file_of(namebuf);

	if( old_ifp != NULL ){
		del_img_file(old_ifp);
		rls_str((char *)old_ifp->if_name);	// BUG?  release name here or not?
		old_ifp = NULL;
	}

	new_ifp=new_img_file(namebuf);
	if( new_ifp==NULL ){
		snprintf(ERROR_STRING,LLEN,
			"recv_file:  couldn't create file struct \"%s\"",
			namebuf);
		warn(ERROR_STRING);
		goto error_return;
	}

	new_ifp->if_nfrms = imgf.if_nfrms;
	new_ifp->if_ftp = filetype_for_code(QSP_ARG  IFT_NETWORK);
	new_ifp->if_flags = imgf.if_flags;
	new_ifp->if_dp = NULL;	/* BUG should receive? */
	new_ifp->if_pathname = new_ifp->if_name;

	code = get_port_int32(mpp);
	if( code == -1 )
		error1("error port code received!?");
	if( code != P_DATA ){
		snprintf(ERROR_STRING,LLEN,
	"recv_file:  expected data object packet to complete transmission of file %s!?",
			new_ifp->if_name);
		error1(ERROR_STRING);
	}
		
	// the cast generates a compiler warning???
	if( recv_obj(mpp, pkp ) != sizeof(Data_Obj) ){
		warn("Error receiving data object!?");
		goto error_return;
	}

	// The packet returns the dp in pk_extra...
	return sizeof(*new_ifp);	// BUG - what size should we return???

error_return:
	return -1;
}


