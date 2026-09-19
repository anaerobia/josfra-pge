/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
    PGS_CBP_ASCtoBIN.c

DESCRIPTION:
    This file contains PGS_CBP_ASCtoBIN.c which creates de200.eos binary JPL 
    Planetary Ephemeris file from a specially formatted text file "de200.dat".


AUTHOR:
    Snehavadan Macwan / Applied Research Corp.

HISTORY:
    03-Jun-1994		SM	Initial Version - converted from Fortran
    31-Aug-1994		SM 	Updated prologs, improved error handling
    27-Feb-2017   Abe Taaheri   Added data for another 20 to de200.dat extending
                                Final Epoch from JED=  2459215.5 2021 JAN 01 00:00:00
                                to JED=  2466543.5 2041 JAN 24 00:00:00
END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
    Create binary JPL Planetary Ephemeris file 

NAME: 
    PGS_CBP_ASCtoBIN 

SYNOPSIS:
    At the UNIX prompt type name of the executable followed by the file 
    "de200.dat". 
     
DESCRIPTION:
    This program creates "de200.eos" binary JPL Planetary Ephemeris file from a 
    specially formatted text file "de200.dat".

INPUTS:
    de200.dat 		text file

OUTPUTS:
    de200.eos		binary file
          
RETURNS:
    returns 0 for the successful completion and 1 for any of the error messages

EXAMPLES:
    % PGS_CBP_ASCtoBIN  de200.dat

      
    
NOTES:
    The text file "de200.dat" which contains header information in text form 
    followed by the actual ephemeris data -- also in text form must be supplied 
    at the command line.
 
    By default, the output ephemeris spans the time interval from startTime = 
    Dec 2, 1957 0 hrs to stopTime = Jan 24, 2041 0 hrs. The values of startTime 
    and stopTime are hard coded in this program. Their values can be changed to 
    include any time interval that is within the time interval covered by the 
    text file "de200.dat". startTime and stopTime must be specified in Julian 
    Ephemeris Days (jedTDB).
    
REQUIREMENTS:
    PGSTK-0800

DETAILS:
    N/A

GLOBALS:
    none
			  
FILES:
    none

FUNCTIONS CALLED:
    PGS_CBP_NextGroup()

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_SMF.h>

#define   MAXSIZE      200   /* maximum array size expected */
#define   SUCCESS        0   /* succeful return state */
#define   READ_ERROR   666   /* error reading input file */

/* Declaration of a  function */

PGSt_integer 
PGS_CBP_NextGroup(
    FILE           *fp1,
    char           buf[MAXSIZE]); 

/* main */

int
main(
    int            argc,
    char           *argv[])
{
    FILE           *fp;                   /* file pointer for input file */
    FILE           *fpp;                  /* file pointer for output file */

    struct         struct_type	          /* Structure for header record */
    {
	PGSt_uinteger 	number;	          /* Record number */
	char		ttl[3][84];       /* Title lines */
	char		cnam[MAXSIZE][8]; /* Names of the constants */
	PGSt_double	ss[3];	          /* Start, end and record span */
	PGSt_integer	ncon;	          /* Number of constants */
	PGSt_double	au;	          /* Number of kilometers per 
					     astronomical unit */
	PGSt_double    emrat;	          /* Earth Moon ratio */
	PGSt_integer   ipt[12][3];        /* Pointers needed by INTERP */
	PGSt_integer   numde;	          /* Planetary ephemeris 
					     number */
	PGSt_integer   lpt[3];	          /* Pointers needed by INTERP */
    }headerRec;
    
    struct        cvalType	          /* Structure for constants record */
    {
	PGSt_uinteger  number;	          /* Record number */
	PGSt_double    cval[MAXSIZE];     /* Values of the ephemeris 
					     constants */
    }cvalRec;

    
    struct        dataType	          /* Structure for ephemeris records */
    {
	PGSt_uinteger number;             /* Record number */
        PGSt_double   db[826];            /* Ephemeris data */
    }dataRec;

    char	  buf[MAXSIZE];           /* buffer to store line read from
					     input file */
    char          numbuff[3][100];        /* buffer to store ascii numbers */
    char	  header[MAXSIZE];        /* buffer to store header line read 
					     from input file */
		
    PGSt_double   db2z = 0.0;             /* time of the previous block */
    PGSt_double	  startTime;              /* jedTDB start time */
    PGSt_double   stopTime;               /* jedTDB stop time */
		
    PGSt_integer  counter;                /* loop counter */
    PGSt_integer  count;	          /* loop counter */
    PGSt_integer  cnt;	                  /* loop counter */
    PGSt_integer  cntr;                   /* loop counter */
    PGSt_integer  ksize;	          /* size of ephemeris record */
    PGSt_integer  ncon;	                  /* number of constants */
    PGSt_integer  ncoeff;	          /* number of coefficients */
    PGSt_integer  nrw;	                  /* nonoverlapped record number */
    PGSt_integer  nrout = 0;              /* counter for number of records 
					     written */
    PGSt_integer  out = 0;                /* checks if error occurred on writing
					     a record to the binary file */
    PGSt_uinteger line = 0;               /* line number */
    		
    PGSt_boolean  first = PGS_TRUE;       /* flag for the first applicable
					     interval */
		
    PGSt_integer  returnStatus = 0;       /* indicates the default success
					     status */
   

    /* By default, the output ephemeris will span the same interval as the
       input text file. Set startTime and stopTime to the begin and end times 
       of the desired span. */

    startTime = 2436175.50E0;	/* Dec 2, 1957 0 hrs */
    stopTime = 2466543.50E0;	/* Jan 24, 2041 0 hrs */
    
    /** Check if the command line argument is < 2. If it is < 2, print error 
        message and return with error status. **/

    if (argc < 2)
    {
	fprintf (stderr, "\nUSAGE: %s <file name>\n", argv[0]);
	fprintf (stderr, "    where <file_name> is the name of the ASCII"
		         " JPL ephemeris\n    file to be converted to binary"
		         " form\n\n");
	returnStatus = 1;
	return returnStatus;
    }

    /** Check if the input file is readable. If unable to read input file, 
        print error message and return with error status. **/

    if (( fp = fopen(argv[1], "r")) == NULL)
    {
	fprintf (stderr, "%s : Can't open %s\n", argv[0], argv[1]);
	returnStatus = 1;
	return returnStatus;
    }

    /** Check if the output file already exists. If it does exist, print an 
	error message, and return with error status. To create a new version of
	the output file, user must remove existing output file and then run this 
	program. **/

        if (( fpp = fopen("de200.eos", "r")) != NULL)
        {
	     fprintf (stderr, "Output file de200.eos EXISTS \n");

		returnStatus = 1;
		return returnStatus;
	}

	

    /** Check if the output file can be opened. If unable to open output file, 
        print error message and return with error status. **/

    if (( fpp = fopen("de200.eos", "wb+")) == NULL)
    {
	fprintf (stderr, "Can't open de200.eos \n");
	returnStatus = 1;
	return returnStatus;
    }

    /** Read the size of main ephemeris records **/

    if (fgets(buf, sizeof(buf), fp) == NULL)
    {
	fprintf (stderr, "error reading input file '%s'\n",argv[1]);
	fclose(fp);
	fclose(fpp);
	return 1;
    }

    sscanf(buf, "%*s %d\n", &ksize);

    /** Skip the header line "GROUP 1010" and then read the alphameric heading 
        records. **/

    returnStatus = PGS_CBP_NextGroup(fp,header);
    
    if (returnStatus == SUCCESS)
    {
	if (strncmp(header,"GROUP   1010", 12) != 0)
	{
	    fprintf (stderr, "Header Group 1010 NOT Found\n");
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
    }
    else
    {
	fprintf(stderr, "error reading input file '%s'\n", argv[1]);
	fclose(fp);
	fclose(fpp);
	return 1;
    }
    
    /** Read three title lines from the text file  **/

    for (counter = 0; counter < 3; counter++)
      if (fgets(headerRec.ttl[counter], sizeof(headerRec.ttl[counter]), fp)
	  == NULL)
      {
	  fprintf (stderr, "error reading input file '%s'\n",argv[1]);
	  fclose(fp);
	  fclose(fpp);
	  return 1;
      }
	
    /** Skip the header line "GROUP 1030" and then read start, end and record 
        span. **/

    returnStatus = PGS_CBP_NextGroup(fp,header);
    
    if(returnStatus == SUCCESS)
    {
        if (strncmp(header, "GROUP   1030", 12) != 0)
	{
	    fprintf (stderr, "Header Group 1030 NOT Found \n");
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
    }
    else
    {
	fprintf(stderr, "error reading input file '%s'\n", argv[1]);
	fclose(fp);
	fclose(fpp);
	return 1;
    }

    /* Read start, stop and record span and store them in "ss" field of the 
       headerRec structure. */

    if (fgets (buf, sizeof(buf), fp) == NULL)
    {
        fprintf (stderr, "error reading input file '%s'\n",argv[1]);
        fclose(fp);
        fclose(fpp);
        return 1;
    }

    sscanf (buf, "%lf %lf %lf\n", &headerRec.ss[0], &headerRec.ss[1], 
	    &headerRec.ss[2]);


    /** Skip the header line "GROUP 1040" and then read number of constants and
        names of constants. **/

    returnStatus = PGS_CBP_NextGroup(fp, header);
    if (returnStatus == SUCCESS)
    {
	if (strncmp(header, "GROUP   1040", 12) != 0)
	{
	    fprintf (stderr, "Header  Group 1040 NOT Found\n");
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
    }
    else
    {
	fprintf(stderr, "error reading input file '%s'\n", argv[1]);
	fclose(fp);
	fclose(fpp);
	return 1;
    }
        
    /* Read name of the constants and store them in "cnam" field of the 
       headerRec structure. */

    if (fgets(buf, sizeof(buf), fp) == NULL)
    {
        fprintf (stderr, "error reading input file '%s'\n",argv[1]);
        fclose(fp);
        fclose(fpp);
        return 1;
    }
    
    sscanf(buf, "%d\n", &headerRec.ncon);

    for (counter = 0; counter < headerRec.ncon; counter+=10)
    {
	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
	    fprintf (stderr, "error reading input file '%s'\n",argv[1]);
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
	
	sscanf(buf,"%s %s %s %s %s %s %s %s %s %s\n",headerRec.cnam[counter],
	       headerRec.cnam[counter+1],headerRec.cnam[counter+2],
	       headerRec.cnam[counter+3],headerRec.cnam[counter+4],
	       headerRec.cnam[counter+5],headerRec.cnam[counter+6],
	       headerRec.cnam[counter+7],headerRec.cnam[counter+8],
	       headerRec.cnam[counter+9]);
	
    }

    /** Skip the header line "GROUP 1041" and then read number of constants and
        constants' values.  **/

    returnStatus = PGS_CBP_NextGroup(fp, header);
    
    if (returnStatus == SUCCESS)
    {
	if (strncmp(header, "GROUP   1041", 12) != 0)
	{
	    fprintf (stderr, "Header Group 1041 NOT Found \n");
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
    }
    else
    {
	fprintf(stderr, "error reading input file '%s'\n", argv[1]);
	fclose(fp);
	fclose(fpp);
	return 1;
    }
    
    /* Read the number of constants. */
 
    if (fgets(buf, sizeof(buf), fp) == NULL)
    {
        fprintf (stderr, "error reading input file '%s'\n",argv[1]);
        fclose(fp);
        fclose(fpp);
        return 1;
    }

    sscanf(buf, "%d\n", &ncon);

    /* Read one line of constants' values as string. In this line, search for 
       D which is double precision D in Fortran. Replace character D with E 
       which is a double E in C. This replacement is essential as to maintain 
       precision. Then break down text line into three double type data values
       and store them in "cval" field of the cvalRec structure. Do it for all 
       the constants' values found in this record. */
    
    for (counter = 0; counter < ncon; counter+=3)
    {
	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
	    fprintf (stderr, "error reading input file '%s'\n",argv[1]);
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
       
	for(count = 0; count < (PGSt_integer) strlen(buf); count++) 
        {

	    /** Replace double precision D in Fortran to double E in C **/

            if(buf[count] == 'D') 
	      buf[count] = 'E';
        }
	sscanf(buf,"%s %s %s",numbuff[0],numbuff[1],numbuff[2]);
	
	for (cntr=0;cntr<3 && (cntr+counter)<ncon;cntr++)
		  sscanf(numbuff[cntr], "%lf", &cvalRec.cval[counter+cntr]);

    }
    
    /** Assign values to AU, EMRAT, and DENUM constants **/

    for (counter = 0; counter < ncon; ++counter)
    {
	if (strncmp(headerRec.cnam[counter],"AU",2) == 0)
	  headerRec.au = cvalRec.cval[counter];

	if (strncmp(headerRec.cnam[counter], "EMRAT",5) == 0)
	  headerRec.emrat = cvalRec.cval[counter];

	if (strncmp(headerRec.cnam[counter], "DENUM",5) == 0)
	  headerRec.numde = (PGSt_integer) cvalRec.cval[counter];

    }
  

    /** Skip the header line "GROUP 1050" and then read pointers needed for 
        interpolation. **/

    returnStatus = PGS_CBP_NextGroup(fp, header);
    
    if (returnStatus == SUCCESS)
    {
	if (strncmp(header, "GROUP   1050", 12) != 0)
	{
	    fprintf (stderr, "Header Group 1050 NOT Found \n");
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
    }
    else
    {
	fprintf(stderr, "error reading input file '%s'\n", argv[1]);
	fclose(fp);
	fclose(fpp);
	return 1;
    }
    
    count = -1;
     
    cnt  = -1;
    
    /* There are 13 integer pointer values /line in text file. Read this line as
       string. Break down it into 13 integer pointer values and store them 
       appropriately. NOTE that first 1 to 12 pointer values are stored in "ipt"
       field and the 13th value is stored in "lpt" field of the headerRec 
       structure. */

    for (counter = 0; counter < 3; counter++)
    {
        if (fgets(buf,200,fp) == NULL)
	{
	    fprintf (stderr, "error reading input file '%s'\n",argv[1]);
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}

        sscanf(buf,"%d %d %d %d %d %d %d %d %d %d %d %d %d",
	       &headerRec.ipt[0][count+1],&headerRec.ipt[1][count+1],
	       &headerRec.ipt[2][count+1],&headerRec.ipt[3][count+1],
	       &headerRec.ipt[4][count+1],&headerRec.ipt[5][count+1],
	       &headerRec.ipt[6][count+1],&headerRec.ipt[7][count+1],
	       &headerRec.ipt[8][count+1],&headerRec.ipt[9][count+1],
	       &headerRec.ipt[10][count+1],&headerRec.ipt[11][count+1],
	       &headerRec.lpt[cnt+1]);

        count += 1;
        cnt += 1;

    }
	
    /* Assign a numeric value to record number for header record. */

    headerRec.number = (PGSt_uinteger) line;

    /* Write header record to binary file. Check that this record is 
       successfully written to the binary file. If there is an error in writing
       this record, print an error message and return with an error status. */
    
    out = fwrite (&headerRec, sizeof(struct struct_type), 1, fpp);
    
    if (out != 1)
    {
	fprintf (stderr, "1st record not written because of error\n");
	returnStatus = 1;
	return returnStatus;
    }
    
    /* Increment record number counter and assign it to constants' values 
       record. */ 

    line += 1;
    
    cvalRec.number = (PGSt_uinteger) line;
    
    /* Write constants' values  record to binary file. Check that this record 
       is successfully written to the binary file. If there is an error in 
       writing this record, print an error message and return with an error 
       status. */

    out = fwrite (&cvalRec, sizeof(struct cvalType), 1, fpp);
    
    if (out != 1)
    {
	fprintf (stderr, "2nd record not written because of error\n");
	returnStatus = 1;
	return returnStatus;
    }
     
    
    
    /* Skip the header line "GROUP 1070" and then read ephemeris data records.*/

    
    returnStatus = PGS_CBP_NextGroup(fp, header);
    
    if (returnStatus == SUCCESS)
    {
	if (strncmp(header, "GROUP   1070", 12) != 0)
	{
	    fprintf (stderr, "Header Group 1070 NOT Found \n");
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
    }
    else
    {
	fprintf(stderr, "error reading input file '%s'\n", argv[1]);
	fclose(fp);
	fclose(fpp);
	return 1;
    }

    /* Read nonoverlapped record number and number of coefficients. */

    if (fgets(buf, sizeof(buf), fp) == NULL)
    {
        fprintf (stderr, "error reading input file '%s'\n",argv[1]);
        fclose(fp);
        fclose(fpp);
        return 1;
    }
    
    sscanf(buf, "%d %d\n", &nrw, &ncoeff);
    
    /* Read one line of ephemeris data as string. In this line, search for 
       D which is double precision D in Fortran. Replace character D with E 
       which is a double E in C. This replacement is essential as to maintain 
       precision. Then break down text line into three double type data values
       and store them in "db" field of the dataRec structure. Do it for all 
       the ephemeris data found in this record. */   
    
    for (counter = 0; counter < ncoeff; counter+=3)
    {
	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
	    fprintf (stderr, "error reading input file '%s'\n",argv[1]);
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
	for (count = 0; count < sizeof(buf); count++)
	{
	    if (buf[count] == 'D')
	      buf[count] = 'E';
	}
	
	sscanf(buf,"%s %s %s",numbuff[0],numbuff[1],numbuff[2]);
	
	for (cntr=0;cntr<3 && (cntr+counter)<ncoeff;cntr++)
		  sscanf(numbuff[cntr], "%lf", &dataRec.db[counter+cntr]);
    }
    
    /* Read and write all the ephemeris data records found within the startTime
       and stopTime. */ 

    do
    {
	if (2*ncoeff != ksize)
	{
	    fprintf (stderr, "Invalid ephemeris record size\n");
	    returnStatus = 1;
	    return returnStatus;
	}
	
	/** Skip this data block if the end of the interval is less than the 
	    specified start time or if it does not begin where the previous
	    block ended. **/

	if ((dataRec.db[1] >= startTime) && (dataRec.db[0] >= db2z))
	{
	    if (first == PGS_TRUE)
	    {
		/** Don't worry about the intervals overlapping or abutting
		    if this is the first applicable interval. **/

		db2z = dataRec.db[0];
		first = PGS_FALSE;
	    }
	    
	    if (dataRec.db[0] != db2z)
	    {
		/** Beggining of current interval is past the end of the 
		    previous one. **/

		fprintf (stderr, "%d\t", nrw);
		fprintf (stderr, "Records do not overlap or abut\n");
		returnStatus = 1;
		return returnStatus;
	    }
	    
	    /* Save this records end time */
	    
	    db2z = dataRec.db[1];
	    
	    /* Initialize the number of records written counter and record 
	       number counter */
	    
	    nrout += 1;
	    line += 1;
	    
       	    dataRec.number = (PGSt_uinteger) line;
	    
	    /** Write records to output file **/
	    
	    /* Write ephemeris data record to binary file. Check that this 
	       record is successfully written to the binary file. If there is 
	       an error in writing this record, print an error message and 
	       return with an error status. */

	    out = fwrite(&dataRec, sizeof(struct dataType), 1, fpp);
	    
	    if (out != 1)
	    {
		fprintf (stderr, "%3d", nrout);
		fprintf (stderr, "\'th record not written because of error\n");
		fclose(fp);
		fclose(fpp);
		returnStatus = 1;
		return returnStatus;
	    }
	    
	    /** Save this block's starting date, ite interval span, and its 
	        end date. **/

	    if (nrout == 1)
	    {
		headerRec.ss[0] = dataRec.db[0];
		headerRec.ss[2] = dataRec.db[1] - dataRec.db[0];
	    }
	    
	    headerRec.ss[1] = dataRec.db[1];
	    

	    /** Update the user as to our progress every 10th block. **/

	    if ((nrout % 10) == 1)
	    {
		if (dataRec.db[0] >= startTime)
		{
		    fprintf (stdout, "%3d\t",nrout-1);
		    fprintf (stdout, 
			     "EPHEMERIS RECORDS WRITTEN.  "
			     "LAST JED = \t%12.2f\n",
			     dataRec.db[1]);
		}
		
		else 
		  fprintf (stdout, 
			   "Searching for first requested record .........\n");
	    }
	}
	
	/* Read nonoverlapped record number and number of coefficients. */

	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
	    fprintf (stderr, "error reading input file '%s'\n",argv[1]);
	    fclose(fp);
	    fclose(fpp);
	    return 1;
	}
    
	sscanf(buf, "%d %d\n", &nrw, &ncoeff);
    
	/* Read one line of ephemeris data as string. In this line, search for 
	   D which is double precision D in Fortran. Replace character D with E 
	   which is a double E in C. This replacement is essential as to 
	   maintain precision. Then break down text line into three double type
	   data values and store them in "db" field of the dataRec structure. 
	   Do it for all the ephemeris data found in this record. */  
   
	for (counter = 0; counter < ncoeff; counter+=3)
	{
	    if (fgets(buf, sizeof(buf), fp) == NULL)
	    {
		fprintf (stderr, "error reading input file '%s'\n",argv[1]);
		fprintf (stdout,"LAST VALID JED = \t%12.2f\n",db2z);
		fclose(fp);
		fclose(fpp);
		return 1;
	    }
	    for (count = 0; count < sizeof(buf); count++)
	    {
		if (buf[count] == 'D')
		  buf[count] = 'E';
	    }
	    sscanf(buf,"%s %s %s",numbuff[0],numbuff[1],numbuff[2]);
	    
	    for (cntr=0;cntr<3 && (cntr+counter)<ncoeff;cntr++)
	      sscanf(numbuff[cntr], "%lf", &dataRec.db[counter+cntr]);
	}
    } while (dataRec.db[1] < stopTime);

    fprintf (stdout, "%3d\t", nrout);
    fprintf (stdout, "EPHEMERIS RECORDS WRITTEN.  LAST JED = \t%12.2f\n", 
	     dataRec.db[1]);  

    /* Rewind the binary file and rewrite the header record because "ss" field in 
       headerRec structure is changed. */

    rewind(fpp);

    headerRec.number = 0;
   
    out = fwrite (&headerRec, sizeof(struct struct_type), 1, fpp);
    
    if (out != 1)
    {
	fprintf (stderr, "1st record not written because of error\n");
	returnStatus = 1;
	return returnStatus;
    }
    
    fclose(fp);
    fclose(fpp);

    return returnStatus;
    
}

/* This function reads the header line information from the text file while 
   skipping the blank line. It returns the header line information . */

PGSt_integer
PGS_CBP_NextGroup(
    FILE         *fp1, 		   /* File pointer for the text file. */
    char         buf[MAXSIZE])     /* buffer for the line read */
{    
    char	 tempStr[MAXSIZE]; /* temporary buffer to hold text
				      line information */
    char         *readError;       /* return value of call to fgets */
    char         *cptr;            /* character pointer */
    
    size_t       blanks;           /* counts empty space in string */
    
    PGSt_integer returnStatus;     /* return status for this 
				      function */

    /* Initialize return value to indicate success */

    returnStatus = SUCCESS;
    
    /* Read one line of text file at a time. If the line is blank, skip it. 
       If the line contains string of characters read it in tempStr and read
       any blank lines that follow it.  Note that a "blank" line is any line
       that contains only ' ' (blanks) '\t' (tabs) or '\n' (newline). */
    
    do
    {
	readError = fgets(tempStr,sizeof(tempStr),fp1);
	if((cptr=strchr(tempStr,'\n')) != NULL)
	  *cptr = '\0';
	blanks = strspn(tempStr,"\t ");
	
    } while (readError != NULL && strlen(tempStr+blanks) == 0);
    
    if (readError != NULL)
    {
	strcpy(buf,tempStr);
	readError = fgets(tempStr,sizeof(tempStr),fp1);
    }
    
    if (readError == NULL)
      returnStatus = READ_ERROR;
    
    return returnStatus; 
}
