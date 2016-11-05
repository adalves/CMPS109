# $Id$
# Ana Carolina Alves - adalves

GPP      = g++ -g -O0 -Wall -Wextra -std=gnu++14

DEPFILE  = Makefile.dep
HEADERS  = sockets.h protocol.h logstream.h
CPPLIBS  = sockets.cpp protocol.cpp 
CPPSRCS  = ${CPPLIBS} cix.cpp cixd.cpp
LIBOBJS  = ${CPPLIBS:.cpp=.o}
CIXOBJS  = cix.o ${LIBOBJS}
CIXDOBJS = cixd.o ${LIBOBJS}
OBJECTS  = ${CIXOBJS} ${CIXDOBJS}
EXECBINS = cix cixd
LISTING  = Listing.ps
SOURCES  = ${HEADERS} ${CPPSRCS} Makefile

all: ${DEPFILE} ${EXECBINS}

setup: ${EXECBINS}
	- mkdir serverdir
	- mkdir clientdir
	- cp cix clientdir/
	- cp cixd serverdir/
	- echo "server test file" > serverdir/test
	- echo "client test file" > clientdir/test

cix: ${CIXOBJS}
	${GPP} -o $@ ${CIXOBJS}

cixd: ${CIXDOBJS}
	${GPP} -o $@ ${CIXDOBJS}

%.o: %.cpp
	${GPP} -c $<

ci:
	- checksource ${SOURCES}
	- cid + ${SOURCES}

lis: all ${SOURCES} ${DEPFILE}
	mkpspdf ${LISTING} ${SOURCES} ${DEPFILE}

clean:
	- rm ${LISTING} ${LISTING:.ps=.pdf} ${OBJECTS} Makefile.dep

spotless: clean
	- rm ${EXECBINS}

dep:
	- rm ${DEPFILE}
	make --no-print-directory ${DEPFILE}

${DEPFILE}:
	${GPP} -MM ${CPPSRCS} >${DEPFILE}

again: ${SOURCES}
	make --no-print-directory spotless ci all lis

submit: ${SOURCES}
	submit cmps109-wm.w16 asg5 ${SOURCES}

include ${DEPFILE}

