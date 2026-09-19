/* $Header:   /home/pvcs/tempdir/gettrack.c_v   1.2   04/03/95 19:13:06   pvcs  $
 *
 * CONTAINS: 	gettrack 
 */
  
/*		   
$Log:   /home/pvcs/tempdir/gettrack.c_v  $
 * 
 *    Rev 1.2   04/03/95 19:13:06   pvcs
 * some newer version I found on MVG's PC
 * 
 *    Rev 1.0   03 Apr 1992 10:30:22   tedh
 * Initial revision.
 */

#include <conio.h>
#include <limits.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <errno.h>

#include <vidprim.h>


#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

#include <databin.h>
#include <dataview.h>

#define ROUTINE_NAME "Gettrack"

#define SCRATCH_SIZE 4096
#define BUFFER_SIZE 61440
#define VECTOR_CACHE_SIZE 16384
#define TEMP_BUFFER_SIZE 128

#define VIEW_TRACKLINES      1
#define TRACKLINE_RETRIEVAL  2
#define LAT_LON_RETRIEVAL    3
#define TRACKLINE_FILE       4
#define QUICK_EXIT           5

void print_menu(int *);
void print_help(void);
int print_trackline(DATA_VIEW_PTR, VARIABLE_LIST_PTR, int);
int check_trackline(DATA_VIEW_PTR);

/*
 * NAME:	gettrack
 *
 * PURPOSE:	To retrieve tracklines from large binary data files
 *
 * USAGE:	gettrack bin_index_file bfm_index_format afm_index_format
 *                   bin_data_file bfm_data_format  output_format output_file
 *
 * AUTHORS: Ted Habermann/Mark Van Gorp, NGDC, (303)497-6472/6221, mvg@kryton.colorado.edu
 *
 * COMMENTS: Gettrack uses a binary flightline index file to retrieve tracklines
 *           from a large binary data file. Individual tracklines or ranges of tracklines
 *           may be retrieved. Retrieval is done using trackline names or by specifying
 *           latitude/longitude limits. The binary flightline index file is referred to
 *           as a lookup_table data bin.
 *
 */

void main(int argc, char **argv)
{
	/* Data Bins:
		This program uses two data bins. One has the lookup table,
		and one has the actual input data file.
	*/
	DATA_BIN_PTR lookup_table			= NULL;
	DATA_BIN_PTR input_data				= NULL;
	DATA_VIEW_PTR offset_array			= NULL;
	DATA_VIEW_PTR vector				= NULL;

	VARIABLE_LIST_PTR	offset_var		= NULL;
	VARIABLE_LIST_PTR	trackline_var 	= NULL;

    ROW_SIZES_PTR		rowsize 		= NULL;

	long bytes_to_read;
	long bytes_to_write;
	long trial_long_ptr = 0L;
	long data_buffer_size 				= BUFFER_SIZE;
	long saved_num_recs_in_cache		= 0L;

	unsigned num_in_records;

	int error 					= 0;
	int data_file;
	int output_file;
	int j;
	int num_recs 				= 0;
	int option					= VIEW_TRACKLINES;
	int count					= 0;
	int num_lines 				= 0;
	int num_read				= 0;

	size_t buffer_length;

	char *scratch  				= NULL;
	char *scratch_begin 		= NULL;
	char *header_buffer 		= NULL;
	char *buffer				= NULL;
	char *data_buffer			= NULL;
	char *view_cache			= NULL;
	char *tmp_buffer			= NULL;
	char *ch					= NULL;
	char *newline				= NULL;
	char *trial;

	char file_name[32];
	char trackline_name[MAX_NAME_LENGTH];
	char var_min_trackline[32];
	char var_max_trackline[32];
	char var_min_latlon[32];
	char var_max_latlon[32];
	char val_min_lat[32];
	char val_max_lat[32];
	char val_min_long[32];
	char val_max_long[32];
	char error_buf[MAX_ERRSTR_BUFFER];	
	char trackline_option		= 'y';
	char diff_var_option		= '0';
	char var_option				= 'y';
	char clip_option			= '\0';

	char*	lat_lon_limits[]	= {"-181", "180", "-91", "90"};

	/* Check Usage */
	if(argc == 1){
		print_help();
		exit(1);
	}

	/* Allocate scratch buffer */
	scratch_begin = scratch = (char *)malloc(SCRATCH_SIZE);
	data_buffer = (char *)malloc((size_t)BUFFER_SIZE);
	trial = (char *)malloc(16);

	if(!scratch){
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "scratch buffer");
		err_disp();
		exit(1);
	}
	if(!data_buffer){
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "data buffer");
		err_disp();
		exit(1);
	}


	/* BINS AND VIEWS:

	lookup_table bin: This bin is an index into the data file.
		It has a variable in it called offset which gives the offset in bytes
		to the beginning of each indexed block of data. These offsets are accessed
		through an array in the header view.

	input_data bin: This bin is the data file of track lines associated with 
	    the lookup table. An eventual view is created called vector which uses
		the offset_array view rowsize structure to access track lines.		
			
	*/

	/* Create the header list bin. */
	lookup_table = make_dbin("lookup_table");

	if(!lookup_table){
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, "Header list");
		err_disp();
		exit(1);
	}
	error =	db_set(lookup_table,
			BUFFER,				scratch,
			DBIN_FILE_NAME,		*(argv + 1),
			DBIN_FILE_HANDLE,
			FORMAT_LIST,		NULL, NULL,
			DBIN_CACHE_SIZE,	BUFFER_SIZE,
			END_ARGS);

	if(error){
		err_push(ROUTINE_NAME, ERR_SET_DBIN, "Lookup table");
		err_disp();
		exit(1);
	}

	/* A variable named offset needs to exist */

	offset_var = ff_find_variable("offset", lookup_table->input_format);   

	if(offset_var == NULL){
			err_push(ROUTINE_NAME, ERR_VARIABLE_NOT_FOUND, "offset");
			err_disp();
			exit(1);
	}

	/* Fill the cache for the lookup_table */
	error =	dbin_event(lookup_table,
			DBIN_FILL_CACHE,
			0);

	if (error) {
		err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Fill lookup table cache");
		err_disp();
		exit(1);
	}
	
	/* Convert binary lookup table to ascii for possible viewing in the
	   menu options */
	error = db_show(lookup_table,
			DBIN_BYTE_COUNTS, &bytes_to_read, &bytes_to_write,
			END_ARGS);

	if(error) {
		err_push(ROUTINE_NAME, ERR_SHOW_DBIN, "Finding bytes to read/write");
		err_disp();
		exit(1);
	}
	if(bytes_to_write > UINT_MAX){
		err_push(ROUTINE_NAME, ERR_WRITE_FILE, "Output buffer to large");
		err_disp();
		exit(1);
	}
  	view_cache = (char *)malloc((size_t)(bytes_to_write + 1));

	if(!view_cache) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "view_cache");
		err_disp();
		exit(1);
	}

	error = dbin_event(lookup_table,
		DBIN_CONVERT_CACHE, view_cache,
		(FORMAT_PTR)NULL, (long*)&trial_long_ptr,
		END_ARGS);

	if (error) {
		err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Converting lookup table cache");
		err_disp();
		exit(1);
	}

	/* END OF THE LOOKUP TABLE BIN DEFINITION */

	/* Define the input file data bin */
	input_data = make_dbin("input_data");

	if(!input_data){
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, "Input data");
		err_disp();
		exit(1);
	}

	/* Set the input data file and Format for the input data file 
	   Cache size is allocated to support trackline data retrieval later
	   in the program */
/*	error =	db_set(input_data,
		BUFFER, scratch,
		DBIN_FILE_NAME,		*(argv + 2),
		FORMAT_LIST,		(char*)NULL, (char*)NULL,
		OUTPUT_FORMAT,		*(argv + 3),NULL,
		DBIN_CACHE_SIZE, 	VECTOR_CACHE_SIZE,
		DBIN_FILE_HANDLE,
		CHECK_FILE_SIZE,
		END_ARGS);

	if(error){
		err_push(ROUTINE_NAME, ERR_SET_DBIN, "Input data");
		err_disp();
		exit(1);
	}
*/
	error =	db_set(input_data,
		BUFFER, scratch,
		DBIN_FILE_NAME,		*(argv + 2),
		INPUT_FORMAT,		*(argv + 3),NULL,
		OUTPUT_FORMAT,		*(argv + 4),NULL,
		DBIN_CACHE_SIZE, 	VECTOR_CACHE_SIZE,
		DBIN_FILE_HANDLE,
		CHECK_FILE_SIZE,
		END_ARGS);

	if(error){
		err_push(ROUTINE_NAME, ERR_SET_DBIN, "Input data");
		err_disp();
		exit(1);
	}

	/* Make sure a variable named line_number exists 
	   needed for further reference */

	/* END OF THE INPUT DATA BIN DEFINITIONS */

	/* A data bin has been created for the offsets. The offsets are an
	array within this file. An array view needs to be created to provide
	access to this array */
	offset_array = make_view("offsets", scratch);

	if(!offset_array){
		err_push(ROUTINE_NAME, ERR_MAKE_VIEW, "Offset array");
		err_disp();
		exit(1);
	}

	/* Find number of records (track lines) in lookup table */
	num_recs = (lookup_table->cache_end - lookup_table->cache) / lookup_table->input_format->max_length;

	error = set_view(offset_array,
		VIEW_DATA_BIN,		lookup_table,
		VIEW_NUM_POINTERS,	1,
		VIEW_INCREMENT,		lookup_table->input_format->max_length, /* record length */
		VIEW_FIRST_POINTER, lookup_table->cache,
		VIEW_LENGTH,		num_recs,
		VIEW_TYPE,			(long)V_TYPE_ARRAY,
		END_ARGS);

	/* define a VECTOR data view */
	vector = make_view("Vector", scratch);

	if(!vector){
		err_push(ROUTINE_NAME, ERR_MAKE_VIEW, "Vector");
		err_disp();
		exit(1);
	}
	/* Point the View at the input data file and Set the view type to VIEW_VECTOR */

	error = set_view(vector,
			VIEW_DATA_BIN,	input_data,
			VIEW_TYPE,		(long)V_TYPE_VECTOR,
			END_ARGS);

	if (error) {
		err_push(ROUTINE_NAME, ERR_SET_VIEW, "Vector");
		err_disp();
		exit(1);
    }

	/* Preliminary setup of views and data bins is finished. Can now process
	   data through various menu options */

	while (option == VIEW_TRACKLINES) {
		print_menu(&option);

		switch (option) {
	
			case VIEW_TRACKLINES:

				(void) scrltext(view_cache);
				if(view_cache) {
					free(view_cache);
					view_cache = NULL;
				}			
				break;

			case TRACKLINE_RETRIEVAL:
		
				/* Retrieve specific track lines or ranges of track lines */
					
				system("cls");
				fprintf(stderr,"Maximum number of track lines to\n");
				fprintf(stderr,"retrieve in one program run is 256\n");
				while (trackline_option == 'Y' || trackline_option == 'y') {

					if (diff_var_option != '0') {
						fprintf(stderr,"Enter a different variable name to search on (y or n)?");
						var_option = getch();
						fprintf(stderr,"\n");
					}
					diff_var_option = '1';
					if (var_option == 'Y' || var_option == 'y') {
						fprintf(stderr,"Please enter the variable name on which to\n");
						fprintf(stderr,"receive information: ");
						scanf("%s",trackline_name);
						trackline_var = ff_find_variable(trackline_name, lookup_table->input_format);
						if(trackline_var == NULL){
							err_push(ROUTINE_NAME, ERR_VARIABLE_NOT_FOUND, trackline_name);
							err_disp();
							exit(1);
						}
					}
								
					/* values are entered -- not var names. ie think of 
					   var_min_trackline as val_min_trackline */

					fprintf(stderr,"Please enter the minumum value of %s: ", trackline_var->name);
					scanf("%s", var_min_trackline);
				
					fprintf(stderr,"Please enter the maximum value of %s: ", trackline_var->name);
					scanf("%s", var_max_trackline);
				
					sprintf(scratch, "%s %s %s\n", trackline_var->name, var_min_trackline, var_max_trackline);
					scratch += strlen(scratch);
			
					fprintf(stderr,"Enter another track line value/range (y or n)?");
					trackline_option = getch();
					fprintf(stderr,"\n");
				}
					break;

			case LAT_LON_RETRIEVAL:
			
					/* Retrieve track lines intersecting a latitude and longitude
					   rectangle. A clip option is also used in which
					   only portions of intersecting data in each track line
					   are retrieved -- rather than the entire trackline. There
					   is a specific setup of entered variables and values so
					   intersection is determined correctly in dv_events.
					*/
					system("cls");
					fprintf(stderr,"Maximum number of track lines to retrieve in a\n");
					fprintf(stderr,"specified latitude\\longitude range is 256\n\n");
					fprintf(stderr,"Enter latitude and longitude in decimal format\n");
			
					fprintf(stderr,"Please enter the minimum longitude variable name: ");
					scanf("%s",var_min_latlon);
					fprintf(stderr,"Please enter the limit for %s: ",var_min_latlon);
					scanf("%s",val_min_long);
					fprintf(stderr,"Please enter the maximum longitude variable name: ");		
					scanf("%s",var_max_latlon);
					fprintf(stderr,"Please enter the limit for %s: ",var_max_latlon);
					scanf("%s",val_max_long);
					sprintf(scratch,"%s %s %s\n%s %s %s\n", var_min_latlon, lat_lon_limits[0], val_max_long,
															var_max_latlon, val_min_long, lat_lon_limits[1]);
					scratch += strlen(scratch);
			
					fprintf(stderr,"Please enter the minimum latitude variable name: ");
					scanf("%s",var_min_latlon);
					fprintf(stderr,"Please enter the limit for %s: ",var_min_latlon);
					scanf("%s",val_min_lat);
					fprintf(stderr,"Please enter the maximum latitude variable name: ");		
					scanf("%s",var_max_latlon);
					fprintf(stderr,"Please enter the limit for %s: ",var_max_latlon);
					scanf("%s",val_max_lat);
					sprintf(scratch,"%s %s %s\n%s %s %s\n", var_min_latlon, lat_lon_limits[2], val_max_lat,
															var_max_latlon, val_min_lat, lat_lon_limits[3]);
					system("cls");
					fprintf(stderr,"Clip the track lines on lat lon values (y or n)?");
					clip_option = getch();
					fprintf(stderr,"\n\n");

					break;

			case TRACKLINE_FILE:

				/* Get track lines to retrieve from a file. Each line of the
				   file must contain three entries: the designated variable
				   a min variable (track line) value and a max variable (track line) value. */
				tmp_buffer = (char *)malloc(TEMP_BUFFER_SIZE);
				fprintf(stderr,"Enter name of trackline input file: ");
				scanf("%s",file_name);
				/* Read the file into the data_buffer */
				buffer_length = ff_file_to_buffer(file_name, data_buffer);
		
				if(buffer_length == 0){
					err_push(ROUTINE_NAME,ERR_READ_FILE,file_name);
					err_disp();
				    exit(1);
				}

				/* Read these values with a proper format into the scratch_buffer
				   and eventually use the scratch buffer to make a parameter list */
				ch = data_buffer;
				while((ch = strtok((num_lines++) ? NULL : ch, "\n")) != NULL){

					/* the vars are actually min and max track line values 
					   as in TRACKLINE_RETRIEVAL option */
  					num_read = sscanf(ch,"%s %s %s", trackline_name, var_min_trackline, var_max_trackline);

					/* Do some error checking on the track line file */
					if(num_read == -1) continue;	/* skip blank lines */
			
					if(num_read != 3){
						newline = strchr(ch, '\015');
						if(newline) *newline = ' ';
						sprintf(error_buf,"\nLine ->%s<- Does Not Have 3 Entries\n",ch);
						err_push(ROUTINE_NAME,ERR_READ_FILE,error_buf);
						err_disp();
						exit(1);
					}
					trackline_var = ff_find_variable(trackline_name, lookup_table->input_format);
					if(trackline_var == NULL){
						err_push(ROUTINE_NAME, ERR_VARIABLE_NOT_FOUND, trackline_name);
						err_disp();
						exit(1);
					}

					sprintf(scratch,"%s %s %s\n",trackline_var->name, var_min_trackline, var_max_trackline);
					scratch += strlen(scratch);

				}

				break;

			case QUICK_EXIT:

				exit(0);

			default:

				err_push(ROUTINE_NAME, ERR_UNKNOWN_SECTION, NULL);
				err_disp();
				exit(1);
		}
	}

	/* Make a parameter list associated with offset_array. Each node in the
	   parameter list contains the track line variable name and a max and min
	   value for the track line.
	*/
	error = set_view(offset_array,
			VIEW_PARAMETER_LIST, scratch_begin,
			0);

	if(error){
		err_push(ROUTINE_NAME, ERR_SET_VIEW, "Making Parameter list");
		err_disp();
		exit(1);
	}

	/* If retrieving on track line values, determine if all
	   entered values are valid before continuing
	*/
	if (option != LAT_LON_RETRIEVAL) {
		error = check_tracklines(offset_array);
		if(error) {
			fprintf(stderr,"\nProgram Exit-- The following variable value was not found:\n");
			print_trackline(offset_array, trackline_var, option);
			exit(1);
		}
	}

	/* Set up the rowsize structure in offset_array. This structure contains
	   the start position (offset) and number of bytes of each desired track
	   line to retrieve	in the file
	*/
	if (option == LAT_LON_RETRIEVAL)
		error = dv_events(offset_array, MAKE_ROWSIZE_AREA_INTERSECT, 0);
	else
		error = dv_events(offset_array, VIEW_PLIST_TO_ROWSIZE, 0);

	if ((error) && (option == LAT_LON_RETRIEVAL)) {
		err_push(ROUTINE_NAME, ERR_FIND_INTERSECT, "Between latitude longitude");
		err_disp();
		exit(1);
	} 
	else if (error) {
		err_push(ROUTINE_NAME, ERR_CONVERT, "Converting plist to rowsize");
		err_disp();
		exit(1);
	}
	
	/* Open the output file to place desired track line data */
	output_file = open(*(argv + 5), O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	if (output_file == -1) {
		err_push(ROUTINE_NAME, errno, "Problem opening output file");
		err_disp();
		exit(1);
	}
	data_file = vector->dbin->data_file;
	/* No header information needed */
 	error = ff_skip_header(vector->dbin->input_format, data_file);
	if (err_state()) {
		err_push(ROUTINE_NAME, ERR_SKIP_HEADER, "data_file");
		err_disp();
		exit(1);
	}

	if (clip_option == 'y'|| clip_option == 'Y') {

		/*  Need to enter the latitude and longitude variable names
			from the data file, so the values representing those variables
			can be clipped if necessary */
		system("cls");
		fprintf(stderr,"Please enter the longitude variable name from data\n");
		fprintf(stderr,"file format to clip on: ");
		scanf("%s",var_min_latlon);
		fprintf(stderr,"Please enter the latitude variable name from data\n");
		fprintf(stderr,"file format to clip on: ");
		scanf("%s",var_max_latlon);

		sprintf(scratch_begin,"%s %s %s\n%s %s %s\n",
				var_min_latlon, val_min_long, val_max_long,
				var_max_latlon, val_min_lat, val_max_lat);

		/*	Reset the vector view to allow VIEW_GET_DATA to work properly
			and make a new parameter list corresponding to the vector view.
			The parameter list contains the latitude and longitude variable
			names of the data file and the max and min values to clip on */

		error = set_view(vector,
				VIEW_FIRST_POINTER,		input_data->cache,
				VIEW_INCREMENT,			input_data->input_format->max_length,
				VIEW_PARAMETER_LIST,	scratch_begin,
				VIEW_TYPE,				(long)V_TYPE_ARRAY,
				0);
	
		if(error){
			err_push(ROUTINE_NAME, ERR_SET_VIEW, "Resetting vector view to V_TYPE_ARRAY");
			err_disp();
			exit(1);
		}
	}
saved_num_recs_in_cache = input_data->records_in_cache;
	for (j=0, rowsize=offset_array->row_sizes;
		j<offset_array->num_pointers;
		j++,rowsize++){

		if((clip_option == 'y') || (clip_option == 'Y')) 
   			fprintf(stderr,"Retrieving and clipping track line of %ld binary bytes\n",rowsize->num_bytes);
		else
			fprintf(stderr,"Retrieving track line of %ld binary bytes\n",rowsize->num_bytes);

		/* set the data available flag to the row size and the file
		position to the start of the row (offset) */
		error = db_set(vector->dbin,
			DBIN_FILE_POSITION, rowsize->start,
			DBIN_DATA_AVAILABLE,rowsize->num_bytes,
			END_ARGS);
		if (error) {
			err_push(ROUTINE_NAME, ERR_SET_DBIN, "Setting start and size");
			err_disp();
			exit(1);
		}

		while(vector->dbin->data_available){
			error = dbin_event(vector->dbin,
				PROCESS_FORMAT_LIST, FFF_ALL_TYPES,
				END_ARGS);

			if(error) {
				err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Process format list");
				err_disp();
				exit(1);
			}

			/* Clip the view if necessary */
			if (clip_option == 'y' || clip_option == 'Y') {
				error = dv_events(vector, CLIP_VIEW_INTERSECT, 0);
				if(error){
					err_push(ROUTINE_NAME, ERR_SET_VIEW, "Clipping view");
					err_disp();
					exit(1);
				}
			}				

			/* Convert the cache into the output buffer */
			error = db_show(vector->dbin,
				DBIN_BYTE_COUNTS, &bytes_to_read, &bytes_to_write,
				END_ARGS);
			if(error) {
				err_push(ROUTINE_NAME, ERR_SHOW_DBIN, "Finding bytes to read/write");
				err_disp();
				exit(1);
			}
			if(bytes_to_write > UINT_MAX){
				err_push(ROUTINE_NAME, ERR_WRITE_FILE, "Output buffer to large");
				err_disp();
				exit(1);
			}

			if(bytes_to_write > data_buffer_size){
				data_buffer = realloc(data_buffer, (size_t) bytes_to_write);
				if (!data_buffer) {
					err_push(ROUTINE_NAME, ERR_MEM_LACK, "Realloc data buffer");
					err_disp();
					exit(1);
				}
				data_buffer_size =  bytes_to_write;
			}
			
			error = dbin_event(vector->dbin,
				DBIN_CONVERT_CACHE, data_buffer,
				(FORMAT_PTR)NULL, (long*)&trial_long_ptr,
				END_ARGS);
			if(error) {
				err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Problem converting cache");
				err_disp();
				exit(1);
			}

			error = write(output_file, data_buffer, (size_t)bytes_to_write);
			if (error == -1) {
				err_push(ROUTINE_NAME, errno, "Problem writing data_buffer to file");
				err_disp();
				exit(1);
			}
			input_data->records_in_cache = saved_num_recs_in_cache;			
		}

	}
}

void print_menu(int *option_ptr)
{
	/* Menu options for gettrack */
	
		system("cls");
		fprintf(stderr,"Track line options are as follows:\n");
		fprintf(stderr,"	1 -- Window view into flightline index file\n");
		fprintf(stderr,"	2 -- Specific/range trackline retrieval\n");
		fprintf(stderr,"	3 -- Range of latitude/longitude trackline retrieval\n");
		fprintf(stderr,"		 (Possible clip if necessary)\n");
		fprintf(stderr,"	4 -- Trackline retrieval using a targeted trackline input file\n");
		fprintf(stderr,"	5 -- Quick Exit from program\n");
		fprintf(stderr,"\n\nEnter an option: ");
		scanf("%d",option_ptr);

}

int print_trackline(DATA_VIEW_PTR view, VARIABLE_LIST_PTR var, int option)
{
	/* Used to print an invalid trackline */

	char			trackline_buffer[32];
	short length	= 0;

	switch (var->type){
	case FFV_LONG:
		fprintf(stderr,"%ld\n", (*(long *)view->data));
		break;

	case FFV_SHORT:
		fprintf(stderr,"%hd\n", (*(short *)view->data));
		break;

	case FFV_USHORT:
		fprintf(stderr,"%hu\n", (*(unsigned short *)view->data));
		break;

	case FFV_UCHAR:
		fprintf(stderr,"%u\n", (*(unsigned char *)view->data));
		break;

	case FFV_ULONG:
		fprintf(stderr,"%lu\n", (*(unsigned long *)view->data));
		break;

	case FFV_CHAR:
		length = var->end_pos - var->start_pos + 1;
		strncpy(trackline_buffer, view->data, length);
		memset((trackline_buffer + length), '\0', 1);
		fprintf(stderr,"%s\n", trackline_buffer);
		break;

	case FFV_DOUBLE:
		fprintf(stderr,"%lf\n", (*(double *)view->data));
		break;

	case FFV_FLOAT:
		fprintf(stderr,"%f\n", (*(float *)view->data));
		break;

	default:
		err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE, var->name);
		return(1);
	}

} 

int check_tracklines(DATA_VIEW_PTR view)
{
	/* error check for valid track lines entered in plist */	

	short min_found;
	short max_found;
	short length;
	char* data_ptr  = NULL;
	PARAM_LIST_PTR pl;

	fprintf(stderr,"Checking for valid track lines...\n");
	pl = view->p_list;
	while (pl) {
		min_found = 0;
		max_found = 0;

			while ((set_view(view, VIEW_GET_DATA, 0) != EOF) &&
				   !(min_found && max_found)) {

					data_ptr = (view->data + pl->var->start_pos - 1);
			
					switch (pl->var->type){
					case FFV_LONG:
						if(*((long *)data_ptr) == *((long *)pl->minimum))
							min_found = 1;							
						if(*((long *)data_ptr) == *((long *)pl->maximum)) 
							max_found = 1;
						break;
				
					case FFV_SHORT:
						if(*((short *)data_ptr) == *((short *)pl->minimum))
							min_found = 1;
						if(*((short *)data_ptr) == *((short *)pl->maximum))
							max_found = 1;
						break;
				
					case FFV_USHORT:
						if(*((unsigned short *)data_ptr) == *((unsigned short *)pl->minimum))
							min_found = 1;
						if(*((unsigned short *)data_ptr) == *((unsigned short *)pl->maximum))
							max_found = 1;
						break;
			
					case FFV_UCHAR:
						if(*((unsigned char *)data_ptr) == *((unsigned char *)pl->minimum))
							min_found = 1;
						if(*((unsigned char *)data_ptr) == *((unsigned char *)pl->maximum))
							max_found = 1;
						break;
			
					case FFV_ULONG:
						if(*((unsigned long *)data_ptr) == *((unsigned long *)pl->minimum))
							min_found = 1;
						if(*((unsigned long *)data_ptr) == *((unsigned long *)pl->maximum))
							max_found = 1;
						break;
				
					case FFV_CHAR:
						length = pl->var->end_pos - pl->var->start_pos + 1;
						if(strncmp((char *)data_ptr, (char *)pl->minimum, length) == 0)
							min_found = 1;
						if(strncmp((char *)data_ptr, (char *)pl->maximum, length) == 0)
							max_found = 1;
						break;
				
					case FFV_DOUBLE:
						if(*((double *)data_ptr) == *((double *)pl->minimum))
							min_found = 1;
						if(*((double *)data_ptr) == *((double *)pl->maximum))
							max_found = 1;
						break;
				
					case FFV_FLOAT:
						if(*((float *)data_ptr) == *((float *)pl->minimum))
							min_found = 1;
						if(*((float *)data_ptr) == *((float *)pl->maximum))
							max_found = 1;
						break;
				
					default:
						err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE, pl->var->name);
						return(1);
					}

				if (min_found && max_found) {
					pl = pl->next;
					break;
				}
			}	/* End of while(set_view) */

			/* view->data is set to the FIRST determined invalid track line
			   (if an invalid track line exists) allowing print_trackline to
			   print the invalid track line correctly */

			if (!min_found){
				view->data = (char *)pl->minimum;
				return(1);
			}
			else if (!max_found) {
				view->data = (char *)pl->maximum;
				return(1);
			}
		view->data = NULL;

	} /* end while(pl) */

	fprintf(stderr,"All entered track line values are valid\n");
	return(0);
}

void print_help(void) 
{

	fprintf(stderr,"USAGE:\n");
	fprintf(stderr,"  gettrack bin_index_file bin_data_file input_format output_format output_file\n\n");
	fprintf(stderr,"The formats for bin_index_file are located according to freeform finding\n");
	fprintf(stderr,"native format rules\n\n");
	fprintf(stderr,"bin_index_file   = binary index file of flightline information\n");
	fprintf(stderr,"bin_data_file  	 = binary data file of trackline information\n");
	fprintf(stderr,"input_format     = binary_data_file input format\n");
	fprintf(stderr,"output_format	 = desired ASCII format for viewing retrieved tracklines\n");
	fprintf(stderr,"                   (generally ASCII output format for bin_data_file)\n");
	fprintf(stderr,"output_file      = designated output file for retrieved tracklines\n\n");
	fprintf(stderr,"!!!NOTE!!! THE NORMAL VERSION OF GETTRACK FINDS FORMATS ACCORDING\n");
	fprintf(stderr,"TO FREEFORM FINDING FORMAT RULES: THIS VERSION IS SLIGHTLY CHANGED\n");
	fprintf(stderr,"TO ALLOW THE SPECIFICATION OF DATA INPUT AND OUTPUT FORMATS\n\n");
	fprintf(stderr,"When entering targeted tracklines to retrieve from a file (menu option 4),\n");
	fprintf(stderr,"the file must have the following format for each line:\n"); 
	fprintf(stderr,"variable_name   min_variable_value   max_variable_value\n");

}

