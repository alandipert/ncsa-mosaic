! MMS description file for LIBHTMLW
! Bjorn S. Nilsson, Aleph, CERN, 20-Nov-1993
! (Mosaic version 2.0)
! Motif 1.2 support added on 3-Jun-1994
! Mosaic 2.4 20-Aug-1994
!
! Usage:
!       $ MMS                           for DEC/UCX
!       $ MMS/MACRO=(MULTINET=1)        for Multinet
!       $ MMS/MACRO=(PATHWAY=1)         for Pathway
!	$ MMS/MACRO=(SOCKETSHR=1)	for SOCKETSHR  BGT
! Add a DEBUG=1 macro when debugging.
! Add a DECC=1 macro when compiling with DEC C.
! Add a MOTIF1_2 macro when Motif 1.2 is installed.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libhtmlw.olb

.IFDEF MOTIF1_2
MOTIF = MOTIF,MOTIF1_2
.ELSE
MOTIF = MOTIF
.ENDIF

.IFDEF PATHWAY
.IFDEF DECC
.INCLUDE [-.TWG]DECC_PREFIX_RULES.MMS
CQUALC=/DECC/Standard=VAXC/Precision=SINGLE $(CC_PREFIX_NO_SIN)
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF
CDEFS = /DEFINE=($(MOTIF),XMOSAIC)

.ELSE ! Not PATHWAY

.IFDEF SOCKETSHR ! BGT
.IFDEF DECC ! BGT
CQUALC=/DECC/Standard=VAXC/Precision=SINGLE/Prefix=ANSI ! BGT
.ELSE ! BGT
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF ! BGT
CDEFS = /DEFINE=($(MOTIF),XMOSAIC,SOCKETSHR) ! BGT

.ELSE ! Not SOCKETSHR  BGT

.IFDEF MULTINET
.IFDEF DECC
CQUALC=/DECC/Standard=VAXC/Precision=SINGLE/Prefix=ANSI
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF
CDEFS = /DEFINE=($(MOTIF),XMOSAIC,MULTINET)

.ELSE ! Not MultiNet

!UCX
.IFDEF DECC
CQUALC=/DECC/Standard=VAXC/Precision=SINGLE
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF
CDEFS = /DEFINE=($(MOTIF),XMOSAIC)
.ENDIF
.ENDIF

.ENDIF ! BGT

.IFDEF DEBUG
CFLAGS = $(CQUALC)$(CDEFS)/NoOpt/Debug
.ELSE
CFLAGS = $(CQUALC)$(CDEFS)
.ENDIF

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
.IFDEF PATHWAY
	@ @[-.twg]def
.ELSE
	@ Define/NoLog Sys SYS$Library
.IFDEF DECC
.IFDEF ALPHA
	@ If F$TRNLNM("ALPHA$LIBRARY") .NES. "" Then Define/NoLog Sys Alpha$Library
.ELSE
	@ If F$TRNLNM("DECC$LIBRARY_INCLUDE") .NES. "" Then Define/NoLog Sys DECC$Library_Include
.ENDIF
.ENDIF
.ENDIF

OBJECTS = Odir:HTML.obj Odir:HTMLlists.obj Odir:HTMLformat.obj \
 Odir:HTMLparse.obj Odir:HTMLimages.obj Odir:HTMLwidgets.obj \
 Odir:HTML-PSformat.obj Odir:HTMLjot.obj Odir:HTMLtable.obj Odir:list.obj

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libhtmlw.olb built."

Odir:HTML.obj : HTML.c
Odir:HTMLlists.obj : HTMLlists.c
Odir:HTMLformat.obj : HTMLformat.c
Odir:HTMLparse.obj : HTMLparse.c
Odir:HTMLimages.obj : HTMLimages.c
Odir:HTMLwidgets.obj : HTMLwidgets.c
Odir:HTML-PSformat.obj : HTML-PSformat.c
Odir:HTMLjot.obj : HTMLjot.c
Odir:HTMLtable.obj : HTMLtable.c
Odir:list.obj : list.c

.c.obj :
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

