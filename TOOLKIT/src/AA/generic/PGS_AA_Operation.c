/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	PGS_AA_Operation
 
DESCRIPTION:
  	The file contains all operations (including autoOperations)
        currently defined to be used in the 2 and 3 D tools.


AUTHOR:
  	Graham Bland / EOSL

HISTORY:
  	07-July-94 	GJB 	Initial version
  	11-Aug -94 	GJB     Update for coding standards
	31-August-94    GJB     Post review update
	28-Jan-95       GJB     Update IDL start to account for general case
	05-Feb-95       GJB     Update nearest cell so that if x/yStart = 0
	                        then cell 1 is selected in each case (this
				accounts for the edge problem).
	14-Jun-95	ANS	Introduced bilinear interpolation routines
  
END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <stdio.h>  
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <math.h> 

#undef DEFINE_DATA
#include <freeform.h>
#include <databin.h>
#include <PGS_AA_Tools.h>

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Transform coordinates to floating point cell start points
  	
 
NAME:
  	PGS_AA_AOP_PlatteCarree

SYNOPSIS:
  	N/A

DESCRIPTION:
 Calculates the start positions in floating point terms assuming a Platte Carre 
 projection. 

 [start]
 CALCULATE	degPeryCell from ( maxLat - minLat ) / yCells
 CALCULATE	degPerxCell from ( maxLon - minLon ) / xCells
 CALCULATE	xRealStart = longitude / degPerxCell 
 CALCULATE	yRealStart = latitiude / degPeryCell 
 IF	yRealStart or xRealStart < 0	THEN
 	PERFORM	PGS_SMF_SetStaticMsg "PGSAA_E_GEOTOSTRUCT"
 	DO return to calling module with "PGSAA_E_GEOERROR"
 ENDIF
 [end]

 
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        latitude        latitude(s) of the      degrees -90.00  90.00
                        requested point
        longitude       longitude(s) of the     degrees -180.00 180.00
                        requested point
                     
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       xRealStart       start location in cells cells   1       variable
       yRealStart       start location in cells cells   1       variable
   

RETURNS:
	PGSAA_E_GEOERROR	(user)
	PGSAA_E_GEOTOSTRUCT	(log) 

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:
	N/A

DETAILS:
        NONE

GLOBALS:
	as in PGS_AA_Global.h

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status PGS_AA_AOP_PLATTECARRE (   
				   PGSt_double latitude, 
				   PGSt_double longitude, 
				   PGSt_double *xRealStart,
				   PGSt_double *yRealStart)
{
    PGSt_double degPeryCell;   /* number of degrees per y cell */
    PGSt_double degPerxCell;   /* number of degrees per x cell */

    /* this operation assumes the data set starts at -180 Long and +90 Lat */
   
    degPeryCell = (PGSt_double)((maxLat - minLat) / yCells);
    degPerxCell = (PGSt_double)((maxLong - minLong) / xCells);
    
    *xRealStart = longitude / degPerxCell;
    
    *yRealStart = latitude / degPeryCell;   
    
    if (*xRealStart < 0 || *yRealStart < 0 )
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_GEOTOSTRUCT,"PGS_AA_AOP_PLATTECARRE");
	return PGSAA_E_GEOERROR;	
    }
    return PGS_S_SUCCESS;   
}

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Transform coordinates to floating point cell start points in NMC RUC polar 
        stereographic grid
  	
 
NAME:
  	PGS_AA_AOP_W3FB06

SYNOPSIS:
  	N/A

DESCRIPTION:
 Calculates the real start from latitude & longitude for a grid having a polar 
 stereo projection.This function taken from the NMC server (nic.fb4.noaa.gov) 
 recoded to C and tested against the original F77.  Its design is 
 straightforward and based on established algorithms and is described 
 in the code block below.  Parameters specific to the transform are 
 extracted from global.
 
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        latitude        latitude(s) of the      degrees -90.00  90.00
                        requested point
        longitude       longitude(s) of the     degrees -180.00 180.00
                        requested point
                     
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       xRealStart       start location in cells cells   1       variable
       yRealStart       start location in cells cells   1       variable
   

RETURNS:
        NONE

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	as in PGS_AA_Global.h

FILES:
	NONE

FUNCTIONS_CALLED:
        NONE

END_PROLOG:
***************************************************************************/

/*  SUBROUTINE W3FB06(ALAT,ALON,ALAT1,ALON1,DX,ALONV,XI,XJ)

$$$   SUBPROGRAM  DOCUMENTATION  BLOCK
C
C SUBPROGRAM:  W3FB06        LAT/LON TO POLA (I,J) FOR GRIB
C   PRGMMR: STACKPOLE        ORG: NMC42       DATE:88-04-05
C
C ABSTRACT: CONVERTS THE COORDINATES OF A LOCATION ON EARTH GIVEN IN
C   THE NATURAL COORDINATE SYSTEM OF LATITUDE/LONGITUDE TO A GRID
C   COORDINATE SYSTEM OVERLAID ON A POLAR STEREOGRAPHIC MAP PRO-
C   JECTION TRUE AT 60 DEGREES N OR S LATITUDE. W3FB06 IS THE REVERSE
C   OF W3FB07. USES GRIB SPECIFICATION OF THE LOCATION OF THE GRID
C
C PROGRAM HISTORY LOG:
C   88-01-01  ORIGINAL AUTHOR:  STACKPOLE, W/NMC42
C
C USAGE:  CALL W3FB06 (ALAT,ALON,ALAT1,ALON1,DX,ALONV,XI,XJ)
C   INPUT ARGUMENT LIST:
C     ALAT     - LATITUDE IN DEGREES (NEGATIVE IN SOUTHERN HEMIS)
C     ALON     - EAST LONGITUDE IN DEGREES, REAL*4
C     ALAT1    - LATITUDE  OF LOWER LEFT POINT OF GRID (POINT (1,1))
C     ALON1    - LONGITUDE OF LOWER LEFT POINT OF GRID (POINT (1,1))
C                ALL REAL*4
C     DX       - MESH LENGTH OF GRID IN METERS AT 60 DEG LAT
C                 MUST BE SET NEGATIVE IF USING
C                 SOUTHERN HEMISPHERE PROJECTION.
C                   190500.0 LFM GRID,
C                   381000.0 NH PE GRID, -381000.0 SH PE GRID, ETC.
C     ALONV    - THE ORIENTATION OF THE GRID.  I.E.,
C                THE EAST LONGITUDE VALUE OF THE VERTICAL MERIDIAN
C                WHICH IS PARALLEL TO THE Y-AXIS (OR COLUMNS OF
C                OF THE GRID)ALONG WHICH LATITUDE INCREASES AS
C                THE Y-COORDINATE INCREASES.  REAL*4
C                   255.0 FOR LFM GRID,
C                   280.0 NH PE GRID, 100.0 SH PE GRID, ETC.
C
C   OUTPUT ARGUMENT LIST:
C     XI       - I COORDINATE OF THE POINT SPECIFIED BY ALAT, ALON
C     XJ       - J COORDINATE OF THE POINT; BOTH REAL*4
C
C   REMARKS: FORMULAE AND NOTATION LOOSELY BASED ON HOKE, HAYES,
C     AND RENNINGER'S "MAP PROJECTIONS AND GRID SYSTEMS...", MARCH 1981
C     AFGWC/TN-79/003
CC ATTRIBUTES:
C   LANGUAGE: IBM VS FORTRAN
C   MACHINE:  NAS
C
C$$$
C
         DATA  RERTH /6.3712E+6/, PI/3.1416/
         DATA  SS60 /1.86603/
C
C        PRELIMINARY VARIABLES AND REDIFINITIONS
C
C        H = 1 FOR NORTHERN HEMISPHERE; = -1 FOR SOUTHERN
C
C        REFLON IS LONGITUDE UPON WHICH THE POSITIVE X-COORDINATE
C        DRAWN THROUGH THE POLE AND TO THE RIGHT LIES
C        ROTATED AROUND FROM ORIENTATION (Y-COORDINATE) LONGITUDE
C        DIFFERENTLY IN EACH HEMISPHERE
C
         IF(DX.LT.0) THEN
           H = -1.
           DXL = -DX
           REFLON = ALONV - 90.
         ELSE
           H = 1.
           DXL = DX
           REFLON = ALONV - 270.
         ENDIF
C         
         RADPD = PI/180.0
         REBYDX = RERTH/DXL
C
C        RADIUS TO LOWER LEFT HAND (LL) CORNER
C
         ALA1 =  ALAT1 * RADPD
         RMLL = REBYDX * COS(ALA1) * SS60/(1. + H * SIN(ALA1))
C
C        USE LL POINT INFO TO LOCATE POLE POINT
C
         ALO1 = (ALON1 - REFLON) * RADPD
         POLEI = 1. - RMLL * COS(ALO1)
         POLEJ = 1. - H * RMLL * SIN(ALO1)
C
C        RADIUS TO DESIRED POINT AND THE I J TOO
C
         ALA =  ALAT * RADPD
         RM = REBYDX * COS(ALA) * SS60/(1. + H * SIN(ALA))
C
         ALO = (ALON - REFLON) * RADPD
         XI = POLEI + RM * COS(ALO)
         XJ = POLEJ + H * RM * SIN(ALO)
C
      RETURN
      END
*/

#include <math.h>

PGSt_SMF_status PGS_AA_AOP_W3FB06(
		  PGSt_double latitude, 
		  PGSt_double longitude, 
		  PGSt_double *xRealStart,
		  PGSt_double *yRealStart)

{
    /* get constants - until the CUC tool is coded, these must be hard coded here */

    const float pi = 3.1416;
    const float ss60 = 1.86603;
    const float radiusEarth = 6371200.0;
   
    float answer;
    
    float hemiSphere;
    float meshLengthLength;
    float refLongitude;
    float radpd;   
    float rebydx;   
    float ala1;    
    float rmll;    
    float al01;    
    float polei;
    float polej;    
    float ala;    
    float rm;    
    float al0;

    /* no comments here - refer to F77 code above */
   
    
     /* first fix the longitude so it's east rather than standard frame */
    
    if (longitude < 0)
    {
        longitude = 360.0 + longitude;
    }

    if ( meshLength < 0 )
    {
	hemiSphere = -1;
	meshLengthLength = -(float)meshLength;
	refLongitude = gridOrientation - 90.0;
    }
    else
    {
	hemiSphere = 1;
	meshLengthLength = (float)meshLength;
	refLongitude = (float)gridOrientation - 270.0;
    }
    
    radpd = pi / 180.0;
 
    rebydx = radiusEarth / meshLengthLength;
 
    ala1 = (float)lowerLeftLat * radpd;
    
    rmll = rebydx * cos(ala1) * ss60 / (1.0 + hemiSphere * sin(ala1));       
    
    al01 = ((float)lowerLeftLong - refLongitude) * radpd;
    
    polei = 1. - (rmll * cos(al01));

    polej = 1. - (hemiSphere * rmll * sin(al01));

    ala = (float)latitude * radpd;
        
    rm = rebydx * cos(ala) * ss60/(1. + (hemiSphere * sin(ala)));
    
    al0 = ((float)longitude - refLongitude) * radpd;
    
    answer = polei + rm * cos(al0);
    *xRealStart = answer;
        
    answer =  (float)(polej + hemiSphere * rm * sin(al0));
    *yRealStart =  answer;

    return PGS_S_SUCCESS;
}

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Calculate start points of nearest cell center
  	
 
NAME:
  	PGS_AA_OP_NearestCell

SYNOPSIS:
  	N/A

DESCRIPTION:
 Calculates the X and Y start values corresponding to the cell centers nearest 
 to the input latitude and longitude.  It is usually selected by the user to be 
 meshed with one of the autoOperations AOP_PLATTCARRE etc. which calculate the 
 floating point start cell values.  A function to round up to the nearest 
 integer is used.

[start]
CALCULATE	xStart = nearest integer to xRealstart + 0.5
CALCULATE	yStart = nearest integer to yRealStart  + 0.5
[end]

 
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        xRealStart      start location in cells cells   1       variable
        yRealStart      start location in cells cells   1       variable
     
                     
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        xStart          start location in cells cells 1       variable
        yStart          start location in cells cells 1       variable
   

RETURNS:
	NONE

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

PGSt_SMF_status PGS_AA_OP_NearestCell(
			 PGSt_double xRealStart,
			 PGSt_double yRealStart,
			 PGSt_integer *xStart,
			 PGSt_integer *yStart)
{
    *xStart = ceil(xRealStart);
    *yStart = ceil(yRealStart);

    /* extra check to ensure that a result is found even if the 
       point falls exactly on the boundary of a data set */

    if (*xStart == 0 ) *xStart = 1;
    if (*yStart == 0 ) *yStart = 1;

    return PGS_S_SUCCESS;
}

   
/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Change coordinates to Greenwich start
  	
 
NAME:  PGS_AA_AOP_GreenwichStart

SYNOPSIS:
  	N/A

DESCRIPTION:
 Changes the latitude and longitude input from user input frame to one 
 starting at Greenwich meridian and at the North Pole.  This is to account for 
 data sets having cells with this start point.

 Input:	 latitude, longitude
 Output:	latitude, longitude
    
 [start]
 IF	longitude < 0	THEN
 	CALCULATE	add 360.0 to longitude
 ENDIF
 CALCULATE	latitude = 90.0 - latitude
 [end]

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        latitude        latitude(s) of the      degrees  -90.00  90.00
                        requested point
        longitude       longitude(s) of the     degrees  -180.00 180.00
                        requested point
     
                     
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        mlatitude       modified latitude       degrees  -90.00  90.00
                        of the requested point
        mlongitude      modifieded longitude    degrees  -180.00 180.00
                        of the requested point

RETURNS:
	NONE

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


PGSt_SMF_status
PGS_AA_AOP_GreenwichStart(PGSt_double latitude, PGSt_double longitude,
			  PGSt_double *mlatitude, PGSt_double *mlongitude)

{
/* enters in standard frame - change so long starts at Greenwich 
and lat at north pole */

    if (longitude < 0.0)
    {
	*mlongitude = 360.0 + longitude;
    }
    
    else
    {
	*mlongitude = longitude;
    }
    
    *mlatitude = 90.0 - latitude;
    
    return PGS_S_SUCCESS;  
}

  
/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Change coordinates to IDL start
  	
 
NAME:  PGS_AA_AOP_IDLStart

SYNOPSIS:
  	N/A

DESCRIPTION:
 Changes the latitude and longitude input from user input frame to one 
 starting at the 'top left' of the data set.  This is to account for data 
 sets having cells with this start point.  this works for data 
 sets fixed at the IDl as well as data sets having other more 
 limited geographic coverage.
 
 [start]
 CALCULATE	longitude = (minLong - longitude) * -1 
 CALCULATE	latitude = maxlat - latitude
 [end]

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        latitude        latitude(s) of the      degrees  -90.00  90.00
                        requested point
        longitude       longitude(s) of the     degrees  -180.00 180.00
                        requested point
     
                     
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        mlatitude       modified latitude       degrees  -90.00  90.00
                        of the requested point
        mlongitude      modifieded longitude    degrees  -180.00 180.00
                        of the requested point

RETURNS:
        NONE

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	as in PGS_AA_Global.h

FILES:
	NONE

FUNCTIONS_CALLED:
        NONE

END_PROLOG:
***************************************************************************/ 

PGSt_SMF_status
PGS_AA_AOP_IDLStart(PGSt_double latitude, PGSt_double longitude, 
		    PGSt_double *mlatitude, PGSt_double *mlongitude)

{
    /* enters in standard frame, fix so long starts at top left of data set */ 
    
    *mlongitude = ( ( minLong - longitude) * -1 ) ;
    
    *mlatitude = maxLat - latitude;
 
    return PGS_S_SUCCESS;
}


/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Calculate start points of nearest cell center (modified)
  	
 
NAME:
  	PGS_AA_OP_NINTCELL

SYNOPSIS:
  	N/A

DESCRIPTION:
 Calculates the X and Y start values corresponding to the cell centers 
 appropriate to the transform.  This autoOperation is designed to account for the 
 calculation vagaries of AOP_W3FB06 which does not currently accurately 
 locate cells.

 [start]
 IF	x/yRealStart is just > the nearest integer (0.01 tolerance) 	THEN
 	CALCULATE x/yStart = integer value less than x/yRealStart
 ELSE
 CALCULATE x/yStart = nearest integer to x/yRealStart  + 0.5
 ENDIF
 [end]
 
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        xRealStart      start location in cells cells   1       variable
        yRealStart      start location in cells cells   1       variable
     
                     
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        xStart          start location in cells cells 1       variable
        yStart          start location in cells cells 1       variable
   
RETURNS:
	NONE

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
************************************************************************/
 
PGSt_SMF_status PGS_AA_OP_NINTCELL(
			 PGSt_double xRealStart,
			 PGSt_double yRealStart,
			 PGSt_integer *xStart,
			 PGSt_integer *yStart)
{
    if ((xRealStart - floor(xRealStart)) < 0.01 )
    {
	*xStart = floor(xRealStart);
    }
    else
    {
	*xStart = ceil(xRealStart);
    }
    
    if ((yRealStart - floor(yRealStart)) < 0.01 )
    {
	*yStart = floor(yRealStart);
    }
    else
    {
	*yStart = ceil(yRealStart);
	
    }
        
    return PGS_S_SUCCESS;
}
/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Bilinear interpolation routine for a regular grid
  	
 
NAME:
  	PGS_AA_BilinearInterp

SYNOPSIS:
  	N/A

DESCRIPTION:
	Interpolates for the given values of the two dimentions using
	function values from the grid points. Grid could be any size
	and could be rectangular. The routine is taken from Numerical
	recipes for C (by William H. Press) page 106.
	

INPUT:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        xDimention1	first dimention		numeric variable
	xDimention2	second dimention	numeric variable
	yGridValues	grid values		numeric variable
	rank1		first dimention size	numeric	0 	variable
	rank2		second dimention size    numeric 0       variable
	xVal1		1st dimention value of
			given point		numeric	variable
	xVal2		2st dimention value of
                        given point             numeric variable
	yResult		result value		numeric variable
	dyResult	estimated error		numeric variable
OUTPUTS:
        Name            Description             Units   Min     Max
        ----            -----------             -----   ---     ---
	yResult         result value            numeric variable
        dyResult        estimated error         numeric variable
	
   
RETURNS:
	PGS_S_SUCCESS
	PGSAA_E_MALLOC

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
	PGS_AA_LinearInterp

END_PROLOG:
************************************************************************/

PGSt_SMF_status PGS_AA_BiLinearInterp(
		PGSt_double xDimention1[], /* first dimention */
		PGSt_double xDimention2[], /* second dimention */
 		PGSt_double **yGridValues, /* grid values */
		PGSt_integer  rank1,       /* grid size in 1st dimention */
                PGSt_integer  rank2,       /* grid size in 2nd dimention */
		PGSt_double xVal1, 	   /* first dimention value of the given point */
		PGSt_double xVal2, 	   /* seconfd dimention value of the given point */
		PGSt_double* yResult, 	   /* pointer to point to the result */
		PGSt_double* dyResult)	   /* error estimate */
{
	PGSt_integer loopCount;
	PGSt_double *yTemp;
	PGSt_SMF_status retVal;

	retVal = PGS_MEM_Malloc((void **)&yTemp, rank1 * sizeof(PGSt_double));
	if(retVal != PGS_S_SUCCESS)
	{
		PGS_SMF_SetStaticMsg (PGSAA_E_MALLOC,"PGS_AA_BiLinearInterp");
            	return PGSAA_E_MALLOC;
	}
	
	
	for(loopCount =0; loopCount<rank1; loopCount++)
	{
		retVal = PGS_AA_LinearInterp(xDimention1, yGridValues[loopCount], rank2, xVal1, &yTemp[loopCount], dyResult);
		if(retVal != PGS_S_SUCCESS) return retVal;
	}
	retVal = PGS_AA_LinearInterp(xDimention2, yTemp, rank1, xVal2, yResult, dyResult);
	if(retVal != PGS_S_SUCCESS) return retVal;
	
	(void) PGS_MEM_Free(yTemp);
	yTemp = (PGSt_double *) NULL;
	return (PGS_S_SUCCESS);
}
/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Linear interpolation routine 
  	
 
NAME:
  	PGS_AA_LinearInterp

SYNOPSIS:
  	N/A

DESCRIPTION:
	Interpolates for the given values of the independent variable using
	function values from the given points. 
	The routine is taken from Numerical recipes for C (by William H. Press)
	page 90
	

INPUT:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        yDimention	dependent variable	numeric variable
	xDimention	independent variable	numeric variable
	nPoints		number of points	numeric	0 	variable
	xVal1		xDimention value of
			given point		numeric	variable
OUTPUTS:
        Name            Description             Units   Min     Max
        ----            -----------             -----   ---     ---
	yResult         result value            numeric variable
        dyResult        estimated error         numeric variable
	
   
RETURNS:
	PGS_S_SUCCESS
	PGSAA_E_MALLOC

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
	None

END_PROLOG:
************************************************************************/
	
PGSt_SMF_status PGS_AA_LinearInterp(
		PGSt_double xDimention[], /* independent variable */
		PGSt_double yDimention[], /* dependent variable */
		PGSt_integer nPoints, 	  /* number of points */
		PGSt_double xVal, 	  /* x value where the y value is required */
		PGSt_double *yRes, 	  /* address to hold the result value */
		PGSt_double *dyRes)	  /* error estimate */
{
	PGSt_integer loopCount, mLoop;
	PGSt_integer nStart = 1;
	PGSt_double *cDiff, *dDiff;
	PGSt_double *xLocDim, *yLocDim;
	PGSt_double den, dif, dift, ho, hpi, we;
	PGSt_SMF_status retVal;

	xLocDim = xDimention -1;
	yLocDim = yDimention -1;
	dif = fabs(xVal-xLocDim[1]);
	retVal = PGS_MEM_Malloc((void **)&cDiff, nPoints * sizeof(PGSt_double));
	if(retVal != PGS_S_SUCCESS)
        {
                PGS_SMF_SetStaticMsg (PGSAA_E_MALLOC,"PGS_AA_LinearInterp");
                return PGSAA_E_MALLOC;
        }
	cDiff = cDiff-1;
	retVal = PGS_MEM_Malloc((void **)&dDiff, nPoints * sizeof(PGSt_double));
	if(retVal != PGS_S_SUCCESS)
        {
                PGS_SMF_SetStaticMsg (PGSAA_E_MALLOC,"PGS_AA_LinearInterp");
                return PGSAA_E_MALLOC;
        }
	dDiff = dDiff-1;

	for(loopCount=1; loopCount<=nPoints; loopCount++)
	{
		if((dift=fabs(xVal-xLocDim[loopCount])) < dif)
		{
			nStart = loopCount;
			dif=dift;
		}
		cDiff[loopCount] = yLocDim[loopCount];
		dDiff[loopCount] = yLocDim[loopCount];
	}
	*yRes = yLocDim[nStart--];
	for(mLoop=1;mLoop<nPoints;mLoop++)
	{
		for(loopCount=1; loopCount<=nPoints-mLoop; loopCount++)
		{
			ho=xLocDim[loopCount]-xVal;
			hpi=xLocDim[loopCount+mLoop]-xVal;
			we=cDiff[loopCount+1]-dDiff[loopCount];
			den=ho-hpi;
			den = we/den;
			dDiff[loopCount] = hpi*den;
			cDiff[loopCount] = ho*den;
		}
		*yRes += (*dyRes = (2*nStart < (nPoints-mLoop) ? cDiff[nStart+1] : dDiff[nStart--]));
	}
	(void) PGS_MEM_Free(cDiff + 1);
	cDiff = (PGSt_double *) NULL;
	(void) PGS_MEM_Free(dDiff + 1);
	dDiff = (PGSt_double *) NULL;
	return PGS_S_SUCCESS;
}
	

