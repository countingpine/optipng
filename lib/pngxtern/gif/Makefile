CC=cc
CFLAGS=-g
LD=${CC}
LDFLAGS=${CFLAGS}

all: gifdump

.c.o:
	${CC} -c ${CFLAGS} $<

gifdump: gifdump.o gifread.o
	${LD} ${LDFLAGS} -o $@ gifdump.o gifread.o

gifdump.o: gifdump.c gifread.h
gifread.o: gifread.c gifread.h

clean:
	rm -f gifdump gifdump.o gifread.o
