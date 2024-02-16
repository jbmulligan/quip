/** user interface routines for interactive progs */

#include "quip_config.h"

#include <stdio.h>
#include "quip_prot.h"
#include "data_obj.h"


/* Look for an object described by a possibly indexed string
 * no warning issued if the object does not exist
 */

#define MAX_NAME_LEN	512	// BUG need to check for buffer overrun

Data_Obj * _hunt_obj(QSP_ARG_DECL  const char *name)
{
	Data_Obj *dp;
	char stem[MAX_NAME_LEN], *cp;
	const char *s;

#ifdef QUIP_DEBUG
/*
if( debug & debug_data ){
snprintf(ERROR_STRING,LLEN,"hunt_obj:  passed name \"%s\"",name);
advise(ERROR_STRING);
list_dobjs(SINGLE_QSP_ARG);
}
*/
#endif /* QUIP_DEBUG */

	dp=dobj_of(name);
	if( dp != NULL ){
		return(dp);
	}

	/* maybe this string has an index tacked onto the end? */
	/* copy the stem name into the buffer */

	cp=stem;
	s=name;

	/* use symbolic constants for delimiters here? */
	while( *s && *s != '[' && *s != '{' )
		*cp++ = *s++;
	*cp=0;

	dp = dobj_of(stem);
	if( dp == NULL ) return(dp);

	return( index_data(dp,s) );
}

Data_Obj *_get_obj(QSP_ARG_DECL  const char *name)
{
	Data_Obj *dp;

	dp = hunt_obj(name);
	if( dp == NULL ){
		snprintf(ERROR_STRING,LLEN,"No data object \"%s\"",name);
		warn(ERROR_STRING);
	}
	return(dp);
}

Data_Obj *get_vec(QSP_ARG_DECL  const char *s)
{
	Data_Obj *dp;

	dp=get_obj(s);
	if( dp==NULL ) return(dp);
	if( !(OBJ_FLAGS(dp) & DT_VECTOR) ){
		snprintf(ERROR_STRING,LLEN,"object \"%s\" is not a vector",s);
		warn(ERROR_STRING);
		return(NULL);
	}
	return(dp);
}

Data_Obj *				/**/
img_of(QSP_ARG_DECL  const char *s)				/**/
{
	Data_Obj *dp;

	dp=get_obj(s);
	if( dp==NULL ) return(dp);
	if( !(OBJ_FLAGS(dp) & DT_IMAGE) ){
		snprintf(ERROR_STRING,LLEN,"object \"%s\" is not an image",s);
		warn(ERROR_STRING);
		return(NULL);
	}
	return(dp);
}

Data_Obj *get_seq(QSP_ARG_DECL  const char *s)
{
	Data_Obj *dp;

	dp=get_obj(s);
	if( dp==NULL ) return(dp);
	if( !(OBJ_FLAGS(dp) & DT_SEQUENCE) ){
		snprintf(ERROR_STRING,LLEN,"object \"%s\" is not an sequence",s);
		warn(ERROR_STRING);
		return(NULL);
	}
	return(dp);
}

Data_Obj * get_img( QSP_ARG_DECL  const char *s )
{
	Data_Obj *dp;

	dp=get_obj(s);
	if( dp==NULL ) return(dp);
	if( !IS_IMAGE(dp) ){
		snprintf(ERROR_STRING,LLEN,"data object %s is not an image",s);
		warn(ERROR_STRING);
		return(NULL);
	}
	return(dp);
}

