/*********************************************************
GDconver_ij2ll.c--

This function converts pixel coordinates i,j into lat/lon for a grid in 
an hdf-eos file. Once installed executable will be in hdfeos/bin/<brand> 
directory. Followings are how to run the executible, and the outcome:

a) ./GDconver_ij2ll <input_hdf_file_name> <input_grid_name>

   will write ASCII output for i,j,lat,lon onto STDOUT for whole grid.

b) ./GDconvert_ij2ll <input_hdf_file_name> <input_grid_name> <output_file_name_template> <-a>
   will write ASCII output for i,j,lat,lon into a file.

c) ./GDconvert_ij2ll <input_hdf_file_name> <input_grid_name> <output_file_name_template> <-b>
   will write 3 output files with names constructed using output_file_name_template:
       1. An ascii file containing info for input file and grid.
       2. A binary file containing latitudes (64bit flaot data), 
          a row follows another row for the grid
       3. A binary file containing longitudes (64bit flaot data), 
          a row follows another row for the grid

d) ./GDconvert_ij2ll <input_hdf_file_name> <input_grid_name> STDOUT <-a> <i j>
   will write ASCII output for i,j,lat,lon onto STDOUT for a single pair.

User may type ./GDconvert_ij2ll to see what the usage is. Also typing a
dummy grid name will return error, specifying what the valid grid names 
are in the hdf file.

Author--
Abe Taaheri, Raytheon IIS

Dates--
2/20/2007   AT  First Programming 

*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "hdf.h"
#include "mfhdf.h"
#include <math.h>
#include "HdfEosDef.h" 
#include "cproj.h"
#include "proj.h"

#define COMMAND_STRING      1024

typedef struct
{
  char inputFile[COMMAND_STRING];
  char gridName[COMMAND_STRING];
  char outputFile_nxny[COMMAND_STRING];
  char outputFile_lat[COMMAND_STRING];
  char outputFile_lon[COMMAND_STRING];
  char outFileFlag[COMMAND_STRING];
  long RowCol[2];
}
CommandArgument;

CommandArgument Com;

void strip_quote(char *gridname)
{
  char outstr[128];
  int lenstr=0;
  if(gridname[0] =='"')
    {
      gridname++;/*strip first quote */
      strcpy(outstr, gridname);
      lenstr = strlen(gridname);
      outstr[lenstr-1]='\0'; /*strip last quote */
      strcpy(gridname, outstr);
    }
}


void CommandLineUsage()
{
    fprintf( stderr, "\tUsage (ASCII output into STDOUT):\n\t./GDconvert_ij2ll <input_hdf_file_name> <input_grid_name>\n" );

    fprintf( stderr, " OR\n");
    fprintf( stderr, "\tUsage (ASCII output into a file):\n\t./GDconvert_ij2ll <input_hdf_file_name> <input_grid_name> <output_file_name_template> <-a>\n" );
 
    fprintf( stderr, " OR\n");
    fprintf( stderr, "\tUsage (for Binary output into a file):\n\t./GDconvert_ij2ll <input_hdf_file_name> <input_grid_name> <output_file_name_template> <-b>\n" );

    fprintf( stderr, " OR\n");
    fprintf( stderr, "\tUsage (lat/lon for a given (row col)):\n\t./GDconvert_ij2ll <input_hdf_file_name> <input_grid_name> STDOUT <-a> <i j>\n" );
    fprintf( stderr, " \n");
    fprintf( stderr, "\tNote: You should put gridname inside double quotes (\") if there is a blank space in it. \n" );
 }
/* get: 
   input hdf file
   Grid name to process
   Output ASCII filename
*/
int commandLineReader(int argc, char *argv[], CommandArgument *Com )
{
  printf( "\tNote: You should put gridname inside double quotes (\") if there is a blank space in it. \n" );
  printf( "\t      Otherwise the conversion may fail or produce unexpected results. \n" );
  if ( argc < 3 || argc > 7 || argc == 6) /* display usage in case of no or 
					  insufficient arguments */
    {
      /* user need help read the command line option, it is not error */
      CommandLineUsage();
      exit(-1); 
    }
  
  /* read command line parameters */
  if(argc ==  7)
    {/* will write one lat/lon pair ASCII output to stdout */
      strcpy(Com->inputFile, argv[1]);
      strcpy(Com->gridName, argv[2]);
      strip_quote(Com->gridName);
      strcpy(Com->outputFile_nxny, "");
      strcpy(Com->outputFile_lat, "");
      strcpy(Com->outputFile_lon, "");
      strcpy(Com->outFileFlag, argv[4]);
      Com->RowCol[0] = atol(argv[5]);
      Com->RowCol[1] = atol(argv[6]);
      if( Com->RowCol[0] < 0 || Com->RowCol[1] < 0)
	{
	  HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	  HEreport("Row or Column numbers cannot be negative.\n");
	  HEprint(stdout, 0);
	  return (-1);
	}
      return 0;
    }
  else if (argc ==  5)
    {/* will write (rows X cols) ASCII or BINARY output to output file */ 
      strcpy(Com->inputFile, argv[1]);
      strcpy(Com->gridName, argv[2]);
      strip_quote(Com->gridName);
      strcpy(Com->outputFile_nxny, argv[3]);
      strcpy(Com->outputFile_lat, argv[3]);
      strcpy(Com->outputFile_lon, argv[3]);
      strcat(Com->outputFile_nxny,"_ASCII");
      strcat(Com->outputFile_lat,"_lat");
      strcat(Com->outputFile_lon,"_lon");
      strcpy(Com->outFileFlag, argv[4]);
      Com->RowCol[0] = -1;
      Com->RowCol[1] = -1;
      return 0;
    }
  else if(argc ==  4)
    {/* will write (rows X cols) ASCII output to output file */ 
      strcpy(Com->inputFile, argv[1]);
      strcpy(Com->gridName, argv[2]);
      strip_quote(Com->gridName);
      strcpy(Com->outputFile_nxny, argv[3]);
      strcpy(Com->outputFile_lat, argv[3]);
      strcpy(Com->outputFile_lon, argv[3]);
      strcat(Com->outputFile_nxny,"_ASCII");
      strcat(Com->outputFile_lat,"_lat");
      strcat(Com->outputFile_lon,"_lon");
      strcpy(Com->outFileFlag, "-a");
      Com->RowCol[0] = -1;
      Com->RowCol[1] = -1;
      return 0;
    }
  else if (argc ==  3)
    { /* will write (rows X cols) ASCII output to stdout */ 
      strcpy(Com->inputFile, argv[1]);
      strcpy(Com->gridName, argv[2]);
      strip_quote(Com->gridName);
      strcpy(Com->outputFile_nxny, "");
      strcpy(Com->outputFile_lat, "");
      strcpy(Com->outputFile_lon, "");
      strcpy(Com->outFileFlag, "-a");
      Com->RowCol[0] = -1;
      Com->RowCol[1] = -1;
      return 0;
    }
}


/* Separating string */

void separating_String(char *stringlist, int32 *nstring, char *strings[], 
		       char strs[])
{
  char  *astring = NULL;
    
  *nstring=0;
  astring=strtok(stringlist, strs);
  while (astring!=NULL)
  {
    strings[*nstring]=astring;  
    astring=strtok(NULL, strs);
    *nstring=*nstring+1;
  } 
}



int main(int argc, char *argv[])
{
  int status = 0;
  int i, j, jx, jy,k, is;
  int        Singl_latlon = 0; /* get the lat/lon for whole grid */
  int32      gdfid, gdid[10],xdimsize, ydimsize;
  
  int32      ngrid, grid_not_found;
  char       gridlist[1000], tmp_gridlist[1000],*grids[20];
  int32      projcode=0; 
  int32      zonecode=0;
  float64    projparam[16]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		    	   0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			   0.0, 0.0, 0.0, 0.0};
  int32      spherecode =0;
  int32      strsize, pixregcode, origincode;
  float64    upleft[2], lowright[2];
  float64    lowleft[2], upright[2];
  int32      npnts;
  float64   *lats = NULL; 
  float64   *lons = NULL;
  int32     *rows = NULL; 
  int32     *cols = NULL;
  int        v1;

  FILE *outfile_nxny = NULL;  /* Pointer for ASCII outfile file for rows,cols*/
  FILE *outfile_lat = NULL;   /* Pointer for outfile file for lats*/
  FILE *outfile_lon = NULL;   /* Pointer for outfile file for lons */
  char projection[64];

 /* process command-line arguments (to get parameter filename) */
  status = commandLineReader(argc, argv, &Com);
    if (status == -1)
    {
      status = -1;
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("Problem with command line entries.\n");
      HEprint(stdout, 0);
      return(status);
    }
  gdfid=GDopen((&Com)->inputFile, DFACC_RDONLY);
  
  if (gdfid == -1)
    {
      status = -1;
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("The file %s cannot be opened.\n",(&Com)->inputFile);
      HEprint(stdout, 0);
      return(status);
    }
  
	  
  /*
    Inquire grid
  */
  
  ngrid=GDinqgrid((&Com)->inputFile, gridlist, &strsize);
  if (ngrid<1)
    {
      GDclose(gdfid);
      status = -1;
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("No grid exists in the file %s.\n",
	       (&Com)->inputFile);
      HEprint(stdout, 0);
      return(status);
    }

  strcpy(tmp_gridlist, gridlist);
  separating_String(tmp_gridlist, &ngrid, grids, ",");

  /* check for  the desired grid */
  grid_not_found = 0;
  for (i=0; i<ngrid; i++)
    {
      if(strcmp((&Com)->gridName,grids[i]) !=0)
	{
	  if(i == (ngrid -1))
	    {
	      grid_not_found = 1;
	      break;
	    }
	  else
	    {
	      continue;
	    }
	}
      else
	{
	  is =i;
	  break;
	}
    }
  
  if(grid_not_found == 1)
    {
      status = -1;
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("The file %s does not contain grid %s. The grids in this file are: %s\n",
	       (&Com)->inputFile, (&Com)->gridName,gridlist);
      HEprint(stdout, 0);
      GDclose(gdfid);
      return(status);
    }
  
  gdid[is]=GDattach(gdfid, grids[is]);
  
  status=GDprojinfo(gdid[is], &projcode, &zonecode, &spherecode, projparam);
  
  if (status==-1)
    {
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("No projection information for grid %s.\n",gdid[is]);
      HEprint(stdout, 0);
      GDdetach(gdid[is]);
      GDclose(gdfid);
      return(status);
    }

    /* set grid INFO */

    switch (projcode)
      {
      case GCTP_GEO:
	strcpy(projection,"GEOGRAPHIC");
	break;
      case GCTP_ISINUS1:
 	strcpy(projection,"INTEGERIZED SINUSOIDAL");
	break;
      case GCTP_UTM:
	strcpy(projection,"UNIVERSAL TRANSVERSE MERCATOR");
	break;
      case GCTP_LAMCC:
 	strcpy(projection,"LAMBERT CONFORMAL CONIC");
	break;
      case GCTP_PS:
	strcpy(projection,"POLAR STREOGRAPHIC");
	break;
      case GCTP_POLYC:
	strcpy(projection,"POLYCONIC");
	break;
      case GCTP_TM:
	strcpy(projection,"TRANSVERSE MERCATOR");
	break;
      case GCTP_LAMAZ:
	strcpy(projection,"LAMBERT AZIMUTHAL");
	break;
      case GCTP_HOM:
	strcpy(projection,"HOTIN OBLIQUE MERCATOR");
	break;
      case GCTP_SOM:
	strcpy(projection,"SPACE OBLIQUE MERCATOR");
	break;
      case GCTP_GOOD:
	strcpy(projection,"INTERRPTED GOODE");
	break;
      case GCTP_SPCS:
	strcpy(projection,"STATE PLANE");	
	break;
      case 99:
	strcpy(projection,"INTEGERIZED SINUSOIDAL");
	break;
      case GCTP_SNSOID:
	strcpy(projection,"SINUSOIDAL");
	break;
      case GCTP_MERCAT:
	strcpy(projection,"MERCATOR");
	break;
      case GCTP_ALBERS:
	strcpy(projection,"ALBERS CONICAL EQUAL AREA");
	break;
	
      default:
	strcpy(projection,"NOT SUPPORTED");
	fprintf(stdout,"Projection type does not exist.");
      }

  status=GDpixreginfo(gdid[is], &pixregcode);
  if (status==-1)
    {
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("No pixel rgistration code information for grid %s, assuming the default HDFE_CENTER.\n",gdid[is]);
      HEprint(stdout, 0);
      pixregcode = HDFE_CENTER;
    }
  else
    {
      if(pixregcode == HDFE_CORNER)
	{
	  status=GDorigininfo(gdid[is], &origincode);
	  if (status==-1)
	    {
	      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	      HEreport("No origin information for grid %s, assuming the default HDFE_GD_UL.\n",gdid[is]);
	      HEprint(stdout, 0);
	      origincode = HDFE_GD_UL;
	    }
	}
    }

  status=GDgridinfo(gdid[is], &xdimsize, &ydimsize, upleft, lowright);
  if (status==-1)
    {
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("No grid information for grid %s.\n",gdid[is]);
      HEprint(stdout, 0);
      GDdetach(gdid[is]);
      GDclose(gdfid);
      return(status);
    }
  

  /* lat/lon for other two corners UR and LL */
  /* these lat/los are in DMS format for GEO projection, and
     meters for others.
  */

  upright[0] = lowright[0];
  upright[1] = upleft[1];
  lowleft[0] = upleft[0];
  lowleft[1] = lowright[1];

  /* we now have upper left, Lower right corners, xdim, and ydim  
     lets get now the lat/lon for every i/j */

  if((&Com)->RowCol[0] >= 0 && (&Com)->RowCol[1] >= 0)
    {/* check to see if they are valid numbers */
      if((&Com)->RowCol[0] > (ydimsize-1))
	{
	  status = -1;
	  HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	  HEreport("Requestrd Row number %ld invalid. Should be less than %d.\n",(&Com)->RowCol[0], ydimsize);
	  HEprint(stdout, 0);
	  GDdetach(gdid[is]);
	  GDclose(gdfid);
	  return(status);
	}

      if((&Com)->RowCol[1] > (xdimsize-1))
	{
	  status = -1;
	  HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	  HEreport("Requestrd Column number %ld invalid. Should be less than %d.\n",(&Com)->RowCol[1],xdimsize);
	  HEprint(stdout, 0);
	  GDdetach(gdid[is]);
	  GDclose(gdfid);
	  return(status);
	}
      Singl_latlon = 1;/* get the lat/lon for a single pixel in the grid */
    }
  /* testing
  printf("Rows: ydimsize=%d\n",ydimsize);
  printf("cols: xdimsize=%d\n",xdimsize);
  printf("Row#: %d\n",(&Com)->RowCol[0]);
  printf("col#: %d\n",(&Com)->RowCol[1]);
  */
  if(Singl_latlon == 1)
    {
      npnts = 1;
    }
  else
    {
      npnts = xdimsize * ydimsize;
    }
      
  lons = (float64 *) malloc(npnts*sizeof(float64));
  if(lons == NULL) 
    {
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("memory problem allocating space to lons....\n");
      HEprint(stdout, 0);
      GDdetach(gdid[is]);
      GDclose(gdfid);
      return(status);
    }
  
  lats = (float64 *) malloc(npnts*sizeof(float64));
  if(lats == NULL) 
    {
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("memory problem allocating space to lats....\n");
      HEprint(stdout, 0);
      GDdetach(gdid[is]);
      GDclose(gdfid);
      return(status);
    }
  
  rows = (int32 *) malloc(npnts*sizeof(int32));
  if(rows == NULL) 
    {
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("memory problem allocating space to rows....\n");
      HEprint(stdout, 0);
      GDdetach(gdid[is]);
      GDclose(gdfid);
      return(status);
    }
  
  cols = (int32 *) malloc(npnts*sizeof(int32));
  if(cols == NULL) 
    {
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("memory problem allocating space to cols....\n");
      HEprint(stdout, 0);
      GDdetach(gdid[is]);
      GDclose(gdfid);
      return(status);
    }
  
  if(Singl_latlon == 1)
    {
      rows[0] = (int32) (&Com)->RowCol[0];
      cols[0] = (int32) (&Com)->RowCol[1];
    }
  else
    {
      /* LOAD UP row and col arrays for use by GDconvert_ij2ll */
      /* ----------------------------------------------*/
      k = 0;   /*  used for counting */
      
      for( jy = 0; jy < ydimsize; jy++ )   /* LOOP for each row     */
	{
	  for( jx = 0; jx < xdimsize; jx++ )   /* LOOP for each column  */
	    {  
	      rows[k] = jy;
	      cols[k] = jx;
	      k++;
	    }
	}
    }

  fprintf(stdout,"\n...calculating lat/lon values for grid...please wait... \n");

  /* Get lat/lon values of grid for given i,j values*/
  /* ---------------------------------------------- */
  status  =  GDij2ll(projcode, zonecode, projparam,
		     spherecode, xdimsize, ydimsize,
		     upleft, lowright,
		     npnts, rows, cols,
		     lons, lats, pixregcode, origincode);

  if(status == -1)
    {
      free(rows);
      rows = NULL;
      free(cols);
      cols = NULL;
      free(lons);
      lons = NULL;
      free(lats);
      lats = NULL;
      HEprint(stdout, 0);/* First print report issued in GDij2ll */
      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
      HEreport("Problem converting i,j to lat,lon in grid %s.\n",
	       (&Com)->gridName);
      HEprint(stdout, 0);
      GDdetach(gdid[is]);
      GDclose(gdfid);
      return(status);
    }
  else
    {
      /* open output ASCII/Binary file and print results to it */
      /* Or just print it stdout                               */

      if(strcmp((&Com)->outputFile_nxny, "") != 0) /* Put output into a file */
	{
	  if(strcmp((&Com)->outFileFlag,"-b") == 0) /*open in binary mode*/
	    {
	      outfile_nxny=fopen((&Com)->outputFile_nxny, "w" );
	      outfile_lat=fopen((&Com)->outputFile_lat, "wb" );
	      outfile_lon=fopen((&Com)->outputFile_lon, "wb" );
	    }
	  else if(strcmp((&Com)->outFileFlag,"-a") == 0) /*open in ascii mode*/
	    {
	      outfile_nxny=fopen((&Com)->outputFile_nxny, "w" );
	    }
	  else /*open in ascii mode*/
	    {
	      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	      HEreport("The flag for creating ASCII/Binary output file %s is -a or -b. \nAssuming -a : ASCII output file will be created.\n");
	      HEprint(stdout, 0);
	      outfile_nxny=fopen((&Com)->outputFile_nxny, "w" );
	    }

	  if(outfile_nxny == NULL )
	    {
	      free(rows);
	      rows = NULL;
	      free(cols);
	      cols = NULL;
	      free(lons);
	      lons = NULL;
	      free(lats);
	      lats = NULL;
	      status = -1;
	      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	      HEreport("The file %s cannot be opened.\n",
		       (&Com)->outputFile_nxny);
	      HEprint(stdout, 0);
	      GDdetach(gdid[is]);
	      GDclose(gdfid);
	      if(outfile_lat == NULL ) fclose(outfile_lat);
	      if(outfile_lon == NULL ) fclose(outfile_lon);
	      return(status);
	    }
	  else if(outfile_lat == NULL && 
		  (strcmp((&Com)->outFileFlag,"-b") == 0))
	    {
	      free(rows);
	      rows = NULL;
	      free(cols);
	      cols = NULL;
	      free(lons);
	      lons = NULL;
	      free(lats);
	      lats = NULL;
	      status = -1;
	      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	      HEreport("The file %s cannot be opened.\n",
		       (&Com)->outputFile_lat);
	      HEprint(stdout, 0);
	      GDdetach(gdid[is]);
	      GDclose(gdfid);
	      fclose(outfile_nxny);
	      if(outfile_lon == NULL ) fclose(outfile_lon);
	      return(status);
	    }
	  else if(outfile_lon == NULL  && 
		  (strcmp((&Com)->outFileFlag,"-b") == 0))
	    {
	      free(rows);
	      rows = NULL;
	      free(cols);
	      cols = NULL;
	      free(lons);
	      lons = NULL;
	      free(lats);
	      lats = NULL;
	      status = -1;
	      HEpush(DFE_GENAPP, "GDconvert_ij2ll", __FILE__, __LINE__);
	      HEreport("The file %s cannot be opened.\n",
		       (&Com)->outputFile_lon);
	      HEprint(stdout, 0);
	      GDdetach(gdid[is]);
	      GDclose(gdfid);
	      fclose(outfile_nxny);
	      fclose(outfile_lat);
	      return(status);
	    }
	  else
	    {
	      if(strcmp((&Com)->outFileFlag,"-b") == 0) /* binary data */
		{
		  fprintf(stdout,"The output binary files contain 64bit float data for Latitude and  Longitude of the desired grid: {n=(%d * %d) 64bit float data; First row at the beginning, Last row at the end}\n\n", ydimsize,xdimsize);

		  /* 
		     write lat/lon to binary file in dataset format:
		     (ydimsize Rows) X (xdimsize cloums)
		  */
		  status = 0;
		  fprintf(outfile_nxny,"Input file: %s\nGrid: %s\nNumber of rows in the grid = %d\nNumber of columns in the grid = %d\nThe output binary files containing latitudes and longitudes:\n\t%s\n\t%s\n\n Each file contains %d X %d of 64bit float data; First row at the beginning, Last row at the end of the file.\nProjection: %s\nProjection Parameters: ( ", (&Com)->inputFile, (&Com)->gridName, ydimsize, xdimsize, (&Com)->outputFile_lat, (&Com)->outputFile_lon, ydimsize, xdimsize,projection);
		  for( v1=0; v1<13; v1++) 
		    {
		      fprintf(outfile_nxny,"  %lf",projparam[v1]);
		    }
		  fprintf(outfile_nxny," )");
		  if(projcode == GCTP_UTM)
		    {
		      fprintf(outfile_nxny,"Zonecode: %d\n\n", zonecode);
		    }
		  else
		    {
		      fprintf(outfile_nxny,"\n\n");
		    }

		  if(status != -1 && 
		     fwrite(lats, sizeof lats[0], npnts, outfile_lat) != npnts)
		    {
		      status = -1;
		      HEpush(DFE_GENAPP,"GDconvert_ij2ll", __FILE__, __LINE__);
		      HEreport("Error writing latitudes to binary output.\n");
		      HEprint(stdout, 0);
		    }
		  if(status != -1 && 
		     fwrite(lons, sizeof lons[0], npnts, outfile_lon) != npnts)
		    {
		      status = -1;
		      HEpush(DFE_GENAPP,"GDconvert_ij2ll", __FILE__, __LINE__);
		      HEreport("Error writing longitudes to binary output.\n");
		      HEprint(stdout, 0);
		    }
		  if(status == -1)
		    {
		      fflush(outfile_nxny);
		      if(strcmp((&Com)->outFileFlag,"-b") == 0) 
			fflush(outfile_lat);
		      if(strcmp((&Com)->outFileFlag,"-b") == 0) 
			fflush(outfile_lon);
		      free(rows);
		      rows = NULL;
		      free(cols);
		      cols = NULL;
		      free(lons);
		      lons = NULL;
		      free(lats);
		      lats = NULL;
		      GDdetach(gdid[is]);
		      GDclose(gdfid);
		      if(outfile_nxny == NULL ) fclose(outfile_nxny);
		      if(outfile_lat == NULL  && 
			 (strcmp((&Com)->outFileFlag,"-b") == 0)) 
			fclose(outfile_lat);
		      if(outfile_lon == NULL  && 
			 (strcmp((&Com)->outFileFlag,"-b") == 0)) 
			fclose(outfile_lon);
		      return(-1);
		    }
		}
	      else/* ascii data to output file */
		{
		  fprintf(stdout,"Number of rows =%d\n",ydimsize);
 		  fprintf(stdout,"Number of cols =%d\n",xdimsize);
 		  fprintf(stdout,"Projection: %s\nProjection Parameters: ( ",projection);
		  for( v1=0; v1<13; v1++) 
		    {
		      fprintf(stdout,"  %lf",projparam[v1]);
		    }
		  fprintf(stdout," )");
		  if(projcode == GCTP_UTM)
		    {
		      fprintf(stdout,"Zonecode: %d\n", zonecode);
		    }
		  else
		    {
		      fprintf(stdout,"\n");
		    }

		  fprintf(stdout,"\n");
		  fprintf(stdout,"Order of paramters written to output file:\n");
		  fprintf(stdout,"row column Latitude Longitude\n");
		  fprintf(outfile_nxny,"Input file: %s\n",(&Com)->inputFile);
		  fprintf(outfile_nxny,"Grid: %s\n",(&Com)->gridName);
		  fprintf(outfile_nxny,"Number of rows =%d\n",ydimsize);
 		  fprintf(outfile_nxny,"Number of cols =%d\n",xdimsize);
 		  fprintf(outfile_nxny,"Projection: %s\nProjection Parameters: ( ",projection);
		  for( v1=0; v1<13; v1++) 
		    {
		      fprintf(outfile_nxny,"  %lf",projparam[v1]);
		    }
		  fprintf(outfile_nxny," )");
		  if(projcode == GCTP_UTM)
		    {
		      fprintf(outfile_nxny,"Zonecode: %d\n\n", zonecode);
		    }
		  else
		    {
		      fprintf(outfile_nxny,"\n\n");
		    }

		  for (k=0; k<npnts; k++)
		    {
		      fprintf(outfile_nxny,"%d %d %lf %lf\n",
			      rows[k], cols[k], lats[k], lons[k]);
		    }
		}
	    }

	  fflush(outfile_nxny);
	  if(strcmp((&Com)->outFileFlag,"-b") == 0) fflush(outfile_lat);
	  if(strcmp((&Com)->outFileFlag,"-b") == 0) fflush(outfile_lon);
	  free(rows);
	  rows = NULL;
	  free(cols);
	  cols = NULL;
	  free(lons);
	  lons = NULL;
	  free(lats);
	  lats = NULL;
	  GDdetach(gdid[is]);
	  GDclose(gdfid);
	  if(outfile_nxny == NULL ) fclose(outfile_nxny);
	  if(outfile_lat == NULL && 
	     (strcmp((&Com)->outFileFlag,"-b") == 0)) fclose(outfile_lat);
	  if(outfile_lon == NULL && 
	     (strcmp((&Com)->outFileFlag,"-b") == 0)) fclose(outfile_lon);
	  return(0);
	}
      else
	{
	  /* write output to STDOUT */
	  fprintf(stdout,"row column Latitude Longitude\n");
	  for (k=0; k<npnts; k++)
	    {
	      fprintf(stdout,"%d %d %lf %lf\n",rows[k], cols[k], 
		      lats[k], lons[k]);
	    }
	  
	  fflush(stdout);
	  free(rows);
	  rows = NULL;
	  free(cols);
	  cols = NULL;
	  free(lons);
	  lons = NULL;
	  free(lats);
	  lats = NULL;
	  GDdetach(gdid[is]);
	  GDclose(gdfid);
	  return(0);
	}
    }
}

