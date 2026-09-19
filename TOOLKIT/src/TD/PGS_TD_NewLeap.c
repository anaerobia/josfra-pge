/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:
 

FILENAME:

DESCRIPTION:
      This file contains the function PGS_TD_NewLeap()

AUTHOR:
      Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
      16-Jun-1994   PDN          designed and wrote
      23-Jun-1994   PDN          interim 
      23-Jun-1994   PDN          finished
      01-Jul-1994   PDN          tidied up - removed duplicate code
      20-Mar-1995   PDN          changed error action for better use by script
      20-Aug-1996   PDN          complete rewrite to use Naval Observatory data
                                 instead of IERS (Int'l Earth Rotation Svce).
      10-Oct-1996   PDN          rewrite DAAC version to omit predicted leap
                                 seconds and to use IO tools to open/close
      25-Mar-1997   PDN          Fixed problem with writing an extra line on
                                 some SGI platforms; fixed several return
                                 values to be more informative; checked closing
                                 of files in case of abortion. Improved messaging
                                 to logfile
      10-Jul-1997   PDN          Cleaned up unused variables; added checks for
                                 number of fields read
      11-Jul-1997   PDN          Added checks of lengths of lines written
      04-Sep-1998   PDN          Fixed return status in case of header update only;
                                 altered message to LogStatus file 
                   
END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG
 
TITLE: 
      This function updates the file of leap seconds
      using the file "tai-utc.dat" from the U.S. Naval Observatory
      server "maia.usno.navy.mil", in the directory "ser7"
  
NAME: 
      PGS_TD_NewLeap.c      

SYNOPSIS: 
C:     
  
DESCRIPTION:
      
INPUTS:
      Name       Description               Units           Min     Max
      ----       -----------               -----           ---     ---
     leapsec.dat     file - containing a header line and the following
     fields in each later line:
     yearl		year
     monthl		month 
     dayl		day
     jd        		UTC Julian Date leap second is effective
     leapseconds	leap second value (cumulative)
     slope		slopes used to compute leap seconds values
					  for 1961 to 1971
     baseDay		constants to subtract from MJD in equation used to
				  compute leap seconds - 1961 to 1971
     predict		character string indicating whether the leap second 
				values are PREDICTED or ACTUAL 

OUTPUTS:
      Name       Description               Units            Min        Max
      ----       -----------                -----            ---       ---
     leapsec.dat.new  file - like leapsec.dat but updated by using "tai-utc.dat" 

RETURNS:
      PGS_S_SUCCESS                          
      PGSTD_W_DATA_FILE_MISSING
      PGSCBP_E_BAD_TIME_FORMAT 
      PGSPC_E_FILE_OPEN_ERROR
      PGSTD_M_HEADER_UPDATED
      PGS_E_TOOLKIT
      positive integers - various problems

EXAMPLES:

NOTES:

      This function is called by either script: 

         $PGSSRC/TD/update_leapsec.sh         or
         $PGSSRC/TD/update_leapsec_CC.sh  (Clear Case capable version)
      to update the leap seconds file 
         $PGSHOME/database/common/TD/leapsec.dat

      "MJD" means Modified Julian Date = Julian Date - 2400000.5.

      The method is to read in the data file "leapsec.dat", noting the date
      of the last leap second.  Then we read in the USNO file and examine 
      the last entry to see if it is for a later date than the 
      last entry in the leap seconds file .  If not, we just update the header 
      of the leap seconds file, indicating it was checked.

      If we do have a new leap second in the USNO file, we  update the header,
      copy all the USNO records into our file, and write the file.
 
      Leap seconds are normally announced 5 to 6 months in advance, and are 
      "ACTUAL" as soon as announced as there is no provision for rescission.
      It is  probably most convenient to run this function via its calling script 
      weekly, because new leap seconds can be announced on 90 days' notice if
      deemed necessary by the IERS.

      The "RETURNS" listed herein are actually temporary values used in writing
      to the LogStatus file; actually low integers are returned and interpreted
      by the calling script.

REQUIREMENTS:
      PGSTK - 1050, 0930 
 
DETAILS:
      NONE

GLOBALS:
      NONE

FILES:
      leapsec.dat
      tai-utc.dat

FUNCTIONS CALLED:
      PGS_SMF_SetStaticMsg()
      PGS_SMF_SetDynamicMsg()

END_PROLOG
*******************************************************************************/
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <PGS_SMF.h>
#include <PGS_TD.h>
#include <PGS_IO.h>

#define  FUNCTION_NAME "PGS_TD_NewLeap()"

/* logical identifier for leap seconds file leapsec.dat (defined in Process
   Control File) */

#define  LEAPSEC 10301

#define  MAX_HEADER      600
#define  MAX_LEAP_RECS   230
#define  ARRAYSIZE       250
#define  PREDCHAR    10
#define  JAN_1ST_1970  -725846418.999918

#define  SSTRING  "S + (MJD -"
#define  S2TRING  ") X"

int main()
{
    char argv[80];

/* THIS GROUP FOR READING IN and Converting USNO data - tai-utc.dat */

    double usno_leap = 0;     /* total leap seconds from tai-utc.dat */
    int    itry;              /* loop counter */
    int    year;              /* to get Julian Day */
    int    day;               /* to get Julian Day */

/* THIS GROUP FOR READING IN "leapsec.dat"  */

    int        yearl[ARRAYSIZE];       /* array of years in leapsec file */
    char       monthl[ARRAYSIZE][4];   /* array of months in leapsec file */
    int        dayl[ARRAYSIZE];        /* array of days in leapsec file */
    double     jdl[ARRAYSIZE];          /* array of Julian Day numbers in
					  the leap second table */
    int        yearu[ARRAYSIZE];       /* array of years in USNO file */
    char       monthu[ARRAYSIZE][4];   /* array of months in USNO file */
    int        dayu[ARRAYSIZE];        /* array of days in USNO file */
    double     jdu[ARRAYSIZE];          /* array of Julian Day numbers in
					  in USNO file */
    double     leapsecondsl[ARRAYSIZE]; /* array of leap second values in
					  leap second table */
    double     leapsecondsu[ARRAYSIZE]; /* array of leap second values in
					  USNO table */
    float      slope[ARRAYSIZE];       /* array of slopes used to
					  compute leap seconds values
					  for 1961 to 1971 */
    float      baseDay[ARRAYSIZE];     /* array of constants to subtract
					  from MJD in equation used to
					  compute leap seconds */  
    float      bD;                     /* from USNO for comparison */
    char predict[ARRAYSIZE][PREDCHAR];  /* character array formerly (SCF 
                                           Toolkit) indicating whether the 
                                           leap second values are PREDICTED 
                                           or ACTUAL; now must be ACTUAL */ 
    PGSt_uinteger numrecsl;              /* leap second file/table record
			                     number or array member counter
					     = 0 for header line, 1 for
                                                   first data line, etc.  */
    PGSt_uinteger numrecsu;              /* tai-utc.dat file/table record
			                     number or array member counter
					     = 0 for header line, 1 for
                                                   first data line, etc.  */
    PGSt_uinteger numrecsN;              /* record number or array member counter
                                            for new leap seconds file
					     = 0 for header line, 1 for
                                                   first data line, etc.  */
    int        yearN[ARRAYSIZE];        /* new array of years */
    char       monthN[ARRAYSIZE][4];    /* new array of months */
    int        dayN[ARRAYSIZE];         /* new array of days */
    double     jdN[ARRAYSIZE];          /* new array of Julian Day numbers in
					   the leap second table */
    double     leapsecondsN[ARRAYSIZE]; /* new array of leap second values in
					   leap second table */
    float      baseDayN[ARRAYSIZE];     /* new array of constants to subtract
                                           from MJD in equation used to
                                           compute leap seconds */  
    char predictN[ARRAYSIZE][PREDCHAR]; /* new character array indicating
					   whether the leap second values
					   are PREDICTED or ACTUAL; must be
                                           ACTUAL only in TK5 and beyond */ 

    PGSt_uinteger lastnum;              /* numrecsN - 1 (for writing table) */
    PGSt_uinteger lastACT;              /* last record with ACTUAL leap second */
    PGSt_boolean head_only = PGS_FALSE; /* TRUE => tai-utc.dat has no new leap 
                                           sec - update header only */
    
    PGSt_integer recordread;            /* fscanf return value */ 
    PGSt_integer recordwrit;            /* fprintf return value */ 

    PGSt_SMF_status returnStatus;       /* return status of this function */
    PGSt_SMF_status returnStatus1;      /* return status of PGS function calls */
    PGSt_SMF_status code;	    	/* code returned by PGS_SMF_GetMsg() */
		   
    char mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
						    PGS_SMF_GetMsg() */
    char msg[PGS_SMF_MAX_MSG_SIZE];	      /* message returned by
						    PGS_SMF_GetMsg() */
    char noFileMsg[PGS_SMF_MAX_MSG_SIZE];     /* message returned by
						    PGS_IO_Gen_Open() */

/* THIS GROUP FOR CREATING NEW "leapsec.dat"  */

    double   jdUTC;    /* date of last leap second (UTC) in USNO data */
 

    /* this group for leapsec.dat */
    char  field1L[10];  /* the year  of leap second - encode for doymjd.c */
    char  field2L[10];  /* the month of leap second - encode for doymjd.c */
    char  field3L[10];  /* the day   of leap second - encode for doymjd.c */
    char  field4L[10];  /* the characters "=JD " */
    /* end group for leapsec.dat */

    char *token;                   /* token for checking name of month
                                      in results of "dir" in "ftp" */
    FILE          *fp_in;          /* file pointer for leapsec.dat */
    FILE          *fp_usno;        /* file pointer for tai-utc.dat */
    FILE          *fp_next;        /* file pointer for leapsec.dat.new */


    char        *months[]=
                       {"Moth", /* humor */
                        "Jan",
                        "Feb",
                        "Mar",
                        "Apr",
                        "May",
                        "Jun",
                        "Jul",
                        "Aug",
                        "Sep",
                        "Oct",
                        "Nov",
                        "Dec"};
 

    char        *monthUC[]=     /* UPPER CASE MONTHS */
                       {"Illegal BYTES", /* humor */
                        "JAN",
                        "FEB",
                        "MAR",
                        "APR",
                        "MAY",
                        "JUN",
                        "JUL",
                        "AUG",
                        "SEP",
                        "OCT",
                        "NOV",
                        "DEC"};

    int        fileread;        /* file read error return returnStatus */
    char       new_leapf[50];

    char    headerl[MAX_HEADER];   /* used for reading in header lines */
    char    headeru[MAX_HEADER];  /* used for reading in USNO lines */
    char    head_out[MAX_HEADER]; /* used for writing new header line */
    char    update_t[60];
    char    asciiUTC[28];
    PGSt_double  secTAI93;
    time_t  clock;          /* system clock */
 
    char    specifics[PGS_SMF_MAX_MSG_SIZE]; /* detailed error msg */

    /* set message to indicate success */

    returnStatus = PGS_S_SUCCESS;

    /* Open old leap seconds file  */
    
	    returnStatus = PGS_IO_Gen_Open(LEAPSEC,PGSd_IO_Gen_Read,&fp_in,1);
/*  SCF version
            fp_in=fopen("leapsec.dat","r");
*/
            switch (returnStatus)
	    {
	      case PGS_S_SUCCESS:
		break;
	      case PGS_E_UNIX:
	      case PGSIO_E_GEN_OPENMODE:
	      case PGSIO_E_GEN_FILE_NOEXIST:
	      case PGSIO_E_GEN_REFERENCE_FAILURE:
	      case PGSIO_E_GEN_BAD_ENVIRONMENT:
		PGS_SMF_GetMsg(&code,mnemonic,noFileMsg);
		if (code != returnStatus)
		  PGS_SMF_GetMsgByCode(returnStatus,noFileMsg);
	      default:
		PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
		return 3;
	    }
	    
       if((fp_usno=fopen("tai-utc.dat","r")) == NULL) 
      {
        sprintf(specifics,
               "No USNO tai-utc.dat found; ftp failure\n");
        returnStatus = PGS_E_TOOLKIT;
        PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                    FUNCTION_NAME);
        PGS_IO_Gen_Close(fp_in);
        return 9;
    }

    /* Open new leapsec file */

    strcpy(new_leapf ,"leapsec.dat.new");

    if((fp_next =fopen(new_leapf,"w")) == NULL)
    {
        sprintf(specifics,
               "Unable to open new Leap Seconds file\n");
        returnStatus = PGS_E_TOOLKIT;
        PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                    FUNCTION_NAME);
        PGS_IO_Gen_Close(fp_in);
        PGS_IO_Gen_Close(fp_usno);
        return 4;
    }

    /* READ ATTEMPT - USNO FILE */

    /* initialize record counter to zero */

    numrecsu = 0;

    /*  Read first and later lines USNO data */

    /* in the next line the part after the && is to handle blank records at end 
       of tai-utc.dat - if any (\n's) */

    while (fgets(headeru,MAX_HEADER,fp_usno) != NULL 
             && (int) strlen(headeru) > 20 )
    {
        if (numrecsu == MAX_LEAP_RECS)
        {
            returnStatus = PGS_E_TOOLKIT;
            sprintf(specifics,"%s %d %s","leap seconds file exceeds ",
                            MAX_LEAP_RECS , "records ");
            PGS_SMF_SetDynamicMsg(returnStatus,
                                       specifics,
                                       FUNCTION_NAME);
            PGS_IO_Gen_Close(fp_in);
            PGS_IO_Gen_Close(fp_usno);
            PGS_IO_Gen_Close(fp_next);
            return 13;
        }

        /*  GET YEAR, MONTH, DAY USNO data - decode into integers */

        fileread = sscanf(headeru,"%s %s %s %s",field1L,field2L,field3L,field4L);
        if (fileread < 4)
        {
            returnStatus = PGS_E_TOOLKIT;
            sprintf(specifics,
             "error in reading 1st 4 fields found in USNO data line %d\n",numrecsu);
            PGS_SMF_SetDynamicMsg(returnStatus,
                                       specifics, FUNCTION_NAME);
            PGS_IO_Gen_Close(fp_in);
            PGS_IO_Gen_Close(fp_usno);
            PGS_IO_Gen_Close(fp_next);
            return 14;
        }


        year = atoi(field1L);
        yearu[numrecsu] = year;
        strcpy(monthu[numrecsu],field2L);
        day = atoi(field3L);
        dayu[numrecsu] = day;

        /*  OMIT YEAR, MONTH, DAY - GET REST OF USNO DATA on same line */

        recordread = sscanf(headeru, "%*d %*s %*d %*s %lf %*s %lf %*s %*s %*s"
                            "%*s %f%*s %*s %*f %*s", &jdu[numrecsu],
                             &leapsecondsu[numrecsu], &bD);
                             
       if (recordread < 3)
        {
            returnStatus = PGS_E_TOOLKIT;
            sprintf(specifics,
             "error in reading JD leapseconds, or base day in USNO data line %d\n",
                                                                         numrecsu);
            PGS_SMF_SetDynamicMsg(returnStatus,
                                       specifics, FUNCTION_NAME);
            PGS_IO_Gen_Close(fp_in);
            PGS_IO_Gen_Close(fp_usno);
            PGS_IO_Gen_Close(fp_next);
            return 14;
        }
 
        /* copy into new array as well, for output of leapsec.dat.new  */
        /* Numbering is incremented by one as our file has a header line */

        numrecsN = numrecsu + 1;

        yearN[numrecsN]        =  yearu[numrecsu];
        dayN[numrecsN]         =  dayu[numrecsu];
        strcpy(monthN[numrecsN],monthu[numrecsu]);
        jdN[numrecsN]          =  jdu[numrecsu];
        leapsecondsN[numrecsN] =  leapsecondsu[numrecsu];
        baseDayN[numrecsN]     =  bD;
	strcpy(predictN[numrecsN],"ACTUAL");

		/* be ready to complete file */
		numrecsu++;
	    } /* end while */
	    jdUTC = jdu[numrecsu-1];
	    usno_leap = leapsecondsu[numrecsu-1];

	    /*  close USNO file */
	  
	      returnStatus1 = PGS_IO_Gen_Close(fp_usno);
	      if (returnStatus1 != PGS_S_SUCCESS)
	      {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if (code != returnStatus1)
		PGS_SMF_GetMsgByCode(returnStatus1,msg);
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
                PGS_IO_Gen_Close(fp_in);
                PGS_IO_Gen_Close(fp_next);
		return 18;
	      }
	  
	    /* BEGIN leapsec.dat SECTION */

	    /* Read first record of OLD  file - it is to be stripped */

	    fgets(headerl,MAX_HEADER,fp_in);

	    /* initialize Leap Seconds record counter to zero */
	    /* do NOT count header line !! */
	    numrecsl = 0; 
		
	    /*  Read second and later lines */

	    /* The following && clause is to handle blank records at end 
	       of leapsec.dat - if any (\n's) */
	    while (fgets(headerl,MAX_HEADER,fp_in) != NULL &&
		  (int) strlen(headerl) > 20 )   
	    {
		numrecsl++;
		if (numrecsl == MAX_LEAP_RECS)
		{
		    returnStatus = PGS_E_TOOLKIT;
		    sprintf(specifics,"%s %d %s","leap seconds file exceeds ",
				    MAX_LEAP_RECS , "records ");
		    PGS_SMF_SetDynamicMsg(returnStatus,
					       specifics,
					       FUNCTION_NAME);
                    PGS_IO_Gen_Close(fp_in);
                    PGS_IO_Gen_Close(fp_next);
		    return 12;
		}

	    /*  GET YEAR, MONTH, DAY - decode into integers */

		fileread = sscanf(headerl,"%s %s %s %s",field1L,field2L,field3L,field4L);
		year = atoi(field1L);
		yearl[numrecsl] = year;
		strcpy(monthl[numrecsl],field2L);
		day = atoi(field3L);
		dayl[numrecsl] = day;

		/*  OMIT YEAR, MONTH, DAY - GET REST OF THE DATA */

		recordread = sscanf(headerl, "%*d %*s %*d %*s %lf %*s %lf %*s %*s %*s"
				    "%*s %f%*s %*s %f %*s %s", &jdl[numrecsl],
				     &leapsecondsl[numrecsl], &baseDay[numrecsl],
				     &slope[numrecsl],predict[numrecsl]);

		/* find last ACTUAL case */
        if(strcmp(predict[numrecsl],"ACTUAL") == 0) 
        {
             lastACT = numrecsl;
        }

    }  /* end while */ 

    lastnum = numrecsl;
    /* close file */
	
    returnStatus1 = PGS_IO_Gen_Close(fp_in);
    if (returnStatus1 != PGS_S_SUCCESS)
    {
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatus1)
	PGS_SMF_GetMsgByCode(returnStatus1,msg);
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
        PGS_IO_Gen_Close(fp_next);
        return 15;
    }
	
/*-------- Check Reasonableness and Treat Case of Header Update only ---------*/

    if(jdUTC ==  jdl[lastACT])
    { 
        /* Last USNO leap second same as ours -
        check tally OK and write new header and close files etc */
/* in case of negative ones best to check Julian Date AND leap seconds value */
        if(leapsecondsl[lastACT] == usno_leap && jdl[lastACT] == jdUTC)
        {
            head_only = PGS_TRUE;
            sprintf(specifics,
           "Last leap second in Toolkit file same as at USNO - header update only");

            returnStatus =  PGSTD_M_HEADER_UPDATED;
 
            PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                    FUNCTION_NAME);
            head_only = PGS_TRUE;
            returnStatus = PGS_S_SUCCESS;
        }
    }
      else if (jdUTC <  jdl[lastACT])
    { 
        sprintf(specifics,
        "New USNO leap second before our last actual one\n");
        returnStatus = PGS_E_TOOLKIT;
        PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                    FUNCTION_NAME);
        PGS_IO_Gen_Close(fp_next);
        return 7;
    }

    /* create new header */
    clock = time(NULL); /* get time as seconds since 12 am January 1st 1970 */

    secTAI93 = (PGSt_double) clock + JAN_1ST_1970; /* adjust time to TAI
                                                    (seconds since 12 am
                                                    January 1st, 1993) */
    PGS_TD_TAItoUTC(secTAI93, asciiUTC);
    
    strncpy(update_t, asciiUTC, 20);

    if(!head_only)
    {
       strcpy(head_out,"Updated: ");
    }else
    {
       strcpy(head_out,"Checked(unchanged): ");
    }
    strncat(head_out, asciiUTC, 19);
 
    strcat(head_out,"Z using USNO tai-utc.dat");

/* read string sent in from script */
   gets(argv);
 
/* at this point argv contains a typical result from the script,
   originating from "dir" on "maia.usno.navy.mil" . An example
   would be:             Feb 6 1996
   It would seem simple to use this in our new header. But we
   check that the first token is a month name.
*/
 
   token = strtok(argv," ");
 for (itry = 1; itry < 13; itry++)
         {
            if((strcmp(token,months[itry]) ==0) ||
            (strcmp(token,monthUC[itry]) ==0)) goto pop;
         }

   while((token = strtok(NULL," ")) !=NULL)
   {
      for (itry = 1; itry < 13; itry++)
         { 
            if((strcmp(token,months[itry]) ==0) ||
            (strcmp(token,monthUC[itry]) ==0)) goto pop;
         } 
   }
   /* If the code reaches here, there was no month name in the
       data from ftp - or it was the first field - impossible */
   sprintf(specifics,  
         "No month name found in token search %s\n",argv);
   returnStatus = PGS_E_TOOLKIT;
   PGS_SMF_SetDynamicMsg(returnStatus,specifics,
               "PGS_CSC_NewLeap()");
   PGS_IO_Gen_Close(fp_next);
   return 2; 
 
   pop: 
 
   strcat(head_out," file of ");
   strcat(head_out,token); 
   token = strtok(NULL," "); 
   strcat(head_out,"  "); 
   strcat(head_out,token); 
   token = strtok(NULL," "); 
   strcat(head_out,"  "); 
   strcat(head_out,token); 

   /* write header */

   recordwrit = fprintf(fp_next,"%s\n",head_out);

   if(recordwrit < 70 ) 
   /* If the code reaches here,  header is impossibly short*/
   {    
      sprintf(specifics,
         "Faulty file header construction: %s\n",head_out);
      returnStatus = PGS_E_TOOLKIT;
      PGS_SMF_SetDynamicMsg(returnStatus,specifics,
               "PGS_CSC_NewLeap()");
      PGS_IO_Gen_Close(fp_next);
      return 5;
   }    

   /*  if head_only write file and exit */

   if(head_only)
   {
      for(numrecsl = 1; numrecsl <= lastnum; numrecsl++)
          {
             if(jdl[numrecsl] <= 0.0) break;
             recordwrit = fprintf(fp_next,
             " %d %s  %d =JD%10.1f  TAI-UTC=  %10.7f %s %5.0f.%s %8.7f S   %s\n",
              yearl[numrecsl], monthl[numrecsl], dayl[numrecsl],jdl[numrecsl],
              leapsecondsl[numrecsl],SSTRING,baseDay[numrecsl],
              S2TRING,slope[numrecsl],predict[numrecsl]);
          }
   if(recordwrit < 87 ) 
   /* If the code reaches here,  data line is  short */
   {    
      sprintf(specifics,
         "Faulty data line (writing aborted) of length %d\n",recordwrit);
      returnStatus = PGS_E_TOOLKIT;
      PGS_SMF_SetDynamicMsg(returnStatus,specifics,
               "PGS_CSC_NewLeap()");
      PGS_IO_Gen_Close(fp_next);
      return 6;
   }    

    goto tern;

    }

/*-------------------- Start Updating ------------------*/

/* We copy the rest of the USNO array into the new array */

    numrecsl = lastACT + 1;

/* start copying into the first record after the last leap sec */
    numrecsN++;
    
if(lastnum > MAX_LEAP_RECS) lastnum = MAX_LEAP_RECS;

       for(numrecsN = 1; numrecsN <= numrecsu ; numrecsN++)
          {
          if(jdN[numrecsN] <= 0.0) break;
          recordwrit = fprintf(fp_next,
          " %d %s  %d =JD%10.1f  TAI-UTC=  %10.7f %s %5.0f.%s %8.7f S   %s\n",
              yearN[numrecsN], monthN[numrecsN], dayN[numrecsN],jdN[numrecsN],
	      leapsecondsN[numrecsN],SSTRING,baseDayN[numrecsN],
	      S2TRING,slope[numrecsN],predictN[numrecsN]);
          }

       if(recordwrit < 87 )
       /* If the code reaches here,  data line is  short */
       {
          sprintf(specifics,
             "Faulty data line (writing aborted) of length %d\n",recordwrit);
          returnStatus = PGS_E_TOOLKIT;
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                   "PGS_CSC_NewLeap()");
          PGS_IO_Gen_Close(fp_next);
          return 6;
       }

/*-------------------- End Updating -------------------*/

tern:
returnStatus1 = PGS_IO_Gen_Close(fp_next);
    
    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME); 
    return 0;
} 

