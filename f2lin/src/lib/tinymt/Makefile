#for GNU make

#DDEBUG = -O0 -g -ggdb -DDEBUG=1

CC = gcc -Wall -Wextra -Wsign-compare -Wconversion -Wmissing-prototypes \
-O3 -std=c99 $(DDEBUG)
#CC = icc -Wall -O3 -std=c99 -Wmissing-prototypes $(DDEBUG)

all: check64

check64:  check64.c tinymt64.o
	${CC} -o $@  check64.c tinymt64.o ${LINKOPT}

doc: doxygen.cfg tinymt32.c tinymt64.c tinymt32.h tinymt64.h mainpage.txt
	doxygen doxygen.cfg

.c.o:
	${CC} -c $<

clean:
	rm -rf *.o *~ *.dSYM html
