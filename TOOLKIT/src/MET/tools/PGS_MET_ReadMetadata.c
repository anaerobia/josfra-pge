
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_ReadMetadata
 
DESCRIPTION:
	Enables the user to read entire metadata (Structuremetadata, 
        coremetadata, and Archivemetadata from an HDF-EOS2 or HDF-EOS5 
        product. This file is compiled into an executable.
	
AUTHOR: 
        Abe Taaheri /Raytheon SSI

HISTORY:
        01-DEC-09      AT     Initial version 
                              (Extending GetMetadata by John Rishea)
        20-APR-10      AT     Added file header. Modfied for error handlings 

END_FILE_PROLOG
*******************************************************************************/

/* include files */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <PGS_MET.h>
#include <PGS_PC_Prototypes.h>
#include <hdf.h>
#include <mfhdf.h>
#include <hdf5.h>

PGSt_integer		FILE_IS_HDF4;
PGSt_integer		FILE_IS_HDF5;
PGSt_integer	        FILE_NOT_HDF;


#define NO_ERROR 0
#define DISPLAY_COMMAND_LINE_UASGE 1
#define ERROR_OPEN_INPUT 2
#define ERROR_OPEN_OUTPUT 3
#define ERROR_READ_INPUT 4
#define ERROR_MEMORY 5
#define WARNING 10

void CommandLineUsage();

int main( int argc, char *argv[])
{
  int status;
  intn                    HDF4ret;
  htri_t                  HDF5ret;
  PGSt_SMF_status         hdfstatus = PGS_S_SUCCESS;
  char *                  funcName = "PGS_MET_HDFFileType";
  char *                  errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};
  int errVal = NO_ERROR;
  int i, j, key, AuxKey;
  int Id[] = {1, 2, 3};
  char *options[] = {"-h", "", "-help"};
  char inputFile[256], outputFile[256]; 
  PGSt_integer sd_id;
  FILE *in_id, *out_id = NULL;
  intn trans_status;
  
  if ( argc == 1) /* display usage in case of no arguments */
    {
      /* user need help read the command line option, it is not error */
      CommandLineUsage();
      exit(DISPLAY_COMMAND_LINE_UASGE); 
    }
  
  /* read command line parameters */
  key=4;
  for(j=0; j < 3; j++)
    {
      if (strcmp(argv[1], options[j]) == 0)
	key = Id[j];
    }
  switch(key)
    {
    case 1:
    case 2:
    case 3:
      CommandLineUsage();
      exit(DISPLAY_COMMAND_LINE_UASGE);
      break;
    case 4:
      if(argc <3)
	{
	  CommandLineUsage();
	  exit(DISPLAY_COMMAND_LINE_UASGE);
	}
      else
	{
	  strcpy(inputFile, argv[1]);
	  strcpy(outputFile, argv[2]);
	}
      break;
    }

  
  /* We just want to write metadata to an ascii file. So first we determine
     whether the file is HDF or not */
  
  
  if((in_id = fopen(inputFile, "r")) == NULL)
    {
      fprintf(stderr,"Error: Failed opening input file %s.\n",inputFile);
      exit(ERROR_OPEN_INPUT);
    }
  else
    {
      fclose(in_id );
    }

  /* Open the output file for reading */
  /* If it exist we will not overwight */
  out_id = fopen(outputFile, "r");
  if ( out_id != NULL )
    {
      fprintf( stderr, "Error: The ouput file %s already exist. Use a new filename.\n",outputFile );
      exit( ERROR_OPEN_OUTPUT);
    }

  if((out_id = fopen(outputFile, "w")) == NULL)
    {
      fprintf(stderr,"Error: Failed opening output file %s for writing.\n",outputFile);
      exit(ERROR_OPEN_OUTPUT);
    }
  else
    {
      fclose(out_id );
    }


  /* Initialize file type flags to 0 */
  FILE_IS_HDF4 = 0;
  FILE_IS_HDF5 = 0;
  FILE_NOT_HDF = 0;
  
  /* Is file HDF5 type? */
  HDF5ret = H5Fis_hdf5(inputFile);
  if( HDF5ret == 0 )
    {
      /* File is not HDF5 type, is it HDF4 type ? */
      FILE_IS_HDF5 = 0;
      HDF4ret = Hishdf(inputFile);
      if( HDF4ret != 0)
	{
	  /* File is HDF4 type */
	  FILE_IS_HDF4 = 1;
	}
      else
	{
	  /* File is niether HDF5 type, nor HDF4 type */
	  FILE_IS_HDF4 = 0;
	  FILE_NOT_HDF = 1;
	}
    }
  else if ( HDF5ret < 0 )
    {
      fprintf(stderr,"Error: Failed determining file type as HDF5.\n");
      exit(PGSMET_E_HDF5_FILE_TYPE_ERROR);
    }
  else
    {
      /* File is  HDF5 type */
      FILE_IS_HDF5 = 1;
    }


  if(FILE_NOT_HDF == 1)
    {
      fprintf(stderr,"Error: Input file is niether HDF5 type, nor HDF4 type.\n");
      exit(PGSMET_E_HDF5_FILE_TYPE_ERROR);
    }
  
  if(FILE_IS_HDF5 == 1 || FILE_IS_HDF4 == 1 )/* do the following only if the output file is hdf */
    {
      /* Open the HDF file for reading */
      if(FILE_IS_HDF4 == 1 ) hdfstatus = PGS_MET_SDstart( inputFile, DFACC_RDONLY,  &sd_id);
      if(FILE_IS_HDF5 == 1 ) hdfstatus = PGS_MET_SDstart( inputFile, H5F_ACC_RDONLY, &sd_id);
      
      if ( hdfstatus == -1 )
	{
	  fprintf( stderr, "Error: unable to open file %s \n", inputFile );
	  exit( ERROR_OPEN_INPUT);
	}
    }

  /* Open the output file for writing */
  out_id = fopen(outputFile, "w");
  if ( out_id == NULL )
    {
      /* first close the input file */
      PGS_MET_SDend(sd_id);
      fprintf( stderr, "Error: unable to open ouput file %s \n", outputFile );
      exit( ERROR_OPEN_OUTPUT);
    }


    printf("Getting Metadata. Please wait ........\n");


  /* transfer metadata from input HDF file to output file */
  trans_status = GetMetadata( sd_id, out_id);
  
  /* if the read was unsuccessful, return an error */
  if ( trans_status == -1 )
    {
      /* first close the input and output files */
      status = PGS_MET_SDend(sd_id);
      status = fclose( out_id );
      
      fprintf( stderr, "Error: Unable to get metadata from\n" );
      fprintf( stderr, "%s to %s\n", inputFile, outputFile );
      exit( trans_status );
    }
  
  /* close the input HDF file */
  status = SDend( sd_id );
  
  /* close the output file */
  status = fclose( out_id);

  printf(".....Done!\n");

  return errVal;
}


void CommandLineUsage()
{
  fprintf( stderr, "Usage: DumpMetadata <input_HDF_file_name> <output_ASCII_file_name>\n");
}


/*************************************************************************

MODULE: GetMetadata

PURPOSE: 
    This module sends requests to GetAttr to read user-defined core,
    archive, and structural metadata and write it to an ASCII output
    file.
 

RETURN VALUE:

Value    	Description
-----		-----------
NO_ERROR	Returns NO_ERROR if the metadata reading was
                successful.
4               Returns ERROR_READ_INPUT if the HDF file can't be
                read.

HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
           02/01    John Rishea              Original Development based on
                                             Doug Ilg's metadmp.c program.
           11/08    Abe Taaheri              Modified for this utility

NOTES:

**************************************************************************/

intn GetMetadata
(
 PGSt_integer sd_id,	/* the file id for input HDF file */ 
 FILE *out_id	        /* the file id for ouput file */ 
 )
  
{
  intn i;                /* used to loop through attrs */
  intn j;
  intn status;           /* receives result of processing
			    from Getattr function */     
  char attrname[256];    /* holds the file_name string */ 
  char *root = NULL;     /* will point to attr type */
  
  /* set up a loop to check for and read the three
     types of metadata -- core, archive, and structural */
  
  for ( i = 0; i < 3; i++ )
    {
      /* set up root to hold correct string */ 
      switch( i )
        {
	case 0:
	  root = "StructMetadata";
	  break;
	case 1:
	  root = "CoreMetadata";
	  break;
	case 2:
	  root = "ArchiveMetadata";
	  break;
        }  
      
      /* First, try root name alone */
      status = GetAttr( sd_id, out_id, root );
      
      /* Now, try concatenating sequence numbers */
      for ( j = 0; j <= 9; j++ )
        {
	  sprintf( attrname, "%s.%d", root, j );
	  status = GetAttr( sd_id, out_id, attrname );
        }
      
    }  /* end outer for loop */
  
  /* return the value of the function to main function */
  return ( status );
  
}  /* end GetMetadata function */


/*************************************************************************

MODULE: GetAttr

PURPOSE: 
    This module reads the user-defined core, archive, and structural metadata
    from the original (input) HDF file and writes it as new user-defined
    ASCII metadat file.  

RETURN VALUE:
Type = intn
Value    	Description
-----		-----------
NO_ERROR	Returns NO_ERROR if the metadata writing operation was
                successful.
5         	Returns ERROR_MEMORY if mem allocation fails.
4               Returns ERROR_READ_INPUT if the HDF file can't be
                read from.


HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
           02/01    John Rishea              Original Development based on
                                             Doug Ilg's metadmp.c program.
           11/08    Abe Taaheri              Modified for this utility adding
                                             HDF5 reading
NOTES:

**************************************************************************/
intn GetAttr
(
 PGSt_integer fid_in   ,	/* the file id for input HDF file */ 
 FILE *fid_out,	        /* the file id for ouput file */
 char *attr		        /* the filename handle of the attribute to move */ 
 )
  
{
  intn status = NO_ERROR; /* this is the var that holds return val */
  int32 my_attr_index;    /* holds return val from SDfindattr */ 
  int32 data_type;	    /* holds attribute's data type */ 
  int32 n_values;         /* stores # of vals of the attribute */
  char *file_data = NULL,                 /* char ptr used to allocate temp space
					   * during transfer of attribute info */
    attr_name[MAX_NC_NAME],            /* holds attribute's name */
    new_attr_name[MAX_NC_NAME + 3];    /* holds new attr name */
  PGSt_SMF_status    retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
  herr_t             HDF5status = -1;
  hid_t              datid;
  hid_t              ggid;
  hid_t              atype;
  hid_t              HDFfid;
  size_t             size;

  if(FILE_IS_HDF4 == 1) /* if input file is HDF4 type */
    {
      /* look for attribute in the HDF file */
      my_attr_index = SDfindattr( fid_in, attr );
      
      /* only proceed if the attribute was found */
      if ( my_attr_index == -1 )
	{
	  return( WARNING);
	}
      
      /* get size of HDF file attribute  */
      status = SDattrinfo( fid_in, my_attr_index, attr_name, &data_type, &n_values );
      
      /* only proceed if successful read of attribute info */
      if ( status == -1 )
	{
	  return( ERROR_READ_INPUT);
	} 
      
      /* attempt to allocate memory for HDF file attribute contents */
      file_data = ( char * ) calloc( (n_values+1), sizeof(char) );
      
      if ( file_data == NULL )
	{
	  fprintf( stderr, "Error: Unable to allocate %d bytes for %s\n",
		   n_values, attr );
	  return ( ERROR_MEMORY );
	}
      
      /* read attribute from the HDF file */
      status = SDreadattr( fid_in, my_attr_index, file_data );
      if(status == -1 )
	{
	  /* first free the allocated memory */
	  free( file_data );
	  return( ERROR_READ_INPUT);
	}
      
      /* attempt to write metadata to output HDF file */
      status = fprintf( fid_out, "%s\n",file_data);
      
      /* free dynamically allocated memory */
      free( file_data );
    }
  else if (FILE_IS_HDF5 ==1) /* input file is HDF5 type */
    {
      HDFfid =(hid_t) fid_in;
      retVal = NO_ERROR;
      
      /* probe: open group "HDFEOS INFORMATION" */
      HDF5status = H5Eset_auto(NULL, NULL);
      ggid = H5Gopen(HDFfid,"HDFEOS INFORMATION");
      if (ggid == -1)
	{
	  fprintf( stderr, "Error: Cannot open \"HDFEOS INFORMATION\" group in input HDF file.\n");
	  return( ERROR_READ_INPUT);
	}
      
      datid = H5Dopen(ggid, attr);

      /* only proceed if the attribute was found */
      if(datid == -1)
	{
	  return( WARNING);
	}
      
      /*retVal = PGS_MEM_Calloc((void **)&file_data, 
	(PGSt_integer)(MAX_ORDER + 1), sizeof(char));*/
      
      file_data  = ( char * ) calloc( (MAX_ORDER+1), sizeof(char) );
      if(retVal != SUCCEED)
	{
	  n_values=MAX_ORDER;
	  fprintf( stderr, "Error: Unable to allocate %d bytes for %s\n",
		   n_values, attr );
	  return ( ERROR_MEMORY );
	}
      
      atype   = H5Tcopy(H5T_C_S1);
      size = (size_t)MAX_ORDER;
      status  = H5Tset_size(atype,size);
      if (status == -1)
	{
	  fprintf(stderr, "Error: Cannot set the total size for atomic datatype.");                       
       	  return( ERROR_READ_INPUT);
	}
      HDF5status = H5Dread(datid,atype,H5S_ALL,H5S_ALL,H5P_DEFAULT,file_data);
      if(HDF5status != SUCCEED)
	{
	  /* first free the allocated memory */
	  (void) H5Dclose(datid);
	  (void) H5Gclose(ggid);
	  free( file_data );
	  return( ERROR_READ_INPUT);
	}
      
      /* attempt to write metadata to output HDF file */
      status = fprintf( fid_out, "%s\n",file_data);
      
      /* free dynamically allocated memory */
      (void) H5Dclose(datid);
      (void) H5Gclose(ggid);
      free( file_data );
      status = SUCCEED;
    }
  
  /* return the value of status to GetMetadata */
  return ( status );
}

