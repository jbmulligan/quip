#!/bin/csh

# this line is a test modification for git.

# how we normally call configure in the vision lab

## on Mac OSX, check for fink installation
##if( -e /sw ) then
##./configure --enable-thread-safe-query --enable-n-processors=16 --enable-sse-extensions CPPFLAGS=-I/sw/include LDFLAGS=-L/sw/lib
## this version disables optimization!
#./configure --enable-thread-safe-query --enable-n-processors=16 CPPFLAGS=-I/sw/include LDFLAGS=-L/sw/lib CFLAGS=-g
#else

#./configure --enable-thread-safe-query --enable-n-processors=16
#./configure --disable-avi --enable-thread-safe-query --enable-n-processors=16 --disable-spinnaker
./configure --disable-avi --enable-thread-safe-query --enable-n-processors=16 --enable-rawvol=force

#endif

#./configure --enable-thread-safe-query

#./configure --enable-thread-safe-query --enable-n-processors=16

