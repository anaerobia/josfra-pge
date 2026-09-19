/*************************************************************************
 *
 *N  Module VPFREAD.C
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This module contains functions for reading VPF tables.  It, along
 *     with VPFTABLE.C and VPFWRITE.C (and VPFIO.C for UNIX), comprises a
 *     fairly extensive set of functions for handling VPF tables.
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
 *    Barry Michaels   May 1991  Original Version   DOS Turbo C
 *    David Flinn      Jul 1991	 Merged with Barry & Mody's code for UNIX
 *    Jim TenBrink     Oct 1991  Split this module off from vpftable and
 *                               merged converter and vpfview branches
 *                               for the functions included here..
 *    20-July-1995	Alward Siyyid freeed malloced space in checkmalloc
 *    25-July-95	Alward Siyyid Undid earlier change to checkmalloc for ECSed01025
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.  It includes #ifdefs for
 *    all system dependencies, so that it will work efficiently with
 *    either Turbo C in DOS or (at least) GNU C in UNIX.
 *E
 *
 *************************************************************************/

#include "vpftable.h"
#include "vvmisc.h"
#include "machine.h"

#ifdef __MSDOS__
#include <mem.h>
#include <io.h>
#include <alloc.h>
#include <dos.h>
#include <graphics.h>
#else
#include <malloc.h>
#endif

#ifdef __unix__
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

#ifndef Max
#define Max(a,b)       ( (a > b) ? a : b)   /* Added to remove external */
#endif                                      /* dependency. */
#ifndef Min
#define Min(a,b)       ( (a < b) ? a : b)
#endif

extern int STORAGE_BYTE_ORDER;


/*
 * Read a line from a file.  Ignore comment lines (begining with #) and
 * allow for the line continuation character (\).  The string returned
 * by this function is dynamically allocated, and should be freed outside
 * of this function when no longer needed.
 */
static char * get_line (FILE *fp)
{
  int CurrentChar,   /* This is an int because fgetc returns an int */
      count ,
      NextBlock = 256 ,
      LineAllocation = 0 ;
  char *CurrentLine = (char *) NULL ;

/* This forever loop searches past all lines beginning with #,
   indicating comments. */

  for (;;) {
      CurrentChar = fgetc(fp);
      if ( CurrentChar == COMMENT )             /* skip past comment line */
        for (;CurrentChar != NEW_LINE; ) {
          if (CurrentChar == EOF) return (char *) NULL ;
          CurrentChar = fgetc (fp) ;
        }
      else
        break ;
    }  /* end of forever loop */

  if (CurrentChar == EOF ) return (char *) NULL ;

  for(count = 0; CurrentChar != EOF; CurrentChar = fgetc(fp), count++) {

    /* Allocate space for output line, if needed */
 
    if (! ( count < LineAllocation )) {
      LineAllocation += NextBlock ;
      if ( CurrentLine )
        CurrentLine = (char *) realloc ( CurrentLine, LineAllocation );
      else
        CurrentLine = (char *) checkmalloc ( LineAllocation );
      if (!CurrentLine) {
        fprintf(stderr,"get_line: Out of Memory");
        return (char *) NULL ;
      }
    }  
    if ( ( CurrentChar == (int) LINE_CONTINUE ) ) {
      CurrentChar = fgetc(fp ) ;        /* read character after backslash */
      /* A newline will be ignored and thus skipped over */
      if ( CurrentChar == (int) SPACE )  /* Assume line continue error */
        while ( fgetc (fp) != (int) SPACE ) ;
      else if (CurrentChar != (int) NEW_LINE ) {
        /* copy it if not new line */
        CurrentLine[count++] = (char) LINE_CONTINUE ;
        CurrentLine[count] = (char) CurrentChar ;
      } else
        count -- ;      /* Decrement the counter on a newline character */
    } else if (CurrentChar == (int) NEW_LINE )     /* We're done */
        break;
    else
      CurrentLine[count] = (char)CurrentChar;

  }  /* end of for count */

  CurrentLine[count] = (char) NULL ;  /* terminate string */
  return CurrentLine ;

}


/* Functions shared from VPFTABLE.C */
char *parse_get_string(int *ind,char *src,char delimeter);
char  parse_get_char  (int *ind,char *src);
int parse_get_number(int *ind,char *src,char delimeter);



/*************************************************************************
 *
 *N  VpfRead
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose: 
 *P
 *     This function reads an element from the current position in the 
 *     specified file (in VPF table format) into a VPF data element.
 *     The data type and count must be specified correctly.  The value
 *     pointer (to) must point to the correct data type and be allocated
 *     correctly.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    to     <output> == (void *) returned data value read from the table.
 *    type    <input> == (VpfDataType) data type of the value.
 *    count   <input> == (int) number of elements to read.
 *    from    <input> == (FILE *) file pointer to read from.
 *    return <output> == (int) the number of elements actually read.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   April 1991                        DOS Turbo C
 *E
 *************************************************************************/
int VpfRead ( void *to, VpfDataType type, int count, FILE *from )
{
  int retval , i ;

  switch ( type ) {
  case VpfChar:
    retval = fread ( to, sizeof (char), count, from ) ;
    break ;
  case VpfShort:
    {
      short int stemp ,
                *sptr = (short *) to ;
      for ( i=0; i < count; i++ ) {
	retval = fread ( &stemp, sizeof (short), 1, from ) ;
	if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER)
	   swap_two ( (char *)&stemp, (char *)sptr ) ;
	else
	   *sptr = stemp;
        sptr++ ;
      }
    }  
    break ;
  case VpfInteger:
    {
      if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER) {
	int itemp,
	  *iptr = (int *) to ;
	for ( i=0; i < count; i++ ) {
/* changed int to int for fread */
	  retval = fread ( &itemp, sizeof (int), 1, from ) ;
	  swap_four ( (char *)&itemp, (char *)iptr ) ;
	  iptr++ ;
	}
      } else {
/* changed int to int for fread */
	retval = fread ( to, sizeof (int), count, from ) ;
      }
    }  
    break ;
  case VpfFloat:
    {
      float ftemp ,
            *fptr = (float *) to ;
      for ( i=0; i < count; i++ ) {
        retval = fread ( &ftemp, sizeof (float), 1, from ) ;
	if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER)
	   swap_four ( (char *)&ftemp, (char *)fptr ) ;
	else
	   *fptr = ftemp;
	fptr++ ;
      }
    }
    break ;
  case VpfDouble:
    {
      double dtemp ,
             *dptr = (double *) to ;
      for ( i=0; i < count; i++ ) {
        retval = fread ( &dtemp, sizeof (double), 1, from ) ;
	if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER)
	   swap_eight ( (char *)&dtemp, (char *)dptr ) ;
	else
	   *dptr = dtemp;
	dptr++ ;
      }
    }
    break ;
  case VpfDate:
    {
      date_type *dp = (date_type *) to ;
      retval = fread(dp,sizeof(date_type)-1,count,from);
    }
    break ;
  case VpfCoordinate:
    {
      if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER) {
	 coordinate_type ctemp ,
		      *cptr = (coordinate_type *) to ;
	 for ( i=0; i < count; i++ ) {
	   retval = fread ( &ctemp, sizeof (coordinate_type), 1, from ) ;
	   swap_four ( (char *)&ctemp.x, (char *)&cptr->x ) ;
	   swap_four ( (char *)&ctemp.y, (char *)&cptr->y ) ;
	   cptr++ ;
	 }
      } else {
	 retval = fread ( to, sizeof (coordinate_type), count, from ) ;
      }
    }  
    break ;
  case VpfDoubleCoordinate:
    {
      double_coordinate_type dctemp ,
                             *dcptr = (double_coordinate_type *) to ;
      for ( i=0; i < count; i++ ) {
        retval = fread ( &dctemp, sizeof (double_coordinate_type), 1, from ) ;
	if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER) {
	   swap_eight ( (char *)&dctemp.x, (char *)&dcptr->x ) ;
	   swap_eight ( (char *)&dctemp.y, (char *)&dcptr->y ) ;
	} else {
	   dcptr->x = dctemp.x;
	   dcptr->y = dctemp.y;
	}
	dcptr++ ;
      }
    }
    break ;
  case VpfTriCoordinate:
    {
      tri_coordinate_type ttemp ,
                          *tptr = (tri_coordinate_type *) to ;
      for ( i=0; i < count; i++ ) {
        retval = fread ( &ttemp, sizeof (tri_coordinate_type), 1, from ) ;
	if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER) {
	   swap_four ( (char *)&ttemp.x, (char *)&tptr->x ) ;
	   swap_four ( (char *)&ttemp.y, (char *)&tptr->y ) ;
	   swap_four ( (char *)&ttemp.z, (char *)&tptr->z ) ;
	} else {
	   tptr->x = ttemp.x;
	   tptr->y = ttemp.y;
	   tptr->z = ttemp.z;
	}
	tptr++ ;
      }
    }
    break ;
  case VpfDoubleTriCoordinate:
    {
      double_tri_coordinate_type dttemp ,
                                 *dtptr = (double_tri_coordinate_type *) to ;
      for ( i=0; i < count; i++ ) {
        retval = fread ( &dttemp,sizeof (double_tri_coordinate_type), 1, from);
	if (STORAGE_BYTE_ORDER != MACHINE_BYTE_ORDER) {
	   swap_eight ( (char *)&dttemp.x, (char *)&dtptr->x ) ;
	   swap_eight ( (char *)&dttemp.y, (char *)&dtptr->y ) ;
	   swap_eight ( (char *)&dttemp.z, (char *)&dtptr->z ) ;
	} else {
	   dtptr->x = dttemp.x;
	   dtptr->y = dttemp.y;
	   dtptr->z = dttemp.z;
	}
	dtptr++ ;
      }
    }  
    break ;
  case VpfNull:
    /* Do Nothing */
    break ;
  default:
    fprintf (stderr,"\nVpfRead: error on data type < %d >", type ) ;
    break ;
  }   /* end of switch */

  return retval ;       /* whatever fread returns */

}




/*************************************************************************
 *
 *N  read_text_defstr
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function reads the definition string from an input VPF table 
 *     file for creating a copy of a VPF table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    infile  <input> == (FILE *) file pointer to the input file.
 *    outerr  <input> == (FILE *) file pointer to the error file.
 *    return <output> == (char *) definition string of the file.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Dave Flinn       July 1991      Original version developed for UNIX
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
 *    get_line()
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
char *read_text_defstr ( FILE *infile, FILE *outerr )
{
  char *definition ;
  FILE *out ;

  if ( outerr == NULL )
    out = stderr ;     /* Use global value error file */
  else
    out = outerr ;

  rewind ( infile );

  if (( (definition = get_line (infile )) == NULL ) ||
      ( definition[strlen(definition)-1] != ';' )) {
    fprintf(out,"\nread_text_defstr: No definition string in input file\n");
    return (char *) NULL ;
  }

  return definition ;

}

#ifdef __unix__
/*
 * currently being used by converter routines, not by vpfview
 * routines
 */
int add_null_values ( char *name, vpf_table_type table, FILE *fpout )
{
  FILE *nullfp , *out ;
  int  i , ptr ;
  char *cval, *line, *field ;
 
  if ( fpout == NULL )
    out = stderr ;             /* use global error file */
  else
    out = fpout ;

  nullfp = fopen( name, "r");
  if ( !nullfp )
    return NULL ;
 
  /* Now read nulls from file and populate table structure */

  while ( line = get_line ( nullfp ) ) {
 
    ptr = 0 ;
    field = parse_get_string ( &ptr, line, FIELD_SEPERATOR ) ;
    i = table_pos ( field, table ) ;
 
    switch ( table.header[i].type ) {
    case 'T':
      cval = parse_get_string ( &ptr, line, FIELD_SEPERATOR ) ;
      free ( table.header[i].nullval.Char ) ;   /* get rid of default */
      table.header[i].nullval.Char = (char *) checkmalloc ( strlen (cval)) ;
      strcpy ( table.header[i].nullval.Char, cval ) ;
      free (cval) ;
      break ;
    case 'I':
      table.header[i].nullval.Int = parse_get_number ( &ptr, line,
                                                       FIELD_SEPERATOR );
      break ;
    case 'S':
      table.header[i].nullval.Short =
        (short int) parse_get_number ( &ptr, line, FIELD_SEPERATOR );
      break ;
    case 'R':
      cval = parse_get_string ( &ptr, line, FIELD_SEPERATOR ) ;
      table.header[i].nullval.Double = atof ( cval ) ;
      free ( cval ) ;
      break ;
    case 'F':
      cval = parse_get_string ( &ptr, line, FIELD_SEPERATOR ) ;
      table.header[i].nullval.Float = (float) atof ( cval ) ;
      free ( cval ) ;
      break ;
    default:
      fprintf(out, "\nadd_null_values: column definition < %c > not supported",
              table.header[i].type ) ;
      return NULL ;
      break ;
    }
  }

  return 1 ;
}
#endif

/*************************************************************************
 *
 *N  index_length
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the length of a specified row from the table
 *     index.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    row_number <input> == (int) row number in the table.
 *    table      <input> == (vpf_table_type) VPF table structure.
 *    return    <output> == (int) length of the table row or 0 on error.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991       Original Version   DOS Turbo C
 *    Dave Flinn       July 1991      UNIX extensions
 *    JTB              10/91          removed aborts()
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
int index_length( int row_number,
		       vpf_table_type table )
{
   int   recsize,len;
   unsigned int ulen;
   int pos;

   STORAGE_BYTE_ORDER = table.byte_order;

   if (row_number < 1) row_number = 1;
   if (row_number > table.nrows) row_number = table.nrows;

   switch (table.xstorage) {
      case COMPUTE:
	 len = table.reclen;
	 break;
      case DISK:
	 recsize = sizeof(index_cell);
/* changing int to int for fseek */
	 fseek( table.xfp, (int)(row_number*recsize), SEEK_SET );

	 if ( ! Read_Vpf_Int(&pos,table.xfp,1) ) {
	   fprintf (stderr,"\nindex_length: error reading index.") ;
	   len = (int)NULL ;
	 }

	 if ( ! Read_Vpf_Int(&ulen,table.xfp,1) ) {
	   fprintf (stderr,"\nindex_length: error reading index.") ;
	   return (int) NULL ;
	 }
	 len = ulen;
	 break;
      case RAM:
	 len = table.index[row_number-1].length;
	 break;
      default:
	if ( table.mode == Write && table.nrows != row_number ) {
	   /* Just an error check, should never get here in writing */
	   fprintf(stderr,"\nindex_length: error trying to access row %d",
		   row_number ) ;
	   len = (int)NULL ;
	}
	break;
   }
   return len;
}

/*************************************************************************
 *
 *N  index_pos
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the position of a specified row from the table
 *     index.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    row_number <input> == (int) row number in the table.
 *    table      <input> == (vpf_table_type) VPF table structure.
 *    return    <output> == (int) position of the table row 
 *                          or zero on error.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991       Original Version   DOS Turbo C
 *    Dave Flinn       July 1991      Updated for UNIX
 *    JTB              10/91          removed aborts()
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
int index_pos( int row_number,
		    vpf_table_type table )
{
   int   recsize;
   unsigned int pos;

   STORAGE_BYTE_ORDER = table.byte_order;

   if (row_number < 1) row_number = 1;
   if (row_number > table.nrows) row_number = table.nrows;

   switch (table.xstorage) {
      case COMPUTE:
	 pos = table.ddlen + ((row_number-1) * table.reclen);
	 break;
      case DISK:
	 recsize = sizeof(index_cell);
/* changed int to int for fseek on dec */
	 fseek( table.xfp, (int)(row_number*recsize), SEEK_SET );
	 if ( ! Read_Vpf_Int(&pos,table.xfp,1) ) {
	   fprintf (stderr,"\nindex_pos: error reading index.") ;
	   pos = (int)NULL ;
	 }
	 break;
      case RAM:
	 pos = table.index[row_number-1].pos;
	 break;
      default:
	 if ( table.mode == Write && table.nrows != row_number ) {
	   /* Just an error check, should never get here in writing */
	   fprintf(stderr,"\nindex_pos: error trying to access row %d",
		   row_number ) ;
	   pos = (int)NULL;
	 }
	 break;
   }
   return pos;
}


/*************************************************************************
 *
 *N  row_offset
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose: 
 *P
 *     Compute the offset from the start of the row to the given field.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    field   <input> == (int) field number.
 *    row     <input> == (row_type) table row structure.
 *    table   <input> == (vpf_table_type) VPF table.
 *    return <output> == (int) the byte offset from the start of the
 *                       the given row to the specified field.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   April 1991                        DOS Turbo C
 *E
 *************************************************************************/
int row_offset( int field, row_type row, vpf_table_type table)
{
   int offset,n,size;
   int i;
   id_triplet_type key;
   int keysize[] = {0,sizeof(char),sizeof(short int),sizeof(int)};

   if (field < 0 || field >= table.nfields) return -1;

   offset = 0L;
   for (i=0;i<field;i++) {
      switch (table.header[i].type) {
	 case 'I':
	    offset += sizeof(int)*row[i].count;
	    break;
	 case 'S':
	    offset += sizeof(short int)*row[i].count;
	    break;
	 case 'T':
	    offset += sizeof(char)*row[i].count;
	    break;
	 case 'F':
	    offset += sizeof(float)*row[i].count;
	    break;
	 case 'D':
	    offset += sizeof(date_type)*row[i].count;
	    break;
	 case 'K':
	    get_table_element(i,row,table,&key,&n);
	    size = sizeof(char) +
		   keysize[TYPE0(key.type)] +
		   keysize[TYPE1(key.type)] +
		   keysize[TYPE2(key.type)];
	    offset += size*row[i].count;
	    break;
	 case 'R':
	    offset += sizeof(double)*row[i].count;
	    break;
	 case 'C':
	    offset += sizeof(coordinate_type)*row[i].count;
	    break;
	 case 'B':
	    offset += sizeof(double_coordinate_type)*row[i].count;
	    break;
	 case 'Z':
	    offset += sizeof(tri_coordinate_type)*row[i].count;
	    break;
	 case 'Y':
	    offset += sizeof(double_tri_coordinate_type)*row[i].count;
	    break;
      }
   }
   return offset;
}

/*************************************************************************
 *
 *N  read_key
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function reads an id triplet key from a VPF table.
 *     The table must be open for read.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    table    <input>  == (vpf_table_type) VPF table.
 *    read_key <output> == (id_triplet_type) id triplet key.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991       Original Version   DOS Turbo C
 *    Dave Flinn       July 1991      Updated for UNIX
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
id_triplet_type read_key( vpf_table_type table )
{
   id_triplet_type key;
   unsigned char ucval;
   unsigned short int uival;

   STORAGE_BYTE_ORDER = table.byte_order;

   key.id = 0L;
   key.tile = 0L;
   key.exid = 0L;

   /* just doing this to be consistent */
   Read_Vpf_Char (&(key.type),table.fp,1);

   switch (TYPE0(key.type)) {
      case 0:
	 break;
      case 1:

	 Read_Vpf_Char (&ucval, table.fp, 1 ) ;
	 key.id = (int)ucval;
	 break;
      case 2:

	 Read_Vpf_Short (&uival, table.fp, 1 ) ;
	 key.id = (int)uival;
	 break;
      case 3:

	 Read_Vpf_Int (&(key.id), table.fp, 1 ) ;
	 break;
   }
   switch (TYPE1(key.type)) {
   case 0:
     break;
   case 1:
     Read_Vpf_Char (&ucval, table.fp, 1 ) ;
     key.tile = (int)ucval;
     break;
   case 2:
     Read_Vpf_Short (&uival, table.fp, 1 ) ;
     key.tile = (int)uival;
     break;
   case 3:
     Read_Vpf_Int (&(key.tile), table.fp, 1 ) ;
     break;
   }

   switch (TYPE2(key.type)) {
   case 0:
     break;
   case 1:
     Read_Vpf_Char (&ucval, table.fp, 1 ) ;
     key.exid = (int)ucval;
     break;
   case 2:
     Read_Vpf_Short (&uival, table.fp, 1 ) ;
     key.exid = (int)uival;
     break;
   case 3:
     Read_Vpf_Int (&(key.exid), table.fp, 1 ) ;
     break;
   }

   return key;
 }

/*************************************************************************
 *
 *N  read_next_row
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function reads the next row of the table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    table      <input> == (vpf_table_type) vpf table structure.
 *    return    <output> == (row_type) the next row in the table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991       Original Version   DOS Turbo C
 *    Dave Flinn       July 1991      Updated for UNIX
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
 *    void *checkmalloc()
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
row_type read_next_row( vpf_table_type table )
{
   register int i,j;
   int      status;
   char     *tptr;
   int size,count;
   row_type row;
   id_triplet_type * keys;
   coordinate_type dummycoord;

   if (feof(table.fp)) {
      return NULL;
   }

   STORAGE_BYTE_ORDER = table.byte_order;

   row = (row_type)checkmalloc((table.nfields+1) * sizeof(column_type));

   for (i=0;i<table.nfields;i++) row[i].ptr = NULL;

   for (i=0;i<table.nfields;i++) {
      if (table.header[i].count < 0) {

	 Read_Vpf_Int (&count,table.fp,1) ;

      } else {
	 count = table.header[i].count;
      }
      row[i].count = count;
      if (!row[i].count) continue;

      status = 0;
      switch (table.header[i].type) {
	 case 'T':
	    if (count == 1) {
	       row[i].ptr = (char *)checkmalloc(sizeof(char));
	       Read_Vpf_Char(row[i].ptr, table.fp, 1) ;
	    } else {
	       size = count*sizeof(char);
	       row[i].ptr = (char *)checkmalloc(size+2);
	       tptr = (char *)checkmalloc(size+2);
	       Read_Vpf_Char(tptr,table.fp,count) ;
	       tptr[count] = '\0';
               for (j=0;j<count;j++)
                  if (tptr[j] == (char)127) tptr[j] = '\0';
	       strcpy((char *)row[i].ptr,tptr);
	       free(tptr);
	    }
	    break;
	 case 'I':
	    row[i].ptr = (int *)checkmalloc(count*sizeof(int));
	    Read_Vpf_Int (row[i].ptr, table.fp, count ) ;
	    break;
	 case 'S':
	    row[i].ptr = (short int *)checkmalloc(count*sizeof(short int));
	    Read_Vpf_Short (row[i].ptr, table.fp, count ) ;
	    break;
	 case 'F':
	    row[i].ptr = (float *)checkmalloc(count*sizeof(float));
	    Read_Vpf_Float (row[i].ptr, table.fp, count ) ;
	    break;
	 case 'R':
	    row[i].ptr = (double *)checkmalloc(count*sizeof(double));
	    Read_Vpf_Double (row[i].ptr, table.fp, count ) ;
	    break;
	 case 'D':
	    row[i].ptr = (date_type *)checkmalloc(count*sizeof(date_type));
	    Read_Vpf_Date (row[i].ptr, table.fp, count ) ;
	    break;
	 case 'C':
	    /* Coordinate strings may be quite large.          */
	    /* Allow for null coordinate string pointer if     */
	    /* not enough memory that can be handled one       */
	    /* coordinate at a time in higher level functions. */
            /* Generally a DOS problem.                        */
	    row[i].ptr = (coordinate_type *)malloc(count*
			 sizeof(coordinate_type));
	    if (row[i].ptr)
	       Read_Vpf_Coordinate(row[i].ptr,table.fp,count);
	    else
	       for (j=0;j<count;j++)
		  Read_Vpf_Coordinate(&dummycoord,table.fp,1);
	    break;
	 case 'Z':
	    row[i].ptr = (tri_coordinate_type *)checkmalloc(count*
			 sizeof(tri_coordinate_type));
	    Read_Vpf_CoordinateZ(row[i].ptr,table.fp,count);
	    break;
	 case 'B':
	    row[i].ptr = (double_coordinate_type *)checkmalloc(count*
			 sizeof(double_coordinate_type));
	    Read_Vpf_DoubleCoordinate(row[i].ptr,table.fp,count);
	    break;
	 case 'Y':
	    row[i].ptr = (double_tri_coordinate_type *)checkmalloc(count*
			 sizeof(double_tri_coordinate_type));
	    Read_Vpf_DoubleCoordinateZ(row[i].ptr,table.fp,count);
	    break;
	 case 'K':   /* ID Triplet */
	    row[i].ptr = (id_triplet_type *)checkmalloc(count*
			 sizeof(id_triplet_type));
	    keys = (id_triplet_type *)checkmalloc(count*
		   sizeof(id_triplet_type));
	    for (j=0;j<count;j++) {
	       keys[j] = read_key(table);
	    }
	    memcpy(row[i].ptr,keys,count*sizeof(id_triplet_type));
	    free(keys);
	    break;
	 case 'X':
	    row[i].ptr = NULL;
	    break;
	  default:
	    fprintf(stderr,"\n%s%s >>> read_next_row: no such type < %c >",
		table.path,table.name,table.header[i].type ) ;
	    status = 1;
	    break ;
	  }   /* end of switch */
      if (status == 1) {
	 free_row ( row, table ) ;
	 return (row_type) NULL;
      }
   }
   return row;
}

/*************************************************************************
 *
 *N  rowcpy
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns a copy of the specified row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    origrow    <input> == (row_type) row to copy.
 *    table      <input> == (vpf_table_type) vpf table structure.
 *    return    <output> == (row_type) copy of the row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
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
 *    void *checkmalloc()                            VPFMISC.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
row_type rowcpy( row_type origrow,
		 vpf_table_type table )
{
   register int i;
   int      count;
   int size;
   row_type row;

   row = (row_type)checkmalloc(table.nfields * sizeof(column_type));
   for (i=0;i<table.nfields;i++) {
      count = origrow[i].count;
      row[i].count = count;
      switch (table.header[i].type) {
	 case 'T':
	    if (count==1) {
	       row[i].ptr = (char *)checkmalloc(1);
	       memcpy(row[i].ptr,origrow[i].ptr,sizeof(char));
	    } else {
	       size = count*sizeof(char);
	       row[i].ptr = (char *)checkmalloc(size+1);
	       strcpy((char *)row[i].ptr,(char *)origrow[i].ptr);
	    }
	    break;
	 case 'I':
	    size = count*sizeof(int);
	    row[i].ptr = (int *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'S':
	    size = count*sizeof(short int);
	    row[i].ptr = (short int *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'F':
	    size = count*sizeof(float);
	    row[i].ptr = (float *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'R':
	    size = count*sizeof(double);
	    row[i].ptr = (double *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'C':
	    size = count*sizeof(coordinate_type);
	    row[i].ptr = (coordinate_type *)malloc(size);
	    if (row[i].ptr && origrow[i].ptr)
	       memcpy(row[i].ptr,origrow[i].ptr,size);
	    else
	       row[i].ptr = NULL;
	    break;
	 case 'Z':
	    size = count*sizeof(tri_coordinate_type);
	    row[i].ptr = (tri_coordinate_type *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'B':
	    size = count*sizeof(double_coordinate_type);
	    row[i].ptr = (double_coordinate_type *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'Y':
	    size = count*sizeof(double_tri_coordinate_type);
	    row[i].ptr = (double_tri_coordinate_type *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'D':  /* Date */
	    size = count*sizeof(date_type);
	    row[i].ptr = (date_type *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'K':  /* ID Triplet */
	    size = count*sizeof(id_triplet_type);
	    row[i].ptr = (id_triplet_type *)checkmalloc(size);
	    memcpy(row[i].ptr,origrow[i].ptr,size);
	    break;
	 case 'X':
	    row[i].ptr = NULL;
	    break;
	  default:
	    fprintf (stderr,"\nrow_cpy: error in data type < %c >",
		     table.header[i].type ) ;
	    abort () ;
	    break ;
	  }   	/* end of switch */
    }    	/* end of table.nfields */
   return row;
}

/*************************************************************************
 *
 *N  read_row
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function reads a specified row from the table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    row_number <input> == (int) row number.
 *    table      <input> == (vpf_table_type) vpf table structure.
 *    return    <output> == (row_type) row that was read in.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991      DOS Turbo C
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
 *    int index_pos()                             VPFTABLE.C
 *    row_type read_next_row()                         VPFTABLE.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
row_type read_row( int row_number,
		   vpf_table_type table )
{
/* changing int to int for fpos due to dec */
   int fpos;

   fpos = index_pos(row_number,table);

   fseek(table.fp,fpos,SEEK_SET);

   return read_next_row(table);
}

/*************************************************************************
 *
 *N  free_row
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function frees the memory that was dynamically allocated for the
 *     specified row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    row   <input> == (row_type) row to be freed.
 *    table <input> == (vpf_table_type) vpf table structure.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
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
void free_row( row_type row,
	       vpf_table_type table)
{
   register int i;

   if (!row) return;
   for (i=0;i<table.nfields;i++)
      if (row[i].ptr) free(row[i].ptr);
   free(row);
}

/*************************************************************************
 *
 *N  get_row
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the specified row of the table.  If the table
 *     is stored in memory, the row is copied from there.  If it is on disk,
 *     it is read and returned.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    row_number <input> == (int) row number in range [1 .. table.nrows].
 *    table      <input> == (vpf_table_type) vpf table structure.
 *    return    <output> == (row_type) returned row.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991      DOS Turbo C
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
 *    row_type rowcpy                VPFREAD.C
 *    row_type read_row              VPFREAD.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
row_type get_row( int row_number,
		  vpf_table_type table )
{
   row_type row;

   row_number = Max(Min(row_number, table.nrows), 1);

   if (table.storage == RAM) {
      row = rowcpy(table.row[row_number-1],table);
      return row;
   } else {
      return read_row( row_number, table );
   }
}

/*************************************************************************
 *
 *N  table_pos
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the column offset of the specified field name
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    field_name <input> == (char *) field name.
 *    table      <input> == (vpf_table_type) VPF table structure.
 *    table_pos  <output> == (int) returned column number.
 *                          UNIX returns -1 if not exists
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
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
int table_pos( char           * field_name,
		    vpf_table_type   table )
{
   register int i;
   int col;

   col = -1;
   for (i = 0; i < table.nfields; i++) {
      if (stricmp(field_name,table.header[i].name) == 0) {
	 col = i;
	 break;
      }
   }
   return col;
}

/*************************************************************************
 *
 *N  get_table_element
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the element in the given row in the given
 *     column.  If the element is a single element (count=1), the value
 *     is passed back via the void pointer *value; otherwise, an array
 *     is allocated and passed back as the return value.
 *     NOTE: If an array is allocated in this function, it should be freed
 *     when no longer needed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    field_number <input> == (int) field column number.
 *    row          <input> == (row_type) vpf table row.
 *    table        <input> == (vpf_table_type) VPF table structure.
 *    value       <output> == (void *) pointer to a single element value.
 *    count       <output> == (int *) pointer to the array size for a
 *                                    multiple element value.
 *    return      <output> == (void *) returned multiple element value.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991             DOS Turbo C
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
 *    void *checkmalloc()
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
void *get_table_element( int field_number,
			 row_type row,
			 vpf_table_type table,
			 void *value,
			 int  *count )
{
   int   col;
   char     * tptr;
   void     * retvalue;

   retvalue = NULL;
   col = field_number;
   if ((col < 0) || (col >= table.nfields)) {
      fprintf(stderr,"%s: Invalid field number %d\n",
	      table.name,field_number);
      return NULL;
   }

   if (!row) return NULL;

   switch (table.header[col].type) {
      case 'X':
	 retvalue = NULL;
	 break;
      case 'T':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(char));
	 } else {
	    retvalue = (char *)checkmalloc((row[col].count+1)*sizeof(char));
	    tptr = (char *)checkmalloc((row[col].count+1)*sizeof(char));
	    memcpy(tptr,row[col].ptr,row[col].count*sizeof(char));
	    tptr[row[col].count] = '\0';
	    strcpy((char *)retvalue,tptr);
	    free(tptr);
	 }
	 break;
      case 'I':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(int));
	 } else {
	    retvalue = (int *)checkmalloc(row[col].count*
					     sizeof(int));
	    memcpy(retvalue,row[col].ptr,row[col].count*sizeof(int));
	 }
	 break;
      case 'S':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(short int));
	 } else {
	    retvalue = (short int *)checkmalloc(row[col].count*
					     sizeof(short int));
	    memcpy(retvalue,row[col].ptr,row[col].count*sizeof(short int));
	 }
	 break;
      case 'F':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(float));
	 } else {
	    retvalue = (float *)checkmalloc(row[col].count*sizeof(float));
	    memcpy(retvalue,row[col].ptr,row[col].count*sizeof(float));
	 }
	 break;
      case 'R':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(double));
	 } else {
	    retvalue = (double *)checkmalloc(row[col].count*sizeof(double));
	    memcpy(retvalue,row[col].ptr,row[col].count*sizeof(double));
	 }
	 break;
      case 'C':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(coordinate_type));
	 } else {
	    if (row[col].ptr) {
	       retvalue = (coordinate_type *)malloc(row[col].count*
			  sizeof(coordinate_type));
	       if (retvalue)
		  memcpy(retvalue,row[col].ptr,row[col].count*
			 sizeof(coordinate_type));
	    } else {
	       retvalue = NULL;
	    }
	 }
	 break;
      case 'Z':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(tri_coordinate_type));
	 } else {
	    retvalue = (tri_coordinate_type *)checkmalloc(row[col].count*
			sizeof(tri_coordinate_type));
	    memcpy(retvalue,row[col].ptr,row[col].count*
		    sizeof(tri_coordinate_type));
	 }
	 break;
      case 'B':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(double_coordinate_type));
	 } else {
	    retvalue = (double_coordinate_type *)checkmalloc(row[col].count*
			sizeof(double_coordinate_type));
	    memcpy(retvalue,row[col].ptr,row[col].count*
		    sizeof(double_coordinate_type));
	 }
	 break;
      case 'Y':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(double_tri_coordinate_type));
	 } else {
	    retvalue = (double_tri_coordinate_type *)checkmalloc(row[col].count*
			sizeof(double_tri_coordinate_type));
	    memcpy(retvalue,row[col].ptr,row[col].count*
		    sizeof(double_tri_coordinate_type));
	 }
	 break ;
      case 'D':
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(date_type));
	 } else {
	    retvalue = (date_type *)checkmalloc(row[col].count*
					      sizeof(date_type));
	    memcpy(retvalue,row[col].ptr,row[col].count*
					  sizeof(date_type));
	 }
	 break;
      case 'K':  /* ID Triplet */
	 if (table.header[col].count == 1) {
	    memcpy(value,row[col].ptr,sizeof(id_triplet_type));
	 } else {
	    retvalue = (id_triplet_type *)checkmalloc(row[col].count*
					      sizeof(id_triplet_type));
	    memcpy(retvalue,row[col].ptr,row[col].count*
					  sizeof(id_triplet_type));
	 }
	 break;
   }
   *count = row[col].count;

   return retvalue;
}

/*************************************************************************
 *
 *N  named_table_element
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the element in the specified row in the column
 *     matching the given field name.  If the element is a single element
 *     (count=1), the value is passed back via the void pointer *value;
 *     otherwise, an array is allocated and passed back as the return value.
 *     NOTE: If an array is allocated in this function, it should be freed
 *     when no longer needed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    field_name <input> == (char *) field name.
 *    row        <input> == (row_type) row from which element is to be 
 *                          retrieved.
 *    table      <input> == (vpf_table_type) VPF table structure.
 *    value     <output> == (void *) pointer to a single element value.
 *    count     <output> == (int *) pointer to the array size for a 
 *                          multiple element value.
 *    return    <output> == (void *) returned multiple element value.
 *                          or NULL if field_name could not be found
 *                          as a column
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
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
 *    void *checkmalloc()                    VPFREAD.C
 *    row_type get_row()                   VPFREAD.C
 *    void free_row()                      VPFREAD.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
void *named_table_element( char *field_name, row_type row,
			   vpf_table_type table, void *value,
			   int *count )
{
   int     col;
   void       * retvalue;

   col = table_pos(field_name, table);

   if (col < 0) {
      fprintf(stderr,"%s: Invalid field name <%s>\n",table.name,field_name);
      return NULL;
   }

   retvalue = get_table_element( col, row, table, value, count );

   return retvalue;
}


/*************************************************************************
 *
 *N  table_element
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function returns the element in the specified row in the column
 *     matching the given field number.  If the element is a single element
 *     (count=1), the value is passed back via the void pointer *value;
 *     otherwise, an array is allocated and passed back as the return value.
 *     NOTE: If an array is allocated in this function, it should be freed
 *     when no longer needed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    field_number <input> == (int) field number (offset from
 *                                   first field in table).
 *    row_number <input> == (int) row_number.
 *    table      <input> == (vpf_table_type) VPF table structure.
 *    value     <output> == (void *) pointer to a single element value.
 *    count     <output> == (int *) pointer to the array size for a multiple
 *                                  element value.
 *    return    <output> == (void *) returned multiple element value or
 *                          NULL of the field number is invalid
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
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
 *    row_type get_row()                   VPFREAD.C
 *    void free_row()                      VPFREAD.C
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
void *table_element( int         field_number,
		     int         row_number,
		     vpf_table_type   table,
		     void           * value,
		     int       * count )
{
   row_type    row;
   void      * retvalue;

   row      = get_row(row_number, table);
   retvalue = get_table_element(field_number, row, table, value, count);
   free_row(row,table);

   return retvalue;
}


int get_table_int( int field_number,
                        row_type row,
                        vpf_table_type table,
                        key_field_type keyfield )
{
   int count, lval=NULLINT, *lptr;
   short int sval, *sptr;
   float fval, *fptr;
   double rval, *rptr;
   id_triplet_type kval, *kptr;
   char tval, *tptr;

   switch (table.header[field_number].type) {
      case 'I':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&lval,&count);
         } else {
            lptr = (int *)get_table_element(field_number,row,table,
                                                 NULL,&count);
            lval = lptr[0];
            free(lptr);
         }
         break;
      case 'S':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&sval,&count);
            lval = (int)sval;
         } else {
            sptr = (short int *)get_table_element(field_number,row,table,
                                                  NULL,&count);
            lval = (int)sptr[0]; 
            free(sptr); 
         }
         if (lval == NULLSHORT) lval = NULLINT;
         break;
      case 'F':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&fval,&count);
         } else {
            fptr = (float *)get_table_element(field_number,row,table,
                                                 NULL,&count);
            fval = (int)fptr[0]; 
            free(fptr); 
         }
         if (fval == NULLFLOAT)
            lval = NULLINT;
         else
            lval = (int)fval;
         break;
      case 'R':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&rval,&count);
         } else {
            rptr = (double *)get_table_element(field_number,row,table,
                                                 NULL,&count);
            rval = (int)rptr[0]; 
            free(rptr); 
         }
         if (rval == NULLDOUBLE)
            lval = NULLINT;
         else
            lval = (int)rval;
         break;
      case 'K':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&kval,&count);
         } else {
            kptr = (id_triplet_type *)get_table_element(field_number,row,
                                                        table,NULL,&count);
            kval = kptr[0]; 
            free(kptr); 
         }
         switch (keyfield) {
            case key_id:
               lval = (int)kval.id;
               break;
            case key_tile:
               lval = (int)kval.tile;
               break;
            case key_exid:
               lval = (int)kval.exid;
               break;
            default:
               lval = NULLINT;
               break;
         }
         break;
      case 'T':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&tval,&count);
            tptr = (char *)checkmalloc(3);
            tptr[0] = tval;
            tptr[1] = (char)0;
         } else {
            tptr = (char *)get_table_element(field_number,row,table,
                                             NULL,&count);
         }
         lval = atoi(tptr);
         free(tptr); 
         break;
      default:
         lval = NULLINT;
         break;
   }

   return lval;
}




double get_table_number( int field_number,
                         row_type row,
                         vpf_table_type table,
                         key_field_type keyfield )
{
   int count, lval, *lptr;
   short int sval, *sptr;
   float fval, *fptr;   
   double rval=NULLDOUBLE, *rptr;
   id_triplet_type kval, *kptr;
   char tval, *tptr;
 
   switch (table.header[field_number].type) {
      case 'I':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&lval,&count);
         } else {
            lptr = (int *)get_table_element(field_number,row,table,
                                                 NULL,&count);
            lval = lptr[0];   
            free(lptr);   
         }
         if (lval == NULLINT)
            rval = NULLDOUBLE;
         else
            rval = (double)lval;
         break;
      case 'S':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&sval,&count);
         } else {
            sptr = (short int *)get_table_element(field_number,row,table,
                                                  NULL,&count);
            sval = sptr[0];
            free(sptr); 
         }
         if (sval == NULLSHORT)
            rval = NULLSHORT;
         else
            rval = (double)sval;
         break;
      case 'F':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&fval,&count);
         } else {
            fptr = (float *)get_table_element(field_number,row,table,
                                                 NULL,&count);
            fval = (int)fptr[0];
            free(fptr); 
         }
         if (fval == NULLFLOAT)
            rval = NULLDOUBLE;   
         else
            rval = (double)fval;
         break;
      case 'R':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&rval,&count);
         } else {
            rptr = (double *)get_table_element(field_number,row,table,
                                                 NULL,&count);
            rval = (int)rptr[0];
            free(rptr);
         }
         break;
      case 'K':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&kval,&count);
         } else {
            kptr = (id_triplet_type *)get_table_element(field_number,row,
                                                        table,NULL,&count);
            kval = kptr[0];
            free(kptr);
         }
         switch (keyfield) {
            case key_id:
               rval = (double)kval.id;
               break;
            case key_tile:
               rval = (double)kval.tile;
               break;
            case key_exid:
               rval = (double)kval.exid;
               break;
            default:
               rval = NULLDOUBLE;
               break;
         }
         break;
      case 'T':
         if (table.header[field_number].count == 1) {
            get_table_element(field_number,row,table,&tval,&count);
            tptr = (char *)checkmalloc(3);
            tptr[0] = tval;
            tptr[1] = (char)0;
         } else {
            tptr = (char *)get_table_element(field_number,row,table,
                                             NULL,&count);
         }
         rval = atof(tptr);
         free(tptr);
         break;
      default:
         rval = NULLDOUBLE;
         break;
   }
 
   return rval;
}
 



