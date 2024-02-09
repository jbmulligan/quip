#include "quip_config.h"
#include "fio_api.h"
#include "hips/hips2.h"
#include "quip_prot.h"

/*
 * Copyright (c) 1991 Michael Landy
 *
 * Disclaimer:  No guarantees of performance accompany this software,
 * nor is any responsibility assumed on the part of the authors.  All the
 * software has been tested extensively and every effort has been made to
 * insure its reliability.
 *
 * rewritten 2015 by jbm to get rid of variable format strings
 */

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef FOOBAR
#ifdef SUN
#include <varargs.h>
#else /* ! SUN */
#include <stdarg.h>
#endif /* ! SUN  */
#endif /* FOOBAR */

#ifdef PC
#include <process.h>		/* exit() */
#endif /* PC */

char badfmt[] = "invalid error format code %d";

// BUG?  global vars not thread-safe?  jbm
int j_hipserrlev = HEL_ERROR;
int j_hipserrprt = HEL_INFORM;	/* default: print&return for all others */
int j_hipserrno;
char j_hipserr[200];

#define hipserrlev	j_hipserrlev
#define hipserrprt	j_hipserrprt
#define hipserrno	j_hipserrno
#define hipserr		j_hipserr


#ifndef SUN
int perr(int,...);
#endif

/* perr(errorcode,errorprintargs...) */

#ifdef SUN
int perr(va_alist)
va_dcl	
#else /* ! SUN */
int perr(int error_code, ...)
#endif /* ! SUN */
{
	va_list ap;
	int  i1,i2,i3,i4;
	char *s1,*s2,*s3,*s4;
#ifdef SUN
	int error_code;

	va_start(ap);
	error_code = va_arg(ap,int);
#else
	va_start(ap,error_code);
#endif /* ! SUN  */

//fprintf(stderr,"perr:  error_code = %d (0x%x)\n",error_code,error_code);

	/*
	hipserrno = n;
	snprintf(hipserr,200,"%s: ",tell_progname());
	*/

	hipserr[0]=0;		/* clear string; errors are not necessarily fatal */

	switch( error_code ){

		// No args

		case HE_SETFP:
			strncpy(hipserr, "setformat: can't handle pyramid formats",200);
			break;
		case HE_FRMEND:
			strncpy(hipserr, "read_frame: did not find frame-end",200);
			break;
		case HE_PYRTL:
			strncpy(hipserr, "pyrnumpix: toplev too large?",200);
			break;
		case HE_PYRTLZ:
			strncpy(hipserr, "pyrnumpix: toplev less than zero?",200);
			break;
		case HE_CUTI:
			strncpy(hipserr, "cut_frame: intersections not between?",200);
			break;
		case HE_CUT1:
			strncpy(hipserr, "cut_frame: 1 intersection",200);
			break;
		case HE_COMPOV:
			strncpy(hipserr, "component table overflow",200);
			break;
		case HE_MULT2:
			strncpy(hipserr, "image dimensions must be multiples of 2",200);
			break;
		case HE_STDIN:
			strncpy(hipserr, "can't open the standard input twice",200);
			break;
		case HE_SMALL:
			strncpy(hipserr, "frame dimensions are too small",200);
			break;
		case HE_SETPF:
			strncpy(hipserr, "setpyrformat: can only handle pyramid formats",200);
			break;
		case HE_FILTPAR:
			strncpy(hipserr, "bad filter parameter",200);
			break;
		case HE_PNTOV:
			strncpy(hipserr, "point table overflow",200);
			break;
		case HE_ZNEG:
			strncpy(hipserr, "zero or negative dimension",200);
			break;
		case HE_LARGE:
			strncpy(hipserr, "frame dimensions are too large",200);
			break;
		case HE_FRMATCH:
			strncpy(hipserr, "mismatch of number of frames",200);
			break;
		case HE_SMATCH:
			strncpy(hipserr, "size mismatch",200);
			break;
		case HE_SNEG:
			strncpy(hipserr, "number of frames must be zero or positive",200);
			break;
		case HE_POW2:
			strncpy(hipserr, "image dimensions must be powers of 2",200);
			break;
		case HE_FMTMATCH:
			strncpy(hipserr, "pixel format mismatch",200);
			break;
		case HE_SEEK:
			strncpy(hipserr, "can't perform seek",200);
			break;
		case HE_READ:
			strncpy(hipserr, "error during read",200);
			break;
		case HE_FREE:
			strncpy(hipserr, "can't free memory",200);
			break;
		case HE_ALLOC:
			strncpy(hipserr, "can't allocate memory",200);
			break;


		case HE_BADFMT:
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "invalid format code %d", i1);
			break;
		case HE_READFR:
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "error reading frame %d", i1);
			break;
		case HE_WRITEFR:
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "error writing frame %d", i1);
			break;

		// One string arg:
		case HE_ALLOCSUBR:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200,"%s: can't allocate memory",s1);
			break;
		case HE_BUF:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: buffer limit exceeded", s1);
			break;
		case HE_FMT:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "can't handle pixel format %s", s1);
			break;
		case HE_FMTMATCHFILE:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "pixel format mismatch, file %s", s1);
			break;
		case HE_OPEN:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "can't open file: `%s'", s1);
			break;
		case HE_READFILE:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error reading from file %s", s1);
			break;
		case HE_HDRBREAD:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error reading binary parameters, file %s", s1);
			break;
		case HE_HDRPREAD:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error reading extended parameters, file %s", s1);
			break;
		case HE_HDRREAD:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error while reading header, file %s", s1);
			break;
		case HE_CODE:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: unknown op-code", s1);
			break;
		case HE_C_NCL:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched number of colors, file: %s", s1);
			break;
		case HE_C_FRM:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched number of frames, file: %s", s1);
			break;
		case HE_C_COL:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched number of columns, file: %s", s1);
			break;
		case HE_C_ROW:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched number of rows, file: %s", s1);
			break;
		case HE_HDRBWRT:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error while writing header binary area, file %s", s1);
			break;
		case HE_HDRPWRT:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error while writing header parameters, file %s", s1);
			break;
		case HE_HDRWRT:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error while writing header, file %s", s1);
			break;
		case HE_IMSG:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s", s1);
			break;
		case HE_MUTEX:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "flags are mutually exclusive\n%s", s1);
			break;
		case HE_ROI8C:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: packed image ROI - columns not multiple of 8, clearing ROI", s1);
			break;
		case HE_ROI8:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: packed image ROI - columns not multiple of 8", s1);
			break;
		case HE_RNG:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: invalid range", s1);
			break;
		case HE_MSG:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s", s1);
			break;
		case HE_C_NLV:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched number of pyramid levels, file: %s", s1);
			break;
		case HE_C_OCOL:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched total number of columns, file: %s", s1);
			break;
		case HE_C_OROW:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched total number of rows, file: %s", s1);
			break;
		case HE_RCSELN:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: row/column selection out of range", s1);
			break;
		case HE_INVFILE:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "invalid image filename %s", s1);
			break;
		case HE_MISSFILE:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "missing image filename(s)\n%s", s1);
			break;
		case HE_FILECNT:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "too many image filenames\n%s", s1);
			break;
		case HE_SYNTAX:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "invalid syntax\n%s", s1);
			break;
		case HE_COL3:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "can't convert 1-color image with numcolor!=3 to 3-color, file %s", s1);
			break;
		case HE_COL1:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "input 3-color image has numcolor>1, file %s", s1);
			break;
		case HE_FMTSLEN:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "length of formats array mismatch, file: %s", s1);
			break;
		case HE_PTWICE:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "flag `-%s' specified more than once", s1);
			break;
		case HE_SIGMA:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: sigma less than or equal to zero", s1);
			break;
		case HE_C_FRMC:
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched number of frames and numcolor>1, file: %s", s1);
			break;

		// one string, one int SD
		case HE_REFL:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: Invalid reflection type %d", s1,i1);
			break;
		case HE_FFTI:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: strange index=%d", s1,i1);
			break;
		case HE_CTORTP:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: unknown complex-to-real conversion: %d", s1,i1);
			break;
		case HE_HDPTYPE:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: invalid extended parameter format %d", s1,i1);
			break;
		case HE_MSKFUNSUBR:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: bad mask function number %d", s1,i1);
			break;
		case HE_RTOCTP:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: unknown real-to-complex conversion: %d", s1,i1);
			break;
		case HE_WINDSZ:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: invalid window size (%d)", s1,i1);
			break;

		// one int, one string DS
		case HE_MSKFUNFILE:
			i1 = va_arg(ap,int);
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "bad mask function number %d, file: %s", i1,s1);
			break;
		case HE_HDWPTYPE:
			i1 = va_arg(ap,int);
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "invalid extended parameter format %d during write, file %s", i1,s1);
			break;
		case HE_HDRPTYPE:
			i1 = va_arg(ap,int);
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "invalid extended parameter format %d, file %s", i1,s1);
			break;
		case HE_WRITEFRFILE:
			i1 = va_arg(ap,int);
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error writing frame %d to file %s", i1,s1);
			break;
		case HE_READFRFILE:
			i1 = va_arg(ap,int);
			s1 = va_arg(ap,char *);
			snprintf(hipserr,200, "error reading frame %d from file %s", i1,s1);
			break;

		// two strings SS
		case HE_COLSPEC:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: invalid color specification: `%s'", s1,s2);
			break;
		case HE_PNAME:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: parameter name contains white space: `%s'", s1,s2);
			break;
		case HE_PCOUNT:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: supplied count doesn't match that of `%s'", s1,s2);
			break;
		case HE_COLOVF:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: colormap table overflow, file: %s", s1,s2);
			break;
		case HE_HDRPTYPES:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "invalid extended parameter format %s, file %s", s1,s2);
			break;
		case HE_MISSFPAR:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "missing parameter for flag -%s\n%s", s1,s2);
			break;
		case HE_UNKFLG:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "unrecognised flag option %s\n%s", s1,s2);
			break;
		case HE_ROI8F:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: packed image ROI - columns not multiple of 8, file: %s", s1,s2);
			break;
		case HE_C_FMT:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "mismatched pixel format (%s), file: %s", s1,s2);
			break;
		case HE_MISSPAR:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: can't find extended parameter %s", s1,s2);
			break;
		case HE_FMTFILE:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "can't handle pixel format %s, file: %s", s1,s2);
			break;

		// one string, two ints SDD
		case HE_HDRXOV:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			snprintf(hipserr,200,
		"header parameters overflow, file: %s, size should be %d and was %d", s1,i1,i2);
			break;
		case HE_REQ:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: reqested %d, got %d", s1,i1,i2);
			break;
		case HE_MSKCNT:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			snprintf(hipserr,200, "%s: mask function %d, bad mask count %d", s1,i1,i2);
			break;

		// string, int, string SDS
		case HE_METH:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: unknown method (%d), file: %s", s1,i1,s2);
			break;

		// three strings SSS
		case HE_CONVI:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			snprintf(hipserr,200, "converting from %s to %s via integer, file: %s", s1,s2,s3);
			break;
		case HE_INVFPAR:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			snprintf(hipserr,200, "invalid parameter \"%s\" for flag -%s\n%s", s1,s2,s3);
			break;
		case HE_FMT2SUBR:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: can't handle pixel format combination %s/%s", s1,s2,s3);
			break;
		case HE_FMTSUBRFILE:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: can't handle pixel format %s, file: %s", s1,s2,s3);
			break;
		case HE_CONV:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			snprintf(hipserr,200, "converting from %s to %s, file: %s",
				s1,s2,s3);
			break;

		// four ints, DDDD
		case HE_ROI:
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			i3 = va_arg(ap,int);
			i4 = va_arg(ap,int);
			snprintf(hipserr,200, "setroi: ROI out of bounds, first=(%d,%d), size=(%d,%d)",
				i1,i2,i3,i4);
			break;

		// string, four ints, string, SDDDDS
		case HE_XINC:
			s1 = va_arg(ap,char *);
			i1 = va_arg(ap,int);
			i2 = va_arg(ap,int);
			i3 = va_arg(ap,int);
			i4 = va_arg(ap,int);
			s2 = va_arg(ap,char *);
			snprintf(hipserr,200,
		"header parameters inconsistency for (%s %d %d %d) offset is %d, file: %s",
				s1,i1,i2,i3,i4,s2);
			break;
		// four strings SSSS
		case HE_FMT3SUBR:
			s1 = va_arg(ap,char *);
			s2 = va_arg(ap,char *);
			s3 = va_arg(ap,char *);
			s4 = va_arg(ap,char *);
			snprintf(hipserr,200, "%s: can't handle pixel format combination %s/%s/%s",
					s1,s2,s3,s4);
			break;

		default:
			snprintf(hipserr,200, "perr: unknown error code %d", error_code);
			break;
	}
	va_end(ap);

	/* jbm:  old herrs.c file had table w/ error messages and level of severity
	 * of each errors...
	 */

	snprintf(DEFAULT_ERROR_STRING,LLEN,"%s",hipserr);
	NWARN(DEFAULT_ERROR_STRING);

#ifdef FOOBAR
	if (h_errors[error_code-1].h_errsev >= hipserrprt ||
	    h_errors[error_code-1].h_errsev >= hipserrlev){
		snprintf(DEFAULT_ERROR_STRING,LLEN,"%s",hipserr);
		NWARN(DEFAULT_ERROR_STRING);
	}
	if (h_errors[error_code-1].h_errsev >= hipserrlev){
		NADVISE("NOT exiting despite severe HIPS error");
		/* exit(h_errors[n-1].h_errsev); */
	}
#endif // FOOBAR

	return(HIPS_ERROR);
}

