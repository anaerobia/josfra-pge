#include "hdf.h"
#include "HdfEosDef.h"

/*
 * In this example we will (1) open the "SwathFile" HDF file, (2) attach to
 * the "Swath1" swath, and (3) set dimension scale for a dimension in a filed.
 */


main()
{

    intn            status, i, j;
    int32           swfid, SWid;
    int32           nbands, nGeoTrack;
    int32           bands[15]={3,6,9,12,15,18,23,26,29,32,33,34,36,37,39};
    char            label[16];
    char            unit[16];
    char            format[16];
    float           dataGeoTrack[20]={1.,2.,3.,6.,9.,12.,14.,15.,16.,17.,18.,23.,26.,29.,32.,33.,34.,36.,37.,39.};

    /*
     * We first open the HDF swath file, "SwathFile.hdf".  Because this file
     * already exist and we wish to write to it, we use the DFACC_RDWR access
     * code in the open statement.  The SWopen routine returns the swath file
     * id, swfid, which is used to identify the file in subsequent routines.
     */

    swfid = SWopen("SwathFile_created_with_hadeos_sample_file_writer_of_HDFEOS2_version_219_or_higher_release.hdf", DFACC_RDWR);
    if (swfid == -1)
      {
	printf("\t\tError: Cannot open file \"SwathFile.hdf\"\n");
	return -1;
      }

    /*
     * If the swath file cannot be found, SWopen will return -1 for the file
     * handle (swfid).  We there check that this is not the case before
     * proceeding with the other routines.
     * 
     * The SWattach routine returns the handle to the existing swath "Swath1",
     * GDid.  If the swath is not found, SWattach returns -1 for the handle.
     */

    if (swfid != -1)
      {
	/* the field Spectra has Bands, GeoTrack, GeoXtrack dimensions. 
	   see SetupSwath.c and Defineflds.c
	   GeoTrack = 20;
	   GeoXtrack = 10;
	*/
	
	SWid = SWattach(swfid, "Swath1");
	if (SWid == -1)
	  {
	    printf("\t\tError: Cannot attach to swath \"Swath1\"\n");
	    SWclose(swfid);
	    return -1;
	  }


	nGeoTrack = 20;
	strcpy(label, "GeoTrack");
	strcpy(unit, "Degree");
	strcpy(format, "F2");

	status = SWsetdimscale(SWid, "Temperature", "GeoTrack", nGeoTrack, DFNT_FLOAT32, dataGeoTrack);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale for GeoTrack dimemnsion in field \"Temperature\"\n");
	    SWdetach(SWid);
	    SWclose(swfid);
	    return -1;
	  }
	status = SWsetdimstrs(SWid, "Temperature", "GeoTrack", label, unit, format);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale strs for GeoTrack dimemnsion in field \"Temperature\"\n");
	    SWdetach(SWid);
	    SWclose(swfid);
	    return -1;
	  }


	nbands =  15;
	
	status = SWsetdimscale(SWid, "Spectra", "Bands", nbands, DFNT_INT32, bands);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale for Bands dimemnsion in field \"Spectra\"\n");
	    SWdetach(SWid);
	    SWclose(swfid);
	    return -1;
	  }
	
	strcpy(label, "Bands");
	strcpy(unit, "none");
	strcpy(format, "I2");
	
	status = SWsetdimstrs(SWid, "Spectra", "Bands", label, unit, format);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot set Dimension Scale strs for Bands dimemnsion in field \"Spectra\"\n");
	    SWdetach(SWid);
	    SWclose(swfid);
	    return -1;
	  }
	
	SWdetach(SWid);
	
	SWclose(swfid);
	
	return 0;

      }
}
