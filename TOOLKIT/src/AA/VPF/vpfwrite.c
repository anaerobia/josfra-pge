/*************************************************************************
 *
 *N  Module VPFWRITE.C
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This contains functions for writing data to VPF tables.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     N/A
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *     Original Coding:  Tom Wood	Fall 1990
 *       Modifications:	 David Flinn	January 1991
 *					July 1991
 *		         Barry Michaels  October 1991
 *					Modified from converter
 *					software (UNIX) for
 *					VPFVIEW software (DOS).
 *			 Jim TenBrink   October 1991
 *					Made vpfread.c and vpfwrite.c
 *					disjoint
 *                       Kevin McCarthy July 1993
 *                                      write_row now accomadates variable
 *                                      length rows
 *			Simon Tackley 1/25/95
 *			  added include <sys/types.h> 
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
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/

#ifdef __MSDOS__
#include <mem.h>
#include <io.h>
#include <alloc.h>
#include <dos.h>
#include <graphics.h>
#else
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#define   SEEK_SET    0         /* Turbo C fseek value */
#define   SEEK_CUR    1
#define   farmalloc   malloc    /* no farmallocs on UNIX */
#endif

#if ( defined(CYGWIN) )
#include <cygwin_misc.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "vpftable.h"

extern int STORAGE_BYTE_ORDER;

/* Include statically to reduce external module dependencies */

/* Allocate memory and check results */
static void *checkmalloc( unsigned long size )
{
   void *p;
   p = malloc( size );
   if (p == NULL) {
      printf("out of memory in vpfwrite");
#ifdef __MSDOS__
      printf("  %ld  %ld\n",size,farcoreleft());
      getch();
      if (getgraphmode() >= 0) closegraph();
#else
      printf("\n");
#endif
      exit(1);
   }
   return p;
}




/*************************************************************************
 *
 *N  write_key
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function writes an id triplet key from the specified file.
 *     It is assumed that there is enough free disk space to write to the
 *     file. It is also assumed that the file pointer (fp) is already opened
 *     for writing.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    key     <input> == (id_triplet_type) id triplet key.
 *    fp      <input> == (FILE *) input file pointer.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Dave Flinn       July 1991      Based on read_key in vpftable.c
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
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
int write_key( id_triplet_type key, FILE *fp )
{
  int size = 0 ;	/* to count size of key write */
  unsigned char tint ;
  short int tshort ;

   /* Assume that any count value has been written before this */
   /* Only write one key in this subroutine, do not write more */

  Write_Vpf_Char (&(key.type),fp,1);
  size += sizeof ( char ) ;

   switch (TYPE0(key.type)) {
   case 0:
     break;
   case 1:
     tint = (unsigned char) key.id ;
     Write_Vpf_Char ( &tint, fp, 1 ) ;
     size += sizeof ( char ) ;
     break;
   case 2:
     tshort = (short) key.id ;
     Write_Vpf_Short ( &tshort, fp, 1 ) ;
     size += sizeof ( short int ) ;
     break;
   case 3:
     Write_Vpf_Int (&(key.id), fp, 1 ) ;
     size += sizeof ( int ) ;
     break;
   }

   switch (TYPE1(key.type)) {
   case 0:
     break;
   case 1:
     tint = (unsigned char) key.tile ;
     Write_Vpf_Char ( &tint, fp, 1 ) ;
     size += sizeof ( char ) ;
     break;
   case 2:
     tshort = (short) key.tile ;
     Write_Vpf_Short ( &tshort, fp, 1 ) ;
     size += sizeof ( short int ) ;
     break;
   case 3:
     Write_Vpf_Int (&(key.tile), fp, 1 ) ;
     size += sizeof ( int ) ;
     break;
   }

   switch (TYPE2(key.type)) {
   case 0:
     break;
   case 1:
     tint = (unsigned char) key.exid ;
     Write_Vpf_Char ( &tint, fp, 1 ) ;
     size += sizeof ( char ) ;
     break;
   case 2:
     tshort = (short) key.exid ;
     Write_Vpf_Short ( &tshort, fp, 1 ) ;
     size += sizeof ( short int ) ;
     break;
   case 3:
     Write_Vpf_Int (&(key.exid), fp, 1 ) ;
     size += sizeof ( int ) ;
     break;
   }
  return size ;
}

/*************************************************************************
 *
 *N  write_next_row
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function writes the next row of the table.
 *     The parameter row must be initialized prior to this functional, either
 *     buy being read in from an existing table or set to valid values.
 *     A row with any empty columns should not be written out.
 *     The parameter table must be a valid table and initialized prior to
 *     this function, by vpf_open_table.  It is assumed that there is
 *     enough free disk space to write to the file. It is also assumed that
 *     the file pointer (table->fp) is already opened for writing. The
 *     variable count, set to the values in row, must be greater than 0,
 *     otherwise, if count is -1 the vpf_write functions will lock up
 *     (row[].count should never have a value of 0). Note that if stderr
 *     is used, it must be opened prior to this function.
 *
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    row        <input> == (row_type) the row to write to the table.
 *    table      <input> == (vpf_table_type *) vpf table structure.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Dave Flinn       July 1991      Based on read_next_row.
 *    Barry Michaels    Oct 1991      Added row as a parameter.
 *    JTB              10/91          guaranteed function always
 *                                    returns a value:
 *                                     0: record written
 *                                    -1: unknown field type
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
 *   None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
int write_next_row(row_type row, vpf_table_type * table )
{
   register int i,
		     j;
   char            * tptr,
		   * output ;
   int          recordsize = 0;
   int	     count;
   id_triplet_type * keys;
   unsigned int pos_for_ndx,
		     length;
   coordinate_type   dummycoord = {0.0,0.0};

   STORAGE_BYTE_ORDER = table->byte_order;

   table->nrows++;
   fseek(table->fp, 0L, SEEK_END);
   pos_for_ndx = ftell(table->fp); /* begining of new row */

   for (i = 0; i < table->nfields; i++) {   /* for each column */

     count = row[i].count ;          /* Retrieve count from row.  Should
					be 0 if variable length null */

     /* In case this column is variable length, write out count */

     if (count == 0) count = 1;

     if ( table->header[i].count < 0 ) {
       Write_Vpf_Int ( &count, table->fp, 1 ) ;
       recordsize += sizeof ( int ) ;
     }

     /* Now write out the data type */

     switch (table->header[i].type) {

     case 'T':
       if ( count == 0 ) 	/* Assume this is variable length text
				   and don't do anything */
	 break ;

       /* This loop insures that the exact number of characters are written
	  out to disk. */

       output = (char *) checkmalloc ( count + 1 ) ;  /* include null byte */
       for (j = 0, tptr = (char *)row[i].ptr; j < count; j++, tptr++)
	 if ( *tptr )
	   output[j] = *tptr ;
	 else
	   output[j] = SPACE ;
       output[count] = '\0' ;
       Write_Vpf_Char( output ,table->fp, count) ;
       free ( output ) ;
       recordsize += sizeof ( char ) * count ;
       break;

     case 'I':
       Write_Vpf_Int (row[i].ptr, table->fp, count ) ;
       recordsize += sizeof ( int ) * count ;
       break;

     case 'S':
       Write_Vpf_Short (row[i].ptr, table->fp, count ) ;
       recordsize += sizeof ( short int ) * count ;
       break;

     case 'F':
       Write_Vpf_Float (row[i].ptr, table->fp, count ) ;
       recordsize += sizeof ( float ) * count ;
       break;

     case 'R':
       Write_Vpf_Double (row[i].ptr, table->fp, count ) ;
       recordsize += sizeof ( double ) * count ;
       break;

     case 'D':	/* date has 21 chars in memory, not on disk */
       Write_Vpf_Date (row[i].ptr, table->fp, count ) ;
       recordsize += ( sizeof ( date_type ) - 1 ) * count ;
       break;

     case 'C':
       if (row[i].ptr) {
	  Write_Vpf_Coordinate(row[i].ptr,table->fp,count);
       } else {
	  for (j=0;j<count;j++)
	     Write_Vpf_Coordinate(&dummycoord,table->fp,count);
       }
       recordsize += sizeof ( coordinate_type ) * count ;
       break;

     case 'B':
       Write_Vpf_DoubleCoordinate(row[i].ptr,table->fp,count);
       recordsize += sizeof ( double_coordinate_type ) * count ;
       break;

     case 'Z':
       Write_Vpf_CoordinateZ(row[i].ptr,table->fp,count);
       recordsize += sizeof ( tri_coordinate_type ) * count ;
       break;

     case 'Y':
       Write_Vpf_DoubleCoordinateZ(row[i].ptr,table->fp,count);
       recordsize += sizeof ( double_tri_coordinate_type ) * count ;
       break;

     case 'K':
       keys = (id_triplet_type *) checkmalloc (count*sizeof(id_triplet_type)) ;
       memcpy ( (void *)keys, row[i].ptr, count*sizeof(id_triplet_type) ) ;
       for (j=0;j<count;j++)
	 recordsize += write_key ( keys[j], table->fp);
       free ( keys ) ;
       break;

     case 'X':
       /* do nothing */
       break;

     default:
       fprintf(stderr,"\nwrite_next_row: no such type < %c >",
	       table->header[i].type ) ;
       return(-1);
     }
   }

   table->size += recordsize;

   if ( table->xfp ) {  /* only for variable length columns */
     length = recordsize ;
     fseek( table->xfp, 0, SEEK_END );

     Write_Vpf_Int ( &pos_for_ndx, table->xfp, 1 ) ;
     Write_Vpf_Int ( &length, table->xfp, 1 ) ;
   }

   return recordsize;
}

/**************************************************************************
 *
 *N  write_row
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function writes the next row of the table.
 *     The parameter row must be initialized prior to this function, either
 *     by being read in from an existing table or set to valid values.
 *     A row with any empty columns should not be written out.
 *     The parameter table must be a valid table and initialized prior to
 *     this function, by vpf_open_table.  It is assumed that there is
 *     enough free disk space to write to the file. It is also assumed that
 *     the file pointer (table->fp) is already opened for writing. The
 *     variable count, set to the values in row, must be greater than 0,
 *     otherwise, if count is -1 the vpf_write functions will lock up
 *     (row[].count should never have a value of 0). Note that if stderr
 *     is used, it must be opened prior to this function.
 *
 *     If variable length rows are being written, the file should be opened
 *     for update (mode "r+") in vpf_open_table.
 *
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    rownum     <input> == (int) row number of the row write
 *                           to the table.
 *    row        <input> == (row_type) the row to write to the table.
 *    table      <input> == (vpf_table_type *) vpf table structure.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels    Dec 1992      Adapted from write_next_row()
 *    Kevin McCarthy    July 1993     Removed recordsize calculation: put
 *                                    in calculate_row_size and 
 *                                    calculate_key_size, added adjust_table_size
                                      for variable length writes
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
 *   None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
int write_row( int rownum,
                    row_type row, 
                    vpf_table_type * table )
{
   register int i,
		     j;
   char            * tptr,
		   * output ;
   int	     count;
   id_triplet_type * keys;
   coordinate_type   dummycoord = {0.0,0.0};
   int row_size, stored_row_size;

   if (rownum > table->nrows) return -1;

   stored_row_size = index_length(rownum, *table);
   row_size = calculate_row_size(row, table);

   if ( stored_row_size != row_size )
   {
      adjust_table_size(rownum, table, (int)row_size - stored_row_size);
   }


   STORAGE_BYTE_ORDER = table->byte_order;

   fseek(table->fp, index_pos(rownum,*table), SEEK_SET);

   for (i = 0; i < table->nfields; i++) {   /* for each column */

     count = row[i].count ;          /* Retrieve count from row.  Should
					be 0 if variable length null */

     /* In case this column is variable length, write out count */

     if (count == 0) count = 1;

     if ( table->header[i].count < 0 ) {
       Write_Vpf_Int ( &count, table->fp, 1 ) ;
     }

     /* Now write out the data type */

     switch (table->header[i].type) {

     case 'T':
       if ( count == 0 ) 	/* Assume this is variable length text
				   and don't do anything */
	 break ;

       /* This loop insures that the exact number of characters are written
	  out to disk. */

       output = (char *) checkmalloc ( count + 1 ) ;  /* include null byte */
       for (j = 0, tptr = (char *)row[i].ptr; j < count; j++, tptr++)
	 if ( *tptr )
	   output[j] = *tptr ;
	 else
	   output[j] = SPACE ;
       output[count] = '\0' ;
       Write_Vpf_Char( output ,table->fp, count) ;
       free ( output ) ;
       break;

     case 'I':
       Write_Vpf_Int (row[i].ptr, table->fp, count ) ;
       break;

     case 'S':
       Write_Vpf_Short (row[i].ptr, table->fp, count ) ;
       break;

     case 'F':
       Write_Vpf_Float (row[i].ptr, table->fp, count ) ;
       break;

     case 'R':
       Write_Vpf_Double (row[i].ptr, table->fp, count ) ;
       break;

     case 'D':	/* date has 21 chars in memory, not on disk */
       Write_Vpf_Date (row[i].ptr, table->fp, count ) ;
       break;

     case 'C':
       if (row[i].ptr) {
	  Write_Vpf_Coordinate(row[i].ptr,table->fp,count);
       } else {
	  for (j=0;j<count;j++)
	     Write_Vpf_Coordinate(&dummycoord,table->fp,count);
       }
       break;

     case 'B':
       Write_Vpf_DoubleCoordinate(row[i].ptr,table->fp,count);
       break;

     case 'Z':
       Write_Vpf_CoordinateZ(row[i].ptr,table->fp,count);
       break;

     case 'Y':
       Write_Vpf_DoubleCoordinateZ(row[i].ptr,table->fp,count);
       break;

     case 'K':
       keys = (id_triplet_type *) checkmalloc (count*sizeof(id_triplet_type)) ;
       memcpy ( (void *)keys, row[i].ptr, count*sizeof(id_triplet_type) ) ;
       for (j=0;j<count;j++)
	  write_key(keys[j], table->fp);
       free ( keys ) ;
       break;

     case 'X':
       /* do nothing */
       break;

     default:
       fprintf(stderr,"\nwrite_row: no such type < %c >",
	       table->header[i].type ) ;
       return(-1);
     }
   }

   return(row_size);
}

/*************************************************************************
 *
 *N  create_row
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function creates a null row for the given table.
 *     The parameter table must be a valid table and initialized prior to
 *     this function, by vpf_open_table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    table      <input> == (vpf_table_type) vpf table structure.
 *    return    <output> == (row_type) row of the table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels    Oct 1991
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
 *   None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
row_type create_row( vpf_table_type table )
{
   int i;
   row_type row;

   row = (row_type)checkmalloc(table.nfields*sizeof(column_type));
   for (i=0;i<table.nfields;i++) {
      row[i].count = table.header[i].count;
      row[i].ptr = NULL;
   }
   return row;
}



/*************************************************************************
 *
 *N  destroy_table_element
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Frees one field element - no action is taken if the
 *     field index is invalid.
 *     The parameter row must be initialized prior to this functional, either
 *     buy being read in from an existing table or set to valid values. The
 *     parameter table must be a valid table and initialized prior to this
 *     function, by vpf_open_table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     field <input>  == (int) column offset.
 *     row   <inout> == (row_type) row containing element to be removed.
 *     table <inout> == (vpf_table_type) VPF table owning row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *   RDF  7/91  original
 *   JTB  10/91 removed call to exit();
 *E
 *************************************************************************/
void destroy_table_element( int            field,
			    row_type       row,
			    vpf_table_type table )
{
   if (field < 0 || field >= table.nfields)
     return;

   if (row[field].ptr)
   {
     free(row[field].ptr);
     row[field].ptr = NULL;
     row[field].count = table.header[field].count;
   }
}


/*************************************************************************
 *
 *N  nullify_table_element
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Frees one field element - no action is taken if the
 *     field index is invalid.
 *     The parameter row must be initialized prior to this functional, either
 *     buy being read in from an existing table or set to valid values. The
 *     parameter table must be a valid table and initialized prior to this
 *     function, by vpf_open_table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     field <input>  == (int) column offset.
 *     row   <inout> == (row_type) row containing element to be removed.
 *     table <inout> == (vpf_table_type) VPF table owning row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *   RDF  7/91  original
 *   JTB  10/91 removed call to exit();
 *E
 *************************************************************************/
void nullify_table_element( int       field,
                            row_type       row,
                            vpf_table_type table )
{
   int i, count, datasize, *iptr;
   short int *sptr;
   float *fptr;
   double *rptr;
   date_type *dateptr;
   id_triplet_type kval={0,0,0,0}, *kptr;
   char *tptr;
   coordinate_type cval={NULLFLOAT,NULLFLOAT}, *cptr;
   tri_coordinate_type zval={NULLFLOAT,NULLFLOAT,NULLFLOAT}, *zptr;
   double_coordinate_type bval={NULLDOUBLE,NULLDOUBLE}, *bptr;
   double_tri_coordinate_type yval={NULLDOUBLE,NULLDOUBLE,NULLDOUBLE},
*yptr;
 
   if (field < 0 || field >= table.nfields)
     return;
 
   if (row[field].ptr)
   {
     free(row[field].ptr);
     row[field].ptr = NULL;
     row[field].count = table.header[field].count;
   }
 
   if (table.header[field].count == 0) {
      row[field].count = 0;
      return;
   } else count = table.header[field].count;
 
   switch (table.header[field].type) {
      case 'I':
         datasize = count*sizeof(int);
         row[field].ptr = (void *)malloc(datasize);
         iptr = (int *)malloc(datasize);
         for (i=0;i<count;i++)
            iptr[i] = NULLINT;
         memcpy(row[field].ptr,(void *)iptr,datasize);
         free(iptr);
         break;
      case 'S':
         datasize = count*sizeof(short int);
         row[field].ptr = (void *)malloc(datasize);
         sptr = (short int *)malloc(datasize);
         for (i=0;i<count;i++)
            sptr[i] = NULLSHORT;
         memcpy(row[field].ptr,(void *)sptr,datasize);
         free(sptr);
         break;
      case 'F':
         datasize = count*sizeof(float);
         row[field].ptr = (void *)malloc(datasize);
         fptr = (float *)malloc(datasize);
         for (i=0;i<count;i++)
            fptr[i] = NULLFLOAT;
         memcpy(row[field].ptr,(void *)fptr,datasize);
         free(fptr);
         break;
      case 'R':
         datasize = count*sizeof(double);
         row[field].ptr = (void *)malloc(datasize);
         rptr = (double *)malloc(datasize);
         for (i=0;i<count;i++)
            rptr[i] = NULLDOUBLE;
         memcpy(row[field].ptr,(void *)rptr,datasize);
         free(rptr);
         break;
      case 'D':
         datasize = count*sizeof(date_type);
         row[field].ptr = (void *)malloc(datasize);
         dateptr = (date_type *)malloc(datasize);
         for (i=0;i<count;i++)
            strcpy(dateptr[i],NULLDATE);
         memcpy(row[field].ptr,(void *)dateptr,datasize);
         free(dateptr);
         break;
      case 'T':
         datasize = count*sizeof(int);
         row[field].ptr = (void *)malloc(datasize);
         tptr = (char *)malloc(datasize);
         if (count <= 2)
            tptr[0] = '-';
         else
            tptr[0] = 'N';
         if (count == 2)
            tptr[1] = '-';
         else
            tptr[1] = '/';
         if (count >= 3) {
            tptr[2] = 'A';
            for (i=3;i<count;i++)
               tptr[i] = ' ';
         }
         memcpy(row[field].ptr,(void *)tptr,datasize);
         free(tptr);
         break;
      case 'K':
         datasize = count*sizeof(id_triplet_type);
         row[field].ptr = (void *)malloc(datasize);
         kptr = (id_triplet_type *)malloc(datasize);
         for (i=0;i<count;i++)
            kptr[i] = kval;
         memcpy(row[field].ptr,(void *)kptr,datasize);
         free(kptr);
         break;
      case 'C':
         datasize = count*sizeof(coordinate_type);
         row[field].ptr = (void *)malloc(datasize);
         cptr = (coordinate_type *)malloc(datasize);
         for (i=0;i<count;i++)
            cptr[i] = cval;
         memcpy(row[field].ptr,(void *)cptr,datasize);
         free(cptr);
         break;
      case 'B':
         datasize = count*sizeof(double_coordinate_type);
         row[field].ptr = (void *)malloc(datasize);
         bptr = (double_coordinate_type *)malloc(datasize);
         for (i=0;i<count;i++)
            bptr[i] = bval;
         memcpy(row[field].ptr,(void *)bptr,datasize);
         free(bptr);
         break;
      case 'Z':
         datasize = count*sizeof(tri_coordinate_type);
         row[field].ptr = (void *)malloc(datasize);
         zptr = (tri_coordinate_type *)malloc(datasize);
         for (i=0;i<count;i++)
            zptr[i] = zval;
         memcpy(row[field].ptr,(void *)zptr,datasize);
         free(zptr);
         break;
      case 'Y':
         datasize = count*sizeof(double_tri_coordinate_type);
         row[field].ptr = (void *)malloc(datasize);
         yptr = (double_tri_coordinate_type *)malloc(datasize);
         for (i=0;i<count;i++)
            yptr[i] = yval;
         memcpy(row[field].ptr,(void *)yptr,datasize);
         free(yptr);
         break;
   }
}



/*************************************************************************
 *
 *N  put_table_element
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Copies one element into the designated field.
 *     The parameter row must be initialized prior to this functional, either
 *     buy being read in from an existing table or set to valid values. The
 *     parameter table must be a valid table and initialized prior to this
 *     function, by vpf_open_table. Note that if stderr is used, it must
 *     be opened prior to this function.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     field <input>  == (int) column offset.
 *     row   <in-out> == (row_type) row containing target field.
 *     table <in-out> == (vpf_table_type) VPF table owning row.
 *     value <in>     == (void *) source field element.
 *     count <in>     == (int) number of items in value.
 *     put_table_element <output> == (int)
 *                                    0 --> element write succeeded
 *                                    1 --> unknown element type or
 *                                          invalid column offset
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *   RDF  7/91  original
 *   JTB  10/91 removed call to exit();
 *              guaranteed function always returns value
 *              0: element write succeeded
 *             -1: unknown element type or invalid column (field) offset
 *E
 *************************************************************************/
int put_table_element( int              field,
		       row_type         row,
		       vpf_table_type   table,
		       void           * value,
		       int         count )
{
   int i, len, stat;
   char *str;

   stat=0;

   if ((count != table.header[field].count) &&
       (table.header[field].count > 0)) {
      printf("Invalid element count! (%ld, %ld)\n",
	     count,table.header[field].count);
      return -1;
   }

   if (field < 0 || field >= table.nfields)
     return -1;

   row[field].count = count;

   if (row[field].ptr) {
      free(row[field].ptr);
      row[field].ptr = NULL;
   }

   switch ( table.header[field].type ) {
      case 'T':
	len = (int)Max(count,table.header[field].count);
	str = (char *) checkmalloc( len + 1 );
	row[field].ptr = (char *) checkmalloc ( len + 1 ) ;
	strcpy( str, (char *)value );
	for ( i = strlen((char *)value) ; i < table.header[field].count; i++ )
	   str[i] = SPACE ;
	str[len] = '\0';
	memcpy(row[field].ptr, str, len+1);
	free(str);
	break ;

      case 'D':
	row[field].ptr = (date_type *) checkmalloc (count*sizeof(date_type));
	memcpy ( row[field].ptr, value, sizeof (date_type) * count ) ;
	break;

      case 'I' :
	row[field].ptr = (int *) checkmalloc (count*sizeof(int));
	memcpy ( row[field].ptr, value, sizeof (int) * count ) ;
	break;

      case 'S' :
	row[field].ptr = (short int *) checkmalloc (count*sizeof(short int));
	memcpy ( row[field].ptr, value, sizeof (short int) * count ) ;
	break;

      case 'F':
	row[field].ptr = (float *) checkmalloc (count*sizeof(float));
	memcpy ( row[field].ptr, value, sizeof (float) * count ) ;
	break;

      case 'R':
	row[field].ptr = (double *) checkmalloc (count*sizeof(double));
	memcpy ( row[field].ptr, value, sizeof (double) * count ) ;
	break;

      case 'K':
	row[field].ptr =
	  (id_triplet_type *) checkmalloc ( count*sizeof(id_triplet_type ));
	memcpy ( row[field].ptr, value, sizeof(id_triplet_type)*count ) ;
	break;

      case 'C':
	if (value) {
	   row[field].ptr = (coordinate_type *)
	       malloc ( count * sizeof( coordinate_type ));
	   if (row[field].ptr)
	      memcpy ( row[field].ptr, value,
		       sizeof(coordinate_type)*count ) ;
	} else {
	   row[field].ptr = NULL;
	}
	break;

      case 'Z':
	if (value) {
	  row[field].ptr = (tri_coordinate_type *)
	     malloc ( count * sizeof( tri_coordinate_type ));
	  if (row[field].ptr)
	     memcpy ( row[field].ptr, value,
		      sizeof(tri_coordinate_type)*count ) ;
	} else {
	   row[field].ptr = NULL;
	}
	break;

      case 'B':
	if (value) {
	   row[field].ptr = (double_coordinate_type *)
	     malloc ( count * sizeof( double_coordinate_type ));
	   if (row[field].ptr)
	      memcpy ( row[field].ptr, value,
		       sizeof(double_coordinate_type)*count ) ;
	} else {
	   row[field].ptr = NULL;
	}
	break;

      case 'Y':
	if (value) {
	   row[field].ptr = (double_tri_coordinate_type *)
	     malloc ( count * sizeof( double_tri_coordinate_type ));
	   if (row[field].ptr)
	      memcpy( row[field].ptr, value,
		      sizeof(double_tri_coordinate_type)*count);
	} else {
	   row[field].ptr = NULL;
	}
	break;

      default:
	fprintf(stderr,"\nput_table_element: No such data type < %c > in vpf\n",
		table.header[field].type ) ;
	stat = -1;
	break ;
   }

   return stat;
}

 
int put_table_int( int field_number,
                        row_type row,
                        vpf_table_type table,
                        int value,
                        key_field_type keyfield )
{
   int count, retval=0, *lptr, i;
   short int sval, *sptr;
   float fval, *fptr;
   double rval, *rptr;
   id_triplet_type kval, *kptr;
   char *tptr;
 
   count = table.header[field_number].count;
   if (count < 1) count=1;
 
   switch (table.header[field_number].type) {
      case 'I':
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&value,1);         } else {
            lptr = (int *)checkmalloc(count*sizeof(int));
            for (i=0;i<count;i++) lptr[i] = value;
            retval = put_table_element(field_number,row,table,lptr,count);
            free(lptr);
         }
         break;
      case 'S':
         if (value==NULLINT)
            sval = NULLSHORT;
         else
            sval = (short int)value;
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&sval,1);
         } else {
            sptr = (short int *)checkmalloc(count*sizeof(short int));
            for (i=0;i<count;i++) sptr[i] = sval;
            retval = put_table_element(field_number,row,table,sptr,count);
            free(sptr);
         }
         break;
      case 'F':
         if (value==NULLINT)
            fval = NULLFLOAT;
         else
            fval = (float)value;
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&fval,1);
         } else {
            fptr = (float *)checkmalloc(count*sizeof(float));
            for (i=0;i<count;i++) fptr[i] = fval;
            retval = put_table_element(field_number,row,table,fptr,count);
            free(fptr);
         }
         break;
      case 'R':
         if (value==NULLINT)
            rval = NULLDOUBLE;
         else
            rval = (double)value;
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&rval,1);
         } else {
            rptr = (double *)checkmalloc(count*sizeof(double));
            for (i=0;i<count;i++) rptr[i] = rval;
            retval = put_table_element(field_number,row,table,rptr,count);
            free(rptr);
         }
         break;
      case 'K':
         kptr = (id_triplet_type *)get_table_element(field_number,row,table,&kval,&count);

         if (count == 1) {
           switch (keyfield) {
              case key_id:
                 kval.id = value;
                 ASSIGN_KEY( SETTYPE0, kval, id, value );
                 break;
              case key_tile:
                 kval.tile = value;
                 ASSIGN_KEY( SETTYPE1, kval, tile, value );
                 break;
              case key_exid:
                 kval.exid = value;
                 ASSIGN_KEY( SETTYPE2, kval, exid, value );
                 break;
           }
           retval = put_table_element(field_number,row,table,&kval,1);
         } else {
           for (i=0;i<count;i++) {
             switch (keyfield) {
                case key_id:
                   kptr[i].id = value;
                   ASSIGN_KEY( SETTYPE0, kptr[i], id, value );
                   break;
                case key_tile:
                   kptr[i].tile = value;
                   ASSIGN_KEY( SETTYPE1, kptr[i], tile, value );
                   break;
                case key_exid:
                   kptr[i].exid = value;
                   ASSIGN_KEY( SETTYPE2, kptr[i], exid, value );
                   break;
             } /* switch */
           } /* for */

           retval = put_table_element(field_number,row,table,kptr,count);
           free(kptr);
         } /* else */
         break;
      case 'T':
         tptr = (char *)checkmalloc(Max(count,20));
         sprintf(tptr,"%ld",value);
         retval = put_table_element(field_number,row,table,tptr,count);
         free(tptr);
         break;
      default:
         break;
   }

   return retval;
}   

  
int put_table_number( int field_number,
                           row_type row,
                           vpf_table_type table,
                           double value,
                           key_field_type keyfield )
{
   int count, lval, *lptr, i, retval=0;
   short int sval, *sptr;
   float fval, *fptr;
   double rval, *rptr;
   id_triplet_type kval, *kptr;
   char *tptr;
 
   count = table.header[field_number].count;
   if (count < 1) count=1;

   switch (table.header[field_number].type) {
      case 'I':
         if (value == NULLDOUBLE)
            lval = NULLINT;
         else
            lval = (int)value;
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&lval,1);
         } else {
            lptr = (int *)checkmalloc(count*sizeof(int));
            for (i=0;i<count;i++) lptr[i] = lval;
            retval = put_table_element(field_number,row,table,lptr,count);
            free(lptr);
         }
         break;
      case 'S':
         if (value==NULLDOUBLE)
            sval = NULLSHORT;
         else
            sval = (short int)value;
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&sval,1);
         } else {
            sptr = (short int *)checkmalloc(count*sizeof(short int));
            for (i=0;i<count;i++) sptr[i] = sval;
            retval = put_table_element(field_number,row,table,sptr,count);
            free(sptr);
         }
         break;
      case 'F':
         if (value==NULLDOUBLE)
            fval = NULLFLOAT;
         else
            fval = (float)value;
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&fval,1);
         } else {
            fptr = (float *)checkmalloc(count*sizeof(float));
            for (i=0;i<count;i++) fptr[i] = fval;
            retval = put_table_element(field_number,row,table,fptr,count);
            free(fptr);
         }
         break;
      case 'R':
         rval = value;
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&rval,1);
         } else {
            rptr = (double *)checkmalloc(count*sizeof(double));
            for (i=0;i<count;i++) rptr[i] = rval;
            retval = put_table_element(field_number,row,table,rptr,count);
            free(rptr);
         }
         break;
      case 'K':
         if (value == NULLDOUBLE)
            lval = NULLINT;
         else
            lval = (int)value;
         kval.type = kval.id = kval.tile = kval.exid = 0;
         switch (keyfield) {
            case key_id:
               kval.id = lval;
               ASSIGN_KEY( SETTYPE0, kval, id, lval );
               break;
            case key_tile:
               kval.tile = lval;;
               ASSIGN_KEY( SETTYPE1, kval, tile, lval );
               break;
            case key_exid:
               kval.exid = lval;;
               ASSIGN_KEY( SETTYPE2, kval, exid, lval );
               break;
         }
         if (table.header[field_number].count == 1) {
            retval = put_table_element(field_number,row,table,&kval,1);
         } else {
            kptr = (id_triplet_type *)checkmalloc(count*
                                      sizeof(id_triplet_type));
            for (i=0;i<count;i++) kptr[i] = kval;
            retval = put_table_element(field_number,row,table,kptr,count);
            free(kptr);
         }
         break;
      case 'T':
         tptr = (char *)checkmalloc(Max(count,20));
         sprintf(tptr,"%f",value);
         retval = put_table_element(field_number,row,table,tptr,count);
         free(tptr);
         break;
      default:
         break;
   }

   return retval;
}   
 




/*************************************************************************
 *
 *N  VpfWrite
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose: 
 *P
 *     This function writes a VPF data element to a file in VPF storage
 *     format.  It takes into account the byte order of both the host  
 *     platform and the VPF table file.  The data pointer (from), the data
 *     type, and the element count must all be correctly specified and
 *     must correspond with each other.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    from    <input> == (void *) data element pointer.
 *    type    <input> == (VpfDataType) data type.
 *    count   <input> == (int) number of data elements in the field.
 *    to      <input> == (FILE *) opened file pointer.
 *    return <output> == (int) the number of data elements written.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Tom Wood          Fall 1990
 *    David Flinn	January 1991
 *                      July 1991
 *    JTB               10/91
 *E
 *************************************************************************/
int VpfWrite ( void *from, VpfDataType type, int count, FILE *to )
{
  int retval , i ;

  switch ( type ) {
  case VpfChar:
    retval = fwrite ( from, sizeof (char), count, to ) ;
    break ;
  case VpfShort:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	short int stemp ,
		*sptr = (short *) from ;
	for ( i=0; i < count; i++, sptr++ ) {
	   swap_two ( (char *)sptr, (char *)&stemp ) ;
	   retval = fwrite ( &stemp, sizeof (short), 1, to ) ;
	}
      } else {
	retval = fwrite ( from, sizeof (short), count, to ) ;
      }
    }
    break ;
  case VpfInteger:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	 int itemp,
	   *iptr = (int *) from ;
	 for ( i=0; i < count; i++, iptr++ ) {
	   swap_four ( (char *)iptr, (char *)&itemp ) ;
	   retval = fwrite ( &itemp, sizeof (int), 1, to ) ;
	 }
      } else {
	 retval = fwrite ( from, sizeof (int), count, to ) ;
      }
    }
    break ;
  case VpfFloat:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	 float ftemp ,
	    *fptr = (float *) from ;
	 for ( i=0; i < count; i++, fptr++ ) {
	   swap_four ( (char *)fptr, (char *)&ftemp ) ;
	   retval = fwrite ( &ftemp, sizeof (float), 1, to ) ;
	 }
      } else {
	 retval = fwrite ( from, sizeof (int), count, to ) ;
      }
    }
    break ;
  case VpfDouble:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	 double dtemp ,
	     *dptr = (double *) from ;
	 for ( i=0; i < count; i++, dptr++ ) {
	   swap_eight ( (char *)dptr, (char *)&dtemp ) ;
	   retval = fwrite ( &dtemp, sizeof (double), 1, to ) ;
	 }
      } else {
	 retval = fwrite ( from, sizeof (double), count, to ) ;
      }
    }
    break ;
  case VpfDate:	/* only write out 20, not 21 chars */
    retval = fwrite ( from, sizeof ( date_type ) - 1, count, to ) ;
    break ;
  case VpfCoordinate:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	 coordinate_type ctemp ,
		      *cptr = (coordinate_type *) from ;
	 for ( i=0; i < count; i++, cptr++ ) {
	   swap_four ( (char *)&cptr->x, (char *)&ctemp.x ) ;
	   swap_four ( (char *)&cptr->y, (char *)&ctemp.y ) ;
	   retval = fwrite ( &ctemp, sizeof (coordinate_type), 1, to ) ;
	 }
      } else {
	 retval = fwrite ( from, sizeof (coordinate_type), count, to ) ;
      }
    }
    break ;
  case VpfDoubleCoordinate:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	 double_coordinate_type dctemp ,
			     *dcptr = (double_coordinate_type *) from ;
	 for ( i=0; i < count; i++, dcptr++ ) {
	   swap_eight ( (char *)&dcptr->x, (char *)&dctemp.x ) ;
	   swap_eight ( (char *)&dcptr->y, (char *)&dctemp.y ) ;
	   retval = fwrite ( &dctemp, sizeof (double_coordinate_type),
	                     1, to ) ;
	 }
      } else {
	 retval = fwrite ( from, sizeof (double_coordinate_type),
			   count, to ) ;
      }
    }
    break ;
  case VpfTriCoordinate:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	tri_coordinate_type ttemp ,
			  *tptr = (tri_coordinate_type *) from ;
	for ( i=0; i < count; i++, tptr++ ) {
	   swap_four ( (char *)&tptr->x, (char *)&ttemp.x ) ;
	   swap_four ( (char *)&tptr->y, (char *)&ttemp.y ) ;
	   swap_four ( (char *)&tptr->z, (char *)&ttemp.z ) ;
	   retval = fwrite ( &ttemp, sizeof (tri_coordinate_type), 1, to ) ;
	}
      } else {
	retval = fwrite ( from, sizeof (tri_coordinate_type), count, to ) ;
      }
    }
    break ;
  case VpfDoubleTriCoordinate:
    {
      if (MACHINE_BYTE_ORDER != STORAGE_BYTE_ORDER) {
	double_tri_coordinate_type dttemp ,
		    *dtptr = (double_tri_coordinate_type *) from ;
	for ( i=0; i < count; i++, dtptr++ ) {
	   swap_eight ( (char *)&dtptr->x, (char *)&dttemp.x ) ;
	   swap_eight ( (char *)&dtptr->y, (char *)&dttemp.y ) ;
	   swap_eight ( (char *)&dtptr->z, (char *)&dttemp.z ) ;
	   retval = fwrite ( &dttemp,sizeof (double_tri_coordinate_type),
			     1, to);
	}
      } else {
	retval = fwrite ( from,sizeof (double_tri_coordinate_type),
			  count, to);
      }
    }
    break ;
  case VpfNull:
    /* Do Nothing */
    break ;
  default:
    fprintf (stderr,"\nVpfWrite: error on data type < %d >", type ) ;
    break ;
  }

  return retval;
}


/*************************************************************************
 *
 *N  calculate_row_size
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This calculates and returns the size of a row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    row        <input> == (row_type) the row to calculate the size of.
 *    table      <input> == (vpf_table_type *) vpf table structure.
 *    return     <output> == size of the row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Kevin McCarthy    July 1992    extracted from write_row
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
 *   None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
int
calculate_row_size(row_type row, vpf_table_type *table)
{
   int recordsize, i, j, count;
   id_triplet_type *keys;

   recordsize = 0;

   for (i = 0; i < table->nfields; i++)    /* for each column */
   {
     count = row[i].count ;          /* Retrieve count from row.  Should
					be 0 if variable length null */

      if (count == 0)
         count = 1;

      if ( table->header[i].count < 0 )
         recordsize += sizeof ( int ) ;

      switch (table->header[i].type)
      {

         case 'T':
            recordsize += sizeof ( char ) * count ;
            break;
         case 'I':
            recordsize += sizeof ( int ) * count ;
            break;
         case 'S':
            recordsize += sizeof ( short int ) * count ;
            break;
         case 'F':
            recordsize += sizeof ( float ) * count ;
            break;
         case 'R':
            recordsize += sizeof ( double ) * count ;
            break;
         case 'D':	/* date has 21 chars in memory, not on disk */
            recordsize += ( sizeof ( date_type ) - 1 ) * count ;
            break;
         case 'C':
            recordsize += sizeof ( coordinate_type ) * count ;
            break;
         case 'B':
            recordsize += sizeof ( double_coordinate_type ) * count ;
            break;
         case 'Z':
            recordsize += sizeof ( tri_coordinate_type ) * count ;
            break;
         case 'Y':
            recordsize += sizeof ( double_tri_coordinate_type ) * count ;
            break;
         case 'K':
            keys = (id_triplet_type *)checkmalloc(count *
                                                  sizeof(id_triplet_type));
            memcpy((void *)keys, row[i].ptr, count * sizeof(id_triplet_type)) ;
            for (j=0; j<count; j++)
               recordsize += calculate_key_size(keys[j]);
            free(keys) ;
            break;
         case 'X':
            /* do nothing */
            break;
         default:
            fprintf(stderr,"\nwrite_row: no such type < %c >",
	            table->header[i].type ) ;
            return(-1);
      }
   }

   return(recordsize);
}


/*************************************************************************
 *
 *N  calculate_key_size
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function calculates the size of a triplet_id_key as it will
 *     be written to disk.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    key     <input> == (id_triplet_type) id triplet key.
 *    return <output> == (int) size of the key.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Kevin McCarthy   July 1993      Extracted from write_key
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
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
int
calculate_key_size(id_triplet_type key)
{
  int size;

   size = 0;

   size += sizeof(char) ;

   switch (TYPE0(key.type))
   {
      case 0:
         break;
      case 1:
         size += sizeof ( char ) ;
         break;
      case 2:
         size += sizeof ( short int ) ;
         break;
      case 3:
         size += sizeof ( int ) ;
         break;
   }

   switch (TYPE1(key.type))
   {
      case 0:
         break;
      case 1:
         size += sizeof ( char ) ;
         break;
      case 2:
         size += sizeof ( short int ) ;
         break;
      case 3:
         size += sizeof ( int ) ;
         break;
   }

   switch (TYPE2(key.type))
   {
      case 0:
         break;
      case 1:
         size += sizeof ( char ) ;
         break;
      case 2:
         size += sizeof ( short int ) ;
         break;
      case 3:
         size += sizeof ( int ) ;
         break;
   }

  return(size);
}


/*************************************************************************
 *
 *N  adjust_table_size
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Increases or decreases the size of a table from rownum + 1 onward by
 *    offset bytes.  This is used when writing a row that does not match
 *    the size of the currently stored row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    rownum <input> == (int) the row number to start adjusting at
 *    table <inout> == (vpf_table_type) the table to adjust
 *    offset <input> == (int) the offset to adjust the table by
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Kevin McCarthy    July 1993                     gcc
 *E
 *************************************************************************/
void
adjust_table_size(int rownum, vpf_table_type *table, int offset)
{
   if ( offset > 0 )
      increase_table_size(rownum, table, offset);
   else if ( offset < 0 )
      decrease_table_size(rownum, table, offset);

   adjust_index(rownum, table, offset);

   table->size += offset;
}



/*************************************************************************
 *
 *N  increase_table_size
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Moves all the rows in the table over by offset bytes, starting at
 *    row rownum + 1
 *    It works from the back of the table up and moves bytes over in the
 *    largest chunks possible.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    rownum <input> == (int) the row number to start adjusting at
 *    table <inout> == (vpf_table_type) the table to adjust
 *    offset <input> == (int) the offset to adjust the table by
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Kevin McCarthy    July 1993                     gcc
 *E
 *************************************************************************/
void
increase_table_size(int rownum, vpf_table_type *table, int offset)
{
/* removing transfersize from int decl it as an int */
   int  transfersize;
   int start, finish, currentpos, iosize;
   void *transferbuffer;

   finish = index_pos(rownum, *table) + index_length(rownum, *table); 

   fseek(table->fp, 0L, SEEK_END);
   start = ftell(table->fp);

   transfersize = start - finish;
   if ( transfersize > 0 )
   {
   /* allocate the buffer */
      transferbuffer = malloc(transfersize);
      while ( (transferbuffer == NULL) && (transfersize > 0) )
      {
         transfersize /= 2;
         transferbuffer = malloc(transfersize);
      }
      if ( transfersize == 0 )
      {
         fprintf(stderr,"Out of memory in increase_table_size\n");
         exit(1);
      }

      currentpos = start;
      while ( currentpos != finish )
      {
         currentpos -= transfersize;
   /* if the buffer amount is larger than the remaining amount to transfer,
      decrease the transfer size */
         if ( currentpos < finish )
         {
            transfersize -= (finish - currentpos);
            currentpos = finish;
         }

         fseek(table->fp, currentpos, SEEK_SET);
         iosize = (int)fread(transferbuffer, (size_t)transfersize,
                                  (size_t)1, table->fp);
         if ( iosize != 1 )
         {
            fprintf(stderr, "Read error in increase_table_size\n");
            exit(1);
         }

         fseek(table->fp, currentpos + offset, SEEK_SET);
         iosize = (int)fwrite(transferbuffer, (size_t)transfersize,
                                   (size_t)1, table->fp);
         if ( iosize != 1 )
         {
            fprintf(stderr, "Write error in increase_table_size\n");
            exit(1);
         }
      }

      free(transferbuffer);
   }
}

/*************************************************************************
 *
 *N  decrease_table_size
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Moves all the rows in the table back by offset bytes, starting at
 *    row rownum + 1. Works from the next row up moving the largest chunks
 *    of bytes over it can.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    rownum <input> == (int) the row number to start adjusting at
 *    table <inout> == (vpf_table_type) the table to adjust
 *    offset <input> == (int) the offset to adjust the table by
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Kevin McCarthy    July 1993                     gcc
 *E
 *************************************************************************/
void
decrease_table_size(int rownum, vpf_table_type *table, int offset)
{
/* removing transfersize as int and decl it as int */
  int transfersize;
   int start, finish, currentpos, iosize;
   void *transferbuffer;

   start = index_pos(rownum, *table) + index_length(rownum, *table);

   fseek(table->fp, 0L, SEEK_END);
   finish = ftell(table->fp);

   transfersize = finish - start;

   if ( transfersize > 0 )
   {
      transferbuffer = malloc(transfersize);
      while ( (transferbuffer == NULL) && (transfersize > 0) )
      {
         transfersize /= 2;
         transferbuffer = malloc(transfersize);
      }
      if ( transfersize == 0 )
      {
         fprintf(stderr, "Out of memory in decrease_table_size\n");
         exit(1);
      }

      currentpos = start;
      while ( currentpos != finish )
      {
   /* if the buffer amount is larger than the remaining amount to transfer,
      decrease the transfer size */
         if ( (currentpos + transfersize) > finish )
            transfersize = finish - currentpos;

         fseek(table->fp, currentpos, SEEK_SET);
         iosize = (int)fread(transferbuffer, (size_t)transfersize, (size_t)1,
                                  table->fp);
         if ( iosize != 1 )
         {
            fprintf(stderr,"Read error in decrease_table_size\n");
            exit(1);
         }

         fseek(table->fp, currentpos + offset, SEEK_SET);
         iosize = (int)fwrite(transferbuffer, (size_t)transfersize,
                                   (size_t)1, table->fp);
         if ( iosize != 1 )
         {
            fprintf(stderr, "Write error in decrease_table_size\n");
            exit(1);
         }

         currentpos += transfersize;
      }

      free(transferbuffer);
   }
}


/*************************************************************************
 *
 *N  adjust_index
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    After adjusting the size of the table, the table index needs to
 *    be adjusted by offset bytes.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    startrow <input> == (int) first row to adjust indexpos of
 *    table <inout> == (vpf_table_type) table index to adjust
 *    offset <input> == (int) amount to adjust index by
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Kevin McCarthy    July 1993                     gcc
 *E
 *************************************************************************/
void
adjust_index(int startrow, vpf_table_type *table, int offset)
{
   int i, recsize, pos, len;

   switch(table->xstorage)
   {
      case DISK:
         recsize = sizeof(index_cell);
 /* adjust startrow's length */
/* make int an int to run on dec */
         fseek(table->xfp, (int)(startrow * recsize + 4), SEEK_SET);
         if ( Read_Vpf_Int(&len, table->xfp, 1) == 0 )
         {
            fprintf(stderr,"Read error in adjust_index\n");
            exit(1);
         }
         len += offset;
/* make int an int to run on dec with fseek */
         fseek(table->xfp, (int)(startrow * recsize + 4), SEEK_SET);
         if ( Write_Vpf_Int(&len, table->xfp, 1) == 0 )
         {
            fprintf(stderr,"Write error in adjust_index\n");
            exit(1);
         }
         
/* adjust all following rows' position */
         for (i = startrow + 1; i <= table->nrows; i++)
         {
/* change int to int for dec use with fseek */
            fseek(table->xfp, (int)(i * recsize), SEEK_SET);
            if ( Read_Vpf_Int(&pos, table->xfp, 1) == 0 )
            {
               fprintf(stderr,"Read error in adjust_index\n");
               exit(1);
            }
            pos += offset;
/* change int to int for use with dec */
            fseek(table->xfp, (int)(i * recsize), SEEK_SET);
            if ( Write_Vpf_Int(&pos, table->xfp, 1) == 0 )
            {
               fprintf(stderr,"Write error in adjust_index\n");
               exit(1);
            }
         }
         break;
      case RAM:
         table->index[startrow - 1].length += offset;
         for (i = startrow; i < table->nrows; i++)
            table->index[i].pos += offset;
         break;
      default:
         fprintf(stderr,"Error: index not found in adjust_index\n");
         exit(1);
   }
}

