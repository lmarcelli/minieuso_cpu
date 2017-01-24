CC = g++
CFLAGS+=-g
INCLUDE = -I../../include -I/usr/include/flycapture
LIBS =   -lflycapture 

OUTDIR = /home/minieusouser/CPU/cameras/test/bin/

PROG=noverbose
OBJS=$(PROG).o

.PHONY: all clean

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LIBS} ${COMMON_LIBS} 
	 mv ${PROG} ${OUTDIR}
	 
%.o: %.cpp
	$(CC) -c $(CFLAGS) ${INCLUDE} -c $*.cpp

clean:
	rm -f ${OUTDIR}/${PROG} ${OBJS}	@echo "all cleaned up!"
