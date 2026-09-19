/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_AA_dem.c
 
DESCRIPTION:
        Retrieves Digital Elevation Model (DEM) data from specific data files
	for point locations defined by geographical coordinates of latitude
	and longitudes.

AUTHOR: 
        Alward N. Siyyid/ EOSL

HISTORY:
        19-JAN-95      ANS     Initial version
	15-Feb-95      ANS     Fix DR 00701
	17-Feb-95      ANS     Fixed so that missing points are reported
			       if at the beginning or end of data array
        23-Jun-95       ANS     Removed cfortran inclusion 
	11-July-95	ANS	Improved Fortran example in the prolog
	14-July-95      ANS     Byte swapping should only be performed if necessary on dec
	21-July-95	ANS	explained byteswapping and dec long datatype problem in the prolog
        11-Sep-97     CSWT     Fixed bug ECSed06666 ifferent results are occurring
                               for he AA_dem_interger
        06-July-99      SZ      Updated for the thread-safe functionality

END_FILE_PROLOG
*******************************************************************************/

/* include files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#undef _CFE
#undef DEFINE_DATA
#include <freeform.h>
#include <databin.h>
#include <PGS_AA_Tools.h>
#include <PGS_TSF.h>


/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Retrieves Digital Elevation Model Data
  
NAME:  
        PGS_AA_dem()

SYNOPSIS:
C:
        #include "PGS_AA.h"

	PGSt_SMF_status
	PGS_AA_dem(char parms[][100],
		   PGSt_integer nParms,
		   PGSt_double latitude[],
		   PGSt_double longitude[],
		   PGSt_integer versionFlag[],
		   PGSt_integer nPoints,
		   PGSt_integer fileId,
	   	   PGSt_integer operation,
		   void *results)

FORTRAN:
 	include "PGS_AA_10.f"    
	
	integer function pgs_aa_dem( parms, nparms, latitude, longitude, versionflag,
                  npoints, fileId, operation, results ) 

	character*99 parms(*)
	integer      nParms
	real*8       latitude(*)
	real*8       longitude(*)
	integer      versionflag(*)
	integer      npoints
	integer      fileId
	integer	     operation
	'user specified'   results (see Notes)
 
DESCRIPTION:
	This routine provides the interface to retrieve DEM values from the 
	gridded data set. 

INPUTS:
        Name            Description            Units    Min     Max
        ----            -----------             -----   ---     ---
        parms            parameter names         see notes
                        requested
        nParms          number of parms         none    1       #defined
        latitude        latitude(s) of the      degrees -90.00  90.00
                        requested point
        longitude       longitude(s) of the     degrees -180.00 180.00
                        requested point
	versionFlag	indicates tile location see notes
			for a point (see notes)
        nPoints         no. of points requested none    1       variable
        fileId          logical file number     none   variable variable
        operation       defines user required   none    1       variable

OUTPUTS:
        Name            Description             Units   Min     Max
        ----            -----------             -----   ---     ---
        results         results                 see notes

RETURNS:   
        PGS_S_SUCCESS
	PGSAA_E_NPOINTSINVALID		number of points invalid
	PGSAA_E_TILE_STATUS		could not establish tile status of the DEM file
	PGSAA_E_2DGEO			error returned from PGS_AA_2Dgeo
	PGSAA_E_SUPPORTID		could not establish support file id
	PGSAA_E_MINMAX			could not establish min/max range for the DEM 
	PGSAA_E_DATATYPE		could not establish parameter datatype
	PGSAA_E_UNKNOWN_DATATYPE	DEM datafile datatype is unknown
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
C:      #include <PGS_AA.h>
	PGSt_SMF_status retStatus;

	char parms[][100] = { "USAelevation" };
	long nParms = 1;
	PGSt_double latitude[MAX_POINTS] = {51.5, 51.23666, 50.973333} ;
	PGSt_double longitude[MAX_POINTS] = {0.1666666,0.3832,  0.5999};

	PGSt_integer  versionFlag[MAX_POINTS];

	PGSt_integer nPoints = 3;
	PGSt_integer fileId = 210;
	PGSt_integer version = 0;
	PGSt_integer operation = 1;
	short results[3];
	retStatus = PGS_AA_2Dgeo(parms, nParms, latitude, longitude, versionFlag, nPoints,
                  fileId, operation, results);

FORTRAN:
	include "PGS_AA_10.f"
	include "PGS_SMF.f"

	IMPLICIT   		NONE
	integer			pgs_aa_dem
        character*99 		parms(4)
	integer	     		versionFlag(300)
        integer      		nParms
        double precision       	latitude(300)
        double precision       	longitude(300)
        integer      		fileId
        integer      		nPoints
        integer      		version
        integer      		operation
        integer      		results(300)
	integer			retStatus
        parms(1)= "USAelevation"
        nParms = 1
        fileId = 202
        operation = 1
        nPoints = 300
        do 10 i = 1, 300
        .
        .
        latitude(i) = calculated_user_lat
        longitude(i) = calculated_user_lon
        .
        .
 10      continue

         retStatus = pgs_aa_dem( parms, nparms,latitude, longitude, versionFlag, npoints,
       1             fileId, operation, results )
             
NOTES:
	The added facility that differentiates this tool from its sister tool
	PGS_AA_2Dgeo is that this routine can handle tiled data sets. Some of
	the DEM datafiles can be very large files and are necessarily tiled 
	into smaller files to avoid memory problems. 

	Also this routine processes all input point data and returns a warning
	if some of the input points were found to be out of range. In such an
	event user can examine versionFlag[] to locate the offending points.
	For such points the cooresponding location in versionFlag would contain
	a value PGSd_AA_OUT_OF_RANGE. For eg.

		if latitude[3] and longitude[3] is the offending point then
		   versionFlag[3] = PGSd_AA_OUT_OF_RANGE.
		
	For other points the versionFlag[] would actually contain the number
	of the tile where the point was located. 

	For the details of DEM datafiles the user is referred to appendix[TBD].

	The FORTRAN result argument returned is not specified since
        it depends on the data set used; e.g. it could be real or
        integer.

	The results buffer holds the final output sent back to the user.
        It can hold data of 4 types (long, short, float,double).

	For more details the user is referred to information regarding
	PGS_AA_2Dgeo.

	---DEC---- users
	"dec users be aware that for some of the product files a dec version (eg etop05.dat__dec)
	is supplied. The user should use these instead of the normal files. This is for backward
	compatibility with the PGS_AA_2Dgeo tool. For the rest of the data files there is an 
	inbuilt facility to swap the bytes. For these files there is a flag 'swapBytes = yes'
	in the support file. This flag is set to 'no' for the data files with 'dec' versions.

	Another aspect that the user should be aware of is that dec represents 'long' datatype
	as 8 bytes long. Therefore if there is a datafile created on a different platform 
	(most other platforms represents 'log' as 4 bytes) than that file must be converted
	first to be used on dec. Conversion should simply be reading the fileas 'int' (4 bytes)
	and writing it out as 'long' (8 bytes) on the dec. To take care of byteswapping the
	support file for such datafile should contain a flag 'swapBytes = yes' 

REQUIREMENTS:
	PGSTK-0980 PGSTK-0840
        

DETAILS:
	Data sets must be set up through the process control tables
        and properly referenced in the user code (fileId).  A format
        file and a support file must be present for each data set
	and associated tiles being accessed .

GLOBALS:
        minLat, maxLat, minLong, maxLong, datatype

FILES:
        The Process control file pointed to by PGS_PC_INFO_FILE must
        be set before tool use.
        The general support file containing mapping of parameters to
        logical format and support files must be present (current suppSupport)
        For each data file to be accessed through the tool, a format and
        support file must exist.

FUNCTIONS_CALLED:
	PGS_AA_2Dgeo
	PGS_PC_GetFileAttr
	PGS_PC_GetNumberOfFiles
	PGS_AA_GetSuppGeo
	PGS_AA_GetSupp
	PGS_SMF_SetStaticMsg
        PGS_TSF_LockIt 
        PGS_TSF_UnlockIt   
        PGS_SMF_TestErrorLevel
	
END_PROLOG:
***************************************************************************/
PGSt_SMF_status
PGS_AA_dem(				/* fetches DEM values for the given set of points */
	   char parms[][100],		/* parameter names (elevation, quality etc)
					 * in a DEM data file 
					 */
	   PGSt_integer nParms, 	/* number of parameters */
	   PGSt_double latitude[],	/* latitude vector of point with dimension equal to nPoints */
	   PGSt_double longitude[],	/* longitude vector of points with dimension equal to nPoints  */
	   PGSt_integer versionFlag[], 	/* DEM data flag may be tiled and this array contains
					 * tile number for each point on return. If a particular
					 * point is not in the DEM file then the associated 
					 * version flag would contain PGSd_AA_OUT_OF_RANGE
					 * With dimension equal to nPoints
					 */
	   PGSt_integer nPoints, 	/* Number of points */
	   PGSt_integer fileId, 	/* DEM data File id */
	   PGSt_integer operation, 	/* Mainly for future use but for now must be set to 1 */
	   void *results)		/* result vector with dimension equal to nPoints */
     
{
    
    PGSt_SMF_status retVal = PGS_S_SUCCESS;
    PGSt_SMF_status fnRetVal = PGS_S_SUCCESS;
    PGSt_integer numFiles = 0;
    char fileAttribute[PGSd_PC_FILE_PATH_MAX] = "";
    char *last = NULL;    
    char *dummyStr = NULL;


    PGSt_integer actualVersion = 0;
    PGSt_integer supportId = 0;
    PGSt_integer version = 0;
    PGSt_integer firstCount = 0;
    PGSt_integer secondCount = 0;
    PGSt_integer startPoint = 0;
    PGSt_integer numPoints = 0;
    PGSt_integer dummyInt = 0;
    PGSt_integer logSuppFile[1] = {0};
    PGSt_integer dataSize = 0;
    char localParms[PGSd_AA_MAXNOPARMS][100];
#ifdef dec
    char *resultBytePtr = NULL;
    char temp[20] = "";
    char swapBytes[10];
#endif

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retLock;         /* lock and unlock return */
#endif
    
   /* Check that the number of points is not less than one */

   if(nPoints < 1)
   {
       retVal= PGS_SMF_SetStaticMsg
                                (PGSAA_E_NPOINTSINVALID, "PGS_AA_dem");
       return PGSAA_E_NPOINTSINVALID;
   }

   /* DEM file may be tiled so get the number of file */

    retVal =  PGS_PC_GetNumberOfFiles(fileId, &numFiles);

    if (retVal != PGS_S_SUCCESS || numFiles <= 0)
    {
	retVal= PGS_SMF_SetStaticMsg
          			(PGSAA_E_TILE_STATUS, "PGS_AA_dem");
	return PGSAA_E_TILE_STATUS;
    }
    
    /* initialized the version flag array with the PGSd_AA_OUT_OF_RANGE */

    for(firstCount =0; firstCount < nPoints; firstCount++)
    {
          versionFlag[firstCount] = PGSd_AA_OUT_OF_RANGE;
    }
    
    /* for each tiled file obtain lat/long min/max range and the datatype */

    for ( version = 1; version < numFiles +1 ; version++)
    {    
	retVal = PGS_PC_GetFileAttr(fileId, (numFiles - version + 1),
				    PGSd_PC_ATTRIBUTE_LOCATION, PGSd_PC_FILE_PATH_MAX,
				    fileAttribute);
	if(retVal != PGS_S_SUCCESS)
        {
            retVal= PGS_SMF_SetStaticMsg
                            (PGSAA_E_SUPPORTID, "PGS_AA_dem");
            return PGSAA_E_SUPPORTID;
        }
	    
	/* the following is a temporary fix to allow the attribute field 
	   to be used for a fileId pending larger scale changes in 
	   getSuppGeo */
	    
	last = strrchr(fileAttribute,'/');
	supportId = atoi(last+1);
	    
	retVal = PGS_AA_GetSuppGeo(&supportId); /* To set the min/max latitude and longitude 
				    		* for the particular version(tile) 
						*/
	if(retVal != PGS_S_SUCCESS)
        {
            retVal= PGS_SMF_SetStaticMsg
                            (PGSAA_E_MINMAX, "PGS_AA_dem");
            return PGSAA_E_MINMAX;
        }
	/* This function is called solely to set the datatype field
	 * This is also a temporary measure pending larger scale changes in
         * getSupp(). This means that the DEM tool currently only handles
	 * datafile with only one parameter
	 */
	logSuppFile[0] = supportId;
	retVal = PGS_AA_GetSupp(&dummyStr, 0,  logSuppFile, dummyStr, &dummyInt);
	    
        if(retVal != PGS_S_SUCCESS)
        {
            retVal= PGS_SMF_SetStaticMsg
                            (PGSAA_E_DATATYPE, "PGS_AA_dem");
            return PGSAA_E_DATATYPE;
        }
	  
	/* get the datatype size to process results buffer */
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	if(strcmp("double", dataType[0]) == 0)
	{
	    dataSize = sizeof(double);
	}
        else if(strcmp("short", dataType[0]) == 0)
        {
            dataSize = sizeof(short);
        }
        else if(strcmp("long", dataType[0]) == 0)
        {
            dataSize = sizeof(long);
        }
        else if(strcmp("float", dataType[0]) == 0)
        {
            dataSize = sizeof(PGSt_real);
        }
	else
	{
	    retVal= PGS_SMF_SetStaticMsg
                            (PGSAA_E_UNKNOWN_DATATYPE, "PGS_AA_dem");
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
            return PGSAA_E_UNKNOWN_DATATYPE;
	}


	/* fill flag array for each point with the associated version number 
	 * of the file where the point exist 
	 */

	    
	for(firstCount=0; firstCount< nPoints; firstCount++)
	{
	    if(versionFlag[firstCount] == PGSd_AA_OUT_OF_RANGE)
	    {
	       if ( latitude[firstCount] <=  maxLat && latitude[firstCount] >= minLat
               && longitude[firstCount] <= maxLong && longitude[firstCount] >= minLong)
               {
	           versionFlag[firstCount] = version;
	       }
	    }
	}
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif
    }
	    
    /* results buffer is processed for each contigous set of points 
     * belonging to a particular tile (version). Points which could not
     * be located (with version flag = PGSd_AA_OUT_OF_RANGE) will be left alone
     */

        startPoint = 0; 	    /* start of first contigous set of points */
        numPoints  = 1; 	    /* number of points in a tile should atleast be one */

        for (firstCount = 0; firstCount < nPoints; firstCount++)
        {
	        if(versionFlag[startPoint]!= PGSd_AA_OUT_OF_RANGE) /* do not process points that could not 
						       * be located
						       */
                {
		   for (secondCount = 0; secondCount < nParms; secondCount++) 
        	   {
		       if(numFiles > 1)
                       {
		       	   sprintf(localParms[secondCount],
				   "%s%d", parms[secondCount], versionFlag[startPoint]);
		       }
		       else
		       {
			   sprintf(localParms[secondCount],"%s", parms[secondCount]);
		       }
		    }
		    actualVersion = numFiles - versionFlag[startPoint] + 1;
		    retVal = PGS_AA_2Dgeo(localParms, nParms, &latitude[startPoint], 
				          &longitude[startPoint], numPoints,
				          fileId, actualVersion,
				  operation, (void *) ((char *)results + (startPoint * dataSize * nParms)));
		    if(retVal != PGS_S_SUCCESS)
        	    {
            	        return (retVal);
        	    }
	        }
	        else
	        {
                    /* removed the memory space in the case of missing points by calling
                     * the function memset
                     */
                    memset((char *)results + (startPoint * dataSize * nParms),0,dataSize);
		    fnRetVal = PGSAA_W_MISSING_POINTS;
	        }	
	        /* update startpoint,  number of points */
	        startPoint = firstCount + 1;
	        numPoints = 1;
        }
/* byte swap if dec machine */

#ifdef dec
    retVal = PGS_AA_PeV_string(logSuppFile[0], "swapBytes",
                             swapBytes);
    if (retVal != PGS_S_SUCCESS ) return PGSAA_E_GETSUPP;
    if(strcmp(swapBytes, "yes") == 0)
    {
      for(firstCount = 0; firstCount < (nPoints * nParms); firstCount++)
      {
	/* transfer individual bytes into an temp array */
	for(secondCount = 0; secondCount < dataSize; secondCount++)
	{
	    resultBytePtr = (char *) ((char *)results + ((firstCount * dataSize) + secondCount));
	    temp[dataSize - secondCount - 1] = *resultBytePtr;
	}
	/* transfer from temp array back to result buffer */      
        for(secondCount = 0; secondCount < dataSize; secondCount++)     
        {       
            resultBytePtr = (char *) ((char *)results + ((firstCount * dataSize) + secondCount));
	    *resultBytePtr = temp[secondCount];
        }
      }
    }
#endif 
    if (fnRetVal == PGSAA_W_MISSING_POINTS)
    {
	retVal = PGS_SMF_SetStaticMsg (PGSAA_W_MISSING_POINTS, "PGS_AA_dem");
    }
    return (fnRetVal);
}   
