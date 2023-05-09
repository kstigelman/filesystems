CC := gcc
CFLAGS := -O1 -Wall -Werror -std=c11 -g

.PHONY : all clean

all : create_fs runner

runner.o : runner.c fs.h

fs.o : fs.c fs.h

runner : runner.o fs.o
	$(LINK.c) runner.o fs.o -o runner

clean :
	$(RM) create_fs runner *.o
