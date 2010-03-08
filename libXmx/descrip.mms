! MMS description file for LIBXMX
! Bjorn S. Nilsson, Aleph, CERN, 20-Nov-1993
! (Mosaic version 2.0)
! Motif 1.2 support added on 3-Jun-1994
! Mosaic 2.4 20-Aug-1994
!
! Usage:
!       $ MMS                           for DEC/UCX
!       $ MMS/MACRO=(MULTINET=1)        for MultiNet
! Add a DEBUG=1     MACRO when debugging.
! Add a DECC=1      MACRO when compiling with DEC C.
! Add a MOTIF1_2=1  MACRO when compiling with Motif 1.2
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libxmx.olb

.IFDEF DECC
.IFDEF PATHWAY
.INCLUDE [-.TWG]DECC_PREFIX_RULES.MMS
CQUALC=/DECC $(CC_PREFIX_NO_SIN)
.ELSE
.IFDEF MULTINET
CQUALC=/DECC/Prefix=ANSI
.ELSE
CQUALC=/DECC/Prefix=ALL
.ENDIF
.ENDIF
.ELSE ! Not DEC C
.IFDEF DECCVAXC
CQUALC=/VAXC
.ELSE
CQUALC=
.ENDIF
.ENDIF

.IFDEF MOTIF1_2
MOTIF = MOTIF1_2
.ELSE
MOTIF = MOTIF1_1
.ENDIF

.IFDEF MULTINET
TCPIP = ,MULTINET
.ELSE
.IFDEF PATHWAY
TCPIP = ,WIN_TCP
.ELSE
TCPIP =
.ENDIF
.ENDIF

.IFDEF DEBUG
CFLAGS = $(CQUALC)/Define=($(MOTIF)$(TCPIP))/NoOpt/Debug
.ELSE
CFLAGS = $(CQUALC)/Define=($(MOTIF)$(TCPIP))
.ENDIF

OBJECTS = Odir:xmx.obj Odir:xmx2.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
.IFDEF PATHWAY
	@ @[-.TWG]def
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libXmx.olb built."

Odir:xmx.obj : xmx.c
Odir:xmx2.obj : xmx2.c

.c.obj :
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

