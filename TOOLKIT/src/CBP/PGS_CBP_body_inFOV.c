/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CBP_body_inFOV.c

DESCRIPTION:
   This file contains the function PGS_CBP_body_inFOV().
   
AUTHOR:
   Anubha Singhal     / Applied Research Corporation
   Peter Noerdlinger  / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   25-Oct-1994 PDN  Designed				
   10-Nov-1994 AS   Coded
   12-Dec-1994 AS   Used crossproducts to calculate the intersection of the 
                    poles instead of simultaneous equations
   13-Dec-1994 AS   Changed Pluto's radius to include Charon
   13-Dec-1994 AS   Mallocs changed to used PGS_MEM tools and only to use
                    one single malloc
   19-Dec-1994 AS   Updated to allow moving FOV's and fixed up prolog
   09-Jan-1995 AS   Made changes suggested in code inspection
   14-Feb-1995 AS   Added error handling for cbID equal to SSBARY or EMBARY
   25-May-1995 PDN  Fixed error detection on return from 
                    PGS_CSC_PointInFOVgeom()
   05-Jun-1995 PDN  Additional error messaging to logfile
   19-Jun-1995 PDN  Brought notes section up to date  
   07-Jul-1995 PDN  Improved error handling
   07-Jul-1995 GTSK Altered returned values of cb_SCvector to be in meters (this
                    vector WAS being returned normalized), fixed up prolog
   12-Jul-1995 PDN  fixed prolog (details)  
   14-Jul-1995 PDN  fixed normalization error line 1214 - found a vertex inside
                    disc of CB when it should not have done so
   18-Jul-1995 PDN  fixed logic for cases where boundary arcs are to be checked,
                    so an array "checkArc" is saved telling which ones are 
                    relevant; deleted extraneous "break" statement which was
                    preventing the correct initialization of the cross products
                    of certain perimeter vectors.
   27-May-1995 PDN  Deleted references to PGSTD_W_PRED_LEAPS

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE: 
   Indicate if a CB is in the FOV.

NAME:   
   PGS_CBP_body_inFOV()

SYNOPSIS:
C:
   #include <PGS_CBP.h>

   PGSt_SMF_status
   PGS_CBP_body_inFOV(  
              PGSt_integer numValues,
              char         asciiUTC[28],
              PGSt_double  offsets[],
              PGSt_tag     spacecraftTag,
              PGSt_integer numFOVperimVec,
              PGSt_double  inFOVvector[][3],
              PGSt_double  *perimFOV_vectors,
              PGSt_tag     cbID,
              PGSt_boolean inFOVflag[],
              PGSt_double  cb_vector[][3],
              PGSt_double  cb_SCvector[][3])
	      
FORTRAN:
      include 'PGS_MEM_7.f'
      include 'PGS_CBP_6.f'
      include 'PGS_EPH_5.f'
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function pgs_cbp_body_infov(numvalues,asciiutc,offsets,
     >                                    spacecrafttag,numfovperimvec,
     >                                    infovvector,perimfov_vectors,
     >                                    cbid,infovflag,cb_vector,
     >                                    cb_scvector)
   
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)
      integer          spacecrafttag
      integer          numfovperimvec
      double precision infovvector(*)
      double precision perimfov_vectors(3,numfovperimvec,*)
      integer          cbid
      integer          infovflag(*)
      double precision cb_vector(3,*)
      double precision cb_scvector(3,*)
       
DESCRIPTION:
   Given a celestial body (CB) identifier (as in the CBP tools) and a FOV
   description, tool returns a flag or flags indicating if the CB is in the
   FOV, as well as the coordinates of the CB in SC coordinates.
   Alternatively, the user can specify CB identifier 999 or PGSd_STAR and 
   supply the ECI vector to the body.

INPUTS:
   Name             Description        Units    Min                 Max
   ----             -----------        -----    ---                 --- 
   numValues        number of time     N/A      1                   any
                    gridpoints

   asciiUTC         UTC start time     N/A      ** see NOTES section below ** 

   offsets          array of time      sec      ** see NOTES section below **
                    offsets

   spacecraftTag    unique spacecraft  N/A      N/A                 N/A
                    identifier       

   numFOVperimVec   number of vectors  N/A      3                   any
                    defining FOV
                    perimeter

   inFOVvector      vector in FOV, in  N/A      N/A                 N/A
                    SC coordinates

   perimFOV_vectors vectors in SC      N/A      N/A                 N/A
                    coords defining
                    FOV's; MUST be
                    sequential around
                    FOV "see notes";
		    the middle
		    dimension must be
		    exactly the same
		    as numFOVperimVec
		    because of the way
		    the array dimensioning
		    works in the function

   cbId             celestial body ID  N/A      1                   13
                    (Earth not included - see PGS_CSC_EarthPt_FOV)

   cb_vector        ECI vectors of CB  meters   see PGS_CBP_Earth_CB_vector 
                                                notes
                    (this is an input only when cbId = 999, meaning user input
		     of ECI vector for CB - see notes)
                                                   
OUTPUTS:
   Name           Description                Units   Min       Max
   ----           -----------                ----    ---       ---
   inFOVflag      PGS_TRUE if CB is in       N/A     N/A       N/A
                  FOV - see notes

   cb_SCvector    vector of CB in SC coords  meters  see PGS_CBP_body_inFOV()
                                                     notes

RETURNS:
   PGS_S_SUCCESS		 Success
   PGSCSC_E_SC_TAG_UNKNOWN	 Invalid Spacecraft tag
   PGSCBP_W_BAD_CB_VECTOR        one or more bad vectors for requested times
   PGSCBP_W_ERROR_IN_BODYINFOV   an error occurred in determining if the CB 
                                 was in the FOV for at least one case
   PGSCSC_E_INVALID_FOV_DATA     FOV perimeter vectors or inFOVvector invalid
   PGSCSC_E_FOV_TOO_LARGE        FOV specification outside algorithmic limits
   PGSCBP_E_TIME_OUT_OF_RANGE    initial time is outside the ephemeris 
	                         bounds
   PGSTD_E_BAD_INITIAL_TIME      initial time is incorrect 
   PGSTD_E_NO_LEAP_SECS          no leap seconds correction available for
                                 input time
   PGSCSC_W_DATA_FILE_MISSING    the data file earthfigure.dat is missing
   PGSCBP_E_UNABLE_TO_OPEN_FILE  unable to open file
   PGSCBP_E_INVALID_CB_ID	 invalid celestial body identifier
   PGSCBP_W_EARTH_CB_ID          the tool PGS_CSC_Earthpt_FOV() must be used 
                                 to check for earth points in the FOV
   PGSMEM_E_NO_MEMORY            no memory is available to allocate vectors
   PGSCSC_E_BAD_ARRAY_SIZE       incorrect array size
   PGSEPH_E_BAD_EPHEM_FILE_HDR   no s/c ephem files had readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE     no s/c ephem files could be found for input
                                 times
   PGSEPH_E_NO_DATA_REQUESTED    both orb and att flags are set to false
   PGS_E_TOOLKIT                 something unexpected happened
               
EXAMPLES:
C:
   #define   ARRAY_SIZE   3
   #define   PERIMVEC_SIZE 4
   PGSt_SMF_status    returnStatus;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_integer       numValues;
   PGSt_integer       numFOVperimVec;
   PGSt_double        inFOVvector[ARRAY_SIZE][3] = { {0.0,0.0,100.0},
                                                     {0.0,0.0,200.0},
						     {0.0,0.0,300.0}
						     };
   PGSt_double        perimFOV_vectors[ARRAY_SIZE][PERIMVEC_SIZE][3]=
                                  { {100.0,100.0,100.0},
				    {-100.0,100.0,100.0},
				    {-100.0,-100.0,100.0},
				    {100.0,-100.0,100.0},
				    {200.0,200.0,200.0},
				    {-200.0,200.0,200.0},
				    {-200.0,-200.0,200.0},
				    {200.0,-200.0,200.0},
				    {300.0,200.0,200.0},
				    {-200.0,300.0,200.0},
				    {-200.0,-300.0,300.0},
				    {300.0,-200.0,200.0},
				  };  		      
   PGSt_boolean       inFOVflag[ARRAY_SIZE];
   PGSt_double        cb_SCvector[ARRAY_SIZE][3];
  
   numValues = ARRAY_SIZE;
   numFOVperimVec = PERIMVEC_SIZE;
   strcpy(asciiUTC,"1995-06-21T11:29:30.123211Z");

   returnStatus = PGS_CBP_body_inFOV(numValues,asciiUTC,offsets,PGSd_TRMM,
                                     numFOVperimVec,inFOVvector,
				     perimFOV_vectors,PGSd_MOON,
				     inFOVflag,NULL,cb_SCvector);

   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:
      implicit none
      integer           pgs_cbp_body_infov

      integer	        returnstatus
      integer           spacecrafttag
      integer           numvalues
      integer           spacecrafttag
      integer           numfovperimvec
      integer           cbid
      integer           infovflag(3)

      double precision  offsets(3)
      double precision  infovvector(3,3)
      double precision  perimfov_vectors(3,4,3)
      double precision  cb_vector(3,3)
      double precision  cb_scvector(3,3)

      character*27      asciiutc
      character*33      err
      character*241     msg
      
      data offsets/3600.0, 7200.0, 10800.0/
      
      infovvector(1,1) = 0.0
      infovvector(1,2) = 0.0
      infovvector(1,3) = 0.0
      
      infovvector(2,1) = 0.0
      infovvector(2,2) = 0.0
      infovvector(2,3) = 0.0
      
      infovvector(3,1) = 0.0
      infovvector(3,2) = 0.0
      infovvector(3,3) = 0.0
      
      perimfov_vectors(1,1,1) = 100.0
      perimfov_vectors(2,1,1) = 100.0
      perimfov_vectors(3,1,1) = 100.0
      
      perimfov_vectors(1,2,1) = -100.0
      perimfov_vectors(2,2,1) = 100.0
      perimfov_vectors(3,2,1) = 100.0
      
      perimfov_vectors(1,3,1) = -100.0
      perimfov_vectors(2,3,1) = -100.0
      perimfov_vectors(3,3,1) = 100.0
      
      perimfov_vectors(1,4,1) = 100.0
      perimfov_vectors(2,4,1) = -100.0
      perimfov_vectors(3,4,1) = 100.0
      
      perimfov_vectors(1,1,2) = 200.0
      perimfov_vectors(2,1,2) = 200.0
      perimfov_vectors(3,1,2) = 200.0
      
      perimfov_vectors(1,2,2) = -200.0
      perimfov_vectors(2,2,2) = 200.0
      perimfov_vectors(3,2,2) = 200.0
      
      perimfov_vectors(1,3,2) = -200.0
      perimfov_vectors(2,3,2) = -200.0
      perimfov_vectors(3,3,2) = 200.0
      
      perimfov_vectors(1,4,2) = 200.0
      perimfov_vectors(2,4,2) = -200.0
      perimfov_vectors(3,4,2) = 200.0
      
      perimfov_vectors(1,1,3) = 300.0
      perimfov_vectors(2,1,3) = 300.0
      perimfov_vectors(3,1,3) = 300.0
      
      perimfov_vectors(1,2,3) = -300.0
      perimfov_vectors(2,2,3) = 300.0
      perimfov_vectors(3,2,3) = 300.0
      
      perimfov_vectors(1,3,3) = -300.0
      perimfov_vectors(2,3,3) = -300.0
      perimfov_vectors(3,3,3) = 300.0

      perimfov_vectors(1,4,3) = 300.0
      perimfov_vectors(2,4,3) = -300.0
      perimfov_vectors(3,4,3) = 300.0

      asciiutc = '1995-06-21T11:04:57.987654Z'
      numvalues = 3
      numfovperimvec = 4

      returnstatus = pgs_cbp_body_infov(numvalues,asciiutc,offsets,
     >                                  pgsd_trmm,numfovperimvec,
     >                                  infovvector,perimfov_vectors,
     >	                                pgsd_moon,infovflag,null,
     >                                  cb_scvector)

      if (returnstatus .ne. pgs_s_success) then
	  pgs_smf_getmsg(returnstatus, err, msg)
          write(*,*) err, msg
      endif

NOTES:
   The FOV is always specified in SC coordinates; for an instrument fixed to
   the SC, use the same FOV description always; for scanning instruments, user
   should provide the description appropriate to the scan instant.

   numFOVperim must be at least 3. The tool determines if any part of the CB 
   requested lies within the perimeter defined by the vectors 
   perimFOV_vectors[][][3]. The first index in C (last in FORTRAN) is the time
   offset index, and the second MUST be sequential around the FOV perimeter. 
   The vector inFOVvector[][3] MUST lie within the FOV. It need not be 
   central, but there will be loss of efficiency if not. The last index in C 
   (first in FORTRAN) on these vectors is for X,Y and Z components in SC 
   coordinates. It is necessary for the user to supply a vector within the FOV
   for the reason that on the surface of a sphere, a closed curve or 
   "perimeter" does not have an inside nor outside, except by arbitrary 
   definition; i.e. this vector tells the algorithm which part of sky is 
   inside FOV, which outside. 

   The vectors "perimFOV_vectors[][][3]" defining the FOV perimeter can be in
   clock- or counter-clockwise sequence . 

   Initially, the tool checks for Earth blockage by locating Earth center and 
   assuming a spherical model (in cases where Earth figure is important a more
   sophisticated but slower test is used, based on a WGS84 model).  The call
   to PGS_CSC_ECItoSC() with Earth Center as input will result in warning
   messages to the logfile: "PGSCSC_W_BELOW_SURFACE:(code) location is below 
   surface" , which should be ignored. ("code" is a code number from the SMF
   utilities). 

   The tool may be used on the Sun, Moon, and planets other than the Earth, in
   which case the cbID must be selected from the standard set (see the tool 
   PGS_CBP_Earth_CB_Vector()). The tool may also be used on another object 
   (such as a star), in which case cbID should be set = 999 and the ECI J2000 
   coordinates of the star must be supplied in cb_vector[]. The Sun, Moon and 
   planets have finite radii, as specified in the Table below; CB's with cdID 
   = 999 (PGSd_STAR) are assumed to be of negligible radius.

   Note on Finite Size of CB: Since a primary use of this tool will be to 
   determine if the Sun, Moon, or a planet intrudes into the FOV, it is 
   important to allow for the finite size of the object.  For this purpose, 
   the Moon and Planets are replaced with spheres of the following radii, 
   which are projected on the celestial sphere:

   CB                      radius (km)       explanation
   Sun                     7 e 5
   Moon                    1739              allows for topography
   Mercury                 2440
   Venus                   6055
   Earth                    n/a              use tool PGS_CSC_Earthpt_FOV()
   Mars                    3397              ignore satellites
   Jupiter                 1890 e 3          include Galilean
                                             satellites
   Saturn                  1225 e 3          include rings, satellites to Titan
   Uranus                  25600             planet only
   Neptune                 24800             planet only
   Pluto                   19600             planet and Charon
   999 (STAR)              0.0               a star, or user-defined point

   In general, we have included satellites down to the 10th magnitude. 

   In the case that the celestial body position is invalid for a particular 
   time then the corresponding cb_SCvector will be set to PGSd_GEO_ERROR_VALUE.

   If the CB disk overlaps the FOV only behind the Earth's equatorial bulge 
   and the overlap is barely hidden by it, and the FOV has a sharp corner 
   protuding past the Earth limb it is possible in rare cases that a false 
   positive answer will issue.
   
   The subsidiary function PGS_CSC_FOVconicalHull() performs certain quality
   checks on the FOV and inFOVvector.  If the dot product of the inFOVvector
   and any perimeter vector is negative, the FOV is deemed too big and an
   error status PGSCSC_E_INVALID_FOV_DATA is returned.  If the smallest dot 
   product of any perimeter vector with the inFOVvector equals or nearly equals 
   1.0 then that perimeter vector and the infov vector are essentially the same 
   so  the same return PGSCSC_E_INVALID_FOV_DATA is issued.  The number used 
   for the test on the cosine of the dot product, 0.99999999999, amounts to 
   defining an angle of ~0.9 arc sec between the perimeter vertex and the 
   inFOVvector. A distance smaller than that could lead to erratic performance 
   of the algorithm due to truncation error, and represents an unlikely FOV. 

   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a new
     leap second event is announced, contains actual (definitive) and predicted
     (long term; very approximate) dates of leap second events.  The latter can
     be used only in simulations.   If an input date is outside of the range of
     dates in the file (or if the file cannot be read) the function will use
     a calculated value of TAI-UTC based on a linear fit of the data known to be
     in the table.  This value of TAI-UTC is relatively crude estimate and may
     be off by as much as 1.0 or more seconds.  Thus, when the function is used
     for dates in the future of the date and time of invocation, the user ought
     to carefully check the return status.  The status level of the return 
     status of this function will be 'W' (at least) if the TAI-UTC value used
     is a predicted value.  The status level will be 'E' if the TAI-UTC value
     is calculated (although processing will continue in this case, using the
     calculated value of TAI-UTC).

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

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

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

REQUIREMENTS:
   PGSTK - 0780, 0680

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   the spacecraft ephemeris is required by the ephemeris access tool
   earthfigure.dat
   utcpole.dat
   leapsec.dat

FUNCTIONS CALLED:
   PGS_CBP_Sat_CB_Vector()
   PGS_CSC_ECItoSC()
   PGS_CSC_PointInFOVgeom()
   PGS_CSC_FOVconicalHull()
   PGS_CSC_EarthOccult()
   PGS_CSC_GetFOV_Pixel()
   PGS_CSC_Norm()
   PGS_CSC_crossProduct()
   PGS_CSC_dotProduct()
   PGS_MEM_Malloc()
   PGS_MEM_Free()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetUnknownMsg()    
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_GetMsg()
   PGS_SMF_GetMsgByCode()

END_PROLOG
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <stdlib.h>
#include <PGS_TD.h>
#include <PGS_CSC.h>
#include <PGS_CBP.h>
#include <PGS_EPH.h>
#include <PGS_MEM.h>

extern PGSt_integer PGS_SMF_FEQ(PGSt_double, PGSt_double);

/* name of this function */

#define FUNCTION_NAME "PGS_CBP_body_inFOV()"

/* radii in meters of various celestial bodies */

#define RSUN 7.0e8
#define RMOON 1739000.0
#define RMERCURY 2440000.0
#define RVENUS 6055000.0
#define RMARS 3397000.0
#define RJUPITER 1890.0e6
#define RSATURN 1225.0e6
#define RURANUS 25600000.0
#define RNEPTUNE 24800000.0
#define RPLUTO 19600000.0

#define PIDIVTWO 1.57079632679489

PGSt_SMF_status
PGS_CBP_body_inFOV(                      /* returns a flag or flags indicating
                                            if the Celestial body or star
                                            is in the FOV */ 
    PGSt_integer numValues,              /* number of values requested */
    char         asciiUTC[28],           /* UTC start time in CCSDS ASCII Time
                                            Code A or B format */
    PGSt_double  offsets[],              /* time offset array */
    PGSt_tag	 spacecraftTag,          /* unique spacecraft identifier */ 
    PGSt_integer numFOVperimVec,         /* number of vectors defining FOV 
                                            perimeter */
    PGSt_double  inFOVvector[][3],       /* vector in FOV in SC coordinates */
    void         *tempPerimFOV_vectors,  /* vectors in SC coords defining 
                                            FOV's; must be sequential around
                                            FOV */
    PGSt_integer cbID,                   /* celestial body ID */
    PGSt_boolean inFOVflag[],            /* PGS_TRUE if CB in FOV */
    PGSt_double  cb_vector[][3],         /* ECI vectors of CB */
    PGSt_double  cb_SCvector[][3])       /* vector of CB in SC coords */
{
    PGSt_double  cb_SCunitVec[3];   /* nomalized vector of CB in SC coords */
    PGSt_double  *perimFOV_vectors; /* vectors in SC coords defining FOV's; 
				       must be sequential around FOV */
    PGSt_double	 norm;	            /* norm of input vector */
    PGSt_double  dotProduct;        /* dot product of vector */
    PGSt_double  perp_pole[3];      /* pole of the perpendicular from the 
				       center of the CB to the pole of the
				       great circle through two perimeter
				       vectors */
    PGSt_double  xf[3];             /* intersection of the great circles */
    PGSt_double  xfOpp[3];          /* intersection of the great circles */
    PGSt_double  xfNear[3];         /* intersection closest to the center of
				       the CB or star */
    PGSt_double  xfNearToPerim1[3]; /* vector from xfNear to first perimeter
				       vector */
    PGSt_double  xfNearToPerim2[3]; /* vector from xfNear to second perimeter
				       vector */
    PGSt_double  rAngCB;            /* angular radius of the celestial body */
    PGSt_boolean cleanMiss;         /* set to PGS_TRUE if the disk does not
				       intersect the arc or its prologation */
    PGSt_boolean invalidFOVdata;    /* set to PGS_TRUE if invalid fov data is
				       found */
    PGSt_boolean checkEarthBulge;
    PGSt_double  cos_rAngCB;        /* cosine of the angular radius of the
				       celestial body */
    PGSt_double  rLinCB;            /* linear radius of celestial body */
    PGSt_double  inFOVvecNorm[3];   /* in FOV vector normalized */
    PGSt_double  rMdotProduct;      /* smallest dot product between the inFOV
				       vector and any perimeter vector */
    PGSt_double  (*extremeVec)[3];  /* vector in SC coordinates that points at
				       the part of the CB most distant from 
				       Earth center */
    PGSt_double  (*scToEcenter)[3]; /* position vectors from spacecraft to 
				       earth center */
    PGSt_double  (*posECI)[3];      /* ECI position vector */
    PGSt_double  (*perimFOV_vecCopy)[3]; /* copy of FOV perimeter vectors */
    PGSt_double  (*perimFOV_vecNorm)[3]; /* FOV perimeter vectors normalized 
					    after call to 
					    PGS_CSC_FOVconicalHull()*/
    PGSt_double  (*perimFOV_crossProduct)[3]; /* cross product of consecutive
						 perimeter vectors */
    PGSt_double  (*perimFOV_crossProductNorm);/* norm of the cross product of
						 consecutive perimeter
						 vectors */
   /*   This function contains a static which is not THREADSAFE: countMsg
        This is used to stop warning messages at 50.
        The messages from all threads will go to the same file, this will make
        the order unpredictable.  Therefore there is no reason to protect each
        individual thread's number of messages.  After any 50 messages the
        file will issue No more messages. */

    static PGSt_integer countMsg;   /* counter to avoid deluge of warnings */

    PGSt_integer counter;           /* loop counter */
    PGSt_integer counter2;          /* loop counter */
    PGSt_integer counter3;          /* loop counter */
    short        cnt4;              /* loop counter */
    PGSt_integer maxValues;         /* counter cannot equal or exceed this 
				       value */
    char         *startMem = NULL;  /* pointer to start of memory block 
				       allocated */
    char         *ptrMem;           /* pointer to a location in the memory
				       block allocated */
    PGSt_double  latitudeFOV[1];    /* latitude of the look point */
    PGSt_double  longitudeFOV[1];   /* longitude of the look point */
    PGSt_double  pixelUnitECR[1][3];/* ECR unit pixel vector */
    PGSt_double  slantRange[1];     /* slant range; SC to lookpoint */
    PGSt_double  velocDoppl[1];     /* Doppler velocity of the look point */
    PGSt_integer fovValues;         /* number of offsets passed to 
                                       PGS_CSC_GetFOV_Pixel */
    PGSt_double  timeOffset[1];     /* time offset parameter used in call to
				       GetFOV_Pixel() */
    PGSt_boolean (*checkArc);       /* Tells if CB meets arc or its 
                                       prolongation into a great circle */
    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* message returned by
     					                 PGS_SMF_GetMsg() */
    char         specifics[PGS_SMF_MAX_MSG_SIZE];     /* detailed error msg */


    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */
    PGSt_SMF_status holdStatus;     /* to hold error returnStatus until end */
    PGSt_boolean    saveStatus;     /* PGS_TRUE if any good offsets processed */
    PGSt_SMF_status returnStatus1;  /* error status of call to second or later
                                       functions */
    PGSt_SMF_status code;           /* status code returned by
				       PGS_SMF_GetMsg()*/
      
    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS;  
    holdStatus   = PGS_S_SUCCESS;  
    saveStatus   = PGS_FALSE;

    /* assign tempPerimFOV_vectors to perimFOV_vectors */

    perimFOV_vectors  = (PGSt_double *) tempPerimFOV_vectors;

    /* Test size of arrays */

    if ( numValues < 0 )
    {
	/* NO negative array size is allowed, return with an error status. */

	returnStatus = PGSCSC_E_BAD_ARRAY_SIZE;
	PGS_SMF_SetStaticMsg( returnStatus,FUNCTION_NAME);
	return returnStatus;
    }
    else if (numValues == 0)
	maxValues = 1;
    else
	maxValues = numValues;
    
    /* check that the cbID is not equal to the EARTH in which case the tool
       PGS_CSC_Earthpt_FOV() must be used or not equal to SSBARY or EMBARY 
       since barycenters are not valid celestial objects */
    
    switch (cbID)
    {
      case PGSd_EARTH:
	returnStatus = PGSCBP_W_EARTH_CB_ID;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "use the tool PGS_CSC_Earthpt_FOV() to check for "
			      "earth points in the FOV",FUNCTION_NAME);
	return returnStatus;
      case PGSd_SSBARY:
      case PGSd_EMBARY:
	returnStatus = PGSCBP_E_INVALID_CB_ID;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "invalid celestial body - barycenters are not "
   			      "valid celestial objects",FUNCTION_NAME);
	return returnStatus;  
    }
    
    /* Get spacecraft to CB vector */

    /* if the celestial body is not a star call PGS_CBP_Sat_CB_Vector to get
       the spacecraft coordinates of the celestial body */

    if (cbID != PGSd_STAR)
    {
	returnStatus1 = PGS_CBP_Sat_CB_Vector(spacecraftTag,numValues,asciiUTC,
					      offsets,cbID,cb_SCvector);
      
	/* Check errors returned from PGS_CBP_Sat_CB_Vector */
    
	switch(returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCSC_E_SC_TAG_UNKNOWN:
	  case PGSTD_E_SC_TAG_UNKNOWN:
	    return returnStatus1;
	  case PGSTD_E_NO_LEAP_SECS:
          case PGSCBP_W_BAD_CB_VECTOR:
	  case PGSCSC_W_BELOW_SURFACE:
            holdStatus = returnStatus1;
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
		PGS_SMF_GetMsgByCode(returnStatus1,msg);
	    break;
	  case PGSCSC_W_BAD_TRANSFORM_VALUE:
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code != returnStatus1)
		PGS_SMF_GetMsgByCode(returnStatus1,msg);
	    returnStatus = PGSCBP_W_BAD_CB_VECTOR;
	    break;
	  case PGSTD_E_BAD_INITIAL_TIME:
	  case PGSCBP_E_UNABLE_TO_OPEN_FILE:
	  case PGSCBP_E_TIME_OUT_OF_RANGE: 
	  case PGSCBP_E_INVALID_CB_ID:
	  case PGSMEM_E_NO_MEMORY:
	  case PGSEPH_E_BAD_EPHEM_FILE_HDR:
	  case PGSEPH_E_NO_SC_EPHEM_FILE:
	  case PGSEPH_E_NO_DATA_REQUESTED:
	    return returnStatus1;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}

	/* get the linear radius of the body */

	switch (cbID)
	{
	  case PGSd_MERCURY:
	    rLinCB = RMERCURY;
	    break;
	  case PGSd_VENUS:
	    rLinCB = RVENUS;
	    break;
	  case PGSd_MARS:
	    rLinCB = RMARS;
	    break;
	  case PGSd_JUPITER:
	    rLinCB = RJUPITER;
	    break;
	  case PGSd_SATURN:
	    rLinCB = RSATURN;
	    break;
	  case PGSd_URANUS:
	    rLinCB = RURANUS;
	    break;
	  case PGSd_NEPTUNE:
	    rLinCB = RNEPTUNE;
	    break;
	  case PGSd_PLUTO:
	    rLinCB = RPLUTO;
	    break;
	  case PGSd_MOON:
	    rLinCB = RMOON;
	    break;
	  case PGSd_SUN:
	    rLinCB = RSUN;
	    break;
	}  

    } /* end if cbID != STAR */ 

    /* else call PGS_CSC_ECItoSC() to transform the ECI coordinates of the star
       into spacecraft coordinates */

    else /* cbID == STAR */
    {	
	returnStatus1 = PGS_CSC_ECItoSC(spacecraftTag,numValues,asciiUTC,
					offsets,cb_vector,cb_SCvector);

	switch(returnStatus1)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_E_NO_LEAP_SECS:
            holdStatus = returnStatus1;
	    break;
	  case PGSCSC_W_BAD_TRANSFORM_VALUE:
            holdStatus = returnStatus1;
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code != returnStatus1)
		PGS_SMF_GetMsgByCode(returnStatus1,msg);
	    returnStatus =  PGSCBP_W_ERROR_IN_BODYINFOV;
	    break;
	  case PGSCSC_W_BELOW_SURFACE:
            holdStatus = returnStatus1;
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if(code != returnStatus1)
		PGS_SMF_GetMsgByCode(returnStatus1,msg);   
	    returnStatus =  PGSCBP_W_ERROR_IN_BODYINFOV;
	    break;
	  case PGSTD_E_TIME_VALUE_ERROR:
	  case PGSTD_E_TIME_FMT_ERROR:
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code != returnStatus)
		PGS_SMF_GetMsgByCode(returnStatus1,msg);
	    returnStatus = PGSTD_E_BAD_INITIAL_TIME;
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	    return returnStatus;
	  case PGSMEM_E_NO_MEMORY:
	  case PGSEPH_E_BAD_EPHEM_FILE_HDR:
	  case PGSEPH_E_NO_SC_EPHEM_FILE:
	  case PGSEPH_E_NO_DATA_REQUESTED:
	    return returnStatus1;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}

	/* set the angular radius of the celestial body to zero */
	
	rAngCB = 0.0;

    } /* end else CBID == STAR */
 
    /* allocate memory */

    returnStatus1 = PGS_MEM_Malloc((void**)&startMem,
				   (sizeof(PGSt_double)*3*numFOVperimVec)+
                                   (sizeof(PGSt_double)*3*maxValues)+
                                   (sizeof(PGSt_double)*3*maxValues)+
				   (sizeof(PGSt_double)*3*maxValues)+
				   (sizeof(PGSt_double)*3*numFOVperimVec)+
                                   (sizeof(PGSt_double)*3*numFOVperimVec)+
                                   (sizeof(PGSt_double)*numFOVperimVec)+
                                   (sizeof(PGSt_boolean)*numFOVperimVec));
  
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSMEM_E_NO_MEMORY:
	return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }
    
    ptrMem = startMem;
    
    /* assign perimFOV_vecCopy vectors */

    perimFOV_vecCopy = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*numFOVperimVec);
    
    /* assign scToEcenter vectors */

    scToEcenter = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*maxValues);

    /* assign extremeVec vectors */

    extremeVec  = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*maxValues);

    /* assign posECI vectors */

    posECI = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*maxValues);

    /* assign perimFOV_vecNorm vectors */

    perimFOV_vecNorm = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*numFOVperimVec);

    /* assign perimFOV_crossProduct vectors */

    perimFOV_crossProduct = (PGSt_double (*)[3]) (ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*3*numFOVperimVec);

    /* assign perimFOV_crossProductNorm vector */

    perimFOV_crossProductNorm = (PGSt_double *)(ptrMem);
    ptrMem = ptrMem + (sizeof(PGSt_double)*numFOVperimVec);

    /* assign perimFOV_crossProductNorm vector */

    checkArc = (PGSt_boolean *)(ptrMem);
    
    /* determine the spacecraft to Earth center vector for the specified times
     */
       
    for (counter = 0; counter < maxValues; counter++)
	for (cnt4 = 0; cnt4 < 3; cnt4++)
	    posECI[counter][cnt4] = 0.0;
    
    returnStatus1 = PGS_CSC_ECItoSC(spacecraftTag,numValues,asciiUTC,offsets,
                                    posECI,scToEcenter);
    
    switch(returnStatus1)
    {
      case PGSTD_E_NO_LEAP_SECS:
        holdStatus = returnStatus1;
      case PGS_S_SUCCESS:
      case PGSCSC_W_BELOW_SURFACE:
	break;
      case PGSCSC_W_BAD_TRANSFORM_VALUE:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if (code != returnStatus)
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
        returnStatus = PGSCBP_W_ERROR_IN_BODYINFOV;
	break;
      case PGSMEM_E_NO_MEMORY:
      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
      case PGSEPH_E_NO_SC_EPHEM_FILE:
      case PGSEPH_E_NO_DATA_REQUESTED:
        PGS_MEM_Free(startMem);
	return returnStatus1;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
        PGS_MEM_Free(startMem);
	return PGS_E_TOOLKIT;
    }
    
    /* for each time offset, determine if the celestial body is in the FOV */
    
    for (counter = 0; counter < maxValues; counter++)
    {
	
	/* initialize the inFOVflag to PGS_FALSE */
	
	inFOVflag[counter] = PGS_FALSE;

	/* if the cb_SCvector equals PGSd_GEO_ERROR_VALUE process the next time
           value else process this time value */

	if (cb_SCvector[counter][0] == PGSd_GEO_ERROR_VALUE)
	    continue;
        else
	{    
	    norm = PGS_CSC_Norm(cb_SCvector[counter]);
	    if (norm != 0.0)
	    {
		/* calculate the angular radius of the celestial body - in the
		   case of the MOON it is calculated for each time offset and
		   for other celestial bodies it is sufficient to use the same
		   angular radius as that for the first time offset */
	      
		if (cbID == PGSd_MOON)
		    rAngCB = asin(rLinCB / norm);
                else if ((counter == 0) && (cbID != PGSd_STAR))
		    rAngCB = asin(rLinCB / norm);

		/* normalize the cb_SCvector */
 
		for (cnt4 = 0; cnt4 < 3; cnt4++)
		    cb_SCunitVec[cnt4] = cb_SCvector[counter][cnt4] / norm;
	    }	    
	    else
	    {
	        returnStatus = PGSCBP_W_BAD_CB_VECTOR;
	        PGS_SMF_GetMsgByCode(returnStatus,msg);
	        for (cnt4 = 0; cnt4 < 3; cnt4++)
		    cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
                if(countMsg < 50)
		{
		    countMsg += 1;
		    sprintf(specifics,"%s %d",
			    "Zero cb_SCvector, offset #", (int) counter);
		    PGS_SMF_SetDynamicMsg(returnStatus,
							  specifics,
							  FUNCTION_NAME);
		}
		continue;
	    }  
	}
	
	/* copy the FOV perimeter vectors for the time offset into a two 
	   dimensional array */
	
	for (counter2 = 0; counter2 < numFOVperimVec; counter2++)
	{    
	    for (cnt4 = 0; cnt4 < 3; cnt4++)
	    {
		perimFOV_vecCopy[counter2][cnt4] = 
		  perimFOV_vectors[counter * numFOVperimVec * 3 + counter2 * 3 
				   + cnt4];
	    }
	}

	/* normalize the in field of view vector */
	
	norm = PGS_CSC_Norm(inFOVvector[counter]);
	if (norm != 0.0)
	    for (cnt4 = 0; cnt4 < 3; cnt4++)
		inFOVvecNorm[cnt4] = inFOVvector[counter][cnt4] / norm;
	else
	{
            returnStatus = PGSCSC_E_INVALID_FOV_DATA;
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
            for (cnt4 = 0; cnt4 < 3; cnt4++)
		cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
            if(countMsg < 50)
	    {
                countMsg += 1;
                sprintf(specifics,"%s %d",
                      "Bad inFOVvector offset #", (int) counter);
                PGS_SMF_SetDynamicMsg(returnStatus,specifics,
						      FUNCTION_NAME);
	    }
	    continue;
	} 

	/* call function PGS_CSC_FOVconicalHull() which draws a circular cone 
	   around the FOV and first checks if the CB point is inside 
	   it before going any further */

	returnStatus1 = PGS_CSC_FOVconicalHull(numFOVperimVec,inFOVvecNorm,
					       perimFOV_vecCopy,
					       rAngCB,cb_SCunitVec,
					       perimFOV_vecNorm,
					       &inFOVflag[counter],
					       &rMdotProduct);
	
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCSC_E_INVALID_FOV_DATA:
	  case PGSCSC_E_FOV_TOO_LARGE:
            holdStatus = returnStatus1;
	    if (returnStatus == PGS_S_SUCCESS)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		  PGS_SMF_GetMsgByCode(returnStatus1,msg);   
                returnStatus = PGSCBP_W_ERROR_IN_BODYINFOV;
                if(countMsg < 50)
		{
		    countMsg += 1;
		    sprintf(specifics,"%s %s %d",
			    "Bad FOV data passed to PGS_CSC_FOVconicalHull, ",
			    "offset #", (int) counter);
		    PGS_SMF_SetDynamicMsg(returnStatus,
							  specifics,
							  FUNCTION_NAME);
		}
	    }
	    for (cnt4 = 0; cnt4 < 3; cnt4++)
		cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
	    continue;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
	    PGS_MEM_Free(startMem);
	    for (cnt4 = 0; cnt4 < 3; cnt4++)
		cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
	    return PGS_E_TOOLKIT;
	}

	if (inFOVflag[counter] == PGS_FALSE)
        {
	    /* a decision has been made that CB is outside FOV */
	    saveStatus = PGS_TRUE;
	    continue;
        }
    
	/* call function PGS_CSC_EarthOccult() to test for Earth occultation 
	   of the celestial body */

	checkEarthBulge = PGS_FALSE;
	returnStatus1 = PGS_CSC_EarthOccult(inFOVvecNorm,rMdotProduct,rAngCB,
					    cb_SCunitVec,
					    scToEcenter[counter],
					    extremeVec[counter],
					    &inFOVflag[counter]); 
	
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:	    
	    break;
	  case PGSCSC_M_CHECK_EARTH_BULGE:
            holdStatus = returnStatus1;
	    checkEarthBulge = PGS_TRUE;
	    break;
	  case PGSCSC_M_EARTH_BLOCKS_CB: 
 	  case PGSCSC_M_EARTH_BLOCKS_FOV: 
            holdStatus = returnStatus1;
	    continue;  
	  case PGSCSC_W_DATA_FILE_MISSING:
	    PGS_MEM_Free(startMem);
	    return returnStatus1;
	  default:
	     PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
            /* note this is only for current counter value - could help
               identify place where Toolkit Error occurred */
             for (cnt4 = 0; cnt4 < 3; cnt4++)
	         cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
             PGS_MEM_Free(startMem);
	     return PGS_E_TOOLKIT;
	}
	
	/* call function PGS_CSC_PointInFOVgeom() to determine if the center 
	   of the CB is in the FOV at the specified times */

	returnStatus1 = PGS_CSC_PointInFOVgeom(numFOVperimVec,inFOVvecNorm,
					       perimFOV_vecNorm,
					       cb_SCunitVec,
					       &inFOVflag[counter]);
	
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSMEM_E_NO_MEMORY:
	    PGS_MEM_Free(startMem);
	    return returnStatus1;
          case  PGSCSC_E_INVALID_FOV_DATA:
            holdStatus = returnStatus1;
            PGS_SMF_GetMsgByCode(returnStatus1,msg);
            returnStatus = PGSCBP_W_ERROR_IN_BODYINFOV;
	    for (cnt4 = 0; cnt4 < 3; cnt4++)
		cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
	    if(countMsg < 50)
	    {
		countMsg += 1;
		sprintf(specifics,"%s %d",
			"Bad perimeter or inFOVvector, offset #",
			(int) counter);
		PGS_SMF_SetDynamicMsg(returnStatus, specifics,
						      FUNCTION_NAME);
	    }
	    continue;  
	  default:
	     PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
            /* note this is only for current counter value - could help
               identify place where Toolkit Error occurred */
	    for (cnt4 = 0; cnt4 < 3; cnt4++)
		cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
             PGS_MEM_Free(startMem);
	     return PGS_E_TOOLKIT;
	}
	
	cos_rAngCB = cos(rAngCB);
	
	/* if the celestial body center is not in the field of view check if 
	   any vertex of the perimeter vectors or the inFOVvector lies in the 
           diskCB - note that this test does not apply in the case of a star */
	
	if ((inFOVflag[counter] == PGS_FALSE) && (cbID != PGSd_STAR))

	{
            if(PGS_CSC_dotProduct(cb_SCunitVec,inFOVvecNorm,3)  > cos_rAngCB)
            {
                inFOVflag[counter] = PGS_TRUE;
                /* a decision has been made that CB is in FOV */
                saveStatus = PGS_TRUE;
            }

	    else 
                for (counter2 = 0; counter2 < numFOVperimVec; counter2++)
	        {
		    dotProduct = PGS_CSC_dotProduct(cb_SCunitVec,
						    perimFOV_vecNorm[counter2],
						    3);
		    if (dotProduct > cos_rAngCB)
		    {
		        inFOVflag[counter] = PGS_TRUE;
                        /* a decision has been made that CB is in FOV */
                        saveStatus = PGS_TRUE;
		        break;
		    }  
	        }
	}
	
	/* if the celestial body center is found to be in the FOV, or a vertex 
           or the inFOVvector lies inside the CB disk, determine if the 
           Earth's bulge (difference in radius over that of an inscribed sphere) 
           occults the CB - note that this test is only done if checkEarthBulge
           was set to PGS_TRUE  by PGS_CSC_EarthOccult() */
	
	if (inFOVflag[counter] == PGS_TRUE) 
	{      
	    if (checkEarthBulge == PGS_TRUE)
	    {
		/* call GetFOV_Pixel to determine if the line of sight of the 
		   extreme vector intersects the Earth - if so, the body is 
		   occulted */
	    
		if (numValues == 0 )
		  fovValues = 0;
		else
		{
		    timeOffset[0] = offsets[counter];
		    fovValues = 1;
		}
				
		returnStatus1 = PGS_CSC_GetFOV_Pixel(spacecraftTag,
						     fovValues,asciiUTC,
						     timeOffset,"WGS84",
						     PGS_FALSE,
						     (extremeVec+counter),NULL,
						     latitudeFOV,longitudeFOV,
						     pixelUnitECR,
						     slantRange,velocDoppl);
			
		if  (returnStatus1 == PGS_S_SUCCESS)
		{
                    inFOVflag[counter] = PGS_FALSE;	
		}
                else
                {
                    holdStatus = returnStatus1;
                }
	    } 
	    continue;
	}

	/* in the case where the angular radius of the body is zero then no
	   further processing can be done - continue with the next time 
	   offset */

	if (PGS_SMF_FEQ(rAngCB, 0.0))
	    continue;
    
	/* if the center or vertex of the celestial body is not in the field 
	   of view, then determine if it might intercept an arc bounding the
           FOV */

	invalidFOVdata = PGS_FALSE;
	cleanMiss = PGS_TRUE;
	
	for (counter2 = 0; counter2 < numFOVperimVec; counter2++)	  
	{
	        
            checkArc[counter2] = PGS_FALSE;
	    counter3 = (counter2 + 1) % numFOVperimVec;
		
	    /* find the cross product of two consecutive perimeter vectors */

	    PGS_CSC_crossProduct(perimFOV_vecCopy[counter2],
				 perimFOV_vecCopy[counter3],
				 perimFOV_crossProduct[counter2]);

	    /* find the norm of the cross product of two consecutive perimeter 
	       vectors */     

	    perimFOV_crossProductNorm[counter2] = 
	      PGS_CSC_Norm(perimFOV_crossProduct[counter2]);

	    /* if the norm of the cross product is zero then the field of view
	       data are invalid */

	    if (perimFOV_crossProductNorm[counter2] == 0.0)
	    {
		returnStatus = PGSCSC_E_INVALID_FOV_DATA;
                holdStatus = returnStatus;
                if(countMsg < 50)
		{
		    countMsg += 1;
		    sprintf(specifics, "%s%s %d", "Two identical or opposite ",
			    "perimeter vectors, offset #", (int) counter);
		    PGS_SMF_SetDynamicMsg(returnStatus,
							  specifics,
							  FUNCTION_NAME);
		}
		PGS_SMF_GetMsgByCode(returnStatus,msg);
		for (cnt4 = 0; cnt4 < 3; cnt4++)
		    cb_SCvector[counter][cnt4] = PGSd_GEO_ERROR_VALUE;
                invalidFOVdata = PGS_TRUE;
                break;
	    }
	
	    /* find the pole of the great circle through the two perimeter
	       vectors */		
	    /* determine if there is a clean miss; ie the CB does not meet an
	       arc or its prolongation*/
	    /* corrected the following line to match 
	       |acos(cp (*) eta - pi/2| < rAngCB
	       in Document 445-TP-002-002, Theoritical Basis of the 
	       SDP TOOLKIT Geolocation Package for the ECS project, 
	       May 1995 section 8.6.5.1
	    */
	    if (fabs(acos(PGS_CSC_dotProduct(perimFOV_crossProduct[counter2],
					       cb_SCunitVec,3) /
			    perimFOV_crossProductNorm[counter2])-
		  PIDIVTWO)
		<= (rAngCB))
	    {
		cleanMiss = PGS_FALSE;
                checkArc[counter2] = PGS_TRUE;
	    }
	}
	
	/* if the FOV data was invalid or there is a clean miss then process
	   the next time offset */

	if ((cleanMiss == PGS_TRUE) || (invalidFOVdata == PGS_TRUE)) 
	    continue;
	
	/* if none of the vertices lies in the diskCB, then check if the arc 
	   formed by two consecutive perimeter vectors is a chord of the 
	   diskCB */
	
	for (counter2 = 0; counter2 < numFOVperimVec; counter2++)
	{
            if(checkArc[counter2] == PGS_FALSE) continue;
	    counter3 = (counter2 + 1) % numFOVperimVec;
			
	    /* find the pole of the perpendicular from the center of the CB
	       to the pole of the great circle through two perimeter vectors */

	    PGS_CSC_crossProduct(perimFOV_crossProduct[counter2],
				 cb_SCunitVec,perp_pole);
		
	    /* find the intersections of the great circles */
		
	    PGS_CSC_crossProduct(perimFOV_crossProduct[counter2],perp_pole,xf);
                
	    /* find the vector diametrically opposite */

	    xfOpp[0] = -xf[0];
	    xfOpp[1] = -xf[1];
	    xfOpp[2] = -xf[2];
		
	    /* normalize the intersections */

	    norm = PGS_CSC_Norm(xf);

	    /* if norm equals zero, this indicates that the center of the
	       celestial body lies on the periphery - this condition is
	       unlikely because the test for the object's center being
	       in the FOV using PGS_CSC_PointInFOVgeom() is expected to
	       report positive, yet just in case this condition does occur due
	       to round off errors the inFOVflag will be set to PGS_TRUE */

	    if (PGS_SMF_FEQ(norm, 0.0))
	    {
		inFOVflag[counter] = PGS_TRUE;
                /* a decision has been made that CB is in FOV */
                saveStatus = PGS_TRUE;
		break;
	    }
	    else
	    {
		for (cnt4 = 0; cnt4 < 3 ; cnt4++)
		{
		    xf[cnt4]= xf[cnt4] / norm;	
		    xfOpp[cnt4]= -xf[cnt4];
		}
	    
		/* find the vector nearest to the center point of the CB */

		if (PGS_CSC_dotProduct(cb_SCunitVec,xf,3) >
		    PGS_CSC_dotProduct(cb_SCunitVec,xfOpp,3))
		    for (cnt4 = 0; cnt4 < 3 ;cnt4++)
			xfNear[cnt4] = xf[cnt4];
		else
		    for (cnt4 = 0; cnt4 < 3 ;cnt4++)
			xfNear[cnt4] = xfOpp[cnt4];
		
		/* find the vectors from xfNear to each of the perimeter 
		   vectors */ 

		for (cnt4 = 0; cnt4 < 3; cnt4++)
		  xfNearToPerim1[cnt4] = perimFOV_vecNorm[counter2][cnt4] - 
		    xfNear[cnt4];
		
		for (cnt4 = 0; cnt4 < 3; cnt4++)
		  xfNearToPerim2[cnt4] = perimFOV_vecNorm[counter3][cnt4] - 
		    xfNear[cnt4];

		/* if the dot product of the vectors from xfNear to each of 
		   the perimeter vectors is negative then the celestial body 
		   is in the field of view */
			    
		if (PGS_CSC_dotProduct(xfNearToPerim1,xfNearToPerim2,3) < 0.0)
		{   
		    inFOVflag[counter] = PGS_TRUE;
                    /* a decision has been made that CB is in FOV */
                    saveStatus = PGS_TRUE;
		    break;
		}
	    } /* END: else    (norm != 0.0) */	    
	} /* END: for(counter2 = 0; counter2 < numFOVperimVec ... */
	
	/* if the celestial body is found to be in the FOV,determine if the 
	   Earth's bulge (difference in radius over that of an inscribed 
	   sphere) occults the CB - note that this test is only done if 
	   checkEarthBulge equals PGS_TRUE*/
	
	if ((inFOVflag[counter] == PGS_TRUE) &&
	    (checkEarthBulge == PGS_TRUE))
	{
	    /* call GetFOV_Pixel to determine if the line of sight of the 
	       extreme vector intersects the Earth - if so, the body is 
	       occulted */
	    
	    if (numValues == 0 )
		fovValues = 0;
	    else
	    {
		timeOffset[0] = offsets[counter];
		fovValues = 1;
	    }
	    	    
	    returnStatus1 = PGS_CSC_GetFOV_Pixel(spacecraftTag,
						 fovValues,asciiUTC,
						 timeOffset,"WGS84",
						 PGS_FALSE,
						 (extremeVec+counter),NULL,
						 latitudeFOV,longitudeFOV,
						 pixelUnitECR,
						 slantRange,velocDoppl);
			
	    if (returnStatus1 == PGS_S_SUCCESS)
	    {
                inFOVflag[counter] = PGS_FALSE;	
	    }
            else
            {
                holdStatus = returnStatus1;
            }
	} /* END: if ((inFOVflag[counter] == PGS_TRUE) &&... */
    } /* END: for(counter=0;counter < ... */

    PGS_MEM_Free(startMem);
    
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME);
    else if (saveStatus == PGS_FALSE)
	/* No good offsets processed - return error and message */
    {
	switch (PGS_SMF_TestStatusLevel(returnStatus))
	{
	  case PGS_SMF_MASK_LEV_E:
	    break;
	  default: 
	    if(holdStatus != PGS_S_SUCCESS)
	    {
		returnStatus = holdStatus;
	    }
	}
	PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    }
    else
	switch (PGS_SMF_TestStatusLevel(returnStatus))
    	{
	    /* error for only some offsets - in message Logfile - return
	       warning */
	  case PGS_SMF_MASK_LEV_E:
	    returnStatus = PGSCBP_W_ERROR_IN_BODYINFOV; 
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	    break;
	  default: /* cases W, M ... * all ok */ 
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
    	}
    return returnStatus;
}
