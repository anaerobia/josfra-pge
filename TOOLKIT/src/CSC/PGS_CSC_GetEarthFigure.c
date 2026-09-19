/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CSC_GetEarthFigure.c

DESCRIPTION:
  This file contains the function PGS_CSC_GetEarthFigure()

AUTHOR:
  Peter D. Noerdlinger / Applied Research Corporation
  Anubha Singhal / Applied Research Corporation
  Guru Tej S. Khalsa / Applied Research Corporation
  Curt Schafer       / Steven Myers & Associates

HISTORY:
  31-Aug-1994     PDN   Designed
  01-Sep-1994     AS    Original version
  23-Sep-1994     GTSK  Modified to use PGS_IO_ routines to open and
                        close data file.
  25-May-1995     GTSK  Changed algorithm to remember earth tag and earth
                        radii values from any previous calls and return these
			radii if earth tag is the same as the last call rather
			than reopen the EARTHFIGURE file.
  21-Dec-1995     GTSK  What I said above, only this time I mean it.  Code was
                        diligently checking if the earth tag was the same as in
			the previous call but it never was since the "saved"
			value of the earth tag was never being set (other than
			the bogus default value with which it is initialized).
  11-July-1996    PDN   Fixed comments describing the variables
  07-Jul-1999     CS    Updated for Threadsafe functionality

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:   
   Get Polar and Equatorial Radii
 
NAME:    
   PGS_CSC_GetEarthFigure() 

SYNOPSIS:
C:
    #include <PGS_CSC.h>

    PGSt_SMF_status
    PGS_CSC_GetEarthFigure(
        char        *earthEllipsTag,      
        PGSt_double *equatRad_A,    
        PGSt_double *polarRad_C)    
         
FORTRAN:
   include 'PGS_SMF.f'
   include 'PGS_CSC_4.f'

   integer function pgs_csc_getearthfigure(earthellipstag,equatrad_a,
                                           polarrad_c)
	
   char*20           earthellipstag				      
   double precision  equatrad_a
   double precision  polarrad_c

DESCRIPTION:
   This tool gets the equatorial and polar radii from the earthfigure.dat 
   file for the earth model input.  

INPUTS:
   Name            Description     Units    Min        Max
   ----            -----------     -----    ---        ---
   earthellipstag  earth model     N/A      N/A        N/A               
                   used
      
OUTPUTS:
   Name            Description     Units     Min       Max
   ----            -----------     -----     ---       ---
   equatrad_a      equatorial      meters    * depends on the earth model used *
                   radius
   polarrad_c      polar radius    meters    * depends on the earth model used *
          
RETURNS:
   PGS_S_SUCCESS                  successful return
   PGSCSC_W_DEFAULT_EARTH_MODEL   default earth model is being used        
   PGSTSF_E_GENERAL_FAILURE       bad return from PGS_TSF_GetTSFMaster or 
                                  PGS_TSF_GetMasterIndex()
      
EXAMPLES:
C:
   PGSt_SMF_status      returnStatus;
   char                 *earthEllipsTag = "WGS84";
   PGSt_double          equatRad_A;
   PGSt_double          polarRad_C;
   char                 err[PGS_SMF_MAX_MNEMONIC_SIZE];  
   char                 msg[PGS_SMF_MAX_MSG_SIZE];  
      
   returnStatus = PGS_CSC_GetEarthFigure(earthEllipsTag,equatRad_A,
                                         polarRad_C);

   if(returnStatus != PGS_S_SUCCESS)
	{
	   PGS_SMF_GetMsg(&returnStatus,err,msg);
	   printf("\nERROR: %s",msg);
	}

FORTRAN:
      implicit none
      integer           pgs_csc_getearthfigure
      integer           returnstatus
      char*20           earthellipstag
      double precision  equatrad_a
      double precision  polarrad_c
      character*33 	err
      character*241 	msg
      
      earthellipstag = 'WGS84'
      
      returnstatus = pgs_csc_getearthfigure(earthellipstag,equatrad_a,
     >                                      polarrad_c)
      
      if(returnstatus .ne. pgs_s_success) then
	  returnstatus = pgs_smf_getmsg(returnstatus,err,msg)
	  write(*,*) err, msg
      endif

NOTES:
   NONE
      
REQUIREMENTS:
   PGSTK - 1059, 0930

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
      PGSg_TSF_CSCGetEarthFigurelastEquatRad
      PGSg_TSF_CSCGetEarthFigurelastPolarRad
      PGSg_TSF_CSCGetEarthFigurereturnStatus

FILES:
   earthfigure.dat

FUNCTION CALLS:
   PGS_IO_Gen_Open
   PGS_IO_Gen_Close
   PGS_SMF_SetStaticMsg  -   set static error/status message
   PGS_SMF_SetDynamicMsg -   set dynamic error/status message
   PGS_TSF_GetTSFMaster      setup threads version
   PGS_TSF_GetMasterIndex    get the index for this thread

   
END_PROLOG
******************************************************************************/ 

#include <stdio.h>
#include <string.h>
#include <PGS_CSC.h>            
#include <PGS_IO.h>
#include <PGS_TSF.h>

/* constants */

#define  MAX_HEADER         139            /* maximum size of EARTHFIGURE file
					      header */
/*#define  DEFAULT_POLAR_RAD  6356752.314245  WGS84 Earth polar radius (m) */
/*#define  DEFAULT_EQUAT_RAD  6378137.0       WGS84 Earth equatorial 
					      radius (m) */

/* default value of oldEarthTag */

/*#define  DEFAULT_EARTH_TAG "THIS IS THE DEFAULT (WGS84) EARTH MODEL"*/

/* logical identifier for earthfigure.dat data file (defined in Process
   Control File) */
 
#define EARTHFIGURE 10402

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_GetEarthFigure()"

PGSt_SMF_status
PGS_CSC_GetEarthFigure(                     /* accesses user-editable  ASCII
                                               file normally in 
                                               $PGSDAT/CSC/earthfigure.dat */ 
    char        *earthEllipsTag,            /* character string naming Earth model */
    PGSt_double *equatRad_A,                /* equatorial radius of the earth (m) */
    PGSt_double *polarRad_C)                /* polar radius of the earth (m) */   
{
    FILE        *fp;                        /* pointer to EARTHFIGURE file */ 

    int         fileread;                   /* file read return value */

    double      doubleRad_A;                /* equatorial radius of the earth 
					     (type "double" for fscanf()) */
    double      doubleRad_C;                /* polar radius of the earth 
					     (type "double" for fscanf()) */   

    char        header[MAX_HEADER];         /* used for reading in header */
    char        earthTagRead[50];           /* earth tag read from file */
    char        msg[PGS_SMF_MAX_MSG_SIZE];  /* holds error messages */


    PGSt_SMF_status returnStatus1;          /* return status of PGS function
					       calls */
    PGSt_SMF_status code;	    	    /* status code returned by
					       PGS_SMF_GetMsg() */
    
    /* indicates if the call was successful or the nature of any error */

    static PGSt_SMF_status returnStatus=PGS_S_SUCCESS;
    
    /* earth tag from any previous call to this function, initialized to some
       nonsense value (hopefully) */

    static char oldEarthTag[50]=PGSd_DEFAULT_EARTH_TAG;

    static PGSt_double lastEquatRad=PGSd_DEFAULT_EQUAT_RAD;   /* equatorial radius */
    static PGSt_double lastPolarRad=PGSd_DEFAULT_POLAR_RAD;   /* polar radius */

    char	    mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							    PGS_SMF_GetMsg() */

#ifdef _PGS_THREADSAFE

    /* Declare variables used for THREADSAFE version to replace statics
       The local names are appended with TSF and globals are preceded with
       directory and function name  */

    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_double lastEquatRadTSF;
    PGSt_double lastPolarRadTSF;
    PGSt_SMF_status returnStatusTSF;
    PGSt_SMF_status returnStatus2;  /* return for GetTSFMaster()  */
    char *oldEarthTagTSF;          /* TSD key     does Not use global */

    /*  Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_double PGSg_TSF_CSCGetEarthFigurelastEquatRad[];
    extern PGSt_double PGSg_TSF_CSCGetEarthFigurelastPolarRad[];
    extern PGSt_SMF_status PGSg_TSF_CSCGetEarthFigurereturnStatus[];

    int masterTSFIndex;

    /* Get index  and initialize keys  Then test for bad returns */

    returnStatus2 = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(returnStatus2) || masterTSFIndex ==
                                         PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* Initialize the variables used for the THREADSAFE version */

    oldEarthTagTSF = (char *) pthread_getspecific(
            masterTSF->keyArray[PGSd_TSF_KEYCSCGETEARTHFIGUREOLDEARTHTAG]);
    returnStatusTSF = PGSg_TSF_CSCGetEarthFigurereturnStatus[masterTSFIndex];
    lastEquatRadTSF = PGSg_TSF_CSCGetEarthFigurelastEquatRad[masterTSFIndex];
    lastPolarRadTSF = PGSg_TSF_CSCGetEarthFigurelastPolarRad[masterTSFIndex];

    /* Almost entire function is duplicated for the THREADSAFE version to
       protect statics.  If a value is reassigned, the global or key is updated 
                         NO LOCKS  1KEY   3 GLOBALS  */

    /* if earthEllipsTag is the same as the oldEarthTag, return the saved
       values */
    /* Threadsafe Protect: oldEarthTag, lastEquatRad, lastPolarRad,
       returnStatus */

    if (strcmp(earthEllipsTag,oldEarthTagTSF) == 0)
    {
	*equatRad_A = lastEquatRadTSF;
	*polarRad_C = lastPolarRadTSF;
	PGS_SMF_SetStaticMsg(returnStatusTSF,FUNCTION_NAME);
	return returnStatusTSF;
    }
    
    /* open earthfigure file */
    /* Threadsafe Protect: returnStatus    Reassign it's value for next use */

    returnStatusTSF = PGS_IO_Gen_Open(EARTHFIGURE,PGSd_IO_Gen_Read,&fp,1);
    PGSg_TSF_CSCGetEarthFigurereturnStatus[masterTSFIndex] = returnStatusTSF;
    switch (returnStatusTSF)
    {
      case PGS_S_SUCCESS:
	break;
      case PGS_E_UNIX:
      case PGSIO_E_GEN_OPENMODE:
      case PGSIO_E_GEN_FILE_NOEXIST:
      case PGSIO_E_GEN_REFERENCE_FAILURE:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatusTSF)
	{
	    PGS_SMF_GetMsgByCode(returnStatusTSF,msg);
	}

	/* error opening EARTHFIGURE data file, use defaults */
        /* Threadsafe Protect:returnStatus  Reassign its value for next use */

	returnStatusTSF = PGSCSC_W_DEFAULT_EARTH_MODEL;
        PGSg_TSF_CSCGetEarthFigurereturnStatus[masterTSFIndex] = 
                                                      returnStatusTSF;
	PGS_SMF_SetDynamicMsg(returnStatusTSF,msg,FUNCTION_NAME);

	*equatRad_A = PGSd_DEFAULT_EQUAT_RAD;
	*polarRad_C = PGSd_DEFAULT_POLAR_RAD;

        /* Threadsafe Protect: lastEquatRad, lastPolarRad, returnStatus
           Reassign  lastEquatRad & lastPolarRad values for next use */

	lastEquatRadTSF = PGSd_DEFAULT_EQUAT_RAD;
        PGSg_TSF_CSCGetEarthFigurelastEquatRad[masterTSFIndex] =lastEquatRadTSF;
	lastPolarRadTSF = PGSd_DEFAULT_POLAR_RAD;
        PGSg_TSF_CSCGetEarthFigurelastPolarRad[masterTSFIndex] =lastPolarRadTSF;

	return returnStatusTSF;
      default:
	PGS_SMF_SetUnknownMsg(returnStatusTSF,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Read first record of file, with modification history */
    
    fgets(header,MAX_HEADER,fp);
    
    /* Read second record of file with column titles */
    
    fgets(header,MAX_HEADER,fp);
    
    /* get earth tag information from file */
    
    fileread = fscanf(fp, "%s %lf %lf",earthTagRead,&doubleRad_A,&doubleRad_C);
    
    while (fileread != EOF)   
    {
	if (strcmp(earthEllipsTag,earthTagRead) == 0 && fileread == 3)
        {  
	    *equatRad_A = (PGSt_double) doubleRad_A;
	    *polarRad_C = (PGSt_double) doubleRad_C;

        /* Threadsafe Protect: lastEquatRad, lastPolarRad
           Reassign  lastEquatRad & lastPolarRad values for next use */

	    lastEquatRadTSF = *equatRad_A;
            PGSg_TSF_CSCGetEarthFigurelastEquatRad[masterTSFIndex] = 
                                                        lastEquatRadTSF;
	    lastPolarRadTSF = *polarRad_C;
            PGSg_TSF_CSCGetEarthFigurelastPolarRad[masterTSFIndex] = 
                                                        lastPolarRadTSF;
	    
	    /* save the current value of the earth model (earthEllipsTag) */
            /* Threadsafe Protect: oldEarthTag
                   Reassign  oldEarthTag's KEY value for next use */

	    oldEarthTagTSF[49] = '\0';
	    strncpy(oldEarthTagTSF, earthEllipsTag, 49);
            pthread_setspecific(
              masterTSF->keyArray[PGSd_TSF_KEYCSCGETEARTHFIGUREOLDEARTHTAG],
               (void *) oldEarthTagTSF);
	    
	    PGS_SMF_SetStaticMsg(returnStatusTSF,FUNCTION_NAME);
	    returnStatus1 = PGS_IO_Gen_Close(fp);
	    if (returnStatus1 != PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if (code != returnStatus1)
		{
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);
		}
                /* Threadsafe Protect: returnStatus
                     Reassign  returnStatus's value for next use */

		returnStatusTSF = PGS_E_TOOLKIT;
                PGSg_TSF_CSCGetEarthFigurereturnStatus[masterTSFIndex] =
                                                           returnStatusTSF;
		PGS_SMF_SetDynamicMsg(returnStatusTSF,msg,FUNCTION_NAME);
	    }
	    return returnStatusTSF;
        }
	else 
	{
	    fileread =fscanf(fp,"%s %lf %lf",earthTagRead,&doubleRad_A,
			     &doubleRad_C);
	}
    }
    
    /* the earth model hasn't been found in the file so the default value
       is being used */
   
    *equatRad_A = PGSd_DEFAULT_EQUAT_RAD;
    *polarRad_C = PGSd_DEFAULT_POLAR_RAD;

    /* Threadsafe Protect: lastEquatRad, lastPolarRad, returnStatus
       Reassign  all 3 values for next use */

    lastEquatRadTSF = PGSd_DEFAULT_EQUAT_RAD;
    PGSg_TSF_CSCGetEarthFigurelastEquatRad[masterTSFIndex] = lastEquatRadTSF;
    lastPolarRadTSF = PGSd_DEFAULT_POLAR_RAD;
    PGSg_TSF_CSCGetEarthFigurelastPolarRad[masterTSFIndex] = lastPolarRadTSF;
    
    returnStatusTSF = PGSCSC_W_DEFAULT_EARTH_MODEL;
    PGSg_TSF_CSCGetEarthFigurereturnStatus[masterTSFIndex] = returnStatusTSF;
    
    /* set oldEarthTag to the default earth model tag */
    /* Threadsafe Protect: oldEarthTag
                     Reassign it's KEY value for next use */

    strcpy(oldEarthTagTSF, PGSd_DEFAULT_EARTH_TAG);
     pthread_setspecific(
        masterTSF->keyArray[PGSd_TSF_KEYCSCGETEARTHFIGUREOLDEARTHTAG],
                (void *) oldEarthTagTSF);

    /* close the file and return */

    returnStatus1 = PGS_IO_Gen_Close(fp);
    if (returnStatus1 != PGS_S_SUCCESS)
    {
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatus1)
	{
	    PGS_SMF_GetMsgByCode(returnStatus1,msg);
	}
        /* Threadsafe Protect: returnStatus
                     Reassign it's value for next use */

	returnStatusTSF = PGS_E_TOOLKIT;
        PGSg_TSF_CSCGetEarthFigurereturnStatus[masterTSFIndex] = 
                                                       returnStatusTSF;
	PGS_SMF_SetDynamicMsg(returnStatusTSF,msg,FUNCTION_NAME);
	return returnStatusTSF;
    }

    PGS_SMF_SetStaticMsg(returnStatusTSF,FUNCTION_NAME);

    /* Threadsafe Protect: returnStatus */

    return returnStatusTSF;

#else
    /* if earthEllipsTag is the same as the oldEarthTag, return the saved
       values */

    if (strcmp(earthEllipsTag,oldEarthTag) == 0)
    {
	*equatRad_A = lastEquatRad;
	*polarRad_C = lastPolarRad;
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	return returnStatus;
    }
    
    /* open earthfigure file */

   returnStatus = PGS_IO_Gen_Open(EARTHFIGURE,PGSd_IO_Gen_Read,&fp,1);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGS_E_UNIX:
      case PGSIO_E_GEN_OPENMODE:
      case PGSIO_E_GEN_FILE_NOEXIST:
      case PGSIO_E_GEN_REFERENCE_FAILURE:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatus)
	{
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
	}

	/* error opening EARTHFIGURE data file, use defaults */

	returnStatus = PGSCSC_W_DEFAULT_EARTH_MODEL;
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);

	*equatRad_A = PGSd_DEFAULT_EQUAT_RAD;
	*polarRad_C = PGSd_DEFAULT_POLAR_RAD;
	lastEquatRad = PGSd_DEFAULT_EQUAT_RAD;
	lastPolarRad = PGSd_DEFAULT_POLAR_RAD;

	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    /* Read first record of file, with modification history */
    
    fgets(header,MAX_HEADER,fp);
    
    /* Read second record of file with column titles */
    
    fgets(header,MAX_HEADER,fp);
    
    /* get earth tag information from file */
    
    fileread = fscanf(fp, "%s %lf %lf",earthTagRead,&doubleRad_A,&doubleRad_C);
    
    while (fileread != EOF)   
    {
	if (strcmp(earthEllipsTag,earthTagRead) == 0 && fileread == 3)
        {  
	    *equatRad_A = (PGSt_double) doubleRad_A;
	    *polarRad_C = (PGSt_double) doubleRad_C;

	    lastEquatRad = *equatRad_A;
	    lastPolarRad = *polarRad_C;
	    
	    /* save the current value of the earth model (earthEllipsTag) */

	    oldEarthTag[49] = '\0';
	    strncpy(oldEarthTag, earthEllipsTag, 49);
	    
	    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
	    returnStatus1 = PGS_IO_Gen_Close(fp);
	    if (returnStatus1 != PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if (code != returnStatus1)
		{
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);
		}
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	    }
	    return returnStatus;
        }
	else 
	{
	    fileread =fscanf(fp,"%s %lf %lf",earthTagRead,&doubleRad_A,
			     &doubleRad_C);
	}
    }
    
    /* the earth model hasn't been found in the file so the default value
       is being used */
   
    *equatRad_A = PGSd_DEFAULT_EQUAT_RAD;
    *polarRad_C = PGSd_DEFAULT_POLAR_RAD;
    lastEquatRad = PGSd_DEFAULT_EQUAT_RAD;
    lastPolarRad = PGSd_DEFAULT_POLAR_RAD;
    
    returnStatus = PGSCSC_W_DEFAULT_EARTH_MODEL;
    
    /* set oldEarthTag to the default earth model tag */
    
    strcpy(oldEarthTag, PGSd_DEFAULT_EARTH_TAG);

    /* close the file and return */

    returnStatus1 = PGS_IO_Gen_Close(fp);
    if (returnStatus1 != PGS_S_SUCCESS)
    {
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatus1)
	{
	    PGS_SMF_GetMsgByCode(returnStatus1,msg);
	}
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	return returnStatus;
    }

    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);

    return returnStatus;
#endif
    
}
