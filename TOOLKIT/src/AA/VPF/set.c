/*************************************************************************
 *
 *N  Module SET.C
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This module contains functions that make up an abstract data type
 *     "set".  The data structures and algorithms herein allow programs
 *     to perform basic manipulations defined in the mathematics of set
 *     theory.  These operations are fundamental to relational database
 *     theory, as well.
 *
 *     Sets are initialized with a user-defined size, and elements in
 *     the set may be accessed from 0 up to and including the given
 *     set size.  All sets passed into functions in this module are
 *     expected to have been initialized with set_init().
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    N/A
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                       DOS Turbo C
 *    Nov 1991 - Embedded bit manipulation routines instead of using a
 *               separate module (speed & size).
 *    Jun 1992   UNIX Port
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    set_type set_init( int n );
 *    int  set_empty( set_type set );
 *    void set_insert( int element, set_type set );
 *    void set_delete( int element, set_type set );
 *    int set_member( int element, set_type set );
 *    int set_min( set_type set );
 *    int set_max( set_type set );
 *    int  num_in_set( set_type set );
 *    void set_on( set_type set );
 *    void set_off( set_type set );
 *    int  set_equal( set_type a, set_type b );
 *    void set_assign( set_type *a, set_type b );
 *    set_type set_union( set_type a, set_type b );
 *    set_type set_intersection( set_type a, set_type b );
 *    set_type set_difference( set_type a, set_type b );
 *    void set_nuke( set_type *set );
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    These functions are all ANSI C compatible.
 *E
 *************************************************************************/

#ifdef __MSDOS__
#include <alloc.h>
#include <mem.h>
#include <dos.h>
#else
#include <malloc.h>
#include <memory.h>
#endif

#if ( defined(CYGWIN) )
#include <cygwin_misc.h>
#else
#include <values.h>
#endif

#include <stdlib.h>
#include "set.h"

#include <stdio.h>

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

static unsigned char checkmask[] = {254,253,251,247,239,223,191,127};
static unsigned char setmask[] = {1,2,4,8,16,32,64,128};

#define BITSET(bit,byte)  ((byte | checkmask[bit]) ^ checkmask[bit])

#define SET_BIT(bit,byte)  (byte | setmask[bit])

/* Warning: UNSET_BIT should only be called if the bit is not
   already set.  If it is already set, this macro may actually
   turn the bit on. */
#define UNSET_BIT(bit,byte)  (byte ^ setmask[bit])

/* The number of bytes in the set.  The byte buffer should
   only access bytes 0 through (NBYTES(set)-1).  */
#define NBYTES(set)  ((set.size>>3L) + 1L)

#ifndef max
#define max(a,b)   ( (a > b) ? a : b )
#endif

/* #define BOUNDSCHECK 1 */


static unsigned char set_byte( int nbyte, set_type set )
{
   if ( (nbyte < 0) || (nbyte >= NBYTES(set)) ) return 0;
   return set.buf[nbyte];
}

#define SET_BYTE( nbyte, set, byte ) 				\
   if ( (nbyte < 0) || (nbyte >= NBYTES(set)) )                 \
      byte = 0;							\
   else								\
      byte = set.buf[nbyte];


/*************************************************************************
 *
 *N  set_off
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Turns each element in the set 'off'.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    set <inout> == (set_type) set to be acted upon.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   Nov 1991                   DOS Turbo C
 *    Brian Glover     Nov 1992                   MDB upgrade
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
void set_off( set_type set )
{
   memset(set.buf,0,(int)NBYTES(set));
}

/*************************************************************************
 *
 *N  set_on
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Turns each element in the set 'on'.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    set <inout> == (set_type) set to be acted upon.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   Nov 1991                   DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
void set_on( set_type set )
{
   register long i;
   unsigned char byte=255;

   /* Turn on all bits up through set.size. */
   /* All but the last byte. */
   memset(set.buf,byte,(set.size>>3L));
   /* The valid bits of the last byte. */
   for (i=(set.size>>3L)*8L;i<=set.size;i++)
      set_insert((int)i,set);
}



/*************************************************************************
 *
 *N  set_init
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Initialize the set for 'n' elements.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    n       <input> == (int) maximum number of elements in the set.
 *    return <output> == (set_type) initialized set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
set_type set_init( int n )
{
   set_type s;
   long nbytes;

   s.size = n;
   nbytes = NBYTES(s);
   s.buf = (unsigned char *)malloc((int)nbytes+1L);
   set_off(s);

   return s;
}

/*************************************************************************
 *
 *N  set_empty
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns TRUE if the given set is empty; else it
 *     returns FALSE.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    set     <input> == (set_type) set.
 *    return <output> == (int) TRUE[1] or FALSE[0].
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
int set_empty( set_type set )
{
   register long nbytes;
   register int i;

   nbytes = NBYTES(set);
   for (i=0;i<nbytes;i++) {
      if (set_byte(i,set)) {
	 return FALSE;
      }
   }
   return TRUE;
}

/*************************************************************************
 *
 *N  set_insert
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function inserts the given element into the specified set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    element <input> == (int) element to insert into the set.
 *    set     <inout> == (set_type) set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
void set_insert( int element,
		 set_type set )
{
   long nbyte,bit;
   unsigned char byte;

   if ((element<0)||(element>set.size)) {
#ifdef BOUNDSCHECK
      fprintf(stderr,"Invalid call to set_insert! (%ld, %ld)\n",
             element,set.size);
      exit(1);
#endif
      return;
   }
   nbyte = element>>3L; /* element/8 */
   bit = element%8L;
   SET_BYTE(nbyte,set,byte);
   byte = SET_BIT(bit,byte);
   set.buf[nbyte] = byte;
}

/*************************************************************************
 *
 *N  set_delete
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function deletes the given element from the specified set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    element <input> == (int) element to delete from the set.
 *    set     <inout> == (set_type) set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    void unset_bit()    BITSTUFF.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
void set_delete( int element,
		 set_type set )
{
   long nbyte,bit;
   unsigned char byte;

   if ((element<0)||(element>set.size)) {
#ifdef BOUNDSCHECK
      fprintf(stderr,"Invalid call to set_delete!\n");
      exit(1);
#endif
      return;
   }
   nbyte = element>>3L;  /* element/8 */
   bit = element%8L;
   SET_BYTE(nbyte,set,byte);
   if (!BITSET(bit,byte)) return;
   byte = UNSET_BIT(bit,byte);
   set.buf[nbyte] = byte;
}

/*************************************************************************
 *
 *N  set_member
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function determines whether a given element is a member of
 *     the specified set.  It returns either TRUE or FALSE.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    element <input> == (int) element to check in the set.
 *    set     <input> == (set_type) set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    void bitset()    BITSTUFF.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
int set_member( int element,
		set_type set )
{
   long nbyte,bit;
   unsigned char byte;

   if ((element < 0)||(element > set.size)) return FALSE;
   nbyte = element>>3L;  /* element/8L */
   bit = element%8L;
   SET_BYTE(nbyte,set,byte);
   return BITSET(bit,byte);
}

/*************************************************************************
 *
 *N  set_min
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the minimum element in the given set.
 *     If the set is empty, the return value is MAXLONG.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    set     <input> == (set_type) set.
 *    return <output> == (int) minimum element in the set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    void bitset()    BITSTUFF.C
 *    int set_empty( set_type set )     SET.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
int set_min( set_type set )
{
   register int nbyte, bit;
   register long nbytes, element;
   unsigned char byte;

   if (!set.size) return MAXLONG;

   /* Find the first byte with a bit set */
   nbytes = NBYTES(set);

   for (nbyte=0;nbyte<nbytes;nbyte++)
      if (set.buf[nbyte]) {
         byte = set.buf[nbyte];
	 break;
      }

   /* Now find the first bit set in the byte */
   element = nbyte*8L;
   for (bit=0; bit<8; bit++,element++) {
      if (element > set.size) return MAXLONG;
      if (BITSET(bit,byte)) return element;
   }
   return MAXLONG;
}

/*************************************************************************
 *
 *N  set_max
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the maximum element in the given set.
 *     If the set is empty, the return value is -MAXLONG.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    set     <input> == (set_type) set.
 *    return <output> == (int) maximum element in the set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *    Brian Glover     Nov  1992                  MDB upgrade
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
int set_max( set_type set )
{
   register int bit;
   register long nbytes, bytenum;
   unsigned char byte;

   if (!set.size) return -MAXLONG;

   /* Find the last byte with a bit set */
   nbytes = NBYTES(set);
   bytenum = nbytes;

   for (bytenum=bytenum-1;bytenum>=0;bytenum--) {
     if (set.buf[bytenum]) {
       byte = set.buf[bytenum];
       break;
     }
   }

   if (bytenum < 0) return -MAXLONG;

   for (bit=7;bit>=0;bit--) {
     if (BITSET(bit,byte)) {
       return ((bytenum*8L)+bit);
     }
   }

   return -MAXLONG;
}

/*************************************************************************
 *
 *N  num_in_set
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the number of elements in the given set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    set     <input> == (set_type) set.
 *    return <output> == (int) number of elements in the set.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    void bitset()    BITSTUFF.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
int num_in_set( set_type set )
{
   register int nbyte, bit;
   register long nbytes, n=0L;
   unsigned char byte;

   if (set.size == 0) return n;
   nbytes = NBYTES(set);
   for (nbyte=0;nbyte<nbytes;nbyte++) {
      byte = set_byte(nbyte,set);
      if (byte) {
	 for (bit=0;bit<8;bit++)
	    if (BITSET(bit,byte)) n++;
      }
   }
   return n;
}


/*************************************************************************
 *
 *N  set_equal
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function determines whether two sets are equal to each other.
 *     It returns TRUE or FALSE.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    a       <input> == (set_type) first set to compare.
 *    b       <input> == (set_type) second set to compare.
 *    return <output> == (int) TRUE if (a==b) or FALSE if (a!=b).
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
int  set_equal( set_type a,
		set_type b )
{

   if (a.size != b.size) return FALSE;
   if (memcmp(a.buf,b.buf,(int)NBYTES(a))==0)
      return TRUE;
   else
      return FALSE;
}


/*************************************************************************
 *
 *N  set_assign
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function assigns set a to be equal to set b.  If a and b are
 *     different sizes, the function will reallocate a to match b.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    a       <input> == (set_type *) set to be assigned.
 *    b       <input> == (set_type) set to assign to a.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
void set_assign( set_type *a,
		 set_type b )
{
   register int nbytes;

   nbytes = (int)NBYTES(b);

   if (a->size == b.size) {
      memcpy(a->buf,b.buf,(int)nbytes);
   } else {    /* a and b are different sizes */
      a->buf = (unsigned char *)realloc(a->buf,nbytes+1L);
      if (!a->buf) {
         fprintf(stderr,"Memory reallocation error in set_assign\n");
         exit(1);
      }
      memcpy(a->buf,b.buf,(int)nbytes);
      a->size = b.size;
   }
}

/*************************************************************************
 *
 *N  set_union
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Return the set C such that C = (A U B).  C is initialized within
 *     this function, and should be nuked when no longer needed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    a       <input> == (set_type) set to be unioned.
 *    b       <input> == (set_type) set to be unioned.
 *    return <output> == (set_type) (A U B).
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    set_type set_init()   SET.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
set_type set_union( set_type a,
		    set_type b )
{
   register int i;
   register long nbytes;
   set_type c;

   c = set_init( (int)max(a.size,b.size) );

   nbytes = NBYTES(c);

   for (i=0;i<nbytes;i++)
      c.buf[i] = set_byte(i,a) | set_byte(i,b);

   return c;
}


/*************************************************************************
 *
 *N  set_intersection
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Return the set C such that C = (A o B).  C is initialized within
 *     this function, and should be nuked when no longer needed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    a       <input> == (set_type) set to be intersectioned.
 *    b       <input> == (set_type) set to be intersectioned.
 *    return <output> == (set_type) (A o B).
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    set_type set_init()   SET.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
set_type set_intersection( set_type a,
			   set_type b )
{
   register int i;
   register long nbytes;
   set_type c;

   c = set_init( (int)max(a.size,b.size) );

   nbytes = NBYTES(c);
   for (i=0;i<nbytes;i++)
      c.buf[i] = set_byte(i,a) & set_byte(i,b);

   return c;
}

/*************************************************************************
 *
 *N  set_difference
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Return the set C such that C = (A - B).  C is initialized within
 *     this function, and should be nuked when no longer needed.
 *
 *     NOTE:  This function can be sped up, if necessary, by direct
 *     manipulation of the bytes and bits rather than the abstract
 *     set function calls used presently.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    a       <input> == (set_type) set to subtract from.
 *    b       <input> == (set_type) set to be subtracted.
 *    return <output> == (set_type) (A - B).
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    set_type set_init()   SET.C
 *    int set_member()      SET.C
 *    void set_insert()     SET.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
set_type set_difference( set_type a,
			 set_type b )
{
   register int i;
   set_type c;

   c = set_init( a.size );

   for (i=0;i<=a.size;i++) {
      if ( i > b.size ) {
	 if (set_member(i,a)) set_insert( i, c );
      } else {
	 if ((set_member(i,a)) && (!set_member(i,b))) set_insert(i,c);
      }
   }

   return c;
}


/*************************************************************************
 *
 *N  set_nuke
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Nucleate a set from existence.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    set     <inout> == (set_type *) set to be nuked.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1990                  DOS Turbo C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This function conforms to ANSI C standards.
 *E
 *************************************************************************/
void set_nuke( set_type *set )
{
   if (set->buf) free( set->buf );
   set->size = -1;
}
