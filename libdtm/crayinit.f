C                         NCSA DTM version 2.0
C                               June 10, 1991
C
C NCSA DTM Version 2.0 source code and documentation are in the public
C domain.  Specifically, we give to the public domain all rights for future
C licensing of the source code, all resale rights, and all publishing rights.
C
C We ask, but do not require, that the following message be included in all
C derived works:
C
C Portions developed at the National Center for Supercomputing Applications at
C the University of Illinois at Urbana-Champaign.
C
C THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
C SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
C WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
C

C	$Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/crayinit.f,v 1.1.1.1 1995/01/11 00:02:57 alanb Exp $

C	$Log: crayinit.f,v $
C	Revision 1.1.1.1  1995/01/11 00:02:57  alanb
C	New CVS source tree, Mosaic 2.5 beta 4
C
c Revision 2.5  1994/12/29  23:39:23  alanb
c I'm committing with a new symbolic revision number.
c
c Revision 1.1.1.1  1994/12/28  21:37:30  alanb
c
c Revision 1.1.1.1  1993/07/04  00:03:09  marca
c Mosaic for X version 2 distribution
c
c Revision 1.1  1993/01/18  21:50:07  marca
c I think I got it now.
c
c Revision 1.2  1993/01/18  21:46:45  marca
c Plugging DTM in...
c
c Revision 1.3  91/06/13  21:23:57  jefft
c added type of function to prevent returning of a float
c 
c Revision 1.2  1991/06/11  15:21:54  sreedhar
c disclaimer added
c
c Revision 1.1  1990/11/08  16:29:43  jefft
c Initial revision
c

       integer function DTMINIT()
       integer i, tmp, pcnt
       integer DTMMIP, DTMMOP
       character*64 portname

       pcnt = 0

       do 10 i = 1, iargc()

         call getarg(i, portname)
         if (index(portname, '-DTMIN') .NE. 0) then
           call getarg(i+1, portname)
           tmp = DTMMIP(portname)
           pcnt = pcnt + 1
         elseif (index(portname, '-DTMOUT') .NE. 0) then
           call getarg(i+1, portname)
           tmp = DTMMOP(portname)
           pcnt = pcnt + 1
         elseif (index(portname, '-DTM') .NE. 0) then
           call getarg(i+1, portname)
           tmp = DTMMOP(portname)
           pcnt = pcnt + 1
         endif

 10    continue

       DTMINIT = pcnt
       return
       end
