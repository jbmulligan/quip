#include "quip_config.h"

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>			/* prototype for drand48() */
#endif

#ifdef HAVE_GETTIMEOFDAY
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H
#elif HAVE_FTIME
#include <sys/timeb.h>
#elif HAVE_TIME
#include <time.h>
#endif

#include "quip_prot.h"
#include "rn.h"
#include "debug.h"


static int is_seeded=0;

u_long rn( u_long max ) /** returns a number between 0 and max (inclusive) */
{
        u_long n;
#ifdef HAVE_DRAND48
	double dr;
#elif HAVE_RANDOM
	extern long random();
	float f;
#elif HAVE_RAND
	extern int rand();
	float f;
#endif

	_rninit(SGL_DEFAULT_QSP_ARG);

#ifdef HAVE_DRAND48

	dr = drand48();
	n = (u_long) (dr * (max+1));
	if( n > max ) n=max;

#elif HAVE_RANDOM

#define BIGINT	0x7fffffff
	n=random();
	f = n / (float) BIGINT;
	f *= max+1;
	n = f;
	if( n > max ) n = max;

#elif HAVE_RAND

#define BIGINT	0x7fff
	n=rand();
	f = n / (float) BIGINT;
	f *= (float)(max+1);
	n = (int)f;
	if( n > max ) n = (int)max;

#else
	NERROR1("rn.c:  No random number function found on this system by configure!?");
#endif /* DRAND48, RANDOM */

	return(n);
}

void _set_random_seed(SINGLE_QSP_ARG_DECL)
{
	long seed_val;
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;

	gettimeofday(&tv,NULL);
	seed_val = tv.tv_usec & 0xffff;	/* 64k different seeds */
#else
	warn("rn.c:  need to implement set_random_seed without gettimeofday!?");
	seed_val=1;
#endif

#ifdef HAVE_SRAND48
	srand48( seed_val );
#else
	//warn("rn.c:  need to implement set_random_seed without srand48!?");
#endif
}

void _set_seed(QSP_ARG_DECL  u_long seed)
{
#ifdef HAVE_SRAND48

	if( verbose ){
		snprintf(ERROR_STRING,LLEN,"set_seed:  srand48(%ld)",seed);
		advise(ERROR_STRING);
	}

	srand48( seed );
#elif HAVE_SRANDOM
	if( verbose ){
		snprintf(ERROR_STRING,LLEN,"set_seed:  srandom(%ld)",seed);
		advise(ERROR_STRING);
	}

	srandom( seed );
#elif HAVE_SRAND
	if( verbose ){
		snprintf(ERROR_STRING,LLEN,"set_seed:  srand(%d)",(int)seed);
		advise(ERROR_STRING);
	}

	srand( (int)seed );
#else
	warn("rn.c:  no function to set random number generator seed identified by configure!?");
#endif /* DRAND48 */

	is_seeded=1;
}

#define HOURS_PER_DAY		24
#define MINUTES_PER_HOUR	60
#define SECONDS_PER_MINUTE	60
#define SECONDS_PER_HOUR	(SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY		(SECONDS_PER_HOUR * HOURS_PER_DAY)

void _rninit(SINGLE_QSP_ARG_DECL)        /** randomly seed the generator */
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
#elif HAVE_FTIME
	struct timeb tb1, *tbp=(&tb1);
#elif HAVE_TIME
        time_t tvec;
#endif

	int seed;

	if( is_seeded ) return;

#ifdef HAVE_GETTIMEOFDAY

	if( gettimeofday(&tv,NULL) < 0 ){
		tell_sys_error("rninit");
		NADVISE("using a seed value of 1");
		seed=1;
	} else {
		seed = (tv.tv_sec % SECONDS_PER_DAY) + tv.tv_usec;
	}

#elif HAVE_FTIME
	/* on some machines ftime() gives us milliseconds... */
	ftime(tbp);
	seed = (int) tbp->millitm;
#elif HAVE_TIME
	/* this method is less desirable, because two programs run within
	 * the same second will get the same seed value...
	 */

        time( &tvec );
        seed= (int)  tvec & 0xfff ;

#else
	warn("rn.c:  No time function available to generate a random seed, using 0.");
#endif

	if( verbose ){
		snprintf(ERROR_STRING,LLEN,"rninit: seed is %d",seed);
		advise(ERROR_STRING);
	}

	set_seed(seed);
}

/*
 * Permute the existing contents of the buffer buf
 *
 * We do this by switching each element with a random element.
 * We start at the end and count down.
 */

void _scramble(QSP_ARG_DECL  SCRAMBLE_TYPE* buf,u_long n)
     /* buf = buffer of long words to be scrambled */
     /* n = number of words to scramble */
{
        u_long i,j;
        SCRAMBLE_TYPE tmp;
        for( i=n-1; i > 0; i-- ){
                j=rn(i);	// the range is 0-i (inclusive)
		if( j != i ){	// is it worth it to perform this test?
                	tmp=buf[j];
                	buf[j]=buf[i];
                	buf[i]=tmp;
		}
        }
}

/*
 * Initialize the contents of the buffer, then scramble
 */

void _permute(QSP_ARG_DECL  SCRAMBLE_TYPE *buf,int n)
     /* buf =  permutation buffer */
     /* n = number to scramble */
{
        SCRAMBLE_TYPE i;

	if( n <= 0 ){
		snprintf(ERROR_STRING,LLEN,"permute:  number of elements (%d) must be positive!?",n);
		warn(ERROR_STRING);
		return;
	}
        for(i=0;i<n;i++) buf[i]=i;
        scramble(buf,n);
}


