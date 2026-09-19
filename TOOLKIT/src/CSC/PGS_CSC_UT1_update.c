/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
      PGS_CSC_UT1_update.c 

DESCRIPTION:
      This file contains the function PGS_CSC_UT1_update()

AUTHOR:
      Peter D. Noerdlinger / Applied Research Corporation

HISTORY:
      09-Jun-1994   PDN          designed and wrote version 1 using
                                   IERS bulletinb 
      08-Feb-1996   PDN          redesign to use USNO "finals.data"
      11-Mar-1996   PDN          redesign to save storage - read 1 line
                                 of USNO data at a time
      20-Mar-1996   PDN          changed return values to integers

      02-Jan-1997   PDN          caused printing of the sign of each entry
                                 in columns 2,4, and 6; this results in a
                                 uniform density of 65 characters per line,
                                 including the newline, facilitating "fseek"

      20-Feb-1997   PDN          fixed to produce a constant length header
                                 2 lines, totalling 168 characters; the first
                                 is 99 and the second is 69, including in 
                                 each case the newline.

      21-Feb-1997   PDN          Added more error messagingi; changed time 
                                 string in header to CCSDS ASCII format

      24-Feb-1997   PDN          More care in closing files before error exits
      20-Aug-1997   PDN          Added several more blanks in string used to pad 
                                 header. Added a check on header length (first 
                                 line) with branch to error return 11.
      04-Sep-1997   PDN          Added a check for MJD sequencing.
      09-Dec-1997   PDN          Reduced number of days past realtime update to 83.
      10-Dec-1997   PDN          Added code to check for new USNO format expected in
                                 AD 2000 or 2001 - 2 lines per day.  Will skip 
                                 alternate lines. Added return value 15 for failure
      24-Nov-1999   PDN          Changed MAX_RECS to MAXRECORD so as not to conflict
                                 with PGS_CSC.h
*************************************************************************/
/*************************************************************************
BEGIN_PROLOG
 
TITLE: 
      This function updates the file of differences UT1-UTC and x,y pole
      offsets using final and predicted data from the USNO data file
      "finals.data"

NAME: 
      PGS_CSC_UT1_update()

SYNOPSIS: 
C:     
  
DESCRIPTION:
      
INPUTS:
      Name       Description               Units           Min     Max
      ----       -----------               -----           ---     ---
    utcpole.dat     file - containing:
    xpole      x pole position           seconds of arc   -1.0       +1.0
    ypole      y pole position           seconds of arc   -1.0       +1.0
    diffUT1UTC difference of UT1 and UTC seconds of time  -1.0       +1.0
    accuracy indicator                     n/a            'f' or 'p'
      

OUTPUTS:
      Name       Description               Units            Min        Max
      ----       -----------                -----            ---       ---
   utcpole.dat.NEW     file - containing:
      xpole      x pole position           seconds of arc   -1.0       +1.0
      ypole      y pole position           seconds of arc   -1.0       +1.0
      diffUT1UTC difference of UT1 and UTC seconds of time  -1.0       +1.0
                 in seconds
    accuracy indicator                     n/a            'f'  or 'p'

RETURNS:
    
      PGSCSC_W_DATA_FILE_MISSING
      PGSPC_E_FILE_OPEN_ERROR
      PGSCSC_E_ABORT_UTCPOLE_UPDATE (see Notes)
      PGS_E_TOOLKIT
      various integer returns > 0 activate messaging in shell

EXAMPLES:

NOTES:

      The code send a return value to the script: 
               $PGSSRC/CSC/update_utcpole.sh
      a Bourne shell script that must receive low integers, not SMF return
      values, because the system reserves the larger integers for system use.
      Therefore, the generic "return" "PGSCSC_E_ABORT_UTCPOLE_UPDATE" is used
      only as a step to invoke the SMF messaging system to the file:
          $PGSRUN/LogStatus
      If the script aborts, e-mail issues to some of the Toolkit staff and to
      the Toolkit e-mail address at SDPS, but the local user needs to check
      "$PGSRUN/LogStatus" for a description of the problem. 

      The record limit of 33000 lines built in this function should be adequate
      until about 2062 AD.  After that, increase the file size.

      "MJD" means Modified Julian Date = Julian Date - 2400000.5.

      Error estimates are included in "utcpole.dat" as a convenience
      for users with critical Earth Orientation requirements, and
      in the spirit of passing valuable information with the data
      rather than discarding it.
    
       Although individual error estimates are supplied, users may be interested in
       general guidlines to obtain an approximate idea as to the accuracy of their
       answers, depending on how recently the Earth moton file was updated. To that
       end we quote from the ....

      "March 1995   
   
      EXPLANATORY  SUPPLEMENT TO  IERS  BULLETINS  A  AND  B

      Table 1- Precision of the various solutions. The accuracy which 
      includes the uncertainty of the tie to the IERS System can be 
      estimated by adding quadratically 0.0007" in terrestrial pole, 
      0.00012s in UT1, and 0.0005" in celestial pole.
      -------------------------------------------------------------
          Solutions             !  terr.pole      UT      celest.pole
                                !   0.001"     0.0001s      0.001"
      --------------------------!----------------------------------
      Bulletin A daily (1)      !    0.1          0.5        0.3
        prediction (2)   10d    !    3.5         15.         0.3
                         40d    !   10.          64.         0.3
                         90d    !   16.         117.         0.3
                                !
      Bulletin B                !
        smoothed (1)1-d, 5-d    !    0.5          0.5        0.5
        raw (1)          5-d    !    0.3          0.3        0.4
        prediction (1)   10d    !    3.0         13.0        0.4
                         30d    !    8.0         35.0        0.9

      Notes.
      (1) Based on 1993-94 data.
      (2) Based on data since 1990.  "

      (End of quoted section from "bulletinb.guide" of the IERS)
	Fuller reports are available on the EHDS home page at 
       http://edhs1.gsfc.nasa.gov under "Toolkits" and "validation
       documents"

      Author's note:

      The USNO data labeled "I", which we use, are basically the IERS
      solution; the two data sets present in "finals.data" are essentially
      identical for EOSDIS purposes - thus the foregoing table applies,
      even though it refers to the IERS and we use USNO.

      The "Celestial Pole" errors refer to the "EOP" parameters which
      are not used by Goddard Space Flight Center (GSFC), or anybody 
      else in the Earth Observing community, in reducing spacecraft
      ephemeris data. We likewise ignore these tiny corrections. They
      represent measured deviations of the true Earth rotation pole from 
      the 1980 IAU nutation theory, which is used by virtually everyone.
      The error in omitting the EOP data is of order <~ 0.04   seconds 
      of arc, anyway, and affects only the "absolute" position of the 
      celestial pole in relation to the background of stars.  Thus it
      does not affect geolocation at all when we use GSFC data and 
      omit these data ourselves, except for users needing accurate
      positions of celestial bodies.  In other words, the spacecraft
      to Earth relationship will be rigorous without the EOP data,
      but the stars and planets will have erroneous places in Earth
      and in spacecraft coordinates by up to ~0.04" .

      The errors in UT1 are the most significant.  An error of 1" time
      translates to 15" arc, or 450 m at  the equator.  Thus, for 5 meters
      accuracy, you need ~ 0.01" time accuracy, which is easily attained
      after date, and usually up to a month in advance in the predicted
      UT1 values.  For 0.5 meters accuracy, you need  ~0.001" time accuracy,
      again easily available after date, but not in predictions beyond a
      day or a very few days.  Since the predictions are released only
      weekly, this means users wanting accuracy better than a meter must
      reprocess.  There is no avoiding this problem - it lies in the
      vagaries of Earth rotation.

      - Peter D. Noerdlinger    Feb 10, 1996

      Fixed program to write a constant-length header of 168 characters,
      including two (2) newline characters.  This facilitates spooling 
      to correct place in old file to update. PDN - Feb 22, 1997. 

*********************************************************************
From the server "maia.usno.navy.mil", Series 7 "README":

         The format of "new" finals data set is year, month, day,
fractional MJD, IERS or Prediction indicator for NEOS polar motion,
NEOS x (sec. of arc), NEOS error in x (sec. of arc), NEOS y (sec.
of arc), NEOS error in y (sec. of arc), IERS or Prediction
indicator for NEOS UT1-UTC, NEOS UT1-UTC (sec. of time), NEOS error
in UT1-UTC (sec. of time), NEOS LOD (msec. of time), NEOS error in
LOD (msec. of time), IERS or Prediction indicator for NEOS
nutation, NEOS dpsi (msec. of arc), NEOS error in dpsi (msec. of arc),
NEOS di (msec. of arc), NEOS error in di (msec. of arc), IERS/CB x
(sec. of arc), IERS/CB y (sec. of arc), IERS/CB UT1-UTC (sec. of
time), IERS/CB dpsi (msec. of arc), IERS/CB  di (msec. of arc).

Author's note: The NEOS and IERS solutions for Earth rotation are so close 
that the   distinction is immaterial for our purposes.  I choose the NEOS 
solution because the  predictions range farther into the future - a 
convenience for users, but a possible source   of error if the update 
process does not take place successfully and warnings are ignored.
*********************************************************************

This program uses:

Columns Data
8-15    MJD
19-27   xpole
28-36   xpole error
38-46   ypole
47-55   ypole error
58      Quality (I means IERS final, P predicted)
59-68   UT1-UTC 
69-78   UT1 - UTC error 

*********************************************************************

REQUIREMENTS:
      PGSTK - 1170
 
DETAILS:
      NONE

GLOBALS:
      NONE

FILES:
      utcpole.dat
      USNO "finals.data"

FUNCTIONS CALLED:
      
END_PROLOG
*************************************************************************/
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <PGS_SMF.h>
#include <PGS_CSC.h>
#include <PGS_IO.h>

#define  JAN_1ST_1970  -725846418.999918
#define  FUNCTION_NAME "PGS_CSC_UT1_update()"
#define  MAXRECORD       33000
#define  ALLOWABLE_FUTURE 83
#define  JD_MINUS_MJD  2400000.5
#define  MAX_HEADER       600
#define  NEOS_NAME "finals.data"

int main()
{  /* main */
    char argv[80];
/* THIS GROUP FOR READING IN data */
    float   xpoleTemp;             /* for reading in xpole */
    float   xerrTemp;              /* for reading in xerror */
    float   ypoleTemp;             /* for reading in ypole */
    float   yerrTemp;              /* for reading in yerror */
    float   ut1utcTemp;            /* for reading UT1-UTC */    
    float   ut1ersTemp;            /* for reading in UT1 error */
    char    accuracyTemp;          /* for reading accuracy */
    double  mjdTemp = 0.0;         /* Modified Julian Date = 
                                      JD - 2400000.5 */
    double  mjdLast = 0.0;         /* for checking sequence */
    double  jdFINAL = 0.0;         /* last allowable Julian Date */
    double  mjdBare = 0.0;         /* truncated - to allow skipping
                                      half-integral MJD's */

    int final_rec = 0;             /* index for records in 
                                      updated utcpole.dat */
    int start_rec = 0;             /* index for records in USNO data */
    int skipper = 0;               /* checker for skipping not more than
                                       1 record at a time */

/* THIS GROUP FOR READING IN NEOS data - "finals.dat" */

    float   xpoleNEOS;             /* x pole position in " of arc*/
    float   ypoleNEOS;             /* y pole position in " of arc */
    float   xerrNEOS;              /* x pole error in " of arc */
    float   yerrNEOS;              /* y pole error in " of arc */
    char    qualNEOS;              /* accuracy of data - 'I' for final,  
                                      'P' for predicted */
    float   ut1utcNEOS;            /* UT1-UTC in seconds of time */
    float   ut1erNEOS;             /* UT1 -UTC error in " of time */
    double  mjdNEOS = 0.0 ;        /* modified Julian Date  */
    double  dmjd_first = 0.0;      /* first NEOS Modified Julian Date */
    int     neos_rec = 0;          /* running index for records in 
                                      finals.dat */
    int     itry = 0;              /* running index for tries in filename */
    int     neos_total = 0;        /* number of records read in from file 
                                      (minus 1) */

/*  THIS GROUP FOR READING IN "utcpole.dat" (terminal "s" for "stored") - and
    then for holding the final result with some new records */

/* "old" refers to the original utcpole.dat file, and "old record" to 
    the numbering therein (not counting 2 header records) */

    float   xpoles[MAXRECORD];      /* x pole position in " of arc */
    float   xerror[MAXRECORD];      /* x pole error, " of arc */
    float   ypoles[MAXRECORD];      /* y pole position in " of arc */
    float   yerror[MAXRECORD];      /* y pole error, " of arc */
    float   ut1utcs[MAXRECORD];     /* UT1-UTC in seconds of time */
    float   ut1ers[MAXRECORD];      /* UT1-UTC error, " of time */
    double  mjds[MAXRECORD];        /* modified Julian Date */
    char    accu[MAXRECORD];        /* accuracy of data - 'f' for final,  
                                      'p' for predicted */
    int old_rec = 0;               /* running index for old records */
    int last_rec = 0;              /* number of last old record read */

    FILE          *fp_in;          /* file pointer for utcpole.dat */
    FILE          *fp_neos;        /* file pointer for finals.dat */
    FILE          *fp_out;         /* file pointer for utcpole.dat.NEW */

    PGSt_SMF_status returnStatus;   /* return status of function calls */

    int     fileread;               /* file read error returnStatus */
    char    new_utcpf[50];          /* name of new file that the script 
                                       will use to replace "utcpole.dat" 
                                       if the run is successful */

    char    head0[MAX_HEADER];    /* used for reading in header lines */
    char    head1[MAX_HEADER];    /* used for reading in header lines */
    char    header[MAX_HEADER];   /* used for reading in header lines */
    char    head_out[MAX_HEADER]; /* used for writing new header line */

    /* this group for general use */
    char    asciiUTC[28];
    PGSt_double secTAI93;         /* TAI time of the actual update */
    PGSt_double jdUTC[2];         /* Julian Date of the actual update */
    int     head_length;
    time_t  clock;                /* system clock */
    char    specifics[100];       /* detailed error msg */
    char    *blanks = {"                       "};        
                                  /* filler for header */

    char    *token;               /* token for finding the month name in 
                                      the results of "dir" in the script */
    char    *months[]= 
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

    char    *monthUC[]=         /* UPPER CASE MONTHS */
                       {"MOTH", /* humor */
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
    
    /* set message to indicate success */

    returnStatus = PGS_S_SUCCESS;


/* version suitable for use with a local file 

   if((fp_in = fopen("utcpole.dat","r")) == NULL)
*/

/* REPLACE THE FOLLOWING two-line sequence with the above WHEN not 
           USING EOSDIS SDP TOOLKIT */  
   returnStatus = PGS_IO_Gen_Open(PGSd_UTCPOLE,PGSd_IO_Gen_Read,&fp_in,1);
   if (returnStatus != PGS_S_SUCCESS)
   {
       sprintf(specifics,
                "Unable to open file $PGSDAT/CSC/utcpole.dat");
       PGS_SMF_SetDynamicMsg(returnStatus,specifics,
              "PGS_CSC_UT1_update()");
       return 2;
   } /* end switch on return from utcpole.dat file opening */

/*---------------------------------------------------------------------*/

/* begin utcpole section */
   /* Read first record of file with modification history */
   fgets(head1,MAX_HEADER,fp_in);         

   /* create new header */
   strcpy(head_out,"File Updated: ");

   clock = time(NULL); /* get time as seconds since 12 am January 1st 1970 */

   secTAI93 = (PGSt_double) clock + JAN_1ST_1970; /* adjust time to TAI
                                                     (seconds since 12 am
                                                     January 1st, 1993) */
   PGS_TD_TAItoUTC(secTAI93, asciiUTC);

   PGS_TD_TAItoUTCjd(secTAI93,jdUTC);
   /* do NOT check return status - even if leap seconds file is badly
      out of date, or missing, this function returns an approximate,
      meaningful answer */

   jdFINAL =  ALLOWABLE_FUTURE + jdUTC[0];

/* Write asciUTC to new header */
   strncat(head_out, asciiUTC, 19);

/* read string sent in from script */
   gets(argv); 

/* at this point argv contains a typical result from the script, 
   originating from "dir" on "maia.usno.navy.mil" . An example
   would be:    	   358516 Feb 6 14:54 
   It would seem simple to parse out the second and third fields for
   use in our new header.  Note, however, that some UNIX platforms
   insert a group id after the file owner returned by "dir" - in that
   case the overlying script will return that in place of the file 
   size, the file size in place of (e.g.) "Feb", etc.  So we would want
   the third and fourth fields.  Thus we now cycle through tokens in 
   argv - this is in case "dir" returns different fields - we find 
   the month name and use it with the next field in our new header.*/

   token = strtok(argv," ");
   while((token = strtok(NULL," ")) !=NULL)
   {
      for (itry = 1; itry < 13; itry++)
         {
            if((strcmp(token,months[itry]) ==0) || 
            (strcmp(token,monthUC[itry]) ==0)) goto branch;
         }
   }
   /* If the code reaches here, there was no month name in the
       data from ftp - or it was the first field - impossible */
   returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
   PGS_IO_Gen_Close(fp_in);
   sprintf(specifics, 
         "No month name found in token search %s\n",argv);
   PGS_SMF_SetDynamicMsg(returnStatus,specifics,
               "PGS_CSC_UT1_update()");
   return 3;

   branch:

   strcat(head_out,"Z, using USNO ser7 finals.data file of ");
   strcat(head_out,token);
   token = strtok(NULL," ");
   strcat(head_out,"  ");
   strcat(head_out,token);

   /* Pad header to standard length */

   head_length = (int) strlen(head_out);
   strncat(head_out,blanks,PGSd_UTCPOLE_FIRST_TWO - 70 - head_length);
   /* using 70, not 69 because the writing statement appends a newline */

   itry = (int) strlen(head_out);
   if(itry != PGSd_UTCPOLE_FIRST_TWO - 70)
   {
      returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
      sprintf(specifics,
               "Created wrong length string for header: %d, should be %d\n",
                itry, PGSd_UTCPOLE_FIRST_TWO - 70);
      PGS_SMF_SetDynamicMsg(returnStatus,specifics,
          "PGS_CSC_UT1_update()");
      return 11;
    }

   /* Read second record of file, with misc data */
   fgets(head0,MAX_HEADER,fp_in);

   /* overwrite anyway with standard 2nd line: */
   sprintf(head0,
    "MJD\tx(arc sec)\tx error\t\ty(arc sec)\ty error\t\tUT1-UTC(s)\tUT error\tqual\n");

   /* Open new utcpole file */
   /* We do this before doing much real work in case there is a local
      storage or permission problem - no sense wasting flops on a file
      we can't write */

   strcpy(new_utcpf,"utcpole.dat.NEW");
   if((fp_out = fopen(new_utcpf,"w")) == NULL) 
   {
      returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
      PGS_IO_Gen_Close(fp_in);
      sprintf(specifics,
         "Unable to open new utcpole file.\n");
      PGS_SMF_SetDynamicMsg(returnStatus,specifics,
               "PGS_CSC_UT1_update()");
      return 4;
   }

   /* Read utcpole and load x,y positions and difference of UT1 and
    UTC into arrays in memory until the end of file is encountered */
    old_rec = 0;           /* initialize record count */
    fileread = fscanf(fp_in,"%lf %f %f %f %f %f %f %c",&mjdTemp,&xpoleTemp,
               &xerrTemp,&ypoleTemp,&yerrTemp,&ut1utcTemp,&ut1ersTemp,
               &accuracyTemp);
    while (fileread != EOF)    /* WHILE START */
    { /* while loop for reading old records */
        if (old_rec == MAXRECORD)
           {
              returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
              sprintf(specifics, 
               "file 'utcpole.dat' exceeds number of records defined in table\n");
              PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                 "PGS_CSC_UT1_update()");
              fclose(fp_out);
              PGS_IO_Gen_Close(fp_in);
              return 5;
           }   
        /* assign the values read from the utcpole file to the table */

        mjdLast            = mjdTemp;
        mjds[old_rec]      = mjdTemp;
        xpoles[old_rec]    = xpoleTemp;
        ypoles[old_rec]    = ypoleTemp;
        ut1utcs[old_rec]   = ut1utcTemp;
        accu[old_rec]      = accuracyTemp; 
        xerror[old_rec]    = xerrTemp;
        yerror[old_rec]    = yerrTemp;
        ut1ers[old_rec]    = ut1ersTemp;
          old_rec++;

        fileread = fscanf(fp_in,"%lf %f %f %f %f %f %f %c",&mjdTemp,&xpoleTemp,
               &xerrTemp,&ypoleTemp,&yerrTemp,&ut1utcTemp,&ut1ersTemp,
               &accuracyTemp);

        if(old_rec > 0 && mjdTemp != mjdLast + 1.0 && fileread != EOF)
           {
              returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
              sprintf(specifics,
      "file 'utcpole.dat' contains out-of-order record at line %d\n",
              old_rec);
              PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                 "PGS_CSC_UT1_update()");
              fclose(fp_out);
              PGS_IO_Gen_Close(fp_in);
              return 14;
           }

     }   /* end while loop for reading old records */

     /* save last one because the read loop exited before storing */
     last_rec = old_rec-1;

     mjds[old_rec-1]      = mjdTemp;  /* LAST MJD IN OLD utcpole file */
     xpoles[old_rec-1]    = xpoleTemp;
     ypoles[old_rec-1]    = ypoleTemp;
     ut1utcs[old_rec-1]   = ut1utcTemp;
     accu[old_rec-1]      = accuracyTemp;    

     /* close original file "utcpole.dat" */
     if(PGS_IO_Gen_Close(fp_in) != PGS_S_SUCCESS)
     {              
        returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
        sprintf(specifics, 
               "can't close original utcpole file(input)\n");
              PGS_SMF_SetDynamicMsg(returnStatus,specifics,
                 "PGS_CSC_UT1_update()");
       fclose(fp_out);
       return 6;
     }

/* end utcpole section */

/*-------------------------------------------------------------------------------*/

/* begin NEOS section */

   if((fp_neos=fopen(NEOS_NAME,"r")) == NULL)
   {
      returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE; 
      sprintf(specifics, 
           "unable to open USNO data file\n");
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,
              "PGS_CSC_UT1_update()");
      fclose(fp_out);
      return 7;
   } /* end switch on return from utcpole.dat file opening */

/* First Pass Reading USNO/NEOS data: Read all usable records once before
   updating; save MJD of first and last usable records.  First MJD will be
   used to find place in "utcpole" file to start updating; last to terminate
   the process.  This approach could be avoided, going directly to an
   update, but it is more cautious - any read error will enable graceful
   escape - or it will core dump - leaving old utcpole.dat file usable. */

    neos_rec  = 0;

{/* while loop reading neos (USNO) data */
    while (fgets(header,MAX_HEADER,fp_neos) != NULL 
           && mjdNEOS + JD_MINUS_MJD <= jdFINAL) 
    {
       fileread = sscanf(header,
       "%*c%*c%*c%*c%*c%*c%*c%8lf%*c%*c%f%f%f%f%*c%*c%c%f%f",&mjdNEOS,
       &xpoleNEOS,&xerrNEOS,&ypoleNEOS,&yerrNEOS,&qualNEOS,
       &ut1utcNEOS,&ut1erNEOS);

    /* test for 2 lines per day - new format to start in about the year
       2000   */
 
       mjdBare = (double) ( (long) mjdNEOS );
       if(mjdNEOS - mjdBare > 0.2)  /* half integral case - read next line */
       {
         /* increment skipper - if it happens twice in a row we're dead */
          skipper++;
          fileread = sscanf(header,
             "%*c%*c%*c%*c%*c%*c%*c%8lf%*c%*c%f%f%f%f%*c%*c%c%f%f",&mjdNEOS,
             &xpoleNEOS,&xerrNEOS,&ypoleNEOS,&yerrNEOS,&qualNEOS,
             &ut1utcNEOS,&ut1erNEOS);
       }
       else   /* OK case - we did not skip a line */
       {
          skipper = 0;
       }
       if (skipper > 1)
       {
          returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
          sprintf(specifics,
       "Skipped 2 consecutive lines in finals.data at line %d (half-integral MJDs present)\n",neos_rec);
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,
          "PGS_CSC_UT1_update()");
          return 15;
        }


    /* at end of file, USNO tends to list dates and Julian Dates,
       but no useful Earth rotation data.  Break in this case */
       if((strchr(header,'I') == NULL) && (strchr(header,'P') == NULL))
       break;

    /* Check for insufficient valid fields */
       if(fileread < 8)
       {
          returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
          sprintf(specifics, 
                "Deficient field at line %d of finals.dat\n",neos_rec);
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,
              "PGS_CSC_UT1_update()");
          fclose(fp_out);
          fclose(fp_neos);
          return 8;
       }
       if(neos_rec == 0)
       {
          dmjd_first = mjdNEOS;
       }
       if(skipper == 0 )
       {
          neos_rec++;
       }
    }/*  end while loop reading neos (USNO) data - pass 1 */

    neos_total = neos_rec - 1;

/* find where to start updating utcpole.dat */

   if(mjds[0] >= dmjd_first)  /* unexpected = USNO file starts before we do */
   {
      start_rec = 0;
      returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
      sprintf(specifics, 
           "Unexpected beginning of NEOS data before utcpole data");
      PGS_SMF_SetDynamicMsg(returnStatus,specifics,
           "PGS_CSC_UT1_update()");
      fclose(fp_out);
      fclose(fp_neos);
      return 9;
   }
   else
   {
     /* start overwriting utcpole records */
      for(start_rec =1; start_rec <= last_rec; start_rec++)
      {
          if(mjds[start_rec] > dmjd_first) goto got_record;
      }
   /* real problems here - NEOS data start after end of old data */
      returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
      sprintf(specifics, 
         "Gap from last utcpole mjd %f to first %f in finals.dat\n",
              dmjd_first,mjds[start_rec]);
      PGS_SMF_SetDynamicMsg(returnStatus,specifics,
           "PGS_CSC_UT1_update()");
      fclose(fp_out);
      fclose(fp_neos);
      return 10;
   }/* end setup for updating */

   got_record:

/* Rewind and start reading again - this time updating */
   neos_rec  = 0;

/* We are one record too far - we don't want two adjacent with the 
   same dates */

   start_rec--;

/* Rewind "finals.dat" */
   rewind(fp_neos);

 /* while loop reading neos (USNO) data 2nd time */
   skipper = 0;
   while ((fgets(header,MAX_HEADER,fp_neos) != NULL) 
           &&  (neos_rec <= neos_total))
   {
      fileread = sscanf(header,
       "%*c%*c%*c%*c%*c%*c%*c%8lf%*c%*c%f%f%f%f%*c%*c%c%f%f",&mjdNEOS,
       &xpoleNEOS,&xerrNEOS,&ypoleNEOS,&yerrNEOS,&qualNEOS,
       &ut1utcNEOS,&ut1erNEOS);

  /* test for 2 lines per day - new format to start in about the year
       2000   */
 
       mjdBare = (double) ( (long) mjdNEOS );
       if(mjdNEOS - mjdBare > 0.2)  /* half integral case - read next line */
       {
         /* increment skipper - if it happens twice in a row we're dead */
          skipper++;
          fileread = sscanf(header,
             "%*c%*c%*c%*c%*c%*c%*c%8lf%*c%*c%f%f%f%f%*c%*c%c%f%f",&mjdNEOS,
             &xpoleNEOS,&xerrNEOS,&ypoleNEOS,&yerrNEOS,&qualNEOS,
             &ut1utcNEOS,&ut1erNEOS);
       }
       else   /* OK case - we did not skip a line */
       {
          skipper = 0;

          xpoles[start_rec]  = xpoleNEOS;
          xerror[start_rec]  = xerrNEOS;
          ypoles[start_rec]  = ypoleNEOS;
          yerror[start_rec]  = yerrNEOS;
          ut1utcs[start_rec] = ut1utcNEOS;
          ut1ers[start_rec]  = ut1erNEOS;
          mjds[start_rec]    = mjdNEOS;
          accu[start_rec]    = (qualNEOS == 'I') ? 'f' : 'p';
          start_rec++;
          neos_rec++;
       }
   }
} 

/*-------------------- End Updating -------------------*/

    fprintf(fp_out,"%s\n",head_out);
    fprintf(fp_out,"%s",head0);
    final_rec = 0;
    /* test format against header requirement to ensure commonality
       with the function that eventually reads this file; that
       function uses a "seek" capability which depends on the
       record length */

    sprintf(head0,"%5.0f\t%+f\t%f\t%+f\t%f\t%+f\t%f\t%c\n",
       mjds[1], xpoles[1],xerror[1] , ypoles[1],yerror[1] ,
       ut1utcs[1] , ut1ers[1], accu[1]);

    itry = (int) strlen(head0);
    if(itry != PGSd_UTCPOLE_RECORD) 
    {
       returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
       sprintf(specifics,
                "Trying to write wrong record length: %d, should be %d\n",
                 PGSd_UTCPOLE_RECORD, itry);
       PGS_SMF_SetDynamicMsg(returnStatus,specifics,
           "PGS_CSC_UT1_update()");
       fclose(fp_neos);
       fclose(fp_out);
       return 11;
    }

    while(final_rec  < MAXRECORD && final_rec < start_rec )
    {
       fprintf(fp_out,"%5.0f\t%+f\t%f\t%+f\t%f\t%+f\t%f\t%c\n",
          mjds[final_rec], xpoles[final_rec],xerror[final_rec] ,
          ypoles[final_rec],yerror[final_rec] ,ut1utcs[final_rec] ,
          ut1ers[final_rec], accu[final_rec]);
       final_rec++;
    }  
    /* close USNO file */
    if( fclose(fp_neos) !=0 )
    {
       returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
       sprintf(specifics,
               "Can't close data file from ftp\n");
       PGS_SMF_SetDynamicMsg(returnStatus,specifics,
          "PGS_CSC_UT1_update()");
       return 12;
    }
    
     /* close file */
     if( fclose(fp_out) !=0 )
     {
        returnStatus =  PGSCSC_E_ABORT_UTCPOLE_UPDATE;
        sprintf(specifics,
                "Can't close output file\n");
        PGS_SMF_SetDynamicMsg(returnStatus,specifics,
           "PGS_CSC_UT1_update()");
        fclose(fp_out);
        return 13;
     }
     return 0; 
} 
