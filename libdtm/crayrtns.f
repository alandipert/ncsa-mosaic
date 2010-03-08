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

C	$Header: /X11/mosaic/cvsroot/xmosaic3/libdtm/crayrtns.f,v 1.1.1.1 1995/01/11 00:02:57 alanb Exp $

C	$Log: crayrtns.f,v $
C	Revision 1.1.1.1  1995/01/11 00:02:57  alanb
C	New CVS source tree, Mosaic 2.5 beta 4
C
c Revision 2.5  1994/12/29  23:39:27  alanb
c I'm committing with a new symbolic revision number.
c
c Revision 1.1.1.1  1994/12/28  21:37:30  alanb
c
c Revision 1.1.1.1  1993/07/04  00:03:09  marca
c Mosaic for X version 2 distribution
c
c Revision 1.1  1993/01/18  21:50:08  marca
c I think I got it now.
c
c Revision 1.2  1993/01/18  21:46:47  marca
c Plugging DTM in...
c
c Revision 1.4  91/06/11  15:21:47  sreedhar
c disclaimer added
c 
c Revision 1.3  1991/05/16  04:40:13  jefft
c Fixed bugs in TRIPLET conversion
c
c Revision 1.2  1991/01/31  17:06:05  jefft
c Fixed bug in Cray floating point conversion routines.
c
c Revision 1.1  90/11/08  16:30:17  jefft
c Initial revision
c 



C   Cray_to_Sun_32-bit_floating-point (with packing) conversion routine.
C   USAGE:   call cspk32 (carray, sarray, size)
C   WHERE    carray is the array of Cray floating point numbers
C            (64 bit) to be converted to 32-bit IEEE format
C            reals and packed 2 to 1 (high to low order) in sarray.
C            Size is the dimension of the input carray.
C            (sarray is assumed to be (size+1)/2 )
C            Icheck, if 1, provides checking for over/underflow
C            and writes the appropriate "infinity" pattern.
C
       subroutine DTMCSF (carray,sarray,size,icheck)
       integer size,carray(0:size-1),sarray(0:(size+1)/2-1)
       integer stemp1,stemp2,exp1,exp2,mant1,mant2
       if (icheck.eq.1) goto 30
       do 20 i=0,(size+1)/2-1
          sign1=and(carray(2*i),x'8000000000000000')
          sign2=and(carray(2*i+1),x'8000000000000000')
          exp1=shiftr(and(carray(2*i),x'7fff000000000000'),48)-16258
          exp2=shiftr(and(carray(2*i+1),x'7fff000000000000'),48)-16258
          mant1=shiftr(and(carray(2*i),x'00007fffff000000'),24)
     &    +shiftr(and(carray(2*i),x'0000000000800000'),23)
          mant2=shiftr(and(carray(2*i+1),x'00007fffff000000'),24)
     &    +shiftr(and(carray(2*i+1),x'0000000000800000'),23)
          stemp1=or(sign1, (shiftl(exp1,55) + shiftl(mant1,32)) )
          stemp2=or(sign2, (shiftl(exp2,55) + shiftl(mant2,32)) )
          stemp1=cvmgn(stemp1,0,carray(2*i))
          stemp2=cvmgn(stemp2,0,carray(2*i+1))
          sarray(i)=or(stemp1,shiftr(stemp2,32))
20     continue
       return
30     continue
       do 10 i=0,(size+1)/2-1
          sign1=and(carray(2*i),x'8000000000000000')
          sign2=and(carray(2*i+1),x'8000000000000000')
          exp1=shiftr(and(carray(2*i),x'7fff000000000000'),48)-16258
          exp2=shiftr(and(carray(2*i+1),x'7fff000000000000'),48)-16258
          mant1=shiftr(and(carray(2*i),x'00007fffff000000'),24)
     &    +shiftr(and(carray(2*i),x'0000000000800000'),23)
          mant2=shiftr(and(carray(2*i+1),x'00007fffff000000'),24)
     &    +shiftr(and(carray(2*i+1),x'0000000000800000'),23)
          stemp1=or(sign1, (shiftl(exp1,55) + shiftl(mant1,32)) )
          stemp2=or(sign2, (shiftl(exp2,55) + shiftl(mant2,32)) )
          stemp1=cvmgm(or(sign1,x'7f80000000000000'),stemp1,254-exp1)
          stemp2=cvmgm(or(sign2,x'7f80000000000000'),stemp2,254-exp2)
          stemp1=cvmgn(stemp1,0,carray(2*i))
          stemp2=cvmgn(stemp2,0,carray(2*i+1))
          stemp1=cvmgm(0,stemp1,exp1-1)
          stemp2=cvmgm(0,stemp2,exp2-1)
          sarray(i)=or(stemp1,shiftr(stemp2,32))
10     continue
       end


C   Sun_to_Cray_32-bit_Floating-point (with unpacking) conversion routine.
C   USAGE:   call scup32 (sarray, carray, size)
C   WHERE    sarray is the array of 32-bit IEEE floating-point numbers
C            (packed 2 per word) to be converted 64-bit Cray format
C            and stored in carray.  Size is the dimension
C            of the output carray.  (sarray is assumed to be (size+1)/2 ).
C            Icheck is not used.
C
       subroutine DTMSCF (sarray,carray,size,icheck)
       integer size,sarray(0:(size+1)/2-1),carray(0:size-1),temp
cdir$ ivdep
       do 40 i=(size+1)/2-1,0,-1
          carray(2*i)=and(sarray(i),x'ffffffff00000000')
          carray(2*i+1)=shiftl(sarray(i),32)
40     continue
       do 50 i=0, size-1
          temp=carray(i)
          CARRAY(I)=OR(OR(AND(carray(I),X'8000000000000000'),SHIFTR
     &      (AND(carray(I),X'7F80000000000000'),7)+shiftl(16258,48)),or(
     &shiftr(AND(carray(I),X'007FFFFF00000000'),8),X'0000800000000000'))
          carray(i)=cvmgn(carray(i),0,shiftl(temp,1))
50     continue
       end


C
C   Integer_Cray_to_Sun_32-bit (with packing) conversion routine.
C   USAGE:   call icspk32 (carray, sarray, size)
C   WHERE    carray is the array of 64-bit signed integers to be
C            into 32-bit integers packed 2 per word and stored in
C            sarray.  Size is the dimension of the input carray,
C            Sarray is assumed to be (size+1)/2.
C
       subroutine DTMCSI  (sarray,carray,size)
       integer size,sarray(0:(size+1)/2-1),carray(0:size-1),temp
cdir$ ivdep
       do 60 i=0, (size+1)/2-1
          sarray(i)=or(
     &      and(shiftl(carray(2*i),32),x'ffffffff00000000'),
     &      and(carray(2*i+1), x'00000000ffffffff'))
60     continue
       end



C
C   Integer_Sun_32-bit_to_Cray_64-bit with unpacking conversion routine.
C   USAGE:   call iscup32 (sarray, carray, size)
C   WHERE    sarray is the array of 32-bit signed integers packed 2
C            per word to be converted to 64 bit Cray integers and
C            stored in carray.  Size is the dimension of the output
C            carray.  (sarray is assumed to be (size+1)/2 ).
C
       subroutine DTMSCI (sarray,carray,size)
       integer size,sarray(0:(size+1)/2-1),carray(0:size-1),temp
cdir$ ivdep
       do 80 i=(size+1)/2-1,0,-1
          carray(2*i) = shiftr(sarray(i), 32)
          carray(2*i+1)=and(sarray(i),x'ffffffff')
80     continue
       do 90 i=0, size-1
          carray(i) = or(carray(i), cvmgz(0, x'ffffffff00000000',
     &      and(carray(i), x'80000000')))
90     continue
       end



C
C   Cray to Sun Triplet (with packing) conversion routine.
C   USAGE:   call DTMCST (carray, sarray, size)
C   WHERE    carray is the array of DTM triplet structures
C            (cray format) to be converted into standard format
C            and packed into sarray.
C            size is the dimension of the input carray,
C            sarray is assumed to be size/2.
C
       subroutine DTMCST (sarray,carray,size)
       integer size,sarray(0:size*2-1),carray(0:size*4-1)
       integer stemp1,stemp2,exp1,exp2,mant1,mant2
cdir$ ivdep
       do 100 i=0, size*2-1, 2
          sign2=and(carray(2*i+1),x'8000000000000000')
          exp2=shiftr(and(carray(2*i+1),x'7fff000000000000'),48)-16258
          mant2=shiftr(and(carray(2*i+1),x'00007fffff000000'),24)
     &    +shiftr(and(carray(2*i+1),x'0000000000800000'),23)
          stemp2=or(sign2, (shiftl(exp2,55) + shiftl(mant2,32)) )
          stemp2=cvmgm(or(sign2,x'7f80000000000000'),stemp2,254-exp2)
          stemp2=cvmgn(stemp2,0,carray(2*i+1))
          stemp2=cvmgm(0,stemp2,exp2-1)
          sarray(i)=or(and(shiftl(carray(2*i),32),x'ffffffff00000000'),
     &      shiftr(stemp2,32))

          sign1=and(carray(2*i+2),x'8000000000000000')
          sign2=and(carray(2*i+3),x'8000000000000000')
          exp1=shiftr(and(carray(2*i+2),x'7fff000000000000'),48)-16258
          exp2=shiftr(and(carray(2*i+3),x'7fff000000000000'),48)-16258
          mant1=shiftr(and(carray(2*i+2),x'00007fffff000000'),24)
     &    +shiftr(and(carray(2*i+2),x'0000000000800000'),23)
          mant2=shiftr(and(carray(2*i+3),x'00007fffff000000'),24)
     &    +shiftr(and(carray(2*i+3),x'0000000000800000'),23)
          stemp1=or(sign1, (shiftl(exp1,55) + shiftl(mant1,32)) )
          stemp2=or(sign2, (shiftl(exp2,55) + shiftl(mant2,32)) )
          stemp1=cvmgm(or(sign1,x'7f80000000000000'),stemp1,254-exp1)
          stemp2=cvmgm(or(sign2,x'7f80000000000000'),stemp2,254-exp2)
          stemp1=cvmgn(stemp1,0,carray(2*i+2))
          stemp2=cvmgn(stemp2,0,carray(2*i+3))
          stemp1=cvmgm(0,stemp1,exp1-1)
          stemp2=cvmgm(0,stemp2,exp2-1)
          sarray(i+1)=or(stemp1,shiftr(stemp2,32))
100    continue
       end



C
C   Sun to Cray Triplet with unpacking conversion routine.
C   USAGE:   call DTMSCT (sarray, carray, size)
C   WHERE    sarray is the array of 32-bit signed integers packed 2
C            per word to be converted to 64 bit Cray integers and
C            stored in carray.  Size is the dimension of the output
C            carray.  (sarray is assumed to be (size+1)/2 ).
C
       subroutine DTMSCT (sarray,carray,size)
       integer size,sarray(0:size*2-1),carray(0:size*4-1),temp
cdir$ ivdep
       do 110 i=size*2-1, 0, -1
          carray(2*i)=and(sarray(i),x'ffffffff00000000')
          carray(2*i+1)=shiftl(sarray(i),32)
110    continue
       do 120 i=0, size-1, 4

C  convert tag
          carray(i) = or(carray(i), cvmgz(0, x'ffffffff00000000',
     &      and(carray(i), x'80000000')))

C  convert 1st float
          temp=carray(i+1)
          CARRAY(I+1)=OR(OR(AND(carray(I+1),X'8000000000000000'),
     &      SHIFTR(AND(carray(I+1),X'7F80000000000000'),7)+
     &      shiftl(16258,48)),or(shiftr(AND(carray(I+1),
     &      X'007FFFFF00000000'),8),X'0000800000000000'))
          carray(i+1)=cvmgn(carray(i+1),0,shiftl(temp,1))

C  convert 2nd float
          temp=carray(i+2)
          CARRAY(I+2)=OR(OR(AND(carray(I+2),X'8000000000000000'),
     &      SHIFTR(AND(carray(I+2),X'7F80000000000000'),7)+
     &      shiftl(16258,48)),or(shiftr(AND(carray(I+2),
     &      X'007FFFFF00000000'),8),X'0000800000000000'))
          carray(i+2)=cvmgn(carray(i+2),0,shiftl(temp,1))

C  convert 3rd float
          temp=carray(i+3)
          CARRAY(I+3)=OR(OR(AND(carray(I+3),X'8000000000000000'),
     &      SHIFTR(AND(carray(I+3),X'7F80000000000000'),7)+
     &      shiftl(16258,48)),or(shiftr(AND(carray(I+3),
     &      X'007FFFFF00000000'),8),X'0000800000000000'))
          carray(i+3)=cvmgn(carray(i+3),0,shiftl(temp,1))

120    continue
       end

