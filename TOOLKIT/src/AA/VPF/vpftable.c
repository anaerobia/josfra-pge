/*************************************************************************
 *
 * Environmental Systems Research Institute (ESRI) Applications Programming
 *
 *N  Module VPFTABLE.C
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This module contains functions to open, close and parse VPF relational
 *     tables.  VPF tables are defined as being a set of rows and columns.
 *     Each column may contain a single value, a fixed array of values,
 *     or a variable number of values.
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
 *    Barry Michaels   April 1991  - DOS Turbo C.
 *    Mody Buchbinder  May 1991    - Modified parse_data_def for new table
 *                                   header structure.
 *    Dave Flinn       July 1991   - Updated file for UNIX.
 *    JTB              10/91       - split off read routines into vpfread.c
 *                                   merged view and converter branches of
 *                                   this module; replaced various aborts()
 *                                   and exits with return codes
 *    Kevin McCarthy   July 1993   - modified vpf_open_table and vpf_close_table
 *                                   for Update file_mode and variable length
 *                                   write_row
 *
 *   Simon Tackley - 1/25/95 - switched order of sys/stat & sys/types includes & added (char *) to lines 927 & 1368

 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    (function) displayerror(char *text[], int nlines) - function
 *       specified by the application to display a message to the user.
 *       It should return an integer indicating whether to retry (1) or
 *       not (0).
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Functions Called:
 *F
 *    N/A
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *    Updated to port to UNIX platforms with big-endian architectures
 *E
 *************************************************************************/

#ifdef __MSDOS__
#include <mem.h>
#include <io.h>
#include <alloc.h>
#include <dos.h>
#include <graphics.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <memory.h>
#define   farcoreleft()	" "
#endif


#if ( defined(CYGWIN) )
#include <cygwin_misc.h>
#endif


#ifdef _PGS_THREADSAFE
#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "vpftable.h"
#include "vvmisc.h"

#ifndef __64K
#define __64K 65536
#endif

/* Need to prototype these functions here because of */
/* the redefinition of rightjust().		     */
char *strupr( char *str );
static char *rightjust( char *str );

/*
 * per heap block byte overhead
 */
#define HEAP_OVERHEAD 4

/* Global variable shared by VPFTABLE.C, VPFREAD.C, and VPFWRITE.C  */
/* The above modules are logically one, only separated by file size */
/* constraints. */
int STORAGE_BYTE_ORDER = LEAST_SIGNIFICANT;  /* default */


/* Include statics to decrease external module dependencies */

/* Right justify a character string */
static char *rightjust( char *str )
{
   register int  len,i;

   len = strlen(str);
   i = len - 1;
   while ((i>0) && ((str[i]==0) || (str[i]==' '))) i--;
   if (i < (len-1)) str[i+1] = '\0';
   for (i=0;i<(int)strlen(str);i++) if (str[i]=='\n') str[i] = '\0';
   return str;
}



/* Return floating point Not a Number (for NULL values) */
static double quiet_notan( int unused )
{
   int nanstr[8] = {-1,-1,-1,-1,-1,-1,-1,127};
   double n;
   memcpy((void *)&n,(void *)&nanstr[0],sizeof(n));
   if (unused) return n;
   return n;
}


/***********Mody B**********/
/* get string until delimeter */
static char *cpy_del(char *src, char delimiter, int *ind )
{
  int i, skipchar ;
  char *temp, *tempstr ;

  /* remove all blanks ahead of good data */

  skipchar = 0 ;
  while ( src[skipchar] == SPACE || src[skipchar] == TAB )
    skipchar++ ;

  temp = &src[skipchar];

  /* If the first character is a COMMENT, goto LINE_CONTINUE */

  if ( *temp == COMMENT ) {
    while ( *temp != LINE_CONTINUE && *temp != END_OF_FIELD && *temp != '\0'){
      temp++ ;
      skipchar ++ ;
    }
    skipchar++ ;
    temp++ ;		/* skip past LC, EOF, or NULL */
  }

  /* Start with temporary string value */

  tempstr = (char *)checkmalloc ( strlen ( temp ) + 10 ) ;

  if ( *temp == '"' ) {	/* If field is quoted, do no error checks */

    temp++ ; 	  /* skip past quote character */
    skipchar++ ;  /* update the position pointer */

    for ( i=0 ; *temp != '\0'; temp++,i++) {
      if ( *temp == LINE_CONTINUE || *temp == TAB ) {
	temp++ ;
	skipchar++ ;
      } else if ( *temp == '"' )
	break ;
      /* Now copy the char into the output string */
      tempstr[i] = *temp ;
    }
    tempstr[i] = (char) NULL ;		/* terminate string */
    *ind += ( i + skipchar + 2) ;	/* Increment position locate past */
    return tempstr ;			/* quote and semicolon */
  }

  /* search for delimiter to end, or end of string */

  i=0 ;	/* initialize */

  if ( *temp != END_OF_FIELD ) {	/* backward compatability check */

    for ( i=0; *temp != '\0';temp++,i++){/* Stop on NULL*/

      if ( ( *temp == LINE_CONTINUE && *(temp+1) == '\n') ||  *temp == TAB ) {
	temp++ ;
	skipchar++ ;
      } else if ( *temp == delimiter )
	break ;					/* break for delimiter  */
      /* Now copy the char into the output string */
      tempstr[i] = *temp ;
    }
                             /* Eat the delimiter from ind also */
    *ind += ( i + skipchar + 1) ;	/* Increment position locate */
  }	
  tempstr[i] = (char) NULL ;		/* terminate string */   
  return tempstr;
}

/* The next 3 functions are local to VPFTABLE and VPFREAD */
/***********Mody B*********/
/* parse the next string token from the input string */
char *parse_get_string(int *ind,char *src,char delimeter )
{ char *temp;
  temp  = cpy_del(&src[*ind],delimeter, ind);
  if( ! strcmp ( temp, TEXT_NULL ))
    strcpy ( temp, "" ) ;
  return temp;
}
/**********Mody B*************/
/* parse the next character from the input string */
char parse_get_char(int *ind, char *src)
{  char temp;
   while ( src[*ind] == SPACE || src[*ind] == TAB ) (*ind)++ ;
   temp  = src[*ind];
   *ind += 2;
   return temp;
}
/***********Mody B***********/
/* parse the next numeric token from the input string */
int parse_get_number(int *ind, char *src,char delimeter)
{  char *temp;
   int  num;
   temp  = cpy_del(&src[*ind],delimeter, ind);
   if (strchr(temp, VARIABLE_COUNT ) == NULL)
      num = atoi(temp);
   else
      num = -1;
   free(temp);
   return num;
}


/*************************************************************************
 *
 *N  swap_two 
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function performs a byte swap for a two-byte numeric field.
 *     This may be necessary if the data is stored in the opposite
 *     order of significance than the host platform.  Both parameters
 *     should point to a two-byte data element.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    in    <input> == (char *) pointer to the input value.
 *    out  <output> == (char *) pointer to the returned swapped value.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   History:
 *H
 *    Barry Michaels   April 1991                        DOS Turbo C
 *E
 *************************************************************************/
void swap_two ( char *in, char *out )
{
  out[0] = in[1] ;
  out[1] = in[0] ;
}

/*************************************************************************
 *
 *N  swap_four
 * 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *
 *   Purpose: 
 *P 
 *     This function performs a byte swap for a four-byte numeric field. 
 *     This may be necessary if the data is stored in the opposite
 *     order of significance than the host platform.  Both parameters
 *     should point to a four-byte data element. 
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   Parameters: 
 *A 
 *    in    <input> == (char *) pointer to the input value.
 *    out  <output> == (char *) pointer to the returned swapped value. 
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   History: 
 *H 
 *    Barry Michaels   April 1991                        DOS Turbo C 
 *E 
 *************************************************************************/
void swap_four ( char *in, char *out )
{
  out[0] = in[3] ;
  out[1] = in[2] ;
  out[2] = in[1] ;
  out[3] = in[0] ;
}

/*************************************************************************
 *
 *N  swap_eight
 * 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *
 *   Purpose: 
 *P 
 *     This function performs a byte swap for an eight-byte numeric field. 
 *     This may be necessary if the data is stored in the opposite
 *     order of significance than the host platform.  Both parameters
 *     should point to an eight-byte data element. 
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   Parameters: 
 *A 
 *    in    <input> == (char *) pointer to the input value.
 *    out  <output> == (char *) pointer to the returned swapped value. 
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   History: 
 *H 
 *    Barry Michaels   April 1991                        DOS Turbo C 
 *E 
 *************************************************************************/
void swap_eight ( char *in, char *out )
{
  out[0] = in[7] ;
  out[1] = in[6] ;
  out[2] = in[5] ;
  out[3] = in[4] ;
  out[4] = in[3] ;
  out[5] = in[2] ;
  out[6] = in[1] ;
  out[7] = in[0] ;
}


/*************************************************************************
 *
 *N  parse_data_def
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function parses a table's data definition and creates a header
 *     in memory that is associated with the table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    table <inout> == (vpf_table_type *) vpf table structure.
 *    ddlen <input> == (int) length of the table's data definition.
 *
 *    return value is the record length if all items are fixed length, or
 *    -1 if the record contains variable length items
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   April 1991                        DOS Turbo C
 *    Mody Buchbinder  May 1991 - Modified for new table header.
 *    Dave Flinn       July 1991 - updated for UNIX
 *    Kevin McCarthy   July 1993 - added code to handle Update table->mode type 
 *E
 *************************************************************************/
int parse_data_def( vpf_table_type *table )
{
   register int n,i;
   int p, k;
   char *buf,*des,*nar,*vdt, *tdx, *doc, byte ;/*temporary storage */
   char end_of_rec;
   int status;
    int ddlen;
   int reclen = 0;

   if ( (table->mode == Read) || (table->mode == Update) ) {
     fread(&ddlen,sizeof(ddlen),1,table->fp);

     /* Check the next byte to see if the byte order is specified */
     fread(&byte,1,1,table->fp);
     p=0;
     table->byte_order = LEAST_SIGNIFICANT; /* default */
     switch (toupper(byte)) {
	case 'L':
	   p++;
	   break;
	case 'M':
	   table->byte_order = MOST_SIGNIFICANT;
	   p++;
	   break;
     }
     if (MACHINE_BYTE_ORDER != table->byte_order) {
	k = ddlen;
	swap_four((char *)&k,(char *)&ddlen);
     }
     STORAGE_BYTE_ORDER = table->byte_order;

     if ( ddlen < 0 ) {
       fprintf (stderr,"\nparse_data_def: Bad VPF file.\n");
       return (int) NULL ;
     }

     /* header without first 4 bytes */
/* change sizeof int to int for use with dec */
     table->ddlen = ddlen + sizeof (int) ;
     buf = (char *)checkmalloc((ddlen+3)*sizeof(char));
     buf[0] = byte; /* already have the first byte of the buffer */
     Read_Vpf_Char(&buf[1],table->fp,ddlen-1) ;
   } else {
     table->ddlen = strlen ( table->defstr ) ;
     ddlen = table->ddlen ;
     buf = (char *)checkmalloc((ddlen+3)*sizeof(char));
     strncpy ( buf, table->defstr, ddlen ) ;
     p=0;
     table->byte_order = LEAST_SIGNIFICANT; /* default */
     byte = buf[0];
     switch (toupper(byte)) {
	case 'L':
	   p++;
	   break;
	case 'M':
	   table->byte_order = MOST_SIGNIFICANT;
	   p++;
	   break;
     }
     STORAGE_BYTE_ORDER = table->byte_order;
   }

   buf[ddlen-1] = '\0'; /* mark end of string for reading functions */
   if ( buf[p] == ';' )
     p++; /* buf[p] is semi-colon */
   des = parse_get_string(&p,buf,COMPONENT_SEPERATOR );
   strncpy(table->description,des,80);
   free(des);
   nar = parse_get_string(&p,buf,COMPONENT_SEPERATOR );
   strncpy(table->narrative ,nar,12);
   free(nar);
   n = 0 ;
   /* get number of fields */
   for (i=p; i < ddlen;i++)
     if ( buf[i] == LINE_CONTINUE )
       i++ ;	/* skip past line continue, and next character */
     else if (buf[i] == END_OF_FIELD ) 		/* Found end of field */
	n++;					/* increment nfields */

#if 0
     else if (buf[i] == COMMENT )		/* skip past comments */
       while ( buf[i] != LINE_CONTINUE &&
	       buf[i] != END_OF_FIELD &&
	       buf[i] != '\0')
	 i++ ;					/* increment i */
#endif

   table->nfields = n ;
   table->header = (header_type)checkmalloc((n+1)*sizeof(header_cell));

   for(i=0;i<n;i++) {
     end_of_rec = FALSE;
     table->header[i].name  = parse_get_string(&p,buf, FIELD_COUNT); /***/
     rightjust(table->header[i].name);
     strupr(table->header[i].name);
     table->header[i].type  = toupper(parse_get_char  (&p,buf));
     table->header[i].count = parse_get_number(&p,buf,FIELD_SEPERATOR );

#if 0
     if ( i == 0 )
       if ( strcmp ( table->header[0].name, "ID" ) ) {
	 fprintf (stderr,"\nparse_data_def: No 'ID' in header definition.");
	 fprintf (stderr,"\n\t\tPlease fix input text header file.\n");
	 return NULL ;
       }
#endif

     if(table->header[i].count == -1)
       reclen = -1;			/* set reclen to variable len flag */

     /* Now set null values and add up record length, if fixed length */

     status = 0;

     switch (table->header[i].type) {
     case 'I':
       if ( reclen >= 0 )
/* changed sizeof int to int for use on DEC */
	 reclen += (sizeof(int)*table->header[i].count);
       table->header[i].nullval.Int = NULLINT ;
       break;
     case 'S':
       if ( reclen >= 0 )
	 reclen += (sizeof(short int)*table->header[i].count);
       table->header[i].nullval.Short = NULLSHORT ;
       break;
     case 'F':
       if ( reclen >= 0 )
	 reclen += (sizeof(float)*table->header[i].count);
       table->header[i].nullval.Float = NULLFLOAT ;
       break;
     case 'R':
       if ( reclen >= 0 )
	 reclen += (sizeof(double)*table->header[i].count);
       table->header[i].nullval.Double = NULLDOUBLE ;
       break;
     case 'T':
       if ( reclen >= 0 ) { 		/* if fixed length */
	 reclen += (sizeof(char)*table->header[i].count);
	 table->header[i].nullval.Char =
	   (char *) checkmalloc ( table->header[i].count + 1 ) ;
	 for ( k=0; k < table->header[i].count; k++ )
	   table->header[i].nullval.Char[k] = NULLCHAR ;
	 table->header[i].nullval.Char[k] = (char) NULL ;
       } else {			/* variable length */
	 table->header[i].nullval.Char =
	   (char *) checkmalloc ( VARIABLE_STRING_NULL_LENGTH + 1 ) ;
	 for ( k=0; k < VARIABLE_STRING_NULL_LENGTH ; k++ )
	   table->header[i].nullval.Char[k] = NULLCHAR ;
	 table->header[i].nullval.Char[k] = (char) NULL ;
       }
       break;
     case 'C':
       if ( reclen >= 0 )
	 reclen += (sizeof(coordinate_type)*table->header[i].count);
       table->header[i].nullval.Other = (char) NULL ;
       break;
     case 'Z':
       if ( reclen >= 0 )
	 reclen += (sizeof(tri_coordinate_type)*table->header[i].count);
       table->header[i].nullval.Other = (char) NULL ;
       break;
     case 'B':
       if ( reclen >= 0 )
	 reclen += (sizeof(double_coordinate_type)*table->header[i].count);
       table->header[i].nullval.Other = (char) NULL ;
       break;
     case 'Y':
       if ( reclen >= 0 )
	 reclen +=
	   (sizeof(double_tri_coordinate_type)*table->header[i].count);
       table->header[i].nullval.Other =(char) NULL;
       break;
     case 'D':
       if ( reclen >= 0 )
	 reclen += ((sizeof(date_type)-1)*table->header[i].count);
       strcpy ( table->header[i].nullval.Date, NULLDATE ) ;
       break;
     case 'K':
       reclen = -1;
       table->header[i].nullval.Other = (char) NULL ;
       break;
     case 'X':
       /* do nothing */
       table->header[i].nullval.Other = (char) NULL ;
       break ;
     default:
       fprintf(stderr,"\nparse_data_def: no such type < %c >",
	       table->header[i].type ) ;
       status = 1;
       break ;
     } /** switch type **/

     if (status) return (int)NULL;   

     table->header[i].keytype     = parse_get_char  (&p,buf);
     des = parse_get_string(&p,buf, FIELD_SEPERATOR );
     rightjust(des);
     strncpy(table->header[i].description,des,80);
     free(des);
     vdt = parse_get_string(&p,buf, FIELD_SEPERATOR );
     strncpy(table->header[i].vdt,vdt,12);
     free(vdt);

     table->header[i].tdx = (char *)NULL;
     table->header[i].narrative = (char *)NULL;

     tdx = parse_get_string(&p,buf, FIELD_SEPERATOR ) ;
     if ( ! strcmp ( tdx, "" ) ) {
       table->header[i].tdx = (char *) NULL ;
     } else {
       if (strcmp(tdx,"-") != 0) {
	  table->header[i].tdx = (char *)checkmalloc ( strlen ( tdx )+1 ) ;
	  strcpy (table->header[i].tdx, tdx );
       } else table->header[i].tdx = (char *)NULL;
     }
     free(tdx);
     if (buf[p+1]==';') end_of_rec = TRUE;
     if (!end_of_rec) {
	doc = parse_get_string(&p,buf, FIELD_SEPERATOR ) ;
	if ( ! strcmp ( doc, "" ) ) {
	  table->header[i].narrative = (char *) NULL ;
	  end_of_rec = TRUE;
	} else {
	  if (strcmp(doc,"-") != 0) {
	     table->header[i].narrative = (char *)checkmalloc ( strlen(doc)+1 ) ;
	     strcpy (table->header[i].narrative, doc );
	  } else table->header[i].narrative = (char *)NULL;
	}
	free(doc);
     } else table->header[i].narrative = (char *)NULL;
     p += 1; /** eat semicolon **/
    }
   free(buf);
   return reclen;
}


/**************************************************************************
 *
 *N  vpf_nullify_table
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Nullify the given VPF table structure.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *E
 *************************************************************************/
void vpf_nullify_table( vpf_table_type *table )
{

   if (!table) return;

   strcpy(table->name,"");
   table->path = NULL;
   table->nfields = 0;
   strcpy(table->description,"");
   strcpy(table->narrative,"");
   table->header = NULL;
   table->xfp = NULL;
   table->index = NULL;
   table->xstorage = (storage_type) 0;
   table->fp = NULL;
   table->nrows = 0;
   table->row = NULL;
   table->reclen = 0;
   table->ddlen = 0;
   table->defstr = NULL;
   table->storage = (storage_type) 0;
   table->mode = (file_mode) 0;
   table->byte_order = LEAST_SIGNIFICANT;
   table->status = CLOSED;
}


/*************************************************************************
 *
 *N  vpfhandler
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function temporarily takes the place of the DOS hard error
 *     handler.  It is needed when 'vpfopencheck' replaces the DOS error
 *     handler for disk operations, so that the disk errors may be handled
 *     more suitably for our environment.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels    May 1991                       DOS Turbo C
 *    Dave Flinn        July 1991  Updated for UNIX, basically commented
 *                                 the subroutine out
 *E
 *************************************************************************/
#define IGNORE 0
#define RETRY  1
#define ABORT  2
#define DISPLAY_STRING 0x09
static int vpfhandler(int errval, int ax, int bp, int si)
{
#ifdef __MSDOS__
   if (ax < 0) {
      bdosptr(DISPLAY_STRING,"device error$", 0);
      hardretn(-1);
   }

   si++; bp++;  /* get rid of compiler warning */
#endif
   return(IGNORE);
}


/*************************************************************************
 *
 *N  vpfopencheck
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function tries its darndest to open a file.  It initially calls
 *     fopen with the given filename and mode.  If that doesn't work and
 *     the file is not on the hard disk, it displays a message asking the
 *     user to enter the correct disk in the drive, waits for either a retry
 *     or a cancel response, and, if told to retry, tries again.  This
 *     process is repeated until either the file is opened or the user
 *     requests cancel.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    filename <input> == (char *) full path name of the file to be opened.
 *    mode     <input> == (char *) mode of the file.
 *    diskname <input> == (char *) descriptive name of the disk the file is
 *                                 on.
 *    return  <output> == (FILE *) file pointer newly associated with
 *                                 filename.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels    May 1991                       DOS Turbo C
 *    Dave Flinn        July 1991     Updated for UNIX
 *E
 *************************************************************************/
static FILE *vpfopencheck( char *filename,
                           char *mode,
                           char *diskname )
{
   FILE *fp;
#ifdef __MSDOS__
   char *text[] = {"Please insert",
		   "                                        ",
		   "in data drive",
		   "                                                      "};
#endif
   int retry;
   int len;
   char *copy;
#ifdef __MSDOS__
   extern char home[255];
   void interrupt (*doshandler)();

   doshandler = getvect(36);
   harderr(vpfhandler);

   strncpy(text[1],diskname,strlen(text[1]));
   strcpy(text[3],filename);
#endif
   len = strlen(filename);
   copy = (char *)checkmalloc(sizeof(char)*(len+2));
   strcpy(copy,filename);
   copy[len] = '.';
   copy[len+1] = '\0';
   fp = NULL;
   while (fp == NULL) {
      fp = fopen(filename,mode);
      if ((fp == NULL) && ((fp = fopen(copy,mode)) == NULL)){
#ifdef __MSDOS__
	 if ( toupper(home[0]) != toupper(filename[0]) )
	    retry = displayerror(text,   4);
	 else
#endif
	    retry = FALSE;
	 if (!retry) break;
      }
   }
#ifdef __MSDOS__
   setvect(36,doshandler);
#endif
   free(copy);
   return fp;
}


#ifndef __MSDOS__
int filelength( int fd )
{
   struct stat statbuf ;
 
   if ( fstat ( fd, &statbuf ) < 0 )
        return 0;
   return statbuf.st_size ;
}
#endif


/*************************************************************************
 *
 *N  vpf_open_table
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function opens a vpf table and either loads it into RAM or sets
 *     up the structure to read off of disk.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    tablename <input> == (char *) file name of the table.  As stated
 *                                  in the VPF standard, the table name
 *                                  must not end in 'x'.
 *    storage   <input> == (storage_type) table storage mode -
 *                                  MUST be ram, disk, or either.
 *    mode      <input> == (char *) file mode for opening the table -
 *                                  MUST be the same as fopen() mode in C.
 *    defstr    <input> == (char *) table definition string used for
 *                                  creating a writable table.
 *                                  If write mode this MUST be a valid
 *                                  VPF table definition string.
 *    vpf_open_table <output> == (vpf_table_type) VPF table structure.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   April 1991                   DOS Turbo C
 *    Dave Flinn       July 1991                    UNIX compatable
 *    Kevin McCarthy   July 1993                  - Added Update file mode
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
 *     Write_Vpf_Int (macro in vpfio.h)
 *     Write_Vpf_Char (macro in vpfio.h)
 *     Read_Vpf_Char (macro in vpfio.h)
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Portability:
 *O
 *    This module should be ANSI C compatible.
 *E
 *************************************************************************/
vpf_table_type vpf_open_table( char         * tablename,
			       storage_type   storage ,
			       char         * mode,
			       char         * defstr )
{
   vpf_table_type   table;
   char             tablepath[255],
		  * idxname;
   int         i,
		    j;
   int         idxsize,
		    memsize;
   unsigned int ulval;
   char            * diskname = "VPF data disc";
   int modelength;

   strcpy(tablepath,tablename);
   rightjust(tablepath);

   /* Parse out name and path */
   j = -1;
   i=strlen(tablepath);
   while (i>0) {
#ifdef __MSDOS__
      if (tablepath[i] == '\\') {
#else
      if (tablepath[i] == '/') {
#endif
	 j = i;
	 break;
      }
      i--;
   }
   strncpy(table.name,&(tablepath[j+1]),12);
   table.name[12] = '\0';
   rightjust(table.name);
   table.path = (char *)checkmalloc((strlen(tablepath)+5)*sizeof(char));
   strcpy(table.path, tablepath);
   table.path[j+1] = '\0';

 /* Establish a read or write table operation */

   modelength = strlen(mode);
   if ( mode[0] == 'r' ) {
      table.mode = Read;
      if ( modelength > 1 )
         if (mode[1] == '+') table.mode = Update;
   } else {
     table.mode = Write ;
   }

   table.fp = vpfopencheck(tablepath,mode,diskname);
   if (table.fp == NULL) {
      fprintf(stderr, "\nvpf_open_table: error opening <%s>\n",tablepath);
      free(table.path);
      return table;
   }

   /* If file is to be created, copy the def string ptr into header for now */

   if ( table.mode == Write )
     table.defstr = (char *)strdup(defstr);

   table.size = (filelength(fileno(table.fp)));

   /* Populate table structure with correct data, either for read or write */

   table.reclen = parse_data_def(&table);

   if ( table.mode == Write ) {   /* write out header */
     rewind ( table.fp ) ;
     Write_Vpf_Int ( &table.ddlen, table.fp, 1 ) ;
     table.size += sizeof(int);
     Write_Vpf_Char ( table.defstr, table.fp, table.ddlen ) ;
     table.size += table.ddlen;
     free ( table.defstr ) ;
     table.defstr = (char *) NULL ;
     table.nrows = 0 ;
   }


   if (table.reclen > 0) {      /* Index file */
      table.xstorage = (storage_type) COMPUTE;
      if (table.mode != Write)
	 table.nrows = (table.size - table.ddlen)/table.reclen;
      table.xfp = (FILE *) NULL ;
   } else {
      table.xstorage = (storage_type) DISK;
      idxname = (char *)checkmalloc( sizeof(char) * (strlen(tablepath)+2) );
      strcpy(idxname,tablepath);
      /* MAKING CHANGES HERE 5/7/92 FOR UNIX CD-ROM CONSIDERATIONS */
      if(idxname[strlen(tablepath)-1] == '.')
	idxname[strlen(tablepath)-2] = 'x';
      else
	idxname[strlen(tablepath)-1] = 'x';
      table.xfp = fopen(idxname, mode);

      if ((!table.xfp) && 
         ((table.mode == Update) || (table.mode == Read)) ) {
	strcat(idxname,".");
	table.xfp = fopen(idxname, mode);
      }

      if ((!table.xfp) && 
         ((table.mode == Update) || (table.mode == Read)) ) {
	 perror(idxname);
         fprintf(stderr, "hit RETURN to continue...");
	 i=getc(stdin);
	 free(idxname);
	 for (i = 0; i < table.nfields; i++)
	   free(table.header[i].name);
	 free(table.header);
	 free(table.path);
         fclose(table.fp);
         table.fp = NULL;
         return table;
      }

      free(idxname);

#ifdef __MSDOS__
      table.xstorage = DISK;   /* Worst case default */
#endif

   /* Only read in index if file is read only */

      if (table.xfp && ( table.mode == Read ) )
      {
         Read_Vpf_Int (&(table.nrows), table.xfp, 1 ) ;
         Read_Vpf_Int (&ulval, table.xfp, 1 ) ;
         idxsize = table.nrows*sizeof(index_cell) + 10L;

#ifdef __MSDOS__
	 if ( (idxsize < (farcoreleft()/2)) && (idxsize < __64K) )
#endif
	 {
	    table.xstorage = (storage_type) RAM;
	    table.index = (index_type)checkmalloc(idxsize);
	    for (i=0;i<table.nrows;i++)
            {
	      Read_Vpf_Int (&(table.index[i].pos), table.xfp, 1) ;
	      Read_Vpf_Int (&(table.index[i].length),table.xfp,1 ) ;
	    }
	    fclose(table.xfp);
         }
      }
      else if (table.mode == Write)
      {

     /* Write out dummy header record for index file. vpf_close_table finishes
	the job. */

	 Write_Vpf_Int ( &(table.ddlen), table.xfp, 1 ) ;
	 Write_Vpf_Int ( &(table.ddlen), table.xfp, 1 ) ;
	 table.xstorage = (storage_type) DISK;
	 table.index = (index_type) NULL ;

      }
      else   /* table.mode == Update */
      {
         table.xstorage = (storage_type) DISK;
         Read_Vpf_Int(&(table.nrows), table.xfp, 1);
         table.index = (index_type) NULL;
      }
   }  /* end of if table .reclen */

    table.storage = (storage_type) DISK;
#ifdef __MSDOS__
   memsize = (int)Min(farcoreleft(),__64K);
#else
   memsize = MAXINT;
#endif

   if ( (storage != disk) && ( table.mode == Read ) ) {
      if (table.size + table.nrows * table.nfields * HEAP_OVERHEAD < memsize) {
	 fseek(table.fp,index_pos(1,table),SEEK_SET);
	 table.row = (row_type *)checkmalloc((table.nrows+1)*sizeof(row_type));
	 for (i=0;i<table.nrows;i++) {
	    table.row[i] = read_next_row(table);
	 }
	 fclose(table.fp);
	 table.storage = (storage_type) RAM;
      }
   }
   table.status = OPENED;
   return table;
 }

/**************************************************************************
 *
 *N  vpf_close_table
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function frees an entire table from memory.
 *     The table must have been previously opened with vpf_open_table().
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    table       <inout> == (vpf_table_type) VPF table structure.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
 *    Kevin McCarthy   July 1993   - Added Update file mode, file truncation
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
void vpf_close_table( vpf_table_type *table )
{
   register int i;
   int tablesize;

   if (!table) return;

   if (table->status != OPENED) {
      return;
   }

   /* If the table is writable, write out the final record count */

   if ( ((table->mode == Update) || (table->mode == Write))
         && table->xfp ) {
     rewind ( table->xfp ) ;
     Write_Vpf_Int ( &table->nrows, table->xfp, 1 ) ;
     Write_Vpf_Int ( &table->ddlen, table->xfp, 1 ) ;
   }

   for (i=0;i<table->nfields;i++) {
      free(table->header[i].name);
      /* free up null text string */
      if ( table->header[i].type == 'T')
	 free(table->header[i].nullval.Char);
      /* free up index file string */
      if (table->header[i].tdx!=(char *)NULL)
	free ( table->header[i].tdx ) ;
      /* free up narrative table string */
      if (table->header[i].narrative!=(char *)NULL) {
	free ( table->header[i].narrative ) ;
      }
   }
   free(table->header);

   fflush(table->fp);

   if (table->mode != Read) {

      tablesize = (filelength(fileno(table->fp)));
#ifdef __MSDOS__
      /* NEED TO PUT IN A DOS TRUNCATE ROUTINE */
#else
      {
          if ( tablesize > table->size )
          {
             ftruncate(fileno(table->fp), table->size);
          }
          else if ( tablesize < table->size )
          {
  /* all additions to the table should automatically register in filelength, so
     table->tablesize must not have been incremented properly for an
     addition */
             fprintf(stderr,"Error: incorrect table size count\n");
          }
      }
#endif

   }

   switch (table->storage) {
      case RAM:
	 for (i=0;i<table->nrows;i++) free_row(table->row[i],*table);
	 free(table->row);
	 break;
      case DISK:
	 fclose(table->fp);
	 break;
      default:
	 printf("%s%s: unknown storage flag: %d\n",table->path,table->name,
		table->storage);
	 break;
   }

   switch (table->xstorage) {
      case RAM:
	 free(table->index);
	 break;
      case DISK:
	 fclose(table->xfp);
	 break;
      case COMPUTE:
	 break;
      default:
	 printf("%s%s: unknown index storage flag: %d\n",
		table->path,table->name,table->storage);
	 break;
   }
   table->nfields = 0;
   free(table->path);
   table->status = CLOSED;
}


/*************************************************************************
 *
 *N  is_vpf_table 
 * 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *
 *   Purpose: 
 *P 
 *     This function determines if the file at the specified location
 *     is a valid VPF table.  It returns TRUE or FALSE.
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   Parameters: 
 *A 
 *    fname   <input> == (char *) file system path.
 *    return <output> == (int) TRUE (1) if the file is a VPF table;
 *                             FALSE (0) if not.
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   History: 
 *H 
 *    Barry Michaels   April 1991                        DOS Turbo C 
 *    Kevin McCarthy   July 1993     - Set STORAGE_BYTE_ORDER so that
 *                                     Read_Vpf_Int works properly
 *E 
 *************************************************************************/
int is_vpf_table( char *fname )
{
   FILE *fp;
   int n, ok;
/* chaning int to int to help fread on ether */
   int ddlen;
   char byte;

   fp = fopen( fname, "rb" );
   if (!fp) {
      return FALSE;
   }
   fread(&ddlen, sizeof(ddlen), 1, fp);
   fread(&byte, 1, 1, fp);
   switch(toupper(byte))
   {
      case 'L':
         STORAGE_BYTE_ORDER = LEAST_SIGNIFICANT;
         break;
      case 'M':
         STORAGE_BYTE_ORDER = MOST_SIGNIFICANT;
         break;
      default:
         STORAGE_BYTE_ORDER = LEAST_SIGNIFICANT;
         break;
   }
   fseek( fp, 0L, SEEK_SET);
   
   Read_Vpf_Int ( &n, fp, 1 ) ;
   fseek( fp, n-1, SEEK_CUR );
   if (fgetc(fp) == ';')
      ok = TRUE;
   else
      ok = FALSE;
   fclose(fp);
   return ok;
}


/*************************************************************************
 *
 *N  is_vpf_null_float
 * 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *
 *   Purpose: 
 *P 
 *     This function determines whether a floating point number is the  
 *     same as the VPF representation of the floating point NULL value.
 *     It returns TRUE or FALSE.
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   Parameters: 
 *A 
 *    num     <input> == (float) number to evaluate.
 *    return <output> == (int) TRUE (1) if the number is VPF NULL;
 *                             FALSE (0) if not.
 *E 
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  *  
 *   History: 
 *H 
 *    Barry Michaels   April 1991                        DOS Turbo C 
 *E 
 *************************************************************************/
int is_vpf_null_float( float num )
{
   float nan;

   nan = (float)quiet_notan(0);
   if (memcmp((void *)&nan,(void *)&num,sizeof(float))==0) return 1;
   return 0;
}

/*************************************************************************
 *
 *N  is_vpf_null_double
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose: 
 *P
 *     This function determines whether a double precision floating point
 *     number is the same as the VPF representation of the double
 *     precision floating point NULL value.  It returns TRUE or FALSE. 
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    num     <input> == (double) number to evaluate.
 *    return <output> == (int) TRUE (1) if the number is VPF NULL;
 *                             FALSE (0) if not.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   April 1991                        DOS Turbo C
 *E
 *************************************************************************/
int is_vpf_null_double( double num )
{
   double nan;

   nan = (double)quiet_notan(0);
   if (memcmp((void *)&nan,(void *)&num,sizeof(double))==0) return 1;
   return 0;
}


/*************************************************************************
 *
 *N  is_vpf_null_text
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function determines whether a character string is the same
 *     as the VPF representation of the text NULL value.  The null text
 *     value is dependent upon the count of the field.  This function
 *     returns TRUE or FALSE.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    str         <input> == (char *) string to evaluate.
 *    field_count <input> == (int) field count in the table.
 *    return     <output> == (int) TRUE (1) if the string is VPF NULL;
 *                             FALSE (0) if not.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1993                     UNIX ANSI C
 *E
 *************************************************************************/
int is_vpf_null_text( char *str, int field_count )
{
   int is_null=0;
   char *copy;
 
   if (field_count < 1)
      is_null = (*str == (char)NULL);
   else if (field_count == 1)
      is_null = (*str == '-');
   else if (field_count == 2)
      is_null = (strcmp(str,"--")==0);
   else {
      copy = (char *)strdup(str);
      rightjust(copy);
      is_null = (strcmp(copy,"N/A")==0);
      free(copy);
   }
   return is_null;
}   



