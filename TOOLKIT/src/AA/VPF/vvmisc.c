/*************************************************************************
 *
 *N  Module VVMISC.C - Miscellaneous Support Functions
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This module contains a fairly random collection of miscellaneous
 *     functions used by several of the VPFVIEW procedural functions.
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
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992      UNIX mdb port  Gnu C
 *E
 *    Phuong T. Nguyen (L3 Communication Corp.) Jun 17, 2002  
 *                                              Modified for C++ on linux.
 **************************************************************************/

#ifdef _PGS_THREADSAFE
#include <pthread.h>
#endif

#include <malloc.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#define INIT            register char *sp = instring;
#define GETC()          (*(sp++))
#define PEEKC()         (*sp)
#define UNGETC(c)       (--sp)
#define RETURN(c)       return c;
#define ERROR(c)        {printf("\nregexp error is %d\n",c);exit(-1);}
#define ESIZE 1024      /* Maximum compiled regular expression size */

#ifdef __cplusplus
int  __regcomp (void **, const char *, int, void *);
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
#include <regexp1.h>
#else
#include <regexp.h>
#endif
#else
#if (defined(CYGWIN) )
#include <regex.h>
#else
#include <regexp.h>
#endif
#endif

#include <stdarg.h>
#include "vvmisc.h"
#include "strfunc.h"



/*************************************************************************
 *
 *N  checkmalloc            
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P  
 *     Allocates the specified amount of memory and returns a pointer to
 *     the allocated data.  If the memory allocation fails, the application
 *     exits with an error message.  The memory allocated here should be
 *     freed when no longer needed.
 *E  
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A  
 *    size    <input>==(unsigned long int) size of memory block to be
 *                     allocated.
 *    return <output>==(void *) allocated pointer, if successful.
 *E  
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H  
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992    UNIX mdb port 
 *E
 *************************************************************************/
void *checkmalloc( int size )
{
  void *ptr;

  if (size == 0) {
    return NULL;
  }

  ptr = (void *)malloc(size);

  if (ptr == NULL) {
    printf("checkmalloc:  Failed to allocate %lu bytes.\n",size);
    exit(1); 
  }

  return ptr;
}

/*************************************************************************
 *
 *N  checkrealloc
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Reallocates the specified amount of memory and returns a pointer to
 *     the allocated data.  If the memory allocation fails, the application
 *     exits with an error message.  The memory allocated here should be
 *     freed when no longer needed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    pointer <input>==(void *) pointer to reallocate.
 *    size    <input>==(unsigned long int) size of memory block to be
 *                     allocated.
 *    return <output>==(void *) allocated pointer, if successful.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992    UNIX mdb port 
 *E
 *************************************************************************/
void *checkrealloc( void *pointer, unsigned  int size )
{
  void *ptr;

  if (size == 0) {
    free(pointer);
    return NULL;
  }

  ptr = (void *)realloc(pointer,size);

  if (ptr == NULL) {
    printf("checkrealloc: Failed to reallocate %lu bytes.\n",size);
    exit(1);
  }

  return ptr;
}

/*************************************************************************
 *
 *N  vpfcatpath 
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Concatenates a directory path in a way that is specific to the
 *     current operating system.  For UNIX, all VPF standard `\` directory
 *     separator characters are replaced with the UNIX '/' directory
 *     separator, and new directory names given from VPF sources are
 *     forced to lower case.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    dest   <output>==(char *) returned directory path string.       
 *    src     <input>==(char *) new string to add to the path.       
 *    return <output>==(char *) returned directory path string.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Brian Glover     Sep 1992                                     
 *E
 *************************************************************************/
char *vpfcatpath( char *dest, const char *src )
{
  char *copy, *p;
  register int length;

  length = strlen(src);
  if (length < 1) return dest;

  copy = (char *)checkmalloc(sizeof(char) * length);
  strcpy(copy,src);

  /* Replace VPF path separators with UNIX */
  p = &copy[0];
  while (*p) {
    if (*p == '\\') *p = '/';
    p++;
  }   
 
  rightjust(dest);
  leftjust(copy);
  strcat(dest, (char *)strlwr(copy));
 
  free(copy);
 
  return dest;

}

/*************************************************************************
 *
 *N  fileaccess
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Checks the existence and accessibility of a specified file.  If the
 *     file does not exist, it attempts to access the filename with a '.'
 *     appended to the end, in case the file is to be accessed from an
 *     ISO-9660 CD-ROM device on the SUN.  The access mode parameter (amode)
 *     determines the type of access check to be performed:
 *        0 - Check for existence of the file
 *        1 - Check for execute permission
 *        2 - Check for write permission
 *        4 - Check for read permission
 *        6 - Check for read and write permission
 *     The return value is 0 if the file is accessible for the specified
 *     mode; otherwise, the function returns a -1.  This function is
 *     similar to many system implementations of the function 'access()'.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    filename<input>==(char *) file (or directory) name to be checked. 
 *    amode   <input>==(int) access mode parameter.
 *    return <output>==(int) 0 if the file is accessible, -1 if not.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Bian Glover      Sep 1992                  
 *E
 *************************************************************************/
int fileaccess( const char *filename, int amode )
{
  struct stat statbuf;
  char *copy= NULL;	/* =NULL added */

/*  len = strlen(filename);
  copy = (char *) checkmalloc(sizeof(char) * (len + 1));

  (void) strcpy(copy,filename);
  copy[len] = '.';
  copy[len+1] = '\0';*/

  /* Retrieve status buffer for file via filename */

  if((stat(filename, &statbuf)) != 0)
    if((stat(copy, &statbuf)) != 0){
      free(copy);
      return(-1);
    }
 
/*  free(copy);*/
 
  switch (amode) {
    case 0:             /* If stat returned, the file exists */
           return(0);
    case 1:             /* Check for execute bits set in st_mode */
           if(geteuid() == statbuf.st_uid)
             return ((statbuf.st_mode & S_IXUSR)?0:-1);
           else if(getegid() == statbuf.st_gid)
             return ((statbuf.st_mode & S_IXGRP)?0:-1);
           return ((statbuf.st_mode & S_IXOTH)?0:-1);
    case 2:             /* Check for write bits set in st_mode */
           if(geteuid() == statbuf.st_uid)
             return ((statbuf.st_mode & S_IWUSR)?0:-1);
           else if(getegid() == statbuf.st_gid)
             return ((statbuf.st_mode & S_IWGRP)?0:-1);
           return((statbuf.st_mode & S_IWOTH)?0:-1);
    case 4:             /* Check for read bits set in st_mode */
           if(geteuid() == statbuf.st_uid)
             return ((statbuf.st_mode & S_IRUSR)?0:-1);
           else if(getegid() == statbuf.st_gid)
             return ((statbuf.st_mode & S_IRGRP)?0:-1);
           return((statbuf.st_mode & S_IROTH)?0:-1);
    case 6:             /* Check for read and write bits set in st_mode */
           if(geteuid() == statbuf.st_uid)
             return (((statbuf.st_mode & S_IWUSR) && 
                      (statbuf.st_mode & S_IRUSR))?0:-1);
           else if(getegid() == statbuf.st_gid)
             return (((statbuf.st_mode & S_IWGRP) && 
                      (statbuf.st_mode & S_IRGRP))?0:-1);
           return(((statbuf.st_mode & S_IWOTH) && 
                   (statbuf.st_mode & S_IROTH))?0:-1);
    default:          /* Fail if an incorrect amode was specified */
           return(-1);
  }

}

/*************************************************************************
 *
 *N  rand_name  
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Generates a random name of the specified length and returns it.  
 *     'name' must be allocated with at least 'length' bytes.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    length  <input>==(int) length of name string.
 *    name   <output>==(char *) pointer to the returned name string.
 *    return <output>==(char *) random name string.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992    UNIX mdb port 
 *E
 *************************************************************************/
char *rand_name( int length, char *name )
{
  int i,t,validchar;

  for(i=0;i<length;i++){
    validchar = 0;
    while (!validchar) {
       t = rand()%123;
       if (t >= '0' && t <= '9') validchar = 1;
       if (t >= 'A' && t <= 'Z') validchar = 1;
       if (t >= 'a' && t <= 'z') validchar = 1;
    }
    name[i] = t;
  }
  name[i] = '\0';
  return name;
}


static int mycmp (char **a, char **b)
{
  return strcmp(*a, *b);
}


/*************************************************************************
 *
 *N  findall     
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Returns a sorted list of all filenames matching the specified pattern,
 *     and a count of how many filenames matched (member).  This array of
 *     file names is dynamically allocated and should be freed when no longer
 *     needed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    path    <input>==(char *) directory path.
 *    pattern <input>==(char *) search pattern for UNIX regular expressions.
 *    member <output>==(int *) number of files found.
 *    return <output>==(char **) array of file names found.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Brian Glover     Sep 1992     
 *E
 *************************************************************************/
char **findall( char *path, char *pattern, int *member )
{
  char expbuff[ESIZE];
  DIR *testdir;
  struct dirent *entry;
  char **file_list;
 
  *member = 0;
 
  file_list = (char **)checkmalloc(sizeof(char *)*5);
 
  compile(pattern, expbuff, &expbuff[ESIZE], '\0');
 
  if(fileaccess(path,4) != 0){
    free(file_list);
    file_list = NULL;
    return file_list;
  }
 
  if((testdir = opendir(path)) != NULL) {
    while((entry=readdir(testdir)) != NULL)
      if(step(entry->d_name,expbuff)){
        file_list[*member] = (char *)checkmalloc(sizeof(char) *
                                               strlen(entry->d_name));
        (void) strcpy(file_list[*member],entry->d_name);
        *member+=1;
        file_list = (char **) checkrealloc(file_list, sizeof(char *) *
                                                    (*member + 5));
      }
    closedir(testdir);
  }    
 
  qsort(file_list, *member,sizeof(char*),(int (*)(const void *, const void *))mycmp);
  return file_list;

}

/*************************************************************************
 *
 *N  displayerror
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Displays an array of text strings to the user as an error message 
 *     and allows the user to select either retry or cancel.  Returns 1 
 *     if the user indicates retry, 0 for cancel.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    text    <input>==(char *)[] array of text strings.
 *    nlines  <input>==(int) number of lines of text.
 *    return <output>==(int) 1 for retry, 0 for cancel.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992    UNIX mdb port
 *E
 *************************************************************************/
int displayerror( char *text[], int nlines )
{
   int i;
   char ch;

   for (i=0;i<nlines;i++)
      printf("%s\n",text[i]);
   ch = 0;
   while (toupper(ch) != 'R' && toupper(ch) != 'C') {
      printf("R)etry / C)ancel: ");
      ch = getchar();
   }

   return (toupper(ch)=='R');
}

/*************************************************************************
 *
 *N  displayinfo 
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Displays an array of text strings as an informational message.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    text    <input>==(char *)[] array of text strings.
 *    nlines  <input>==(int) number of lines of text.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992    UNIX mdb port
 *E
 *************************************************************************/
void displayinfo( char *text[], int nlines )
{
   int i;

   for (i=0;i<nlines;i++)
      printf("%s\n",text[i]);
}

/*************************************************************************
 *
 *N  display_message
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Displays a single string to the user as an informational message.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    str     <input>==(char *) text string to be displayed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992    UNIX mdb port
 *E
 *************************************************************************/
void display_message( char *str )
{
   printf("%s\n",str);
}

/*************************************************************************
 *
 *N  var_display_message
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Displays a variable number of strings to the user as an informational
*      message.  The list of parameters must end with a NULL.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    str     <input>==(char *) first text string to be displayed.
 *    ...     <input>==(char *) additional text strings to be displayed.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                     DOS Turbo C
 *                     Nov 1992    UNIX mdb port
 *E
 *************************************************************************/
void var_display_message( char *str, ... )
{
    
   printf("** message display functionality disabled **\n");
   
/* NOTE: the following code was commented out by Tej TEJ tej (it doesn't seem to
   be used by anything anyway) since it won't compile on the DEC (the above line
   of code was substituted). */

/*
   printf("%s\n",str);
   va_start(ap,string);
   while ((s = va_arg(ap,char *)) != NULL)
      printf("%s\n",s);
   va_end(ap);
*/
}





