#include "quip_config.h"
#include "container_fwd.h"

#ifdef HAVE_PARPORT

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* close */
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_LINUX_PPDEV_H
#include <linux/ppdev.h>
#endif

#include "quip_prot.h"
#include "my_parport.h"
//#include "debug.h"

ITEM_INTERFACE_DECLARATIONS(ParPort,parport,DEFAULT_CONTAINER_TYPE)

static char *default_parport="/dev/parport0";

ParPort * _open_parport(QSP_ARG_DECL  const char *name)
{
	ParPort *ppp;

	if( name == NULL || *name == 0 ){
		name = default_parport;
		if( verbose ){
			snprintf(ERROR_STRING,LLEN,"open_parport:  using default parallel port %s",name);
			advise(ERROR_STRING);
		}
	}
	ppp = parport_of(name);
	if( ppp != NULL ){
		snprintf(ERROR_STRING,LLEN,"ParPort %s is already open",name);
		warn(ERROR_STRING);
		return NULL;
	}

	ppp = new_parport(name);
	if( ppp == NULL ) {
		snprintf(ERROR_STRING,LLEN,"Unable to create new parallel port structure %s",name);
		warn(ERROR_STRING);
		return NULL;
	}

	ppp->pp_fd = open(ppp->pp_name,O_RDWR);
	if( ppp->pp_fd < 0 ){
		perror("open");
		warn("error opening parallel port device");
		return NULL;
	}
	if( ioctl(ppp->pp_fd,PPCLAIM,NULL) < 0 ){
		perror("ioctl");
		warn("error claiming parallel port device");
		close(ppp->pp_fd);
		del_parport(ppp);
		return NULL;
	}
	return ppp;
}

int _read_parport_status(QSP_ARG_DECL  ParPort *ppp)
{
	unsigned char b;

	if( ioctl(ppp->pp_fd,PPRSTATUS,&b) < 0 ){
		perror("ioctl");
		warn("error reading parport status");
		return -1;
	}
	return (int) b;
}

/* This function is useful for when we use the parallel port to monitor a pulse generator,
 * such as a TTL video sync signal.
 */

int _read_til_transition(QSP_ARG_DECL  ParPort *ppp, int mask)
{
	int original_value, current_value;

	original_value = read_parport_status(ppp) & mask;

	do {
		current_value = read_parport_status(ppp) & mask;
	} while( current_value == original_value );

	return(current_value);
}

#endif /* HAVE_PARPORT */
