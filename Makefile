CC = gcc
CFLAGS = -Wall
LDFLAGS = -ldl -lpthread -lm
TARGET = soundkit
PREFIX = /usr/local

objects = soundkit.o

${TARGET}: ${objects}
	${CC} ${CFLAGS} -o ${TARGET} ${objects} ${LDFLAGS}

soundkit.o: sample.h miniaudio/miniaudio.h

.PHONY: clean install
clean:
	rm -f ${objects} ${TARGET}

install: ${TARGET}
	cp ${TARGET} ${PREFIX}/bin/
