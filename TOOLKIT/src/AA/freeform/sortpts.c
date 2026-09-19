/*
 * NAME:	sortpts
 *		
 * PURPOSE:	sortpts is a program that sorts binary point data
 *			into a 2 dimensional grid of boxes.
 *			The program gets information about the variables from a file
 *			which is specified as the first argument. The File has two lines,
 *			which contain the name, min, max, and bin sizes
 *			for the x (line 1) and y (line 2) axes.
 *
 * USAGE :	sortpts  binary_input_file  [binary_format_file] < variable file > output
 *			sortpts writes the data to a file called ptdata.bin
 *			and the index to a file called ptdata.ind
 *
 * RETURNS:	
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc1.colorado.edu
 *          Mark Van Gorp, NGDC, (303) 497 - 6221, mvg@kryton.colorado.edu
 *				- major revision (Nov. 1992)
 *
 * COMMENTS: This version of sortpts reads and writes an old index
 *	structure:	
 *	The index file now has a header which is 161 bytes long.
 *	The header contains:
 *		number of levels:	unsigned char which is a constant 2
 *		xname:		x variable name
 *		xmin:		long integer with three characters of precision
 *		xmax:		long integer with three characters of precision
 *		xsize:		long integer with three characters of precision
 *		num_xbox:	long integer representing number of x boxes
 *		yname:		y variable name
 *		ymin:		long integer with three characters of precision
 *		ymax:		long integer with three characters of precision
 *		ysize:		long integer with three characters of precision
 *		num_ybox:	long integer representing number of y boxes
 *
 *	The feature added to this version is variable flexibility.
 *
 *	Following the header are the offsets to each box.
 *	The first box is in the lower left hand corner and the
 *	last is in the upper right hand corner. File size is the last long
 *  written to the file
 *
 * KEYWORDS:	
 *
 */

#include <stdio.h>

#include <process.h>
#include <stdlib.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA
#include <databin.h>
#include <dataview.h>

/* Define the linear list of sequences */
struct seq_node {
	long start;
	long end;
	struct seq_node *next;
}seq_node;
typedef struct seq_node SEQUENCE;
typedef SEQUENCE* SEQUENCE_PTR;

/* Function Prototypes: */
int sp_find_record(short, char*, long, long, unsigned long, VARIABLE_PTR, VARIABLE_PTR);
int add_size(char *, void *);
void add_sequence(SEQUENCE, long);
int out_of_range(char*, VARIABLE_PTR, VARIABLE_PTR, unsigned long);
int print_box_array(void);
extern int ff_compare_variables(char *record_1, char *record_2);
static int w_ff_compare_variables(const void *record_1, const void *record_2);

#undef ROUTINE_NAME
#define ROUTINE_NAME "sortpts"
#define SORTPTS_CACHE_SIZE 61440
#define FORWARD 0
#define BACKWARD 1
/*#define ARRAY_ADJUST 131072*/

char *input_buffer					= NULL;

int record_length;
unsigned short head_length = 0;
int index_file; /* ptdata.ind */
int tmp_file;
long num_boxes;
int out_of_range_flag			= 0;

unsigned long num_sequences = 0;

DATA_VIEW_PTR view;
SEQUENCE_PTR HUGE *box_array          = NULL;

main(int argc, char **argv)
{
	char file_name[MAX_NAME_LENGTH];
	char *format_file				= NULL;
	char *xname						= NULL;
	char *yname						= NULL;
	char *scratch_buffer			= NULL;
	char *xymin_buffer				= NULL; /* used by sp_find_record for ff_compare_variables to find first */
	char *xymax_buffer				= NULL; /* and last intersecting records in each box */
	char *y_start_ptr				= NULL; /* points to beginning of first intersecting y record in box */
	char *x_start_ptr				= NULL; /* points to beginning of first intersecting x record in box */
	char *x_end_ptr					= NULL; /* points to beginning of last intersecting x record in box */
	char *y_end_ptr					= NULL; /* points to beginning of last intersecting y record in box */
	char *save_cache_end			= NULL; /* view->dbin->cache_end is adjusted for box boundaries */

	DATA_BIN_PTR input_data			= NULL;
	FFF_STD_ARGS std_args;

	int error						= 0;
	int i,j;
	int perc_left					= 0;

	long x_start_rec				= 0; /* number of first intersecting x record in box */
	long x_end_rec					= 0; /* number of last intersecting x record in box */
	long y_start_rec				= 0; /* number of first intersecting y record in box */ 
	long y_end_rec					= 0; /* number of last intersecting y record in box */ 
	long num_y_records				= 0; /* number of y intersecting records in a y box */
	long cache_end_rec				= 0; /* number of last record in cache */
	long index						= 0; /* box number */
	long cache_offset				= 0;
	long prev_cache_size			= 0;
	long position					= 0;
	long lxnum;				/* used to write out y coordinate data */
	long lynum;				/* used to write out x coordinate of data */
	long file_size					= 0L;


	double	xmax;							/* x maximum */
	double	ymax;							/* y maximum */
	double  ybox_boundary;					/* each ybox boundary: |ymin,ybox_boundary| */
	double  xbox_boundary;					/* each xbox boundary: |xmin,xbox_boundary| */
	double	xmin;							/* x minimum */
	double	ymin;							/* y minimum */
	double  save_xmin;
	double  save_ymin;
	double  xy_ymin;						/* ymin matching data */
	double  xy_ymax;						/* ymax matching data */
	double  xy_xmin;						/* xmin matching data */
	double  xy_xmax;						/* xmax matching data */
	double  original_ymin;					/* saves ymin value */
	double  original_xmin;					/* saves xmin value */
	double	xsize;							/* size of x boxes */
	double	ysize;							/* size of y boxes */
	double	xrange;							/* width of x axis */
	double	yrange;							/* width of y axis */
	double  exponent;						/* used to adjust cache to a
											   power of two if necessary */

	ROW_SIZES_PTR y_rowsizes_struct = NULL; /* offset and number of intersecting
	                                           bytes for each y box */
	ROW_SIZES_PTR row_sizes			= NULL;

	SEQUENCE seq;

	unsigned char number_of_levels	= 2;	/* 2 dimensional */
	unsigned int num_bytes			= 0;
	unsigned	num_xbox;					/* Number of x boxes */
	unsigned	num_ybox;					/* Number of y boxes */

	VARIABLE_PTR x_format_var = NULL;
	VARIABLE_PTR y_format_var = NULL;

	if(argc == 1){
		fprintf(stderr,"\nUSAGE: sortpts  binary_input_file  [binary_format_file] < variable_file > output\n\n");
		fprintf(stderr,"Sortpts is a program that sorts binary point data\n");
		fprintf(stderr,"into a 2 dimensional grid of boxes.\n");
		fprintf(stderr,"Sortpts gets information about the variable values in an input file\n");
		fprintf(stderr,"by using a variable_file. The variable_file has two lines,\n");
		fprintf(stderr,"each containing a variable name, min, max, and bin (box) size\n");
		fprintf(stderr,"for the x (line 1) and y (line 2) axes.\n");
		fprintf(stderr,"     ie.     xvar_name -10 10 2\n");
		fprintf(stderr,"             yvar_name -5 5 1\n\n");
		fprintf(stderr,"        sortpts writes the data to a file called ptdata.bin\n");
		fprintf(stderr,"        and the index to a file called ptdata.ind\n");
		fprintf(stderr,"(A second file called ptdata.tmp is also generated for\n");
		fprintf(stderr,"temporary data storage)\n\n");
		fprintf(stderr,"If the quotients of (xmax - xmin)/xsize and (ymax - ymin)/ysize are not\n");
		fprintf(stderr,"whole numbers, the xmax and/or ymax is increased allowing both quotients\n");
		fprintf(stderr,"to become whole numbers\n");
		exit(1);
	}

	xname = (char *)malloc((size_t)(MAX_NAME_LENGTH + 1));
	yname = (char *)malloc((size_t)(MAX_NAME_LENGTH + 1));
	if((!xname) || (!yname)) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, ((!xname) ? "xname" : "yname"));
		err_disp();
		exit(1);
	}
	input_buffer = (char *)malloc((size_t)DEFAULT_CACHE_SIZE);
	if (!input_buffer) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "input_buffer");
		err_disp();
		exit(1);
	}

	scratch_buffer = (char *)malloc((size_t)(DEFAULT_BUFFER_SIZE));
	if (!scratch_buffer) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Scratch_buffer");
		err_disp();
		exit(1);
	}
	y_rowsizes_struct = (ROW_SIZES_PTR)malloc(sizeof(ROW_SIZES));
	if(!y_rowsizes_struct) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "y_rowsizes_struct");
		err_disp();
		exit(1);
	}

	scanf("%s %lf %lf %lf", xname, &xmin, &xmax, &xsize); 
	scanf("%s %lf %lf %lf", yname, &ymin, &ymax, &ysize); 

	printf("X: name, min, max, size mult: %s %g %g %g\n", xname, xmin, xmax, xsize);
	printf("Y: name, min, max, size mult: %s %g %g %g\n", yname, ymin, ymax, ysize);
	fprintf(stderr,"X: name, min, max, size mult: %s %g %g %g\n", xname, xmin, xmax, xsize);
	fprintf(stderr,"Y: name, min, max, size mult: %s %g %g %g\n", yname, ymin, ymax, ysize);

	/* Quick fix for sortpts -- to save values that get changed */
	save_xmin = xmin;
	save_ymin = ymin;

	xrange = xmax - xmin;
	yrange = ymax - ymin;
	printf("xrange, yrange: %g %g\n", xrange, yrange);
	fprintf(stderr,"xrange, yrange: %g %g\n", xrange, yrange);

	num_xbox = (xrange/xsize);
	num_ybox = (yrange/ysize);
	if(xrange - (num_xbox * xsize) >= .000001) {
		++num_xbox;
		xmax = xmin + num_xbox * xsize;
		printf("X (%s) max expanded to: %g\n", xname, xmax);
		fprintf(stderr,"X (%s) max expanded to: %g\n", xname, xmax);
	}
	if(yrange - (num_ybox * ysize) >= .000001) {
		++num_ybox;
		ymax = ymin + num_ybox * ysize;
		printf("Y (%s) max expanded to: %g\n", yname, ymax);
		fprintf(stderr,"Y (%s) max expanded to: %g\n", yname, ymax);
	}
	printf("%d x boxes and %d y boxes\n", num_xbox, num_ybox);
	fprintf(stderr,"%d x boxes and %d y boxes\n", num_xbox, num_ybox);

	/* Halloc the sequence array 
	   Each index in the sequence array corresponds to a box in the
	   grid. Data is added to a box by linking sequences of intersecting data.
	*/
	num_boxes = (long)((long)(num_xbox * num_ybox) * sizeof(SEQUENCE_PTR));
/*	if (num_boxes > ARRAY_ADJUST) {
		exponent = log10(num_boxes) / log10(2.0);
		exponent = floor(exponent + 1);
		num_boxes = (long)(pow(2.0,exponent));
	}
*/
	box_array = (SEQUENCE_PTR HUGE *)halloc(num_boxes,1);
	num_boxes /= sizeof(SEQUENCE_PTR);
	if (!box_array) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "calloc sequence array");
		err_disp();
		exit(1);
	}
	fprintf(stderr, "There are %ld boxes\n", num_boxes );
	printf("There are %ld boxes\n", num_boxes );

	strcpy(file_name, *(argv+1));
	fprintf(stderr,"Sorting Data from file %s\n", file_name);

	if(argc > 2){
		format_file = (char *)malloc((size_t)(MAX_NAME_LENGTH+1));
		if (!format_file) {
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "tmp_buffer");
			err_disp();
			exit(1);
		}
		(void)strcpy(format_file, *(argv + 2));
	}
	
	if (parse_command_line(argc, argv, &std_args))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	std_args.cache_size = SORTPTS_CACHE_SIZE;
	if (make_standard_dbin(&std_args, &input_data, NULL))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
			
	if (input_data->header_format)
		head_length = input_data->header_format->max_length;
	record_length = FORMAT_LENGTH(input_data->input_format);

	x_format_var = ff_find_variable(xname, input_data->input_format);
	y_format_var = ff_find_variable(yname, input_data->input_format);

	if ((!x_format_var) || (!y_format_var)) {
		err_push(ROUTINE_NAME, ERR_VARIABLE_NOT_FOUND, ((!x_format_var) ? xname : yname));
		err_disp();
		exit(1);
	}	

	/* The following segment involving xy variables and buffers are used to
	   set up max's and min's in record sized buffers matching the data. These
	   buffers are used by ff_compare_variables in sp_find_record to shrink the
	   size of each box by searching forwards and backwards for records falling
	   outside the min and max range
	*/
	xy_ymin = ymin;
	xy_ymax = ymax;
	xy_xmin = xmin;
	xy_xmax = xmax;

	/* Adjusts user entered values to match data */	
	if ((x_format_var->type != FFV_FLOAT) && (x_format_var->type != FFV_DOUBLE)) {
		for(i=0; i<x_format_var->precision; i++) {
			xy_xmin *= 10;
			xy_xmax *= 10;
		}
	}
	if ((y_format_var->type != FFV_FLOAT) && (y_format_var->type != FFV_DOUBLE)) {
		for(i=0; i<y_format_var->precision; i++) {
			xy_ymin *= 10;
			xy_ymax *= 10;
		}
	}
	xymin_buffer = (char*)malloc((size_t)record_length);
	xymax_buffer = (char*)malloc((size_t)record_length);
	if((!xymin_buffer) || (!xymax_buffer)) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, ((!xymin_buffer) ? "xymin_buffer" : "xymax_buffer"));
		err_disp();
		exit(1);
	}

	/* Place the values into the buffers */
	sprintf(scratch_buffer,"%lf",xy_xmin);
	ff_string_to_binary(scratch_buffer, x_format_var->type, (xymin_buffer + (x_format_var->start_pos - 1)));
	sprintf(scratch_buffer,"%lf",xy_ymin);
	ff_string_to_binary(scratch_buffer, y_format_var->type, (xymin_buffer + (y_format_var->start_pos - 1)));
	sprintf(scratch_buffer,"%lf",xy_xmax);
	ff_string_to_binary(scratch_buffer, x_format_var->type, (xymax_buffer + (x_format_var->start_pos - 1)));
	sprintf(scratch_buffer,"%lf",xy_ymax);
	ff_string_to_binary(scratch_buffer, y_format_var->type, (xymax_buffer + (y_format_var->start_pos - 1)));
	
	view = dv_make("view", scratch_buffer);
	if(!view){
		err_push(ROUTINE_NAME, ERR_MAKE_VIEW, "view");
		err_disp();
		exit(1);
	}
	error = dv_set(view,
		VIEW_DATA_BIN,		input_data,
		BUFFER,				scratch_buffer,
		VIEW_NUM_POINTERS,  1,
		VIEW_INCREMENT,     record_length,
		VIEW_FIRST_POINTER, input_data->cache,
		VIEW_TYPE,          (long)V_TYPE_ARRAY,
		END_ARGS);

	if(error) {
		err_push(ROUTINE_NAME, ERR_SET_VIEW, "input_data view");
		err_disp();
		exit(1);
	}

	view->row_sizes = (ROW_SIZES_PTR)calloc(num_xbox,sizeof(ROW_SIZES));
	if(!view->row_sizes) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "view->row_sizes");
		err_disp();
		exit(1);
	}

	original_ymin = ymin;
	original_xmin = xmin;
	/* ptdata.tmp is a temporary file in which to place the sorted caches;
	   ptdata.tmp will be used by print_box_array to print intersecting data
	   to ptdata.bin;
	   The size of ptdata.tmp will be the same as the input file, so there
	   must be enough room for ptdata.tmp -- and also for ptdata.bin
	   and ptdata.ind later
	*/
	tmp_file = open("ptdata.tmp", O_RDWR | O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
	if (tmp_file == -1)
		tmp_file = open("ptdata.tmp", O_TRUNC | O_BINARY | O_RDWR);
	if (tmp_file == -1) {
		err_push(ROUTINE_NAME, errno, "ptdata.tmp");
		err_disp();
		exit(1);
	}
	file_size = input_data->bytes_available;
	/* loop on each cache of the input file */
	while (input_data->bytes_available) {

		/* reset ymin and ybox_boundary on each cache read */
		ybox_boundary = ymin = original_ymin;
		error = db_events(input_data, PROCESS_FORMAT_LIST, FFF_ALL_TYPES, END_ARGS);
		if (error) {
			err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Process format list");
			err_disp();
			exit(1);
		}

		/* Calculate percentage left to process and display */
		perc_left = ((float)input_data->bytes_available / file_size) * 100;
		if (out_of_range_flag) {
			fprintf(stderr,"\n");
			out_of_range_flag = 0;
		} 
		fprintf(stderr,"\r%8d %% left to process...", perc_left);

		/* Adjust cache offset for each cache read */
		cache_offset += prev_cache_size;
		prev_cache_size = input_data->records_in_cache;
		cache_end_rec = input_data->records_in_cache - 1;

		/* Sort entire cache on y column */
		key_var = y_format_var;
		qsort((void *)input_data->cache, (size_t)input_data->records_in_cache, 
			  (size_t)record_length, w_ff_compare_variables);

		/* Find first and last y records in range */
		y_start_rec = sp_find_record(FORWARD, xymin_buffer, 0, cache_end_rec,
									cache_offset, x_format_var, y_format_var);
		y_start_ptr = view->dbin->cache + y_start_rec * record_length;
		y_end_rec = sp_find_record(BACKWARD, xymax_buffer, cache_end_rec, y_start_rec,
									cache_offset, x_format_var, y_format_var);

		/* if no intersecting y data, then continue to next cache */
		if (y_end_rec < y_start_rec) 
			continue;

		save_cache_end = view->dbin->cache_end = view->dbin->cache + (y_end_rec+1) * (record_length) - 1;
		view->data =  y_start_ptr + y_format_var->start_pos - 1; 
		/* find the offset and size of each y_box in the cache
		   All x_boxes in this y_box will be computed before the
		   next y_box is considered
		*/
		for (i=0; i<num_ybox; i++) {
			/*  The last y_box (num_ybox - 1) also includes the max --
			    this is why .000001 is added */
			view->dbin->cache_end = save_cache_end;
			if (i == (num_ybox - 1))
				ybox_boundary = ymax + .000001;
			else
				ybox_boundary += ysize;
			view->var_ptr = y_format_var;
			if( dv_events(view,
						  MAXMIN_CACHE_TO_ROWSIZE,
						  (ROW_SIZES_PTR)y_rowsizes_struct,
						  (double*)&ymin, (double*)&ybox_boundary,
						  END_ARGS) ) {
				err_push(ROUTINE_NAME, ERR_DVIEW_EVENT, "\nMCTR-> determining intersecting y records");
				err_disp();
				exit(1);
			}
			num_y_records = y_rowsizes_struct->num_bytes / record_length;

			/* If no intersecting y records in this box, then go to the next y_box */
			if (num_y_records == 0) {
				ymin = ybox_boundary;
				continue;
			}
			y_end_rec = (y_rowsizes_struct->start / record_length) + num_y_records - 1;
			
			xbox_boundary = xmin = original_xmin;
			/* Sort the y_box on the x values */
			key_var = x_format_var;
			qsort((void*)(input_data->cache + y_rowsizes_struct->start), (size_t)num_y_records, 
				  (size_t)record_length, w_ff_compare_variables);
				
			/* Find first and last records in this single y_box range based upon x values 
			   Any records falling outside this range will be printed */
			x_start_rec = sp_find_record(FORWARD, xymin_buffer, (y_rowsizes_struct->start / record_length),
			                             y_end_rec, cache_offset, x_format_var, y_format_var);
			x_start_ptr = view->dbin->cache + x_start_rec * record_length;
			x_end_rec = sp_find_record(BACKWARD, xymax_buffer, y_end_rec, x_start_rec,
									   cache_offset, x_format_var, y_format_var);
			/* If no intersecting x data, then continue to next y_box */
			if (x_end_rec < x_start_rec) {
				ymin = ybox_boundary;
				y_start_ptr += y_rowsizes_struct->num_bytes;
				view->data = y_start_ptr + y_format_var->start_pos - 1;
				continue;
			}
			view->dbin->cache_end = view->dbin->cache + (x_end_rec+1) * record_length - 1;
			view->data = x_start_ptr + x_format_var->start_pos - 1;

			/* Convert this single y_box into a list of row_sizes
			   Each ROW_SIZE structure in the list corresponds to an x_box in which
			   the cache offset for the beginning of the x_box and the number of
			   bytes for intersecting records in that x_box is stored. */
			for (j=0,row_sizes=view->row_sizes; j<num_xbox; j++,row_sizes++) {

				/* The last x_box (num_xbox - 1) also includes the max --
				   this is why .000001 is added */

				if (j == (num_xbox - 1))
					xbox_boundary = xmax + .000001;
				else
					xbox_boundary += xsize;
				view->var_ptr = x_format_var;
				if( dv_events(view,
							  MAXMIN_CACHE_TO_ROWSIZE,
							  (ROW_SIZES_PTR)row_sizes,
							  (double*)&xmin, (double*)&xbox_boundary,
							  END_ARGS) ) {
					err_push(ROUTINE_NAME, ERR_DVIEW_EVENT, "\nMCTR-> determining intersecting x records");
					err_disp();
					exit(1);
				}
				xmin = xbox_boundary;

			} /* End for j: x cache_to_rowsize */	

			/* Now all the x_boxes for the current y_box have been defined in
			   view->row_sizes. All x_boxes with intersecting data must now
			   be stored in the box array */
			for (j=0,row_sizes=view->row_sizes;j<num_xbox;j++,row_sizes++) {
				index = i*num_xbox + j;
				if (row_sizes->num_bytes != 0) {
					seq.start = row_sizes->start / record_length + cache_offset;
					seq.end = seq.start + (row_sizes->num_bytes / record_length) - 1;
					add_sequence(seq, index);
					num_sequences++;
				}
			}
			y_start_ptr += y_rowsizes_struct->num_bytes;
			view->data = y_start_ptr + y_format_var->start_pos - 1;
			ymin = ybox_boundary;

		} /* End for i: y cache_to_rowsize */

		position = head_length + cache_offset * record_length;
		num_bytes = input_data->records_in_cache * record_length;
		if ((lseek(tmp_file,position,SEEK_SET)) == -1L) {
			err_push(ROUTINE_NAME, errno, "ptdata.tmp");
			err_disp();
			exit(1);
		}
		if ((write(tmp_file, input_data->cache, num_bytes)) != num_bytes) {
			err_push(ROUTINE_NAME, ERR_WRITE_FILE, "ptdata.tmp");
			err_disp();
			exit(1);
		}
		
	}

	index_file = open("ptdata.ind", O_WRONLY | O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
	/* Write header on index file */

	write(index_file,(unsigned char *)&number_of_levels, sizeof(unsigned char));
	write(index_file,xname,MAX_NAME_LENGTH);

	/* Adjust precision of variables to a power of three for ptdata.ind */

	/* write xmin */
	xmin = save_xmin;
	xmin *= 1000;
	lxnum = (xmin >= 0.0) ? xmin + .5 : xmin - .5;
	write(index_file, (char*)&lxnum, sizeof(long));

	/* write xmax */
	xmax *= 1000;
	lxnum = (xmax >= 0.0) ? xmax + .5 : xmax - .5;
	write(index_file, (char*)&lxnum, sizeof(long));

	/* write xsize */
	xsize *= 1000;
	lxnum = (long)xsize;
	write(index_file, (char*)&lxnum, sizeof(long));

	/* write num_xbox */
	lxnum = (long)num_xbox;
	write(index_file, (char*)&lxnum, sizeof(long));

	write(index_file,yname,MAX_NAME_LENGTH);

	/* write ymin */
	ymin = save_ymin;
	ymin *= 1000;
	lynum = (ymin >= 0.0) ? ymin + .5 : ymin - .5;
	write(index_file, (char*)&lynum, sizeof(long));

	/* write ymax */
	ymax *= 1000;
	lynum = (ymax >= 0.0) ? ymax + .5 : ymax - .5;
	write(index_file, (char*)&lynum, sizeof(long));

	/* write ysize */
	ysize *= 1000;
	lynum = (long)ysize;
	write(index_file, (char*)&lynum, sizeof(long));

	/* write num_ybox */
	lynum = (long)num_ybox;
	write(index_file, (char*)&lynum, sizeof(long));

	if (print_box_array()) {
		err_push(ROUTINE_NAME, ERR_WRITE_DATA, "array of boxes");
		err_disp();
		exit(1);
	}

}
int sp_find_record(short direction, char *record_buffer, long start_rec, long last_rec, unsigned long offset,
				VARIABLE_PTR x_format_var, VARIABLE_PTR y_format_var)
{ 
	/* Find the record offsets of the data in range
	   If the data is not in range then print the points to stderr
	*/
	
	int error = 0;
	 
	long num_recs;
	view->data = (view->dbin->cache + (start_rec * view->increment));

	if (direction == FORWARD) {
		while ((start_rec <= last_rec) && 
			   (ff_compare_variables(view->data, record_buffer) < 0)) {
			out_of_range_flag = 1;
			out_of_range(view->data, x_format_var, y_format_var, (unsigned long)(start_rec + offset));
			if (start_rec < last_rec) {
				error = dv_set(view,VIEW_GET_DATA, END_ARGS);
				if (error) {
					err_push(ROUTINE_NAME, ERR_SET_VIEW, "sp_find_record-> FORWARD");
					err_disp();
					exit(1);
				}
			}
			start_rec++;
		}
		return(start_rec);
	}
	else if (direction == BACKWARD){
		view->increment = 0 - view->increment;
		while ((start_rec >= last_rec) && 
			   (ff_compare_variables(view->data, record_buffer) > 0)) {
			out_of_range_flag = 1;
			out_of_range(view->data, x_format_var, y_format_var, (unsigned long)(start_rec + offset));
			if (start_rec > last_rec) {
				error = dv_set(view,VIEW_GET_DATA, END_ARGS);
				/* Generally, an error won't occur here because we are going
				   backwards through the cache. Start_rec must be set correctly */
				if (error) {
					err_push(ROUTINE_NAME, ERR_SET_VIEW, "sp_find_record-> BACKWARD");
					err_disp();
					exit(1);
				}
			}
			start_rec--;
		}
		view->increment = abs(view->increment);
		return(start_rec);
	}
	else {
		err_push(ROUTINE_NAME, ERR_PROCESS_DATA, "sp_find_record-> unknown direction");
		err_disp();
		exit(1);
	}
		
}

void add_sequence(SEQUENCE node, long index)
{
	/* Add a sequence (box) to the box array at box_array[index] */

	SEQUENCE_PTR HUGE *seq_ptr;
	SEQUENCE_PTR sequence;

	seq_ptr = ((SEQUENCE_PTR HUGE *)((char HUGE *)box_array + (index * sizeof(box_array))));
	/* This if...else could be condensed and made unconditional -- but I have not
	   had the time to do it yet */
	if (!(*seq_ptr)) {
		sequence = *seq_ptr = (SEQUENCE_PTR)malloc(sizeof(SEQUENCE));
		if (!sequence) {
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "add_sequence");
			err_disp();
			exit(1);
		}
	}
	else {
		sequence = *seq_ptr;
		while (sequence->next)
			sequence = sequence->next;
		sequence->next = (SEQUENCE_PTR)malloc(sizeof(SEQUENCE));
		if (!sequence->next) {
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "add_sequence");
			err_disp();
			exit(1);
		}
		sequence = sequence->next;
	}
	sequence->next = NULL;
	sequence->start = node.start;
	sequence->end = node.end;

}
int out_of_range(char *record_buffer, VARIABLE_PTR x_var, VARIABLE_PTR y_var, 
				  unsigned long rec_number)
{
	/* Print any records which are out of range */

	short i;
	VARIABLE_PTR var;

	fprintf(stderr,"\nDATA OUT OF RANGE: Pt %lu ", rec_number);
	for(i=0; i<2; i++) {

		if (i == 0) {
			fprintf(stderr,"x= ");
			var = x_var;
		}
		else {
			fprintf(stderr," y= ");
			var = y_var;
		}
		switch(FFV_TYPE(var)){
			case FFV_LONG:
				fprintf(stderr,"%ld",*(long *)(record_buffer + var->start_pos - 1));
				break;
	
			case FFV_ULONG:
				fprintf(stderr,"%lu",*(unsigned long *)(record_buffer + var->start_pos - 1));
				break;
	
			case FFV_SHORT:
				fprintf(stderr,"%hd",*(short *)(record_buffer + var->start_pos - 1));
				break;
	
			case FFV_USHORT:
				fprintf(stderr,"%hu",*(unsigned short *)(record_buffer + var->start_pos - 1));
				break;
	
			case FFV_UCHAR:
				fprintf(stderr,"%u",*(unsigned char *)(record_buffer + var->start_pos - 1));
				break;

			case FFV_DOUBLE:
				fprintf(stderr,"%lf",*(double *)(record_buffer + var->start_pos - 1));
				break;
	
			case FFV_FLOAT:
				fprintf(stderr,"%f",*(float *)(record_buffer + var->start_pos - 1));
				break;
	
	
			default:	/* Error, Unknown Input Variable type */
				err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE,"out_of_range var->type");
				return(1);
		}
	}
	return(0);
	
}
				  
int print_box_array(void)
{
	/* Print the entire box array into ptdata.bin 
	   Print offsets into the index file ptdata.ind
	   The last long written to ptdata.ind is the file size of ptdata.bin */

	int input_file;
	int data;

	long position;
	long num_bytes;
	long total_written;
	long file_offset = 1L;
	long zero = 0L;
	long file_index;

	unsigned long i;

	SEQUENCE_PTR HUGE *index;
	SEQUENCE_PTR seq;

	unsigned int buffer_bytes;
	unsigned int written;

	fprintf(stderr,"\nNumber of filled boxes to write:\n");
	data = open("ptdata.bin", O_WRONLY | O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
	if (data == -1) {
		err_push(ROUTINE_NAME, errno, "Opening ptdata.bin");
		return(1);
	}
	for(index=box_array; num_boxes>0; num_boxes--,index++) {
		fprintf(stderr,"\r%8u",num_boxes);
		seq = *index;
		file_index = 0L;
		while (seq) {
			file_index += (seq->end - seq->start + 1);
			/* Position the input file at the start of the sequence */
			position = head_length + (seq->start * record_length);
			if ((lseek(tmp_file, position, SEEK_SET)) == -1L) {
				err_push(ROUTINE_NAME, errno, "Positioning ptdata.tmp");
				return(1);
			}
			/* Transfer data until num_bytes are gone.
				The data buffer has DEFAULT_CACHE_SIZE bytes. */
	
			num_bytes = (seq->end - seq->start + 1) * record_length;
			while(num_bytes){
				buffer_bytes = (DEFAULT_CACHE_SIZE < num_bytes) ? DEFAULT_CACHE_SIZE : num_bytes;
				if ((read(tmp_file, input_buffer, buffer_bytes)) == -1) {
					err_push(ROUTINE_NAME, errno, "Reading ptdata.tmp to buffer");
					return(1);
				}
				if((written = write(data, input_buffer, buffer_bytes)) != buffer_bytes) {
					err_push(ROUTINE_NAME, ERR_WRITE_FILE, "Improper # of bytes written");
					return(1);
				}
	
				total_written += written;
				num_bytes -= written;
			}
			seq = seq->next;
		}
		if (file_index == 0) {
			if ((write(index_file, (char*)&zero, sizeof(long))) == -1) {
				err_push(ROUTINE_NAME, errno, "Writing 0");
				return(1);
			}
		}
		else {
			if ((write(index_file, (char*)&file_offset, sizeof(long))) == -1) {
				err_push(ROUTINE_NAME, errno, "Writing file offset");
				return(1);
			}
			file_offset += file_index * record_length;
		}
	}
	fprintf(stderr,"\r%8u",num_boxes);
	/* Write size of ptdata.bin */
	file_offset--;
	if ((write(index_file, (char*)&file_offset, sizeof(long))) == -1) {
		err_push(ROUTINE_NAME, errno, "Writing file offset");
		return(1);
	}
	close(index_file);
	close(data);
	return(0);
}

static int w_ff_compare_variables(const void *record_1, const void *record_2)
{
	return(ff_compare_variables((char *)record_1, (char *)record_2));
}