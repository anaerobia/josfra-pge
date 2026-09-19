!/*
! * In this example we will (1) open the "GridFile" HDF file, (2) attach to
! * the "gdid" grid, and (3) set dimension scale for a dimension in a filed.
! */

	program readdimscalegrid
	
	integer       j, status, gddetach, gdclose
	integer*4     gdfid, gdid1, gdid2, gdopen, gdattach
	integer*4     gdgetdimscale, gdgetdimstrs
	real*8        veg1(120),veg2(200)
	real*8        temp1(100),temp2(100)
	integer*4     bands(3)
	integer*4     xdimsize, ydimsize,dimsize
	integer*4     xdimsize1, ydimsize1,dimsize1
	real*8        upleftpt(2), lowrightpt(2)
	integer*4     ntype(32)
	character*16  label, unit, format
	integer*4     len
	integer*4     nbands
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
!     * The GDattach routine returns the handle to the existing grid "gdid",
!     * gdid.  If the grid is not found, gdattach returns -1 for the handle.
!     */	
	if (gdfid .ne. -1) then
	   
	   gdid1 = gdattach(gdfid, "UTMGrid")
	   
	   if (gdid1 .ne. -1) then
	      
	      status = gdgridinfo(gdid1, xdimsize, ydimsize,
     1	      upleftpt, lowrightpt)
	      
	      write(*,*) 'X dim size, Y dim size (PolarGrid): ', 
     1	      xdimsize, ydimsize
	      write(*,*) 'Up left pt (UTMGrid): ', 
     1	      upleftpt(1), upleftpt(2)
	      write(*,*) 'Low right pt (UTMGrid): ', 
     1	      lowrightpt(1), lowrightpt(2)
	      
	      buffsize = gdgetdimscale(gdid1, "Vegetation",
     1         "XDim", xdimsize1, ntype,veg1)
	      if (buffsize .eq. -1) then	   
		 write(*,*) 'Error:Cannot get Dim Scale for XDim',
     1                      ' in field Vegetation'
		 stop  
	      else
		 write(*,*) 'For Vegetation field XDim: xdimsize1 = ', 
     1                      xdimsize1
		 write(*,*) 'For Vegetation field XDim: buffsize = ', 
     1                      buffsize
	      endif
	      
	      write(*,*) 'First 10 elements for Vegetation field XDim',
     1                   ' dimension scale values:'
	      do j=1,10
		 write(*,*) 'j =',j,'         veg1 = ',veg1(j)
	      enddo
	      write(*,*) '  '
	      
	      buffsize = gdgetdimscale(gdid1, "Vegetation", "YDim",
     1                                  ydimsize1,ntype,veg2)
	      if (buffsize .eq. -1) then	   
		 write(*,*) 'Error:Cannot get Dim Scale for YDim',
     1                      ' in field Vegetation"'
		 stop  
	      else
		 write(*,*) 'For Vegetation field YDim: ydimsize1 = ', 
     1                      ydimsize1
		 write(*,*) 'For Vegetation field YDim: buffsize = ', 
     1                      buffsize
	      endif
	      
	      write(*,*) 'First 10 elements for Vegetation field YDim',
     1                   ' dimension scale values:'
	      do j=1,10
		 write(*,*) 'j =',j,'         veg2 = ',veg2(j)
	      enddo
	      write(*,*) '  '
	      label = ''
	      unit = ''
	      format = ''
	      len = 15
	      status = gdgetdimstrs(gdid1, "Vegetation", "YDim", 
     1	      label, unit, format, len)
	      
	      
	      if(status .eq. 0) then	  
		 write(*,*) 'For Vegetation field YDim',
     1                      ' dimension scale attr. strings:'
		 write(*,*) 'label =',label,'  unit =',
     1                      unit,'  format = ',format
	      endif
	      write(*,*) ' '
	      status = gddetach(gdid1)
	   endif

	   gdid2 = gdattach(gdfid, "PolarGrid")
	   if (gdid2 .ne. -1) then
	      
!	/* we should get
!       ydim = 100	
!       nbands = 3;	
!	*/		
!        /* get YDim */	
	   
	      status = gdgridinfo(gdid2, xdimsize, ydimsize,
     1	      upleftpt, lowrightpt)
	      
	      write(*,*) 'X dim size, Y dim size (PolarGrid): ', 
     1	      xdimsize, ydimsize
	      write(*,*) 'Up left pt (PolarGrid): ', 
     1	      upleftpt(1), upleftpt(2)
	      write(*,*) 'Low right pt (PolarGrid): ', 
     1	      lowrightpt(1), lowrightpt(2)
	      
	      buffsize = gdgetdimscale(gdid2, "Spectra", "YDim",
     1	                               ydimsize1,ntype, temp2)
	      if (buffsize .eq. -1) then	   
		 write(*,*) 'Error:Cannot get Dim Scale for YDim',
     1                      ' in field Spectra'
		 stop  
	      else
		 write(*,*) 'For Spectra field YDim: ydimsize1 = ', 
     1                      ydimsize1
		 write(*,*) 'For Spectra field YDim: buffsize = ', 
     1                       buffsize
	      endif
	      
	      write(*,*) 'First 10 elements for Spectra field YDim',
     1                   ' dimension scale values:'
	      do j=1,10
		 write(*,*) 'j =',j,'         temp2 = ',temp2(j)
	      enddo
	      write(*,*) '  '
	      
!	/* get str attributes */
	      len = 15
	      status = gdgetdimstrs(gdid2, "Spectra", "YDim", 
     1	      label, unit, format, len)
	      
	      
	      if(status .eq. 0) then	  
		 write(*,*) 'For Spectra field YDim',
     1                      ' dimension scale attr. strings:'
		 write(*,*) 'label =',label,'  unit =',
     1                      unit,'  format = ',format
	      endif
	      write(*,*) ' '
	      
	      dimsize = gddiminfo(gdid2, "Bands");
	      status = gdgetdimscale(gdid2, "Spectra", "Bands", 
     1                               nbands,ntype, bands)
	      write(*,*) 'For Spectra field Bands: nbands = ', 
     1	                  nbands
	      write(*,*) 'For Spectra field Bands',
     1	                 'dimension scale values:'

	      do j=1,nbands
		 write(*,*) 'j =',j,'         Bnads = ',bands(j)
	      enddo
	      write(*,*) '  '
	      
	      status = gdgetdimstrs(gdid2, "Spectra", "Bands", 
     1	      label, unit, format, len)
	      
	      if(status .eq. 0) then
		 write(*,*) 'For Spectra field Bands',
     1                      ' dimension scale attr. strings:'
		 write(*,*) 'label =',label,'  unit =',
     1                      unit,'  format = ',format
	      endif
	      
	      status = gddetach(gdid2)
	   endif
	   status = gdclose(gdfid)
	endif
	stop
	end
