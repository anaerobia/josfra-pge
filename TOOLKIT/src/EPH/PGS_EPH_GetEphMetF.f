!-------------------------------------------------------------------------!
!                                                                         !
!  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  !
!  and suppliers.  ALL RIGHTS RESERVED.                                   !
!                                                                         !
!-------------------------------------------------------------------------!
!******************************************************************************
!BEGIN_FILE_PROLOG:
!
!FILENAME:
!  PGS_EPH_GetEphMetF.f
!
!DESCRIPTION:
!   This file contains the function PGS_EPH_GetEphMet()
!   This function returns the metadata associated with toolkit spacecraft
!   ephemeris files.
!   This is an intermediary "helper" function.  Its only purpose in life
!   is to handle the case that the input variable "numvalues" is less than
!   one, since the cfortran.h FORTRAN to C binding technique does not
!   handle this situation gracefully.  Having checked the above mentioned
!   condition this function just calls pgs_eph_getephmet2 the underlying
!   fortran function(which declares two temporary arrays for proper fortran
!   binding). The function pgs_eph_getephmet2 calls the underlying C function 
!   (asappropriate).
!
!AUTHOR:
!  Guru Tej S. Khalsa / Applied Research Corporation
!  Abe Taaheri        /Space Applications Corporation
!
!HISTORY:
!  01-Jul-1996  GTSK  Initial version
!  28-Sep-1998  AT    Added Prolog
!                     Added function pgs_eph_getephmet2 and modified
!                     pgs_eph_getephmet for proper FORTRAN binding
!
!END_FILE_PROLOG:
!******************************************************************************
!
!******************************************************************************
!BEGIN_PROLOG:
!
!TITLE:
!   Get Spacecraft Ephemeris Metadata
!
!NAME:
!   PGS_EPH_GetEphMet()
!
!SYNOPSIS:
!C:
!   N/A
!
!
!FORTRAN:
!      include 'PGS_EPH_5.f'
!      include 'PGS_TD_3.f'
!      include 'PGS_SMF.f'
!
!      integer function pgs_eph_getephmet(spacecrafttag,numvalues,
!     >                                   asciiutc,offsets,numorbits,
!     >                                   orbitnumber,orbitascendtime,
!     >                                   orbitdescendtime,
!     >                                   orbitdownlongitude)
!
!      integer          spacecrafttag
!      integer          numvalues
!      character*27     asciiutc
!      double precision offsets(*)
!      integer          numorbits
!      integer          orbitnumber(*)
!      character*27     orbitascendtime(*)
!      character*27     orbitdescendtime(*)
!      double precision orbitdownlongitude(*)
!
!DESCRIPTION:
!   This function returns the metadata associated with toolkit spacecraft
!   ephemeris files.
!
!INPUTS:
!   Name              Description               Units   Min         Max
!   ----              -----------               -----   ---         ---
!   spacecraftTag     spacecraft identifier     N/A
!
!   numValues         num. of values requested  N/A
!
!   asciiUTC          UTC time reference start  ASCII   1961-01-01 see NOTES
!                     time in CCSDS ASCII time
!	             code A format
!
!   offsets           array of time offsets in  sec     **depends on asciiUTC**
!                     seconds relative to
!	             asciiUTC
!
!OUTPUTS:
!   Name               Description              Units   Min         Max
!   ----               -----------              -----   ---         ---
!   numOrbits          number of orbits         N/A
!                      spanned by data set
!
!   orbitNumber        array of orbit numbers   N/A
!                      spanned by data set
!
!   orbitAscendTime    array of times of        ASCII
!                      spacecraft northward
!                      equator crossings
!
!   orbitDescendTime   array of times of        ASCII
!                      spacecraft southward
!                      equator crossings
!
!   orbitDownLongitude array of longitudes      radians
!                      or spacecraft
!		      southward equator
!		      crossings
!
!RETURNS:
!   PGS_S_SUCCESS               successful return
!   PGSTD_E_SC_TAG_UNKNOWN      unknown/unsupported spacecraft tag
!   PGSEPH_E_NO_SC_EPHEM_FILE   no s/c ephem files could be found for input 
!                               times
!   PGSEPH_E_BAD_ARRAY_SIZE     array size specified is less than 0
!   PGSTD_E_TIME_FMT_ERROR      format error in asciiUTC
!   PGSTD_E_TIME_VALUE_ERROR    value error in asciiUTC
!   PGS_E_TOOLKIT               an unexpected error occurred
!
!EXAMPLES:
!C:
!    N/A
!
!FORTRAN:
!      implicit none
!
!      include 'PGS_EPH_5.f'
!      include 'PGS_TD.f'
!      include 'PGS_TD_3.f'
!      include 'PGS_SMF.f'
!
!      integer orbit_array_size/5/   ! maximum number of orbits expected
!      integer ephem_array_size/100/ ! number of ephemeris data points
!
!      double precision offsets(ephem_array_size)
!      double precision orbitdownlongitude(orbit_array_size)
!
!      integer          numorbits
!
!      character*27     asciiutc
!      character*27     orbitascendtime(orbit_array_size)
!      character*27     orbitdescendtime(orbit_array_size)
!
!!    initialize asciiutc and offsets array with the times for actual
!!    ephemeris records that will be processed (i.e. by some other tool)
!
!      asciiutc = '1998-02-03t19:23:45.123'
!      do 100 i=1,ephem_array_size
!          offsets(i) = i*60.D0
! 100  continue
!
!!    get the ephemeris metadata associated with these times
!
!      returnStatus = pgs_eph_getephmet(pgsd_eos_am,ephem_array_size,
!     >                                 asciiutc,offsets,numorbits,
!     >			               orbitascendtime,orbitdescendtime,
!     >  			       orbitdownlongitude)
!
!      if (returnStatus .ne. pgs_s_success) then
!                  :
!       ** do some error handling ***
!                  :
!      endif
!
!    numOrbits will now contain the number of orbits spanned by the data set
!    (as defined by asciiUTC and EPHEM_ARRAY_SIZE offsets). orbitAscendTime
!    will contain numOrbits ASCII UTC times representing the time of northward
!    equator crossing of the spacecraft for each respective orbit.
!    orbitDescendTime will similarly contain the southward equator crossing
!    times and orbitDownLongitude will contain the southward equator crossing
!    longitudes
!
!NOTES:
!   This function will determine the time span of the data set from the 
!   input TC reference time and the offsets array.  It will then determine 
!   the metadata for all orbits spanned by the data set.
!
!   TIME ACRONYMS:
!
!     TAI is:  International Atomic Time
!     UTC is:  Coordinated Universal Time
!
!   TIME BOUNDARIES:
!
!     The minimum and maximum times that can successfully be processed by this
!     function depend on the file leapsec.dat which relates leap second 
!     (TAI-UTC)
!     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
!     therefore an error status message will be returned if this function is
!     called with a time before that date.  The file, which is updated when 
!     a new leap second event is announced, contains actual (definitive) and 
!     predicted (long term; very approximate) dates of leap second events.  
!     The latter can be used only in simulations.   If an input date is 
!     outside of the range of
!     dates in the file (or if the file cannot be read) the function will use
!     a calculated value of TAI-UTC based on a linear fit of the data known 
!     to be in the table.  This value of TAI-UTC is relatively crude estimate 
!     and may be off by as much as 1.0 or more seconds.  Thus, when the 
!     function is used for dates in the future of the date and time of 
!     invocation, the user ought
!     to carefully check the return status.  The status level of the return
!     status of this function will be 'W' (at least) if the TAI-UTC value used
!     is a predicted value.  The status level will be 'E' if the TAI-UTC value
!     is calculated (although processing will continue in this case, using the
!     calculated value of TAI-UTC).
!
!   ASCII UTC:
!
!     Toolkit functions that accept an ASCII time as input require the time to
!     be UTC time in CCSDS ASCII Time Code A or CCSDS ASCII Time Code B format.
!     Toolkit functions that return an ASCII time return the UTC time in CCSDS
!     ASCII Time Code A format (see CCSDS 301.0-B-2 for details).
!     (CCSDS => Consultative Committee for Space Data Systems)
!
!      The general format is:
!
!          YYYY-MM-DDThh:mm:ss.ddddddZ (Time Code A)
!          YYYY-DDDThh:mm:ss.ddddddZ   (Time Code B)
!
!          where:
!              -,T,: are field delimiters
!              Z is the (optional) time code terminator
!              other fields are numbers as implied:
!                Time Code A:
!                   4 Year digits, 2 Month, 2 Day, 2 hour,
!                   2 minute, 2 second, and up to 6 decimal
!                   digits representing a fractional second
!                Time Code B:
!                   4 Year digits, 3 Day of year, 2 hour,
!                   2 minute, 2 second, and up to 6 decimal
!                   digits representing a fractional second
!
!   TOOLKIT INTERNAL TIME (TAI):
!
!     Toolkit internal time is the real number of continuous SI seconds 
!     since the epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also 
!     referred to in the toolkit as TAI (upon which it is based).
!
!   TIME OFFSETS:
!
!      This function accepts an ASCII UTC time, an array of time offsets and 
!      the number of offsets as input.  Each element in the offset array is 
!      an offset in seconds relative to the initial input ASCII UTC time. An
!      error will be returned if the number of offsets specified is less than
!      zero.  If the number of offsets specified is actually zero, the offsets
!      array will be ignored.  In this case the input ASCII UTC time will be
!      converted to Toolkit internal time (TAI) and this time will be used to
!      process the data.  If the number of offsets specified is one (1) or
!      greater, the input ASCII UTC time will converted to TAI and each element
!      'i' of the input data will be processed at the time: (initial time) +
!      (offset[i]).
!
!      Examples:
!
!       if numValues is 0 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
!       then input[0] will be processed at time 432000.0 and return output[0]
!
!	if numValues is 1 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
!       then input[0] will be processed at time 432000.0 + offsets[0] and
!       return output[0]
!
!       if numValues is N and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
!       then each input[i] will be processed at time 43000.0 + offsets[i] and
!       the result will be output[i], where i is on the interval [0,N)
!
!REQUIREMENTS:
!   PGSTK - 0720
!
!DETAILS:
!   None
!
!GLOBALS:
!   None
!
!FILES:
!   None
!
!FUNCTIONS_CALLED:
!   pgs_eph_getephmet2
!   pgs_smf_setstaticmsg
!
!END_PROLOG:
!******************************************************************************

      integer function pgs_eph_getephmet(spacecrafttag, numvalues,
     >                                   asciiutc, offsets, numorbits,
     >                                   orbitnumber, orbitascendtime,
     >                                   orbitdescendtime,
     >                                   orbitdownlongitude)

      implicit none

      integer          spacecrafttag
      integer          numvalues
      character*(*)    asciiutc
      double precision offsets(*)
      integer          numorbits
      integer          orbitnumber(*)
      character*(*)    orbitascendtime(*)
      character*(*)    orbitdescendtime(*)
      double precision orbitdownlongitude(*)

      include 'PGS_EPH_5.f'

      integer          pgs_smf_setstaticmsg
      integer          pgs_eph_getephmet2

      integer          tmpvalues
      integer          returnstatus

      if (numvalues .lt. 0) then
         returnstatus = pgs_smf_setstaticmsg(pgseph_e_bad_array_size,
     >                                       'PGS_EPH_GetEphMet()') 
         pgs_eph_getephmet = pgseph_e_bad_array_size
         return
      else if (numvalues .eq. 0) then
         tmpvalues = 1
      else
         tmpvalues = numvalues
      endif

      pgs_eph_getephmet = pgs_eph_getephmet2(spacecrafttag, tmpvalues,
     >                                   asciiutc, offsets, numorbits,
     >                                   orbitnumber, orbitascendtime,
     >                                   orbitdescendtime,
     >                                   orbitdownlongitude)

      return

      end
      
      
! This function is used to declare temporary arrays of orbit
! descent/ascent times.  The purpose of this is that the exact size of
! these arrays is known and is properly captured by the underlying C
! function.  Upon return the values in the temporary arrays are assigned
! to the actual user input arrays based on the actual number of orbits
! spanned by the input times.  Note that the value of for temporary arrays
! the size assigned is PGSd_EPH_MAX_ORBIT_ARRAY_SIZE. If actual number of 
! orbits spanned by the input times is greater than 
! PGSd_EPH_MAX_ORBIT_ARRAY_SIZE, error will be returned.

      integer function pgs_eph_getephmet2(spacecrafttag, numvalues,
     >                                    asciiutc, offsets, numorbits,
     >                                    orbitnumber, orbitascendtime,
     >                                    orbitdescendtime,
     >                                    orbitdownlongitude)

      implicit none
      integer          i
      integer          spacecrafttag
      integer          numvalues
      character*(*)    asciiutc
      double precision offsets(*)
      integer          numorbits
      integer          orbitnumber(*)
      character*(*)    orbitascendtime(*)
      character*(*)    orbitdescendtime(*)
      double precision orbitdownlongitude(*)
      include 'PGS_EPH_5.f'
      
      integer          returnstatus
      integer          maxnumorbits
      parameter(maxnumorbits = 1024)
      character*27     oat(maxnumorbits)   ! temporary orbit ascend time
      character*27     odt(maxnumorbits)   ! temporary orbit descend time
      
      integer          pgs_eph_getephmetc
      integer          pgs_smf_setdynamicmsg
      
      pgs_eph_getephmet2 = pgs_eph_getephmetc(spacecrafttag, numvalues,
     >                                        maxnumorbits,
     >                                        asciiutc, offsets,
     >                                        numorbits, orbitnumber,
     >                                        oat, odt,
     >                                        orbitdownlongitude)
      

      if (numorbits .gt. maxnumorbits) then
         returnstatus = pgs_smf_setdynamicmsg(pgseph_e_bad_array_size,
     >'actual numorbits larger than PGSd_EPH_MAX_ORBIT_ARRAY_SIZE in',
     >                                       'pgs_eph_getephmet2()') 
         pgs_eph_getephmet2 = pgseph_e_bad_array_size
         return
      else 
         do 100 i=1,numorbits
            orbitascendtime(i) = oat(i)
            orbitdescendtime(i) = odt(i)
 100     continue
      endif
      return

      end


