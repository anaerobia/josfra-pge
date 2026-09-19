	
!/*
! * In this example we will (1) open the "SwathFile" HDF file, (2) attach to
! * the "swid" swath, and (3) set dimension scale for a dimension in a filed.
! */

	program writedimscaleswath

	integer       j, status, swdetach, swclose
	integer*4     swfid, swid, swopen, swattach
	integer*4     swsetdimscale, swsetdimstrs
	real*8        BuffGeoTrack(20)
	real*4        BuffGeoTrack2(10)
	integer*4     GTdim,GTdim1,GTdim2,GTdim3
	integer*4     bands(15)
	integer*4     IBuffGeoTrack(40)
	integer*4     IBuffGeoTrack2(20)
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
	      write(*,*) '=================================='
	if (swfid .ne. -1) then

!	/* the field Density has GeoTrack dimensions.
!	   the field Spectra has Res2xtr,Res2tr,Bands dimensions.
!	   see setupswath.f and defineflds.f
!	   nbands = 15;
!	   GeoTrack = 20;
!	*/
	  swid = swattach(swfid, "Swath1")
	  
	  if (swid .ne. -1) then
	    GTdim = 20
	    GTdim1 = 40
	    GTdim2 = 20
	    GTdim3 = 10
	    
	    do j=1,GTdim1
	       IBuffGeoTrack(j) = j
	    enddo
	    do j=1,GTdim
	       BuffGeoTrack(j) = j*2.5
	    enddo
	    status = swsetdimscale(swid,"Pressure","Res2tr",GTdim1,
     1	    DFNT_INT32, IBuffGeoTrack)
	    write(*,*) 'status swsetdimscale:Pressure:Res2tr=',status
	    status = swsetdimstrs(swid, "Pressure", "Res2tr", 
     1	    "Res2tr", "degrees", "I2")
	     
	     
	    status = swsetdimscale(swid,"Pressure","Res2xtr",GTdim2,
     1	    DFNT_INT32, IBuffGeoTrack2)
	    write(*,*) 'status swsetdimscale:Pressure:Res2xtr=',status

	    status = swsetdimscale(swid,"Density","GeoTrack",GTdim,
     1	    DFNT_FLOAT64, BuffGeoTrack)
	    write(*,*) 'status swsetdimscale:Density=',status,
     1      '    Error expected. Field is 1-dimensional'
	    
	    status = swsetdimscale(swid,"Temperature","GeoTrack",GTdim,
     1	    DFNT_FLOAT64, BuffGeoTrack)
	    write(*,*) 'status swsetdimscale:Temperature:GeoTrack=',
     1	    status
	    status = swsetdimstrs(swid, "Temperature", "GeoTrack", 
     1	    "GeoTrack", "degrees", "F7.2")
	    write(*,*) 'status swsetdimstrs:Temperature:GeoTrack=',
     1      status
	    
	    status = swsetdimscale(swid, "Temperature", "GeoXtrack",
     1	    GTdim3,DFNT_FLAOT32, BuffGeoTrack2)
	    write(*,*) 'status swsetdimscale:Temperature:GeoXtrack=',
     1	    status
	    
	    nbands = 15;
	    do j=1,nbands
	       bands(j) = j*2+1
	    enddo
	    status = swsetdimscale(swid, "Spectra", "Bands", nbands, 
     1	    DFNT_INT32, bands);
	    write(*,*) 'status swsetdimscale:Spectra =',status
	    
	    status = swsetdimstrs(swid, "Spectra", "Bands", "BANDS", 
     1	    "None", "I2")
	    
	    status = swdetach(swid)
	 endif
	 status = swclose(swfid)
	endif
	
	stop
	end
	
