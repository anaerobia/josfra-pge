/*
$Header:   K:/utilapps/archive/makehdf.c_v   1.9   Jan 17 1996 17:07:32   mao  $
 *
 * CONTAINS: 	makehdf
 */
  
/*		   
$Log:   K:/utilapps/archive/makehdf.c_v  $
 * 
 *    Rev 1.9   Jan 17 1996 17:07:32   mao
 * Updated with respect to ff_text_pre_parser
 * 
 *    Rev 1.8   Dec 22 1995 11:48:22   mao
 * The bare minimum to conform to latest FreeForm library.
 * 
 *    Rev 1.7   03 Aug 1995 07:55:02   globepc
 * Fixed a little bug with command-line
 * argument ignoring
 * 
 *    Rev 1.5   04/03/95 19:13:18   pvcs
 * changed function calls to memtrack wrappers
 * 
 *    Rev 1.5   09 Nov 1992 17:14:16   haber
 * added write(,,);iting of HDF headers with byte swapping
 * added type to MAX_MIN structure
 * 
 *    Rev 1.4   06 Nov 1992 13:14:50   haber
 * Works now on ASCII file, fixed scale formats, does scaling +reformatting
 * 
 *    Rev 1.3   02 Nov 1992 14:38:16   haber
 * Got the scaling + reformatting to work, now I need to add writing
 * out the scaled bytes to the file.
 * 
 *    Rev 1.2   27 Oct 1992 16:45:18   haber
 * mins and maxes are calculated
 * 
 *    Rev 1.1   26 Oct 1992 13:18:04   haber
 * checked for five variable set
 * added list of variables from standard input
 * create binary format from just desired variables
 * added row_size structures
 */

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

#ifdef CCMSC
#define PC
#include <malloc.h>
#define BUFFER_SIZE 32768
#else
#define BUFFER_SIZE 49512
#endif

#include <databin.h>
#include <dataview.h>
#include <maxmin.h>
#include <vg.h>
#include <df.h>

/* Function Prototypes */
int define_vset_fields(int32, FORMAT_PTR, char*);
void write_max_min(MAX_MIN_PTR);
void help(void);
int32 SDcreate(int32 fid, char *name, int32 nt, int32 rank, int32 *dimsizes);
intn SDend(int32 id);
int32 SDendaccess(int32 id);
intn SDsetexternalfile(int32 id, char *filename, int32 offset);
int32 SDstart(const char *name, int32 HDFmode);

/* Global variables- Yuk! */
#ifndef CCMSC
char little_endian_nt = 0;
#endif

#undef ROUTINE_NAME
#define ROUTINE_NAME "Makehdf"
#define NULL_STR ""

/*
			
 * NAME:	makehdf
 *
 * PURPOSE:	Translate given file into an HDF vset or SDS
 *         	Also includes a demultiplexing option
 *			
 *
 * AUTHOR: 	Ted Habermann, NGDC, (303)497-6472, haber@ngdc1.noaa.gov
 *        	Mark VanGorp, NGDC, (303)497-6221, mvg@ngdc.noaa.gov
 *
 * USAGE:
 *       	makehdf input_file [-r rows] [-c columns] [-v variable_file]
 *       	[-d HDF_description_file] [-xl x_axis_label -yl y_axis_label]
 *			[-xu x_units -yu y_units] [-xf x_format -yf y_format] [-id file_id]
 *			[-vd [file_name]] [-dmx [-sep]] [-df] [-md missing_data_file]
 *                      [-ho (header only)] [-le (little endian (Unix only))]
 *
 * COMMENTS: 
 *
 */

void main(int argc, char **argv)
{

	BOOLEAN		skip_variable			= 0;
	BOOLEAN		sep_var_files			= 0;
	BOOLEAN		create_dmx_file			= 1;
	BOOLEAN		write_SDS			= 0;
	BOOLEAN		dmx_only			= 0;
	BOOLEAN		write_vset			= 0;

	char		dmx_file_name[256];
	char		hdf_file_name[256];
	char		format_buffer[MAX_NAME_LENGTH];
	char		error_buf[MAX_ERRSTR_BUFFER];

	char		*vdata_file_name			= NULL;
	char		*output_file_name			= NULL;
	char		*file_extension				= NULL;
	char		*sys_command				= NULL;
	char 		*buffer					= NULL;
	char		*buffer_ptr				= NULL;
	char 		*point			 		= NULL;
	char		*char_buffer				= NULL;
	char		*current_record				= NULL;
	char		*var_file				= NULL;
	char		*hdf_descrip_file			= NULL;
	char		*missing_data_file			= NULL;
	char		*x_label				= NULL_STR;
	char		*y_label				= NULL_STR;
	char		*x_unit					= NULL_STR;
	char		*y_unit					= NULL_STR;
	char		*x_fmt					= NULL_STR;
	char		*y_fmt					= NULL_STR;
	char		*file_id				= NULL;
	char		*file_path				= NULL;

	char		**var_name_list				= NULL;
	char 		**cache_offset				= NULL;
	char 		**cache_offsets				= NULL;
	char 		**var_name_list_ptr			= NULL;
	char		header_only				= 0;

	DATA_BIN_PTR		input			= NULL;
	DATA_VIEW_PTR		view			= NULL;

/*	double minimum, maximum;*/

	FORMAT_PTR		binary_format		= NULL;
	FORMAT_PTR		hdf_format		= NULL;

	HFILEID hdf_file 		= 0L;
	HFILEID vdata_file		= 0L;

	int error 				= 0;
	int num_cache 			= 0;
/*	int hdf_header_length;*/
	int dmx_file;
/*	int record;*/
	int variable;
	int num_variables;
	int i;
	int num_tokens			= 0;
	int record_length		= 0;
	int max_var_length		= 0;
	int var_length			= 0;
	int rank			= 2;	/* SDS dimensions */

	long cache_bytes	  	= 0L;
	long total_records 		= 0L;
	long cache_records 		= 0L;
	long num_records		= 1;
	long record_number		= 1;
	long total_cache_bytes	= 0L;
	long total_file_bytes	= 0L;
	long result_bytes		= 0L;
	int32 shape[2];						/* SDS dimensions */
	int32 windims[2];					/* SDS slice dimensions */
	int32 numtype;
	long section_bytes		= 0L;

	MAX_MIN_PTR			max_min			= NULL;

	PARAM_LIST *missing_data_list				= NULL;
	PARAM_LIST *missing_data_ptr				= NULL;

	ROW_SIZES_PTR		file_sections	 		= NULL;
	ROW_SIZES_PTR		file_section	 		= NULL;
	ROW_SIZES_PTR		file_sections_copy 		= NULL;
	ROW_SIZES_PTR		dmx_file_offsets		= NULL;
	ROW_SIZES_PTR		dmx_file_offset			= NULL;

	int32 fid;
	int32 sdsid;
	int32 offset = 0;

	unsigned int row_bytes;
	unsigned int num_bytes			= 0;
	unsigned int bytes_read			= 0;
	unsigned int num_vars			= 0;
	unsigned int buffer_offset;
	unsigned short bytes_written	= 0;

	VARIABLE_LIST_PTR	var_list_ptr				= NULL;
	VARIABLE_PTR	var_ptr				= NULL;

 	int32 vdata_set							= 0;
	int32 v_group							= 0;

	/* Check number of command args */
	if(argc < 2) {
		help();
		exit(1);
	}

	/* Check Input File */
	error = open(*(argv + 1), O_RDONLY);
	if(error == -1)
	{
		err_push(ROUTINE_NAME, errno ? errno : ERR_OPEN_FILE, *(argv+1));
		err_disp();
		exit(1);
	}
	close(error);

	/* Initialize shape */
	*shape = *(shape+1) = 0L;

	/* Interpret the remaining command line */
	for (i=2; i<argc; i++) {
		/* Demultiplex data into input_file_base.dmx only: No HDF processing */
		if (!memStrcmp(*(argv+i), "-dmx", "*(argv+i),\"-dmx\"")) {
			dmx_only = 1;
			if ( (i+1) < argc ) {
				if ( !memStrcmp(*(argv+i+1), "-sep", "*(argv+i+1),\"-sep\"") ) {
					i++;
					sep_var_files = 1;
				}
			}
		}
		/* Number of rows */
		else if (!memStrcmp(*(argv+i), "-r", "*(argv+i),\"-r\"")) {
			i++;
			*(shape) = atol(*(argv+i));
		}
		/* Number of columns */
		else if (!memStrcmp(*(argv+i), "-c", "*(argv+i),\"-c\"")) {
			i++;
		    *(shape + 1) = atol(*(argv+i));
		}
		/* Variable file */
		else if (!memStrcmp(*(argv+i), "-v", "*(argv+i),\"-v\"")) { 
			i++;
			/* Check Variable File */
			error = open(*(argv + i), O_RDONLY);
			if(error == -1)
			{
				err_push(ROUTINE_NAME, errno ? errno : ERR_OPEN_FILE, *(argv+i));
				err_disp();
				exit(1);
			}
			var_file = *(argv+i);
			close(error);
		}
		/* HDF description file */
		else if (!memStrcmp(*(argv+i),"-d", "*(argv+i),\"-d\"")) {
					i++;
					/* Check HDF description file */
					error = open(*(argv + i), O_RDONLY);
					if(error == -1)
					{
						err_push(ROUTINE_NAME, errno ? errno : ERR_OPEN_FILE, *(argv+i));
						err_disp();
						exit(1);
					}
					hdf_descrip_file = *(argv+i);
					close(error);
		}
		/* designated HDF output file: default is input_file_base.HDF */
		else if (!memStrcmp(*(argv+i),"-dof", "*(argv+i),\"-dof\"")) {
					i++;
					output_file_name = *(argv+i);
#ifdef SUNCC
					if ( point = memStrrchr(output_file_name,'/', "output_file_name,'/'")) {
						file_path = (char*)memMalloc((size_t)256, "file_path");
						if (!file_path) {
							err_push(ROUTINE_NAME, ERR_MEM_LACK, "file_path");
							err_disp();
							exit(1);
						}
						memStrcpy(file_path, output_file_name, "file_path, output_file_name"); 
						point = memStrrchr(file_path,'/', "file_path,'/'"); 
						*(point+1) = '\0';
					}
#else
					if ((point = memStrrchr(output_file_name,'\\', "output_file_name,'\\'")) != NULL)
					{
						file_path = (char*)memMalloc((size_t)256, "file_path");
						if (!file_path) {
							err_push(ROUTINE_NAME, ERR_MEM_LACK, "file_path");
							err_disp();
							exit(1);
						}
						memStrcpy(file_path, output_file_name, "file_path, output_file_name"); 
						point = memStrrchr(file_path,'\\', "file_path,'\\'"); 
						*(point+1) = '\0';
					}
#endif
		}
		/* missing data file */
		else if (!memStrcmp(*(argv+i),"-md", "*(argv+i),\"-md\"")) {
					i++;
					/* Check missing data file */
					error = open(*(argv + i), O_RDONLY);
					if(error == -1){
						err_push(ROUTINE_NAME, errno ? errno : ERR_OPEN_FILE, *(argv+i));
						err_disp();
						exit(1);
					}
					missing_data_file = *(argv+i);
					close(error);
		}
		/* x-axis label */
		else if (!memStrcmp(*(argv+i),"-xl", "*(argv+i),\"-xl\"")) {
							i++; 
							x_label = *(argv+i);
		}
		/* x-axis unit */ 
		else if (!memStrcmp(*(argv+i),"-xu", "*(argv+i),\"-xu\"")) {
							i++;
							x_unit = *(argv+i);
		}
		/* x scale format */ 
		else if (!memStrcmp(*(argv+i),"-xf", "*(argv+i),\"-xf\"")) {
							i++;
							x_fmt = *(argv+i);
		}
		/* y-axis label */ 
		else if (!memStrcmp(*(argv+i),"-yl", "*(argv+i),\"-yl\"")) {
							i++;
							y_label = *(argv+i);
		}
		/* y-axis unit */
		else if (!memStrcmp(*(argv+i),"-yu", "*(argv+i),\"-yu\"")) {
							i++;
							y_unit = *(argv+i);
		}
		/* y-axis format */
		else if (!memStrcmp(*(argv+i),"-yf", "*(argv+i),\"-yf\"")) {
							i++;
							y_fmt = *(argv+i);
		}

#ifndef CCMSC
		/* little endian byte order */
		else if (!memStrcmp(*(argv+i),"-le", "*(argv+i),\"-le\""))
		{
							little_endian_nt = 1;
		}
#endif

		/* Header only */
		else if (!memStrcmp(*(argv+i),"-ho", "*(argv+i),\"-ho\"")) {
							header_only = 1;
		}
		/* HDF file identification */
		else if (!memStrcmp(*(argv+i),"-id", "*(argv+i),\"-id\"")) {
					i++;
					file_id = *(argv+i);
		}
		/* convert given a demultiplexed file */
		else if (!memStrcmp(*(argv+i),"-df", "*(argv+i),\"-df\"")) {
					memStrcpy(dmx_file_name, *(argv+1), "dmx_file_name, *(argv+1)");
					create_dmx_file = 0;
					write_SDS = 1;
		}
		/* vdata file */
		else if (!memStrcmp(*(argv+i),"-vd", "*(argv+i),\"-vd\"")) {
			create_dmx_file = 0;
			write_vset = 1;
			i++;
			vdata_file_name = (char*)memMalloc((size_t)MAX_NAME_LENGTH, "vdata_file_name");
			if (!vdata_file_name) {
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "vdata file");
				err_disp();
				exit(1);
			}
			if ( (i == argc) || (**(argv+i) == '-') ) {
				/* Use default vdata file name */
				i--;
				memStrcpy(vdata_file_name, *(argv+1), "vdata_file_name, *(argv+1)");
				point = memStrchr(vdata_file_name, '.', "vdata_file_name, '.'");
				if (point)
					memStrcpy((point+1), "HDF\0)", "(point+1),\"HDF\\0)\"");
				else
					memStrcat(vdata_file_name,".HDF\0", "vdata_file_name,\".HDF\\0\"");
			}
			else /* user supplied vdata file name */
				memStrcpy(vdata_file_name, *(argv+i) , "vdata_file_name, *(argv+i)");
			
			vdata_file = (HFILEID)Hopen(vdata_file_name, (intn)DFACC_CREATE, (int16)0);
			if (!vdata_file) {
				err_push(ROUTINE_NAME, ERR_OPEN_FILE, vdata_file_name);
				err_disp();
				exit(1);
			}
			Hclose(vdata_file);
		}
		/* invalid argument */
		else {	
			sprintf(error_buf, "invalid argument --> %s", *(argv+i));
			err_push(ROUTINE_NAME, ERR_MAKE_HDF, error_buf);
			err_disp();
			exit(1);
       		} /* end if */
	} /* end for */


	/* Allocate scratch buffer */
	buffer = (char *)memMalloc((size_t)BUFFER_SIZE, "buffer");
	if (!buffer){
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Buffer");
		err_disp();
		exit(1);
	}

	/* Set up the demux file name */
	/* Current default directories for dmx file:
		1) Path given by -dof option
		2) Current working directory
	*/
	if (create_dmx_file) {
		if (file_path) 
			memStrcpy(dmx_file_name, file_path, "dmx_file_name, file_path");
#ifdef SUNCC
		point = memStrrchr(*(argv+1), '/', "*(argv+1), '/'");
#else
		point = memStrrchr(*(argv+1), '\\', "*(argv+1), '\\'");
#endif
		if (point && file_path)
			memStrcat(dmx_file_name, (point + 1), "dmx_file_name, (point + 1)");
		else if (!point && file_path)
			memStrcat(dmx_file_name, *(argv+1), "dmx_file_name, *(argv+1)");
		else if (point && !file_path)
			memStrcpy(dmx_file_name, (point+1), "dmx_file_name, (point+1)");
		else 
			memStrcpy(dmx_file_name, *(argv+1), "dmx_file_name, *(argv+1)");
		point = memStrchr(dmx_file_name, '.', "dmx_file_name, '.'");
		if(!point) {
			memStrcat(dmx_file_name, ".\0", "dmx_file_name, \".\\0\"");
			point = memStrchr(dmx_file_name,  '.', "dmx_file_name,  '.'");
		}
		file_extension = point + 1;
		/* .dmx will be default unless demultiplexing into separate files is specified */
		sprintf(file_extension, "dmx");
	}
	
	/*
	** SET UP THE DATA OBJECTS:
	**	1) Input bin.
	*/

	input = db_make("input_bin");
	if(!input){
		sprintf(error_buf, "input_bin for %s", *(argv+1));
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, error_buf);
		err_disp();
		exit(1);
	}

	/*
	** Set the Data Bin Attributes 
	*/

	if (db_set(input,
 		BUFFER, buffer,
		DBIN_FILE_NAME,	argv[1],
		DBIN_FILE_HANDLE,
		MAKE_NAME_TABLE, NULL,
		INPUT_FORMAT, NULL, NULL,
		CHECK_FILE_SIZE,
		DBIN_CACHE_SIZE,	BUFFER_SIZE,
		END_ARGS)
	)
	{
		err_push(ROUTINE_NAME, ERR_SET_DBIN, "input_bin");
		err_disp();
		exit(1);
	}

	/* If writing HDF, check for proper row and column specification */
	if ( !(dmx_only || vdata_file_name) ) {
		if ( *(shape) == 0 ) {
			if ( (nt_askvalue(input, "number_of_rows", (unsigned int)FFV_LONG, shape, buffer)) == FALSE) {
				err_push(ROUTINE_NAME, ERR_DIMENSION, "x-dimension: -r option (rows)");
				err_disp();
        			exit(1);
    			}
		}
    		if ( *(shape+1) == 0 ) {
			if ( (nt_askvalue(input, "number_of_columns", (unsigned int)FFV_LONG, (shape + 1), buffer)) == FALSE) {
				err_push(ROUTINE_NAME, ERR_DIMENSION, "y-dimension: -c option (cols)");
				err_disp();
       				exit(1);
    			}
    		}
	}
 
	/* Set HDF file format equal to the input format
	   If the input format is ASCII or a variable file is specified, this
	   input format will subsequently be changed. Thus a copy is made */
	if (var_file || (IS_ASCII(input->input_format)))
	{
		FF_BUFSIZE bufsize;
		PP_OBJECT pp_object;
		FORMAT_LIST_PTR f_list = NULL;
		
		bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN; 
		bufsize.bytes_used = 0;
		bufsize.buffer = buffer;
		
		pp_object.ppo_type = PPO_FORMAT_LIST;
		pp_object.ppo_object.hf_list = &f_list;
		
		ff_list_vars(input->input_format, &bufsize);
		if (ff_text_pre_parser(NULL, &bufsize, &pp_object) ||
		    !(hdf_format = db_find_format(*(pp_object.ppo_object.hf_list),
		                                  FFF_NAME,
		                                  FORMAT_NAME_INIT,
		                                  END_ARGS
		                                 )
		     )
		   )
		{
			err_push(ROUTINE_NAME, ERR_MAKE_FORM, "hdf_format");
			err_disp();
			exit(1);
		}
		hdf_format->type = input->input_format->type;
	}
	else
		hdf_format = input->input_format;

	/* If user specifies a variable file, then process it */
	if (var_file) {

		error = ff_file_to_buffer(var_file, buffer);
		if (error == 0) {
			err_push(ROUTINE_NAME, ERR_READ_FILE, "var_file to buffer");
			err_disp();
			exit(1);
		}

		/* Create an array of pointers to variables listed in var_file */
		var_name_list = var_name_list_ptr = (char**)memCalloc(hdf_format->num_in_list, sizeof(char*), "var_name_list & .._ptr");
		point = strtok(buffer, " ,\r,\n,\t");
		while (point) {
			*var_name_list = point;
			var_name_list++;
			num_tokens++;
			point = strtok(NULL, " ,\r,\n,\t");
		}
	}

	/* dmx_file_offsets only exists if we already have a demultiplexed file
	   and a need to write an SDS */
	if ( (!create_dmx_file) && write_SDS ) {
		dmx_file_offset = dmx_file_offsets = (ROW_SIZES_PTR)memCalloc(var_file ? num_tokens : hdf_format->num_in_list,sizeof(ROW_SIZES), "dmx_file_offset & ..s");
		if(!dmx_file_offsets){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"demux file offsets");
			err_disp();
			exit(1);
		}
	}
	/* If a variable file exists, then delete any unwanted variables in the hdf_format
	   If we already have a demux file and want to write an SDS for all variables, then
	   the dmx_file_offset structure must be filled anyway
	*/
	if ( (var_file) || (!create_dmx_file && write_SDS) ) {
		var_list_ptr = FFV_FIRST_VARIABLE(hdf_format);
		if (var_file)
			printf("\nVariables Translated:\n");
		else
			printf("\nAll Variables Translated:\n");		
		/* Determine which variables in the hdf_format->list also occur
		   in the variable file: ie Which variables are to be translated */

		while(dll_data(var_list_ptr))
		{
			var_list_ptr = dll_next(var_list_ptr);

			var_ptr = FFV_VARIABLE(dll_previous(var_list_ptr));
			/* If we already have a dmx file and are writing an SDS, then
			   we must determine the offsets. Reason -- with a variable file,
			   the desired variable sections may not be contiguous */
			if (dmx_file_offsets) {
					section_bytes = ( (long)FF_VAR_LENGTH(var_ptr) * (*shape) * (*(shape+1)) );
					offset += section_bytes;
			}
			if (var_file) {
			   /* Search through the array of variables from the variable file */
			   for (i=0,var_name_list=var_name_list_ptr; i<num_tokens; i++, var_name_list++) {
				   if (memStrcmp(var_ptr->name, *var_name_list, "var_ptr->name, *var_name_list") == 0)
					   break;
			   }
			}
			else
			   /* All variables used if no variable file exists */
			   i = -1;
			/* If variable not in array of variables,
			   then delete it from the format */
			if (i==num_tokens) {
				hdf_format->max_length -= FF_VAR_LENGTH(var_ptr);
				dll_delete(dll_previous(var_list_ptr), (void (*)(void *))ff_free_variable);
			}
			else { 
				if (var_file )
					printf("%s\n",*var_name_list);
				if ( dmx_file_offsets ) {
					dmx_file_offset->start = offset - section_bytes;
					dmx_file_offset->num_bytes = section_bytes;
					dmx_file_offset++;
				}
			}
						
		} /* End while (var_list_ptr) */

		if (!dll_data(FFV_FIRST_VARIABLE(hdf_format))) {
			err_push(ROUTINE_NAME, ERR_MAKE_HDF, "Empty hdf format list");
			err_disp();	
			exit(1);
		}
		if (var_file) {
			memFree((void*)(var_name_list_ptr), "var_name_list_ptr");
			var_name_list = var_name_list_ptr = NULL;
		}
	} /* End if */

	/* Set missing data flags if file exists */
	if (missing_data_file) { 
		
		error = ff_file_to_buffer(missing_data_file, buffer);
		if (error == 0) {
			err_push(ROUTINE_NAME, ERR_READ_FILE, "missing_data_file to buffer");
			err_disp();
			exit(1);
		}
		missing_data_list = make_param_list(buffer, input->input_format);
		if (!missing_data_list) {
			err_push(ROUTINE_NAME, ERR_MAKE_PLIST, "missing data list");
			err_disp();
			exit(1);
		}
	}

	/* The hdf format must be binary
	   If it currently is not binary, then convert from ASCII to binary
	   If a variable file is specified, the variable positions are
	   renumbered starting from 1 */
	if(var_file || IS_ASCII(hdf_format)){
		binary_format = ff_afm2bfm(hdf_format);
		ff_free_format(hdf_format);
		hdf_format = binary_format;
	}
	record_length = FORMAT_LENGTH(input->input_format);
	total_records	= input->data_available / record_length;
	num_variables	= hdf_format->num_in_list;

	/* Set up the demultiplexing arrays:
	These are ROW_SIZES structures which describe indexed blocks of
	data. Each block has a starting position and a size.
	Only necessary if we are creating a demultimplexed file
	*/
	if (create_dmx_file) {
		file_sections = (ROW_SIZES_PTR)memCalloc(num_variables, sizeof(ROW_SIZES), "file_sections");
		if(!file_sections){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"File Sections");
			err_disp();
			exit(1);
		}
		file_sections_copy = (ROW_SIZES_PTR)memCalloc(num_variables, sizeof(ROW_SIZES), "file_sections_copy");
		if(!file_sections_copy){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"File Sections");
			err_disp();
			exit(1);
		}
		cache_offsets = (char **)memCalloc(num_variables, sizeof(char *), "cache_offsets");
		if(!cache_offsets){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"Cache offsets");
			err_disp();
			exit(1);
		}
	}

	/* Initialize the file_sections ROW_SIZE structure:
	
	file_sections are sections of the output file which hold the
	de-multiplexed data in its original (binary) representation. The size
	of these sections is the total number of records * the size of the
	variable.
	If we already have the demultiplxed file and its offsets, then skip
	this loop: dmx_file_offsets will be set to file_sections later

	*/

	if (create_dmx_file) {
		for(variable = 0, var_list_ptr = FFV_FIRST_VARIABLE(hdf_format),
			file_section = file_sections;
			variable < num_variables;
			variable++, file_section++, var_list_ptr = dll_next(var_list_ptr)){

			var_ptr = FFV_VARIABLE(var_list_ptr);	
			if (sep_var_files)
				sprintf(file_extension,"%d", (variable+1));

			/* If not separate files for each variable, then the same file
			   gets opened and closed */	
			error = open(dmx_file_name, O_CREAT | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
			if(error == -1){
				err_push(ROUTINE_NAME, errno ? errno : ERR_CREATE_FILE, dmx_file_name);
				err_disp();
				exit(1);
			}
			close(error);
	
			if( (variable == 0) || (sep_var_files) )
				file_sections->start		= 0L;
			else {		
				file_section->start = (file_section - 1)->start +
					(file_section - 1)->num_bytes;
			}
			file_section->num_bytes	= FF_VAR_LENGTH(var_ptr) * total_records;
		} /* End for */
	} /* End if */

	if ( IS_ASCII(input->input_format) || (var_file) )
		record_length = FORMAT_LENGTH(hdf_format);
	else
		record_length = FORMAT_LENGTH(input->input_format);

	/* If writing a vset, then set up the lone vdata attached to one vgroup */
	if (write_vset) {
		vdata_file = (HFILEID)Hopen(vdata_file_name, (int)DFACC_ALL, 0);
		Vinitialize(vdata_file);
		v_group = Vattach(vdata_file, -1, "w");
		vdata_set = VSattach(vdata_file, -1, "w");
		VSsetname(vdata_set, vdata_file_name);

		error = define_vset_fields(vdata_set, hdf_format, buffer);
		if (error) {
			err_push(ROUTINE_NAME, ERR_MAKE_VSET, "Setting vset fields");
			err_disp();
			exit(1);
		}
		VSsetfields(vdata_set, buffer);
	}
	else if (create_dmx_file) 
		/* Save a copy of file_sections as it gets written over when writing the
		   caches to the demultiplexed file */
		memMemcpy((void*)file_sections_copy, (void*)file_sections,(size_t)(sizeof(ROW_SIZES) * num_variables), "(void*)file_sections_copy, (void*)file_sections,(size_t)(sizeof(ROW_SIZES) * num_variables)");

	/* NOTE: When writing a vset, a demux file is not created */
	if (create_dmx_file || write_vset) {
		
		/*
		** process the data
		*/
	
		if(db_events(input, PROCESS_FORMAT_LIST, FFF_ALL_TYPES, END_ARGS)){
			err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Processing Input Format");
			err_disp();
			exit(1);
		}
	
	
		do {
	
			/* Convert cache if ASCII or a variable file was specified */
			if (  IS_ASCII(input->input_format) || (var_file) ) {
				if(db_events(input,
					DBIN_CONVERT_CACHE, (DATA_BUFFER)NULL, (FORMAT_PTR)hdf_format,
					(long*)&result_bytes,
					END_ARGS)){
	 				err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Converting cache to binary");
	 				err_disp();
	 				exit(1);
				}
			}
			/* If writing a vset, do the write and continue to the next cache */
			if (write_vset) {
				VSwrite(vdata_set, (uint8 *)input->cache, input->records_in_cache, FULL_INTERLACE);
				total_cache_bytes += (input->records_in_cache * FORMAT_LENGTH(input->input_format));
				printf("%d   Caches (%ld bytes) Processed\n ", ++num_cache, total_cache_bytes);
				continue;
			}
	
			/* Initialize cache pointers:
			The de-multiplexing is done into a buffer which is the
			same size as the input cache. Pointers are dispersed throughout
			that cache at the points where the single arrays begin. These
			pointers are then swept through the output buffer as the
			demultiplexing occurs. The offsets are defined in terms of
			last variable, as it is the size of the block for the last variable
			that determines the offset to the present variable */

			if(db_events(input, DBIN_DEMUX_CACHE, buffer, hdf_format, END_ARGS)){
				err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Processing Input Format");
				err_disp();
				exit(1);
			}

			total_cache_bytes += input->records_in_cache * (long)FORMAT_LENGTH(input->input_format);
			printf("%d   Caches (%ld bytes) Processed", ++num_cache, total_cache_bytes);
	
			/* Write the caches to the demultiplexed file */
			for(variable = 0,
				var_list_ptr = FFV_FIRST_VARIABLE(hdf_format),
				point = buffer,
				file_section = file_sections;
				variable < num_variables;
				variable++, point += num_bytes, file_section++, var_list_ptr = dll_next(var_list_ptr)) {

				var_ptr = FFV_VARIABLE(var_list_ptr);	
				/* Write each cache section out to the output file and then
				increment the file_section pointer so that the next section 
				goes after the present one. */
	
				num_bytes = (unsigned int)(input->records_in_cache * (long)FF_VAR_LENGTH(var_ptr));
				if (sep_var_files)
					sprintf(file_extension,"%d", (variable+1));
				dmx_file = open(dmx_file_name, O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
				if(dmx_file == -1)
				{
					err_push(ROUTINE_NAME, errno ? errno : ERR_CREATE_FILE, dmx_file_name);
					err_disp();
					exit(1);
				}
				
				lseek(dmx_file, file_section->start, SEEK_SET);
				bytes_written = (unsigned short)write(dmx_file, (void *)point, num_bytes);
				close(dmx_file);
	
				if (bytes_written != num_bytes)
				{
					sprintf(error_buf, "%s Writing to Output", dmx_file_name);
					err_push(ROUTINE_NAME, errno ? errno : ERR_WRITE_FILE, error_buf);
					err_disp();
					exit(1);
				}
				total_file_bytes += bytes_written;
				
				file_section->start += num_bytes;
			}
			if ( !(dmx_only & sep_var_files) )
				printf(": %ld bytes written to %s \n", total_file_bytes, dmx_file_name);
			else
				printf(": %ld bytes written to dmx file(s)\n", total_file_bytes);
	
		} while((db_events(input, PROCESS_FORMAT_LIST, FFF_ALL_TYPES, END_ARGS)) != EOF);

	} /* End if (create_dmx_file || write_vset) */

	/* Exit if only demultiplexed file or vset is needed
	   Else reopen demultiplexed file for HDF SDS creation */
	if ( dmx_only || write_vset ) {
		if (write_vset) {
			Vinsert(v_group,vdata_set);
			VSdetach(vdata_set);
			Vdetach(v_group);
			Vfinish(vdata_file);
			Vclose(vdata_file);
		}
		exit(0);
	}
	else {
		dmx_file = open(dmx_file_name, O_BINARY | O_RDONLY, S_IREAD | S_IWRITE);
		if(dmx_file == -1){
			err_push(ROUTINE_NAME, errno ? errno : ERR_OPEN_FILE, dmx_file_name);
			err_disp();
			exit(1);
		}
	}

	/* If a demultiplexed file was not created, then file_sections has not
	   yet been set, else recopy in saved file_sections array */
	if (!create_dmx_file)
		file_sections = dmx_file_offsets;
	else
		memMemcpy((void*)file_sections, (void*)file_sections_copy,(size_t)(sizeof(ROW_SIZES) * num_variables), "(void*)file_sections, (void*)file_sections_copy,(size_t)(sizeof(ROW_SIZES) * num_variables)");

	/* Ready to write out SDS */
	/* Set up the hdf output file */
	if (!output_file_name) {
#ifdef SUNCC
		point = memStrrchr(*(argv+1), '/', "*(argv+1), '/'");
#else
		point = memStrrchr(*(argv+1), '\\', "*(argv+1), '\\'");
#endif
		if (point)
			memStrcpy(hdf_file_name, (point + 1), "hdf_file_name, (point + 1)");
		else
			memStrcpy(hdf_file_name, *(argv + 1), "hdf_file_name, *(argv + 1)");
		point = memStrchr(hdf_file_name, '.', "hdf_file_name, '.'");
		if(point)
			memStrcpy(point + 1, "HDF\0", "point + 1,\"HDF\\0\"");
		else
			memStrcat(hdf_file_name, ".HDF\0", "hdf_file_name,\".HDF\\0\"");
	}
	else
		(void*)memStrcpy(hdf_file_name, output_file_name, "hdf_file_name, output_file_name");

	/* Determine the number of rows and columns for the SDS
	   Number of columns stays the same for slices
	   shape[0] and windims[0] refer to rows
	   shape[1] and windims[1] refer to columns
	*/
	printf("\nWriting %s and calculating maxima and minima ...\n\n",hdf_file_name);

	*(windims+1) = *(shape+1);

	/* input->cache is no longer needed */
	(void)memFree((void*)input->cache, "input->cache");
	input->cache = NULL;

	/* Write out the data set using the demultiplexed file */
	for(variable = 0,
		file_section = file_sections,
		var_list_ptr = FFV_FIRST_VARIABLE(hdf_format);
		variable < num_variables;
		variable++,
		file_section++,
		var_list_ptr = dll_next(var_list_ptr) ){

		var_ptr = FFV_VARIABLE(var_list_ptr);
		/* Set dimensions of the SDS */
		error = DFSDsetdims(rank, shape);
		if (error) {
			err_push(ROUTINE_NAME, ERR_DIMENSION, "Setting rank and shape");
			err_disp();
			exit(1);
		}

		/* Set dimension strings, number types, and data strings of the SDS */
		error = DFSDsetdimstrs(1, x_label, x_unit, x_fmt);
		if (error) {
			err_push(ROUTINE_NAME, ERR_DIMENSION, "Setting x dimension strings");
			err_disp();
			exit(1);
		}

		error = DFSDsetdimstrs(2, y_label, y_unit, y_fmt);
		if (error) {
			err_push(ROUTINE_NAME, ERR_DIMENSION, "Setting y dimension strings");
			err_disp();
			exit(1);
		}

		switch(var_ptr->type){
		case FFV_ULONG:

#ifndef CCMSC
			if(little_endian_nt)
			{
				DFSDsetNT((int32)DFNT_LUINT32);
				numtype = (int32)DFNT_LUINT32;
			}
			else
#endif

			{
				DFSDsetNT((int32)DFNT_UINT32);
				numtype = (int32)DFNT_UINT32;
			}
			sprintf(format_buffer,"I%d",var_ptr->end_pos - var_ptr->start_pos + 1);
			DFSDsetdatastrs(var_ptr->name, "", format_buffer,"");
			break;

		case FFV_LONG:

#ifndef CCMSC
			if(little_endian_nt)
			{
				DFSDsetNT((int32)DFNT_LINT32);
				numtype = (int32)DFNT_LINT32;
			}
			else
#endif

			{
				DFSDsetNT((long)DFNT_INT32);
				numtype = (int32)DFNT_INT32;
			}
			sprintf(format_buffer,"I%d",var_ptr->end_pos - var_ptr->start_pos + 1);
			DFSDsetdatastrs(var_ptr->name, "", format_buffer,"");
			break;

		case FFV_SHORT:

#ifndef CCMSC
			if(little_endian_nt)
			{
				DFSDsetNT((int32)DFNT_LINT16);
				numtype = (int32)DFNT_LINT16;
			}
			else
#endif

			{
				DFSDsetNT((long)DFNT_INT16);
				numtype = (int32)DFNT_INT16;
			}
			sprintf(format_buffer,"I%d",var_ptr->end_pos - var_ptr->start_pos + 1);
			DFSDsetdatastrs(var_ptr->name, "", format_buffer,"");
			break;

		case FFV_USHORT:

#ifndef CCMSC
			if(little_endian_nt)
			{
				DFSDsetNT((int32)DFNT_LUINT16);
				numtype = (int32)DFNT_LUINT16;
			}
			else
#endif

			{
				DFSDsetNT((long)DFNT_UINT16);
				numtype = (int32)DFNT_UINT16;
			}
			sprintf(format_buffer,"I%d",var_ptr->end_pos - var_ptr->start_pos + 1);
			DFSDsetdatastrs(var_ptr->name, "", format_buffer,"");
			break;

		case FFV_UCHAR:

#ifndef CCMSC
			if(little_endian_nt)
			{
				DFSDsetNT((int32)DFNT_LUINT8);
				numtype = (int32)DFNT_LUINT8;
			}
			else
#endif

			{
				DFSDsetNT((long)DFNT_UINT8);
				numtype = (int32)DFNT_UINT8;
			}
			sprintf(format_buffer,"I%d",var_ptr->end_pos - var_ptr->start_pos + 1);
			DFSDsetdatastrs(var_ptr->name, "", format_buffer,"");
			break;

		case FFV_FLOAT:

#ifndef CCMSC
			if(little_endian_nt)
			{
				DFSDsetNT((int32)DFNT_LFLOAT32);
				numtype = (int32)DFNT_LFLOAT32;
			}
			else
#endif

			{
				DFSDsetNT((long)DFNT_FLOAT32);
				numtype = (int32)DFNT_FLOAT32;
			}
			sprintf(format_buffer,"F%d.%d",var_ptr->end_pos - var_ptr->start_pos + 1,
			                               var_ptr->precision);
			DFSDsetdatastrs(var_ptr->name, "", format_buffer,"");
			break;

		case FFV_DOUBLE:

#ifndef CCMSC
			if(little_endian_nt)
			{
				DFSDsetNT((int32)DFNT_LFLOAT64);
				numtype = (int32)DFNT_LFLOAT64;
			}
			else
#endif

			{
				DFSDsetNT((long)DFNT_FLOAT64);
				numtype = (int32)DFNT_FLOAT64;
			}
			sprintf(format_buffer,"F%d.%d",var_ptr->end_pos - var_ptr->start_pos + 1,
			                               var_ptr->precision);
			DFSDsetdatastrs(var_ptr->name, "", format_buffer,"");
 			break;

		default:	/* Error, Unknown Input Variable type */
			sprintf(error_buf, "%s not written to hdf file", var_ptr->name);
			err_push("Make_HDF",ERR_UNKNOWN_VAR_TYPE, error_buf);
			err_disp();
			skip_variable = 1;
		}
		if(!header_only){

		if (!skip_variable) {
			total_cache_bytes = file_section->num_bytes;
			/* number of bytes in each row is the # of columns times
			   the number of bytes in the variable */
			row_bytes =	(unsigned int)(shape[1] * (long)FF_VAR_LENGTH(var_ptr));
			cache_bytes  = BUFFER_SIZE / row_bytes;
			cache_bytes *= row_bytes;
	
			error = (int)DFSDstartslice(hdf_file_name); 
			if ( error == -1 ) {
				sprintf(error_buf, "%s: start slice", var_ptr->name);
				err_push(ROUTINE_NAME, ERR_WRITE_HDF, error_buf);
				err_disp();
				exit(1);
			}
					
			lseek(dmx_file, file_section->start, SEEK_SET);

			/* Set up max_min structure */
			max_min = mm_make(var_ptr,0);
			var_length = FF_VAR_LENGTH(var_ptr);
			/* Does var_ptr have missing data flags? */
			for (missing_data_ptr=missing_data_list; missing_data_ptr;
			     missing_data_ptr=missing_data_ptr->next) {
				 if ( !memStrcmp(var_ptr->name, missing_data_ptr->var->name, "var_ptr->name, missing_data_ptr->var->name") )
				 	break;
			}

			if (missing_data_ptr) 
				mm_set(max_min,
					   MM_MISSING_DATA_FLAGS,
					   missing_data_ptr->maximum, missing_data_ptr->minimum,
					   END_ARGS);

			if (IS_CHAR(var_ptr)) 
			       char_buffer = (char*)memRealloc(char_buffer,(size_t)(max_var_length + 1), "char_buffer");
	
			while(total_cache_bytes)
			{
				num_bytes = (unsigned int)min(total_cache_bytes, cache_bytes);
				bytes_read = memRead(dmx_file, buffer, num_bytes, "dmx_file, buffer, num_bytes");
				num_vars = bytes_read / var_length;
	
				if (bytes_read != num_bytes)
				{
					err_push(ROUTINE_NAME, errno ? errno : ERR_READ_FILE, dmx_file_name);
					err_disp();
					exit(1);
				}
				/* Loop through buffer and determine the max and mins */
				for (buffer_offset=0, buffer_ptr=buffer;
					 buffer_offset < num_vars;
					 buffer_offset+=1,buffer_ptr+=var_length) {
						if (IS_CHAR(var_ptr)) {
							memMemcpy(char_buffer, buffer_ptr, var_length, "char_buffer, buffer_ptr, var_length");
							*(char_buffer + var_length) = '\0';
							mm_set(max_min, MM_MAX_MIN, char_buffer, record_number, END_ARGS);
						}
						else
							mm_set(max_min, MM_MAX_MIN, buffer_ptr, record_number, END_ARGS);
				}

				/* set windims[0] to number of rows in buffer */
				*(windims) = bytes_read / row_bytes;
	
				error = (int)DFSDputslice(windims, buffer, shape);
				if ( error  == -1 ) {
					sprintf(error_buf, "%s: put slice", var_ptr->name);
					err_push(ROUTINE_NAME, ERR_WRITE_HDF, error_buf);
					err_disp();
					exit(1);
				}
	
				total_cache_bytes -= bytes_read;
			}
			printf("Variable %s:\n", var_ptr->name);
			write_max_min(max_min);
			error = mm_free(max_min);
			if (error) {
				err_push(ROUTINE_NAME, ERR_GENERAL, "Freeing max_min");
				err_disp();
				exit(1);
			}
			error = (int)DFSDendslice();
			if ( error  == -1 ) {
				sprintf(error_buf, "%s: end slice", var_ptr->name);
				err_push(ROUTINE_NAME, ERR_WRITE_HDF, error_buf);
				err_disp();
				exit(1);
			}

		}
		else
			skip_variable = 0;
		} /******* END IF FOR IF(!HEADER ONLY) ***********/
		else{ /* ALL WE WANT TO DO IS WRITE A HEADER....  */
 
			fid = SDstart(hdf_file_name, DFACC_CREATE);
			if(fid == FAIL){
				printf("SDstart failed\n");
				exit(0);
			}
			sdsid = SDcreate(fid, argv[1], numtype, rank, (int32 *) shape);
			if(sdsid == FAIL){
				printf("SDcreate failed\n");
				exit(0);
			}
 
			offset = 0;
			printf("Creating external file element: %s\n", argv[1]);
			if (SDsetexternalfile(sdsid, argv[1], offset) == FAIL){
				printf("SDsetexternalfile failed\n");
				exit(0);
			}
 
			/* Close the HDF file. */
			SDendaccess(sdsid);
			SDend(fid);
	
		}
	}
	close(dmx_file);

	/* If any annotations specified, write them to file 
			- file id
			- file description
	*/
	if (file_id || hdf_descrip_file) {

		hdf_file = (int32)Hopen(hdf_file_name, (int)DFACC_WRITE, (int16)0);
		if (file_id)
			DFANaddfid(hdf_file, file_id);
		if (hdf_descrip_file) {
	   		total_file_bytes = ff_file_to_buffer(hdf_descrip_file, buffer);
			DFANaddfds(hdf_file, buffer, total_file_bytes);
		}
		Hclose(hdf_file);
	}

	/* delete demultiplexed file */
	if (create_dmx_file) {
#ifdef CCMSC
		(void) unlink(dmx_file_name);
#endif
#ifdef SUNCC
		sprintf(error_buf, "rm %s", dmx_file_name);
		system(error_buf);
#endif
	}
 
} /* end main()  */

int define_vset_fields (int32 vdata_set, FORMAT_PTR format, char *set_fields_ptr)
{

/*	int i;*/
	VARIABLE_LIST_PTR	var_list_ptr;
	VARIABLE_PTR	var_ptr;

	var_list_ptr = FFV_FIRST_VARIABLE(format);
	while ((var_ptr = dll_data(var_list_ptr)) != NULL)
	{
		memStrcpy(set_fields_ptr, var_ptr->name, "set_fields_ptr, var_ptr->name");
		set_fields_ptr += strlen(var_ptr->name);
		memMemset((void*)set_fields_ptr, ',', (size_t)1, "set_fields_ptr, ',', (size_t)1");
		set_fields_ptr++;

		switch(var_ptr->type){
		case FFV_ULONG:

#ifndef CCMSC
			if(little_endian_nt)
				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_LUINT32, 1);
			else
#endif

				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_UINT32, 1);
			break;

		case FFV_LONG:

#ifndef CCMSC
			if(little_endian_nt)
				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_LINT32, 1);
			else
#endif

				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_INT32, 1);
			break;

		case FFV_SHORT:

#ifndef CCMSC
			if(little_endian_nt)
				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_LINT16, 1);
			else
#endif

				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_INT16, 1);
			break;

		case FFV_USHORT:

#ifndef CCMSC
			if(little_endian_nt)
				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_LUINT16, 1);
			else
#endif

				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_UINT16, 1);
			break;

		case FFV_UCHAR:

#ifndef CCMSC
			if(little_endian_nt)
				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_LUINT8, 1);
			else
#endif

				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_UINT8, 1);
			break;

		case FFV_FLOAT:

#ifndef CCMSC
			if(little_endian_nt)
				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_LFLOAT32, 1);
			else
#endif

				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_FLOAT32, 1);
			break;

		case FFV_DOUBLE:

#ifndef CCMSC
			if(little_endian_nt)
				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_LFLOAT64, 1);
			else
#endif

				VSfdefine(vdata_set, var_ptr->name, (long)DFNT_FLOAT64, 1);
 			break;

		default:	/* Error, Unknown Input Variable type */
			err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE, var_ptr->name);
			return(1);
		}

		var_list_ptr = dll_next(var_list_ptr);
	}
	
	/* Remove last comma */
	*(set_fields_ptr - 1) = '\0';
	return(0);
}
void write_max_min(MAX_MIN_PTR max_min) 
{

	switch(max_min->var->type){
	case FFV_ULONG:
		mm_print(max_min);
		if (DFSDsetrange(max_min->maximum, max_min->minimum)) {
			err_push(ROUTINE_NAME, ERR_SET_SDS_RANGE, "FFV_ULONG");
			err_disp();
			exit(1);
		}
		break;

	case FFV_LONG:
		mm_print(max_min);
		if (DFSDsetrange(max_min->maximum, max_min->minimum)) {
			err_push(ROUTINE_NAME, ERR_SET_SDS_RANGE, "FFV_LONG");
			err_disp();
			exit(1);
		}
		break;

	case FFV_SHORT:
		mm_print(max_min);
		if (DFSDsetrange(max_min->maximum, max_min->minimum)) {
			err_push(ROUTINE_NAME, ERR_SET_SDS_RANGE, "FFV_SHORT");
			err_disp();
			exit(1);
		}
		break;

	case FFV_USHORT:
		mm_print(max_min);
		if (DFSDsetrange(max_min->maximum, max_min->minimum)) {
			err_push(ROUTINE_NAME, ERR_SET_SDS_RANGE, "FFV_USHORT");
			err_disp();
			exit(1);
		}
		break;

	case FFV_UCHAR:
		mm_print(max_min);
		if (DFSDsetrange(max_min->maximum, max_min->minimum)) {
			err_push(ROUTINE_NAME, ERR_SET_SDS_RANGE, "FFV_UCHAR");
			err_disp();
			exit(1);
		}
		break;

	case FFV_FLOAT:
		mm_print(max_min);
		if (DFSDsetrange(max_min->maximum, max_min->minimum)) {
			err_push(ROUTINE_NAME, ERR_SET_SDS_RANGE, "FFV_FLOAT");
			err_disp();
			exit(1);
		}
		break;

	case FFV_DOUBLE:
		mm_print(max_min);
		if (DFSDsetrange(max_min->maximum, max_min->minimum)) {
			err_push(ROUTINE_NAME, ERR_SET_SDS_RANGE, "FFV_DOUBLE");
			err_disp();
			exit(1);
		}
 		break;

	default:	/* Error, Unknown Input Variable type */
		err_push("Make_HDF",ERR_UNKNOWN_VAR_TYPE,"write_max_min");
		err_disp();
	}
}

void help()
{

#ifndef CCMSC
	printf("%s%s%s%s%s%s",
#else
	printf("%s%s%s%s",
#endif

#ifdef ALPHA
"\nWelcome to makehdf alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm/HDF application\n\n",
#elif defined(BETA)
"\nWelcome to makehdf beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm/HDF application\n\n",
#else
"\nWelcome to makehdf release "FF_LIB_VER" -- an NGDC FreeForm/HDF application\n\n",
#endif
"USAGE: makehdf input_file [-r rows] [-c columns] [-v variable_file]\n\
[-d HDF_description_file] [-xl x_label -yl y_label] [-xu x_units -yu y_units]\n\
[-xf x_format -yf y_format ] [-id file_id] [-vd [file_name]]\n\
[-dmx [-sep]] [-df] [-md missing_data_file] [-dof output_file]\n\
[-ho]",

#ifndef CCMSC
" [-le]",
#endif

"\n\
-r/-c: Number of rows/columns in HDF scientific data sets\n\
-v: Only variable names in variable_file will be written to HDF file\n\
-d: Description in HDF_description_file will be associated with HDF file\n\
-xl/-yl: x-axis/y-axis description (label)\n\
-xu/-yu: x-axis/y-axis units\n\
-xf/-yf: format to be used in displaying scale for x/y dimension\n\
-id: HDF file id\n\
-vd: write to a vdata file named file_name (UNIX only)\n\
     default file_name is input_file_base.HDF\n\
-dmx: ONLY demultiplex data into input_file_base.dmx\n\
      -sep: demultiplex into separate variable files\n\
-df: Write given binary demultiplexed file into an SDS\n\
     This file is denoted as input_file on the command line. Each variable of\n\
     input format corresponds to each demultiplexed section of output format\n\
-md: Set missing data values from file (var lower_limit upper_limit)\n\
-dof: designated HDF output file: default is input_file_base.HDF\n\
-ho: write header only.\n",

#ifndef CCMSC
"-le Little endian byte order for data\n",
#endif

"Other than the -vd, -dmx, or -dof options, the output file created\n\
is always an SDS named input_file_base.HDF\n");
}

