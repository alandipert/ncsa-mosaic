LIBTARGET = libwww.a

all: $(LIBTARGET)

CFILES = CUkerb.c HTAccess.c HTAlert.c HTAnchor.c HTAtom.c HTSort.c HTChunk.c \
  HTFTP.c HTFWriter.c HTFile.c HTFormat.c HTGopher.c HTInit.c HTList.c \
  HTMIME.c HTML.c HTMLDTD.c HTMLGen.c HTNews.c HTParse.c HTPlain.c \
  HTMosaicHTML.c HTString.c HTTCP.c HTTP.c HTTelnet.c HTWSRC.c HTWriter.c \
  SGML.c HTWAIS.c HTIcon.c HTCompressed.c HTAAUtil.c HTAssoc.c HTUU.c \
  HTAABrow.c HTMailto.c

OBJS = $(CFILES:.c=.o)

$(LIBTARGET): $(OBJS)
	-rm -f $(LIBTARGET)
	ar rv $(LIBTARGET) $(OBJS)
	$(RANLIB) $(LIBTARGET)

$(OBJS): HTUtils.h
HTFTP.o: HTFTP.h

clean:
	rm $(LIBTARGET) *.o
tags:
	etags -t *.[ch]
