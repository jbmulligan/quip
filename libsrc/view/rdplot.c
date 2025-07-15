#include "quip_config.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "quip_prot.h"
#include "viewer.h"
#include "data_obj.h"
#include "xsupp.h"

void getone( FILE *fp, int *p )
{
	int i;

	i=getc(fp);
	i |= getc(fp) << 8;
	*p = i;
}

void getpair( FILE *fp, int *px, int *py )
{
	getone(fp,px);
	getone(fp,py);
}

void _rdplot(QSP_ARG_DECL  FILE *fp )
{
	int x1,x2,x3,y1,y2,y3,c;
	char modstr[32];
	char *s;

	if( !fp ) return;

	while( (c=getc(fp)) != EOF ){
		switch( c ){
			case 'e': xplot_erase(); break;
			case 's':
				getpair(fp,&x1,&y1);
				getpair(fp,&x2,&y2);
				xplot_space(x1,y1,x2,y2);
				break;
			case 't':
				{
					char labelstring[256];
					int i=0;

					while( (c=getc(fp)) != '\n' && c!=EOF ){
						labelstring[i++]=(char)c;
					}
					labelstring[i]=0;
					xplot_text(labelstring);
				}
				break;
			case 'l':
				getpair(fp,&x1,&y1);
				getpair(fp,&x2,&y2);
				xplot_line(x1,y1,x2,y2);
				break;
			case 'c':
				getpair(fp,&x1,&y1);
				getone(fp,&x2);
				xplot_move(x1,y1);
				xplot_circle(x2);
				break;
			case 'a':
				getpair(fp,&x1,&y1);
				getpair(fp,&x2,&y2);
				getpair(fp,&x3,&y3);
				xplot_arc(x1,y1,x2,y2,x3,y3);
				break;
			case 'm':
				getpair(fp,&x1,&y1);
				xplot_move(x1,y1);
				break;
			case 'n':
				getpair(fp,&x1,&y1);
				xplot_cont(x1,y1);
				break;
			case 'p':
				getpair(fp,&x1,&y1);
				xplot_point(x1,y1);
				break;
			case 'f':
				s=modstr;
				while((c=getc(fp)) != '\n' && c != EOF )
					*s++ = (char)c;
				*s=0;
				if( !strcmp(modstr,"solid") )
					xplot_select(1);
				else if( !strcmp(modstr,"dotted") )
					xplot_select(2);
				else if( !strcmp(modstr,"dotdashed") )
					xplot_select(3);
				else if( !strcmp(modstr,"shortdashed") )
					xplot_select(4);
				else if( !strcmp(modstr,"longdashed") )
					xplot_select(5);
				else warn("unsupported line color");
				break;
			default:
				snprintf(ERROR_STRING,LLEN,
				"unrecognized plot command '%c' (%o)",c,c);
				warn(ERROR_STRING);
				goto plotdun;
		}
	}
plotdun:
	fclose(fp);
}


