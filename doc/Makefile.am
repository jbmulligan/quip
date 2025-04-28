
MAINTAINERCLEANFILES	=	Makefile.in
info_TEXINFOS		=	quip.texi
SUFFIXES		=	.html
HTML_DOCS		=	quip.html

TARGETS			=

#if HAVE_MAKEINFO

#TARGETS			+= pdf html quip.info
TARGETS			+= html quip.info

#endif

.texinfo.html:
	$(MAKEINFO) --html $<

.texi.html:
	$(MAKEINFO) --html $<

# The automatically-generated target "pdf" is not working...  Not sure why not?
quip.pdf:	quip.texi
	texi2dvi --pdf --batch --output=quip.pdf quip.texi

# makeinfo --html is not making the html when called from automake
all:	$(TARGETS) quip.pdf
	cp quip.pdf ../docs/quip_manual.pdf
#	cp -R quip.html ../docs/quip_web_manual
#	BUG ../docs/quip_web_manual must exist!
# html subdir here used to be quip_html???
#	cp -R quip_html/*.html ../docs/quip_web_manual
	cp -R quip/*.html ../docs/quip_web_manual

.PHONY:	html
html:	version.texi $(HTML_DOCS)


# what is the correct way to list an included file
# that should be part of the distribution?

noinst_HEADERS		= COPYING

