/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_SMF_Comp.c

DESCRIPTION:
  This file contains functions for creating smfcompile utility to support
  error status handling.

AUTHOR:
  Kelvin K. Wan / Applied Research Corp.
  Phuong T. Nguyen/ Emergent Information Technologies, Inc. 

HISTORY:
  25-Mar-1994 KCW Standard Convention  
  18-Jul-1994 KCW Modify to conform to PGS_TYPES.h
  27-Jul-1994 KCW Modify to allow digit in the mnemonic label
   4-Jan-1995 KCW Modify error messages for TK5
  26-Jan-1995 KCW enhance smfcompile with "-r" and "-i" for TK5
  16-Oct-1995 DPH Modified the following functions to support the
		  addition if the Channel Action level 'C':
		  PGS_SMF_ValidMnemonic
		  PGS_SMF_ValidMsgStr
		  PGS_SMF_CreateHeaderFile
		  PGS_SMF_CreateAdaFile
		  PGS_SMF_CreateFortranFile
		  PGS_SMF_CreateFortranSubFile
		  PGS_SMF_CorrectNumActMnemonic
		  PGS_SMF_GetErrMsg
		  PGS_SMF_GetBNF
		  Also cleaned-up some in-line comments along the way.
   30-Jun-2000 PN NCR#: 15195, improve smfcompile
   05-Mar-2001 AT Modified for linux
   30-Sep-2003 AT Modified for linux g++
   03-Feb-2005 MP Modified to use strerror instead of sys_errlist 
                  for Linux (ECSed41850)

END_FILE_PROLOG:
*****************************************************************/

/*
 * System Headers 
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h> 
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#if !( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )/* defined in PGS_SMF.h, g++ in linux 
		 gives error if it is redefined */
extern int   errno;
#endif
#if !( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
extern int   sys_nerr;
#endif
#if !( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
extern char *sys_errlist[];
#endif
/*
 * Toolkit Headers 
 */
#include <PGS_SMF.h>
#include <PGS_EcsVersion.h>

/*
 * Macro 
 */
#define CTOI(c) ((PGSt_integer) (unsigned char)(c))

/*
 * Termination Flag 
 */
#define TERMINATE_ERROR       1
#define TERMINATE_SUCCESS     0

/*
 * Redirecting File
 */
#define PGS_SMF_REDIRECT_HEADER  1
#define PGS_SMF_REDIRECT_ADA     2
#define PGS_SMF_REDIRECT_FORTRAN 3

/*
 * Line Delimeter or White-Space 
 */
char tokensep_space[] = " \t";

/*
 * Structure Definition
 */
typedef struct
{
    short             smf_errmsg_no;                               /* error number */
    int               smf_errno;                                   /* UNIX errno */
    char             *smf_redirectFile;                            /* file to redirect */
    char             *smf_filename;                                /* ascii file to process */                             
    char             *smf_fileC;                                   /* C header file */
    char             *smf_fileF;                                   /* Fortran header file */   
    char             *smf_fileAda;                                 /* ADA header file */
    char             *smf_fileB;                                   /* ASCII message file */  
    char              smf_filelabel[PGS_SMF_MAX_LABEL_SIZE];       /* mnemonic label */
    PGSt_SMF_boolean  C;                                           /* flag to create C header file */
    PGSt_SMF_boolean  f90;                                         /* flag to create F90 header file */
    PGSt_SMF_boolean  f77;                                         /* flag to create F77 header file */
    PGSt_SMF_boolean  ada;                                         /* flag to create ADA header file */
    PGSt_SMF_boolean  redirectMSG;                                 /* redirect MSG file to PGSMSG directory */
    PGSt_SMF_boolean  redirectINC;                                 /* redirect INC file to PGSINC directory */
    short             smf_charindex;                               /* index of the line */
    char              smf_linebuf[PGS_SMF_MAX_MSGBUF_SIZE];        /* current line */
    char              smf_action[PGS_SMF_MAX_MSG_SIZE];            /* action label */
    char              smf_mnemonic[PGS_SMF_MAX_MSG_SIZE];          /* mnemonic label */
    short             smf_linecnt;            /* line count */

    fpos_t            startpos;               /* file start position */
    FILE             *fptr;                   /* file pointer */
}PgsSmfGlbVar;

/*
 * Functions 
 */
static void              PGS_SMF_CompGetGlbVar         (PgsSmfGlbVar **globalvar);
static PGSt_SMF_boolean  PGS_SMF_FileExist             (char *filename,FILE **fptr);
static PGSt_SMF_boolean  PGS_SMF_GetFileInfo           (FILE *fptr,PGS_SMF_FileInfo *fileinfo);
static PGSt_SMF_boolean  PGS_SMF_BlankLine             (char *linebuf);
static PGSt_SMF_boolean  PGS_SMF_ValidLine             (char *linebuf);
static PGSt_SMF_boolean  PGS_SMF_PercentChar           (char *linebuf);
static PGSt_SMF_boolean  PGS_SMF_GetInstr              (char *linebuf,PGS_SMF_FileInfo *fileinfo);
static PGSt_SMF_boolean  PGS_SMF_GetLabel              (char *linebuf,PGS_SMF_FileInfo *fileinfo);
static PGSt_SMF_boolean  PGS_SMF_GetSeed               (char *linebuf,PGS_SMF_FileInfo *fileinfo);
static PGSt_SMF_boolean  PGS_SMF_CapitalLetter         (char *str);
static PGSt_SMF_boolean  PGS_SMF_ValidSeed             (char *seed);
static PGSt_SMF_boolean  PGS_SMF_ValidMnemonic         (char *linebuf,char *label,char *mnemonic);
static PGSt_SMF_boolean  PGS_SMF_ValidMsgStr           (char *linebuf,char *mnemonic,char *msg,char *action,char *label);
static void              PGS_SMF_RemoveFrontEndSpace   (char *linebuf);
static int               PGS_SMF_ScanActionSymbol      (char *str);
static void              PGS_SMF_Exit                  (short terminate_status);
static PGSt_SMF_boolean  PGS_SMF_ExtractSMFName        (char *orgpath,char *filepath,char *filename);
static PGSt_SMF_boolean  PGS_SMF_CreateHeaderFile      (PGS_SMF_FileInfo *fileinfo,PGS_SMF_MsgData msgdata[],short cnt,char *filepath,char *filename);
static PGSt_SMF_boolean  PGS_SMF_CreateAdaFile         (PGS_SMF_FileInfo *fileinfo,PGS_SMF_MsgData msgdata[],short cnt,char *filepath,char *filename);
static PGSt_SMF_boolean  PGS_SMF_CreateFortranFile     (PGS_SMF_FileInfo *fileinfo,PGS_SMF_MsgData msgdata[],short cnt,char *filepath,char *filename);
static PGSt_SMF_boolean  PGS_SMF_CreateFortranSubFile  (PGS_SMF_FileInfo *fileinfo,PGS_SMF_MsgData msgdata[],short cnt,char *filepath,char *filename);
static PGSt_SMF_boolean  PGS_SMF_CreateASCIIFile       (PGS_SMF_FileInfo *fileinfo,PGS_SMF_MsgData msgdata[],short cnt,char *filepath,char *filename);
static PGSt_SMF_boolean  PGS_SMF_CorrectNumActMnemonic (PGS_SMF_MsgData *msgdata,short cnt);
static PGSt_SMF_boolean  PGS_SMF_CommentLine           (char *linebuf);
static void              PGS_SMF_HexStrCode            (PGSt_SMF_code code,char *str_code);
static void              PGS_SMF_HexChar               (PGSt_SMF_code byte_val,char *hexchar);
static PGSt_SMF_boolean  PGS_SMF_GetWord               (char *linebuf,char *word,short *index);
static void              PGS_SMF_RemoveCarriageReturn  (char *linebuf);
static PGSt_SMF_boolean  PGS_SMF_StatDefOnSameLine     (char *label);
static PGSt_SMF_boolean  PGS_SMF_StatusLevel           (char *mnemonic,char *stat_lev);
static void              PGS_SMF_RemoveSpecialChar     (char *action_label);
static void              PGS_SMF_ExtractMsg            (char *linebuf,char *mnemonic,char **msgptr) ;
static char             *PGS_SMF_GetErrMsg             (short errmsg_no);
static char             *PGS_SMF_GetBNF                (short bnf_no);
static void              PGS_SMF_ScanWord              (char *word);
static PGSt_SMF_boolean  PGS_SMF_SystemCall            (char *cmdStr);
static PGSt_SMF_boolean  PGS_SMF_RedirectMsgFile       (char *filename);
static PGSt_SMF_boolean  PGS_SMF_RedirectIncFile       (short fileToRedirect,char *filename);



/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    smfcompile

SYNOPSIS:      
C:  
    smfcompile -f textfile [-r] [-i]
    smfcompile -f textfile -c [-r] [-i]

FORTRAN:
    smfcompile -f textfile -f77 [-r] [-i]

ALL:
    smfcompile -f textfile -all [-r] [-i]

Ada:
    smfcompile -f textfile -ada [-r] [-i]

DESCRIPTION:
    This utility generates runtime status message files and language
    dependent include files from user-defined status message text files.

INPUTS:
    textfile - status message text file (eg. PGS_IO_100.t)
    -c       - create C include file
    -f77     - create Fortran include file
    -all     - create Fortran, C and Ada include files
    -r       - redirect the created ASCII runtime message file to the directory
               set in the environment variable "PGSMSG" 
    -i       - redirect the created language-specific include file to the directory
               set in the environment variable "PGSINC" 

OUTPUTS:
    Language-specific include file and ASCII runtime message file (an Ada
    package specification will be produced in place of an include file
    when the '-ada' switch is used).

RETURNS:
    1 - error occured
    0 - successful operation

EXAMPLES:	
    smfcompile -f PGS_IO_100.t
    (produces PGS_IO_100.h and PGS_100)

    smfcompile -f PGS_IO_100.t -c
    (produces PGS_IO_100.h and PGS_100)
    
    smfcompile -f PGS_IO_100.t -f77
    (produces PGS_IO_100.f and PGS_100)

    smfcompile -f PGS_IO_100.t -all
    (produces PGS_IO_100.f, PGS_IO_100.h, PGS_IO_100.a and PGS_100)

NOTES:
    The environment variable PGSMSG must be set to the local Toolkit installation
    directory '/../pgs/message'. The reason for this is that Toolkit status
    message files will already reside in this directory upon completion of
    the Toolkit installation procedure; these files must be visible at runtime
    for the Toolkit to function properly.

    If you do not specify the "-r" input parameter to the smfcompile, then make sure
    that the newly created ASCII runtime message file is moved to the directory set
    in the environment variable "PGSMSG".

REQUIREMENTS:
    PGSTK-0580,0590,0591,0600,0610,0620,0650,0664

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_FileExist()
    PGS_SMF_GetFileInfo()
    PGS_SMF_StatDefOnSameLine()
    PGS_SMF_ValidMsgStr()
    PGS_SMF_CorrectNumActMnemonic()
    PGS_SMF_ExtractSMFName()
    PGS_SMF_CreateAdaFile()
    PGS_SMF_CreateHeaderFile()
    PGS_SMF_CreateFortranFile()
    PGS_SMF_CreateASCIIFile()
    PGS_SMF_Exit()    
    PGS_SMF_RedirectMsgFile()
    PGS_SMF_RedirectIncFile()

END_PROLOG:
*****************************************************************/
int main(            /* Main function */
    int     argc,    /* Input number of parameters */
    char   *argv[])  /* Input parameters */
{
/*  PGSt_SMF_boolean    returnBoolean; */                 /* return boolean */
    PGS_SMF_MsgData     msgdata[PGS_SMF_MAX_MNEMONIC];    /* message data */
    PGS_SMF_FileInfo    fileinfo;                         /* file information */
    short               mnemonic_cnt = 0;                 /* mnemonic counter */
    short               i;                                /* counter */
    char                filename[255];                    /* file name */
    char                filepath[500];                    /* file path */
    char                textfile[500];                    /* text file */
    PgsSmfGlbVar       *global_var;                       /* global static buffer */


    PGS_SMF_CompGetGlbVar(&global_var);

    filename[0] = '\0';
    filepath[0] = '\0';
    textfile[0] = '\0';

    global_var->C   = PGS_FALSE;
    global_var->f90 = PGS_FALSE;
    global_var->f77 = PGS_FALSE;
    global_var->ada = PGS_FALSE;

    /*
     * Parse the input parameter.
     */
    for (i=1 ; i < argc ; i++)
    {
	if (strcmp(argv[i],"-c") == 0) 
        { 
            global_var->C = PGS_TRUE;
	}
        else if (strcmp(argv[i],"-f77") == 0) 
        { 
            global_var->f77 = PGS_TRUE; 
        }    
        else if (strcmp(argv[i],"-ada") == 0) 
        { 
            global_var->ada = PGS_TRUE; 
        }      
        else if (strcmp(argv[i],"-all") == 0) 
        { 
            global_var->ada = PGS_TRUE;
            global_var->f77 = PGS_TRUE;
            global_var->C   = PGS_TRUE;
        }                                                 
        else if (strcmp(argv[i],"-f")   == 0) 
        { 
            sprintf(textfile,"%s",argv[i+1]); 
            i++;
        }
        else if (strcmp(argv[i],"-r")   == 0) 
        { 
            global_var->redirectMSG = PGS_TRUE;
        }
        else if (strcmp(argv[i],"-i")   == 0) 
        { 
            global_var->redirectINC = PGS_TRUE;
        }        
    }

    /*
     * If user type: smfcompile -f <text_file>, then default
     * to C header file.
     */
    if ((strlen(textfile) != 0) &&
        (global_var->f90 == PGS_FALSE) &&
        (global_var->f77 == PGS_FALSE) &&        
        (global_var->ada == PGS_FALSE))
    {
        global_var->C = PGS_TRUE;
    }

    /*
     * Extract header information from the file.
     */
    if (PGS_SMF_FileExist(&textfile[0],&global_var->fptr) == PGS_FALSE)  
    {
        PGS_SMF_Exit(TERMINATE_ERROR);
    }

    if (PGS_SMF_GetFileInfo(global_var->fptr,&fileinfo) == PGS_FALSE)    
    {
        PGS_SMF_Exit(TERMINATE_ERROR);
    }

    /*
     * Extract mnemonic message.
     */
    for (;;)
    {
        fgetpos(global_var->fptr, &global_var->startpos);

        if (fgets(global_var->smf_linebuf,PGS_SMF_MAX_MSGBUF_SIZE,global_var->fptr) == (char *)NULL)
        {
            if (feof(global_var->fptr) != 0)
            {
                /*
                 * EOF reached.
                 */

                if (mnemonic_cnt == 0)
                {
                    /*
                     * No mnemonic is defined in the file.
                     */
                    global_var->smf_linecnt += 1;
                    global_var->smf_errmsg_no = 16;
                    PGS_SMF_Exit(TERMINATE_ERROR);
                }                
            }
            else 
            {
                /*
                 * Some read error.
                 */
                global_var->smf_errno = errno;
                global_var->smf_errmsg_no = 3;
                PGS_SMF_Exit(TERMINATE_ERROR);
            }            

            break;
        }
        else 
        {
            /*
             * Cannot exceed the maximum number of mnemonics allowed by SMF.
             */
            if (mnemonic_cnt >= PGS_SMF_MAX_MNEMONIC) 
            {                       
                global_var->smf_errmsg_no = 10;
                PGS_SMF_Exit(TERMINATE_ERROR);
            }

            /*
             * Multiple message lines per mnemonic is allowed; thus this routine will
             * concatenate all the lines into one line for easy processing.
             */
            if (PGS_SMF_StatDefOnSameLine(fileinfo.label) == PGS_FALSE)                                 
            {
                PGS_SMF_Exit(TERMINATE_ERROR);
            }

            if (global_var->smf_linebuf[0] == '\0')
            {
                /*
                 * EOF reached since no mnemonic is retrieved.
                 */
                break;
            }

            /*
             * Make sure that the concatenated line follows SMF rules.
             */
            if (PGS_SMF_ValidMsgStr(global_var->smf_linebuf,
                                    msgdata[mnemonic_cnt].mnemonic,
                                    msgdata[mnemonic_cnt].msg,
                                    msgdata[mnemonic_cnt].action,
                                    fileinfo.label) == PGS_FALSE)                         
            {                               
                PGS_SMF_Exit(TERMINATE_ERROR);
            }

            mnemonic_cnt++;
        }
    }

    if (global_var->fptr != (FILE *)NULL)
    {
        fclose(global_var->fptr);        
        global_var->fptr = (FILE *)NULL;
    }

    /*
     * Make sure that the corresponding mnemonics with action label exist in the file.
     */
    if (PGS_SMF_CorrectNumActMnemonic(msgdata,mnemonic_cnt) == PGS_FALSE) 
    {
        PGS_SMF_Exit(TERMINATE_ERROR);
    }
    else
    {
        /*
         * Everything is okay, thus proceed to create the appropriate header files.
         */
        PGS_SMF_ExtractSMFName(textfile,filepath,filename);

        if (global_var->C == PGS_TRUE)
        {
            if (PGS_SMF_CreateHeaderFile(&fileinfo,msgdata,mnemonic_cnt,filepath,filename) == PGS_FALSE) 
            {
                PGS_SMF_Exit(TERMINATE_ERROR);
            }
            else
            {
                /*
                 * Check if user wants to redirect the INC file.
                 */
                if (global_var->redirectINC == PGS_TRUE)
                {
                    if (PGS_SMF_RedirectIncFile(PGS_SMF_REDIRECT_HEADER,filename) == PGS_FALSE)            
                    {
                        PGS_SMF_Exit(TERMINATE_ERROR);
                    }
                }   
            }
        }

        if (global_var->f77 == PGS_TRUE)
        {
            if (PGS_SMF_CreateFortranFile(&fileinfo,msgdata,mnemonic_cnt,filepath,filename) == PGS_FALSE) 
            {
                PGS_SMF_Exit(TERMINATE_ERROR);
            }
            else
            {
                /*
                 * Check if user wants to redirect the INC file.
                 */
                if (global_var->redirectINC == PGS_TRUE)
                {
                    if (PGS_SMF_RedirectIncFile(PGS_SMF_REDIRECT_FORTRAN,filename) == PGS_FALSE)            
                    {
                        PGS_SMF_Exit(TERMINATE_ERROR);
                    }
                }   
            }
        }

        if (global_var->ada == PGS_TRUE)
        {
            if (PGS_SMF_CreateAdaFile(&fileinfo,msgdata,mnemonic_cnt,filepath,filename) == PGS_FALSE) 
            {
                PGS_SMF_Exit(TERMINATE_ERROR);
            }
            else
            {
                /*
                 * Check if user wants to redirect the INC file.
                 */
                if (global_var->redirectINC == PGS_TRUE)
                {
                    if (PGS_SMF_RedirectIncFile(PGS_SMF_REDIRECT_ADA,filename) == PGS_FALSE)            
                    {
                        PGS_SMF_Exit(TERMINATE_ERROR);
                    }
                }   
            }
        }

        if (PGS_SMF_CreateASCIIFile(&fileinfo,msgdata,mnemonic_cnt,filepath,filename) == PGS_FALSE)            
        {
            PGS_SMF_Exit(TERMINATE_ERROR);
        }    
        else  
        {            
            /*
             * Check if user wants to redirect the MSG file.
             */
            if (global_var->redirectMSG == PGS_TRUE)
            {
                if (PGS_SMF_RedirectMsgFile(filename) == PGS_FALSE)            
                {
                    PGS_SMF_Exit(TERMINATE_ERROR);
                }   
            }
        }
    }            


    PGS_SMF_Exit(TERMINATE_SUCCESS);

} /* end main */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CompGetGlbVar

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    void 
    PGS_SMF_CompGetGlbVar(
        PgsSmfGlbVar **globalvar);

FORTRAN:
    NONE

DESCRIPTION:
    To encapsulate all the global variables that are used
    in this program in a function.

INPUTS:      
    Name            Description                                Units        Min        Max   

OUTPUTS:   
    Name            Description                                Units        Min        Max

    globalvar       pointer to structure containing global 
                    data (static buffer)

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_CompGetGlbVar(          /* Get global data */
    PgsSmfGlbVar **globalvar)  /* Output pointer to structure containing global data */
{
    static PgsSmfGlbVar      global_var;                  /* global static buffer */
    static PGSt_SMF_boolean  initStaticBuf = PGS_FALSE;   /* flag to intialize static buffer */


    if (initStaticBuf == PGS_FALSE)
    {
        initStaticBuf = PGS_TRUE;

        global_var.smf_errmsg_no    = 0;
        global_var.smf_redirectFile = (char *)NULL;
        global_var.smf_filename     = (char *)NULL;
        global_var.smf_fileC        = (char *)NULL;
        global_var.smf_fileF        = (char *)NULL;
        global_var.smf_fileAda      = (char *)NULL;
        global_var.smf_fileB        = (char *)NULL;        
        global_var.smf_charindex    = 0;
        global_var.smf_linebuf[0]   = '\0';
        global_var.smf_linecnt      = 0;
        global_var.fptr             = (FILE *)NULL;
    }

    *globalvar = &global_var;


} /* end PGS_SMF_CompGetGlbVar */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_FileExist

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_FileExist(
        char  *filename,
        FILE **fptr);

FORTRAN:
    NONE

DESCRIPTION:
    To check if the given status text file exists.

INPUTS:    
    Name            Description                                Units        Min        Max   

    filename        text file name

OUTPUTS:    
    Name            Description                                Units        Min        Max   

    fptr            file pointer

RETURNS: 
    PGS_TRUE - file exist
    PGS_FALSE - file does not exist

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_FileExist(              /* Check text file */
    char  *filename,            /* Input  text file name */
    FILE **fptr)                /* Output file pointer */
{
    PGSt_SMF_boolean   returnBoolean = PGS_TRUE;  /* return boolean */
    PgsSmfGlbVar      *global_var;                /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    if (strlen(filename) == 0)
    {
        global_var->smf_errmsg_no = 1;
        returnBoolean = PGS_FALSE;
    }
    else
    {
        global_var->smf_filename = filename;
        *fptr = fopen(filename,"r");
        if (*fptr == (FILE *)NULL)
        {
            global_var->smf_errmsg_no = 2;
            returnBoolean = PGS_FALSE;
        }
    }

    return(returnBoolean);

} /* end PGS_SMF_FileExist */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetFileInfo

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_GetFileInfo(
        FILE             *fptr,
        PGS_SMF_FileInfo *fileinfo);

FORTRAN:
    NONE

DESCRIPTION:
    To read %INSTR, %LABEL, %SEED from the text file.

INPUTS:
    Name            Description                                Units        Min        Max   

    fptr            open file pointer

OUTPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        file information

RETURNS:
    PGS_TRUE - file info. exist
    PGS_FALSE - file info. do not exist

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_RemoveCarriageReturn()
    PGS_SMF_RemoveFrontEndSpace()
    PGS_SMF_BlankLine()
    PGS_SMF_CommentLine()
    PGS_SMF_PercentChar()
    PGS_SMF_ValidLine()
    PGS_SMF_GetInstr()
    PGS_SMF_GetLabel()
    PGS_SMF_GetSeed()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_GetFileInfo(              /* Get file information */
    FILE             *fptr,       /* Input  open file pointer */
    PGS_SMF_FileInfo *fileinfo)   /* Output file information */
{
    PGSt_SMF_boolean  returnBoolean = PGS_FALSE;         /* return boolean */
    PGSt_SMF_boolean  pass;                              /* flag to indiacte everything is okay */
    PGSt_SMF_boolean  instr = PGS_FALSE;                 /* instr retrieve */
    PGSt_SMF_boolean  label = PGS_FALSE;                 /* label retrieve */    
    PGSt_SMF_boolean  seed  = PGS_FALSE;                 /* seed retrieve */            
    short             cnt = 1;                           /* token counter */
    char              linebuf[PGS_SMF_MAX_MSGBUF_SIZE];  /* line buffer */
/*  int               i; */                              /* counter */
    PgsSmfGlbVar     *global_var;                        /* global static buffer */


    PGS_SMF_CompGetGlbVar(&global_var);

    fileinfo->seed = 0;
    fileinfo->instr[0] = '\0';
    fileinfo->label[0] = '\0';

    for (;;)
    {
        global_var->smf_linebuf[0] = '\0';
        global_var->smf_linecnt += 1; 

        if (fgets(global_var->smf_linebuf,PGS_SMF_MAX_MSGBUF_SIZE,fptr) == (char *)NULL)
        {
            if (feof(fptr) != 0)
            {
                /*
                 * EOF reached.
                 */

                /*
                 * Check which tokens are not retrieved.
                 */
                if (instr == PGS_FALSE)
                {
                    global_var->smf_errmsg_no = 19;
                }
                else if (label == PGS_FALSE)
                {
                    global_var->smf_errmsg_no = 20;
                }
                else if (seed == PGS_FALSE)
                {
                    global_var->smf_errmsg_no = 21;
                }                
            }
            else 
            { 
                /*
                 * Some read error.
                 */
                global_var->smf_errno = errno;
                global_var->smf_errmsg_no = 3;
            }

            break;
        }
        else
        {
            strcpy(linebuf,global_var->smf_linebuf);

            PGS_SMF_RemoveCarriageReturn(linebuf);
            PGS_SMF_RemoveFrontEndSpace(linebuf);

            if (PGS_SMF_BlankLine(linebuf) == PGS_TRUE)    
            {
                continue;
            }

            if (PGS_SMF_CommentLine(linebuf) == PGS_TRUE)  
            {
                continue;
            }

            if (PGS_SMF_PercentChar(linebuf) == PGS_FALSE) 
            { 
                if (instr == PGS_FALSE)
                {
                    global_var->smf_errmsg_no = 19;
                }
                else if (label == PGS_FALSE)
                {
                    global_var->smf_errmsg_no = 20;
                }
                else if (seed == PGS_FALSE)
                {
                    global_var->smf_errmsg_no = 21;
                }

                break;
            }

            if (PGS_SMF_ValidLine(linebuf) == PGS_FALSE)   
            {
                global_var->smf_errmsg_no = 4;
                break;
            }

            /*
             * Check to make sure that the tokens are properly defined.
             */
            pass = PGS_FALSE;

            switch (cnt)
            {
                case 1:
                     if (PGS_SMF_GetInstr(linebuf,fileinfo) == PGS_TRUE)
                     {
                         instr = PGS_TRUE;
                         pass  = PGS_TRUE;
                     }
                     break;
                case 2:
                     if (PGS_SMF_GetLabel(linebuf,fileinfo) == PGS_TRUE)
                     {
                         label = PGS_TRUE;
                         pass  = PGS_TRUE;
                     }
                     break;
                case 3:
                     if (PGS_SMF_GetSeed(linebuf,fileinfo) == PGS_TRUE)
                     {
                         seed = PGS_TRUE;
                         returnBoolean = PGS_TRUE;
                     }
                     break;
            }

            if (pass == PGS_FALSE)  
            {
                break;
            }

            cnt++;
        }
    }

    /*
     * Reset the buffer.
     */
    if (returnBoolean == PGS_TRUE)
    {
        global_var->smf_linebuf[0] = '\0';
    }

    return(returnBoolean);

} /* end PGS_SMF_GetFileInfo */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_BlankLine

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_BlankLine(
        char *linebuf);

FORTRAN:
    NONE

DESCRIPTION:
    To check if the line consists of blank/space/tab characters.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string

OUTPUTS:    
    Name            Description                                Units        Min        Max          

RETURNS:
    PGS_TRUE - it is a blank line
    PGS_FALSE - it is not a blank line

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_BlankLine(  /* Check blank line */
    char *linebuf)  /* Input line string */
{
    short            i;                           /* counter */
    PGSt_SMF_boolean returnBoolean = PGS_TRUE;    /* return boolean */

    for (i=0 ; i < (short)strlen(linebuf) ; i++)
    {
        if ((linebuf[i] != ' ')  && 
            (linebuf[i] != '\t') &&
            (linebuf[i] != '\n'))
        {
            returnBoolean = PGS_FALSE;
            break;
        }
    }
    return(returnBoolean);

} /* end PGS_SMF_BlankLine */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ValidLine

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_ValidLine(
        char *linebuf);

FORTRAN:
    NONE

DESCRIPTION:
    To check if the line follow SMF allowable characters.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string

OUTPUTS:  
    Name            Description                                Units        Min        Max            

RETURNS:
    PGS_TRUE - valid SMF characters
    PGS_FALSE - invalid SMF characters 

EXAMPLES:
    NONE

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_ValidLine(     /* Check SMF valid characters */
    char *linebuf)     /* Input line string */
{
    short             i;                         /* counter */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;  /* return boolean */
    PgsSmfGlbVar     *global_var;                /* global static buffer */


    PGS_SMF_CompGetGlbVar(&global_var);

    for (i=0 ; i < (short)strlen(linebuf) ; i++)
    {
        if ((linebuf[i]  != '\n') && 
            (linebuf[i]  != '\t') && 
            ((linebuf[i]  < ' ')  || 
             (linebuf[i]  > '~')))
        {
            returnBoolean = PGS_FALSE;
            break;
        }
    }

    return(returnBoolean);

} /* end PGS_SMF_ValidLine */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_PercentChar

SYNOPSIS:      
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_PercentChar(
        char *linebuf);

FORTRAN:
    NONE

DESCRIPTION:
    To check if the line starts with % character.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string

OUTPUTS:    
    Name            Description                                Units        Min        Max            

RETURNS:
    PGS_TRUE  - line starts with % character
    PGS_FALSE - line do not start with % character

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_PercentChar(    /* Check line start with % character */
    char *linebuf)      /* Input line string */
{
    short             i;                          /* counter */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;   /* return boolean */
    PgsSmfGlbVar     *global_var;                 /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    for (i=0 ; i < (short)strlen(linebuf) ; i++)
    {
        if ((linebuf[i] != '\n') &&
            (linebuf[i] != ' ')  &&
            (linebuf[i] != '\t')) 
        {
            if (linebuf[i] != '%')
            {
                returnBoolean = PGS_FALSE;
            }
            break;
        }
    }

    return(returnBoolean);

} /* end PGS_SMF_PercentChar */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetInstr

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_GetInstr(
        char             *linebuf,
        PGS_SMF_FileInfo *fileinfo);

FORTRAN:
    NONE

DESCRIPTION:
    To extract %INSTR.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string

OUTPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        buffer for saving file information

RETURNS:
    PGS_TRUE  - %INSTR found and valid
    PGS_FALSE - %INSTR not found or invalid

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_GetInstr(                /* Extract %INSTR */
    char             *linebuf,   /* Input  line string */
    PGS_SMF_FileInfo *fileinfo)  /* Output file information */
{
    PGSt_SMF_boolean  returnBoolean = PGS_FALSE;    /* return boolean */
    char             *token;                        /* string token */
    char              str[PGS_SMF_MAX_MSG_SIZE];    /* string buffer */
    PgsSmfGlbVar     *global_var;                   /* global static buffer */


    PGS_SMF_CompGetGlbVar(&global_var);

    strcpy(str,linebuf);
    token = strtok(str,tokensep_space);
    if ((token != (char *)NULL) && (strncmp(token,"%INSTR",6) == 0))
    {
        token = strtok((char *)NULL,tokensep_space);
        if ((token != (char *)NULL) && (strcmp(token,"=") == 0))
        {
            token = strtok((char *)NULL,tokensep_space);
            if (token != (char *)NULL)
            {
                strcpy(fileinfo->instr,token);
                token = strtok((char *)NULL,tokensep_space);

                if (token != (char *)NULL)            
                {
                    /*
                     * More than one tokens are defined.
                     */
                    global_var->smf_errmsg_no = 32; 
                } 
                else if (PGS_SMF_CapitalLetter(fileinfo->instr) == PGS_FALSE)
                {
                    /*
                     * Token must be capital letters.
                     */
                    global_var->smf_errmsg_no = 33; 
                }
                else if (((short)strlen(fileinfo->instr) >= 3) && 
                         ((short)strlen(fileinfo->instr) <= (PGS_SMF_MAX_INSTR_SIZE-1))) 
                {
                    returnBoolean = PGS_TRUE;
                }  
                else
                {                                
                    /*
                     * Token exceed the maximum or less than the minimum required characters.
                     */
                    global_var->smf_errmsg_no = 34; 
                }
            }
            else
            {
                /*
                 * No token is defined. Proper usage: %INSTR = instrument.
                 * Note that space characters must be in front and back of "=" character.
                 */
                global_var->smf_errmsg_no = 31;
            }
        }
        else
        {
            /*
             * No token is defined. Proper usage: %INSTR = instrument.
             * Note that space characters must be in front and back of "=" character.
             */
            global_var->smf_errmsg_no = 31;
        }
    }
    else
    {
        /*
         * %INSTR not specify.
         */
        global_var->smf_errmsg_no = 30;
    }

    return(returnBoolean);

} /* end PGS_SMF_GetInstr */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetLabel

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_GetLabel(
        char             *linebuf,
        PGS_SMF_FileInfo *fileinfo);

FORTRAN:
    NONE

DESCRIPTION:
    To extract %LABEL.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string

OUTPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        buffer for saving file information

RETURNS:
    PGS_TRUE - %LABEL found and valid
    PGS_FALSE - %LABEL not found or invalid

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_GetLabel(                /* Extract %LABEL */
    char             *linebuf,   /* Input  line string */
    PGS_SMF_FileInfo *fileinfo)  /* Output file information */
{
    PGSt_SMF_boolean  returnBoolean = PGS_FALSE;    /* return boolean */
    char             *token;                        /* string token */
    char              str[PGS_SMF_MAX_MSG_SIZE];    /* string buffer */
    PgsSmfGlbVar     *global_var;                   /* global static buffer */
    short             i;                            /* counter */

    PGS_SMF_CompGetGlbVar(&global_var);

    strcpy(str,linebuf);
    token = strtok(str,tokensep_space);
    if ((token != (char *)NULL) && (strncmp(token,"%LABEL",6) == 0))
    {
        token = strtok((char *)NULL,tokensep_space);
        if ((token != (char *)NULL) && (strcmp(token,"=") == 0))
        {
            token = strtok((char *)NULL,tokensep_space);
            if (token != (char *)NULL)
            {
                strcpy(fileinfo->label,token);
                token = strtok((char *)NULL,tokensep_space);

                if (token != (char *)NULL)            
                {
                    /*
                     * More than one tokens are defined.
                     */
                    global_var->smf_errmsg_no = 37; 
                } 
                else if (PGS_SMF_CapitalLetter(fileinfo->label) == PGS_FALSE)
                {
                    /*
                     * Token must be capital letters.
                     */
                    global_var->smf_errmsg_no = 38; 
                }
                else if (((short)strlen(fileinfo->label) >= 3) &&
                         ((short)strlen(fileinfo->label) <= (PGS_SMF_MAX_LABEL_SIZE-1)))
                {
                    /*
                     * If ARC people are using the smfcompile, then we make sure that the
                     * defined post-label (eg. %LABEL = PGSIO, the post-label is 'IO' or 
                     * %LABEL = PGSSMF, the post-label is 'SMF') should be used to name
                     * the generated include file(s).
                     */
                    if (strncmp(fileinfo->label,"PGS",3) == 0)
                    {
                        for (i=3 ; i < (short)strlen(fileinfo->label) ; i++)
                        {
                            global_var->smf_filelabel[i-3] = fileinfo->label[i];
                        }

                        global_var->smf_filelabel[i-3] = '\0';
                    }
                    else
                    {
                        sprintf(global_var->smf_filelabel,"%s",fileinfo->label);
                    }

                    returnBoolean = PGS_TRUE;
                }
                else
                {
                    /*
                     * Token exceed the maximum or less than the minimum required characters.
                     */
                    global_var->smf_errmsg_no = 51; 
                }
            }
            else
            { 
                /*
                 * No token is defined. Proper usage: %LABEL = label.
                 * Note that space characters must be in front and back of "=" character.
                 */
                global_var->smf_errmsg_no = 36;
            }
        }
        else
        { 
            /*
             * No token is defined. Proper usage: %LABEL = label.
             * Note that space characters must be in front and back of "=" character.
             */
            global_var->smf_errmsg_no = 36;
        }
    }
    else
    {
        /*
         * %LABEL not specify.
         */
        global_var->smf_errmsg_no = 35;
    }


    return(returnBoolean);

} /* end PGS_SMF_GetLabel */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_GetSeed

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_GetSeed(
        char             *linebuf,
        PGS_SMF_FileInfo *fileinfo);

FORTRAN:
    NONE

DESCRIPTION:
    To extract %SEED.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string

OUTPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        buffer for saving file information

RETURNS:
    PGS_TRUE - %SEED found and valid
    PGS_FALSE - %SEED not found or invalid

EXAMPLES:
    NONE

NOTES:          
    Seed value of 0 is meant for internal used.

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_GetSeed(                 /* Extract %SEED */
    char             *linebuf,   /* Input  line string */
    PGS_SMF_FileInfo *fileinfo)  /* Output file information */
{        
    PGSt_SMF_boolean  returnBoolean = PGS_FALSE;   /* return boolean */
    char             *token;                       /* string token */
    char              str[PGS_SMF_MAX_MSG_SIZE];   /* string buffer */
    char              seed[50];                    /* seed */
    PgsSmfGlbVar     *global_var;                  /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    strcpy(str,linebuf);
    token = strtok(str,tokensep_space);
    if ((token != (char *)NULL) && (strncmp(token,"%SEED",5) == 0))
    {
        token = strtok((char *)NULL,tokensep_space);
        if ((token != (char *)NULL) && (strcmp(token,"=") == 0))
        {
            token = strtok((char *)NULL,tokensep_space);
            if (token != (char *)NULL)
            {
                strcpy(seed,token);
                token = strtok((char *)NULL,tokensep_space);

                if (token != (char *)NULL)
                {
                    /*
                     * More than one tokens are defined.
                     */
                    global_var->smf_errmsg_no = 42; 
                }
                else if (PGS_SMF_ValidSeed(seed) == PGS_FALSE)
                {
                    /*
                     * Invalid seed number.
                     */
                    global_var->smf_errmsg_no = 43; 
                }
                else
                {
                    fileinfo->seed = (PGSt_SMF_code) atol(seed);
                    returnBoolean = PGS_TRUE;
                }
            }
            else
            {                
                /*
                 * No token is defined. Proper usage: %SEED = seed_number
                 * Note that space characters must be in front and back of "=" character.
                 */
                global_var->smf_errmsg_no = 41;
            }
        }
        else
        {                
            /*
             * No token is defined. Proper usage: %SEED = seed_number
             * Note that space characters must be in front and back of "=" character.
             */
            global_var->smf_errmsg_no = 41;
        }
    }
    else
    {
        /*
         * %SEED not specify.
         */
        global_var->smf_errmsg_no = 40;
    }

    return(returnBoolean);

} /* end PGS_SMF_GetSeed */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_CapitalLetter

SYNOPSIS:       
C:  
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_CapitalLetter(
        char *str);

FORTRAN:
    NONE

DESCRIPTION:
    To determine if the string characters are UPPER-CASE letters.

INPUTS:
    Name            Description                                Units        Min        Max   

    string          line string

OUTPUTS:     
    Name            Description                                Units        Min        Max             

RETURNS:
    PGS_TRUE - all UPPER-CASE letters
    PGS_FALSE - not all UPPER-CASE letters

EXAMPLES:
    NONE

NOTES:   
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CapitalLetter(  /* Check UPPER-CASE letters */
    char *str)          /* Input line string */
{
    short            i;                          /* counter */
    PGSt_SMF_boolean returnBoolean = PGS_TRUE;   /* return boolean */

    for (i=0 ; i < (short)strlen(str) ; i++)
    {
        if (isupper(CTOI(str[i])) == 0)
        {
            returnBoolean = PGS_FALSE;
            break;
        }
    }
    return(returnBoolean);

} /* end PGS_SMF_CapitalLetter */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_ValidSeed

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_ValidSeed(
        char *seed);

FORTRAN:
    NONE

DESCRIPTION:
    To determine if the seed string is a valid seed value.

INPUTS:
    Name            Description                                Units        Min        Max   

    seed            seed string

OUTPUTS:   
    Name            Description                                Units        Min        Max           

RETURNS:
    PGS_TRUE - seed value within allowable limit
    PGS_FALSE - seed value not within allowable limit

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_ValidSeed(  /* Check valid seed value */
    char *seed)     /* Input  seed string */
{
    short             i;                         /* counter */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;  /* return boolean */
    PgsSmfGlbVar     *global_var;                /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    for (i=0 ; i < (short)strlen(seed) ; i++)
    {
        if ((seed[i] < '0') || (seed[i] > '9'))
        {
            returnBoolean = PGS_FALSE;
            break;
        }
    }
    if (returnBoolean == PGS_TRUE)
    {
        if ((PGSt_SMF_code) atol(seed) > PGS_SMF_MAX_SEED_NO)
        {
            returnBoolean = PGS_FALSE;
        }
    }

    return(returnBoolean);

} /* end PGS_SMF_ValidSeed */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ValidMnemonic

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_ValidMnemonic(
        char *linebuf,
        char *label,
        char *mnemonic);

FORTRAN:
    NONE

DESCRIPTION:
    To determine if the mnemonic/action labels follow SMF rules.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string
    label           label 

OUTPUTS:
    Name            Description                                Units        Min        Max   

    mnemonic        mnemonic string

RETURNS:
    PGS_TRUE - valid mnemonic label
    PGS_FALSE - invalid mnemonic string

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_GetWord()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_ValidMnemonic(  /* Check valid mnemonic string */
    char *linebuf,      /* Input  line string */
    char *label,        /* Input  label */
    char *mnemonic)     /* Output mnemonic string */
{
    short             i;                                /* counter */
    short             j;                                /* counter */    
    char             *token;                            /* token */
    char              buf[PGS_SMF_MAX_MSGBUF_SIZE];     /* line buffer */
    char              word[PGS_SMF_MAX_MSG_SIZE];       /* word */
    char              level[5];                         /* level */
    PGSt_SMF_boolean  returnBoolean = PGS_FALSE;        /* return boolean */
    PGSt_SMF_boolean  valid_lev = PGS_FALSE;            /* valid level flag */
    PgsSmfGlbVar     *global_var;                       /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    /*
     * Extract the mnemonic from the linebuf.
     */
    strcpy(buf,linebuf);
    token = strtok(buf,tokensep_space);
    strcpy(global_var->smf_mnemonic,token);

    global_var->smf_errmsg_no = 0;
    mnemonic[0] = '\0';

    i = strlen(label); 
    if ((short)strlen(linebuf) < (i + (short)strlen(PGS_SMF_STAT_LEV_SH)))
    {
        /*
         * No level is defined.
         */
        global_var->smf_errmsg_no = 28;
        returnBoolean = PGS_FALSE; 
	goto DONE;
    }   

    if (strncmp(linebuf,label,i) != 0)
    {
	/*
	 * Mnemonic label does not agree with the %LABEL.
	 */
	global_var->smf_errmsg_no = 27;
	returnBoolean = PGS_FALSE;
	goto DONE;
    }

    j = i;
    
    /*
     * Check if it is a shell defined level.
     */
    level[0] = linebuf[i]; i++; /* _SH_ */
    level[1] = linebuf[i]; i++;
    level[2] = linebuf[i]; i++;
    level[3] = linebuf[i]; i++;
    level[4] = '\0';
    
    if (strncmp(level,PGS_SMF_STAT_LEV_SH,PGS_SMF_LEV_DISPLAY+1) == 0)
    {
	valid_lev = PGS_TRUE;
    }
    else
    {
	i = j;
	
	level[0] = linebuf[i]; i++; /* _?_ */
	level[1] = linebuf[i]; i++;
	level[2] = linebuf[i]; i++;
	level[3] = '\0';
	
	if ((strncmp(level,PGS_SMF_STAT_LEV_M,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_U,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_S,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_N,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_W,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_F,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_E,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_A,PGS_SMF_LEV_DISPLAY)==0)
	    || (strncmp(level,PGS_SMF_STAT_LEV_C,PGS_SMF_LEV_DISPLAY)==0))
	{
	    valid_lev = PGS_TRUE;
	}
    }
    
    if (valid_lev != PGS_TRUE)
    {
	/*
	 * Unknown level.
	 */ 
	global_var->smf_errmsg_no = 22;  
	returnBoolean = PGS_FALSE;
	goto DONE;
    }

    /*
     * Get the rest of the mnemonic label; valid chars are _,A-Z,0-9.
     */
    if (PGS_SMF_GetWord(linebuf,word,&i) != PGS_TRUE)
    {
	/*
	 * Mnemonic label is not fully defined.
	 */
	global_var->smf_errmsg_no = 24;
	returnBoolean = PGS_FALSE;
	goto DONE;
    }
    
    /*
     * Make sure that the mnemonic label does not exceed the maximum characters.
     */
    if ((strlen(label) + strlen(level) + strlen(word)) > (size_t)(PGS_SMF_MAX_MNEMONIC_SIZE - 1))
    {
	global_var->smf_errmsg_no = 50;  
	returnBoolean = PGS_FALSE;         
	goto DONE;
    }
    
    sprintf(mnemonic,"%s%s%s",label,level,word);
    returnBoolean = PGS_TRUE; 
    for (i=0 ; i < (short)strlen(word) ; i++)
    {
	if (word[i] == '_') 
	{
	    if (i > 0)
	    {
		if (word[i-1] == '_')
		{
		    /*
		     * Cannot have 2 under-score simultaneously.
		     */
		    global_var->smf_errmsg_no = 25;
		    returnBoolean = PGS_FALSE;  
		    break;
		}
	    }
	    
	    continue;
	}
	else if ((isupper(CTOI(word[i])) != 0) || (isdigit(CTOI(word[i])) != 0))
	{
	    continue;
	}
	else
	{
	    if ((word[i] >= 'a') && (word[i] <= 'z'))
	    {
		/*
		 * Lower case letter.
		 */ 
		global_var->smf_errmsg_no = 23;
	    }
	    else 
	    {
		/*
		 * Invalid mnemonic characters.
		 */ 
		global_var->smf_errmsg_no = 26; 
	    }
	    
	    returnBoolean = PGS_FALSE;
	    break;
	}
    }                    

DONE:
    return(returnBoolean);

} /* end PGS_SMF_ValidMnemonic */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ValidMsgStr

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_ValidMsgStr(
        char *linebuf,
        char *mnemonic,
        char *msg,
        char *action,
        char *label);

FORTRAN:
    NONE

DESCRIPTION:
    To determine if the message/action string follows SMF rules.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string
    label           label

OUTPUTS:
    Name            Description                                Units        Min        Max   

    mnemonic        mnemonic string
    msg             message string
    action          action string

RETURNS:
    PGS_TRUE  - valid message string and embeded action label
    PGS_FALSE - invalid message string or embeded action label

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_ExtractMsg()
    PGS_SMF_ScanActionSymbol()
    PGS_SMF_GetWord()
    PGS_SMF_RemoveSpecialChar()
    PGS_SMF_ValidMnemonic()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_ValidMsgStr(     /* Check valid message string */
    char *linebuf,       /* Input  line string */
    char *mnemonic,      /* Output mnemonic string */
    char *msg,           /* Output message string */
    char *action,        /* Output action string */
    char *label)         /* Input  label */
{
    char             *token;                             /* string token */
    char             *msgptr;                            /* message pointer */
    char              word[PGS_SMF_MAX_MSG_SIZE];        /* word buffer */
    char              word1[PGS_SMF_MAX_MSG_SIZE];       /* word buffer */
    char              buf[PGS_SMF_MAX_MSGBUF_SIZE];      /* string buffer */
    PGSt_SMF_boolean  returnBoolean = PGS_FALSE;         /* return boolean */
    short             j;                                 /* counter */
    PgsSmfGlbVar     *global_var;                        /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);
    global_var->smf_errmsg_no = 0;

    action[0] = '\0';
    msg[0]    = '\0';

    strcpy(buf,linebuf);
    if ((token = strtok(buf,tokensep_space)) != (char *)NULL)
    {
        /*
         * First token is always a mnemonic.
         */
        strcpy(global_var->smf_mnemonic,token);
        strcpy(mnemonic,token);
        PGS_SMF_ExtractMsg(linebuf,mnemonic,&msgptr);

        if (msgptr == (char *)NULL)
        {
            /*
             * No corresponding message_str is defined for this mnemonic_label. 
             */
            global_var->smf_errmsg_no = 29;
            goto DONE;
        }

        j = PGS_SMF_ScanActionSymbol(msgptr);
        if (j == -1)
        {
            /*
             * Only message string found. 
             */
            if ((short)strlen(msgptr) < PGS_SMF_MAX_MSG_SIZE)
            {
                strcpy(msg,msgptr);
                returnBoolean = PGS_TRUE;
            }
            else
            {
                /*
                 * Message string exceed maximum characters.
                 */
                global_var->smf_errmsg_no = 46;
            }
        }
        else
        {
            /*
             * Then this line contains message_str and action_label. 
             */
            strncpy(msg,msgptr,j);
            msg[j] = '\0';
            PGS_SMF_RemoveFrontEndSpace(msg);

            /*
             * After calling PGS_SMF_RemoveFrontEndSpace(msg) we 
	     * expect action_label must have the special character together; 
	     * eg. '::PGS_A_ACTION', '::PGS_C_MSS_ACTION. 
             * By BNF rules (BNF #9) user should not have other word(s) after 
             * '::PGS_A_ACTION' (eg. ::PGS_A_ACTION word1 word2 etc.). 
             */
            PGS_SMF_GetWord(msgptr,word,&j);
            if (PGS_SMF_GetWord(msgptr,word1,&j) == PGS_TRUE)
            {
                /*
                 * Violate BNF#9 rule.        
                 */
                strcpy(global_var->smf_action,word);
                global_var->smf_errmsg_no = 49;
            }
            else
            {
                /*
                 * Remove ':: ' characters.
                 */
                PGS_SMF_RemoveSpecialChar(word);

                /*
                 * Check if action_label is a valid mnemonic.
                 */
                if (PGS_SMF_ValidMnemonic(word,label,action) == PGS_TRUE)
                {
                    j = strlen(label);
                    word1[0] = word[j];
                    word1[1] = word[j+1];
                    word1[2] = word[j+2];
                    word1[3] = '\0';

                    if ((strncmp(word1,PGS_SMF_STAT_LEV_A,
				 PGS_SMF_LEV_DISPLAY)== 0)
                     || (strncmp(word1,PGS_SMF_STAT_LEV_C,
				 PGS_SMF_LEV_DISPLAY)== 0)) 
                    {
                        if ((short)strlen(action) <= (PGS_SMF_MAX_MNEMONIC_SIZE-1))
                        {
                            j = strlen(msg) - strlen(action);
                            if (j <= (PGS_SMF_MAX_MSG_SIZE-1))
                            {
                                returnBoolean = PGS_TRUE;
                            }
                            else
                            {
                                /*
                                 * Message string exceeds maximum characters.
                                 */
                                global_var->smf_errmsg_no = 46;  
                            }
                        }
                        else
                        {
                            /* 
                             * Action label exceeds maximum characters.
                             */
                            sprintf(global_var->smf_action,"::%s",action);
                            global_var->smf_errmsg_no = 39; 
                        }
                    }
                    else
                    {
                        /*
                         * Action label does not have level _A_ or _C_.
                         */
                        sprintf(global_var->smf_action,"::%s",action);
                        global_var->smf_errmsg_no = 47; 
                    }
                }
                else
                {
                    /*
                     * Error code is set in PGS_SMF_ValidMnemonic().
                     */
                }
            }
        }
    }
    else
    {
        /*
         * No mnemonic is defined.
         */
        global_var->smf_errmsg_no = 45;
    }

DONE:
    return(returnBoolean);

} /* end PGS_SMF_ValidMsgStr */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_RemoveFrontEndSpace

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_RemoveFrontEndSpace(
        char *linebuf);

FORTRAN:
    NONE

DESCRIPTION:
    To remove all the front and end character space of a line string.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string with front/end space

OUTPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string without front/end space

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_RemoveFrontEndSpace(  /* Remove front and back space */
    char *linebuf)            /* Input/Output line string */
{
    short  i;    /* counter */
    short  j;    /* counter */
    short  l;    /* counter */
    size_t len;  /* string length */


    len = strlen(linebuf);

    if (len != 0)
    {
        /*
         * Search for first character.
         */
        for (i=0 ; i < (short)len ; i++)
        {
            if ((linebuf[i] != ' ') && (linebuf[i] != '\t'))
            {
                break;
            }
        }

        /*
         * Search for last character.
         */
        for (j=(short)len-1 ; j > i ; j--)
        {
            if ((linebuf[j] != ' ') && (linebuf[j] != '\t'))
            {
                break;
            }
        }

        if (i == (short)len)
        {
            /*
             * All space.
             */
            linebuf[0] = '\0';
        }
        else if ((i == 0) && (j == ((short)len-1)))
        {
            /* 
             * No space.
             */
        }
        else if (len == 1)
        {
            /*
             * Only 1 character.
             */
            linebuf[0] = linebuf[i];
            linebuf[1] = '\0';
        }
        else 
        {
            /*
             * Extract characters in between space.
             */
            for (l=0 ; l <= (j-i) ; l++)
            {
                linebuf[l] = linebuf[l+i];
            }

            linebuf[l] = '\0';
        }
    }


} /* end PGS_SMF_RemoveFrontEndSpace */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ScanActionSymbol

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    int 
    PGS_SMF_ScanActionSymbol(
        char *str);

FORTRAN:
    NONE

DESCRIPTION:
    To locate if the special symbol "::" exist in the message string.

INPUTS:
    Name            Description                                Units        Min        Max   

    str             line string

OUTPUTS:  
    Name            Description                                Units        Min        Max              

RETURNS:
    -1 - no symbol found
    else the index of the first char of ::

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static int 
PGS_SMF_ScanActionSymbol(  /* Locate "::" characters */
    char *str)             /* Input line string */
{
    short i;             /* counter */
    short index = -1;    /* index */


    for (i=0 ; i < (short)strlen(str) ; i++)
    {
        if (str[i] == ':')
        {
            if ((i+1) < (short)strlen(str))
            {
                if (str[i+1] == ':')
                {
                    index = i;   
                    break;
                }
            }
        }
    }

    return(index);

} /* end PGS_SMF_ScanActionSymbol */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_Exit

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_Exit(
        short terminate_status);

FORTRAN:
    NONE

DESCRIPTION:
    To terminate the program with output status.

INPUTS:
    Name               Description                             Units        Min        Max   

    terminate_status   termination status

OUTPUTS:   
    Name               Description                             Units        Min        Max             

RETURNS:
    TERMINATE_SUCCESS
    TERMINATE_ERROR

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_Exit(                /* Terminate the program */
    short terminate_status)  /* Input termination status */
{
    PgsSmfGlbVar *global_var;  /* global static buffer */
    char         *msg;         /* message pointer */

    PGS_SMF_CompGetGlbVar(&global_var);

    if (terminate_status == TERMINATE_ERROR)
    {
        msg = PGS_SMF_GetErrMsg(global_var->smf_errmsg_no);

        if (global_var->smf_linecnt > 0)
        {
            PGS_SMF_RemoveCarriageReturn(global_var->smf_linebuf);
            fprintf(stderr,"Line %d: %s\n",global_var->smf_linecnt,global_var->smf_linebuf);
        }
    }
    else
    {
        msg = PGS_SMF_GetErrMsg(14);
    }

    fprintf(stderr,"%s",msg);

    if (global_var->smf_fileC != (char *)NULL) 
    {
        free(global_var->smf_fileC);
    }
    if (global_var->smf_fileF != (char *)NULL) 
    {
        free(global_var->smf_fileF);
    }
    if (global_var->smf_fileAda != (char *)NULL) 
    {
        free(global_var->smf_fileAda);
    }
    if (global_var->smf_fileB != (char *)NULL) 
    {
        free(global_var->smf_fileB);
    }
    if (global_var->fptr != (FILE *)NULL)
    {
        fclose(global_var->fptr);
    }

    exit(terminate_status);

} /* end PGS_SMF_Exit */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ExtractSMFName

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_ExtractSMFName(
        char *orgpath,
        char *filepath,
        char *filename);

FORTRAN:
    NONE

DESCRIPTION:
    To extract file path and name.

INPUTS:
    Name            Description                                Units        Min        Max   

    orgpath         original file path_name

OUTPUTS:
    Name            Description                                Units        Min        Max   

    filepath        file path
    filename        file name

RETURNS:
    PGS_TRUE 
    PGS_FALSE 

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_ExtractSMFName(   /* Extract file name */
    char *orgpath,        /* Input  original file path_name */
    char *filepath,       /* Output file path */
    char *filename)       /* Output file name */
{
    char            *c = (char *)NULL;             /* character pointer */
    short            i;                            /* counter */
    PGSt_SMF_boolean returnBoolean = PGS_TRUE;     /* return boolean */


    if (strlen(orgpath) != 0)
    {
        c = strrchr(orgpath,'/');
        if (c == (char *)NULL)
        {
               filepath[0] = '\0';
               strcpy(filename,orgpath);
        }
        else
        {
            for (i=0 ; i < ((short)strlen(orgpath) - (short)strlen(c)) ; i++)
            {
                filepath[i] = orgpath[i];
            }

            filepath[i] = '\0';

            for (i=0; i < (short)strlen(c) ; i++)
            {
                filename[i] = c[i+1];
            }

            filename[i] = '\0';
        }
    }
    else
    {
        returnBoolean = PGS_FALSE;
    }

    return(returnBoolean);

} /* end PGS_SMF_ExtractSMFName */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CreateHeaderFile

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_CreateHeaderFile(
        PGS_SMF_FileInfo *fileinfo,
        PGS_SMF_MsgData   msgdata[],
        short             cnt,
        char             *filepath,
        char             *filename);

FORTRAN:
    NONE

DESCRIPTION:
    To create #define C header file.

INPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        file information
    msgdata         message buffer
    cnt             number of message in buffer
    filepath        file path

OUTPUTS:
    Name            Description                                Units        Min        Max           

    filename        file name

RETURNS:
    PGS_TRUE 
    PGS_FALSE 

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_StatusLevel()
    PGS_SMF_HexStrCode()
    PGS_SMF_SystemCall()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CreateHeaderFile(              /* Create #define C header file */
    PGS_SMF_FileInfo *fileinfo,        /* Input  file information */
    PGS_SMF_MsgData   msgdata[],       /* Input  message buffer */
    short             cnt,             /* Input  number of message in buffer */
    char             *filepath,        /* Input  file path */
    char             *filename)        /* Output file name */
{
    char              str[10];                   /* string buffer */
    char              msg[200];                  /* message buffer */
    FILE             *fptr = (FILE *)NULL;       /* file pointer */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;  /* return boolean */
    short             i;                         /* counter */
    short             k;                         /* counter */
    PGSt_SMF_code     code;                      /* code */
    PGSt_SMF_code     lev;                       /* level */
    PgsSmfGlbVar     *global_var;                /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    /*
     * Extract filename without extension.
     */
    global_var->smf_fileC = (char *) calloc(strlen(filepath)+32,sizeof(char));

    if (strlen(filepath) != 0)
    {
        if (strlen(global_var->smf_filelabel) == 0)
        { 
            sprintf(filename,"%s_%d.h",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
            sprintf(global_var->smf_fileC,"%s/%s",filepath,filename);
        }
        else
        {
            sprintf(filename,"%s_%s_%d.h",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
            sprintf(global_var->smf_fileC,"%s/%s",filepath,filename);
        }
    }
    else
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            sprintf(filename,"%s_%d.h",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
            sprintf(global_var->smf_fileC,"%s",filename);
        }
        else
        { 
            sprintf(filename,"%s_%s_%d.h",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
            sprintf(global_var->smf_fileC,"%s",filename);
        }
    }

    fptr = fopen(global_var->smf_fileC,"w");
    if (fptr == (FILE *)NULL)
    {
        global_var->smf_errmsg_no = 12;
        returnBoolean = PGS_FALSE;
    }
    else
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            fprintf(fptr,"#ifndef _%s_%d_H\n",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
            fprintf(fptr,"#define _%s_%d_H\n",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
        }
        else
        {
            fprintf(fptr,"#ifndef _%s_%s_%d_H\n",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
            fprintf(fptr,"#define _%s_%s_%d_H\n",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
        }

        for (i=0 ; i < cnt ; i++)
        {
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_SH)) 
	      lev = PGS_SMF_MASK_LEV_SH;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_M))  
	      lev = PGS_SMF_MASK_LEV_M;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_U))  
	      lev = PGS_SMF_MASK_LEV_U;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_S))  
	      lev = PGS_SMF_MASK_LEV_S;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_N))  
	      lev = PGS_SMF_MASK_LEV_N;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_W))  
	      lev = PGS_SMF_MASK_LEV_W;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_F))  
	      lev = PGS_SMF_MASK_LEV_F;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_E))  
	      lev = PGS_SMF_MASK_LEV_E;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_A))  
	      lev = PGS_SMF_MASK_LEV_A;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_C))  
	      lev = PGS_SMF_MASK_LEV_C;

            code = (fileinfo->seed << 13) | lev | i;
            PGS_SMF_HexStrCode(code,str);

            sprintf(msg,"%s",msgdata[i].mnemonic);  
            for (k=strlen(msgdata[i].mnemonic) ; k < (PGS_SMF_MAX_MNEMONIC_SIZE + 3) ; k++)
            {
                strcat(msg," ");
            }

            fprintf(fptr,"#define %s %11d /* 0x%sL */\n",msg,code,str);
            msgdata[i].code = code;
        }


        fprintf(fptr,"#endif\n");
        fclose(fptr);
    }    

    return(returnBoolean);

} /* end PGS_SMF_CreateHeaderFile */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CreateAdaFile

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_CreateAdaFile(
        PGS_SMF_FileInfo *fileinfo,
        PGS_SMF_MsgData   msgdata[],
        short             cnt,
        char             *filepath,
        char             *filename);

FORTRAN:
    NONE

DESCRIPTION:
    To create Ada header file.

INPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        file information
    msgdata         message buffer
    cnt             number of message in buffer
    filepath        file path

OUTPUTS:
    Name            Description                                Units        Min        Max           

    filename        file name

RETURNS:
    PGS_TRUE 
    PGS_FALSE 

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_StatusLevel()
    PGS_SMF_HexStrCode()
    PGS_SMF_SystemCall()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CreateAdaFile(                 /* Create Ada header file */
    PGS_SMF_FileInfo *fileinfo,        /* Input  file information */
    PGS_SMF_MsgData   msgdata[],       /* Input  message buffer */
    short             cnt,             /* Input  number of message in buffer */
    char             *filepath,        /* Input  file path */
    char             *filename)        /* Output file name */
{
    char              str[10];                   /* string buffer */
    char              msg[200];                  /* message buffer */
    FILE             *fptr = (FILE *)NULL;       /* file pointer */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;  /* return boolean */
    short             i;                         /* counter */
    short             k;                         /* counter */
    PGSt_SMF_code     code;                      /* code */
    PGSt_SMF_code     lev;                       /* level */
    PgsSmfGlbVar     *global_var;                /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    /*
     * Extract filename without extension.
     */
    global_var->smf_fileAda = (char *) calloc(strlen(filepath)+32,sizeof(char));

    if (strlen(filepath) != 0)
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            sprintf(filename,"%s_%d.a",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
            sprintf(global_var->smf_fileAda,"%s/%s",filepath,filename);
        }
        else
        {
            sprintf(filename,"%s_%s_%d.a",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
            sprintf(global_var->smf_fileAda,"%s/%s",filepath,filename);
        }
    }
    else
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            sprintf(filename,"%s_%d.a",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
            sprintf(global_var->smf_fileAda,"%s",filename);
        }
        else
        { 
            sprintf(filename,"%s_%s_%d.a",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
            sprintf(global_var->smf_fileAda,"%s",filename);
        }
    }

    fptr = fopen(global_var->smf_fileAda,"w");
    if (fptr == (FILE *)NULL)
    {
        global_var->smf_errmsg_no = 18;
        returnBoolean = PGS_FALSE;
    }
    else
    {   
        fprintf(fptr,"with %s_SMF; use %s_SMF;\n",PGS_SMF_SYS_FILEPREFIX,PGS_SMF_SYS_FILEPREFIX);

        if (strlen(global_var->smf_filelabel) == 0)
        {
            fprintf(fptr,"package %s_%d is\n",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);           
        }
        else
        {
            fprintf(fptr,"package %s_%s_%d is\n",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);            
        }

        fprintf(fptr,"\n");

        for (i=0 ; i < cnt ; i++)
        {
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_SH)) 
	      lev = PGS_SMF_MASK_LEV_SH;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_M))  
	      lev = PGS_SMF_MASK_LEV_M;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_U))  
	      lev = PGS_SMF_MASK_LEV_U;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_S))  
	      lev = PGS_SMF_MASK_LEV_S;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_N))  
	      lev = PGS_SMF_MASK_LEV_N;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_W))  
	      lev = PGS_SMF_MASK_LEV_W;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_F))  
	      lev = PGS_SMF_MASK_LEV_F;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_E))  
	      lev = PGS_SMF_MASK_LEV_E;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_A))  
	      lev = PGS_SMF_MASK_LEV_A;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_C))  
	      lev = PGS_SMF_MASK_LEV_C;

            code = (fileinfo->seed << 13) | lev | i;
            PGS_SMF_HexStrCode(code,str);

            sprintf(msg,"%s",msgdata[i].mnemonic);  
            for (k=strlen(msgdata[i].mnemonic) ; k < (PGS_SMF_MAX_MNEMONIC_SIZE + 3) ; k++)
            {
                strcat(msg," ");
            }

            fprintf(fptr,"    %s: constant PGSt_SMF_status :=    %11d; -- 0x%sL\n",msg,code,str);
            msgdata[i].code = code;
        }


        fprintf(fptr,"\n"); 

        if (strlen(global_var->smf_filelabel) == 0)
        {
            fprintf(fptr,"end %s_%d;\n",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);           
        }
        else
        {
            fprintf(fptr,"end %s_%s_%d;\n",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);            
        }

        fclose(fptr);
    }

    return(returnBoolean);

} /* end PGS_SMF_CreateAdaFile */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CreateFortranFile

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_CreateFortranFile(
        PGS_SMF_FileInfo *fileinfo,
        PGS_SMF_MsgData   msgdata[],
        short             cnt,
        char             *filepath,
        char             *filename);

FORTRAN:
    NONE

DESCRIPTION:
    To create Fortran INCLUDE file.

INPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        file information
    msgdata         message buffer
    cnt             number of message in buffer
    filepath        file path

OUTPUTS:
    Name            Description                                Units        Min        Max           

    filename        file name

RETURNS:
    PGS_TRUE 
    PGS_FALSE 

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_StatusLevel()
    PGS_SMF_SystemCall()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CreateFortranFile(              /* Create Fortran INCLUDE file */
    PGS_SMF_FileInfo *fileinfo,         /* Input  file information */ 
    PGS_SMF_MsgData   msgdata[],        /* Input  message buffer */
    short             cnt,              /* Input  number of message in buffer */
    char             *filepath,         /* Input  file path */
    char             *filename)         /* Output file name */
{       
    FILE             *fptr = (FILE *)NULL;       /* file pointer */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;  /* return boolean */
    short             i;                         /* counter */
    PGSt_SMF_code     code;                      /* code */
    PGSt_SMF_code     lev;                       /* level */
    PgsSmfGlbVar     *global_var;                /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    /*
     * Extract filename without extension. 
     */
    global_var->smf_fileF = (char *) calloc(strlen(filepath)+32,sizeof(char));

    if (strlen(filepath) != 0)
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            sprintf(filename,"%s_%d.f",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
            sprintf(global_var->smf_fileF,"%s/%s",filepath,filename);
        }
        else
        {
            sprintf(filename,"%s_%s_%d.f",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
            sprintf(global_var->smf_fileF,"%s/%s",filepath,filename);
        }
    }
    else
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            sprintf(filename,"%s_%d.f",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
            sprintf(global_var->smf_fileF,"%s",filename);
        }
        else
        {
            sprintf(filename,"%s_%s_%d.f",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
            sprintf(global_var->smf_fileF,"%s",filename);
        }
    }

    fptr = fopen(global_var->smf_fileF,"w");
    if (fptr == (FILE *)NULL)
    {
        global_var->smf_errmsg_no = 17;
        returnBoolean = PGS_FALSE;
    }
    else
    {
        for (i=0 ; i < cnt ; i++)
        {
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_SH)) 
	      lev = PGS_SMF_MASK_LEV_SH;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_M))  
	      lev = PGS_SMF_MASK_LEV_M;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_U))  
	      lev = PGS_SMF_MASK_LEV_U;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_S))  
	      lev = PGS_SMF_MASK_LEV_S;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_N))  
	      lev = PGS_SMF_MASK_LEV_N;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_W))  
	      lev = PGS_SMF_MASK_LEV_W;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_F))  
	      lev = PGS_SMF_MASK_LEV_F;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_E))  
	      lev = PGS_SMF_MASK_LEV_E;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_A))  
	      lev = PGS_SMF_MASK_LEV_A;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_C))  
	      lev = PGS_SMF_MASK_LEV_C;

            code = (fileinfo->seed << 13) | lev | i;
            msgdata[i].code = code;
            fprintf(fptr,"        INTEGER %s\n",msgdata[i].mnemonic);
            fprintf(fptr,"        PARAMETER (%s = %d)\n",msgdata[i].mnemonic,code);
        }

        fclose(fptr);
    }

    return(returnBoolean);

} /* end PGS_SMF_CreateFortranFile */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CreateFortranSubFile

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_CreateFortranSubFile(
        PGS_SMF_FileInfo *fileinfo,
        PGS_SMF_MsgData   msgdata[],
        short             cnt,
        char             *filepath,
        char             *filename);

FORTRAN:
    NONE

DESCRIPTION:
    To create Fortran subroutine file.

INPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        file information
    msgdata         message buffer
    cnt             number of message in buffer
    filepath        file path
    filename        file name

OUTPUTS:
    Name            Description                                Units        Min        Max           

RETURNS:
    PGS_TRUE 
    PGS_FALSE 

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_StatusLevel()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CreateFortranSubFile(              /* Create Fortran subroutine file */
    PGS_SMF_FileInfo *fileinfo,            /* Input file information */
    PGS_SMF_MsgData   msgdata[],           /* Input message buffer */
    short             cnt,                 /* Input number of message in buffer */
    char             *filepath,            /* Input file path */
    char             *filename)            /* Input file name */
{    
    FILE             *fptr = (FILE *)NULL;       /* file pointer */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;  /* return boolean */
    short             i;                         /* counter */
    PGSt_SMF_code     code;                      /* code */
    PGSt_SMF_code     lev;                       /* level */
    PgsSmfGlbVar     *global_var;                /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    /*
     * Extract filename without extension. 
     */
    global_var->smf_fileC = (char *) calloc(strlen(filepath)+32,sizeof(char));

    if (strlen(filepath) != 0)
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            sprintf(global_var->smf_fileC,"%s/%s_%d.f",filepath,PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
        }
        else
        {
            sprintf(global_var->smf_fileC,"%s/%s_%s_%d.f",filepath,PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
        }
    }
    else
    {
        if (strlen(global_var->smf_filelabel) == 0)
        {
            sprintf(global_var->smf_fileC,"%s_%d.f",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
        }
        else
        {
            sprintf(global_var->smf_fileC,"%s_%s_%d.f",PGS_SMF_SYS_FILEPREFIX,global_var->smf_filelabel,fileinfo->seed);
        }
    }

    fptr = fopen(global_var->smf_fileC,"w");
    if (fptr == (FILE *)NULL)
    {
        global_var->smf_errmsg_no = 17;
        returnBoolean = PGS_FALSE;
    }
    else
    {
        fprintf(fptr,"      SUBROUTINE %s_%s%d(mnemonic_str,code)\n",fileinfo->label,PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
        fprintf(fptr,"      CHARACTER*(*) mnemonic_str\n");
        fprintf(fptr,"      INTEGER       code\n\n");
        fprintf(fptr,"      INTEGER       next,ptr1,ptr2,length\n");
        fprintf(fptr,"      CHARACTER*31  mnemonic\n\n");

        fprintf(fptr,"      code   = 0\n");
        fprintf(fptr,"      next   = 1\n");
        fprintf(fptr,"      ptr1   = 1\n");
        fprintf(fptr,"      ptr2   = 1\n");
        fprintf(fptr,"      length = LEN(mnemonic_str)\n\n");

        fprintf(fptr,"      DO 10 next = 1, length\n");
        fprintf(fptr,"              IF (mnemonic_str(next : next) .EQ. ' ') THEN\n");
        fprintf(fptr,"                      IF (ptr2 .EQ. 1) THEN\n");
        fprintf(fptr,"                              ptr1 = next + 1\n");
        fprintf(fptr,"                      ELSE\n");
        fprintf(fptr,"                              GOTO 20\n");
        fprintf(fptr,"                      ENDIF\n");
        fprintf(fptr,"              ELSE\n");
        fprintf(fptr,"                      ptr2 = next\n");
        fprintf(fptr,"              ENDIF\n");
        fprintf(fptr,"10    CONTINUE\n");
        fprintf(fptr,"20    CONTINUE\n\n");

        fprintf(fptr,"      IF ((ptr2 .GE. ptr1) .AND. (length .NE. 0)) THEN\n");
        fprintf(fptr,"              mnemonic = mnemonic_str(ptr1 : ptr2)\n\n");

        for (i=0 ; i < cnt ; i++)
        {
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_SH)) 
	      lev = PGS_SMF_MASK_LEV_SH;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_M))  
	      lev = PGS_SMF_MASK_LEV_M;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_U))  
	      lev = PGS_SMF_MASK_LEV_U;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_S))  
	      lev = PGS_SMF_MASK_LEV_S;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_N))  
	      lev = PGS_SMF_MASK_LEV_N;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_W))  
	      lev = PGS_SMF_MASK_LEV_W;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_F))  
	      lev = PGS_SMF_MASK_LEV_F;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_E))  
	      lev = PGS_SMF_MASK_LEV_E;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_A))  
	      lev = PGS_SMF_MASK_LEV_A;
            if (PGS_SMF_StatusLevel(msgdata[i].mnemonic,PGS_SMF_STAT_LEV_C))  
	      lev = PGS_SMF_MASK_LEV_C;

            code = (fileinfo->seed << 13) | lev | i;
            msgdata[i].code = code;

            if (i == 0)
            {
                fprintf(fptr,"              IF (mnemonic .EQ. '%s') THEN\n",msgdata[i].mnemonic);
            }
            else
            {
                fprintf(fptr,"              ELSE IF (mnemonic .EQ. '%s') THEN\n",msgdata[i].mnemonic);
            }

            fprintf(fptr,"                           code = %d\n",code);
        }

        fprintf(fptr,"              ENDIF\n");
        fprintf(fptr,"      ENDIF\n\n");

        fprintf(fptr,"      RETURN\n");
        fprintf(fptr,"      END\n");
        fclose(fptr);
    }

    return(returnBoolean);

} /* end PGS_SMF_CreateFortranSubFile */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CreateASCIIFile

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_CreateASCIIFile(
        PGS_SMF_FileInfo *fileinfo,
        PGS_SMF_MsgData   msgdata[],
        short             cnt,
        char             *filepath,
        char             *filename);

FORTRAN:
    NONE

DESCRIPTION:
    To create ASCII message file.

INPUTS:
    Name            Description                                Units        Min        Max   

    fileinfo        file information
    msgdata         message buffer
    cnt             number of message in buffer
    filepath        file path

OUTPUTS:
    Name            Description                                Units        Min        Max           

    filename        file name

RETURNS:
    PGS_TRUE 
    PGS_FALSE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CreateASCIIFile(              /* Create ASCII message file */
    PGS_SMF_FileInfo *fileinfo,       /* Input  file information */
    PGS_SMF_MsgData   msgdata[],      /* Input  message buffer */
    short             cnt,            /* Input  number of message in buffer */
    char             *filepath,       /* Input  file path */
    char             *filename)       /* Output file name */
{
    FILE             *fptr = (FILE *)NULL;                 /* file pointer */
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;            /* return boolean */
    short             i;                                   /* counter */
    PgsSmfGlbVar     *global_var;                          /* global static buffer */
    char              action[PGS_SMF_MAX_MNEMONIC_SIZE];   /* action buffer */

    PGS_SMF_CompGetGlbVar(&global_var);        

    sprintf(filename,"%s_%d",PGS_SMF_SYS_FILEPREFIX,fileinfo->seed);
    global_var->smf_fileB = (char *) calloc(strlen(filepath)+32,sizeof(char));        
    if (strlen(filepath) == 0)
    {
        sprintf(global_var->smf_fileB,"%s",filename);
    }
    else
    {
        sprintf(global_var->smf_fileB,"%s/%s",filepath,filename);
    }

    fptr = fopen(global_var->smf_fileB,"w");
    if (fptr == (FILE *)NULL)
    {
        global_var->smf_errmsg_no = 13;
        returnBoolean = PGS_FALSE;
    }
    else
    {
        /*
         * Write file info. 
         */
        fprintf(fptr,"%s,%s,%d\n",fileinfo->instr,fileinfo->label,fileinfo->seed);

        /*
         * Write mnemonics. 
         */
        for (i=0 ; i < cnt ; i++)
        {
            if (strlen(msgdata[i].action) == 0)
            {
                strcpy(action,PGS_NULL_STR);
            }
            else
            {
                strcpy(action,msgdata[i].action);
            }

            fprintf(fptr,"%d,%s,%s,%s\n",msgdata[i].code,msgdata[i].mnemonic,action,msgdata[i].msg);
        }

        fclose(fptr);
    }   

    return(returnBoolean);

} /* end PGS_SMF_CreateASCIIFile */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CorrectNumActMnemonic

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

     PGSt_SMF_boolean 
     PGS_SMF_CorrectNumActMnemonic(
         PGS_SMF_MsgData msgdata[],
         short           cnt);

FORTRAN:
    NONE

DESCRIPTION:
    To make sure that the each message string that has an associate action_label
    attached to it, exists in the SMF file.

INPUTS:
    Name            Description                                Units        Min        Max   

    msgdata         message buffer
    cnt             number of message in buffer

OUTPUTS:
    Name            Description                                Units        Min        Max           

RETURNS:
    PGS_TRUE 
    PGS_FALSE 

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_ScanWord()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CorrectNumActMnemonic(    /* Check correct number of action mnemonics */
    PGS_SMF_MsgData msgdata[],    /* Input message buffer */
    short           cnt)          /* Input number of message in buffer */
{       
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;            /* return boolean */
    short             i;                                   /* counter */
    short             j;                                   /* counter */
    short             found;                               /* flag to indicate found */
    PgsSmfGlbVar     *global_var;                          /* global static buffer */
    char              action[100];                         /* action buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    for (i=0 ; i < cnt ; i++)
    {

        if (msgdata[i].action[0] != '\0')
        {            
            /*            
             * Check if the mnemonic is an action mnemonic.
             */
            if (((PGS_SMF_StatusLevel(msgdata[i].mnemonic,
		  PGS_SMF_STAT_LEV_A) == PGS_TRUE)
	     ||  (PGS_SMF_StatusLevel(msgdata[i].mnemonic,
		  PGS_SMF_STAT_LEV_C))) == PGS_TRUE)
            {
                /*
                 * Action mnemonic cannot have another action mnemonic defined 
		 * in the message string.
                 */
                PGS_SMF_ScanWord(msgdata[i].action);
                strcpy(global_var->smf_mnemonic,msgdata[i].action);
                global_var->smf_errmsg_no = 48;
                returnBoolean = PGS_FALSE;
                break;
            }                        
            else
            {
                /*
                 * Check if the mnemonic label that associates an action label 
		 * does exist in the file.
                 */
                found = 0;
                for (j=0 ; j < cnt ; j++)
                {
                    if (strcmp(msgdata[j].mnemonic,msgdata[i].action) == 0)
                    {
                        found = 1;
                        break;
                    }
                }

                if (found == 0)
                {
                    /*
                     * No action label is defined for a given mnemonic label. 
                     */
                    sprintf(action,"::%s",msgdata[i].action);
                    PGS_SMF_ScanWord(action);

                    sprintf(global_var->smf_action,"%s",msgdata[i].action);
                    global_var->smf_errmsg_no = 15;
                    returnBoolean = PGS_FALSE;
                    break;
                }
            }
        }
    }

    return(returnBoolean);

} /* end PGS_SMF_CorrectNumActMnemonic */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_CommentLine

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_CommentLine(
        char *linebuf);

FORTRAN:
    NONE

DESCRIPTION:
    To check if the line starts with '#' character.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string

OUTPUTS:      
    Name            Description                                Units        Min        Max   

RETURNS:
    PGS_TRUE - comment line
    PGS_FALSE - non comment line

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_CommentLine(   /* Check comment line */
    char *linebuf)     /* Input line string */
{
    short            i;                         /* counter */
    PGSt_SMF_boolean returnBoolean = PGS_TRUE;  /* return boolean */

    for (i=0 ; i < (short)strlen(linebuf) ; i++)
    {
        if ((linebuf[i] != ' ')  && 
            (linebuf[i] != '\t') && 
            (linebuf[i] != '\n'))
        {
            if (linebuf[i] != '#')
            {
                returnBoolean = PGS_FALSE;
            }
            break;
        }
    }
    return(returnBoolean);

} /* end PGS_SMF_CommentLine */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_HexStrCode

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_HexStrCode(
        PGSt_SMF_code code,
        char    *str);

FORTRAN:
    NONE

DESCRIPTION:
    To convert the code value to string in hexadecimal format.

INPUTS:
    Name            Description                                Units        Min        Max   

    code            code value

OUTPUTS:
    Name            Description                                Units        Min        Max   

    str             hexadecimal string code

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_HexStrCode(   /* Convert code to string format */
    PGSt_SMF_code code,    /* Input  code value */
    char    *str)     /* Output hexadecimal string code */
{
    char     hex[10];       /* string buffer */
    char     long_hex[10];  /* string buffer */
    short    i;             /* counter */
    short    j;             /* counter */


    sprintf(hex,"%x",code);
    if ((short)strlen(hex) < 8)
    {
        j = 0;
        for (i=strlen(hex) ; i < 8 ; i++)
        {
            long_hex[j] = '0';
            long_hex[j+1] = '\0';
            j++;
        }
        strcat(long_hex,hex);
        long_hex[8] = '\0';
    }
    else
    {
        sprintf(long_hex,"%s",hex);
    }

    strcpy(str,long_hex);

} /* end PGS_SMF_HexStrCode */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_HexChar

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_HexChar(
        PGSt_SMF_code byte_val,
        char    *hexchar);

FORTRAN:
    NONE

DESCRIPTION:
    To convert a byte value to hexadecimal character.

INPUTS:
    Name            Description                                Units        Min        Max   

    byte_val        byte constant

OUTPUTS:
    Name            Description                                Units        Min        Max   

    hexchar         hex character

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_HexChar(        /* Convert byte value to character */
    PGSt_SMF_code byte_val,  /* Input  byte constant */ 
    char    *hexchar)   /* Output hex character */
{

    if      (byte_val <  10)   sprintf(hexchar,"%d",byte_val);
    else if (byte_val == 10)   strcpy(hexchar,"a");
    else if (byte_val == 11)   strcpy(hexchar,"b");
    else if (byte_val == 12)   strcpy(hexchar,"c");
    else if (byte_val == 13)   strcpy(hexchar,"d");
    else if (byte_val == 14)   strcpy(hexchar,"e");
    else if (byte_val == 15)   strcpy(hexchar,"f");


} /* end PGS_SMF_HexChar */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_GetWord

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_GetWord(
        char  *linebuf,
        char  *word,
        short *index);

FORTRAN:
    NONE

DESCRIPTION:
    To extract a word out from the string pointed by an index.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string
    index           pointer to the start of the character for extraction

OUTPUTS:
    Name            Description                                Units        Min        Max   

    word            word extracted
    index           pointer to next character of the line

RETURNS:
    PGS_TRUE - word extracted 
    PGS_FALSE - no word extracted

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_GetWord(        /* Extract word from string */
    char  *linebuf,     /* Input        line string */
    char  *word,        /* Output       word extracted */
    short *index)       /* Output/Input pointer to character */
{
    PGSt_SMF_status  returnBoolean = PGS_FALSE;   /* return boolean */
    short            i;                           /* counter */
    short            j;                           /* counter */
    short            first_char = -1;             /* string index */
    short            last_char  = -1;             /* string index */


    for (i = *index ; i < (short)strlen(linebuf) ; i++)
    {
        if (first_char == -1)
        {
            if ((linebuf[i] != ' ')  && 
                (linebuf[i] != '\t') &&
                (linebuf[i] != '\n'))
            {
                first_char = i;
            }
        }
        else
        {
            if ((linebuf[i] == ' ')  || 
                (linebuf[i] == '\t') ||
                (linebuf[i] == '\n'))
            {
                last_char = i - 1;
                break;
            }
        }
    }


    if (first_char == -1)
    {
        word[0] = '\0';
    }
    else
    {
        if (first_char == last_char)
        {
            word[0] = linebuf[first_char];
            word[1] = '\0';
        }
        else
        {
            if (last_char == -1)
            {
                last_char = strlen(linebuf) - 1;
            }

            j=0;
            for (i=first_char ; i <= last_char ; i++)
            {
                word[j] = linebuf[i];
                j++;
            }
            word[j] = '\0';
        }

        *index = i;
        returnBoolean = PGS_TRUE;
    }

    return(returnBoolean);

} /* end PGS_SMF_GetWord */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_RemoveCarriageReturn

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_RemoveCarriageReturn(
        char *linebuf);

FORTRAN:
    NONE

DESCRIPTION:
    To remove '\n' out from the string.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string with '\n'

OUTPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string without '\n'

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_RemoveCarriageReturn(  /* Remove '\n' from string */
    char *linebuf)             /* Input/Output line string */
{
    short len = strlen(linebuf);  /* string length */

    if ((len != 0) && (linebuf[len-1] == '\n'))
    {
        linebuf[len-1] = '\0';
    }

} /* end PGS_SMF_RemoveCarriageReturn */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_StatDefOnSameLine

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_StatDefOnSameLine(
        char *label);

FORTRAN:
    NONE

DESCRIPTION:
    To assemble mnemonic, message string and action label on one line.

INPUTS:
    Name            Description                                Units        Min        Max   

    label           label string

OUTPUTS:   
    Name            Description                                Units        Min        Max           

RETURNS:
    PGS_TRUE 
    PGS_FALSE 

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_RemoveCarriageReturn()
    PGS_SMF_BlankLine()
    PGS_SMF_CommentLine()
    PGS_SMF_ValidLine()
    PGS_SMF_ValidMnemonic()
    PGS_SMF_RemoveFrontEndSpace()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_StatDefOnSameLine(  /* Assemble text into a line */
    char *label)            /* Input label string */
{      
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;             /* return boolean */
    char              tmp[PGS_SMF_MAX_MSGBUF_SIZE];         /* line buffer */
    char              linebuf[PGS_SMF_MAX_MSGBUF_SIZE];     /* line buffer */
    char              linebufs[PGS_SMF_MAX_MSGBUF_SIZE];    /* line buffer */
    char              mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];  /* mnemonic */    
    PgsSmfGlbVar     *global_var;                           /* global static buffer */
    short             linecnt = 1;                          /* line counter */
/*  short             len; */                               /* length */

    fpos_t            startpos;           /* file start position */
/*  int               i; */               /* counter */

    PGS_SMF_CompGetGlbVar(&global_var);

    global_var->smf_linebuf[0] = '\0';
    linebuf[0] = '\0';
    memset(linebufs,'\0',sizeof(linebufs));

    fsetpos(global_var->fptr,&global_var->startpos);

    for (;;)
    {
        fgetpos(global_var->fptr, &startpos);

        if (fgets(linebuf,PGS_SMF_MAX_MSGBUF_SIZE,global_var->fptr) == 
	    (char *)NULL)
        {
            if (feof(global_var->fptr) != 0)
            {

                /*
                 * It is so happen that EOF occurs while trying to concatenate 
		 * the message lines.
                 */
                if (linecnt > 1)
                {
                    strcpy(global_var->smf_linebuf,linebufs);
                }
                else
                {
                    /*
                     * There is a possibility that no mnemonic is defined in the
		     * file. If so, Check global_var->smf_linebuf[0] == '\0' for
		     * this case.
                     */
                }                
            }
            else 
            {
                global_var->smf_errno = errno;
                global_var->smf_errmsg_no = 3;
                returnBoolean = PGS_FALSE;
            }

            break;
        }
        else
        {

            global_var->smf_linecnt += 1;
            strcpy(tmp,linebuf); 

            PGS_SMF_RemoveCarriageReturn(linebuf);

            /*
             * Note that a blank, comment or next defined mnemonic label are 
	     * criteria to stop concatenating the lines.
             */
            if (PGS_SMF_BlankLine(linebuf) == PGS_TRUE)   
            {  
                if (linecnt == 1)
                {
                    continue;
                }
                else
                {
                    goto NEXT;
                }
            }

            if (PGS_SMF_CommentLine(linebuf) == PGS_TRUE)
            {   
                if (linecnt == 1)
                {
                    continue;
                }
                else
                {
                    goto NEXT;
                }
            }

            if (PGS_SMF_ValidLine(linebuf) == PGS_FALSE)   
            { 
                global_var->smf_errmsg_no = 4;
                strcpy(global_var->smf_linebuf,tmp);
                returnBoolean = PGS_FALSE;
                break;
            }

            /*
             * Mnemonic always starts on column 1. If not, then we interprete 
	     * it as part of the associated message.
             */
            if (linebuf[0] != ' ')
            {
                if (PGS_SMF_ValidMnemonic(linebuf,label,mnemonic) == PGS_TRUE)
                {
                    if (linecnt == 1)
                    {
                        /*
                         * Current mnemonic retrieved. 
                         */
                        PGS_SMF_RemoveFrontEndSpace(linebuf);
                        sprintf(linebufs,"%s",linebuf); 
                    }
                    else
                    {
NEXT:                    
                        /*
                         * Next mnemonic is defined. So we stop concatenating. 
                         */
                        global_var->smf_linecnt -= 1;
                        strcpy(global_var->smf_linebuf,linebufs);
                        fsetpos(global_var->fptr,&startpos);
			break;
                    }
                }
                else
                {
                    /*
                     * Error number is set in PGS_SMF_ValidMnemonic().
                     */             
                    strcpy(global_var->smf_linebuf,tmp);                                        
                    returnBoolean = PGS_FALSE;
                    break;
                }
            }
            else
            {
                /*
                 * This line is part of the associated message string. We will 
		 * concatenate this line with the previous lines. But before 
		 * concatenating, we need to check that the user has defined 
	         * the required mnemonic.
                 */
                if (linecnt == 1)
                {
                    /*
                     * User has not defined the mnemonic on column 1. User has 
		     * to define mnemonic at column 1 so that SMF can recognize 
		     * the difference between the mnemonic and the associated 
		     * message (multiple lines of message).
                     */
                    strcpy(global_var->smf_linebuf,tmp);
                    global_var->smf_errmsg_no = 44;
                    returnBoolean = PGS_FALSE;
                    break;
                }

                /*
                 * Assemble the status definition. Remove front and end space 
		 * characters before concatenating.
                 */
                PGS_SMF_RemoveFrontEndSpace(linebuf);
                strcat(linebufs," ");
                strcat(linebufs,linebuf);
            }

            linecnt++;
        }
    }

    return(returnBoolean);

} /* end PGS_SMF_StatDefOnSameLine */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_StatusLevel

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean 
    PGS_SMF_StatusLevel(
        char *mnemonic,
        char *stat_lev);

FORTRAN:
    NONE

DESCRIPTION:
    To check if the mnemonic is the specify status level.

INPUTS:
    Name            Description                                Units        Min        Max   

    mnemonic        mnemonic label
    stat_lev        status level

OUTPUTS:   
    Name            Description                                Units        Min        Max   

RETURNS:
    PGS_TRUE
    PGS_FALSE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean 
PGS_SMF_StatusLevel(    /* Verify status level */
    char *mnemonic,     /* Input mnemonic label */
    char *stat_lev)     /* Input status level */
{

    if (strstr(mnemonic,stat_lev) == (char *)NULL)
    {
        return(PGS_FALSE);
    }

    return(PGS_TRUE);

} /* end PGS_SMF_StatusLevel */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_RemoveSpecialChar

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_RemoveSpecialChar(
        char *action_label);

FORTRAN:
    NONE

DESCRIPTION:
    To remove special characters '::' that attached to the action label.

INPUTS:
    Name            Description                                Units        Min        Max   

    action_label    action line string with '::'

OUTPUTS:
    Name            Description                                Units        Min        Max   

    action_label    action line string without '::'

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_RemoveSpecialChar(   /* Remove '::' */
    char *action_label)      /* Input/Output action line string */
{
    short i;  /* counter */
    short j;  /* counter */


    j = 0;
    for (i = 0 ; i < ((short)strlen(action_label)-2) ; i++)
    {
        action_label[j] = action_label[j+2];
        j++;
    }
    action_label[j] = '\0';


} /* end PGS_SMF_RemoveSpecialChar */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ExtractMsg

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void 
    PGS_SMF_ExtractMsg(
        char  *linebuf,
        char  *mnemonic,
        char **msgptr);

FORTRAN:
    NONE

DESCRIPTION:
    To extract message string out from the line string.

INPUTS:
    Name            Description                                Units        Min        Max   

    linebuf         line string
    mnemonic        mnemonic label

OUTPUTS:
    Name            Description                                Units        Min        Max   

    msgptr          message string

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_ExtractMsg(       /* Extract message string */
    char  *linebuf,       /* Input  line string */
    char  *mnemonic,      /* Input  mnemonic label */
    char **msgptr)        /* Output message string */
{
    short  i;  /* counter */


    for (i=(short)strlen(mnemonic) ; i < (short)strlen(linebuf) ; i++)
    {
        if ((linebuf[i] != ' ')  &&
            (linebuf[i] != '\n') &&
            (linebuf[i] != '\t'))
        {
            break;
        }
    }

    if (strlen(mnemonic) == strlen(linebuf))
    {
        *msgptr = (char *)NULL;
    }
    else
    {
        *msgptr = &linebuf[i];
    }


} /* end PGS_SMF_ExtractMsg */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetErrMsg

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    char 
    *PGS_SMF_GetErrMsg(
         short errmsg_no);

FORTRAN:
    NONE

DESCRIPTION:
    List of all the error messages for smfcompile executable.

INPUTS:
    Name            Description                                Units        Min        Max   

    errmsg_no       error message number

OUTPUTS:     
    Name            Description                                Units        Min        Max      

RETURNS:
    error message corresponding to the errmsg_no

EXAMPLES:
    NONE

NOTES:          
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static char*
PGS_SMF_GetErrMsg(        /* Get error message */
    short errmsg_no)      /* Input error message number */
{
    static char    msg[5000];  /* message buffer */
    char           misc[500];  /* message buffer */
    PgsSmfGlbVar  *global_var; /* global static buffer */
    short          i;          /* counter */
    short          labelMin = 3;
    short          labelMax = 10;
    short          instrMin = labelMin;
    short          instrMax = labelMax;
    short          mnemonicMin = 1;
    short          mnemonicMax = 17;  

    PGS_SMF_CompGetGlbVar(&global_var);
    memset(msg,'\0',sizeof(msg));

    switch (errmsg_no)
    {
        case 1:
             strcpy(msg, "Usage: smfcompile -f textfile [-r] [-i]\n");
	     strcat(msg, "       smfcompile -f textfile -c\n");
             strcat(msg, "       smfcompile -f textfile -f77\n");
             strcat(msg, "       smfcompile -f textfile -all\n");            
             strcat(msg, "       smfcompile -f textfile -ada\n\n");
             strcat(msg, "Note: -i - redirect the include file to 'PGSINC' directory\n");             
             strcat(msg, "      -r - redirect the message file to 'PGSMSG' directory\n");

             /*
              * Display the BNF rules.
              */
             for (i=1 ; i <= 13 ; i++)
             {
                 strcat(msg,PGS_SMF_GetBNF(i));
             }
             break;
        case 2:         
             sprintf(msg, "Error: can't open file <%s>\n",global_var->smf_filename);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) no such file or directory\n"); 
             break;                        
        case 3:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) "); 
#if ( defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(CYGWIN) )
             strcat (msg,strerror(global_var->smf_errno)); 
#else
             strcat (msg,sys_errlist[global_var->smf_errno]);
#endif
             strcat (msg, "\n");
             break;
        case 4:                      
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) non-allowable ascii character\n");
             break;
        case 5:
             sprintf(msg, "Error: could not redirect file <%s>\n",global_var->smf_redirectFile);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) environment variable 'PGSMSG' does not exist\n");
             break;
        case 6:
             sprintf(msg, "Error: could not redirect file <%s>\n",global_var->smf_redirectFile);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) environment variable 'PGSMSG' is not pointing to a directory\n"); 
             sprintf(misc,"           (%s)\n",getenv("PGSMSG"));
             strcat (msg,misc);
             break;    
        case 7:
             sprintf(msg, "Error: could not redirect file <%s>\n",global_var->smf_redirectFile);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) environment variable 'PGSINC' does not exist\n");
             break;
        case 8:
             sprintf(msg, "Error: could not redirect file <%s>\n",global_var->smf_redirectFile);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) environment variable 'PGSINC' is not pointing to a directory\n"); 
             break;                         
        case 9:
             sprintf(msg, "Error: could not redirect file <%s>\n",global_var->smf_redirectFile);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) UNIX \"mv\" command error\n");
             break;     
        case 10:
             sprintf(msg, "Error: number of defined status_definition(s) exceed %d\n",PGS_SMF_MAX_MNEMONIC);
             break;
/***
        case 11:
             break;
***/                          
        case 12:           
             sprintf(msg, "Error: creating C header file <%s>\n",global_var->smf_fileC);
             strcat (msg, "       Possible cause of errors:\n");
             strcat (msg, "       (1) system could not create this file\n");
             break;
        case 13:
             sprintf(msg, "Error: creating ASCII message file <%s>\n",global_var->smf_fileB);
             strcat (msg, "       Possible cause of errors:\n");
             strcat (msg, "       (1) system could not create this file\n");           
             break;
        case 14:
             sprintf(msg, "Successful operation on SMF file <%s>\n",global_var->smf_filename);
             strcat (msg, "Files created:\n");
             if (global_var->C == PGS_TRUE)
             {
                 strcat (msg, "      C header file: ");       
                 strcat (msg,global_var->smf_fileC); 
                 strcat (msg,"\n");
             }
             if (global_var->f77 == PGS_TRUE)   
             {           
                 strcat (msg, "      Fortran file: ");        
                 strcat (msg,global_var->smf_fileF);   
                 strcat (msg,"\n");
             }
             if (global_var->ada == PGS_TRUE)   
             {    
                 strcat (msg, "      Ada file: ");            
                 strcat (msg,global_var->smf_fileAda); 
                 strcat (msg,"\n");
             }
                 strcat (msg, "      ASCII message file: ");  
                 strcat (msg,global_var->smf_fileB);   
                 strcat(msg,"\n");
             break;
        case 15:
             sprintf(msg,"Error: action label '%s' is not defined in the file\n",global_var->smf_action);
             break;
        case 16:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) no message/action string is defined in the file\n"); 
             break;  
        case 17:            
             sprintf(msg, "Error: creating Fortran file <%s>\n",global_var->smf_fileF);
             strcat (msg, "       Possible cause of errors:\n");
             strcat (msg, "       (1) system could not create this file\n");
             break;                            
        case 18:           
             sprintf(msg, "Error: creating Ada file <%s>\n",global_var->smf_fileAda);
             strcat (msg, "       Possible cause of errors:\n");
             strcat (msg, "       (1) system could not create this file\n");
             break;  
        case 19:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) %INSTR token is not defined\n");
             break;  
        case 20:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) %LABEL token is not defined\n");
             break;               
        case 21:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) %SEED token is not defined\n");
             break; 
        case 22:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' unknown defined level status\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             break;               
        case 23:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) '%s' mnemonic label is not in UPPER_CASE_LETTER\n");
             break;        
        case 24:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' partially defined mnemonic label\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             break;                  
        case 25:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' two under-score characters are used in mnemonic label\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             break;                      
        case 26:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' invalid character used in mnemonic label\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             break;                   
        case 27:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' mnemonic label does not agree with what\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             strcat (msg, "           you have specify for the %LABEL token\n");
             break;                             
        case 28:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' no level status is defined\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             break;   
        case 29:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) no message/action string is defined with the corresponding\n"); 
             sprintf(misc,"           '%s' label\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             break;  
        case 30:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) %INSTR token not specify\n");
             break;
        case 31:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) no instrument has been defined or\n");
             strcat (msg, "       (2) proper usage is: %INSTR = instrument\n");
             break; 
        case 32:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) more than one instruments are defined\n");
             break;              
        case 33:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) defined instrument is not in UPPER_CASE_LETTER\n");
             break;
        case 34:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) defined instrument length is less than %d or\n",instrMin);
             strcat (msg,misc);
             sprintf(misc,"           exceed %d number of characters\n",instrMax);
             strcat (msg,misc);
             break; 
        case 35:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) %LABEL token not specify\n");
             break;
        case 36:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) no label has been defined or\n");
             strcat (msg, "       (2) proper usage is: %LABEL = label\n");
             break;  
        case 37:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) more than one labels are defined\n");
             break;
        case 38:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) defined label is not in UPPER_CASE_LETTER or\n");
             break;
        case 39:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) defined '%s' label length is less than %d or\n",global_var->smf_action,mnemonicMin);
             strcat (msg,misc);             
             sprintf(misc,"           exceed %d number of characters\n",mnemonicMax);
             strcat (msg,misc); 
             break;
        case 40:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) %SEED token not specify\n");
             break;
        case 41:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) no seed number has been defined or\n");
             strcat (msg, "       (2) proper usage is: %SEED = seed_number\n");
             break;
        case 42:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) more than one seed numbers are defined\n");
             break;
        case 43:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) defined seed number is less than or equal to 1\n");
             strcat (msg, "           or exceed 524287\n");
             break;
        case 44:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) mnemonic label has to be defined on column 1\n");
             break;
        case 45:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             strcat (msg, "       (1) no mnemonic label is defined\n");
             break;            
        case 46:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) message/action string exceed %d characters\n",PGS_SMF_MAX_MSG_SIZE-1);
             strcat (msg,misc);
             break;
        case 47:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' intended defined action label does not have\n",global_var->smf_action);
             strcat (msg,misc);
             strcat (msg, "           prefix _A_, or _C_ status\n");
             break;
        case 48:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' action label referencing itself\n",global_var->smf_mnemonic);
             strcat (msg,misc);
             break;
        case 49:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) no word(s) should be defined after '%s'\n",global_var->smf_action);
             strcat (msg,misc);
             break; 
        case 50:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) '%s' mnemonic label exceed %d characters\n",global_var->smf_mnemonic,PGS_SMF_MAX_MNEMONIC_SIZE-1);
             strcat (msg,misc);
             break; 
        case 51:
             sprintf(msg, "Error: reading file <%s> on line %d\n",global_var->smf_filename,global_var->smf_linecnt);
             strcat (msg, "       Possible cause of error:\n");
             sprintf(misc,"       (1) defined label length is less than %d or\n",labelMin);
             strcat (msg,misc);
             sprintf(misc,"           exceed %d number of characters\n",labelMax);
             strcat (msg,misc);
             break; 
    }

    return(msg);

} /* end PGS_SMF_GetErrMsg */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_GetBNF

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    char 
    *PGS_SMF_GetBNF(
         short bnf_no);

FORTRANN:
    NONE

DESCRIPTION:
    List of BNF rules for smfcompile executable.

INPUTS:
    Name            Description                                Units        Min        Max   

    bnf_no          BNF rule number

OUTPUTS:
    Name            Description                                Units        Min        Max            

RETURNS:
    bnf message corresponding to the bnf number

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static char*
PGS_SMF_GetBNF(        /* Get BNF message */
    short bnf_no)      /* Input BNF rule number */
{
    static char msg[1000];  /* message buffer */

    switch (bnf_no)
    {
        case 1:
            strcpy(msg,"BNF: allowed_ascii_char ::= {[\\n \\t sp]\n");
            strcat(msg,"                             [! \" # % & ' ( ) * + , - . /]\n");
            strcat(msg,"                             [DIGIT]\n");
            strcat(msg,"                             [: ; < > = ? @ ]\n");
            strcat(msg,"                             [UPPER_CASE_LETTER]\n");
            strcat(msg,"                             [LOWER_CASE_LETTER]\n");
            strcat(msg,"                             [[ \\ ] ^ _ ` { | } ~]}\n");
            break;
        case 2:
            strcpy(msg,"BNF: The %INSTR, %LABEL and %SEED tokens in the SMF text file\n"); 
            strcat(msg,"     must appear in this order and that it must be specify before\n");
            strcat(msg,"     any of the mnemonic/action labels are defined. Proper usage of\n");
            strcat(msg,"     the tokens are as follows: %INSTR = instrument\n");
            strcat(msg,"                                %LABEL = label\n");
            strcat(msg,"                                %SEED  = seed_number\n");
            break;
        case 3:
            strcpy(msg,"BNF: instrument ::= 3{UPPER_CASE_LETTER}10\n");
            break;
        case 4:
            strcpy(msg,"BNF: label ::= 3{UPPER_CASE_LETTER}10\n");
            break;
        case 5:
            strcpy(msg,"BNF: mnemonic ::= 1{[_][DIGIT]UPPER_CASE_LETTER}17\n");
            break;
        case 6:
            strcpy(msg,"BNF: mnemonic_label ::= label + _ + level + _ + mnemonic\n");
            break;
        case 7:
            strcpy(msg,"BNF: level ::= SH | S | M | U | N | W | E | F\n");
            break;
        case 8:        
            strcpy(msg,"BNF: action_label ::= label + _ + A | C + _ + mnemonic\n");
            break;
        case 9:        
            strcpy(msg,"BNF: status_definition ::= mnemonic_label + indentation +\n");
            strcat(msg,"                           message_str [+ :: + action_label]\n");
            break;
        case 10:        
            strcpy(msg,"BNF: indentation ::= {[\\n] [\\t] [ ]}\n");
            break;
        case 11:               
            strcpy(msg,"BNF: message_str ::= 1{[ ] [allowed_ascii_char]}240\n");
            break;
        case 12:        
            strcpy(msg,"BNF: action_definition ::= action_label + indentation +\n");
            strcat(msg,"     message_str + action_str\n");
            break;
        case 13:               
            strcpy(msg,"BNF: action_str ::= message_str\n");
            break;
    }


    return(msg);

} /* end PGS_SMF_GetBNF */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_ScanWord

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    void
    PGS_SMF_ScanWord(
        char *word);

FORTRAN:
    NONE

DESCRIPTION:
    To locate a word in the file.

INPUTS:
    Name            Description                                Units        Min        Max   

    word            word to locate

OUTPUTS:    
    Name            Description                                Units        Min        Max       

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()
    PGS_SMF_RemoveCarriageReturn()
    PGS_SMF_GetWord()

END_PROLOG:
*****************************************************************/
static void 
PGS_SMF_ScanWord(       /* Locate word in the file */
    char *word)         /* Input word to locate */
{
    FILE             *fptr = (FILE *)NULL;               /* file pointer */
    PgsSmfGlbVar     *global_var;                        /* global static buffer */
    short             i;                                 /* counter */
    PGSt_SMF_boolean  found = PGS_FALSE;                 /* flag to indicate found */
    char              linebuf[PGS_SMF_MAX_MSGBUF_SIZE];  /* line buffer */
    char              word1[PGS_SMF_MAX_MSG_SIZE];                        /* word */


    PGS_SMF_CompGetGlbVar(&global_var);
    global_var->smf_linecnt = 1;
    if ((fptr = fopen(global_var->smf_filename,"r")) != (FILE *)NULL)
    {
        while (fgets(global_var->smf_linebuf,PGS_SMF_MAX_MSGBUF_SIZE,fptr) != (char *)NULL) 
        {
            PGS_SMF_RemoveCarriageReturn(global_var->smf_linebuf);
            sprintf(linebuf,"%s",global_var->smf_linebuf);

            i = 0;
            while (PGS_SMF_GetWord(linebuf,word1,&i))
            {                       
                if (strcmp(word,word1) == 0)
                {
                    found = PGS_TRUE;
                    break;
                }
            }

            if (found == PGS_TRUE)
            {
                break;
            }
            else
            {
                global_var->smf_linecnt += 1;
            }
        }

        fclose(fptr);
    }


} /* end PGS_SMF_ScanWord */
/*****************************************************************    
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:
    PGS_SMF_SystemCall

SYNOPSIS:   
C:      
    #include <PGS_SMF.h>      

    PGSt_SMF_boolean
    PGS_SMF_SystemCall
        char *cmdStr);

FORTRAN:
    NONE

DESCRIPTION:
    This tool implements the POSIX system() routine. 

INPUTS:    
    Name            Description                                Units        Min        Max

    cmdStr          command string	

OUTPUTS:    
    Name            Description                                Units        Min        Max

RETURNS:
    PGS_TRUE
    PGS_FALSE

EXAMPLES:

NOTES:  
    The code to implement this routine is taken largely from "Advanced programming
    in the UNIX Environment" by W. Richard Stevens on page 314. The reason of not using
    system() routine directly is that the code should conform to POSIX.1. 

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:	    
    POSIX.2 requires that system() ignore SIGINT and SIGQUIT and block SIGCHLD.
    According to Steven's explanation, the catching of SIGCHLD should be blocked
    while system() function is executing otherwise when the child created by the 
    system() terminates, it would fool the caller of the system() into thinking
    that one of its own children terminated. system() will fork() a child
    process and when child terminates, it will generate SIGCHLD to the parent
    process.

    Also SIGINT should be blocked because when an interrupt signal is generated
    while system() is executing, the SIGINT will be send to parent and child
    processes. This signal should only be sent to the child process.

    Thus POSIX.2 states that the caller of system() should not be receiving SIGCHLD
    and SIGINT signals.   

    PGS_SMF_System() make use of C-Shell to execute the command.

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    NONE

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean
PGS_SMF_SystemCall(   /* Execute command string */
    char *cmdStr)     /* Input command string */  
{
    PGSt_SMF_boolean returnBoolean = PGS_TRUE;              /* return status */   
    pid_t            pid;                                   /* process id */
    int              status;                                /* status */
    struct sigaction ignore;                                /* ignore signal */
    struct sigaction saveintr;                              /* save signal interrupt */
    struct sigaction savequit;                              /* save signal quit */
    sigset_t         childmask;                             /* child mask */
    sigset_t         savemask;                              /* save mask */    
/*  char             buf[PGS_SMF_MAX_MSGBUF_SIZE]; */       /* max. buffer message */


    if (cmdStr != (char *)NULL)
    {
        returnBoolean = PGS_FALSE;

        /*
         * Ignore SIGINT and SIGQUIT so that caller of PGS_SMF_SystemCall() should not be
         * receiving these signals. 
         */
        ignore.sa_handler = SIG_IGN;
        sigemptyset(&ignore.sa_mask);
        ignore.sa_flags = 0;

        if (sigaction(SIGINT,&ignore,&saveintr) == 0)
        {        
            if (sigaction(SIGQUIT,&ignore,&savequit) == 0)
            {            
                /* 
                 * Now block SIGCHLD so that parent process (which is the caller of 
                 * PGS_SMF_System()) should not be receiving this signal when child process
                 * terminates (either abnormal or normal termination).
                 */
                sigemptyset(&childmask);
                sigaddset(&childmask,SIGCHLD);

                if (sigprocmask(SIG_BLOCK,&childmask,&savemask) == 0)
                {
                    if ((pid = fork()) < 0)
                    {
                        /*
                         * Probably out of processes.
                         */
                    }
                    else if (pid == 0)
                    {
                        /*
                         * Child process.
                         */

                        /*
                         * Restore previous dispositions of the signal actions and reset signal mask. 
                         * Reason is to allow execl() to change their dispositions to the default
                         * based on the caller's dispositions. In other words, the child will inherit
                         * all the caller's signals.
                         */
                        sigaction(SIGINT,&saveintr,NULL);
                        sigaction(SIGQUIT,&savequit,NULL);
                        sigprocmask(SIG_SETMASK,&savemask,NULL);

                        /*
                         * Execute the shell command. execl() will not return on success.
                         */ 
                        execl("/bin/sh","sh","-c",cmdStr,(char *)0);                        

                        /*
                         * If execl() failed (implying that the shell can't be executed), the
                         * return value is as if the shell had executed exit(127).
                         * The call to _exit() instead of exit() is to prevent any standard
                         * I/O buffers (which would have been copied from the parent to the child
                         * across the fork()) from being flushed into the child. Note that the call
                         * to _exit(127) only happens if execl() fails.
                         */  
                        _exit(127); /* exec error */
                    }
                    else
                    {
                        /*
                         * Parent process.
                         */

                        /*
                         * waitpid() will return child pid when successful. Note that 
                         * waitpid() will wait for the specific child process to finish.
                         */
                        while (waitpid(pid,&status,0) < 0)
                        {                            
                            if (errno != EINTR)
                            {     
                                status = -1;                          
                                break;
                            }
                        }

                        if (WIFEXITED(status))
                        {
                            /*
                             * TRUE if status was returned for a child that terminated normally.
                             * In this case, call WEXITSTATUS() to fetch the low-order 8 bits
                             * of the argument that the child passed to exit() or _exit(). The
                             * status returned by WEXITSTATUS() is implementation dependent; in other
                             * words, it depends on exit() in the main() routine of the process. Usually,
                             * exit(0) means success otherwise error.
                             */
                            status = WEXITSTATUS(status);
                            if (status == 0)
                            {
                                returnBoolean = PGS_TRUE;
                            }
                        }
                        else if (WIFSIGNALED(status))
                        {
                            /*
                             * TRUE if status was returned for a child that terminated abnormally.
                             * In this case, call WTERMSIG() to fetch the signal number that 
                             * caused the termination.
                             */
                        }
                        else if (WIFSTOPPED(status))
                        {
                            /*
                             * TRUE if status was returned for a child that is currently stopped.
                             * In this case, call WWSTOPSIG() to fetch the signal number that 
                             * caused the child to stop.
                             */
                        }
                    }

                    /*
                     * Restore previous signal actions and reset signal mask for the caller.
                     */
                    if (sigaction(SIGINT,&saveintr,NULL) == 0)
                    {
                        if (sigaction(SIGQUIT,&saveintr,NULL) == 0)
                        {
                            if (sigprocmask(SIG_SETMASK,&savemask,NULL) == 0)
                            {
                                /* 
                                 * Whatever the returnBoolean from the while() loop above.
                                 */
                            }
                            else
                            {
                                returnBoolean = PGS_FALSE;
                            }
                        }
                        else
                        {
                            returnBoolean = PGS_FALSE;
                        }                            
                    }
                    else
                    {
                        returnBoolean = PGS_FALSE;
                    }
                }
            }
        }
    }                                              


    return(returnBoolean);

} /* end PGS_SMF_SystemCall */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_RedirectMsgFile

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean
    PGS_SMF_RedirectMsgFile( 
        char *filename);

FORTRAN:
    NONE

DESCRIPTION:
    Redirect MSG file.

INPUTS:
    Name            Description                                Units        Min        Max   

    filename        file name

OUTPUTS:    
    Name            Description                                Units        Min        Max       

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean
PGS_SMF_RedirectMsgFile(     /* Redirect MSG file */
    char *filename)          /* Input file name */
{     
    char              buf[1010];                  /* buffer */
    char             *pgsMsgPath = (char *)NULL;  /* PGSMSG path */
    struct stat       statInfo;                   /* file status */        
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;   /* return status */
    PgsSmfGlbVar     *global_var;                 /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    pgsMsgPath = getenv("PGSMSG");
    if (pgsMsgPath == (char *)NULL)
    {
        global_var->smf_errmsg_no = 5;
        returnBoolean = PGS_FALSE;
    }
    else
    {
        /*
         * Check if 'PGSMSG' is a directory.
         */
        stat(pgsMsgPath,&statInfo);
        if (S_ISDIR(statInfo.st_mode))
        {
            sprintf(buf,"mv %s %s",global_var->smf_fileB,pgsMsgPath);
            returnBoolean = PGS_SMF_SystemCall(buf); 
            if (returnBoolean == PGS_TRUE)
            {
                free(global_var->smf_fileB);
                global_var->smf_fileB = (char *) calloc(strlen(pgsMsgPath)+strlen(filename)+32,sizeof(char));
                sprintf(global_var->smf_fileB,"%s/%s",pgsMsgPath,filename); 
            } 
            else
            {
                global_var->smf_errmsg_no = 9;
                returnBoolean = PGS_FALSE;
            }
        }
        else
        {
            global_var->smf_errmsg_no = 6;
            returnBoolean = PGS_FALSE;
        }
    }

    global_var->smf_linecnt = 0;
    global_var->smf_redirectFile = global_var->smf_fileB;

    return(returnBoolean);

} /* end PGS_SMF_RedirectMsgFile */
/*****************************************************************
BEGIN_PROLOG:

TITLE:  
    Error Status Handling

NAME:   
    PGS_SMF_RedirectIncFile

SYNOPSIS:       
C:
    #include <PGS_SMF.h>

    PGSt_SMF_boolean
    PGS_SMF_RedirectIncFile( 
        short fileToRedirect,
        char *filename);

FORTRAN:
    NONE

DESCRIPTION:
    Redirect INC file.

INPUTS:
    Name            Description                                Units        Min        Max   

    fileToRedirect  file to redirect
    filename        file name

OUTPUTS:    
    Name            Description                                Units        Min        Max       

RETURNS:
    NONE

EXAMPLES:
    NONE

NOTES:
    NONE

REQUIREMENTS:
    PGSTK-0580,0590,0600,0610,0620,0650

DETAILS:
    NONE

GLOBALS:
    NONE

FILES:
    NONE

FUNCTIONS_CALLED:
    PGS_SMF_CompGetGlbVar()

END_PROLOG:
*****************************************************************/
static PGSt_SMF_boolean
PGS_SMF_RedirectIncFile(     /* Redirect INC file */
    short fileToRedirect,    /* Input file to redirect */
    char *filename)          /* Input file name */
{     
    char              buf[1010];                  /* buffer */
    char             *pgsIncPath = (char *)NULL;  /* PGSINC path */
    struct stat       statInfo;                   /* file status */        
    PGSt_SMF_boolean  returnBoolean = PGS_TRUE;   /* return status */
    PgsSmfGlbVar     *global_var;                 /* global static buffer */

    PGS_SMF_CompGetGlbVar(&global_var);

    pgsIncPath = getenv("PGSINC");
    if (pgsIncPath == (char *)NULL)
    {
        global_var->smf_errmsg_no = 7;
        returnBoolean = PGS_FALSE;
    }
    else
    {
        /*
         * Check if 'PGSINC' is a directory.
         */
        stat(pgsIncPath,&statInfo);
        if (S_ISDIR(statInfo.st_mode))
        {
            switch (fileToRedirect)
            {
                case PGS_SMF_REDIRECT_FORTRAN:                
                     sprintf(buf,"mv %s %s",global_var->smf_fileF,pgsIncPath);
                     returnBoolean = PGS_SMF_SystemCall(buf); 
                     if (returnBoolean == PGS_TRUE)
                     {
                         free(global_var->smf_fileF);
                         global_var->smf_fileF = (char *) calloc(strlen(pgsIncPath)+strlen(filename)+32,sizeof(char));
                         sprintf(global_var->smf_fileF,"%s/%s",pgsIncPath,filename); 
                     }
                     else
                     {
                         global_var->smf_errmsg_no = 9;
                         returnBoolean = PGS_FALSE;
                     }  

                     global_var->smf_redirectFile = global_var->smf_fileF;
                     break;

                case PGS_SMF_REDIRECT_HEADER:                
                     sprintf(buf,"mv %s %s",global_var->smf_fileC,pgsIncPath);
                     returnBoolean = PGS_SMF_SystemCall(buf); 
                     if (returnBoolean == PGS_TRUE)
                     {
                         free(global_var->smf_fileC);
                         global_var->smf_fileC = (char *) calloc(strlen(pgsIncPath)+strlen(filename)+32,sizeof(char));
                         sprintf(global_var->smf_fileC,"%s/%s",pgsIncPath,filename); 
                     }
                     else
                     {
                         global_var->smf_errmsg_no = 9;
                         returnBoolean = PGS_FALSE;
                     }  

                     global_var->smf_redirectFile = global_var->smf_fileC;                      
                     break;   

                case PGS_SMF_REDIRECT_ADA:                
                     sprintf(buf,"mv %s %s",global_var->smf_fileAda,pgsIncPath);
                     returnBoolean = PGS_SMF_SystemCall(buf); 
                     if (returnBoolean == PGS_TRUE)
                     {
                         free(global_var->smf_fileAda);
                         global_var->smf_fileAda = (char *) calloc(strlen(pgsIncPath)+strlen(filename)+32,sizeof(char));
                         sprintf(global_var->smf_fileAda,"%s/%s",pgsIncPath,filename); 
                     }
                     else
                     {
                         global_var->smf_errmsg_no = 9;
                         returnBoolean = PGS_FALSE;
                     }  

                     global_var->smf_redirectFile = global_var->smf_fileAda;             
                     break; 
            }                                       
        }
        else
        {
            if (returnBoolean == PGS_FALSE)
            {
                global_var->smf_errmsg_no = 8; 
                returnBoolean = PGS_FALSE;             
            }
        }
    }

    global_var->smf_linecnt = 0;

    return(returnBoolean);

} /* end PGS_SMF_RedirectIncFile */
