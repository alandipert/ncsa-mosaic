#
# Makefile for libnut (NCSA Unix Tools Library)
#

CC=gcc
CFLAGS=-g -O
ODIR=./objs
OBJDIR=objs
LIB=libnut.a
####Set in main Mosaic Makefile -- uncomment if not compiled under Mosaic
#RANLIB=ranlib
OBJ=$(ODIR)/mm.o \
$(ODIR)/str-tools.o \
$(ODIR)/system.o \
$(ODIR)/url-utils.o \
$(ODIR)/ellipsis.o

all: objs $(LIB)

$(LIB): $(OBJ)
	-rm -f $(LIB)
	ar rv $(LIB) $(OBJ)
	$(RANLIB) $(LIB)

$(ODIR)/mm.o: mm.c mm.h
	$(CC) $(CFLAGS) -c mm.c -o $(ODIR)/mm.o

$(ODIR)/str-tools.o: str-tools.c
	$(CC) $(CFLAGS) -c str-tools.c -o $(ODIR)/str-tools.o

$(ODIR)/system.o: system.c system.h
	$(CC) $(CFLAGS) -c system.c -o $(ODIR)/system.o

$(ODIR)/url-utils.o: url-utils.c url-utils.h
	$(CC) $(CFLAGS) -c url-utils.c -o $(ODIR)/url-utils.o

$(ODIR)/ellipsis.o: ellipsis.c
	$(CC) $(CFLAGS) -c ellipsis.c -o $(ODIR)/ellipsis.o

objs:
	mkdir objs

clean:
	rm -f $(LIB) $(OBJ)
