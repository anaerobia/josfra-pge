#include "hdf.h"
#include "HdfEosDef.h"

/*
 * In this example we will (1) open the "SwathFile" HDF file, (2) attach to
 * the "Swath1" swath, and (3) get dimension scale for a dimension in a filed.
 */


main()
{

    intn            status, i, j;
    int32           swfid, SWid;
    int32           *datbuf_int32;
    float           *datbuf_fl32;
    float           *datbuf2_fl32;
    int32           nbands;
    intn            buffsize;
    intn            buffsize2;
    intn            buffsize3;
    int32           data_type;
    int32           data_type2;
    int32           data_type3;
    int32           dimsize;
    int32           dimsize1;
    int32           dimsize2;
    int32           dimsize3;
    intn            len;
    intn            len2;
    intn            len3;
    char            label[16], unit[16], format[16];
    char            label2[16], unit2[16], format2[16];
    char            label3[16], unit3[16], format3[16];

    /*
     * We first open the HDF swath file, "GridFile.hdf".  Because this file
     * already exist and we wish to write to it, we use the DFACC_RDWR access
     * code in the open statement.  The SWopen routine returns the swath file
     * id, swfid, which is used to identify the file in subsequent routines.
     */

    swfid = SWopen("SwathFile_created_with_hadeos_sample_file_writer_of_HDFEOS2_version_219_or_higher_release.hdf", DFACC_RDWR);


    /*
     * If the swath file cannot be found, SWopen will return -1 for the file
     * handle (swfid).  We there check that this is not the case before
     * proceeding with the other routines.
     * 
     * The SWattach routine returns the handle to the existing swath "Swath1",
     * SWid.  If the swath is not found, SWattach returns -1 for the handle.
     */

    if (swfid != -1)
      {
	/* the field Spectra has Bands,Res2tr,Res2xtr dimensions. 
	   see SetupSwath.c and SWdefineflds.c 
	   Res2tr = 40
	   Res2xtr = 20
	   Bands = 15
	*/
	
	SWid = SWattach(swfid, "Swath1");
	if (SWid == -1)
	  {
	    printf("\t\tError: Cannot attach to swath \"Swath1\"\n");
	    return -1;
	  }
	
	/* get Bands */
	
	dimsize = SWdiminfo(SWid, "Bands");
	buffsize = SWgetdimscale(SWid, "Spectra", "Bands", &dimsize1, &data_type, NULL);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for Bands dimemnsion in field \"Spectra\"\n");
	    return -1;
	  }
     	else
	  {
	    printf(" \t\tFor Spectra field Bands: dimsize = %d  buffsize = %d data_type = %d\n",dimsize, buffsize,data_type);
	    printf(" \t\tFor Spectra field Bands: dimsize1 = %d  buffsize = %d data_type = %d\n",dimsize1, buffsize,data_type);
	    datbuf_int32 = (int32 *)malloc(buffsize);
	  }
	
	buffsize = SWgetdimscale(SWid, "Spectra", "Bands", &dimsize1, &data_type, (void *)datbuf_int32);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for Bands dimemnsion in field \"Spectra\"\n");
	    return -1;
	  }
     	else
	  {
	    printf(" \t\tFor Spectra field Bands dimension scale values:\n");
	    for (i=0; i<dimsize; i++)
	      {
		printf("i = %d       datbuf_int32 = %d\n", i, datbuf_int32[i]);
	      }
	    printf(" \n");
	  }
	
	/* get str attributes */
	len = 15;
	status = SWgetdimstrs(SWid, "Spectra", "Bands",
			      label, unit, format, len);
	if(status == 0)
	  {
	    printf(" \t\tFor Spectra field Bands dimension scale attr. strings:\n");
	    printf(" \t\tlabel = %s   unit = %s   format = %s \n\n", label, unit, format);
	  }


	/*--------------- "Temperature", "GeoTrack" ------------------*/


	dimsize2 = SWdiminfo(SWid, "GeoTrack");
	buffsize2 = SWgetdimscale(SWid, "Temperature", "GeoTrack", &dimsize2, &data_type2, NULL);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for GeoTrack dimemnsion in field \"Temperature\"\n");
	    return -1;
	  }
     	else
	  {
	    printf(" \t\tFor Temperature field GeoTrack: dimsize = %d  buffsize = %d data_type = %d\n",dimsize2, buffsize2,data_type2);
	    datbuf_fl32 = (float *)malloc(buffsize2);
	  }
	
	buffsize2 = SWgetdimscale(SWid, "Temperature", "GeoTrack", &dimsize2, &data_type2, (void *)datbuf_fl32);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for GeoTrack dimemnsion in field \"Temperature\"\n");
	    return -1;
	  }
     	else
	  {
	    printf(" \t\tFor Temperature field GeoTrack dimension scale values:\n");
	    for (i=0; i<dimsize2; i++)
	      {
		printf("i = %d       datbuf_fl32 = %f\n", i, datbuf_fl32[i]);
	      }
	    printf(" \n");
	  }
	
	/* get str attributes */
	len2 = 20;
	status = SWgetdimstrs(SWid, "Temperature", "GeoTrack",
			      label2, unit2, format2, len2);
	if(status == 0)
	  {
	    printf(" \t\tFor Temperature field GeoTrack dimension scale attr. strings:\n");
	    printf(" \t\tlabel = %s   unit = %s   format = %s \n\n", label2, unit2, format2);
	  }

	/*--------------- "Temperature_3D", "GeoTrack" ------------------*/

	dimsize3 = SWdiminfo(SWid, "GeoTrack");
	buffsize3 = SWgetdimscale(SWid, "Temperature_3D", "GeoTrack", &dimsize3, &data_type3, NULL);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for GeoTrack dimemnsion in field \"Temperature_3D\"\n");
	    return -1;
	  }
     	else
	  {
	    printf(" \t\tFor Temperature_3D field GeoTrack: dimsize = %d  buffsize = %d data_type = %d\n",dimsize3, buffsize3,data_type3);
	    datbuf2_fl32 = (float *)malloc(buffsize3);
	  }
	
	buffsize3 = SWgetdimscale(SWid, "Temperature_3D", "GeoTrack", &dimsize3, &data_type3, (void *)datbuf2_fl32);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for GeoTrack dimemnsion in field \"Temperature_3D\"\n");
	    return -1;
	  }
     	else
	  {
	    printf(" \t\tFor Temperature_3D field GeoTrack dimension scale values:\n");
	    for (i=0; i<dimsize3; i++)
	      {
		printf("i = %d       datbuf2_fl32 = %f\n", i, datbuf2_fl32[i]);
	      }
	    printf(" \n");
	  }
	
	/* get str attributes */
	len3 = 20;
	status = SWgetdimstrs(SWid, "Temperature_3D", "GeoTrack",
			      label3, unit3, format3, len3);
	if(status == 0)
	  {
	    printf(" \t\tFor Temperature_3D field GeoTrack dimension scale attr. strings:\n");
	    printf(" \t\tlabel = %s   unit = %s   format = %s \n\n", label3, unit3, format3);
	  }


	



	if(datbuf_int32 != NULL)
	  {
	    free(datbuf_int32);
	    datbuf_int32 = NULL;
	  }
      }
    




    SWdetach(SWid);
    SWclose(swfid);
    return 0;
}
