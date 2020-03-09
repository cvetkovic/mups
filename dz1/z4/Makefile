MF=	Makefile

#COMPILER
CC=	gcc

CFLAGS=	-g
LFLAGS=	-lm

EXE=	sharpen

SRC= \
	sharpen.c \
	dosharpen.c \
	filter.c \
	cio.c \
	utilities.c

INC = \
	sharpen.h \
	utilities.h

.SUFFIXES:
.SUFFIXES: .c .o

OBJ=	$(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(EXE)

$(EXE):	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LFLAGS)

$(OBJ):	$(MF) $(INC)

clean:
	rm -f $(OBJ) $(EXE) core data/*_sharpened.pgm
