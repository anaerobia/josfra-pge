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
!  PGS_EPH_UnInterpEphAttF.f
!
!DESCRIPTION:
!   This file contains the function PGS_EPH_UnInterpEphAtt()
!   This function gets uninterpolated actual ephemeris and/or attitude 
!   data for the specified spacecraft at a given time period.
!   This is an intermediary "helper" function.  Its only purpose in life
!   is to handle the case that the input variable "numvalues" is less than
!   one, since the cfortran.h FORTRAN to C binding technique does not
!   handle this situation gracefully.  Having checked the above mentioned
!   condition this function just calls pgs_eph_uninterpephAtt2 the underlying
!   fortran function(which declares two temporary arrays for proper fortran
!   binding). The function pgs_eph_uninterpephAtt2 calls the underlying C 
!   function (asappropriate).
!
!AUTHOR:
!  Abe Taaheri        /L3 Communication, EER Systems Inc.
!
!HISTORY:
!  01-Aug-2003  AT    Initial version
!
!END_FILE_PROLOG:
!******************************************************************************
!
!******************************************************************************
!BEGIN_PROLOG:
!
!TITLE:
!    Get uninterpolated Ephemeris and Attitude records
!
!NAME:
!   PGS_EPH_UnInterpEphAtt()
!
!SYNOPSIS:
!C:
!   N/A
!
!
!FORTRAN:
!      include 'PGS_SMF.f'
!      include 'PGS_TD.f'
!      include 'PGS_TD_3.f'
!      include 'PGS_EPH_5.f'
!      include 'PGS_MEM_7.f'
!
!      integer function pgs_eph_uninterpephAtt
!     >                                   (spacecrafttag,asciiutcstart,
!     >                                    asciiutcstop,orbflag,
!     >                                    attflag,qualityflags,
!     >                                    numvalueseph,numvaluesatt,
!     >                                    asciiutceph,asciiutcatt
!     > 				          positioneci,velocityeci,
!     >                                    eulerangles,xyzrotrates,
!     > 				          attitquat)
!      integer           spacecrafttag
!      integer           numvalueseph
!      integer           numvaluesatt
!      character*27      asciiutcstart
!      character*27      asciiutcstop
!      integer           orbflag
!      integer           attflag
!      character         asciiutcatt(27,*)
!      character         asciiutceph(27,*)
!      integer           qualityflags(2,*)
!      double precision  positioneci(3,*)
!      double precision  velocityeci(3,*)
!      double precision  eulerangles(3,*)
!      double precision  xyzrotrates(3,*)
!      double precision  attitquat(4,*)
!
!DESCRIPTION:
!   This tool gets uninterpo;ated ephemeris and/or attitude data for the 
!   specified spacecraft at the specified time period.
!
!INPUTS:
!   Name               Description               Units  Min        Max
!   ----               -----------               -----  ---        ---
!   spacecraftTag      spacecraft identifier     N/A
!
!
!   asciiUTC_start     UTC time reference start  ASCII  1961-01-01 see NOTES
!                      time in CCSDS ASCII time
!	              code A format
!
!   asciiUTC_stop      UTC time reference stop    ASCII  1961-01-01 see NOTES
!                      time in CCSDS ASCII time
!                      code A format
!
!   orbFlag            set to true to get        T/F
!                      ephemeris data
!
!   attFlag            set to true to get        T/F
!                      attitude data
!
!   numValuesEph      As input this is the max 
!                     number of values for EPH 
!                     data that is expecteted
!                     to be retrieved              N/A   
!
!   numValuesAtt      As input this is the max 
!                     number of values for ATT 
!                     data that is expecteted
!                     to be retrieved              N/A  
! 
!OUTPUTS:
!   Name              Description                Units        Min         Max
!   ----              -----------                -----        ---         ---
!   qualityFlags      quality flags for                 ** see NOTES **
!                     position and attitude
! 
!   asciiUTC_Att      UTC time reference        ASCII  1961-01-01 see NOTES
!                     Uninterpolated attitude
!                     time in CCSDS ASCII time
!                     code A format
!		     
!   asciiUTC_Eph      UTC time reference        ASCII  1961-01-01 see NOTES
!                     Uninterpolated ephemeris
!                     time in CCSDS ASCII time
!                     code A format
!		     
!   numValuesEph      As output this is 
!                     Actual num. of values 
!                     for EPH data retrieved      N/A   
!
!   numValuesAtt      As output this is 
!                     Actual num. of values 
!                     for ATT data retrieved      N/A   
!
!   positionECI       ECI position               meters
!
!   velocityECI       ECI velocity               meters/sec
!
!   eulerAngles       s/c attitude as a set of   radians
!                     Euler angles
!
!   xyzRotRates       angular rates about body   radians/sec
!                     x, y and z axes
!
!   attitQuat         spacecraft to ECI          N/A
!                     rotation quaternion
!          
!RETURNS:
!   PGS_S_SUCCESS               successful return 
!   PGSEPH_W_BAD_EPHEM_VALUE    one or more values could not be determined
!   PGSMEM_E_NO_MEMORY          unable to allocate required dynamic memory 
!                               space
!   PGSEPH_E_BAD_EPHEM_FILE_HDR no s/c ephemeris and/or attitude files had
!                               readable headers
!   PGSEPH_E_NO_SC_EPHEM_FILE   no s/c ephemeris and/or attitude files could
!                                be
!                               found for input times
!   PGSEPH_E_NO_DATA_REQUESTED  both orb and att flags are set to false
!   PGSTD_E_SC_TAG_UNKNOWN      unrecognized/unsupported spacecraft tag
!   PGSEPH_E_BAD_ARRAY_SIZE     array size specified is less than 0
!   PGSTD_E_TIME_FMT_ERROR      format error in asciiUTC
!   PGSTD_E_TIME_VALUE_ERROR    value error in asciiUTC
!   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
!                               initial
!                               time (asciiUTC)
!   PGS_E_TOOLKIT               an unexpected error occured
!   PGSTSF_E_GENERAL_FAILURE    problem in the thread-safe code
!
!
!EXAMPLES:
!C:
!    N/A
!
!FORTRAN:
!      implicit none
!
!      include  'PGS_SMF.f'
!      include  'PGS_TD.f'
!      include  'PGS_TD_3.f'
!      include  'PGS_EPH_5.f'
!      include  'PGS_MEM_7.f'
!
!      integer           pgs_eph_uninterpephAtt
!
!      integer           numvaluesatt/2000/
!      integer           numvalueseph/1000/
!      integer           returnstatus
!      integer           qualityflags(2,*)
!     
!      character*27      asciiutcstart 
!      character*27      asciiutcstop
!      
!      character         asciiutcatt(27,*)
!      character         asciiutceph(27,*)
!      double precision  positioneci(3,*)
!      double precision  velocityeci(3,*)
!      double precision  eulerangles(3,*)
!      double precision  xyzrotrates(3,*)
!      double precision  attitquat(4,*)
!
!      asciiutcstart = '1998-02-03T19:23:45.123'
!      asciiutcstop = '1998-02-03T20:23:45.123'
!
!      returnstatus =  pgs_eph_uninterpephAtt
!     >                         (pgsd_eos_am,asciiutcstart,asciiutcstop,
!     >                          pgs_true,pgs_true,qualityflags,
!     >                          numvalueseph,numvaluesatt,
!     >                          asciiutceph,asciiutcatt,positioneci,
!     >          		velocityeci,eulerangles,
!     >                          xyzrotrates,attitquat)
!
!      if (returnstatus .ne. pgs_s_success) then
!                :
!      *** do some error handling ***
!                :
!      endif
!
!NOTES:
!   QUALITY FLAGS:
!
!   The quality flags are returned as integer quantities but should be
!   interpreted as bit fields.  Only the first 32 bits of each quality flag is
!   meaningfully defined, any additional bits should be ignored (currently
!   integer quantities are 32 bits on most UNIX platforms, but this is not
!   guaranteed to be the case--e.g. an integer is 64 bits on a Cray).
!
!   Generally the quality flags are platform specific and are not defined by
!   the Toolkit.  Two bits of these flags have, however, been reserved for SDP
!   Toolkit usage.  Bit 12 will be set by the Toolkit if no data is available
!   at a requested time, bit 14 will be set by the Toolkit if the data at the
!   requested time has been interpolated (the least significant bit is "bit
!   0").  Any other bits are platform specific and are the responsibility of
!   the user to interpret.
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
!     called with a time before that date.  The file, which is updated when a 
!     new leap second event is announced, contains dates of leap second events.
!     If an input date is outside of the range of dates in the file (or if the 
!     file cannot be read) the function PGS_TD_LeapSec() which reads the file
!     will return a calculated value of TAI-UTC based on a linear fit of the 
!     data in the table as of late 1997.  This value of TAI-UTC is relatively 
!     crude estimate and may be off by as much as 1.0 or more seconds. The
!     warning return status from PGS_TD_LeapSec() is promoted to an error 
!     level by any Toolkit function receiving it, which will terminate the 
!     present function.
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
!     Toolkit internal time is the real number of continuous SI seconds since 
!     the epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred 
!     to in the toolkit as TAI (upon which it is based).
!
!
!REQUIREMENTS:
!   PGSTK - 0141, 0720, 0740
!
!DETAILS:
!   None
!
!GLOBALS:
!
!FILES:
!   The function requires the presence of the files:
!      leapsec.dat
!      earthfigure.dat
!      utcpole.dat
!   This function also requires ephemeris and attitude data files.
!
!FUNCTIONS_CALLED:
!   PGS_SMF_SetStaticMsg()        sets the message buffer
!   PGS_SMF_SetDynamicMsg()       sets the message buffer
!   PGS_EPH_UnInterpEphAtt() 
!   PGS_SMF_TestStatusLevel()
!
!
!END_PROLOG:
!******************************************************************************


      integer function pgs_eph_uninterpephAtt
     >                                   (spacecrafttag,asciiutcstart,
     >                                    asciiutcstop,orbflag,
     >                                    attflag,qualityflags,
     >                                    numvalueseph,numvaluesatt,
     >                                    asciiutceph,asciiutcatt,
     > 				          positioneci,velocityeci,
     >                                    eulerangles,xyzrotrates,
     > 				          attitquat)
      integer           spacecrafttag
      integer           numvalueseph
      integer           numvaluesatt
      character*(*)     asciiutcstart
      character*(*)     asciiutcstop
      integer           orbflag
      integer           attflag
      character*(*)     asciiutcatt(*)
      character*(*)     asciiutceph(*)
      integer           qualityflags(2,*)
      double precision  positioneci(3,*)
      double precision  velocityeci(3,*)
      double precision  eulerangles(3,*)
      double precision  xyzrotrates(3,*)
      double precision  attitquat(4,*)

      include 'PGS_SMF.f'
      include 'PGS_TD.f'
      include 'PGS_TD_3.f'
      include 'PGS_EPH_5.f'
      include 'PGS_MEM_7.f'

      integer          pgs_smf_setstaticmsg
      integer          pgs_eph_uninterpephAtt2

      integer          tmpvalueseph
      integer          tmpvaluesatt
      integer          returnstatus

      if (numvalueseph .lt. 0) then
         returnstatus = pgs_smf_setstaticmsg(pgseph_e_bad_array_size,
     >                                     'PGS_EPH_UnInterpEphAtt()') 
         pgs_eph_uninterpephAtt = pgseph_e_bad_array_size
         return
      else if (numvalueseph .eq. 0) then
         tmpvalueseph = 1
      else
         tmpvalueseph = numvalueseph
      endif
      if (numvaluesatt .lt. 0) then
         returnstatus = pgs_smf_setstaticmsg(pgseph_e_bad_array_size,
     >                                     'PGS_EPH_UnInterpEphAtt()') 
         pgs_eph_uninterpephAtt = pgseph_e_bad_array_size
         return
      else if (numvaluesatt .eq. 0) then
         tmpvaluesatt = 1
      else
         tmpvaluesatt = numvaluesatt
      endif
      pgs_eph_uninterpephatt = pgs_eph_uninterpephatt2
     >                                   (spacecrafttag,asciiutcstart,
     >                                    asciiutcstop,orbflag,
     >                                    attflag,qualityflags,
     >                                    tmpvalueseph,tmpvaluesatt,
     >                                    asciiutceph,asciiutcatt,
     > 				          positioneci,velocityeci,
     >                                    eulerangles,xyzrotrates,
     > 				          attitquat)
      numvalueseph = tmpvalueseph
      numvaluesatt = tmpvaluesatt
      return

      end
      
      
! This function is used to declare temporary arrays of orbit
! descent/ascent times.  The purpose of this is that the exact size of
! these arrays is known and is properly captured by the underlying C
! function.  Upon return the values in the temporary arrays are assigned
! to the actual user input arrays based on the actual number of records
! in the input time period.  Note that the value of for temporary arrays
! the size assigned is BLOCK_SIZE = 15000. If actual number of records
! in the input time period is greater than 
! BLOCK_SIZE = 15000 (defined in PGS_EPH_EphAtt_unInterpolate.c), 
! error will be returned.

      integer function pgs_eph_uninterpephAtt2
     >                                   (spacecrafttag,asciiutcstart,
     >                                    asciiutcstop,orbflag,
     >                                    attflag,qualityflags,
     >                                    numvalueseph,numvaluesatt,
     >                                    asciiutceph,asciiutcatt,
     > 				          positioneci,velocityeci,
     >                                    eulerangles,xyzrotrates,
     > 				          attitquat)
      implicit none
      integer           i
      integer           spacecrafttag
      integer           numvalueseph
      integer           numvaluesatt
      integer           numvaluesephatt(2)
      character*(*)     asciiutcstart
      character*(*)     asciiutcstop
      character*(*)     asciiutcatt(*)
      character*(*)     asciiutceph(*)
      integer           orbflag
      integer           attflag
      integer           orbattflags(2)
      integer           maxnumrecs
      parameter(maxnumrecs = 15000) !BLOCK_SIZE = 15000
      character*27     asciiutcatttmp(maxnumrecs)
      character*27     asciiutcephtmp(maxnumrecs)
      integer           qualityflags(2,*)
      double precision  positioneci(3,*)
      double precision  velocityeci(3,*)
      double precision  eulerangles(3,*)
      double precision  xyzrotrates(3,*)
      double precision  attitquat(4,*)

      include 'PGS_SMF.f'
      include 'PGS_TD.f'
      include 'PGS_TD_3.f'
      include 'PGS_EPH_5.f'
      include 'PGS_MEM_7.f'

      integer          pgs_smf_setstaticmsg
      integer          pgs_eph_uninterpephAttc
      integer          pgs_smf_setdynamicmsg

      integer          tmpvalueseph
      integer          tmpvaluesatt
      integer          returnstatus

      orbattflags(1) = orbflag
      orbattflags(2) = attflag
! initialize numvaluesephatt
      numvaluesephatt(1) = numvalueseph
      numvaluesephatt(2) = numvaluesatt

      pgs_eph_uninterpephatt2 = pgs_eph_uninterpephattc
     >                              (spacecrafttag,asciiutcstart,
     >                               asciiutcstop,orbattflags,
     >                               qualityflags,
     >                               numvaluesephatt,
     >                               maxnumrecs,
     >                               asciiutcephtmp,asciiutcatttmp,
     > 		                     positioneci,velocityeci,
     >                               eulerangles,xyzrotrates,
     > 		         	     attitquat)

      numvalueseph = numvaluesephatt(1)
      numvaluesatt = numvaluesephatt(2)

      if (numvalueseph .gt. maxnumrecs) then
         returnstatus = pgs_smf_setdynamicmsg(pgseph_e_bad_array_size,
     >'actual eph numvalues larger than maxnumrecs = 15000 ',
     >                                    'pgs_eph_uninterpephatt2()') 
         pgs_eph_uninterpephatt2 = pgseph_e_bad_array_size
         return
      else 
         do 100 i=1,numvalueseph
            asciiutceph(i) = asciiutcephtmp(i)
 100     continue
      endif

      if (numvaluesatt .gt. maxnumrecs) then
         returnstatus = pgs_smf_setdynamicmsg(pgseph_e_bad_array_size,
     >'actual att numvalues larger than maxnumrecs = 15000 ',
     >                                    'pgs_eph_uninterpephatt2()') 
         pgs_eph_uninterpephatt2 = pgseph_e_bad_array_size
         return
      else 
         do 200 i=1,numvaluesatt
            asciiutcatt(i) = asciiutcatttmp(i)
 200     continue
      endif

      return

      end


