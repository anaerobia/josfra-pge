!/*
! * In this example we will (1) open the "GridFile" HDF file, (2) attach to
! * the "gdid" grid, and (3) set dimension scale for a dimension in a filed.
! */

	program writedimscalegrid

	integer       j, status, gddetach, gdclose
	integer*4     gdfid, gdid, gdopen, gdattach
	integer*4     gdsetdimscale, gdsetdimstrs
	real*8        veg1(120),veg2(200)
	real*8        temp1(100),temp2(100)
	integer*4     bands(3)
	integer*4     xdim,ydim,nbands

	integer DFACC_RDWR
	parameter (DFACC_RDWR=3)
	integer DFNT_FLOAT32
	parameter (DFNT_FLOAT32=5)
	integer DFNT_FLOAT64
	parameter (DFNT_FLOAT64=6)
	integer DFNT_INT32
	parameter (DFNT_INT32=24)

!    /*
!     * We first open the HDF grid file, "GridFile.hdf".  Because this file
!     * already exist and we wish to write to it, we use the DFACC_RDWR access
!     * code in the open statement.  The gdopen routine returns the grid file
!     * id, gdfid, which is used to identify the file in subsequent routines.
!     */

	gdfid = gdopen("GridFile_created_with_hadeos_sample_file_write"//
     1"r_of_HDFEOS2_version_219_or_higher_release.hdf", DFACC_RDWR)

!    /*
!     * If the grid file cannot be found, gdopen will return -1 for the file
!     * handle (gdfid).  We there check that this is not the case before
!     * proceeding with the other routines.
!     * 
!     * The gdattach routine returns the handle to the existing grid "gdid",
!     * gdid.  If the grid is not found, gdattach returns -1 for the handle.
!     */
	
	if (gdfid .ne. -1) then
	   
	   gdid = gdattach(gdfid, "UTMGrid")
	   
	   if (gdid .ne. -1) then
	      
              xdim = 120;
              ydim = 200;
	      
              status = gdsetdimscale(gdid, "Vegetation", "XDim", xdim,
     1	      DFNT_FLOAT64, veg1)
	      write(*,*) 'status: gdsetdimscale Vegetation:XDim',status
              status = gdsetdimscale(gdid, "Vegetation", "YDim", ydim,
     1	      DFNT_FLOAT64, veg2)
	      write(*,*) 'status: gdsetdimscale Vegetation:YDim',status

!	label, = 'X Dimension'
!	unit, = 'meters'
!	format, = 'F7.2'

              status = gdsetdimstrs(gdid, "Vegetation", "YDim", 
     1	      "Y Dimension", "meters", "F7.2")

	   endif

	   status = gddetach(gdid)

	   gdid = gdattach(gdfid, "PolarGrid")
	   if (gdid .ne. -1) then
	      
	      xdim = 100
	      ydim = 100
	      
	      status = gdsetdimscale(gdid, "Temperature", "XDim", xdim,
     1	      DFNT_FLOAT64, temp1)
	      write(*,*) 'status: gdsetdimscale Temperature:XDim',status
	      write(*,*) 'this -1 was expected for Temperature:XDim'
	      status = gdsetdimscale(gdid, "Temperature", "YDim", ydim,
     1	      DFNT_FLOAT64, temp2)
	      write(*,*) 'status: gdsetdimscale Temperature:YDim',status
	      write(*,*) 'this -1 was expected for Temperature:YDim'
	      nbands = 3;
	      do j=1,nbands
		 bands(j) = j*2+1
	      enddo
	      status = gdsetdimscale(gdid, "Spectra", "YDim", ydim, 
     1	      DFNT_FLOAT64, temp2)
	      write(*,*) 'status: gdsetdimscale Spectra:YDim',status

 	      status = gdsetdimscale(gdid, "Spectra", "Bands", nbands,
     1	      DFNT_INT32, bands);
	      write(*,*) 'status: gdsetdimscale Spectra:Bands',status
	      
	      status = gdsetdimstrs(gdid, "Spectra", "YDim", "YDim Dim",
     1	      "meters", "F7.2")
	      
	      status = gdsetdimstrs(gdid, "Spectra", "Bands", "BANDS",
     1	      "None", "I2")
	      
	      status = gddetach(gdid)
	   endif
	   
	   status = gdclose(gdfid)
	endif
	stop
	end

