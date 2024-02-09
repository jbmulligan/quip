
#include "quip_config.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>	/* atoi() */
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* usleep */
#endif

#include "quip_prot.h"
#include "query_bits.h"		// LLEN
#include "serbuf.h"
#include "serial.h"

#ifdef QUIP_DEBUG
static u_long sb_debug=0;

#define CHECK_DEBUG_INITED						\
				if( sb_debug == 0 ){			\
					sb_debug = add_debug_module("expect_string");	\
				}
#endif /* QUIP_DEBUG */

static int i_string=0;
#define N_STRINGS	10
#define CSTR_LEN	4

char *printable_version(int c)
{
	static char str[N_STRINGS][CSTR_LEN];
	char *s;

	s=str[i_string];
	i_string++;
	i_string %= N_STRINGS;

	if( c == '\n' )
		strcpy(s,"\\n");
	else if( c == '\r' )
		strcpy(s,"\\r");
	else if( c == '\b' )
		strcpy(s,"\\b");
	else if( c == '\t' )
		strcpy(s,"\\t");
	else if( c == '\f' )
		strcpy(s,"\\f");
	else if( c == '\v' )
		strcpy(s,"\\v");
	else if( c == '\0' )
		strcpy(s,"\\0");
	else if( iscntrl(c) )
		snprintf(s,CSTR_LEN,"^%c",'A'+c-1);
	else if( isprint(c) )
		snprintf(s,CSTR_LEN,"%c",c);
	else
		snprintf(s,CSTR_LEN,"\%3x",c);
			
	return(s);
}

char *printable_string(const char *s)
{
	static char pbuf[LLEN];
	char *p;

	pbuf[0] = 0;
	while(*s){
		p = printable_version(*s);
		strcat(pbuf,p);
		s++;
	}
	return(pbuf);
}

void show_buffer(Serial_Buffer *sbp)
{
	u_char *s;
	int i;

/*snprintf(ERROR_STRING,LLEN,"show_buffer 0x%lx, %d received, %d scanned",(u_long)sbp,sbp->sb_n_recvd,sbp->sb_n_scanned);*/
/* advise(ERROR_STRING);*/
	s=sbp->sb_buf+sbp->sb_n_scanned;
	while( *s ){
		fputs(printable_version(*s),stderr);
		if( *s == '\n' ) putc('\n',stderr);
		s++;
	}
	fputs("#end\n",stderr);

	for(i=0;i<sbp->sb_n_scanned;i++)
		putc(' ',stderr);
	putc('^',stderr);
	putc('\n',stderr);
	fflush(stderr);
}

void reset_buffer(Serial_Buffer *sbp)
{
/*advise("RESET_BUFFER"); */
	sbp->sb_buf[0]=0;
	sbp->sb_n_recvd = 0;
	sbp->sb_n_scanned = 0;
}

/* replenish_buffer will read up to max_expected chars, but is not guaranteed to read any.
 * the characters are placed in the response buffer.
 */
		
int _replenish_buffer(QSP_ARG_DECL  Serial_Buffer *sbp,int max_expected)
{
	int n;

#ifdef QUIP_DEBUG
	CHECK_DEBUG_INITED
#endif

	/* BUG? are we guaranteed that this won't overflow? */
	if( sbp->sb_n_recvd + max_expected >= BUFSIZE ){
		snprintf(ERROR_STRING,LLEN,"trouble?  n_recvd = %d, max_expected = %d. BUFSIZE = %d",
			sbp->sb_n_recvd,max_expected,BUFSIZE);
		advise(ERROR_STRING);
	}
	n = recv_somex(sbp->sb_fd, &sbp->sb_buf[sbp->sb_n_recvd], BUFSIZE-sbp->sb_n_recvd, max_expected);
//#ifdef QUIP_DEBUG
//if( (debug & sb_debug) ){
//snprintf(msg_str,LLEN,"replenish_buffer:  expected up to %d chars, received %d",
//max_expected,n);
//prt_msg(msg_str);
//}
//#endif /* QUIP_DEBUG */

	sbp->sb_n_recvd += n;		/* Can n_recvd be >= BUFSIZE?? shouldnt... BUG? */
	sbp->sb_buf[sbp->sb_n_recvd]=0;	/* make sure string is NULL-terminated */
	return(n);
}

#define MAX_TRIES	5000		/* 5 seconds? */

/*
 * 5000 sleeps of 200 usec, should be about 1 second?
 */

int _buffered_char(QSP_ARG_DECL  Serial_Buffer *sbp)
{
	u_char *buf;
	int n_tries=0;

#ifdef QUIP_DEBUG
	CHECK_DEBUG_INITED
#endif

	buf = sbp->sb_buf + sbp->sb_n_scanned;

#ifdef QUIP_DEBUG
/*
if( debug & sb_debug ){
if( *buf != 0 ) prt_msg_frag(".");
else prt_msg_frag(",");
}
*/
#endif /* QUIP_DEBUG */

/*snprintf(ERROR_STRING,LLEN,"buffered_char 0x%lx, %d received, %d scanned",(u_long)sbp,sbp->sb_n_recvd,sbp->sb_n_scanned);*/
/*advise(ERROR_STRING); */

	if( *buf == 0 ){
		int n;
		n=n_serial_chars(sbp->sb_fd);
		while(n==0 && n_tries < MAX_TRIES ){
			usleep(200);
			n=n_serial_chars(sbp->sb_fd);
			n_tries++;
		}
		if( n == 0 ){
			if( n_tries >= MAX_TRIES ){
				warn("buffered_char:  No readable chars from serial device, check power");
				return(-1);
			}
		}
//#ifdef QUIP_DEBUG
//if( debug & sb_debug ){
//snprintf(ERROR_STRING,LLEN,"buffered char 0x%lx:  there are %d new chars available, calling replenish_buffer",(u_long) sbp,n);
//advise(ERROR_STRING);
//}
//#endif /* QUIP_DEBUG */
		replenish_buffer(sbp,n);
/*show_buffer(sbp); */
	}

//#ifdef QUIP_DEBUG
//if( debug & sb_debug ){
//snprintf(ERROR_STRING,LLEN,"buffered_char returning '%s' (0x%x)",printable_version(*buf),*buf);
//advise(ERROR_STRING);
//}
//#endif /* QUIP_DEBUG */
	return(*buf);
}

/* This used to be called read_until, and only read until the first char
 * was found.  That should be called read_until_char !?
 */

void _read_until_string(QSP_ARG_DECL  char *dst,Serial_Buffer *sbp, const char *marker,
							Consume_Flag consume)
{
	int c;
	const char *w;

	/* BUG?  should we be careful about not overrunning dst? */

	w=marker;
	while( 1 ){

		c = buffered_char(sbp);
		sbp->sb_n_scanned ++;
		*dst++ = c;

		if( c == *w ){	/* what we expect? */
			w++;
			if( *w == 0 ){	/* done? */
				/* We have put the expected string in the buffer,
				 * now get rid of it.
				 */
				dst -= strlen(marker);
				/* If the consume flag is not set,
				 * then we leave the marker in the
				 * input buffer to be read later.
				 */
				if( consume == PRESERVE_MARKER )
					sbp->sb_n_scanned -= strlen(marker);
				*dst=0;
				return;
			}
		} else {
			w=marker;	/* back to the start */
		}
	}
	/* NOTREACHED */
}

/* 
 * If the remote device drops one char from the expected string, we catch that here,
 * but we don't handle well the case of unexpected chars...
 *
 * expect_string() now returns the received string, or NULL for a perfect match.
 */

char *_expect_string(QSP_ARG_DECL  Serial_Buffer *sbp, const char *expected_str)
{
	int i_e;		/* index of expected char */
	int i_r;		/* index of received char */
	int expected, received;
	static char rcvd_str[LLEN];	/* BUG - is this big enough? */
	int mismatch=0;
	int reading=1;

#ifdef QUIP_DEBUG
	CHECK_DEBUG_INITED
#endif

	expected=0;		// pointless initialization to quiet compiler

#ifdef QUIP_DEBUG
if( debug & sb_debug ){
//int n;
snprintf(ERROR_STRING,LLEN,"expect_string(\"%s\");",printable_string(expected_str));
advise(ERROR_STRING);
//n = n_serial_chars(sbp->sb_fd);
//snprintf(ERROR_STRING,LLEN,"%d chars available on serial port",n);
//advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
	i_r=i_e=0;
	while( reading ){
		if( !mismatch ) {
			// expected always gets set the first time through.
			expected = expected_str[i_e];
			i_e++;
		}

		received = buffered_char(sbp);
		if( received < 0 ){	/* no chars? */
#ifdef QUIP_DEBUG
if( debug & sb_debug ){
snprintf(msg_str,LLEN,"expect_string:  no buffered char!?");
advise(msg_str);
}
#endif /* QUIP_DEBUG */
			goto finish_expect_string;
		} else if( received == '\n' || received == '\r' ){
			if( mismatch )
				goto finish_expect_string;
		}


		sbp->sb_n_scanned ++;

		/* is this the first mismatch? */
		if( (!mismatch) ){
			if( received != expected ){
#ifdef QUIP_DEBUG
if( debug & sb_debug ){
snprintf(msg_str,LLEN,"expect_string:  received 0x%x, expected 0x%x",received,expected);
advise(msg_str);
}
#endif /* QUIP_DEBUG */

				/* If we have a partial match already, copy into the
				 * received buffer...
				 */
				if( i_e > 1 ){
					strncpy(rcvd_str,expected_str,i_e-1);
					i_r = i_e-1;
				}
				mismatch=1;
				sbp->sb_n_scanned --;
			}
		} else {
//if( mismatch ){
//snprintf(msg_str,LLEN,"expect_string:  received '%s' after mismatch",printable_version(received));
//advise(msg_str);
//}
			rcvd_str[i_r] = received;
			i_r++;
			if( i_r >= LLEN ){
				snprintf(ERROR_STRING,LLEN,"expect_string:  too many received characters (%d max)",LLEN-1);
				warn(ERROR_STRING);
			}
		}

		/* Stop if we reach the end of the expected string with no error */
		/* Otherwise read a complete line */
		if( mismatch == 0 && expected_str[i_e] == 0 )
			reading=0;
	}

finish_expect_string:

	rcvd_str[i_r] = 0;	/* terminate string */

#ifdef QUIP_DEBUG
//if( debug & sb_debug ){ prt_msg("#"); }
#endif /* QUIP_DEBUG */

	if( mismatch ) {
#ifdef QUIP_DEBUG
if( debug & sb_debug ){
snprintf(ERROR_STRING,LLEN,"expect_string:  expected \"%s\", but received \"%s\"",
	printable_string(expected_str),printable_string(rcvd_str));
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
		return(rcvd_str);
	} else {
#ifdef QUIP_DEBUG
if( debug & sb_debug ){
snprintf(ERROR_STRING,LLEN,"expect_string:  received expected string \"%s\";",
	printable_string(expected_str));
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
		return(NULL);
	}

} /* end expect_string() */

/* What does get_token do???
 * The original version filled the token buffer with alphanumeric characters,
 * up until a white space character.  But for the knox switcher condensed map
 * format, the numbers are packed with letters.  So we only want to pack digit
 * characters.  We may have to revisit this if we have a device that transmists hex...
 */

#define MAX_TOKEN_CHARS	3

static int get_token(QSP_ARG_DECL  Serial_Buffer *sbp, char *token)
{
	int scanning;
	int n_scanned=0;
	int i=0;
	u_char *s;

	scanning=1;
	token[0]=0;
	s = &sbp->sb_buf[sbp->sb_n_scanned];

	while(scanning){

#ifdef QUIP_DEBUG
/*
if( debug & sb_debug ){
snprintf(ERROR_STRING,LLEN,"get_token:  input string is \"%s\"",s);
advise(ERROR_STRING);
}
*/
#endif /* QUIP_DEBUG */
		if( *s == 0 ){
#ifdef QUIP_DEBUG
/*
if( debug & sb_debug ){
snprintf(ERROR_STRING,LLEN,"get_token fetching more input, position = %d",s-sbp->sb_buf);
advise(ERROR_STRING);
}
*/
#endif /* QUIP_DEBUG */
			replenish_buffer(sbp,MAX_TOKEN_CHARS);
		} else if( isdigit(*s) ){
			/* Store this char */
			if( i < MAX_TOKEN_CHARS ){
				token[i] = *s;
				s++;
				i++;
				n_scanned++;
#ifdef QUIP_DEBUG
/*
if( debug & sb_debug ){
snprintf(ERROR_STRING,LLEN,"get_token:  token = \"%s\", \"%s\" left to scan",token,s);
advise(ERROR_STRING);
}
*/
#endif /* QUIP_DEBUG */
			} else {
				snprintf(ERROR_STRING,LLEN,"get_token:  too many token chars!?");
				warn(ERROR_STRING);
				s++;
				n_scanned++;
			}
		} else {
			if( strlen(token) == 0 ){
				warn("get_token:  non-digit seen before any digits!?");
				s++;
				n_scanned++;
			} else {
				/* non-digit delimits end of token */
				token[i]=0;
				scanning=0;
			}
		}
	}
#ifdef QUIP_DEBUG
if( debug & sb_debug ){
snprintf(ERROR_STRING,LLEN,"get_token returning \"%s\" after scanning %d chars",token,n_scanned);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
	sbp->sb_n_scanned += n_scanned;

	return(n_scanned);
}

int _get_number( QSP_ARG_DECL  Serial_Buffer *sbp )
{
	int n_tok;
	char token[MAX_TOKEN_CHARS+1];

	n_tok = get_token(QSP_ARG  sbp,token);
	if( n_tok > 0 ){	/* success */
		return( atoi(token) );
	}
	warn("get_number:  no token!?");
/* show_buffer(sbp); */
	return(-1);	/* BUG?  can the jukebox ever return a negative number? */
}

/* this has the calling sequence like the old expect_string(), but prints a better error msg */

void _expected_response( QSP_ARG_DECL  Serial_Buffer *sbp, const char *expected_str )
{
	const char *s;

	s=expect_string(sbp,expected_str);
	if( s != NULL ){
		warn("response mismatch");
		snprintf(ERROR_STRING,LLEN,"Expected string:  \"%s\"",printable_string(expected_str));
		advise(ERROR_STRING);
		snprintf(ERROR_STRING,LLEN,"Received string:  \"%s\"",printable_string(s));
		advise(ERROR_STRING);
	}
}

