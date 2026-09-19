/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   PGS_CSC_GetFOV_Pixel.c

DESCRIPTION:
   This file contains the function PGS_CSC_GetFOV_Pixel(), which locates
   the intersection of one or more lines of sight with the Earth.
   
AUTHOR:
   Peter D. Noerdlinger  /   Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation


HISTORY:
   21-Dec 1993 PDN  Designed  
   15-Jan 1994 PDN  Completed initial coding
   10-Mar-1994 PDN  Delivered initial version
   16-Oct-1994 PDN  Delivered version with time offsets
   18-Oct-1994 PDN  Preliminary Delivery of version interfacing to
                    new SC ephemeris tool
   20-Jan-1995 PDN  Corrected algorithm for latitude; in cases when
                    user supplied zero pixel vector, function now
                    zeros out ECI pixel vector to avoid passing 
                    uninitialized data to ECItoECR 
   02 Feb-1995 PDN  Reworked memory access for scratch arrays so as
                    to use MEM tools and malloc only one area, which is
                    then pointered.  Changed INSTRUMENT OFF BOARD message
                    from error to warning status. Corrected call to EPH
                    tool for new calling sequence (quality flags). Fixed
                    approximate slant range computation to use Earth Major
                    axis, not mean axis, avoiding problems in square root
                    when line of sight narrowly misses Earth near poles. 
   23 Mar-1995 PDN  Fixed singularity at N, S poles; fixed use of offsets
                    when there are none (numValues == 0) (thanks to Guru Tej
                    Khalsa  (GTSK) for pointing out the problem).
   09-May-1995 PDN  Fixed error messaging for case of bad SC ephemeris data
   25-May-1995 PDN  New memory allocation method devised by GTSK
   19-Jul-1995 PDN  Deleted obsolete test for missing file return from
                    PGS_CSC_GetEarthFigure().
   29 Mar-1996 PDN   simplified error messaging on return from ephemeris tool
   24 Apr-1996 PDN  put in limits to certain messages to avoid filling log file
   23-Jun-1996 PDN  Deleted stray print statement
   05-Sep-1996 GTSK Enabled use of quality flag masks
   05-Sep-1996 PDN  Fixed an extra zeroing of the output ECR pixel vector
   06-Sep-1996 PDN  Forced bad return for ECR pixel vector when there are
                    no good pixels at all
   07-Sep-1996 PDN  Moved some code setting bad output values to optimize
                    speed
   15-Jan-1997 PDN  Reorganized the error and warning priority system; 
                    improved documentation
   24-Apr-1997 PDN  Added logic to reject subterranean spacecraft (which
                    could have caused core dump) (this condition could
                    occur only if a bad ephemeris were provided)
   15-Jan-1997 PDN  Reorganized the error and warning priority system; 
                    improved documentation
   24-Apr-1997 PDN  Added logic to reject subterranean spacecraft (which
                    could have caused core dump) (this condition could
                    occur only if a bad ephemeris were provided)
   12-Feb-1999 PDN  Fixed nomenclature consistent with AM1 and later spacecraft
                    note that the function is also OK for TRMM, but in that case
                    please observe that angularVelocity is really (yaw rate, 
                    pitch rate, and roll rate in that order). Also deleted 
                    references to predicted leap seconds, which were used in
                    early I&T work only.
END_FILE_PROLOG:
******************************************************************************/
/******************************************************************************
BEGIN_PROLOG

TITLE: 
      This function PGS_CSC_GetFOV_Pixel() locates the intersection of 
      one or more lines of sight with the Earth.  It also provides
      the ECR pixel look vector, slant range and range rate.
 
NAME:   
   PGS_CSC_GetFOV_Pixel()

SYNOPSIS:
C:
    #include <PGS_CSC.h>

    PGSt_SMF_status
    PGS_CSC_GetFOV_Pixel( 
        PGSt_tag     spacecraftTag,
	PGSt_integer numValues,     
	char         asciiUTC[28],   
	PGSt_double  offsets[],    
	char         earthEllipsTag[20], 
	PGSt_boolean accurFlag,     
	PGSt_double  pixelUnitvSC[][3],
	PGSt_double  offsetXYZ[][3], 
	PGSt_double  latitude[],    
	PGSt_double  longitude[],  
	PGSt_double  pixelUnitvECR[][3],
	PGSt_double  slantRange[], 
	PGSt_double  velocDoppl[]) 
           
FORTRAN:
      include 'PGS_MEM_7.f'
      include 'PGS_EPH_5.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function
     >  pgs_csc_getfov_pixel(spacecrafttag,numvalues,asciiutc,offsets,
     >                       earthellipstag,accurflag,pixelUnitvSC,
     >                       offsetXYZ,latitude,longitude,pixelUnitvECR,
     >                       slantRange,velocDoppl)

      integer           spacecrafttag   
      integer           numvalues        
      character*27      asciiutc     
      double precision  offsets(*)      
      character*20      earthellipstag
      integer           accurflag 
      double precision  pixelUnitvSC(3,*)
      double precision  offsetXYZ(3,*)
      double precision  latitude(*)
      double precision  longitude(*)
      double precision  pixelUnitvECR(3,*)
      double precision  slantRange(*)
      double precision  velocDoppl(*) 
                           
 
DESCRIPTION:
 
  This function obtains the latitude and longitude of the intersection of a line
  of sight with the spheroidal Earth, the slant range from Spacecraft to look
  point, and the Doppler velocity along the line of sight.  The ECR pixel vector
  is also returned; it can be used, for example, to determine the zenith angle
  of the line of sight, The line of sight is defined by a unit vector in the
  Spacecraft frame of reference and a time.  (The unit vector along the line of
  sight is called a "look vector" in the sequel.)

  The Doppler velocity is true, in the sense that it is relative to the Earth's
  surface.

 
INPUTS:

 Name             Description                     Units       Min        Max
 ----------------------------------------------------------------------------

 spacecraftTag    Spacecraft identifier            N/A        N/A        N/A

 numValues        number of input time offsets     n/a         0         n/a
                  (to use ascii time with no 
                   offsets, set numValues =0
                   or set it =1 and make first 
                   [and only] offset = 0.0)

 asciiUTC         UTC start time in CCSDS          N/A      1979-06-30 see NOTES
                  ASCII Time A or B format

 offsets          array of time offsets      SI seconds    Max and Min such that
                                                           floating equivalent
							   of asciiUTC+offset
							   is between asciiUTC
							   Min and Max values

 EarthEllipsTag   Tag selecting Earth Ellipsoid    N/A        N/A        N/A
                  model 
     
 accurFlag        flag to regulate accuracy        N/A      PGS_FALSE  PGS_TRUE
 
 pixelUnitvSC     array of pixel unit vectors
                  in SC coords                     N/A        -1          1
 
 offsetXYZ        array of displacements of 
                  instrument boresight 
		  from SC nominal
		  center in SC coordinates          m        -120       +120

     (see overall limit for length of this vector in "RETURNS" section)
            (offsetXYZ is used only when accurFlag == PGS_TRUE)

OUTPUTS:

 Name            Description                      Units       Min        Max
 ----------------------------------------------------------------------------


  latitude        latitude of  the lookpoint       radians    -pi/2      pi/2

  longitude       longitude of the lookpoint       radians     -pi        pi

  pixelUnitvECR   ECR unit pixel vector              n/a        -1          +1

  slantRange      slant range: SC to lookpoint       m          0        100000000

  velocDoppl      range rate of the look
                     point (+ meaning "away")        m/s      -8000      8000
 
RETURNS:
  PGS_S_SUCCESS                  Success
  PGSCSC_W_MISS_EARTH            Look Vector fails to intersect Earth
  PGSTD_E_SC_TAG_UNKNOWN         Invalid Spacecraft tag
  PGSCSC_W_ZERO_PIXEL_VECTOR     Instrument pixel vector of zero length
  PGSCSC_W_BAD_EPH_FOR_PIXEL     Ephemeris Data missing for some pixels
  PGSCSC_W_INSTRUMENT_OFF_BOARD  Instrument offset from SC center is > 120 m
                                 which is considered unreasonably large
                                 (applicable only when accurFlag = PGS_TRUE
  PGSCSC_W_BAD_ACCURACY_FLAG     Accuracy Flag neither PGS_TRUE nor PGS_FALSE
  PGSCSC_E_BAD_ARRAY_SIZE        The user has supplied a negative number of
                                 time offsets
  PGSCSC_W_DEFAULT_EARTH_MODEL   Invalid EarthEllipsTag; WGS84 model used 
  PGSCSC_E_NEG_OR_ZERO_RAD       One of the Earth axes is zero or negative
  PGSMEM_E_NO_MEMORY             malloc operation for scratch memory failed
  PGS_E_TOOLKIT                  error in toolkit - for example, inconsistent
                                 error message from a subordinate function
  PGSEPH_E_BAD_EPHEM_FILE_HDR    no s/c ephem files had readable headers
  PGSEPH_E_NO_SC_EPHEM_FILE      no s/c ephem files could be found for input
  PGSCSC_W_SUBTERRANEAN          ephemeris data gave a subterranean position
  PGSTD_E_NO_LEAP_SECS           no leap seconds correction available for
                                  input time
  PGSTD_E_TIME_FMT_ERROR         format error in asciiUTC
  PGSTD_E_TIME_VALUE_ERROR       value error in asciiUTC
  PGSCSC_W_PREDICTED_UT1         status of UT1-UTC correction is predicted
  PGSTD_E_NO_UT1_VALUE           no UT1-UTC correction available


EXAMPLES:
C:
   #include <PGS_CSC.h>

    char asciiUTC[28]    = "1994-01-15T12:21:33.9939Z\0"; 
    PGSt_tag  spacecraftTag   = PGSd_EOS_AM;    
    char EarthEllipsTag[20]  = "WGS84";
    PGSt_double      offsets[4] = {0.0,0.1,2.0,30.0};
    PGSt_double      pixelUnitvSC[4][3];   
    PGSt_double      offsetXYZ[4][3]    ;  
    PGSt_integer numValues = 4;
    PGSt_boolean accurFlag = PGS_FALSE;

    PGSt_double     latitude[10];     
    PGSt_double     longitude[10];     
    PGSt_double     velocDoppl[10];     
    PGSt_double     slantRange[10];  
    PGSt_double     pixelUnitvECR[4][3];
    PGSt_SMF_status returnStat;
    PGSt_SMF_status	code;	
    char		msg[240];
    char		mnemonic[31];
    int i;
    int jj;
    
    for (i=0;i<4;i++)
        for(jj=0;jj<3 ;++jj)
            offsetXYZ[i][jj] = 0.0;

    ** initialize pixel unit vectors 
       All but the 3rd case hit Earth; to miss Earth reverse the last
       component of any other one **

    pixelUnitvSC[0][0] = 0.03;
    pixelUnitvSC[0][1] = 0.12;
    pixelUnitvSC[0][2] = 0.08;


    pixelUnitvSC[1][0] =  -0.2;
    pixelUnitvSC[1][1] =  0.12;
    pixelUnitvSC[1][2] =  0.6;

    ** This case will display error **

    pixelUnitvSC[2][0] =  -0.0;
    pixelUnitvSC[2][1] =  0.00;
    pixelUnitvSC[2][2] =  0.0;


    pixelUnitvSC[3][0] =  -0.2;
    pixelUnitvSC[3][1] =  -0.12;
    pixelUnitvSC[3][2] =  0.6;

    returnStat = PGS_CSC_GetFOV_Pixel(spacecraftTag,numValues,asciiUTC,offsets,
                                      EarthEllipsTag,accurFlag,pixelUnitvSC,
				      offsetXYZ,latitude,longitude,
				      pixelUnitvECR,santRange,velocDoppl);

    printf("  Toolkit return value:  %d\n\n",returnStat);
    
    PGS_SMF_GetMsg(&code,mnemonic,msg);
    printf("  Return %s: %s\n\n",mnemonic,msg);
    
    printf(" accurFlag ==  %d  Earth Tag == %s  ECR Pixels:\n"
           "%15.11lg    %15.11lg    %15.11lg\n"
	   "%15.11lg    %15.11lg    %15.11lg\n "
	   "%15.11lg    %15.11lg    %15.11lg\n"
	   "%15.11lg    %15.11lg    %15.11lg\n",
	   accurFlag,EarthEllipsTag,
	   pixelUnitvECR[0][0],pixelUnitvECR[0][1],pixelUnitvECR[0][2],
	   pixelUnitvECR[1][0],pixelUnitvECR[1][1],pixelUnitvECR[1][2],
	   pixelUnitvECR[2][0],pixelUnitvECR[2][1],pixelUnitvECR[2][2],
	   pixelUnitvECR[3][0],pixelUnitvECR[3][1],pixelUnitvECR[3][2]);
	   

    **  Test for some variable like latitude = PGSd_GEO_ERROR_VALUE before
        further processing to avoid processing pixels that missed Earth or had
	zero pixel vector.  In multi-pixel processing, results from good and bad
	pixels can be distinguished only by answers being PGSd_GEO_ERROR_VALUE;
	in single pixel processing return status indicates any error **
                                                              
   if(returnStatus != PGS_S_SUCCESS)
   {
     **  print results - latitude, longitude, etc; test errors, 
         take appropriate action **
   }

FORTRAN:
      implicit none
      parameter(numPixels = 4)
      integer           pgs_csc_getfov_pixel
      integer           returnstatus
      integer           spacecrafttag   
      integer           numvalues        
      character*27      asciiutc     
      double precision  offsets(numpixels)      
      character*20      earthellipstag
      integer           accurflag 
      double precision  pixelunitvSC(3,numpixels)
      double precision  offsetxyz(3,numpixels)
      double precision  latitude(numpixels)
      double precision  longitude(numpixels)
      double precision  pixelunitvecr(3,numpixels)
      double precision  slantrange(numpixels)
      double precision  velocdoppl(numpixels) 
      character*33      err
      character*241     msg

      data offsets/360.0, 720.0, 1080.0, 1600.0/
      asciiutc = '1991-07-27T11:04:57.987654Z'
      spacecrafttag = PGSd_EOS_AM

      do 1 jj = 1,3
      do 1 i = 1,4
         offsetxyz(jj,i) = 0.0;
 1    continue
!    
!    This puts instrument at the nominal SC center
!    For example, to put instrument on a 20 m boom fore of
!    SC center, make  offsetXYZ(1,i) = 20.0 for each i

! initialize pixel unit vectors 
! All but the 3rd case hit Earth; to miss Earth reverse the last
! component of any other one 

      pixelunitvsc(1,1) = 0.03;
      pixelunitvsc(2,1) = 0.12;
      pixelunitvsc(3,1) = 0.08;
      

      pixelunitvsc(1,2) =  -0.2;
      pixelunitvsc(2,2) =  0.12;
      pixelunitvsc(3,2) =  0.6;

!   This case will display error 

      pixelUnitvsc(1,3) =  -0.0;
      pixelUnitvsc(2,3) =  0.00;
      pixelUnitvsc(3,3) =  0.0;


      pixelUnitvsc(1,4) =  -0.2;
      pixelUnitvsc(2,4) =  -0.12;
      pixelUnitvsc(3,4) =  0.6;

      returnstatus = pgs_csc_getfov_pixel(spacecrafttag,numvalues,
     >                                    asciiutc,offsets,
     >                                    earthellipstag,accurflag,
     >  				  pixelunitvsc,offsetxyz,
     >                                    latitude,longitude,
     >   				  pixelunitvecr,slantrange,
     >          			  velocdoppl)

!  Print output values

!  Test for some variable like latitude = PGSd_GEO_ERROR_VALUE before
!  further processing to avoid processing pixels that missed Earth or
!  had zero pixel vector

      if (returnstatus .ne. pgs_s_success) then
         pgs_smf_getmsg(returnstatus, err, msg)
         write(*,*) err, msg
      endif
                               

NOTES:  
   An accuracy flag is required, allowing two accuracy levels:

     Normal or PGS_FALSE

             - do ECI to ECR transformation at moment of taking data
             - consider instrument axis to pass through nominal
               center of spacecraft

     High  or PGS_TRUE

             - do ECI to ECR transformation with approximate allowance 
               for Earth rotation during the light travel time.  
               (spherical Earth approximation).   This will slow the 
               calculation slightly.

             - user must supply vector offsetXYZ that represents the 
               displacement in meters of the instrument boresight from 
               nominal spacecraft center.  (Only the part of the dis-
               placement orthogonal to the look vector will have an effect.)  
               Users invoking the High Accuracy option but wishing not to 
               take advantage of this feature should supply zeros for the 
               components of offsetXYZ.

     The maximum error in omitting this calculation is approximately 
     as follows for a worst case of a spacecraft at 700 km altitude, 
     crossing the equator and looking E or W:

               Error due to Earth Motion in Time of Flight of Light

            Nadir Angle (deg)     Slant Range (km)  Worst case Error (m) if 
                                                     accurFlag = PGS_FALSE

                  0                  700                    1.1
                 30                  830                    1.3 
                 40                  945                    1.5 
                 50                 1200                    1.9 
                 55                 1410                    2.1 
                 60                 1770                    2.7 
                 64                 2440                    3.7 
 
   The nature of the error is a smooth distortion such that points near either
   the East or the West limb would be assigned a longitude slightly to the West
   in comparison with points near nadir.  The effect could be somewhat exag-
   gerated, for some orbits, in terms of illumination changes near the
   terminator.

   Caution: The user is advised that the spacecraft ephemeris refers to the
   nominal center of the spacecraft.  The displacements of individual
   instruments relative to the center of the spacecraft are taken into account
   herein through the vector offsetXYZ.  When the flag "accurFlag" is set to
   PGS_TRUE, the user should specify the instrument coordinates relative to
   spacecraft center (in meters) with this vector.  It WILL be used by the
   present function, so if the user does not actually wish to employ it, then
   offsetXYZ must be set to zero (all three components).  If "accurFlag" is set
   to PGS_FALSE, the displacement is ignored.

   TIME ACRONYMS:
     
     UT1 is:  Universal Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a 
     new leap second event is announced, contains dates of leap second events.
     If an input date is outside of the range of dates in the file (or if the 
     file cannot be read) the function PGS_TD_LeapSec() which reads the file
     will return a calculated value of TAI-UTC based on a linear fit of the data
     in the table as of late 1997.  This value of TAI-UTC is relatively crude 
     estimate and may be off by as much as 1.0 or more seconds. The warning 
     return status from PGS_TD_LeapSec() is promoted to an error level by any 
     Toolkit function receiving it, which will terminate the present function.

   ASCII UTC:

     Toolkit functions that accept an ASCII time as input require the time to
     be UTC time in CCSDS ASCII Time Code A or CCSDS ASCII Time Code B format.
     Toolkit functions that return an ASCII time return the UTC time in CCSDS
     ASCII Time Code A format (see CCSDS 301.0-B-2 for details).
     (CCSDS => Consultative Committee for Space Data Systems)

      The general format is:
 
          YYYY-MM-DDThh:mm:ss.ddddddZ (Time Code A)
          YYYY-DDDThh:mm:ss.ddddddZ   (Time Code B)
 
          where:
              -,T,: are field delimiters
              Z is the (optional) time code terminator
              other fields are numbers as implied:
                Time Code A:
                   4 Year digits, 2 Month, 2 Day, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second
                Time Code B:
                   4 Year digits, 3 Day of year, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second


   TIME OFFSETS:

      This function accepts an ASCII UTC time, an array of time offsets and the
      number of offsets as input.  Each element in the offset array is an offset
      in seconds relative to the initial input ASCII UTC time.

      An error will be returned if the number of offsets specified is less than
      zero.  If the number of offsets specified is actually zero, the offsets
      array will be ignored.  In this case the input ASCII UTC time will be
      converted to Toolkit internal time (TAI) and this time will be used to
      process the data.  If the number of offsets specified is one (1) or
      greater, the input ASCII UTC time will converted to TAI and each element
      'i' of the input data will be processed at the time: (initial time) +
      (offset[i]).

      Examples:
         
         if numValues is 0 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
         then input[0] will be processed at time 432000.0 and return output[0]

	 if numValues is 1 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
         then input[0] will be processed at time 432000.0 + offsets[0] and
         return output[0]

         if numValues is N and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
         then each input[i] will be processed at time 43000.0 + offsets[i] and
         the result will be output[i], where i is on the interval [0,N)

   ERROR HANDLING:

      This function processes data over an array of times (specified by an input
      ASCII UTC time and an array of time offsets relative to that time). 

      If processing at each input time is successful the return status of this
      function will be PGS_S_SUCCESS (status level of 'S').

      If processing at ALL input times was unsuccessful the status level of the
      return status of this function will be 'E'.

      If processing at some (but not all) input times was unsuccessful the
      status level (see SMF) of the return status of this function will be 'W'
      AND all high precision real number (C: PGSt_double, FORTRAN: DOUBLE
      PRECISION) output variables that correspond to the times for which
      processing was NOT successful will be set to the value:
      PGSd_GEO_ERROR_VALUE.  In this case users may (should) loop through the
      output testing any one of the aforementioned output variables against
      the value PGSd_GEO_ERROR_VALUE.  This indicates that there was an error in
      processing at the corresponding input time and no useful output data was
      produced for that time.  

      Note: a return status with a status of level of 'W' does not necessarily
      mean that some of the data could not be processed.  The 'W' level may
      indicate a general condition that the user may need to be aware of but
      that did not prohibit processing.  For example, if an Earth ellipsoid
      model is required, but the user supplied value is undefined, the WGS84
      model will be used, and processing will continue normally, except that the
      return status will be have a status level of 'W' to alert the user that
      the default earth model was used and not the one specified by the user.
      The reporting of such general warnings takes precedence over the generic
      warning (see RETURNS above) that processing was not successful at some of
      the requested times.  Therefore in the case of any return status of level
      'W', the returned value of a high precision real variable generally should
      be examined for errors at each time offset, as specified above.

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
   PGSTK - 1080, 1083, 0750, 0930 

DETAILS:
   Basis of algorithm:
   
     See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger,
      for more information on the algorithm.
      This is available on the EHDS Home Page:
      http://edhs1.gsfc.nasa.gov
        at the location (as of early 1997)
      http://edhs1.gsfc.nasa.gov:8001/waisdata/doscw/html/tp4450202.html

   Also see comments in the code
 
   Overall Methodology:

   Starts with Spacecraft Tag, Earth Model Tag, Pixel Unit Vector
   in SC coordinates, and UTC Time.

   Gets Spacecraft position, velocity and attitude (ECI)

   Invokes a  SC to ECI transformation to put instrument look unit vector in ECI
 
   Aberrates look vector opposite to velocity in ECI to compensate aberration.
   Renormalizes the unit vector.

   Makes a preliminary crude check based on a spherical Earth that look vector
   in ECI does not miss the Earth; [a more accurate redundant safeguard is in
   underlying function PGS_CSC_LookPoint(); but time is saved by omitting the
   calls to get ECR vectors when look vector clearly misses the Earth].

   Applies ECI to ECR transformation to position, velocity, and pixel unit
    vector "pixelUnitvECI", obtaining "pixelUnitvECR"

   Invokes PGS_CSC_LookPoint() to find ECR intersection of line of sight with
   ellipsoid

   Calculates the geodetic coordinates at the Lookpoint from the algorithm:
               longitude = atan2(y,x)
               latitude = atan2(z,(sqrt(x*x+y*y)*(1.0-ecc2)))

   (P. R. Escobal, "Methods of Orbit Determination", Wiley, N.Y. 1965, p. 28,
    using the ratio of his Eqs. 1.59, 1.58, and the fact, from his Eq. 1.35,
    that (1.0 - ecc2) = (C/A)*(C/A), where C = polar radius, A = equatorial 
    radius of the Earth, ecc2 = square of Earth's eccentricity as an ellipse).

        The input Pixel vector is checked for zero length and an error message
        is returned in that case, with no further processing on that pixel, but
        the latitude, longitude, slant range and Doppler velocity are set to
        PGSd_GEO_ERROR_VALUE.  The ECR Pixel vector is returned correctly to
        allow independent checking of Earth intersection, unless ALL the pixel
        vectors supplied are of zero length or miss intersecting the Earth).
        Users should test for one of the PGSd_GEO_ERROR_VALUE values to avoid
        further processing, because according to Toolkit policy in case of any
        error that makes further processing of a pixel useless, a value of
        PGSd_GEO_ERROR_VALUE is returned.  (For example, the latitude or slant
        range can be checked).

        Caution: To save time the ECR Pixel Vector may be filled with useless
	numbers when none of the pixels can be processed into a lookpoint
	result; for example, when all the look vectors miss Earth.  Always check
	latitude, longitude, slant range, or Doppler velocity for the value
	PGSd_GEO_ERROR_VALUE before using the ECR pixel vector and discard those
	cases.

        When some pixels process successfully, however, but some have zero input
        values (all 3 components zero), then the function faithfully yields zero
        ECR pixel vectors for those cases.

        It is anticipated that in successful processing, the ECR pixel vector
        can be used in PGS_CSC_ZenithAzimuth() to get the zenith angle and
        azimuth of the look vector.  Furthermore, the CBP Ephemeris tool can be
        used to obtain the ECI Sun and Moon vectors, which can be put in ECR.
        Then the Solar and Lunar zenith and azimuth can be obtained; specular
        reflection conditions can be checked and the angle between the Sun and
        Moon unit vectors and the look vector can be found so as to evaluate
        phase angle effects.

GLOBALS:
   None

FILES:
   The function PGS_CSC_ECItoECR() called herein requires:
      utcpole.dat
      leapsec.dat

   The function PGS_EPH_EphemAttit() called herein requires:
      spacecraft ephemeris file(s) 

   The function  PGS_CSC_GetEarthFigure() called herein requires:
      earthfigure.dat


FUNCTIONS CALLED:
   PGS_CSC_GetEarthFigure()
   PGS_EPH_EphemAttit()
   PGS_EPH_ManageMasks()
   PGS_CSC_quatRotate()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()    
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()
   PGS_CSC_ECItoECR
   PGS_SMF_TestStatusLevel
 
END_PROLOG:
******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>
#include <PGS_MEM.h>

#define speedLight   299792458.0     /* speed of light in m/s */

/* The following is used when the ONLY problem is that the line
   of sight misses the Earth - we do return the ECR pixel vector
   so the user can do further work with it - such as limb sounder
   or near-miss analysis */
#define SET_MISS_POINT(index) \
     latitude[(index)] = PGSd_GEO_ERROR_VALUE; \
     longitude[(index)] = PGSd_GEO_ERROR_VALUE; \
     slantRange[(index)] = PGSd_GEO_ERROR_VALUE; \
     velocDoppl[(index)] = PGSd_GEO_ERROR_VALUE;

/* The following is used when no useful data can be obtained
    for the time offset for some reason other than line-of
    sight missing Earth */
#define SET_BAD_POINT(index) \
     latitude[(index)] = PGSd_GEO_ERROR_VALUE; \
     longitude[(index)] = PGSd_GEO_ERROR_VALUE; \
     slantRange[(index)] = PGSd_GEO_ERROR_VALUE; \
     pixelUnitvECR[(index)][0] = PGSd_GEO_ERROR_VALUE; \
     pixelUnitvECR[(index)][1] = PGSd_GEO_ERROR_VALUE; \
     pixelUnitvECR[(index)][2] = PGSd_GEO_ERROR_VALUE; \
     velocDoppl[(index)] = PGSd_GEO_ERROR_VALUE;

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_GetFOV_Pixel()"

PGSt_SMF_status
PGS_CSC_GetFOV_Pixel(                /* Gets Pixel Center */
    PGSt_tag     spacecraftTag,      /* unique spacecraft identifier */
    PGSt_integer numValues,          /* number of values requested  */
    char         asciiUTC[28],       /* UTC time in CCSDS ASCII Type A format */
    PGSt_double  offsets[],          /* time offsets */
    char         earthEllipsTag[20], /* Tag for selecting Earth Ellipsoid 
					model */
    PGSt_boolean accurFlag,          /* flag to allow for light travel time */
    PGSt_double  pixelUnitvSC[][3],  /* pixel unit vector in SC coordinates */
    PGSt_double  offsetXYZ[][3],     /* displacement of instrument boresight 
                                        from SC nominal center in SC coord's */
    PGSt_double  latitude[],         /* latitude of the look point */
    PGSt_double  longitude[],        /* longitude of the look point */
    PGSt_double  pixelUnitvECR[][3], /* pixel Unit vector in ECR for other 
					use */
    PGSt_double  slantRange[],       /* slant range  Spacecraft to lookpoint */
    PGSt_double  velocDoppl[])       /* Doppler velocity of the look point 
					(+ is "away") */ 
{
    PGSt_double  (*positionECI)[3];  /* ECI position */
    PGSt_double  (*velocityECI)[3];  /* ECI velocity */
    PGSt_double  (*attitQuat)[4];    /* spacecraft to ECI rotation quaternion */
    PGSt_integer (*qualityFlags)[2]; /* quality flags from s/c EPH tool */
    PGSt_double  (*oldTime);         /* back dated time offsets needed in high
					accuracy case - default to offsets 
                                        otherwise */
    PGSt_double  (*eulerAngle)[3];  /* Euler Angles in the given order */
    PGSt_double  (*angularVelocity)[3];
                                    /* J2000 anguler velocity proj. on the 
                                       body axes */
    PGSt_double  (*unitPix6ECI)[6];  /* Pixel unit vector in ECI padded with 
                                        3 zeros */
    PGSt_double  (*unitPix6ECR)[6];  /* Pixel unit vector in ECR padded with 
                                        3 zeros */
    PGSt_double  (*posvelECI)[6];    /* SC State vector (pos/vel) in ECI */
    PGSt_double  (*posvelECR)[6];    /* SC State vector (pos/vel) in ECR */
    PGSt_integer (*goProcess);       /* process flag indicating usable pixel
                                        or error condition */
    /*    Processing Flag:
          Values:  0 - all OK, continue
	  1 - zero pixel vector 
	  2 - MISS EARTH error - line of sight misses Earth
	  3 - other errors 
	  */

    PGSt_double  eciXYZ[3];          /* XYZ instrument offset in ECI  */
    PGSt_double  xLook[3];           /* rectangular ECR coordinates 
                                        of the look point  */
    PGSt_double  equatRad_A;         /* Earth semi-major axis (m) */
    PGSt_double  polarRad_C;         /* Earth semi-minor axis (m) */
    PGSt_double  axrat;              /* axis ratio of Earth's ellipsoidal 
                                        figure, polar over equatorial  */
    PGSt_double  altitude;           /* approximate altitude of the 
                                        spacecraft */
    PGSt_double  meanEarthRad;       /* mean Earth radius for coarse calculation
                                        of light travel time  */
    PGSt_double  nadirAng;           /* angle of look vector to nadir 
                                        (uncorrected for aberration) */
    PGSt_double  normVEC;            /* norm of ECI unit look vector after 
                                        aberration corr */
    PGSt_double  fromCenter;         /* apprx. distance of SC to Earth Center */
    PGSt_double  slantApprox;        /* approximate slant range for light 
                                        travel time */
    PGSt_double  lookDotSCtoEarthV;  /* dot product of the Look unit vector and 
                                        the vector from SC to the Earth 
					Center */

    /* This function contains two statics which are not THREADSAFE: countMsgPix,
        countMesgLOS.  These are used to stop warning messages at 50 and 25.
        The messages from all threads will go to the same file, this will make
        the order unpredictable.  Therefore there is no reason to protect each
        individual threads number of messages.  After any 50 or 25 messages the
        file will issue No more messages. */

    static PGSt_integer countMsgPix; /* counter to avoid deluge of warnings */
    static PGSt_integer countMesgLOS;/* counter to avoid deluge of warnings */

    PGSt_boolean okFlag = PGS_FALSE; /* flag to keep track if there are any good
                                        pixels; if not, we won't call
					ECItoECR */  
    register PGSt_integer countTime; /* loop counter for time offsets */
    PGSt_integer worstERR;           /* indicator of error priority per pixel 
                                        0 = no problems
                                        6 = subterranean spacecraft
                                        5 = worst = ZERO_PIXEL_VECTOR 
                                        4 = INSTRUMENT_OFF_BOARD
                                        3 = BAD_EPH_FOR_PIXEL
                                        2 = BAD_ACCURACY_FLAG
                                        1 = MISS_EARTH = a minor problem */
    register short        iXYZ;      /* loop counter for space coordinates */
    PGSt_integer maxValues;          /* counter cannot equal or exceed this 
                                        value */     
    PGSt_integer qualityFlagsMasks[2]; /* user definable quality flag masks
					  used to check validity of data
					  returned by PGS_EPH_EphemAttit() */
    void *memPtr = NULL;             /* pointer to memory allocated 
					dynamically within this function */
    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
							 PGS_SMF_GetMsg() */
    char         specifics[PGS_SMF_MAX_MSG_SIZE];     /* detailed error msg */
  
    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */
    PGSt_SMF_status code;           /* status code returned by 
				       PGS_SMF_GetMsg()*/
    PGSt_SMF_status statusReturned = PGS_S_SUCCESS; /* status from other 
						       functions, initialized
						       success */
    PGSt_SMF_status statusReturnEPH = PGS_S_SUCCESS;/* status from ephemeris 
						       tool initialized 
						       success */
    PGSt_SMF_status  retrn2 = PGS_S_SUCCESS;  /* return status, initialized
						 to success */

    /****  BEGIN EXECUTABLE CODE *****/

    /* Test size of arrays */

    if ( numValues < 0 )
    {
	/* NO negative array size is allowed, return with an error status. */

	returnStatus = PGSCSC_E_BAD_ARRAY_SIZE;
	retrn2 = PGS_SMF_SetStaticMsg( returnStatus , FUNCTION_NAME );
	return  returnStatus ;
    }

    /* get equatorial and polar radii of Earth  */

    returnStatus = PGS_CSC_GetEarthFigure(earthEllipsTag,&equatRad_A,
                                          &polarRad_C);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCSC_W_DEFAULT_EARTH_MODEL:
	sprintf(specifics,"%s %s %s","Earth Tag",earthEllipsTag,
		"not found - using WGS84 default");
	retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	returnStatus = PGSCSC_W_DEFAULT_EARTH_MODEL;
	break;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
   
    /* issue an error if the equatorial radius is negative */
    if (equatRad_A <= 0)
    {
	returnStatus = PGSCSC_E_NEG_OR_ZERO_RAD;
	retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,"Earth equatorial radius is"
				       " negative or zero",
				       FUNCTION_NAME);
        return returnStatus;
    }
    
    /* issue an error if the polar radius is negative */
    if (polarRad_C <= 0)
    {
	returnStatus = PGSCSC_E_NEG_OR_ZERO_RAD;
	retrn2 = PGS_SMF_SetDynamicMsg(returnStatus, "Earth polar radius is "
				       "negative or zero",FUNCTION_NAME);
	return returnStatus;
    }
        
    /* initialize */

    worstERR = 0;

    /* check for invalid accuracy flag - in this case process as if false */
    if((accurFlag != PGS_TRUE) && (accurFlag != PGS_FALSE))
    {
	returnStatus = PGSCSC_W_BAD_ACCURACY_FLAG; 
         if(worstERR < 2)
         {
            worstERR = 2;
         }
	retrn2 = PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);  
    }
    
    /**    INITIALIZATION of Index Range  **/

    maxValues =  (numValues) ? numValues :1;

    /* begin malloc operations  */
    retrn2 = PGS_MEM_Malloc(&memPtr,(sizeof(PGSt_integer)*(1 + 2)*maxValues) +
			    sizeof(PGSt_double)*maxValues*
			    (3+3+4+3+3+6+6+6+6+1)); 

    switch (retrn2)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSMEM_E_NO_MEMORY:
	return retrn2;
      default:
	PGS_SMF_SetUnknownMsg(retrn2,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* assign pointer for ECI position */
    positionECI = (PGSt_double (*)[3])  (memPtr);
    
    /* assign pointer for ECI velocity */
    velocityECI = positionECI + maxValues;
    
    /* assign pointer for Euler angles */
    eulerAngle = velocityECI + maxValues;

    /* assign pointer for angular velocity components */
    angularVelocity = eulerAngle + maxValues;

    /* assign pointer for spacecraft to ECI rotation quaternion */
    attitQuat = (PGSt_double (*)[4])  (angularVelocity + maxValues);

    /*  assign pointer for padded ECI Unit Pixel Vector */
    unitPix6ECI = (PGSt_double (*)[6])  (attitQuat + maxValues);

    /*  assign pointer for padded ECR Unit Pixel Vector */
    unitPix6ECR = unitPix6ECI + maxValues;

    /*  assign pointer for ECI Spacecraft State  Vector */
    posvelECI = unitPix6ECR + maxValues;

    /*  assign pointer for ECR Spacecraft State  Vector */
    posvelECR = posvelECI + maxValues;

    /*  assign pointer for backdated times when accurFlag is PGS_TRUE */
    oldTime  = (PGSt_double *)  (posvelECR + maxValues);

    /* assign pointer for quality flags */
    qualityFlags = (PGSt_integer(*)[2])  (oldTime + maxValues);

    /*  assign pointer for process status flags */
    goProcess  = (PGSt_integer *)  (qualityFlags + maxValues);

    /*  end pointer assignments */

    /* INITIALIZE oldTime array */

    /*  In low accuracy case the array of time offsets is passed to ECItoECR
        without alteration; in the high accuracy case it is back-dated before
        going into the array "oldTime" which is passed to ECItoECR.  When a
        pixel is bad, then each offset is set to the previous, to avoid
        work in PGS_CSC_ECItoECR() .  To protect user input we here copy to
        a fresh array. */ 

    if(numValues > 0 )
    {
	memcpy((char*) oldTime,(char*) offsets,sizeof(PGSt_double)*maxValues);
    }
    else
    {
	oldTime[0] = 0.0;
    }


    /* Get attitude quaternions and position and velocity vectors 
       If an error occurs return */  
 
    statusReturnEPH =  PGS_EPH_EphemAttit(spacecraftTag,
					  numValues,
					  asciiUTC,
					  offsets,
					  PGS_TRUE,
					  PGS_TRUE,
					  qualityFlags,
					  positionECI,
					  velocityECI,
					  eulerAngle,
					  angularVelocity,
					  attitQuat);
    switch (statusReturnEPH)
    {
      case PGS_S_SUCCESS:
        break;
      case PGSTD_E_SC_TAG_UNKNOWN:
        PGS_MEM_Free(memPtr);
	return statusReturnEPH;
      case PGSEPH_W_BAD_EPHEM_VALUE:
        PGS_SMF_GetMsg(&code,mnemonic,msg);
        if(code != statusReturnEPH)
	    PGS_SMF_GetMsgByCode(statusReturnEPH,msg); 
        returnStatus = PGSCSC_W_BAD_EPH_FOR_PIXEL;
        if(worstERR < 3)
        {
           worstERR = 3;
        }
        break;
      case PGSTD_E_NO_LEAP_SECS:
        PGS_SMF_GetMsg(&code,mnemonic,msg);
        if(code != statusReturnEPH)
	    PGS_SMF_GetMsgByCode(statusReturnEPH,msg);  
        returnStatus = statusReturnEPH;
        break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
      case PGSEPH_E_NO_SC_EPHEM_FILE:
        PGS_MEM_Free(memPtr);
        PGS_SMF_SetStaticMsg(statusReturnEPH,FUNCTION_NAME);
        return statusReturnEPH;
      case PGSEPH_E_NO_DATA_REQUESTED:
      default:
        PGS_MEM_Free(memPtr);
        PGS_SMF_SetUnknownMsg(statusReturnEPH,FUNCTION_NAME);
        return PGS_E_TOOLKIT;
    }  /* end  switch (returnStatus)  */
   

    /* get the user's qualityFlags masks (returns system defaults if the user
       has not set any mask values) */

    statusReturned = PGS_EPH_ManageMasks(PGSd_GET, qualityFlagsMasks);
    switch (statusReturned)
    {
      case PGS_S_SUCCESS:
	break;
	
      case PGSPC_E_DATA_ACCESS_ERROR:
	statusReturned = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(statusReturned, "unexpected error accessing "
			      "Process Control File", FUNCTION_NAME);
	
      case PGS_E_TOOLKIT:
	return statusReturned;
	
      default:
	PGS_SMF_SetUnknownMsg(statusReturned, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* make sure the qualityFlags masks for ephemeris data and attitude data
       contain the "no data" bit set by the toolkit */

    qualityFlagsMasks[0] = qualityFlagsMasks[0] | PGSd_NO_DATA;
    qualityFlagsMasks[1] = qualityFlagsMasks[1] | PGSd_NO_DATA;
    
    /**    INITIALIZATION of Earth figure Data   **/

    /* The following is used only in an estimate of slant range
       to get the light travel time, a small correction  */

    meanEarthRad = (equatRad_A + equatRad_A + polarRad_C)/3.0;

    /* evaluate axis ratio of Earth spheroid */

    axrat =  polarRad_C/equatRad_A; /* this is (1-f) where f = flattening */
	   
    /* BEGIN MAIN LOOP - first true  operations */
    /* This pass does some initializations and also checks for zero pixel "unit"
       vectors, which will not be processed further */

    for(countTime=0;countTime < maxValues;countTime++)
    {

	/* Initialization In Loop */

	/* bad quality flags */

	if (((qualityFlagsMasks[0] & qualityFlags[countTime][0]) != 0) &&
	    ((qualityFlagsMasks[1] & qualityFlags[countTime][1]) != 0))
	{
	    goProcess[countTime]  = -500;
	    SET_BAD_POINT(countTime)
	    if(countMsgPix < 50)
	    {
		countMsgPix += 1;
		sprintf(specifics,"%s%s %d","Bad or Missing Ephemeris Data for",
			" offset", (int) countTime);
		PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	    }
	}
	else  /* OK Initialization */
	{
	    goProcess[countTime] = 0;
	    okFlag = PGS_TRUE;
	}
    }

    /*  Return if there are no good ephemeris/attitude data */

    if(okFlag != PGS_TRUE)
    {
	returnStatus = PGSCSC_W_BAD_EPH_FOR_PIXEL;
	sprintf(specifics,"%s %s","There are no time values ",
		"with valid ephemeris data");
	PGS_MEM_Free(memPtr);
	PGS_SMF_SetDynamicMsg(returnStatus,specifics,
			      FUNCTION_NAME);
	return returnStatus;
    }

    /* re - initialize check for some good pixels */

    okFlag = PGS_FALSE; 

    /* Check that the input Pixel length is nonzero to save processing because
       nothing can be done otherwise; later we'll also normalize for the test
       for Earth intersection etc.  */

    for(countTime=0;countTime < maxValues;countTime++)
    {
	if(goProcess[countTime] == 0)
	    normVEC = PGS_CSC_Norm(pixelUnitvSC[countTime]);
	if(fabs(normVEC) < EPS_64 ) 
	{    
	    goProcess[countTime] = 1;
	    SET_BAD_POINT(countTime)
	    returnStatus  = PGSCSC_W_ZERO_PIXEL_VECTOR;
	    worstERR = 5;
	    if(countMsgPix < 50)
	    {
		countMsgPix += 1;
		sprintf(specifics,"%s%d","ZERO Pixel Vector, offset Number ",
			(int) countTime );
		PGS_SMF_SetDynamicMsg(returnStatus,specifics,
				      FUNCTION_NAME);
	    }
	}  /* end  if(fabs(normVEC) < EPS_64) */    
     else
	{
	    okFlag = PGS_TRUE;
	}  /* ensures some further processing if any pixel vector is good */
    }  /* END MAIN LOOP pass 1 - have checked for valid pixel unit vectors */

    /*  Return if there are no good pixel unit vectors */
    if(okFlag != PGS_TRUE)
    {
	sprintf(specifics,"%s %s","There are no pixels to process ",
		"with nonzero pixel vectors");
	PGS_MEM_Free(memPtr);
	PGS_SMF_SetDynamicMsg(returnStatus,specifics,
			      FUNCTION_NAME);
	return returnStatus;
    }

    /*** in high accuracy case use instrument offset on platform ***/
    if(accurFlag == PGS_TRUE)
    {
	/* re-initialize okFlag to PGS_FALSE as we work on offsets */
 
	okFlag = PGS_FALSE;

	/*  MAIN LOOP pass 1.5 - special case accurFlag == PGS_TRUE */
	for(countTime=0;countTime < maxValues;countTime++)
	{
	    /* do only good pixels */
	    if(goProcess[countTime] == 0)
	    {
		if ( (offsetXYZ[countTime][0] * offsetXYZ[countTime][0]
		      + offsetXYZ[countTime][1] * offsetXYZ[countTime][1]
		      + offsetXYZ[countTime][2] * offsetXYZ[countTime][2]) 
		     > 14400.0)
		{
		    returnStatus = PGSCSC_W_INSTRUMENT_OFF_BOARD;
		    if(worstERR < 4)
		    {
		       worstERR = 4;
		    }
		    if(countMsgPix < 50)
		    {
		       countMsgPix += 1;
		       sprintf(specifics,"%s %d","Instrument off board for offset",
			       (int) countTime);
		       PGS_SMF_SetDynamicMsg(returnStatus,specifics,
					  FUNCTION_NAME);
		    }
		    goProcess[countTime] += 200;
		    SET_BAD_POINT(countTime)
		}
		else  /* displacement is reasonable */
		{ 
		    /*  Put the XYZ displacement in ECI, then correct instrument
			position  */

		    statusReturned = PGS_CSC_quatRotate(*(attitQuat+countTime),
							*(offsetXYZ+countTime),
							eciXYZ);

		    if(statusReturned != PGS_S_SUCCESS)
		    {
			goProcess[countTime] += 4;
			SET_BAD_POINT(countTime)
			if(countMsgPix < 50)
                        {
			   countMsgPix += 1;
			   sprintf(specifics,"%s%s%d","PGS_CSC_quatRotate() ",
				"failure offset Number ",(int) countTime);
			    PGS_SMF_SetDynamicMsg(statusReturned,specifics,
				 FUNCTION_NAME);
			}
                        if(worstERR < 3)
                        {
                          worstERR = 3;
                        }
			returnStatus = statusReturned;
		    }  
		    else  /* do the correction */
		    {
			okFlag = PGS_TRUE;
			for (iXYZ=0; iXYZ<3; ++iXYZ)
			{
			    positionECI[countTime][iXYZ] += eciXYZ[iXYZ];
			}
		    } /* end test on status of quatRotate */
		}  /* end length check on offset */
	    } /* end  if(goProcess) ... work on high accuracy case */
	} /* END MAIN LOOP in high accuracy case */ 
    }  /* end "if(accurFlag == PGS_TRUE)"   */

    /*  Return if there are no good pixel unit vectors */
    if(okFlag != PGS_TRUE)
    {
       goto cleanup;
    }

    /* re-initialize okFlag to PGS_FALSE as we test for Earth intersection */
 
    okFlag = PGS_FALSE;

    /* BEGIN MAIN LOOP - second operations - put pixel in ECI and see if it is
       likely to intersect Earth (later do more accurately in ECR) .  Idea 
       is to avoid calling ECItoECR in case of clean miss; if within a
       sphere whose radius is Earth's semi-major axis, then we'll do more. */

    for(countTime=0;countTime < maxValues;countTime++)
    {
	if(goProcess[countTime] == 0)
	{
	    /*** Put Instrument Look Vector in ECI ***/

	    statusReturned = PGS_CSC_quatRotate(attitQuat[countTime],
						pixelUnitvSC[countTime],
						unitPix6ECI[countTime]);

	    /* call SMF tool to convert toolkit return value to message */
	    if(statusReturned != PGS_S_SUCCESS)
	    {   
		goProcess[countTime] += 8;
		SET_BAD_POINT(countTime)
		if(countMsgPix < 50)
		{
		   countMsgPix += 1;
		   sprintf(specifics,"%s%s%d","PGS_CSC_quatRotate() failure ",
			"offset Number ",(int) countTime );
		   PGS_SMF_SetDynamicMsg(statusReturned,specifics,
				      FUNCTION_NAME);
		}
		returnStatus = statusReturned;
	    }  
	    /*** Instrument Look Vector is now in ECI except for aberration ***/

	    /* Since aberration is not quite negligible, we'll aberrate 
	       the pixel before checking for Earth intersection */

	    /*** Aberrate the look vector to true ECI direction and pad for 
	      transformation to ECR (requires a 6 vector)   ***/

	    /*** Reference on Aberration: Explan. Supplem. to Astron. Almanac 
	      p. 129 ***/

	    for(iXYZ = 0; iXYZ < 3; iXYZ++)
	    {
		unitPix6ECI[countTime][iXYZ] -= velocityECI[countTime][iXYZ]
		    /speedLight;
		unitPix6ECI[countTime][iXYZ+3] = 0.0;
	    }

	    /*** Renormalize to get Unit Vector (p. 129, Eq. 3.252-1) ***/
	    /* this operation cannot have problem with a zero-division because
	       we started with a unit vector and subtracted a vector whose 
	       components are of order v/c, v= SC velocity */


	    normVEC = PGS_CSC_Norm(unitPix6ECI[countTime]);


	    for(iXYZ = 0; iXYZ  < 3; iXYZ++)
	    {
		unitPix6ECI[countTime][iXYZ] = unitPix6ECI[countTime][iXYZ]/
					       normVEC;
	    }
    
	    /* ROUGH CHECK THAT LOOK VECTOR DOESN'T MISS EARTH */

	    /* The dot product of the Look unit vector and the vector from
	       spacecraft to the Earth Center enters the calculation of slant
	       range and the calculation of "nadirAng" = the angle between the
	       Look Unit Vector and Geocentric (NOT Geodetic) nadir.  This
	       calculation is done with the unaberrated look vector because only
	       an approximate nadir angle and slant range are needed here.  The
	       former is used only to perform a cursory check that the Earth is
	       in view of the instrument, and the latter only in case the user
	       wishes the ultra-precision correction for Earth motion during
	       light travel time.  The effect of any slant range inaccuracy on
	       the latter correction is second order for instruments looking at
	       zenith angles less than 85 degrees; users with limb sounders
	       might want a more accurate slant range.  Note that when the Earth
	       is in view, however, a more accurate slant range is indeed
	       returned - it is corrected for look vector aberration.  For a
	       spheroidal (bi-axial, not tri-axial) Earth, the slant range is
	       essentially unaffected by Earth motion during the time of flight
	       of light, so the calculation herein is rigorous even at large
	       look vector zenith angles, except for refraction effects.  */
    
	    lookDotSCtoEarthV = 
		- (unitPix6ECI[countTime][0] * positionECI[countTime][0] +
		   unitPix6ECI[countTime][1] * positionECI[countTime][1] +
		   unitPix6ECI[countTime][2] * positionECI[countTime][2]);

	    /* Now the test for line of sight intersection with the Earth.  To
	       be conservative, use the larger (equatorial) Earth radius in this
	       test; if the line of sight actually misses, this will be detected
	       in the function PGS_CSC_LookPoint */

	    fromCenter = PGS_CSC_Norm(positionECI[countTime]);


            if(fromCenter < equatRad_A) 
            {
		SET_BAD_POINT(countTime)
                if(worstERR < 6)
	        {
	           worstERR = 6;
                }
                returnStatus = PGSCSC_W_SUBTERRANEAN;
		goProcess[countTime] += 17;
            }
            else
            {
	       nadirAng = acos(lookDotSCtoEarthV/fromCenter);
	       if(nadirAng > asin(equatRad_A/fromCenter))
	       {
		   returnStatus = PGSCSC_W_MISS_EARTH;
		    if(worstERR < 1)
		    {
		       worstERR = 1;
		    }
		   goProcess[countTime] += 16;
		   SET_MISS_POINT(countTime)
		   if(countMesgLOS < 25)
		   {
		       sprintf(specifics,"%s%s%d","The line of sight misses the ",
			       "Earth for offset # ",(int) countTime);
		       retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,specifics,
						      FUNCTION_NAME);
		       countMesgLOS += 1;
		   }
	       } 
	       else  /* here we set okFlag good if there is any pixel at all
		        which got this far and did not miss Earth! */
	       {
		   okFlag = PGS_TRUE;
	       }

	       /*** END OF QUICK CHECK WE DON'T MISS EARTH ***/
            }
	} /* end section restricted to goProcess  == 0  */
	else
	{
	    for(iXYZ = 0; iXYZ  < 3; iXYZ++)
	    {
		/* ECItoECR takes arrays - can't feed it uninitialized values */
		unitPix6ECI[countTime][iXYZ] = 0.0;
		unitPix6ECI[countTime][iXYZ+3] = 0.0;
	    }
	}

	/*  Return if all pixels are bad - no point in further processing */

	if ( (countTime == maxValues -1) && (! okFlag)  )
	{
	    PGS_MEM_Free(memPtr);
	    PGS_SMF_SetDynamicMsg(returnStatus,
				  "No Valid Pixels to Process ", FUNCTION_NAME);
	    return returnStatus;
	}

	/* CONTINUE MAIN LOOP - third operations - for those pixels with good
	   pixel vectors and not clearly missing Earth, find Earth intersection,
	   if any.  (A very few may still miss Earth, because the cursory check
	   in the second pass was based on the equatorial Earth radius to be
	   conservative; at that stage, before calling ECI to ECR, it is
	   impossible to determine the intersection any better.  Now, with the
	   true Earth aspect known, as accurate intersection based on a oblate
	   Earth model can be determined.)  */

	/* Place SC position and velocity in a 6-vector that can be passed to
	   the ECI to ECR function .  This will be done even for pixels with bad
	   pixel vectors to avoid sending NaN's or whatever to 
	   PGS_CSC_ECItoECR() */
    
	for (iXYZ=0; iXYZ<3; ++iXYZ)
	{
	    posvelECI[countTime][iXYZ]   = positionECI[countTime][iXYZ];
	    posvelECI[countTime][iXYZ+3] = velocityECI[countTime][iXYZ];
	}

	/* Soon we'll put the Spacecraft Ephemeris and Pixel look vector in ECR.
	   This has to be done at one consistent time - either the observation
	   time (normal accuracy case) or the time the light left the Earth
	   (high accuracy case).  It might seem that we should do the pixel at
	   the observation time and the SC ephemeris at the emission time, but
	   actually when is being done it to slightly change the Earth aspect,
	   and we must work in a consistent reference frame for position and
	   pixel look vector. */
	
	/* For highest accuracy we want this done as of the time that light left
	   the Earth.  Calculate approximate slant Range: */
	
	if( goProcess[countTime] == 0)
	{
	    if( accurFlag == PGS_TRUE )
	    {
		altitude    = fromCenter - equatRad_A;  /* approximate only */
		slantApprox = lookDotSCtoEarthV - 
			      sqrt(lookDotSCtoEarthV*lookDotSCtoEarthV - 
				   2.0*altitude*meanEarthRad - 
				   altitude*altitude);

		/* Get time when light left the Earth heading towards the
			    instrument */

		if(numValues == 0)
		{
		    oldTime[countTime] = 0.0 - slantApprox/speedLight;
		}
		else
		{
		    oldTime[countTime] = offsets[countTime] - 
					 slantApprox/speedLight;
		}
	    } /*  end case (accurFlag == PGS_TRUE)  */
	} /* end case goProcess[...] == 0 */
    }  /* END MAIN LOOP - second and third operations sets */

    /*  Put the Pixel unit vectors in ECR  */

    statusReturned = PGS_CSC_ECItoECR(maxValues,asciiUTC,oldTime,
				      unitPix6ECI,unitPix6ECR);

    if (statusReturned != PGS_S_SUCCESS)
    {    
	/* test for severity of error - if "E" abort, else continue */

	PGS_SMF_GetMsg(&code,mnemonic,msg);
	retrn2 =  PGS_SMF_TestStatusLevel(statusReturned);
	if (retrn2 == PGS_SMF_MASK_LEV_E)
	{
	    PGS_MEM_Free(memPtr);
	    retrn2 = PGS_SMF_SetDynamicMsg(statusReturned,msg,
					   FUNCTION_NAME);
	    return statusReturned;
	}
        else
        {
            PGS_SMF_GetMsg(&code,mnemonic,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
        }
    }   /* end status testing  ECI->ECR for pixel vectors */ 

    /* Now finally get the Spacecraft Vector in ECR */

    statusReturned = PGS_CSC_ECItoECR(maxValues,asciiUTC,oldTime,posvelECI,
				      posvelECR);

    if (statusReturned != PGS_S_SUCCESS)
    {
	/* test for severity of error - if "E" abort, else continue */
	retrn2 =  PGS_SMF_TestStatusLevel(statusReturned);
	if (retrn2 == PGS_SMF_MASK_LEV_E)
	{
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    PGS_MEM_Free(memPtr);
	    retrn2 = PGS_SMF_SetDynamicMsg(statusReturned,msg,FUNCTION_NAME);
	    return statusReturned;
	}
        else
        {
            PGS_SMF_GetMsg(&code,mnemonic,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
        }
    }  /* other level that can be generated is "W" - continue in that case */ 

    /* BEGIN MAIN LOOP - fourth operations - save ECR Pixel vector and find
       Look Point, slant range, Doppler velocity */
 
    for(countTime=0; countTime < maxValues; countTime++)
    {

        /*  save the ECR pixel vector for later use, e.g. zenith angle work */
        /*  This is done even for pixels that miss Earth, just in case user
            wishes to check that condition separately.  Note, however,
            that if all pixels miss Earth, no ECR Pixel vectors will be
            calculated. */

	if( (goProcess[countTime] == 0) || (goProcess[countTime] == 16) )
	{
	    for(iXYZ = 0; iXYZ  < 3; iXYZ++)
	    {
		pixelUnitvECR[countTime][iXYZ] = unitPix6ECR[countTime][iXYZ];
	    }
	}  /* end branch on goProcess */

	/* Now we're ready to call PGS_CSC_LookPoint()  */

	if( goProcess[countTime] == 0)
	{
	    statusReturned = PGS_CSC_LookPoint(posvelECR[countTime],
					       pixelUnitvECR[countTime],
					       equatRad_A,equatRad_A,
					       polarRad_C,slantRange+countTime,
					       xLook); 
	    switch (statusReturned)
	    {
	      case PGS_S_SUCCESS:
		/* special case at N or S pole */
		if( (fabs(xLook[0]) < EPS_64) && (fabs(xLook[1]) < EPS_64) )
		{
		    longitude[countTime] = 0.0;
		    latitude[countTime] = atan2(xLook[2],0.0);
		}
		else /* ordinary case at away from poles */
		{   
		    longitude[countTime] = atan2(xLook[1],xLook[0]);
		    latitude[countTime] = atan2(xLook[2],axrat*axrat*
						sqrt(xLook[0]*xLook[0] +
						     xLook[1]*xLook[1]));
		}
		velocDoppl[countTime] = 
		    ( - unitPix6ECR[countTime][0] * posvelECR[countTime][3] 
		      - unitPix6ECR[countTime][1] * posvelECR[countTime][4] 
		      - unitPix6ECR[countTime][2] * posvelECR[countTime][5]); 
		break;
	      case PGSCSC_W_MISS_EARTH:
		if(worstERR < 1)
		{
		   worstERR = 1;
		}
		goProcess[countTime] += 32;
		SET_MISS_POINT(countTime)
		if(returnStatus == PGS_S_SUCCESS) returnStatus = statusReturned;
		if(countMesgLOS < 25)
		{
		    sprintf(specifics,"%s%s%d","The line of sight narrowly ",
			    "misses the Earth for offset # ",(int) countTime);
		    retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,specifics,
						   FUNCTION_NAME);
		    countMesgLOS += 1;
		}
		break;
	      case PGSCSC_W_ZERO_PIXEL_VECTOR:
		sprintf(specifics,"%s%s%d","PGS_CSC_LookPoint() detected a "
			," zero Look vector (impossible) offset # ",
			(int) countTime);
		PGS_MEM_Free(memPtr);
		retrn2 = PGS_SMF_SetDynamicMsg(statusReturned,specifics,
					       FUNCTION_NAME);
		return PGS_E_TOOLKIT;
	    } /* end switch (statusReturned) */
        } /* end condition goProcess[countTime] == 0 */

    }  /* END MAIN LOOP */
 
    /* Finally, recover the worst warning/error condition(s).
       Note that since messages were already issued to logfile,
       here we only set a final message and the return value */ 

    cleanup:

    switch (worstERR)
    {
       case 6:
       {
          returnStatus = PGSCSC_W_SUBTERRANEAN;
          break;
       }
       case 5:
       {
	  returnStatus  = PGSCSC_W_ZERO_PIXEL_VECTOR;
          break;
       }
       case 4:
       {
	  returnStatus  = PGSCSC_W_INSTRUMENT_OFF_BOARD;
          break;
       }
       case 3:
       {
	  returnStatus  = PGSCSC_W_BAD_EPH_FOR_PIXEL;
          break;
       }
       case 2:
       {
	  returnStatus  = PGSCSC_W_BAD_ACCURACY_FLAG;
          break;
       }
       case 1:
       {
	  returnStatus  = PGSCSC_W_MISS_EARTH;
          break;
       }
    }
    
    if(returnStatus == PGS_S_SUCCESS)
    {
        PGS_MEM_Free(memPtr);
        retrn2 = PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    }
    else
    {
        PGS_MEM_Free(memPtr);
        PGS_SMF_GetMsgByCode(returnStatus,msg);
        retrn2 = PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
    }

    return returnStatus; 
}
