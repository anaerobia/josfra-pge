#include "hdf.h"
#include "HdfEosDef.h"

/*
 * In this example we will (1) open the "GridFile" HDF file, (2) attach to
 * the "Grid1" grid, and (3) set dimension scale for a dimension in all filed of a grid.
 */


main()
{

    intn            status, i, j;
    int32           gdfid, GDid1, GDid2, GDid3;
    int32           xdim, ydim;
    int32           ntime;
    int32           nbands;
    int32           data[10]={3,6,9,12,15,18,23,26,29,32};
    int32           bands[10]={12,14,18};
    char            label[16];
    char            unit[16];
    char            format[16];

    /*
     * We first open the HDF grid file, "GridFile.hdf".  Because this file
     * already exist and we wish to write to it, we use the DFACC_RDWR access
     * code in the open statement.  The GDopen routine returns the grid file
     * id, gdfid, which is used to identify the file in subsequent routines.
     */

    gdfid = GDopen("GridFile_created_with_hadeos_sample_file_writer_of_HDFEOS2_version_219_or_higher_release.hdf", DFACC_RDWR);


    /*
     * If the grid file cannot be found, GDopen will return -1 for the file
     * handle (gdfid).  We there check that this is not the case before
     * proceeding with the other routines.
     * 
     * The GDattach routine returns the handle to the existing grid "Grid1",
     * GDid.  If the grid is not found, GDattach returns -1 for the handle.
     */

    if (gdfid != -1)
    {
	/* the field Pollution has Time,YDim,XDim dimensions. 
	   see SetupGrid.c and DefineGDflds.c 
	   xdim = 120;
	   ydim = 200;
	   Time dim is set to 10
	*/

	GDid1 = GDattach(gdfid, "UTMGrid");
	if (GDid1 == -1)
	  {
	    printf("\t\tError: Cannot attach to grid \"UTMGrid\"\n");
	    GDclose(gdfid);
	    return -1;
	  }

	xdim = 120;
	ntime = 10;
	
	status = GDdefdimscale(GDid1, "XDim", xdim, DFNT_FLOAT64, NULL);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale for XDim dimemnsion in field \"Pollution\"\n");
	    GDdetach(GDid1);
	    GDclose(gdfid);
	    return -1;
	  }

	status = GDdefdimscale(GDid1, "Time", ntime, DFNT_INT32, data);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale for Time dimemnsion in field \"Pollution\"\n");
	    GDdetach(GDid1);
	    GDclose(gdfid);
	    return -1;
	  }

	strcpy(label, "X Dimension");
	strcpy(unit, "meters");
	strcpy(format, "F7.2");

	status = GDdefdimstrs(GDid1, "XDim", label, unit, format);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale strs for XDim dimemnsion in field \"Pollution\"\n");
	    GDdetach(GDid1);
	    GDclose(gdfid);
	    return -1;
	  }

	strcpy(label, "Time Dim");
	strcpy(unit, "s");
	strcpy(format, "I3");

	status = GDdefdimstrs(GDid1, "Time", label, unit, format);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale strs for Time dimemnsion in field \"Pollution\"\n");
	    GDdetach(GDid1);
	    GDclose(gdfid);
	    return -1;
	  }

	GDdetach(GDid1);

	/* the field Spectra has Bands,YDim,XDim dimensions. 
	   see SetupGrid.c and DefineGDflds.c
	   xdim = 100;
	   ydim = 100;
	   Bands dim is set to 3
	*/
	
	GDid2 = GDattach(gdfid, "PolarGrid");
	if (GDid2 == -1)
	  {
	    printf("\t\tError: Cannot attach to grid \"PolarGrid\"\n");
	    GDclose(gdfid);
	    return -1;
	  }
	
	ydim = 100;
	nbands = 3;
	
	status = GDdefdimscale(GDid2, "YDim", ydim, DFNT_FLOAT64, NULL);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale for YDim dimemnsion in field \"Spectra\"\n");
	    GDdetach(GDid2);
	    GDclose(gdfid);
	    return -1;
	  }


	status = GDdefdimscale(GDid2, "Bands", nbands, DFNT_INT32, bands);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale for Bands dimemnsion in field \"Spectra\"\n");
	    GDdetach(GDid2);
	    GDclose(gdfid);
	    return -1;
	  }


	strcpy(label, "Y Dimension");
	strcpy(unit, "meters");
	strcpy(format, "F7.2");

	status = GDdefdimstrs(GDid2, "YDim", label, unit, format);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale strs for YDim dimemnsion in field \"Spectra\"\n");
	    GDdetach(GDid2);
	    GDclose(gdfid);
	    return -1;
	  }

	strcpy(label, "Bands");
	strcpy(unit, "None");
	strcpy(format, "I2");

	status = GDdefdimstrs(GDid2, "Bands", label, unit, format);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale strs for Bands dimemnsion in field \"Spectra\"\n");
	    GDdetach(GDid2);
	    GDclose(gdfid);
	    return -1;
	  }

    }

	GDdetach(GDid2);

    /* the field GeoSpectra has YDim,XDim dimensions. 
       see SetupGrid.c and DefineGDflds.c
       xdim = 100;
       ydim = 100;
       Bands dim is set to 3
    */

    xdim = 60;
    ydim = 40;

    GDid3 = GDattach(gdfid, "GEOGrid");
    if (GDid3 == -1)
      {
	printf("\t\tError: Cannot attach to grid \"GEOGrid\"\n");
	GDclose(gdfid);
	return -1;
      }
    status = GDdefdimscale(GDid3, "YDim", ydim, DFNT_FLOAT64, NULL);
    if (status == -1)
      {
	printf("\t\tError: Cannot set Dimension Scale for YDim dimemnsion in field \"GeoSpectra\"\n");
	GDdetach(GDid3);
	GDclose(gdfid);
	return -1;
      }
    
    strcpy(label, "Y Dimension");
    strcpy(unit, "Decimal Degrees");
    strcpy(format, "F7.2");
    
    status = GDdefdimstrs(GDid3, "YDim", label, unit, format);
    if (status == -1)
      {
	printf("\t\tError: Cannot set Dimension Scale strs for YDim dimemnsion in field \"GeoSpectra\"\n");
	GDdetach(GDid3);
	GDclose(gdfid);
	return -1;
      }
    

    GDdetach(GDid3);
    GDclose(gdfid);

    return 0;
}
