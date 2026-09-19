#include "hdf.h"
#include "HdfEosDef.h"

/*
 * In this example we will (1) open the "GridFile" HDF file, (2) attach to
 * the "Grid1" grid, and (3) get dimension scale for a dimension in a filed.
 */


main()
{

    intn            status, i, j;
    int32           gdfid, GDid1, GDid2;
    int32           xdim, ydim;
    float64         *datbuf_flt64;
    int32           *datbuf_int32;
    int32           nbands;
    int32           ntime;
    intn            buffsize;
    int32           data_type;
    int32           xdimsize, ydimsize, dimsize;
    int32           xdimsize1, ydimsize1, dimsize1;
    intn            len;
    char            label[16], unit[16], format[16];
    float64         upleftpt[2], lowrightpt[2];

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
	   see SetupGrid.c and GDdefineGDflds.c 
	   xdim = 120;
	   ydim = 200;
	   Time dim is set to 10
	*/

	GDid1 = GDattach(gdfid, "UTMGrid");
	if (GDid1 == -1)
	  {
	    printf("\t\tError: Cannot attach to grid \"UTMGrid\"\n");
	    return -1;
	  }

	/* we should get
	   xdim = 120;
	   ntime = 10;
	*/

	/* get XDim */
	status = GDgridinfo(GDid1, &xdimsize, &ydimsize,
			    upleftpt, lowrightpt);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot get grid info for \"UTMGrid\"\n");
	    return -1;
	  }
	printf("X dim size, Y dim size (UTMGrid): %d %d\n", 
	       xdimsize, ydimsize);
	printf("Up left pt (UTMGrid): %lf %lf\n", 
	       upleftpt[0], upleftpt[1]);
	printf("Low right pt (UTMGrid): %lf %lf\n", 
	       lowrightpt[0], lowrightpt[1]);

	buffsize = GDgetdimscale(GDid1, "Pollution", "XDim", &xdimsize1, &data_type, NULL);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for XDim dimemnsion in field \"Pollution\"\n");
	    return -1;
	  }
	else
	  {
	    printf(" \t\tFor Pollution field XDim: xdimsize= %d  buffsize = %d data_type = %d\n",xdimsize, buffsize,data_type);
	    printf(" \t\tFor Pollution field XDim: xdimsize1= %d  buffsize = %d data_type = %d\n",xdimsize1, buffsize,data_type);
	    datbuf_flt64 = (float64 *)malloc(buffsize);
	  }

	buffsize = GDgetdimscale(GDid1, "Pollution", "XDim", &xdimsize1, &data_type, (void *)datbuf_flt64);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for XDim dimemnsion in field \"Pollution\"\n");
	    return -1;
	  }
	else
	  {
	    printf(" \t\tFor Pollution field XDim dimension scale values:\n");
	    for (i=0; i<xdimsize; i++)
	      {
		printf("i = %d       datbuf_flt64 = %lf\n", i, datbuf_flt64[i]);
	      }
	    printf(" \n");
	  }

	if(datbuf_flt64 != NULL)
	  {
	    free(datbuf_flt64);
	    datbuf_flt64 = NULL;
	  }

	/* get str attributes */
	len = 15;
	status = GDgetdimstrs(GDid1, "Pollution", "XDim",
			      label, unit, format, len);
	if(status == 0)
	  {
	    printf(" \t\tFor Pollution field XDim dimension scale attr. strings:\n");
	    printf(" \t\tlabel = %s   unit = %s   format = %s \n\n", label, unit, format);
	  }


	/* get Time */

	dimsize = GDdiminfo(GDid1, "Time");
	buffsize = GDgetdimscale(GDid1, "Pollution", "Time", &dimsize1, &data_type, NULL);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for Time dimemnsion in field \"Pollution\"\n");
	    return -1;
	  }
	else
	  {
	    printf(" \t\tFor Pollution field Time: dimsize= %d buffsize = %d data_type = %d\n",dimsize, buffsize,data_type);
	    printf(" \t\tFor Pollution field Time: dimsize1= %d buffsize = %d data_type = %d\n",dimsize1, buffsize,data_type);
	    datbuf_int32 = (int32 *)malloc(buffsize);
	  }

	buffsize = GDgetdimscale(GDid1, "Pollution", "Time", &dimsize1, &data_type, (void *)datbuf_int32);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for Time dimemnsion in field \"Pollution\"\n");
	    return -1;
	  }
	else
	  {
	    printf(" \t\tFor Pollution field Time dimension scale values:\n");
	    for (i=0; i<dimsize; i++)
	      {
		printf("i = %d       datbuf_int32 = %d\n", i, datbuf_int32[i]);
	      }
	    printf(" \n");
	  }

	/* get str attributes */
	len = 15;
	status = GDgetdimstrs(GDid1, "Pollution", "Time",
			      label, unit, format, len);
	if(status == 0)
	  {
	    printf(" \t\tFor Pollution field Time dimension scale attr. strings:\n");
	    printf(" \t\tlabel = %s   unit = %s   format = %s \n\n", label, unit, format);
	  }

	if(datbuf_int32 != NULL)
	  {
	    free(datbuf_int32);
	    datbuf_int32 = NULL;
	  }

	  /* the field Spectra has Bands,YDim,XDim dimensions. 
	   see SetupGrid.c and GDdefineGDflds.c 
	   xdim = 100;
	   ydim = 100;
	   Bands dim is set to 3
	  */

	GDid2 = GDattach(gdfid, "PolarGrid");
	if (GDid2 == -1)
	  {
	    printf("\t\tError: Cannot attach to grid \"PolarGrid\"\n");
	    return -1;
	  }

	/* we should get
	   ydim = 100;
	   nbands = 3;
	*/

	/* get YDim */
	status = GDgridinfo(GDid2, &xdimsize, &ydimsize,
			    upleftpt, lowrightpt);
	if (status == -1)
	  {
	    printf("\t\tError: Cannot get grid info for \"PolarGrid\"\n");
	    return -1;
	  }
	printf("X dim size, Y dim size (PolarGrid): %d %d\n", 
	       xdimsize, ydimsize);
	printf("Up left pt (UTMGrid): %lf %lf\n", 
	       upleftpt[0], upleftpt[1]);
	printf("Low right pt (UTMGrid): %lf %lf\n", 
	       lowrightpt[0], lowrightpt[1]);


	buffsize = GDgetdimscale(GDid2, "Spectra", "YDim", &ydimsize1, &data_type, NULL);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for YDim dimemnsion in field \"Spectra\"\n");
	    return -1;
	  }
	else
	  {
	    printf(" \t\tFor Spectra field YDim: ydimsize = %d buffsize = %d data_type = %d\n",ydimsize, buffsize,data_type);
	    printf(" \t\tFor Spectra field YDim: ydimsize1 = %d buffsize = %d data_type = %d\n",ydimsize1, buffsize,data_type);
	    datbuf_flt64 = (float64 *)malloc(buffsize);
	  }

	   buffsize = GDgetdimscale(GDid2, "Spectra", "YDim", &ydimsize1, &data_type, (void *)datbuf_flt64);
	if (buffsize == -1)
	  {
	    printf("\t\tError: Cannot get Dimension Scale for YDim dimemnsion in field \"Spectra\"\n");
	    return -1;
	  }
	else
	  {
	    printf(" \t\tFor Spectra field YDim dimension scale values:\n");
	    for (i=0; i<ydimsize; i++)
	      {
		printf("i = %d       datbuf_flt64 = %lf\n", i, datbuf_flt64[i]);
	      }
	    printf(" \n");
	  }

	/* get str attributes */
	len = 15;
	status = GDgetdimstrs(GDid2, "Spectra", "YDim",
			      label, unit, format, len);
	if(status == 0)
	  {
	    printf(" \t\tFor Spectra field YDim dimension scale attr. strings:\n");
	    printf(" \t\tlabel = %s   unit = %s   format = %s \n\n", label, unit, format);
	  }

	if(datbuf_flt64 != NULL)
	  {
	    free(datbuf_flt64);
	    datbuf_flt64 = NULL;
	  }

	/* get Bands */

	dimsize = GDdiminfo(GDid2, "Bands");
	buffsize = GDgetdimscale(GDid2, "Spectra", "Bands", &dimsize1, &data_type, NULL);
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

	buffsize = GDgetdimscale(GDid2, "Spectra", "Bands", &dimsize1, &data_type, (void *)datbuf_int32);
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
	status = GDgetdimstrs(GDid2, "Spectra", "Bands",
			      label, unit, format, len);
	if(status == 0)
	  {
	    printf(" \t\tFor Spectra field Bands dimension scale attr. strings:\n");
	    printf(" \t\tlabel = %s   unit = %s   format = %s \n\n", label, unit, format);
	  }

	if(datbuf_flt64 != NULL)
	  {
	    free(datbuf_flt64);
	    datbuf_flt64 = NULL;
	  }

	if(datbuf_int32 != NULL)
	  {
	    free(datbuf_int32);
	    datbuf_int32 = NULL;
	  }
    }

    GDdetach(GDid1);
    GDdetach(GDid2);

    GDclose(gdfid);

    return 0;
}
