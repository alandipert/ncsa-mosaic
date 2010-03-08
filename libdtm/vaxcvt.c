/*****************************************************************************
*
*                         NCSA DTM version 2.3
*                               May 1, 1992
*
* NCSA DTM Version 2.3 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
*
* We ask, but do not require, that the following message be included in all
* derived works:
*
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
*
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
*
*****************************************************************************/

#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<netinet/in.h>

#include	"dtmint.h"
#include	"debug.h"


#define		swap(x,y)	x ^= y;  y ^= x;  x ^= y


union float_uint_uchar {
    float f;
    int i;
    unsigned char c[4];
};

union double_uint_uchar {
    double f;
    int i[2];
    unsigned char c[8];
};

#ifdef DTM_PROTOTYPES
static int dtm_char(int mode,char *buf,int size)
#else
static int dtm_char(mode, buf, size)
  int	mode, size;
  char	*buf;
#endif
{
  DBGFLOW("# dtm_char called.\n");

  return size;
}

#ifdef DTM_PROTOTYPES
static int dtm_short(int mode,char *buf,int size)
#else
static int dtm_short(mode, buf, size)
  int	mode, size;
  char	*buf;
#endif
{
  int	i;

  DBGFLOW("# dtm_short called.\n");

  if (mode == DTMLOCAL) size /= 2;
  if (buf != NULL) {
     for (i=0; i<size; i++)  {
        swap(*buf, *(buf+1));
        buf += 2;
     }
  }

  return ((mode == DTMLOCAL) ? size : (size * 2));
}


#ifdef DTM_PROTOTYPES
static int dtm_int(int mode,char *buf,int size)
#else
static int dtm_int(mode, buf, size)
  int	mode, size;
  char	*buf;
#endif
{
  int	i;

  DBGFLOW("# dtm_int called.\n");

  if (mode == DTMLOCAL) size /= 4;
  if (buf != NULL) {
     for (i=0; i<size; i++)  {
        swap(*buf, *(buf+3));
        swap(*(buf+1), *(buf+2));
        buf += 4;
     }
  }

  return ((mode == DTMLOCAL) ? size : (size * 4));
}


#ifdef DTM_PROTOTYPES
static int dtm_float(int mode,char *buf,int size)
#else
static int dtm_float(mode, buf, size)
  int	mode, size;
  char	*buf;
#endif
{
  int	i;

  DBGFLOW("# dtm_float called.\n");

  if (mode == DTMLOCAL) {
     size >>= 2;
     if (buf != NULL) DTMVieeeF2vaxF(buf, size);
     return (size);
  } else {
     if (buf != NULL) DTMVvaxF2ieeeF(buf, size);
     return (size << 2);
  }
}


#ifdef DTM_PROTOTYPES
static int dtm_double(int mode,char *buf,int size)
#else
static int dtm_double(mode, buf, size)
  int	mode, size;
  char	*buf;
#endif
{

  DBGFLOW("# dtm_flt64 called.\n");

  if (mode == DTMLOCAL) {
     size >>= 3;
     if (buf != NULL) DTMVieeeD2vaxD(buf, size);
     return (size);
  } else {
     if (buf != NULL) DTMVvaxD2ieeeD(buf, size);
     return (size << 3);
  }
}


#ifdef DTM_PROTOTYPES
static int dtm_complex(int mode,char *buf,int size)
#else
static int dtm_complex(mode, buf, size)
  int	mode, size;
  char	*buf;
#endif
{
  int	i;

  DBGFLOW("# dtm_complex called.\n");

  if (mode == DTMLOCAL) {
     size >>= 2;
     if (buf != NULL) DTMVieeeF2vaxF(buf, size);
     return (size >> 1);
  } else {
     size <<= 1;
     if (buf != NULL) DTMVvaxF2ieeeF(buf, size);
     return (size << 2);
  }
}


#ifdef DTM_PROTOTYPES
static int dtm_triplet(int mode,char *buf,int size)
#else
static int dtm_triplet(mode, buf, size)
  int	mode, size;
  char	*buf;
#endif
{
  int	i;

  DBGFLOW("# dtm_triplet called.\n");

  if (mode == DTMLOCAL) size /= 16;
  if (buf != NULL) {
     for (i=0; i<size; i++)  {
        swap(*buf, *(buf+3));
        swap(*(buf+1), *(buf+2));
        buf += 4;
        if (mode == DTMLOCAL) DTMVieeeF2vaxF(buf, 3);
        else DTMVvaxF2ieeeF(buf, 3);
        buf += 12;
     }
  }

  return ((mode == DTMLOCAL) ? size : (size * 16));
}


/* conversion routine function table */
int	(*DTMconvertRtns[])() = {
		dtm_char,
                dtm_short,
                dtm_int,
                dtm_float,
                dtm_double,
                dtm_complex,
		dtm_triplet
		};

#ifdef DTM_PROTOTYPES
int DTMVvaxF2ieeeF(union float_uint_uchar f[],int size)
#else
int DTMVvaxF2ieeeF(f, size)
union float_uint_uchar f[];
int size;
#endif
{
    register int i;
    register unsigned char exp;
    unsigned char c0, c1, c2, c3;

    for (i=0; i<size; i++)
      {
	c0 = f[i].c[0];
	c1 = f[i].c[1];
	c2 = f[i].c[2];
	c3 = f[i].c[3];
        exp = (c1 << 1) | (c0 >> 7);             /* extract exponent */
        if (!exp && !c1) f[i].i = 0;             /* zero value */
        else if (exp>2) {                        /* normal value */
            f[i].c[0] = c1 - 1;                  /* subtracts 2 from exponent */
                /* copy mantissa, LSB of exponent */
            f[i].c[1] = c0;
            f[i].c[2] = c3;
            f[i].c[3] = c2;
        }
        else if (exp) {                          /* denormalized number */
            register int shft;

            f[i].c[0] = c1 & 0x80;               /* keep sign, zero exponent */
            shft = 3 - exp;
            /* shift original mant by 1 or 2 to get denormalized mant */
            /* prefix mantissa with '1'b or '01'b as appropriate */
            f[i].c[1] = ((c0 & 0x7f) >> shft) | (0x10 << exp);
            f[i].c[2] = (c0 << (8-shft)) | (c3 >> shft);
            f[i].c[3] = (c3 << (8-shft)) | (c2 >> shft);
        }
        else {                                   /* sign=1 -> infinity or NaN */
            f[i].c[0] = 0xff;                    /* set exp to 255 */
                /* copy mantissa */
            f[i].c[1] = c0 | 0x80;               /* LSB of exp = 1 */
            f[i].c[2] = c3;
            f[i].c[3] = c2;
        }
      }
    return(0);
}


#ifdef DTM_PROTOTYPES
int DTMVieeeF2vaxF(union float_uint_uchar f[],int size)
#else
int DTMVieeeF2vaxF(f, size)
union float_uint_uchar f[];
int size;
#endif
{
    register int i;
    register unsigned char exp;
    unsigned char c0, c1, c2, c3;

    for (i=0; i<size; i++)
      {
	 c0 = f[i].c[0];
	 c1 = f[i].c[1];
	 c2 = f[i].c[2];
	 c3 = f[i].c[3];
         exp = (c0 << 1) | (c1 >> 7); 		/* extract exponent */
         if (exp) {                             /* non-zero exponent */
            /* copy mantissa, last bit of exponent */
           f[i].c[0] = c1;
           f[i].c[2] = c3;
           f[i].c[3] = c2;
           if (exp<254)                         /* normal value */
             f[i].c[1] = c0 + 1;		/* actually adds two to exp */
           else {                               /* infinity or NaN */
             if (exp==254)                      /* unrepresentable - OFL */
               f[i].i = 0;                      /* set mant=0 for overflow */
             f[i].c[0] &= 0x7f;                 /* set last bit of exp to 0 */
             f[i].c[1] = 0x80;                  /* sign=1 exp=0 -> OFL or NaN */
          }
        }
        else if (c1 & 0x60) {                   /* denormalized value */
          register int shft;
    
          shft = (c1 & 0x40) ? 1 : 2;           /* shift needed to normalize */
            /* shift mantissa */
            /* note last bit of exp set to 1 implicitly */
          f[i].c[0] = (c1 << shft) | (c2 >> (8-shft));
          f[i].c[3] = (c2 << shft) | (c3 >> (8-shft));
          f[i].c[2] = c3 << shft;
          f[i].c[1] = (c0 & 0x80);              /* sign */
          if (shft==1) {                        /* set exp to 2 */
            f[i].c[1] |= 0x01;
            f[i].c[0] &= 0x7f;                  /* set LSB of exp to 0 */
          }
        }
        else f[i].i = 0;                            /* zero */
      }
    return(0);
}

#ifdef DTM_PROTOTYPES
int DTMVvaxD2ieeeD(union double_uint_uchar d[],int size)
#else
int DTMVvaxD2ieeeD(d, size)
union double_uint_uchar d[];
int size;
#endif
/* GFLOAT is much closer match to IEEE than DFLOAT */
/* but there is no support for GFLOAT under f77 */
/* and both cc and vcc don't work right with GLOAT */
{
    register int i;
    register int exp;
    unsigned char c0, c1, c2, c3, c4, c5, c6, c7;

    for (i=0; i<size; i++)
      {
	c0 = d[i].c[0];
	c1 = d[i].c[1];
	c2 = d[i].c[2];
	c3 = d[i].c[3];
	c4 = d[i].c[4];
	c5 = d[i].c[5];
	c6 = d[i].c[6];
	c7 = d[i].c[7];
        exp = (((c1 & 0x7f) << 1) | (c0 >> 7));	 /* extract exponent */
        if (!exp && !c1) {                       /* zero value */
	    d[i].i[0] = 0;
	    d[i].i[1] = 0;
	} else {
	    if (exp) {                           /* normal value */
	       exp += 894;
	       d[i].c[0] = (c1 & 0x80) | (exp >> 4);
	       d[i].c[1] = ((exp & 0xf) << 4) | (c0 >> 3);
            } else {                             /* sign=1 -> infinity or NaN */
               d[i].c[0] = 0xff;                 /* set exp to 2047 */
	       d[i].c[1] = 0xf0 | (c0 >> 3);     /*  and copy mantissa */
	    }
            d[i].c[2] = (c3 >> 3) | (c0 << 5);
            d[i].c[3] = (c2 >> 3) | (c3 << 5);
            d[i].c[4] = (c5 >> 3) | (c2 << 5);
            d[i].c[5] = (c4 >> 3) | (c5 << 5);
            d[i].c[6] = (c7 >> 3) | (c4 << 5);
            d[i].c[7] = (c6 >> 3);
        }
      }
    return(0);
}

#ifdef DTM_PROTOTYPES
int DTMVieeeD2vaxD(union double_uint_uchar d[],int size)
#else
int DTMVieeeD2vaxD(d, size)
union double_uint_uchar d[];
int size;
#endif
/* GFLOAT is much closer match to IEEE than DFLOAT */
/* but there is no support for GFLOAT under f77 */
/* and both cc and vcc don't work right with GLOAT */
{
    register int i;
    register int exp;
    unsigned char c0, c1, c2, c3, c4, c5, c6, c7;

    for (i=0; i<size; i++)
      {
	 c0 = d[i].c[0];
	 c1 = d[i].c[1];
	 c2 = d[i].c[2];
	 c3 = d[i].c[3];
	 c4 = d[i].c[4];
	 c5 = d[i].c[5];
	 c6 = d[i].c[6];
	 c7 = d[i].c[7];
         exp = (((c0 & 0x7f) << 4) | (c1 >> 4)) - 894;	/* extract exponent */
         if (exp > 0) {                             /* non-zero exponent */
            /* copy mantissa, last bit of exponent */
           d[i].c[0] = (c1 << 3) | (c2 >> 5);
           d[i].c[2] = (c3 << 3) | (c4 >> 5);
           d[i].c[3] = (c2 << 3) | (c3 >> 5);
           d[i].c[4] = (c5 << 3) | (c6 >> 5);
           d[i].c[5] = (c4 << 3) | (c5 >> 5);
           d[i].c[6] = (c7 << 3);
           d[i].c[7] = (c6 << 3) | (c7 >> 5);
           if (exp<=255)                        /* normal value */
             d[i].c[1] = (c0 & 0x80) | (exp >> 1);
           else {                               /* infinity or NaN */
             if (exp != 1153) {                 /* unrepresentable - OFL */
               d[i].i[0] = 0;                   /* set mant=0 for overflow */
               d[i].i[1] = 0;
	     }
             d[i].c[0] &= 0x7f;                 /* set last bit of exp to 0 */
             d[i].c[1] = 0x80;                  /* sign=1 exp=0 -> OFL or NaN */
           }
        }
	/* Some serious shifting of mantissa needed for exp values <= 0 */
        else {
	   d[i].i[0] = 0;                        /* zero */
           d[i].i[1] = 1;
	}
      }
    return(0);
}

#ifdef DTM_PROTOTYPES
int DTMVvaxG2ieeeD(union double_uint_uchar g[],int size)
#else
int DTMVvaxG2ieeeD(g, size)
union double_uint_uchar g[];
int size;
#endif
{
    register int i;
    register int exp;
    unsigned char c0, c1, c2, c3, c4, c5, c6, c7;

    for (i=0; i<size; i++)
      {
	c0 = g[i].c[0];
	c1 = g[i].c[1];
	c2 = g[i].c[2];
	c3 = g[i].c[3];
	c4 = g[i].c[4];
	c5 = g[i].c[5];
	c6 = g[i].c[6];
	c7 = g[i].c[7];
        exp = ((c1 & 0x7f) << 4) | (c0 >> 4);  /* extract exponent */
        if (!exp && !c1) {                     /* zero value */
	    g[i].i[0] = 0;
	    g[i].i[1] = 0;
	}
        else if (exp>2) {                        /* normal value */
	    exp -= 2;
            g[i].c[0] = (c1 & 0x80) | (exp >> 4);
            g[i].c[1] = (c0 & 0x0f) | ((exp & 0x0f) << 4);
            g[i].c[2] = c3;
            g[i].c[3] = c2;
            g[i].c[4] = c5;
            g[i].c[5] = c4;
            g[i].c[6] = c7;
            g[i].c[7] = c6;
        }
        else if (exp) {                          /* denormalized number */
            register int shft;

            g[i].c[0] = c1 & 0x80;               /* keep sign, zero exponent */
            shft = 3 - exp;
            /* shift original mant by 1 or 2 to get denormalized mant */
            /* prefix mantissa with '1'b or '01'b as appropriate */
            g[i].c[1] = ((c0 & 0x0f) >> shft) | (0x02 << exp);
            g[i].c[2] = (c0 << (8-shft)) | (c3 >> shft);
            g[i].c[3] = (c3 << (8-shft)) | (c2 >> shft);
            g[i].c[4] = (c2 << (8-shft)) | (c5 >> shft);
            g[i].c[5] = (c5 << (8-shft)) | (c4 >> shft);
            g[i].c[6] = (c4 << (8-shft)) | (c7 >> shft);
            g[i].c[7] = (c7 << (8-shft)) | (c6 >> shft);
        }
        else {                                   /* sign=1 -> infinity or NaN */
            g[i].c[0] = 0xff;                    /* set exp to 2047 */
                /* copy mantissa */
            g[i].c[1] = c0 | 0xf0;               /* LSBs of exp = 1 */
            g[i].c[2] = c3;
            g[i].c[3] = c2;
            g[i].c[4] = c5;
            g[i].c[5] = c4;
            g[i].c[6] = c7;
            g[i].c[7] = c6;
        }
      }
    return(0);
}

#ifdef DTM_PROTOTYPES
int DTMVieeeD2vaxG(union double_uint_uchar g[],int size)
#else
int DTMVieeeD2vaxG(g, size)
union double_uint_uchar g[];
int size;
#endif
{
    register int i;
    register int exp;
    unsigned char c0, c1, c2, c3, c4, c5, c6, c7;

    for (i=0; i<size; i++)
      {
	 c0 = g[i].c[0];
	 c1 = g[i].c[1];
	 c2 = g[i].c[2];
	 c3 = g[i].c[3];
	 c4 = g[i].c[4];
	 c5 = g[i].c[5];
	 c6 = g[i].c[6];
	 c7 = g[i].c[7];
         exp = (((c0 & 0x7f) << 4) | (c1 >> 4)) + 2;/* extract exponent */
         if (exp > 2) {                             /* non-zero exponent */
            /* copy mantissa */
           g[i].c[2] = c3;
           g[i].c[3] = c2;
           g[i].c[4] = c5;
           g[i].c[5] = c4;
           g[i].c[6] = c7;
           g[i].c[7] = c6;
           if (exp<=2047) {                     /* normal value */
             g[i].c[0] = (c1 & 0x0f) | ((exp & 0x0f) << 4);
             g[i].c[1] = (c0 & 0x80) | (exp >> 4);
           } else {                             /* infinity or NaN */
              if (exp==2048) {                  /* unrepresentable - OFL */
                 g[i].i[0] = 0;                 /* set mant=0 for overflow */
                 g[i].i[1] = 0;
	      }
              g[i].c[0] &= 0x0f;                /* set last bit of exp to 0 */
              g[i].c[1] = 0x80;                 /* sign=1 exp=0 -> OFL or NaN */
           }
         }
         else if (c1 & 0x0c) {                  /* denormalized value */
            register int shft;
    
            shft = (c1 & 0x08) ? 1 : 2;         /* shift needed to normalize */
            /* shift mantissa */
            /* note last bit of exp set to 1 implicitly */
            g[i].c[0] = (c1 << shft) | (c2 >> (8-shft));
            g[i].c[1] = (c0 & 0x80);              /* sign */
            g[i].c[2] = (c3 << shft) | (c4 >> (8-shft));
            g[i].c[3] = (c2 << shft) | (c3 >> (8-shft));
            g[i].c[4] = (c5 << shft) | (c6 >> (8-shft));
            g[i].c[5] = (c4 << shft) | (c5 >> (8-shft));
            g[i].c[6] = c7 << shft;
            g[i].c[7] = (c6 << shft) | (c7 >> (8-shft));
            g[i].c[1] = (c0 & 0x80);              /* sign */
            if (shft==1) {                        /* set exp to 2 */
               g[i].c[1] |= 0x20;
               g[i].c[1] &= 0xef;                  /* set LSB of exp to 0 */
            }
         }
         else {
	    g[i].i[0] = 0;                        /* zero */
            g[i].i[1] = 1;
	 }
      }
    return(0);
}

