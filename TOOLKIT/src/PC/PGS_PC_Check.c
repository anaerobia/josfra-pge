/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	pctcheck.c

DESCRIPTION:
	This file contains all functions necessary for the check program.

AUTHOR:
	Ray Milburn / Applied Research Corp.
        Abe Taaheri / Raytheon IIS

HISTORY:
	15-Aug-94 RM Initial Version
	21-Feb-95 RM Updated #define variable BAD to PGS_PC_BAD
			as per DR ECSed00771.
	07-Apr-95 RM Updated for TK5.  Added code to check default
			file location data.
	12-Dec-95 RM Updated for TK6.  Added code to allow for multiple
			instance of a PRODUCT OUTPUT FILE.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.
	31-Jul-96 RM Updated to include fix of DR ECSed001827.  This
			fix checks to ensure that file names have 
			a length greater that zero (0).
	24-Apr-96 RM Added code to allow for multiple instances of 
			SUPPORT INPUT and SUPPORT OUTPUT FILES.
        22-Jan-09 AT    fix problem with missing linePos++ in 
                        CheckSysConfig() and adding an if statement
                        to catch blank in the first column of a line
                        and report it as error.

END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:  
	Check Process Control Information file
 
NAME:  
	check

SYNOPSIS:
	This program accepts an input file as a command line argurment.

C:
	NONE

FORTRAN:
	NONE

DESCRIPTION:
	This program searches and reports errors and warnings found in the
	Process Control Status information file.  An error is defined as 
	something that will stop the Process Control Tools from returning
	a PGS_S_SUCCESS.  A warning is something that the Process Control
	Tools are not going to flag but there is a STRONG possibility that
	another tool will.  For example, the file name "file one.fil" will
	be happily returned from the Process Control Tools, but if you try
	to open it, BOOOOOOOOOMMM.
 
INPUTS:
        Name            Description                     Units   Min     Max

	argv[0]		executable name (not processed 
			but listed here anyway)

	argv[1]		Process Control information 
			file name to be checked.

OUTPUTS:
        Name            Description                     Units   Min     Max

	NONE

RETURNS:
	0 - unsuccessful completion could be caused by invalid input
		parameters or unable to find input file.
        lineNum - number of lines in input file.

EXAMPLES:

	check /u/PGE/PCFILES/pctinfo.fil

NOTES:
	To run this program enter two things and two things only. One, the
	program name (check or whatever you decide to call the executable)
	and two, the name of the file to be checked.  If the file is not in 
	the current working directory then enter the full path name.  
	For example "check pgsceres1.fil" will invoke the pctcheck program 
	and check the file named pgsceres1.fil.

REQUIREMENTS:
	NONE

DETAILS:
	Currently, this program is invoked from within a shell script named
	pccheck.sh.  The user will probably never run this program stand
	alone from the command line.

GLOBALS:
	NONE

FILES:
	This program opens and reads the file name that is passed in as a
	command line parameter.

FUNCTIONS_CALLED:
	GetIndex	determines if line contains a legal index value
	CheckConfig	determines if line contains valid configuration data
	CheckFileInfo	determines if line contains valid file information
	CheckVersion	determines if a version number is present
	CheckSysConfig	determines if the system configuration data is valid
	CheckLengths	determines if strings in the line are too long
	CheckIndexes	determines if there are any repeat indexes
	DoMessage	displays a generic error/warning message
	CheckDefaultLoc	determine if line contains legal default file location

END_PROLOG:
***************************************************************************/

/***************************************************************************
*    #define(s) necessary for this program that are not included in the 
*    file PGS_PC.h.
***************************************************************************/
#define MAX_SYSTEM_CONFIGS PGSd_PC_SOFTWARE_ID
#define INFILE_LOCATION 1
#define NUM_ARGS 2
#define OK 1
#define PGS_PC_BAD -1
#define WARNING -2
#define PGS_PC_ZERO_LENGTH -10
#define CONFIG_DELIMITERS 2
#define FILE_DELIMITERS 6
#define BLANK ' '
#define FILE_POS 1
#define PATH_POS 2
#define UREF_POS 4
#define ATTR_POS 5
#define VALUE_POS 2
#define MAX_INDEXES 100000

/***************************************************************************
*    Function definitions for functions that are included in this file.
***************************************************************************/
int GetIndex(char *,int *,PGSt_PC_Logical *);
int CheckConfig(char *,int);
int CheckFileInfo(char *,int);
int CheckVersion(char *);
int CheckSysConfig(char *);
int CheckLengths(char *,int,int);
int CheckIndexes(PGSt_PC_Logical, PGSt_PC_Logical [], int *, int);
void DoMessage(char *,int,char *);
int CheckDefaultLoc(char *,char *);

/***************************************************************************
*    Function main().
***************************************************************************/
int 
main(                                      /* function main() */
    int     argc,                          /* command line argument counter */
    char   *argv[])                        /* command line argument value */
{
/*    char    *tem; */                          /* temporary hold for fgets() */
    char     line[PGSd_PC_LINE_LENGTH_MAX]; /* line read from file */
    char     defLoc[PGSd_PC_LINE_LENGTH_MAX]; /* line read from file */
    char     warnMessage[200];             /* warning or error message */
    int      dividersFound = 0;            /* number of dividers found */
    int      lineNum = 0;                  /* current position in file */
    int      sysConfigsFound = 0;    /* number of system config values found */
    int      sysConfigCheck = 0;           /* flag to check sys configs */
    int      pos;                          /* column position in file */
    int      retIndex;                     /* return from GetIndex() */
    int      retCheckIndexes;              /* return from CheckIndexes */
    int      retConfig;                    /* return from CheckConfig() */
    int      retFile;                      /* return from CheckFileInfo() */
    int      retLength;                    /* return from CheckLengths() */
    int      retCheckDef;                  /* return from CheckDefaultLoc() */
    int      problems = 0;                 /* number of problems found */
    int      warnings = 0;                 /* number of warnings found */
    int      fileIndexCount = 0;           /* number of file indexes found */
    int      configIndexCount = 0;         /* number of config indexes found */
    FILE    *fp;                           /* pointer to input file */
    PGSt_boolean divFlag;                  /* states last line is divider */
    PGSt_PC_Logical indexValue;            /* current index value */
    PGSt_PC_Logical fileIndexes[MAX_INDEXES];   /* array of file indexes */
    PGSt_PC_Logical configIndexes[MAX_INDEXES]; /* array of config indexes */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    divFlag = PGS_FALSE;

/***************************************************************************
*    If the correct number of input parameters were passed in then try
*    to open the file.
***************************************************************************/
    if (argc == NUM_ARGS)
    {
        if ((fp = fopen(argv[INFILE_LOCATION],"r")) == NULL)
        {
            printf("Unable to open input file:  %s\n",argv[INFILE_LOCATION]);
            exit(0);
        }
    }
    else
    {
        printf("Incorrect number of command line arguments.\n");
        printf("Received:  %d\n",argc);
        printf("Expected:  %d\n",NUM_ARGS);
        printf("Usage: <executable> <file name to be checked>\n");
        exit(0);
    }
 
/***************************************************************************
*    Let's loop the number of separators times.  We will increment our
*    counter each time we encounter a separator.
***************************************************************************/
    while (dividersFound < PGSd_PC_TOTAL_SEPARATORS)
    {

/***************************************************************************
*    Read a line.  If we hit EOF then there is a big problem.
***************************************************************************/
        if ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,fp)) == NULL)
        {
            printf("Error - Unexpectedly reached EOF.\n");
            printf("Last line number successfully read:  %d\n",lineNum);
            printf("Number of dividers read:  %d\n",dividersFound);
            printf("Number of dividers expected:  %d\n",
                         PGSd_PC_TOTAL_SEPARATORS);
            printf("\nCheck of %s completed\n",argv[1]);
            printf("Errors found:  %d\n",++problems);
            printf("Warnings found:  %d\n",warnings);
            exit(lineNum);
        }

/***************************************************************************
*    Increment the line counter.
***************************************************************************/
        lineNum++;

/***************************************************************************
*    If the line is a comment line then just ignore it and get the next
*    line.
***************************************************************************/
        if (line[0] == PGSd_PC_COMMENT)
        {
            continue;
        }

/***************************************************************************
*    If the line has a separator in the first column then ignore the
*    rest of the line and read the next line.
***************************************************************************/
        else if (line[0] == PGSd_PC_DIVIDER)
        {
            dividersFound++;
            divFlag = PGS_TRUE;
            continue;
        }

/***************************************************************************
*    If the line has a blank in the first column then issue error
***************************************************************************/
        else if (line[0] == BLANK)
        {
	  sprintf(warnMessage,
		  "Error - Invalid blank in the first column.\n");
	  DoMessage(warnMessage,lineNum,line);
	  problems++;
        }

/***************************************************************************
*    If we have only found one divider then we are in the section that
*    contains system configuration values.  We do not check them for
*    validity we just check to make sure that there is the correct number
*    of them.
***************************************************************************/
        else if (dividersFound == PGSd_PC_SYS_CONFIG)
        {
            divFlag = PGS_FALSE;
            sysConfigsFound++;
            retIndex = CheckSysConfig(line);
            if (retIndex == WARNING)
            { 
                sprintf(warnMessage,
                "Warning - possible problem with system configuration value.\n");
                DoMessage(warnMessage,lineNum,line);
                warnings++;
            }
            continue;
        }

/***************************************************************************
*    We are beyond the system configuration values, but before we check
*    anything else, let's make sure that we had the correct number of
*    system defined configuration parameters.
***************************************************************************/
        else if (sysConfigCheck == 0)
        {
            sysConfigCheck = 1;
            if ((dividersFound > PGSd_PC_SYS_CONFIG) &&
                (sysConfigsFound != MAX_SYSTEM_CONFIGS))
            { 
                printf(
                 "Error - Invalid number of system configuration parameters.\n");
                printf("Found:  %d\n",sysConfigsFound);
                printf("Expected:  %d\n\n",MAX_SYSTEM_CONFIGS);
                problems++;
            }
        }

/***************************************************************************
*    Now let's find out where we are in the input file and perform the 
*    proper check on the line.
***************************************************************************/
        switch (dividersFound)
        {

/***************************************************************************
*    This is where the user defined configuration parameters are stored
*    in the file.
***************************************************************************/
            case PGSd_PC_CONFIG_COUNT:
            {

/***************************************************************************
*    Check to make sure that a legal index value is entered in the field.
*    We are going to return the index value so we can check for repeats
*    later on.
***************************************************************************/
                divFlag = PGS_FALSE;
                retIndex = GetIndex(line,&pos,&indexValue);
                if (retIndex == PGS_PC_BAD)
                {
                    sprintf(warnMessage,
                    "Error - Invalid identifier number in user defined configuration parameters.\n");
                    DoMessage(warnMessage,lineNum,line);
                    problems++;
                }

/***************************************************************************
*    We know that up until now the index value is composed of legal
*    digits.  But, now we need to make sure that we do not have any
*    repeats.  Remember, the reason that we are storing them in two 
*    different arrays is because the file indexes are separate than
*    that of the user defined configuration parameters indexes.
***************************************************************************/
                else
                {
                    retCheckIndexes = CheckIndexes(indexValue,
                               configIndexes,&configIndexCount,dividersFound);
                    if (retCheckIndexes == PGS_PC_BAD)
                    {
                        sprintf(warnMessage,
                        "Warning - Repeat index number in user defined configuration parameters.\n");
                        DoMessage(warnMessage,lineNum,line);
                        warnings++;
                    }
                }       /* end else */

/***************************************************************************
*    Check the rest of the line.
***************************************************************************/
                retConfig = CheckConfig(line,pos);

/***************************************************************************
*    If the line is bad inform the user.
***************************************************************************/
                if (retConfig == PGS_PC_BAD)
                {
                    sprintf(warnMessage,
                    "Error - Problem with user defined configuration parameter.\n");
                    DoMessage(warnMessage,lineNum,line);
                    problems++;
                }

/***************************************************************************
*    Let's be real nice and let the user know if there is something
*    that may bite them later on.
***************************************************************************/
               else if (retConfig == WARNING)
               {
                   sprintf(warnMessage,
                   "Warning - extra delimiters in user defined configuration parameters.\n");
                   DoMessage(warnMessage,lineNum,line);
                   warnings++;
               }

/***************************************************************************
*    Check to see if the user configuration value is too long.
***************************************************************************/
               retLength = CheckLengths(line,VALUE_POS,
                                        PGSd_PC_VALUE_LENGTH_MAX);
               if (retLength == PGS_PC_BAD)
               {
                   sprintf(warnMessage,
                        "Error - Configuration value length too long.\n");
                   DoMessage(warnMessage,lineNum,line);
                   problems++;
               }

/***************************************************************************
*    Done with the user configuration stuff.
***************************************************************************/
               break;
            }
          
/***************************************************************************
*    Currently we are handling seven (7) types of files so here is where
*    we handle them.
***************************************************************************/
            case PGSd_PC_INPUT_FILES:
            case PGSd_PC_OUTPUT_FILES:
            case PGSd_PC_TEMP_INFO:
            case PGSd_PC_INTER_INPUT:
            case PGSd_PC_INTER_OUTPUT:
            case PGSd_PC_SUPPORT_INPUT:
            case PGSd_PC_SUPPORT_OUTPUT:
            {

/***************************************************************************
*    Determine if this line contains default file location data.
***************************************************************************/
                if (line[0] == PGSd_PC_DEFAULT_LOC)
                {

/***************************************************************************
*    If the last valid line read was divider then we have a valid
*    default file location line.  Let's check it as such.
***************************************************************************/
                    if (divFlag == PGS_TRUE)
                    {
                        divFlag = PGS_FALSE;
                        retCheckDef = CheckDefaultLoc(line,defLoc);

/***************************************************************************
*    The line was blank after the DEFAULT_LOC flag.
***************************************************************************/
                        if (retCheckDef == PGS_PC_BAD)
                        {
                            sprintf(warnMessage,
                            "Error - Default file location marker contains no data.\n");
                            DoMessage(warnMessage,lineNum,line);
                            problems++;
                        }

/***************************************************************************
*    The data contained something that may hurt the program later.
***************************************************************************/
                        else if (retCheckDef == WARNING)
                        {
                            sprintf(warnMessage,
                            "Warning - possible problem in default file location.\n");
                            DoMessage(warnMessage,lineNum,line);
                            warnings++;
                        }
                        else
                        {

/***************************************************************************
*    Make sure that the length of the default file location string
*    is not too long.
***************************************************************************/
                            retLength = CheckLengths(defLoc,0,
                                                   PGSd_PC_PATH_LENGTH_MAX);
                            if (retLength == PGS_PC_BAD)
                            {
                                sprintf(warnMessage,
                                    "Error - Default File location length too long.\n");
                                DoMessage(warnMessage,lineNum,line);
                                problems++;
                            }
                        }
                    }

/***************************************************************************
*    A default file location was found but it was not the first
*    non-comment line after the divider.
***************************************************************************/
                    else
                    {
                        sprintf(warnMessage,
                        "Warning - Default file location not after divider.\n");
                        DoMessage(warnMessage,lineNum,line);
                        warnings++;
                    }
                }
                else
                {

/***************************************************************************
*    If the divider flag is still true then we have not found a default
*    file location and we have a valid line.  This is a problem.
***************************************************************************/
                    if (divFlag == PGS_TRUE)
                    {
                        sprintf(warnMessage,
                        "Error - Default file location not found.\n");
                        DoMessage(warnMessage,lineNum,line);
                        problems++;
                    }
          
/***************************************************************************
*    Since we have a valid line we must first and foremost check to 
*    see if there is a legal index value entered.  We are going to return 
*    the index value so we can look at this guy a little closer later on.
***************************************************************************/
                    divFlag = PGS_FALSE;
                    retIndex = GetIndex(line,&pos,&indexValue);
                    if (retIndex == PGS_PC_BAD)
                    {
                        sprintf(warnMessage,
                        "Error - Invalid identifier number involving file information.\n");
                        DoMessage(warnMessage,lineNum,line);
                        problems++;
                    }

/***************************************************************************
*    We know that up until now the index value is composed of legal
*    digits.  But, now we need to make sure that we do not have any
*    repeats.  Remember, the reason that we are storing them in two 
*    different arrays is because the file indexes are separate than
*    that of the user defined configuration parameters indexes.
***************************************************************************/
                    else
                    {
                        retCheckIndexes = CheckIndexes(indexValue,
                                    fileIndexes,&fileIndexCount,dividersFound);
                        if (retCheckIndexes == PGS_PC_BAD)
                        {
                            sprintf(warnMessage,
                            "Warning - Repeat index number in file information.\n");
                            DoMessage(warnMessage,lineNum,line);
                            warnings++;
                        }
                    }       /* end else */
          
/***************************************************************************
*    Now, let's check the rest of the line to make sure it looks kosher.
***************************************************************************/
                    retFile = CheckFileInfo(line,pos);
          
/***************************************************************************
*    If we have a problem that will cause a blowout with one of the  
*    Process Control Tools then let's tell them about it here.
***************************************************************************/
                    if (retFile == PGS_PC_BAD)
                    {
                        sprintf(warnMessage,
                        "Error - Invalid number of delimiters involving file information.\n");
                        DoMessage(warnMessage,lineNum,line);
                        problems++;
                    }
          
/***************************************************************************
*    If we have found something that may bite them later on let's show
*    it to them here.  If they want to fix it that's up to them.
***************************************************************************/
                    if (retFile == WARNING)
                    {
                        sprintf(warnMessage,
                        "Warning - possible problem in path or file name.\n");
                        DoMessage(warnMessage,lineNum,line);
                        warnings++;
                    }
          
/***************************************************************************
*    Remember, with Product and Standard Input and Output files we need 
*    a version number.  So let's check to make sure that they actually 
*    included one.
***************************************************************************/
                    if (((dividersFound == PGSd_PC_INPUT_FILES) ||
                         (dividersFound == PGSd_PC_OUTPUT_FILES) || 
                         (dividersFound == PGSd_PC_SUPPORT_INPUT) ||
                         (dividersFound == PGSd_PC_SUPPORT_OUTPUT)) &&
                         (retFile != PGS_PC_BAD))
                    {
                        retFile = CheckVersion(line);
                        if (retFile == PGS_PC_BAD)
                        {
                            sprintf(warnMessage,
                            "Error - problem with version number in Standard input or output file information.\n");
                            DoMessage(warnMessage,lineNum,line);
                            problems++;
                        }
                    }

/***************************************************************************
*    Check to see if the file name stored in the line is too long.
*    or if the file name is of zero length. (DR ECSed001827 31-Jul-96)
***************************************************************************/
                    retLength = CheckLengths(line,FILE_POS,PGSd_PC_FILE_NAME_MAX);
                    if (retLength == PGS_PC_BAD)
                    {
                        sprintf(warnMessage,
                            "Error - File name length too long.\n");
                        DoMessage(warnMessage,lineNum,line);
                        problems++;
                    }

                    if (retLength == PGS_PC_ZERO_LENGTH)
                    {
                        sprintf(warnMessage,
                            "Error - File name does not exist.\n");
                        DoMessage(warnMessage,lineNum,line);
                        problems++;
                    }

/***************************************************************************
*    Check to see if the path stored in the line is too long.
***************************************************************************/
                    retLength = CheckLengths(line,PATH_POS,PGSd_PC_PATH_LENGTH_MAX);
                    if (retLength == PGS_PC_BAD)
                    {
                        sprintf(warnMessage,"Error - Path length too long.\n");
                        DoMessage(warnMessage,lineNum,line);
                        problems++;
                    }

/***************************************************************************
*    Check to see if the attribute file name stored in the line 
*    is too long.
***************************************************************************/
                    retLength = CheckLengths(line,ATTR_POS,PGSd_PC_FILE_NAME_MAX);
                    if (retLength == PGS_PC_BAD)
                    {
                        sprintf(warnMessage,
                          "Error - Attribute file name length too long.\n");
                        DoMessage(warnMessage,lineNum,line);
                        problems++;
                    }

/***************************************************************************
*    Check to see if the universal reference stored in the line
*    is too long.  Remember, universal references do not apply to 
*    TEMPORARY files.  THIS WAS ADDED FOR TK6.
***************************************************************************/
                    if (dividersFound != PGSd_PC_TEMP_INFO)
                    {
                        retLength = CheckLengths(line,UREF_POS,
                                                 PGSd_PC_UREF_LENGTH_MAX);
                        if (retLength == PGS_PC_BAD)
                        {
                            sprintf(warnMessage,
                              "Error - Universal Reference length too long.\n");
                            DoMessage(warnMessage,lineNum,line);
                            problems++;
                        }
                    }    

/***************************************************************************
*    Done with the file stuff........finally!
***************************************************************************/
                    break;
                }  /* end else */
            }
          
/***************************************************************************
*    We should never get here.
***************************************************************************/
            default:
            {
                break;
            }

        } /* end switch */

    }  /* end while */

/***************************************************************************
*    If we are not at the end of the file then we may have a serious
*    problem.  It is possible that they just wrote in stuff after the 
*    last divider which is absolutely no problem, although it is not
*    recommended.
***************************************************************************/
    if ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,fp)) != NULL)
    {
        printf("Warning - information beyond final divider will be ignored.\n");
        printf("line number:  %d\n",++lineNum);
        printf("Number of dividers read:  %d\n",dividersFound);
        printf("Number of dividers expected:  %d\n\n",PGSd_PC_TOTAL_SEPARATORS);
        warnings++;
        lineNum++;

/***************************************************************************
*    Read the rest of the lines in the file so we can get a count of the
*    number of lines.
***************************************************************************/
        while ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,fp)) != NULL)
        {
            lineNum++;
        }
    }

/***************************************************************************
*    Close the file.
***************************************************************************/
    fclose(fp);

/***************************************************************************
*    Give the user a summary.  We want to do this in case there was a 
*    load of errors and warnings.  This way he/she will know how many
*    messages scrolled by at the speed of light.
***************************************************************************/
    printf("\nCheck of %s completed\n",argv[1]);
    printf("Errors found:  %d\n",problems);
    printf("Warnings found:  %d\n\n",warnings);

/***************************************************************************
*    End of the program, return the number of lines in the file.
***************************************************************************/
    return lineNum;
}


/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get index value from line of data.
 
NAME:
	GetIndex()

SYNOPSIS:
	This function parses the index value from a line of data.

C:
	int GetIndex(
		char		*line,
		int		*linePos,
		PGSt_PC_Logical	*indValue);

FORTRAN:
	NONE

DESCRIPTION:
	Search a line of data and return the index value.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of data read from the 
			process control information 
			file.

OUTPUTS:
	Name		Description			Units	Min	Max

	linePos		Last line position evaluated.

	indValue	Actual index value.

RETURNS:
	OK		successful completion
	PGS_PC_BAD	error with index value search

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


int GetIndex(                  /* get index value from line */
    char    *line,             /* line read from file */
    int     *linePos,          /* position in line */
    PGSt_PC_Logical *indValue) /* index value */
{
    char charNum[50];          /* character array to hold index value */
    PGSt_PC_Logical num;       /* return from atol() */

/***************************************************************************
*    We want to start at zero.
***************************************************************************/
    *linePos = 0; 

/***************************************************************************
*    Loop until we find a delimiter.
***************************************************************************/
    while (line[*linePos] != PGSd_PC_DELIMITER)
    {

/***************************************************************************
*    Ensure that the character is between a low digit and a high digit.
*    You know, a '0' and a '9' or something in between.  We also want
*    to load the index value into a character array for later processing.
***************************************************************************/
        if ((line[*linePos] >= PGSd_PC_LOWDIGIT) && 
            (line[*linePos] <= PGSd_PC_HIDIGIT))
        {
            charNum[*linePos] = line[*linePos];
            *linePos = *linePos + 1;
        }

/***************************************************************************
*    If we find something that is not a digit.....red flag.
***************************************************************************/
        else
        {
            return PGS_PC_BAD;
        }

    }  /* end while */

/***************************************************************************
*    We are now at later processing, let's convert the character array
*    storing the index value to the type that we need.  This is what we 
*    need to return.
***************************************************************************/
    if (*linePos > 0)
    {
        charNum[*linePos] = '\0';
        num = (PGSt_PC_Logical) atol(charNum);    
        *indValue = num;    

/***************************************************************************
*    If we made it this far then everything is OK.
***************************************************************************/
        return OK;
    }
    else
    {
       return PGS_PC_BAD;
    }
}




/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Check line of configuration data.
 
NAME:
	CheckConfig()

SYNOPSIS:

C:
	int CheckConfig(
		char		*line,
		int		linePos);

FORTRAN:
	NONE

DESCRIPTION:
	Check a line of configuration data to ensure that the proper 
	number of delimiters are present and that the data does not
	exceed that maximum length.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of Process Control Information
			data.

	linePos		Last position in line that data
			had been checked.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	OK		all data is OK 
	WARNING		may be a problem with the data
	PGS_PC_BAD	error with data

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	N/A

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


int CheckConfig(                 /* check line of config data */
    char     *line,              /* line from input file */
    int       linePos)           /* position of last character processed */
{
    int     delimiters;          /* number of delimiters found */
    int     flag;                /* flag if we checked for a character */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    delimiters = 0;
    flag = 0;

/***************************************************************************
*    Loop through the line until we find the number of delimiters that 
*    we were looking for or we hit the end of the line.
***************************************************************************/
    while ((line[linePos] != PGSd_PC_NEWLINE) &&
           (delimiters < CONFIG_DELIMITERS))
    {
        if (line[linePos] == PGSd_PC_DELIMITER)
        {
            delimiters++;
        }

    linePos++;
    }    /* end while */

/***************************************************************************
*    We need to make sure that we found the correct number of delimiters
*    before we go on.
***************************************************************************/
    if (delimiters == CONFIG_DELIMITERS)
    {

/***************************************************************************
*    Loop until the end of the line.
***************************************************************************/
        while (line[linePos] != PGSd_PC_NEWLINE)
        {

/***************************************************************************
*    Make sure at least one character is a non-blank character.
***************************************************************************/
            if ((line[linePos] != BLANK) && (!flag))
            {
                flag = 1;
            }

/***************************************************************************
*    Let's also make sure that they did not try to slip in some extra
*    delimiters on us.
***************************************************************************/
            if (line[linePos] == PGSd_PC_DELIMITER)
            {
                return WARNING;
            }

        linePos++;
        }   /* end while */
    }    /* end if */

/***************************************************************************
*    Looks like we had a delimiter problem way back in the first loop.
***************************************************************************/
    else
    {
        return PGS_PC_BAD;
    }

/***************************************************************************
*    Our config value was nothing but blanks, that won't float.
***************************************************************************/
    if (!flag)
    {
        return PGS_PC_BAD;
    }

/***************************************************************************
*    If you made it to this point then you deserve a beer.
***************************************************************************/
    return OK;
}





/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Check a line of file information data.
 
NAME:
	CheckFileInfo()

SYNOPSIS:

C:	int CheckFileInfo(
		char		*line,
		int		linePos);

FORTRAN:
	NONE

DESCRIPTION:
	Check a line of file information data for the proper amount of
	delimiters and the proper type of data exists.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of Process Control Information
			data.

	linePos		Last position in line that data
			had been checked.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	OK		all data is OK 
	WARNING		may be a problem with the data
	PGS_PC_BAD	error with data

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


int CheckFileInfo(                /* check line of file info data */
    char     *line,               /* line read in from file */
    int       linePos)            /* last position processed in line */
{
    int      delimiters;          /* number of delimiters found */
    int      pos;                 /* current position in line */
    int      retVal;              /* function return value */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    delimiters = 0;
    pos = linePos;
    retVal = OK;

/***************************************************************************
*    Loop to the end of the line.
***************************************************************************/
    while (line[pos] != PGSd_PC_NEWLINE)
    {

/***************************************************************************
*    Count the number of delimiters.
***************************************************************************/
        if (line[pos] == PGSd_PC_DELIMITER)
        {
            delimiters++;
        }
        pos++;
    }

/***************************************************************************
*    If the line did not have the correct number of delimiters then 
*    there is no sense in checking it for anything else.
***************************************************************************/
    if (delimiters != FILE_DELIMITERS)
    {
        return PGS_PC_BAD;
    }
    else
    {

/***************************************************************************
*    We are going to start back at the beginning of the line and count
*    delimiters.
***************************************************************************/
        linePos = 0;
        delimiters = 0;
        while ((delimiters < 6) && ( retVal == OK))
        {

/***************************************************************************
*    Let's switch on the number of delimiters we have found.
***************************************************************************/
            switch (delimiters)
            {

/***************************************************************************
*    Delimiters number 0, 3, and 4 are not we need right now.  Let's check
*    if the position is a delimiter and increment all that needs it.
***************************************************************************/
                case 0:
                case 3:
                case 4:
                {
                    if (line[linePos] == PGSd_PC_DELIMITER)
                    {
                      delimiters++;
                    }

                    linePos++;
                    break;
                }

/***************************************************************************
*    Delimiters 1, 2, and 5 are important we want to make sure that no
*    blanks are in the path or file name.  Remember, the path and 
*    attribute is an optional field so the very next character can be 
*    a delimiter.
***************************************************************************/
                case 1:
                case 2:
                case 5:
                {
                    if (line[linePos] == BLANK)
                    {
                      retVal = WARNING;
                    }

                    if (line[linePos] == PGSd_PC_DELIMITER)
                    {
                      delimiters++;
                    }

                    linePos++;
                    break;
                }
            } /* end switch */
        } /* end while */
    } /* end else */

/***************************************************************************
*    Return what we discovered.
***************************************************************************/
    return retVal;

}





/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Check version field of Standard Input data.
 
NAME:
	CheckVersion()

SYNOPSIS:

C:	int CheckVersion(
		char		*line);

FORTRAN:
	NONE

DESCRIPTION:
	Check the version field of a line of Standard Input data
	to ensure it exists and is in the form of a digit.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of Process Control Information
			data.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	OK 		version appears to be OK
	PGS_PC_BAD	there is a problem with the version field

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


int CheckVersion(             /* check version number field */
    char      *line)          /* line read in from input file */
{
    int       delimiters;     /* number of delimiters found */
    int       pos;            /* position in the line */
    int       flag;           /* flag indicating position counter above 0 */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    delimiters = 0;
    pos = 0;
    flag = 0;

/***************************************************************************
*    Loop to the position in the line where the version number is supposed    
*    to start.
***************************************************************************/
    while (delimiters < FILE_DELIMITERS)
    {
        if (line[pos] == PGSd_PC_DELIMITER)
        {
          delimiters++;
        }

        pos++;
    }

/***************************************************************************
*    Now, we know we are at the version number, so let's loop until we 
*    hit the new line character.
***************************************************************************/
    while (line[pos] != PGSd_PC_NEWLINE)
    {

/***************************************************************************
*    If the character is a digit, let's move on.
***************************************************************************/
        if ((line[pos] >= PGSd_PC_LOWDIGIT) && (line[pos] <= PGSd_PC_HIDIGIT))
        {
            pos++;
            flag++;
            continue;
        }

/***************************************************************************
*    We have a problem.
***************************************************************************/
        else
        {
            return PGS_PC_BAD;
        }
    }   /* end while */

/***************************************************************************
*    Look at the flag from the first loop before moving on.
***************************************************************************/
    if (flag)
    {
        return OK;
    }
    else
    {
        return PGS_PC_BAD;
    }
}




/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Check line of system configuration data.
 
NAME:
	CheckSysConfig()

SYNOPSIS:

C:	int CheckSysConfig(
		char 		*line);

FORTRAN:
	NONE

DESCRIPTION:
	Check line of system configuration data to ensure that data 
	actually exists.  We do not want to allow a blank line here.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of Process Control Information
			data.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	OK		data is OK
	WARNING		data may be a blank line

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


int CheckSysConfig(           /* check system config data */
    char    *line)            /* line read from file */
{

    int linePos;              /* pointer to position in line */

/***************************************************************************
*    Intialize position counter.
***************************************************************************/
    linePos = 0; 

/***************************************************************************
*    Loop until we find a delimiter (newline character in this case).
***************************************************************************/
    while (line[linePos] != PGSd_PC_NEWLINE)
    {

/***************************************************************************
*    Ensure that this is not just a blank line.  Just make sure that one 
*    of the characters are not blank.
***************************************************************************/
        if (line[linePos] != BLANK)
        {
           return OK;
        }
	linePos++;
     }

/***************************************************************************
     If we made it this far then we had a blank line which may be bad.
***************************************************************************/
    return WARNING;
}





/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Print out a standard message.
 
NAME:
	DoMessage()

SYNOPSIS:

C:
	void DoMessage(
		char		*message,
		int		lineNum,
		char		*badLine);

FORTRAN:
	NONE

DESCRIPTION:
	Print out a standard error/warning message to standard ouput.
 
INPUTS:
	Name		Description			Units	Min	Max

	message		Actual message to print.

	lineNum		Line number that error/warning
			was found on.

	badLine		The actual line that contained
			the error/warning.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	N/A

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


void DoMessage(                         /* print out standard message */
    char *message,                      /* warning or error message */
    int lineNum,                        /* line number */
    char *badLine)                      /* line that contains problem */
{

/***************************************************************************
*    This is a standard warning/error message that will be used quite a
*    bit throughout this program.  So, to cut down on repeat code, we
*    put it in it's own little function.
***************************************************************************/
    printf("%s",message);
    printf("Line number:  %d\n",lineNum);
    printf("Line:  %s\n",badLine);

    return;
}




/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Check the length of entries in a line of data.
 
NAME:
	CheckLengths()

SYNOPSIS:

C:
	int CheckLengths(
		char		*line,
		int		delPos,
		int		maxChars);

FORTRAN:
	NONE

DESCRIPTION:
	Search a line for a certain string and determine if it contains
	too many characters.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of Process Control Information
			data.

	delPos		Delimiter position to start counting
			the number of characters.

	maxChars	Maximum number of characters
			allowed for this string.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	OK			The string is within the limits
	PGS_PC_BAD		The string is too long
	PGS_PC_ZERO_LENGTH	The string has length of zero

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/

int CheckLengths(                 /* check the length of a string */
    char *line,                   /* line to check */
    int delPos,                   /* which delimiter to start at */
    int maxChars)                 /* maximum number of characters allowed */
{
    int pos;                      /* current line position */
    int delCount;                 /* number of delimiters counted */
    int charCount;                /* number of characters counted */
    int retVal;                   /* function return value */

/***************************************************************************
*    Initialize counters.
***************************************************************************/
    delCount = 0;
    pos = 0;
    charCount = 0;

/***************************************************************************
*    Loop until we hit the number of delimiters that we are supposed to
*    hit before counting characters.
***************************************************************************/
    while (delCount < delPos)
    {
        if (line[pos] == PGSd_PC_DELIMITER)
        {
            delCount++;
        }

/***************************************************************************
*    If we hit a newline character then we have a problem that should     
*    have been flagged before this point.  We do not want to bog the 
*    user down with repeating error messages so let's just go back to 
*    the calling function.
***************************************************************************/
       else if (line[pos] == PGSd_PC_NEWLINE)
       {
           return OK;
       }

       pos++;

    }   /* end while */

/***************************************************************************
*    Loop until we hit a delimiter or a newline character here.  We don't
*    care what it is that we hit first, once again if it is a problem
*    then it should have been nailed before here and we do not want to 
*    repeat messages.
***************************************************************************/
    while ((line[pos] != PGSd_PC_DELIMITER) &&
           (line[pos] != PGSd_PC_NEWLINE) &&
           (line[pos] != PGSd_PC_CHAR_NULL))
    {
        charCount++;
        pos++;
    }

/***************************************************************************
*    Check the length here.  The reason that we have greater than or 
*    equal to (>=) here is to account for the terminating NULL character.
*    If it is a zero send it back.  This is only used in file name checks.
*    (DR ECSed001827 31-Jul-96)
***************************************************************************/
    if (charCount >= maxChars)
    {
        retVal =  PGS_PC_BAD;
    }
    else if (charCount == 0)
    {
        retVal = PGS_PC_ZERO_LENGTH;
    }
    else
    {
        retVal =  OK;
    }

/***************************************************************************
*    And away we go.
***************************************************************************/
    return retVal;
}





/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Check the indexes for repeated indexes (or indices).
 
NAME:
	CheckIndexes()

SYNOPSIS:

C:
	int CheckIndexes(
		PGSt_PC_Logical	newValue,
		PGSt_PC_Logical	valueArray[],
		int		*arrayCount,
		int		location);

FORTRAN:
	NONE

DESCRIPTION:
	Check the indexes for repeats.  The Standard Input and output 
	files are allowed a one-to-many relationship and may have more 
	than one file listed under the same index.  But, they must be 
	listed consecutively.  The same index may be used for a file 
	and a user-defined configuration item.  But the same index may 
	not be used more than once for any other combination.
 
INPUTS:
	Name		Description			Units	Min	Max

	newValue	The index value to check for.

	valueArray	Array of index values already
			being used.

	arrayCount	Number of index values in the 
			array.

	location	Location in file that the index
			value was found.

OUTPUTS:
	Name		Description			Units	Min	Max

	NONE

RETURNS:
	OK		index value is OK
	PGS_PC_BAD	index value is illegally repeated

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


int CheckIndexes(                        /* check for repeat indexes */
    PGSt_PC_Logical   newValue,          /* new index value */
    PGSt_PC_Logical   valueArray[],      /* array of index values */
    int              *arrayCount,        /* number of index values in array */
    int               location)          /* location in file of index value */
{
    int            loop;                 /* loop counter */
    int            retValue;             /* function return value */

/***************************************************************************
*    Loop through the array of index values.
***************************************************************************/
    for (loop = 0; loop < *arrayCount; loop++)
    {

/***************************************************************************
*    Check to see if the index value does not already exist.  If it does
*    then let's set our return value and jump out of the loop.
***************************************************************************/
        if (newValue == valueArray[loop])
        {
            retValue = PGS_PC_BAD;
            break;
        }
    }   /* end for */

/***************************************************************************
*    If we hit a match it may not be completely bad.  If the type of
*    index we are looking at is PRODUCT INPUT, PRODUCT OUTPUT, SUPPORT 
*    INPUT, or SUPPORT OUTPUT FILES and the match was on the last index 
*    value then it is OK.  We are going to allow a one-to-many relationship 
*    with PRODUCT INPUT, PRODUCT OUTPUT, SUPPORT INPUT, and SUPPORT OUTPUT 
*    FILES but they must be listed consecutively.
***************************************************************************/
    if ((retValue == PGS_PC_BAD) && 
       ((location == PGSd_PC_INPUT_FILES) || 
        (location == PGSd_PC_OUTPUT_FILES) ||
        (location == PGSd_PC_SUPPORT_INPUT) || 
        (location == PGSd_PC_SUPPORT_OUTPUT)) && 
        (loop == (*arrayCount - 1)))
    {
        retValue = OK;
    }

/***************************************************************************
*    Everything was OK, lets place the new index value in the array and
*    increment the array counter.
***************************************************************************/
    else
    {
        valueArray[*arrayCount] = newValue;
        *arrayCount = *arrayCount + 1;
    }

/***************************************************************************
*    I am just using basic arrays to store the index values, so it is
*    possible to exceed these bounds.  If they do we do not want this
*    puppy to explode, so we will tell them here how to go about   
*    increasing the number of index values that they can check and then
*    exit.
***************************************************************************/
    if (*arrayCount >= MAX_INDEXES)
    {
        printf("Exceeded the number of allowable indexes.  The check\n");
        printf("program will exit here.  To properly check this file\n");
        printf("increase the number in the #define MAX_INDEXES and\n");
        printf("recompile this code.  Allowing this program to continue\n");
        printf("as the program is set up now would cause a core dump.\n");
        printf("Currently, the program is set up to allow up to %d\n",
                MAX_INDEXES);
        printf("User Defined Configuration Paramters and up to %d File\n",
                MAX_INDEXES); 
        printf("Logicals.  As it is now both of these numbers come\n");
        printf("from the same root and should be identical.\n");
        exit(0);
    }

/***************************************************************************
*    Back to the future!!!
***************************************************************************/
    return retValue;
}






/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Check a line of default file location data.
 
NAME:
	CheckDefaultLoc()

SYNOPSIS:

C:	int CheckDefaultLoc(
		char		*line,
		char		*defLoc);

FORTRAN:
	NONE

DESCRIPTION:
	Check a line of default file location data to ensure that a
	string actually exists on the line.
 
INPUTS:
	Name		Description			Units	Min	Max

	line		Line of Process Control Information
			data.

OUTPUTS:
	Name		Description			Units	Min	Max

	defLoc		The actual default file location
			data.

RETURNS:
	OK		all data is OK 
	WARNING		may be a problem with the data
	PGS_PC_BAD	error with data

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:  
	NONE

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	NONE

END_PROLOG:
***************************************************************************/


int CheckDefaultLoc(              /* check line of default file location data */
    char     *line,               /* line read in from file */
    char     *defLoc)             /* default file location */
{
    int       linePos;            /* line position */
    int       defLocPos;          /* default location position */
    int       count;              /* for loop counter */
    int       returnVar;          /* function return */

/***************************************************************************
*    Initialize variables.
***************************************************************************/
    returnVar = OK;

/***************************************************************************
*    Find the first character of data.
***************************************************************************/
    linePos = 1;
    while ((!(isgraph(line[linePos]))) && (line[linePos] != PGSd_PC_NEWLINE))
    {
        linePos++;
    }
 
/***************************************************************************
*    Copy the data into the default location string.
***************************************************************************/
    defLocPos = 0;
    while (line[linePos] != PGSd_PC_NEWLINE)
    {
        defLoc[defLocPos] = line[linePos];
        defLocPos++;
        linePos++;
    }

/***************************************************************************
*    Ensure that the string has data and does not contain embedded 
*    blanks.
***************************************************************************/
    if (defLocPos > 0)
    {
        defLoc[defLocPos] = PGSd_PC_CHAR_NULL;
        for (count = 0; count < (int) strlen(defLoc); count++)
        {
            if (defLoc[count] == BLANK)
            {
                returnVar = WARNING;
                break;
            }
        }
    }
    else
    {
        returnVar = PGS_PC_BAD;
    }

/***************************************************************************
*    Return to calling function.
***************************************************************************/
    return returnVar;
}
