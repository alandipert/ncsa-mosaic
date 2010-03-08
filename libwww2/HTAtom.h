/*  */

/*                      Atoms: Names to numbers                 HTAtom.h
**                      =======================
**
**      Atoms are names which are given representative pointer values
**      so that they can be stored more efficiently, and compaisons
**      for equality done more efficiently.
**
**      HTAtom_for(string) returns a representative value such that it
**      will always (within one run of the program) return the same
**      value for the same given string.
**
** Authors:
**      TBL     Tim Berners-Lee, WorldWideWeb project, CERN
**
**      (c) Copyright CERN 1991 - See Copyright.html
**
*/

#ifndef HTATOM_H
#define HTATOM_H

typedef struct _HTAtom HTAtom;
struct _HTAtom {
        HTAtom *        next;
        char *          name;
}; /* struct _HTAtom */


#ifdef __STDC__
extern HTAtom * HTAtom_for(char * string);
#else
extern HTAtom * HTAtom_for();
#endif

#ifdef __STDC__
extern HTAtom * HTAtom_exists(char * string);
#else
extern HTAtom * HTAtom_exists();
#endif

#define HTAtom_name(a) ((a)->name)

#endif  /* HTATOM_H */
/*

    */
