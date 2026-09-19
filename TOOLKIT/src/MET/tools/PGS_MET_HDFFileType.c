/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:
         PGS_MET_HDFFileType.c 
 
DESCRIPTION:
         The file contains PGS_MET_HDFFileType.
         This function is used to find out whether a file is HDF4, HDF5, 
         or non-HDF file.

AUTHOR:
        Abe Taaheri /Emergent Information Technologies, Inc.
 
HISTORY:
        
        20-Mar-2001 AT   Initial version

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/
 
#include "PGS_MET.h"
#include "PGS_CUC.h"
 
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/* hdf files */
 
#ifdef DEC_ALPHA        /* DEC Alpha platform */
#include <rpc/types.h>  /* this avoids typedef conflict with int32, uint32 */
#endif /* DEC_ALPHA */
#include "hdf.h"
#include "hdf5.h"

/***************************************************************************
BEGIN_PROLOG:
 
 
TITLE:
        This function is used to find out whether a file is HDF4, HDF5, 
         or non-HDF file.
NAME:
        PGS_MET_HDFFileType() 
 
SYNOPSIS:
        N/A
 
DESCRIPTION:
        This function is used to find the type of the file passed into the 
        function.
 
INPUTS:
        Name            Description             Units   Min     Max
        ----            -----------             -----   ---     ---
        fileName        Input file Name         none    

OUTPUTS:
        FILE_IS_HDF4    will be 1 (if TRUE), 0 (if FALSE)
        FILE_IS_HDF5    will be 1 (if TRUE), 0 (if FALSE)
        FILE_NOT_HDF    will be 1 (if TRUE), 0 (if FALSE)
 
RETURNS:
        PGS_S_SUCCESS
        PGSMET_E_HDF5_FILE_TYPE_ERROR

EXAMPLES:
        N/A
 
NOTES:

REQUIREMENTS:
        N/A
 
DETAILS:
        NONE
 
GLOBALS:
        NONE
 
FILES:
        NONE
 
FUNCTIONS_CALLED:
        Hishdf
        H5Fis_hdf5
        PGS_MET_ErrorMsg

END_PROLOG:
***********************************************************************/
PGSt_SMF_status
PGS_MET_HDFFileType(char *        fileName, /* File name */
                    PGSt_integer * FILE_IS_HDF4,
                    PGSt_integer * FILE_IS_HDF5,
                    PGSt_integer * FILE_NOT_HDF)
{

  intn                    HDF4ret;
  htri_t                  HDF5ret;
  PGSt_SMF_status         hdf5status = PGS_S_SUCCESS;
  char *                  funcName = "PGS_MET_HDFFileType";
  char *                  errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  /* Initialize file type flags to 0 */
  *FILE_IS_HDF4 = 0;
  *FILE_IS_HDF5 = 0;
  *FILE_NOT_HDF = 0;

  /* Is file HDF5 type? */
  HDF5ret = H5Fis_hdf5(fileName);
  if( HDF5ret == 0 )
    {
      /* File is not HDF5 type, is it HDF4 type ? */
      *FILE_IS_HDF5 = 0;
      HDF4ret = Hishdf(fileName);
      if( HDF4ret != 0)
	{
	  /* File is HDF4 type */
	  *FILE_IS_HDF4 = 1;
	  return(hdf5status);
	}
      else
	{
	  /* File is niether HDF5 type, nor HDF4 type */
	  *FILE_IS_HDF4 = 0;
	  *FILE_NOT_HDF = 1;
	  return(hdf5status);
	}
    }
  else if ( HDF5ret < 0 )
    {
      hdf5status = PGSMET_E_HDF5_FILE_TYPE_ERROR;
      /* error message is
	 "HDF5 failed determining file type" */
      
      (void) PGS_MET_ErrorMsg(hdf5status, funcName, 0, errInserts);
      return(hdf5status);
    }
  else
    {
      /* File is  HDF5 type */
      *FILE_IS_HDF5 = 1;
      return(hdf5status);
    }

}

