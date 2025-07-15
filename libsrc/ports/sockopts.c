#include "quip_config.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef BUILD_FOR_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif // BUILD_FOR_WINDOWS

#include "ports.h"

#define get_it( opt )	if( getsockopt(mpp->mp_sock, SOL_SOCKET, opt, \
			valbuf, &vbsiz) != 0 ) \
			tell_sys_error("getsockopt"); \
			if( vbsiz!=sizeof(int) ){ \
				snprintf(msg_str,LLEN,"return value has size %d!!!",vbsiz); \
				prt_msg(msg_str);}

#define VBUFSIZ	128

#ifdef HAVE_SOCKET
void show_sockopts(QSP_ARG_DECL  Port *mpp)
{
	char valbuf[VBUFSIZ];
	socklen_t vbsiz=VBUFSIZ;
	int *ival=(int *)valbuf;

	snprintf(msg_str,LLEN,"Options for socket \"%s\":",mpp->mp_name);
	prt_msg(msg_str);

	get_it( SO_DEBUG )
	snprintf(msg_str,LLEN,"\tDEBUG\t%d",*ival);
	prt_msg(msg_str);
	get_it( SO_REUSEADDR )
	snprintf(msg_str,LLEN,"\tREUSEADDR\t%d",*ival);
	prt_msg(msg_str);
	get_it( SO_KEEPALIVE )
	snprintf(msg_str,LLEN,"\tKEEPALIVE\t%d",*ival);
	prt_msg(msg_str);
	get_it( SO_DONTROUTE )
	snprintf(msg_str,LLEN,"\tDONTROUTE\t%d",*ival);
	prt_msg(msg_str);
	get_it( SO_LINGER )
	snprintf(msg_str,LLEN,"\tLINGER\t%d",*ival);
	prt_msg(msg_str);
#ifdef SO_BROADCAST
	get_it( SO_BROADCAST )
	snprintf(msg_str,LLEN,"\tBROADCAST\t%d",*ival);
	prt_msg(msg_str);
#endif
#ifdef SO_OOBINLINE
	get_it( SO_OOBINLINE )
	snprintf(msg_str,LLEN,"\tOOBINLINE\t%d",*ival);
	prt_msg(msg_str);
#endif
#ifdef SO_SNDBUF
	get_it( SO_SNDBUF )
	snprintf(msg_str,LLEN,"\tSNDBUF\t%d",*ival);
	prt_msg(msg_str);
#endif
#ifdef SO_RCVBUF
	get_it( SO_RCVBUF )
	snprintf(msg_str,LLEN,"\tRCVBUF\t%d",*ival);
	prt_msg(msg_str);
#endif
#ifdef SO_TYPE
	get_it( SO_TYPE )
	snprintf(msg_str,LLEN,"\tTYPE\t%d",*ival);
	prt_msg(msg_str);
#endif
#ifdef SO_ERROR
	get_it( SO_ERROR )
	snprintf(msg_str,LLEN,"\tERROR\t%d",*ival);
	prt_msg(msg_str);
#endif
}
#endif // HAVE_SOCKET

