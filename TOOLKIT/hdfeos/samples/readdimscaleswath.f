!/*
! * In this example we will (1) open the "SwathFile" HDF file, (2) attach to
! * the "swid" swath, and (3) set dimension scale for a dimension in a filed.
! */

	program readdimscaleswath
	
	integer       j, status, swdetach, swclose
	integer*4     swfid, swid, swopen, swattach
	integer*4     swdiminfo
	integer*4     swgetdimscale, swgetdimstrs
	real*8        BuffGeoTrack(20)
	integer*4     dimsize
	integer*4     dimsize1
 	integer*4     nbands
	integer*4     bands(15)
	integer*4     ntype(32)
	character*16  label, unit, format
	integer*4     len
	integer DFACC_RDWR
	parameter (DFACC_RDWR=3)
	integer DFNT_FLOAT32
	parameter (DFNT_FLOAT32=5)
	integer DFNT_FLOAT64
	parameter (DFNT_FLOAT64=6)
	integer DFNT_INT32
	parameter (DFNT_INT32=24)

!    /*
!     * We first open the HDF swath file, "SwathFile.hdf".  Because this file
!     * already exist and we wish to write to it, we use the DFACC_RDWR access
!     * code in the open statement.  The swopen routine returns the swath file
!     * id, swfid, which is used to identify the file in subsequent routines.
!     */

	swfid = swopen("SwathFile_created_with_hadeos_sample_file_write"//
     1"r_of_HDFEOS2_version_219_or_higher_release.hdf", DFACC_RDWR)

!    /*
!     * If the swath file cannot be found, swopen will return -1 for the file
!     * handle (swfid).  We there check that this is not the case before
!     * proceeding with the other routines.
!     * 
!     * The swattach routine returns the handle to the existing swath "swid",
!     * swid.  If the swath is not found, swattach returns -1 for the handle.
!     */

	if (swfid .ne. -1) then
	   
	   swid = swattach(swfid, "Swath1")
	   
	   if (swid .ne. -1) then
	      
	      dimsize = swdiminfo(swid, "GeoTrack")
	      
	      write(*,*) 'GeoTrack dim size (Swath1): ', dimsize
	      
	      buffsize = swgetdimscale(swid, "Temperature", "GeoTrack", 
     1	   dimsize1, ntype,BuffGeoTrack)
	      if (buffsize .eq. -1) then	   
		 write(*,*) 'Error:Cannot get Dim Scale for',
     1	      ' GeoTrack in field Density'
		 stop  
	      else
		 write(*,*) 'For Density field GeoTrack: buffsize = ',
     1	      buffsize
	      write(*,*) 'GeoTrack dimsize1 (Swath1): ', dimsize1
	      endif
	      
	      write(*,*) 'For Density field GeoTrack',
     1                   ' dimension scale values:'
	      do j=1,dimsize
		 write(*,*) 'j =',j,'  BuffGeoTrack = ',BuffGeoTrack(j)
	      enddo
	      write(*,*) '  '
	      
	      len = 15
	      
	      status = swgetdimstrs(swid, "Temperature", "GeoTrack", 
     1	   label, unit, format, len)
	      
	      
	      if(status .eq. 0) then	  
	       write(*,*) 'For Density field GeoTrack dimension scale',
     1                      ' attr. strings:'
	       write(*,*) 'label =',label,
     1	      '  unit =',unit,'  format = ',format
	      endif
	      write(*,*) ' '
	      
				!         nbands = 15
	      
	      dimsize = swdiminfo(swid, "Bands");
	      status = swgetdimscale(swid, "Spectra", "Bands", 
     1	   nbands, ntype, bands)
	      write(*,*) 'For nbands for Spectra field = ',nbands
	      write(*,*) 'For Spectra field Bands dim scale values:'
	      do j=1,nbands
		 write(*,*) 'j =',j,'         Bnads = ',bands(j)
	      enddo
	      write(*,*) '  '
	      
	      status = swgetdimstrs(swid, "Spectra", "Bands", 
     1	   label, unit, format, len)
	      
	      if(status .eq. 0) then
		 write(*,*) 'For Spectra field Bands dim scale',
     1		 ' attr. strings:'
		 write(*,*) 'label =',label,'  unit =',
     1	      unit,'  format = ',format
	      endif
	      status = swdetach(swid)
	   endif
	   status = swclose(swfid)
	endif 
	stop
	end
	
