#include "quip_config.h"

#include <stdio.h>

#include "ports.h"
#include "debug.h"

#include "item_type.h"
#include "node.h"

debug_flag_t debug_ports=0;

ITEM_INTERFACE_DECLARATIONS(Port,port,0)

/* del_port deletes given a name */

void delport(QSP_ARG_DECL  Port *mpp)
{
#ifdef CAUTIOUS
	if( mpp==NULL ) {
		error1("delport passed NULL");
		return; // NOTREACHED - silence analyzer
	}
#endif /* CAUTIOUS */
	del_item(port_itp,mpp);
}

void portinfo(QSP_ARG_DECL  Port *mpp)
{
	snprintf(msg_str,LLEN,"Port \"%s\":",mpp->mp_name);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tsock = %d, o_sock = %d",mpp->mp_sock,mpp->mp_o_sock);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\taddrp = 0x%lx",(u_long)mpp->mp_addrp);
	prt_msg(msg_str);

	snprintf(msg_str,LLEN,"\tflags =  0x%x",mpp->mp_flags);
	prt_msg(msg_str);
	snprintf(msg_str,LLEN,"\tsleeptime =  %ld",(long)mpp->mp_sleeptime);
	prt_msg(msg_str);

	if( mpp->mp_flags & PORT_SERVER ){
		snprintf(msg_str,LLEN,"\tSERVER\t\t0x%x",PORT_SERVER);
		prt_msg(msg_str);
	}
	if( mpp->mp_flags & PORT_CLIENT ){
		snprintf(msg_str,LLEN,"\tCLIENT\t\t0x%x",PORT_CLIENT);
		prt_msg(msg_str);
	}
	if( mpp->mp_flags & PORT_CONNECTED ){
		snprintf(msg_str,LLEN,"\tCONNECTED\t0x%x",PORT_CONNECTED);
		prt_msg(msg_str);
	}
}

#ifdef FOOBAR
void init_my_ports(SINGLE_QSP_ARG_DECL)
{
	static int ports_inited=0;

	if( ports_inited ) return;

#ifdef QUIP_DEBUG
	debug_ports = add_debug_module("ports");
#endif /* QUIP_DEBUG */

	init_pdt_tbl();

	ports_inited=1;
}
#endif // FOOBAR

