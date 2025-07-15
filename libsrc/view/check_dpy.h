
#ifdef HAVE_X11
#define CHECK_DPYP(funcname)						\
									\
	if( current_dpyp == NULL ){					\
		snprintf(ERROR_STRING,LLEN,"%s:  no display set",funcname); \
		warn(ERROR_STRING);					\
		return;							\
	}
#else
#define CHECK_DPYP(funcname)
#endif

