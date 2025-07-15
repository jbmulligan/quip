
#define GET_VIEWER( funcname )						\
									\
	vp=pick_vwr("");						\
	if( vp == NULL ) {						\
		snprintf(ERROR_STRING,LLEN,"%s:  invalid viewer selection",	\
						funcname);		\
		WARN(ERROR_STRING);					\
	}

