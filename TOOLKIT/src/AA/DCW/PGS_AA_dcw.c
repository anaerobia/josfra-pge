/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_AA_dcw.c

DESCRIPTION:
	This file contains the toolkit routines and support functions to access
  	land/sea flag values from the DCW database. 
	The following values are retrieved from the data base.

	Value	  =	Surface Cover
	-----		-------------
       -1	  = 	No Data From DCW data base
	1	  =	Land
	2	  =	Open Ocean
	3	  =	Polar Ice
	4	  = 	Pack Ice
	5	  =	Shelf Ice	

AUTHORS:
	Richard Morris /EOSL
	Jolyon Martin / EOSL

HISTORY:
	08/07/94	RM/JM	Initial version
	08/23/94	RM	Code Inspection updates
	08/31/94	RM	Comments added
	09/02/94	RM/JM	Bug Fixing
	10/11/94	RM	Error Check Added
	29-JUN-95	ANS	changed PGS_AA_DCW function name to PGS_AA_dcw
	30-JUN-95	MES	changed PGS_AA_DCW.c file name to PGS_AA_dcw.c
	11-July-95	ANS	Improved fortran example in the prolog
        06-July-99      SZ      Updated for the thread-safe functionality

END_FILE_PROLOG:
***************************************************************************/

/*---- includes ----*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cfortran.h>
/*#include <PGS_AA_DCW.h>*/
#include <PGS_AA_DCW.h>


/*---- DCW includes ----*/

#include <vpftable.h>
#include <coorgeom.h>
#include <vpfprim.h>
#include <vvmisc.h>
#include <strfunc.h>
#include <vpfsprel.h>
#include <vpfprim.h>
#include <PGS_TSF.h>

/*---- mnemonics ----*/

/*---- global variables ----*/

/*---- function declarations ----*/

PGSt_SMF_Status
PGS_AA_DCW_Parm(char *parms[], PGSt_integer nParms, 
			char *dcwParms[], char *dcwLayer[], char *dcwFFile[]);
PGSt_SMF_Status
PGS_AA_DCW_Intile(char *dcwLayer[], PGSt_integer nParms, 
			PGSt_double longitude, PGSt_double latitude, 
			char *continent, char *tiledir, PGSt_integer record[]);
PGSt_SMF_Status
PGS_AA_DCW_Inface(char *dcwLayer[], PGSt_integer nParms, 
			PGSt_double longitude, PGSt_double latitude, 
			char *continent, char *tiledir, PGSt_integer record[]);

PGSt_SMF_Status
PGS_AA_DCW_Feature( char *dcwParms[], char *dcwLayer[], char *dcwFFile[], 	
			PGSt_integer nParms,
			char *continent, char *tiledir, 
			PGSt_integer row[], PGSt_integer results[]);




/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Obtain a value representing surface cover for the location
	supplied by the user.
 
NAME:
	PGS_AA_dcw

SYNOPSIS:
C:
	#include <PGS_AA.h>
	PGSt_SMF_Status
	PGS_AA_dcw ( char iparms[][100],	 coverage name - PO 
		     PGSt_integer nParms,	 number of coverages 
		     PGSt_double longitude[],	 longitude of point(s)
		     PGSt_double latitude[],	 latitude of point(s) 
		     PGSt_integer npoints,	 number of points 
		     void *results)		 result of search 

FORTRAN:
	include	'PGS_AA_10.f'

	IMPLICIT NONE
	INTEGER PGS_AA_dcw(parms, nParms, latitude, longitude, npoints, results)
		   character*100 iparms(*),
		   integer 	nParms,
		   double 	latitude(*),
		   double 	longitude(*),
		   integer 	npoints,
		   'user spec'd' results (SEE NOTES)
	

DESCRIPTION:
	This routine receives either a single point or an array of location points
	and navigates the DCW database in order to find the coverage which the 
	user supplies as parm.  Once the coverage is identified, the database
	path is updated, with each file and table identified, until the table
	containing the locational information is located.  Once this table is
	found, the table is opened and the result for a latitude/longitude is
	extracted and returned in results.

	[start]
	PERFORM		PGS_AA_DCW_Parm
	PERFORM		PGS_AA_DCW_Intile
	PERFORM		PGS_AA_DCW_Inface
	PERFORM		PGS_AA_DCW_Feature
	PERFORM		return PGS_S_SUCCESS

	[end]
	
	

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	parms		parameter wanted	N/A	N/A	N/A
	
	nParms		number of parameters	N/A	1	1
			
	latitude	latitude location	degrees	-90.0	90.0

	longitude	longitude location	degrees	-180.0	180.0
	
	npoints		number of points	N/A	0	Unlimited
	
	

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	results		extracted parameter	user    N/A	N/A

RETURNS:
	PGS_S_SUCCESS			succesful return
	PGSAA_E_DCW_ERROR		error in extracting value required
	PGSAA_W_DCW_NODATA		no data at that point in data base

	The following errors are reported to the error log

	PGSAA_E_CANT_FIND_PARM		
	PGSAA_E_CANT_GET_DATABASE_PATH	
	PGSAA_E_CANT_GET_AFT_PATH
	PGSAA_E_CANT_FIND_FACE
	PGSAA_E_CANT_GET_POINT_INFO
	PGSAA_E_NPOINTSINVALID
	PGSAA_E_NPARMSINVALID
	
EXAMPLES:
C:
	
	
	PGSt_double latitude[2] = {-9.29, -25.34};
	PGSt_double longitude[2] = {110.3, 30.9};
	PGSt_integer results[2];
	char *parm[] = {"po", NULL};
		
	ret_status = PGS_AA_dcw(parm, 1, longitude, latitude, 2, results);

	
	
FORTRAN:
	IMPLICIT      NONE
	integer	      pgs_aa_dcw
	character*100 parms(4)
	integer	    nParms(2)
	double	    latitude(2)
	double 	    longitude(2)
	integer	    npoints(2)
	integer     result(2)
	parms(1)= "po"
	nParms = 2
	latitude = -9.29, -25.34
	longitude = 110.3, 30.9
	npoints = 2

	call pgs_aa_dcw(parms, nParms, longitude, latitude, npoints, results)
	

NOTES:
	For further details of the background to this tools and 
        the available data sets, support files and the means by
        which new data sets can be introduced, see the Ancillary
        Data tools Primer.  The primer also includes details of
        the operations which can be set by the user.
	
	IMPORTANT:- The PGS_AA_dcw code calls a number of library
	modules, which carry out such actions as mallocing
	memory for files, opening files, opening tables, reading tables, 
	extracting information from tables and closing tables.  These
	library modules are detailed in the DCW format spec and the associated
	VPF library software.

	NOTE:  Precision of latitude and longitude is machine specific, not 
	data-base specific.


	
REQUIREMENTS:
	PGSTK-0870, PGSTK-0840 PGSTK-1360 PGSTK-1362

DETAILS:
	N/A

GLOBALS:
	None.
FILES:
	DCW data base will be previously staged, and the directory
	path specified in the Process Control Files.
	
FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()
	PGS_PC_GetPCSData()
	PGS_AA_DCW_Parm()
	PGS_AA_DCW_Intile()
	PGS_AA_DCW_Inface()
	PGS_AA_DCW_Feature()
	


END_PROLOG:
*****************************************************************************/
PGSt_SMF_Status		ret_status;
PGSt_SMF_Status		err_ret_status;

PGSt_SMF_Status
PGS_AA_dcw ( char iparms[][100],	/* coverage name - PO */
		     PGSt_integer nParms,	/* number of coverages */
		     PGSt_double longitude[],	/* longitude of point(s) */
		     PGSt_double latitude[],	/* latitude of point(s) */
		     PGSt_integer npoints,	/* number of points */
		     void *results)		/* result of search */

{
	/*--- local variables --- */

	char 	*parms[PGSd_AA_MAXNOPARMS];	/* parameter input */
	char	*dcwParms[PGSd_AA_MAXNOPARMS];	/* map parm to dcw parm */
	char 	*dcwLayer[PGSd_AA_MAXNOPARMS];	/* map parm to dcw layer */
	char 	*dcwFFile[PGSd_AA_MAXNOPARMS];	/* map parm to feature file */
	char  	tiledir[255];  			/* tile directory */ 
	char	continent[255];			/* continent directory */
	PGSt_integer rec[PGSd_AA_MAXNOPARMS];	/* record in feature file */
	PGSt_integer i;				/* loop */
	PGSt_integer *locres;			/* pointer for results */
	PGSt_double  	loclat, loclon;		/* local copies of the point */
	PGSt_integer	ntries, done;		/* retry variables */
	PGSt_integer 	nodata;			/* error store */
	
	for (i=0; i<nParms; i++)
	{
		parms[i] = iparms[i];		/* To keep fortran happy */
	}

	nodata=FALSE;

	locres = (PGSt_integer *)results;

	ret_status = PGS_AA_DCW_Parm( parms, nParms, 
			dcwParms, dcwLayer, dcwFFile);

	if (ret_status != PGS_S_SUCCESS)
	{
		err_ret_status = PGS_SMF_SetStaticMsg ( 
				PGSAA_E_CANT_FIND_PARM, 
				"PGS_AA_DCW_Parm");
		return PGSAA_E_DCW_ERROR;
	}
	if (npoints <= 0 )
	{
		err_ret_status = PGS_SMF_SetStaticMsg ( 
				PGSAA_E_NPOINTSINVALID, 
				"PGS_AA_dcw");
		return PGSAA_E_DCW_ERROR;
	}

	
	/* begin looping through the number of points */

	for (i = 0; i < npoints; i++)	
	{
		loclat = latitude[i];
		loclon = longitude[i];

		/* 
		 *  there is a special case in which the algorithm
		 *  may fail to find a point within the database:
		 *  this is when the data point lies exactly on one
		 *  of the data points at the first or final node in
		 *  the edge of a polygon:  in this case modify the 
		 *  input data by an ammount that is less than the
		 *  precision of the data set to guarentee that it
		 *  falls off this point.  The error that is introduced
		 *  here is not significant compared to the overall
		 *  accuracy of the database
		 */

		done = FALSE;
		ntries = 0;
		while (done == FALSE)
		{

			ret_status = PGS_AA_DCW_Intile(dcwLayer, nParms,
				loclon, loclat, continent, tiledir, rec);
			if (ret_status == PGSAA_W_DCW_NODATA)
			{
				if (ntries < PGSd_DCW_MAXTRIES)
				{
					/*
					 *  retry with a small shift to
					 *  the data point to anticipate
					 *  problems described above
					 */

					ntries++;
					loclon += 0.00000001;
					loclat += 0.00000001;
				}
				else
				{
					/*
					 *  give up
					 */

					done = TRUE;
				}
			}
			else
			{
				done = TRUE;
			}
			
		}


		switch (ret_status) {
		case PGS_S_SUCCESS:
			/*
			 * continue execution
			 */
			break;
		case PGSAA_W_DCW_NODATA:
			/* 
			 * continue execution,
			 * but remember that there are potential
			 * problems with some of the data points
			 */ 
			nodata = TRUE;
			break;
		default:
			/*
			 * can not handle other errors
			 */
			err_ret_status = PGS_SMF_SetStaticMsg( 
				PGSAA_E_CANT_FIND_FACE, "PGS_AA_DCW_Intile");
			return PGSAA_E_DCW_ERROR;
		}

		ret_status = PGS_AA_DCW_Feature(dcwParms, dcwLayer, dcwFFile, 
				nParms, continent, tiledir, rec, locres);

		if (ret_status != PGS_S_SUCCESS)
		{
			err_ret_status = PGS_SMF_SetStaticMsg ( 
				PGSAA_E_CANT_GET_POINT_INFO,
				"PGS_AA_DCW_Feature");
			return PGSAA_E_DCW_ERROR;
		}

		locres+=nParms;
      	}

	/* return error if there was missing data during the routine */

	if (nodata)
	{
		err_ret_status = PGS_SMF_SetStaticMsg(
				PGSAA_W_DCW_NODATA, "PGS_AA_dcw");
		return PGSAA_W_DCW_NODATA;
	}
	else
	{
		err_ret_status = PGS_SMF_SetStaticMsg(
				 PGS_S_SUCCESS, "PGS_AA_dcw");
		return PGS_S_SUCCESS;
	}
}


/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Check the parameter supplied by the user, concatenate the parm to the directory structure
 
NAME:
	PGS_AA_DCW_Parm

SYNOPSIS:
C:
	PGSt_SMF_Status
	PGS_AA_DCW_Parm ( char *parms[],
			  PGSt_integer nParms
			  char *dcwParms[],
			  char *dcwLayer[],
			  char *dcwFFile[])

FORTRAN:
	N/A
	

DESCRIPTION:
	This routine checks the parm supplied by the user, if it doesnt match
	the parm allowed, an error is returned, otherwise the parm is
	passed to PGS_AA_DCW_Intile.

	[start]
	
	IF		parm is not equal to "po"
	THEN
	PERFORM		PGS_SMF_SetStaticMsg
	ELSE		pass Parm to PGS_AA_DCW_Intile
	[end]


INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	parms		parameter wanted	N/A	N/A	N/A	
			
	nParms		number of parms		N/A	1	1
	
	dcwParms	DCW table parm		N/A	N/A	N/A
	
	dcwLayer	parm			N/A	N/A	N/A

	dcwFFile	area feature table	N/A	N/A	N/A	
	
	

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	dcwFFile	coverage path		N/A	N/A	N/A

RETURNS:
	PGS_S_SUCCESS			succesful return
	PGSAA_E_DCW_ERROR		error in extracting value required

	The following errors are reported to the error log

	PGSAA_E_CANT_FIND_PARM		parm supplied by user is not correct
	PGSAA_E_NPARMSINVALID		nParms 0 or less
	
EXAMPLES:
C:	N/A
	
FORTRAN:
	N/A
	
NOTES:
	N/A

REQUIREMENTS:
	N/A

DETAILS:
	N/A

GLOBALS:
	None

FILES:
	None

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()


END_PROLOG:
*****************************************************************************/
PGSt_SMF_Status
PGS_AA_DCW_Parm (char *parms[], PGSt_integer nParms, 
			char *dcwParms[], char *dcwLayer[], char *dcwFFile[])
{
	/* at the moment only one parameter is supported in the tool */
	/* this may be extended, in which case this routine is the   */
	/* only code that it should be necessary to modify 	     */

	char *AAparms[] = {"po"};
	char *DCWparms[] = {"POPYTYPE"};
	char *DCWffile[] = {"poarea.aft"};
	char *DCWlayer[] = {"po/"};
	PGSt_integer ndcwparms = 1;
	PGSt_integer i, j, found;

	/* does parms supplied by user match those allowed by program */

	/* first check to see if nParms is greater than 0 */

	if(nParms <= 0)
	{
		err_ret_status = PGS_SMF_SetStaticMsg(
				PGSAA_E_NPARMSINVALID, "PGS_AA_DCW_PARM");
		return PGSAA_E_DCW_ERROR;
	}

	for (i=0;i<nParms;i++)
	{
		found = FALSE;
		for (j=0;j<ndcwparms;j++)
		{
			if (strcmp(parms[i],AAparms[j])==0)
			{
				found = TRUE;
				dcwParms[i] = DCWparms[j];
				dcwFFile[i] = DCWffile[j];
				dcwLayer[i] = DCWlayer[j];
			}
		}

		if (found == FALSE)
		{
			err_ret_status = PGS_SMF_SetStaticMsg (
				PGSAA_E_CANT_FIND_PARM, "PGS_AA_DCW_Parm");
			return PGSAA_E_DCW_ERROR;
		}
	}
	return PGS_S_SUCCESS;
}


	
/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Retrieve the start point database path, locate the continent
	which holds the search point
 
NAME:
	PGS_AA_DCW_Intile

SYNOPSIS:
C:
	PGSt_SMF_Status
	PGS_AA_DCW_Intile( char *dcwLayer[],
			   PGSt_integer nParms,
			   PGSt_double longitude,
			   PGSt_double latitude,
			   char *continent, 
			   char *tiledir,
			   PGSt_integer record[])
	

FORTRAN:
	N/A
	

DESCRIPTION:
	This routine uses the PC_GetData function to extract the database
	path from the PC files.  Once it has done this, each point is 
	located within one of the four DCW continent divisions, the name
	of this division is concatenated onto the database path.

	[start]
	PERFORM		PGS_PC_GetPCSData
	CALCULATE	open Face Bounding Rectangle file, to find which continent the
	 		point is situated within.  Once found this file is concatenated onto 
			the directory path  which was the output from 				
			PGS_PC_GetPCSData.
	PERFORM		return /path/continent;
	
	[end]


INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---

	dcwLayer	parm 			N/A	N/A	N/A

	nParms		number of parms		N/A	1	1

	longitude	longitude position	degrees	-90.0	90.0		
			
	latitude	latitude position	degrees	-180.0	180.0
	
	continent	database path		N/A	N/A	N/A

				

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	tiledir		database path		N/A	N/A	N/A
	
	record		record number		N/A	N/A	N/A


RETURNS:
	PGS_S_SUCCESS			succesful return
	PGSAA_E_DCW_ERROR		error in extracting value required

	The following errors are reported to the error log

	PGSAA_E_CANT_GET_DATABASE_PATH		no path available from PC file.
	PGSAA_E_CANT_GET_AFT_PATH		path to area feature table not found
        PGSTSF_E_GENERAL_FAILURE                problem in the thread-safe code
	
EXAMPLES:
C:	N/A
	
FORTRAN:
	N/A

NOTES:
	N/A

REQUIREMENTS:
	N/A

DETAILS:
	N/A

GLOBALS:
	None

FILES:
	None

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()
	PGS_PC_GetPCSData()
	PGS_AA_DCW_Inface()
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

	DCW library Functions
	---------------------
	vpftable()
	vpfprim()
	vpfprim2()
	vvselec()
	vpfsprel()
	vpfproj()
	vvmisc()

END_PROLOG:
*****************************************************************************/


PGSt_SMF_Status
PGS_AA_DCW_Intile(char *dcwLayer[], PGSt_integer nParms, 
			PGSt_double longitude, PGSt_double latitude,
			char *continent, char *tiledir, PGSt_integer record[])
{

   int tilenum, facenum;	/* tile number, and face number	*/		
   int TilePATH_;		/* tile path number, face id number */	

   extent_type tileExtent;	/* tile extent */			
   char  *buf;			/* temporary buffer */
   vpf_table_type tileTable, tileFbr; /* tile table, face bounding rectangle */
   row_type row;		/* row number */
   int n;			/* loop */
   
   char path[255];
   char libpath[255];
   PGSt_integer numfiles = 1;	/* numfiles is tk2 delivery to remove warning redefined as int instead of PGST_integer */
   PGSt_integer  i;

#ifdef _PGS_THREADSAFE
   PGSt_SMF_status retTSF;      /* lock and unlock return */
#endif
	/* 
	   the following sections of code surrounded by the DCWPO
	   directives are redudnent for access to just the
	   the political oceans layer, so is removed
	   for efficiency, if however other layers are
           interfaced the code should be uncommented
	*/
	for (i=0; i<nParms;i++)
		record[i] = -1;


	/* loop through continents between europe/north asia and 
		south asia/australia */		
	for ( i = PGSd_AA_EURNASIA; i <= PGSd_AA_SASAUS; i++)	
	{ 
		ret_status = PGS_PC_GetPCSData( PGSd_PC_INPUT_FILE_NAME,	
				i,
				libpath,
				&numfiles);

		if (ret_status != PGS_S_SUCCESS)
		{
			err_ret_status = PGS_SMF_SetStaticMsg ( 
				PGSAA_E_CANT_GET_DATABASE_PATH, 
				"PGS_PC_GetPCSData");
			return PGSAA_E_DCW_ERROR;
		}

      		sprintf(path,"%stileref%cfbr",libpath,'/');

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (VPF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKVPF);  
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

      		tileFbr = vpf_open_table(path,disk,"rb",NULL);

		/* For description of tilere.aft file, see DCW Documentation */
   		sprintf(path,"%stileref%ctileref.aft",libpath,'/');
   		if (fileaccess(path,0)!=0) 
		{
			err_ret_status = PGS_SMF_SetStaticMsg ( 
				PGSAA_E_CANT_GET_AFT_PATH, 
				"PGS_DCW_AA_Intile");

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had  
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKVPF);
#endif

			return PGSAA_E_DCW_ERROR;
		}			
		
      		tileTable = vpf_open_table(path,disk,"rb",NULL);
      		TilePATH_ = table_pos("TILE_NAME",tileTable);


      		/* Find the tile containing the search point */
      		/* check first row in face bounding rectangle table - for 
			bounding box of whole continent */

      		tileExtent = read_bounding_rect( 1, tileFbr, (int (*)(double *, double *))NULL );
      		if (fwithin(longitude,latitude,tileExtent))
      		{

		    /* continue looping to find the tile containing the 
				point within continent */

		    for (tilenum = 2; tilenum <= tileFbr.nrows; tilenum++) 
      		    {
          		row = read_row(tilenum,tileTable);

#ifdef DCWPO
                        /*face id number */	
                        int FAC_ID_;

                        FAC_ID_ = table_pos("FAC_ID",tileTable);

         		if (FAC_ID_ >= 0) 
         		{
            			if (tileTable.header[FAC_ID_].type == 'I') 
            			{
             			 	get_table_element(FAC_ID_, row, 
						tileTable, &facenum, &n);
           			} 
           			else if (tileTable.header[FAC_ID_].type == 'S') 
           			{
              			 	get_table_element(FAC_ID_, row, 
						tileTable, &sval, &n);
               				facenum = (int)sval;
            			} 
            			else facenum=0;  
         		} 
        		else 
         		{
           		 	facenum = tilenum;
         		}
#else
			facenum = tilenum;
#endif

         		tileExtent = read_bounding_rect(facenum, 
					tileFbr, (int (*)(double *, double *))NULL );
 
         		if (fwithin(longitude,latitude,tileExtent)) 
        		{


				/* if point wanted within actual tile */
				/* extract tilepath */

           			buf = (char *)get_table_element( TilePATH_, 
						row, tileTable, NULL, &n);
            			strcpy(tiledir,buf);
         			rightjust(tiledir);
         			n = strlen(tiledir);
           			tiledir[n+1] = '\0';
           			tiledir[n] = '/';

	   			strcpy(continent, libpath);

				/* see if we can locate inside a face */

				ret_status = PGS_AA_DCW_Inface (
					dcwLayer, nParms, 
					longitude, latitude, 
					continent, tiledir, record);
				if (ret_status == PGS_S_SUCCESS)
				{
					/* found the point so jump out */

		         		free_row(row,tileTable);
      					vpf_close_table(&tileFbr);
      					vpf_close_table(&tileTable);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKVPF);
#endif
	    				return PGS_S_SUCCESS;
				}

				/* otherwise keep looping */
         		}
		        free_row(row,tileTable);
		    }
      		}
      		vpf_close_table(&tileFbr);
      		vpf_close_table(&tileTable);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKVPF);
#endif

  	 }

	err_ret_status = PGS_SMF_SetStaticMsg( 
		PGSAA_W_DCW_NODATA, "PGS_AA_DCW_Intile");
	return PGSAA_W_DCW_NODATA; 
}




/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Retrieve the tile information 
	for the point requested by the user.
 
NAME:
	PGS_AA_DCW_Inface

SYNOPSIS:
C:
	PGSt_SMF_Status
	PGS_AA_DCW_Inface( char dcwLayer,
			   PGSt_ineteger nParms,
			   PGSt_double longitude,
			   PGSt_double latitude,
			   char *continent, 
			   char *tiledir,
			   PGSt_integer record[])
	

FORTRAN:
	None.
	

DESCRIPTION:
	This routine searches through the edge table to find the exact position
	of the point in relation to the edges which surround it.  It then locates
	row which contains that search point.

	[start]
	CALCULATE	which tile the ensuing point is within, by opening the .rng, .fac and .edg.
	CALCULATE	which face primitive the point is within
	PERFORM		return the row within the face
	[end]

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	
	dcwLayer	parm			N/A	N/A	N/A
	
	nParms		number of parms		N/A	1	1

	longitude	longitude position	degrees	-90.0	90.0		
			
	latitude	latitude position	degrees	-180.0	180.0
	
	continent	database path		N/A	N/A	N/A

				

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	tiledir		database path		N/A	N/A	N/A
	
	record		record number		N/A	N/A	N/A


RETURNS:
	PGS_S_SUCCESS			succesful return
	PGSAA_W_DCW_NODATA		warning no data at point

	
EXAMPLES:
C:
	
	N/A
	
FORTRAN:
	N/A

NOTES:
	N/A

REQUIREMENTS:
	N/A

DETAILS:
	N/A

GLOBALS:
	None.

FILES:
	EDG, FAC, RNG

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()

	DCW library Functions
	---------------------
	vpftable()
	vpfsprel()
	vvmisc()
	


END_PROLOG:
*****************************************************************************/

PGSt_SMF_Status
PGS_AA_DCW_Inface(char *dcwLayer[], PGSt_integer nParms, 
		PGSt_double longitude, PGSt_double latitude, 
		char *continent, char *tiledir, PGSt_integer record[])
{

   	int i, p;
   	char tilepath[255];		/* continet + layer + tiledir */
   	char path[255];			/* tiledir + vpftable */
   	vpf_table_type fac, rng, edg;

	ret_status = PGS_S_SUCCESS;	/* innocent until proven guilty */

	for (p=0;p<nParms;p++)
	{
      		strcpy(tilepath, continent);
		vpfcatpath(tilepath, dcwLayer[p]);
      		vpfcatpath(tilepath, tiledir);

		/* Open up the face, ring and edge tables */

      		strcpy(path,tilepath);
      		vpfcatpath(path,"FAC");
      		fac = vpf_open_table(path,disk,"rb",NULL);
      		strcpy(path,tilepath);
      		vpfcatpath(path,"RNG");
      		rng = vpf_open_table(path,disk,"rb",NULL);
		strcpy(path,tilepath);
      		vpfcatpath(path,"EDG");
      		edg = vpf_open_table(path,disk,"rb",NULL);

	/* decide whether to loop at 1 which gives bounds for whole tile */
        /* or at 2 which is the start of the actual faces */
	/* Value 2 was chosen so that the point could be located */
	/* within the faces */

   		record[p] = -1;
      		for (i=2;i<=fac.nrows;i++) 
    		{

	 /* 
	  * Notice that from original code in get_selected_containing_faces 
	  * that the search is reduced here by seeing if the primitive
	  * (row no) was previous found from steps involving
	  * quick search through the FSI spatiial index file
	  */


	 		if ( point_in_face(longitude,latitude,
				i,fac,rng,edg, 
                                (int (*)(double *, double*))NULL) == TRUE) 
	 		{

				record[p] = i;
				break;
	 		}
		}

      		vpf_close_table(&fac);
      		vpf_close_table(&rng);
      		vpf_close_table(&edg);

		if (record[p] == -1)
		{
			ret_status = PGSAA_W_DCW_NODATA;
		}

	}	
	return(ret_status);

}


/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Retrieve the feature point within the specified table.
 
NAME:
	PGS_AA_DCW_Feature

SYNOPSIS:
C:
	PGSt_SMF_Status
	PGS_AA_DCW_Feature(char *dcwParms[],
			   char *dcwLayer[],
			   char dcwFFile[],
			   PGSt_integer	nParms,
			   PGSt_double longitude,
			   PGSt_double latitude,
			   char *coverage, 
			   char *tiledir,
			   PGSt_integer row, 
			   PGSt_integer *results)	

FORTRAN:
	None.

DESCRIPTION:
	This routine searches through the edge table to find the exact position
	of the point in relation to the edges which surround it.  It then locates
	row which contains that seacrh point.

	[start]
	PERFORM		Open the Face table, with the relevant row
			Find the exact location within the file.
			When the relevant flag is found, extract the value for that row.
	PERFORM		return value into results;

	[end]


INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	longitude	longitude position	degrees	-180.0	180.0				
	latitude	latitude position	degrees	-90.0	90.0
	
	coverage	database path		N/A	N/A	N/A
	
	tiledir		database path		N/A	N/A	N/A

	row		table row		N/A	N/A	N/A

				

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	results		land/ sea flag		N/A	1	5


RETURNS:
	PGS_S_SUCCESS			succesful return
	PGSAA_E_DCW_ERROR		error in extracting value required
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

	
	
EXAMPLES:
C:
	N/A
	
FORTRAN:
	N/A

NOTES:
	N/A

REQUIREMENTS:
	N/A

DETAILS:
	N/A

GLOBALS:
	None

FILES:
	FAC

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

	DCW library Functions
	---------------------
	vpftable()
	vvmisc()


END_PROLOG:
*****************************************************************************/



PGSt_SMF_Status
PGS_AA_DCW_Feature( char *dcwParms[], char *dcwLayer[], char *dcwFFile[], 	
		PGSt_integer nParms,
		char *continent, char *tiledir, 
		PGSt_integer record[], PGSt_integer results[])

{
   	char tilepath[255];
   	char path[255];
   	vpf_table_type fac, aft;
   	row_type fac_rec, aft_rec;
   	int aft_key, n;
	int feature;
	int result;
	int p;
#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;        /* lock and unlock return */
#endif

	for (p=0;p<nParms;p++)
	{
		/*
		 * check that the record is a valid record
		 */

		if (record[p] == -1)
		{
			results[p] = -1;
		}
		else
		{
			/* construct layer path */

      			strcpy(tilepath, continent);
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (VPF) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKVPF);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

			vpfcatpath(tilepath, dcwLayer[p]);

			/* open FAC index path */
  
   			strcpy(path,tilepath);
      			vpfcatpath(path, tiledir);
   			vpfcatpath(path,"FAC");
   			fac = vpf_open_table(path,disk,"rb",NULL);

			/* open Feature File */

   			strcpy(path,tilepath);
   			vpfcatpath(path,dcwFFile[p]);
   			aft = vpf_open_table(path,disk,"rb",NULL);

			/* read the face record from fac file */

   			fac_rec = read_row(record[p], fac);

			/* extract the feature key from the record */

   			get_table_element(1,fac_rec,fac,&aft_key,&n);

			/* read the feature record from the feature file */

   			aft_rec = read_row(aft_key,aft);

			/* get required parameter from the feature record */
			
   			feature = table_pos(dcwParms[p],aft);
			get_table_element(feature,aft_rec,aft,&result,&n);

			/* fill the results */

			results[p] = result;

   			free_row(fac_rec,fac);
   			free_row(aft_rec,aft);
   			vpf_close_table(&fac);
  			vpf_close_table(&aft);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKVPF);
#endif

		}
	}
	
	return PGS_S_SUCCESS;

}



/* IMPORTANT */
/* dummy routine to satisfy  undefined symbols, brought from library functions - no editing of source files */
#if !( defined(CYGWIN) )
int stricmp(char *cs, char *ct)
{
	return(strcmp(cs,ct));
}
#endif
