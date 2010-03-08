LIBTARGET = libhtmlw.a

all: $(LIBTARGET)

CFILES = HTML.c HTMLlists.c HTMLformat.c HTMLparse.c HTMLimages.c HTMLwidgets.c HTML-PSformat.c DrawingArea.c HTMLjot.c HTMLtable.c list.c 

OBJS = $(CFILES:.c=.o)
$(OBJS): HTML.h HTMLP.h


$(LIBTARGET): $(OBJS)
	-rm -f $(LIBTARGET)
	ar rv $(LIBTARGET) $(OBJS)
	$(RANLIB) $(LIBTARGET)

HTMLimages.o: AnchoredImage.xbm DelayedImage.xbm

clean:
	-rm $(LIBTARGET) *.o
tags:
	etags -t *.[ch]
