/* FILENAME:  splitdat.c
 *
 * CONTAINS: 	splitdat 
 */
  
#include <limits.h>


#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA
#include <databin.h>
#include <dataview.h>
#include <vg.h>
#include <hdf.h>

/* Function Prototypes */
int define_vset_fields(int32, FORMAT_PTR, char*);

#define SCRATCH_SIZE 10240
#define LOCAL_BUFFER_SIZE 61440
#define SET_FIELDS_BUFFER_SIZE 1024
#define NULL_STR ""

#undef ROUTINE_NAME
#define ROUTINE_NAME "splitdat"

/*
 * NAME:	splitdat
 *
 * PURPOSE:	This program writes record headers to a standard binary output
 *		file or to a vdata named "header". A new offset variable may
 *		also be included with the header.
 *		Splitdat also either writes the data to a specifed data file
 *		or to a second vdata named "data".
 *		If writing a vset, both vdatas are inserted into a single vdata
 *		which has the same name as the original data file.
 *
 * AUTHOR: 	Ted Habermann, NGDC, (303)497-6472, haber@ngdc.noaa.gov
 *		Mark VanGorp, NGDC, (303)496-6221, vset extension
 *
 * USAGE:	Menu Driven, Present Options Are:	
 *
 * COMMENTS:
 *			
 *
 */

void main(int argc, char **argv)
{
	char *data_buffer = NULL;
	char *header_buffer = NULL;
	char *data_ptr = NULL;
	char *header_ptr = NULL;
	char *offset_variable = NULL;
	char *extent_variable = NULL;
	char *vset_fields_buffer = NULL;
	char *point			= NULL;
	char *vdata_file_name		= NULL;

	char offset_format_str[8];
	char extent_format_str[8];
	char control_z_check;
	char vgname[65];
	char vgclass[65];
	char vsname[65];
	char vsclass[65];
	char fields[500];
	char header_format_buffer[SCRATCH_SIZE];
	char data_format_buffer[SCRATCH_SIZE];


	FORMAT_PTR header_output_format = NULL;
	FORMAT_PTR binary_format	= NULL;

	HFILEID vdata_file		= 0L;

	DATA_BIN_PTR	 	input		= NULL;
	VARIABLE_PTR	offset_var	= NULL;
	VARIABLE_PTR	extent_var	= NULL;

	unsigned int out_count = 0;
	unsigned header_bytes;
	unsigned input_bytes;
	unsigned output_bytes;
	unsigned num_cache = 0;
	unsigned data_buffer_remaining = 0;
	unsigned data_bytes_written = 0;
	unsigned header_bytes_written = 0;
	unsigned header_output_format_length = 0;
	unsigned data_output_format_length = 0;
	unsigned num_records = 0;

	int perc_left		= 0;
	int error;
	int output_file = 0;
	int std_output  = 0;
	int write_vset	= 0;

	int32	vg, n, vgotag, vgoref, nvg, vgid = -1;
	int32 t, vstag, vsid = -1, vs, nv, interlace,vsotag, vsoref, vsize;

	long file_size	= 0L;
	long total_data_written = 0L;
	long record_start	= 0L;
	long extent	= 0L;

/*	VDATA *vdata_headers						= NULL;
	VDATA *vdata_data						= NULL;
	VGROUP *v_group							= NULL;
*/
	int32 vdata_headers						= 0;
	int32 vdata_data						= 0;
	int32 v_group							= 0;


	/* If only two command line arguments, then write to an HDF file */
	if(argc == 2){
		write_vset = 1;
		vdata_file_name = (char*)malloc((size_t)MAX_NAME_LENGTH);
		if (!vdata_file_name) {
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "vdata_file_name");
			err_disp();
			exit(1);
		}
		strcpy(vdata_file_name, *(argv+1));
		point = strchr(vdata_file_name, '.');
		if (point)
			strcpy((point+1), "HDF\0)");
		else
			strcat(vdata_file_name,".HDF\0");
	}
	else if (argc != 3)
	{
		fprintf(stderr, "%s%s",
#ifdef ALPHA
"\nWelcome to splitdat alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm/HDF application\n\n",
#elif defined(BETA)
"\nWelcome to splitdat beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm/HDF application\n\n",
#else
"\nWelcome to splitdat release "FF_LIB_VER" -- an NGDC FreeForm/HDF application\n\n",
#endif
"USAGE: splitdat input_file [output_data_file > output_header_file]\n\
     - Splitdat splits record headers and data into separate files or vdatas\n\
     - New variables named \"begin\" and \"extent\" may also be included in\n\
       the record header output format\n\
     - begin and extent refer respectively to the file offset and number of\n\
       bytes in the output_data_file starting with byte 0.\n\
       Or, if writing a Vset, they refer to the beginning record and the number\n\
       number of records in vdata2 (count) starting with record 1\n\
     - If the output files are not specified, input_file_name.HDF\n\
       is created and both hierarchically named and organized as:\n\
                          vgroup\n\
                      input_file_name\n\
                           /      \\ \n\
                          /        \\\n\
                      vdata1        vdata2\n\
                 \"PointIndex\"  \"input_file_name\"\n\n\
     - vdata1 contains the record headers, vdata2 contains the data\n\
     - If writing to a vset, both output formats are converted to binary -- if\n\
       not so already\n");
		exit(1);
	}

	/* Allocate scratch buffer */
	data_ptr = data_buffer = (char *)malloc((size_t)LOCAL_BUFFER_SIZE);
	if (!data_buffer){
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Data Buffer");
		err_disp();
		exit(1);
	}
	data_buffer_remaining = LOCAL_BUFFER_SIZE;

	header_ptr = header_buffer = (char *)malloc((size_t)LOCAL_BUFFER_SIZE);
	if (!header_buffer){
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Header Buffer");
		err_disp();
		exit(1);
	}

	/* If we are not writing out a Vset, then open a binary output file for
	   the data and set standard output to binary for the headers */
	if (!write_vset) {
		output_file = open(*(argv + 2), O_WRONLY | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
		if(output_file == -1){
			err_push(ROUTINE_NAME,errno, *(argv+2));
			err_disp();
			exit(1);
		}

		std_output = fileno(stdout);

#ifndef SUNCC
		if( (setmode(std_output, O_BINARY)) == -1){
			err_push(ROUTINE_NAME, errno, "for std_output");
			err_disp();
			exit(1);
		}
#endif
	}

	/*
	** SET UP THE Input Data Bin:
	*/

	input = db_make(*(argv + 1));
	if(!input){
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, *(argv + 1));
		err_disp();
		exit(1);
	}

	if(db_set(input,
 		DBIN_BUFFER_SIZE,	DEFAULT_BUFFER_SIZE,
		DBIN_FILE_NAME,		*(argv + 1),
		DBIN_FILE_HANDLE,
		MAKE_NAME_TABLE, NULL,
		INPUT_FORMAT, NULL, NULL,
		OUTPUT_FORMAT, NULL, NULL,
		CHECK_FILE_SIZE,
		DBIN_CACHE_SIZE,	DEFAULT_CACHE_SIZE,
		END_ARGS)
	){
		err_push(ROUTINE_NAME, ERR_SET_DBIN, *(argv + 1));
		err_disp();
		exit(1);
	}

	/* Define the header output format */
	header_output_format = db_find_format(input->format_list, FFF_GROUP, FFF_HD | FFF_REC | FFF_OUTPUT, END_ARGS);
	if (!header_output_format) {
		err_push(ROUTINE_NAME, ERR_BINFORM_DEFINED, "NO OUTPUT HEADER FORMAT");
		err_disp();
		exit(1);
	}
	else if (!input->output_format) {
		err_push(ROUTINE_NAME, ERR_BINFORM_DEFINED, "NO OUTPUT DATA FORMAT");
		err_disp();
		exit(1);
	}


	/* If writing headers to a vdata, the header output format must be binary */
	if ( !(IS_BINARY(header_output_format)) && write_vset ) {
		fprintf(stderr,"Changing header output format to binary\n");
		binary_format = ff_afm2bfm(header_output_format);
		if ( IS_DATA(binary_format) )  {
			strcpy(binary_format->name, "new binary output data");
			binary_format->type = FFF_BINARY_OUTPUT_DATA;
		}
		else { /* if ( IS_HD(binary_format) ) */
			strcpy(binary_format->name, "new binary record header");
			binary_format->type = FFF_OUTPUT_BINARY_REC_HD;
		}
		db_delete_format(input->format_list, header_output_format->type);
		input->format_list = db_replace_format(input->format_list, binary_format);
		header_output_format = binary_format;
	}

	header_output_format_length = FORMAT_LENGTH(header_output_format);

	/* If writing data to a vdata, the data output format must be binary */
	if ( !(IS_BINARY(input->output_format)) && (write_vset) ) {
		fprintf(stderr,"Changing data output format to binary\n");
		binary_format = ff_afm2bfm(input->output_format);
		if ( IS_DATA(binary_format) )  {
			strcpy(binary_format->name, "new binary output data");
			binary_format->type = FFF_BINARY_OUTPUT_DATA;
		}
		else { /* if ( IS_HD(binary_format) ) */
			strcpy(binary_format->name, "new binary record header");
			binary_format->type = FFF_OUTPUT_BINARY_REC_HD;
		}
		db_delete_format(input->format_list, input->output_format->type);
		input->format_list = db_replace_format(input->format_list, binary_format);
		input->output_format = binary_format;
	}

	data_output_format_length = FORMAT_LENGTH(input->output_format);

	if (write_vset)
	{
		FF_BUFSIZE bufsize;
		
		bufsize.total_bytes =  -1; /* on purpose, set to UINT_MAX with compiler warning */
		bufsize.bytes_used  = ~-1; /* ugly, funky zero */
		bufsize.buffer = header_format_buffer;
		if (!ff_show_format(header_output_format, &bufsize))
		{
			err_push(ROUTINE_NAME, ERR_SHOW_FORM, "format1: header_format_buffer");
			err_disp();
			exit(1);
		}
		point = bufsize.buffer;
		CH_TO_END(point);
		*point++ = '#';
		*point++ = '#';
		*point++ = '#';
		*point++ = '\n';
		
		bufsize.buffer = point;
		if (!ff_show_format(input->header_format, &bufsize)) {
			err_push(ROUTINE_NAME, ERR_SHOW_FORM, "format2: header_format_buffer");
			err_disp();
			exit(1);
		}

		bufsize.total_bytes =  -1; /* on purpose, set to UINT_MAX with compiler warning */
		bufsize.bytes_used  = ~-1; /* ugly, funky zero */
		bufsize.buffer = data_format_buffer;
		if (!ff_show_format(input->output_format, &bufsize)) {
			err_push(ROUTINE_NAME, ERR_SHOW_FORM, "format1: data_format_buffer");
			err_disp();
			exit(1);
		}
		point = bufsize.buffer;
		CH_TO_END(point);
		*point++ = '#';
		*point++ = '#';
		*point++ = '#';
		*point++ = '\n';
		
		bufsize.buffer = point;
		if (!(ff_show_format(input->input_format, &bufsize))) {
			err_push(ROUTINE_NAME, ERR_SHOW_FORM, "format2: data_format_buffer");
			err_disp();
			exit(1);
		}
	}

	/* Check for offset variable */
	offset_var = ff_find_variable("begin", header_output_format);
	if(offset_var){
		offset_variable = (char *) malloc(FF_VAR_LENGTH(offset_var) + 1);
		if (!offset_variable){
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "Offset Variable Buffer");
			err_disp();
			exit(1);
		}

		if(!(IS_BINARY(header_output_format)))
			sprintf(offset_format_str, "%%%dld", FF_VAR_LENGTH(offset_var));
	}
			
	/* Check for extent variable */
	extent_var = ff_find_variable("extent", header_output_format);
	if(extent_var){
		extent_variable = (char *) malloc(FF_VAR_LENGTH(extent_var) + 1);
		if (!extent_variable){
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "Offset Variable Buffer");
			err_disp();
			exit(1);
		}

		if(!(IS_BINARY(header_output_format)))
			sprintf(extent_format_str, "%%%dld", FF_VAR_LENGTH(extent_var));
	}
			
	file_size = input->bytes_available;	
	fprintf(stderr, "The input file %s is %lu bytes long.\n\n", input->file_name, file_size);

	/* If writing a vset, then set up the vgroup and vdatas */
	if (write_vset) {
		vdata_file = (HFILEID)Hopen(vdata_file_name, (int)DFACC_CREATE, 0);
		Vinitialize(vdata_file);
		v_group = Vattach(vdata_file, -1, "w");
		Vsetname(v_group, *(argv+1)); 
		vdata_headers =  VSattach(vdata_file, -1, "w");
		vdata_data =  VSattach(vdata_file, -1, "w");
		VSsetname(vdata_headers, "PointIndex");
		VSsetname(vdata_data, *(argv + 1));
		VSsetclass(vdata_headers, "Index");
		VSsetclass(vdata_data, "Data");
		vset_fields_buffer = (char*)malloc((size_t)SET_FIELDS_BUFFER_SIZE);
		if (!vset_fields_buffer) {
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "vset_fields_buffer");
			exit(1);
		}
 
		error = define_vset_fields(vdata_headers, header_output_format, vset_fields_buffer);
		if (error) {
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "Setting header vdata fields");
			err_disp();
			exit(1);
		}
		VSsetfields(vdata_headers, vset_fields_buffer);
		error = define_vset_fields(vdata_data, input->output_format, vset_fields_buffer);
		if (error) {
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "Setting data vdata fields");
			err_disp();
			exit(1);
		}
		VSsetfields(vdata_data, vset_fields_buffer);
	}
	/* If writing a vset, then total_data_written refers to the number of records written
	   and starts at the first record */
	if (write_vset)
		total_data_written = 1;

	while(1){

		/* use PROCESS_FORMAT_LIST to fill cache and fill headers */
		if (error = db_events(input, PROCESS_FORMAT_LIST,
			FFF_ALL_TYPES, END_ARGS)) break;

		num_cache++;

		/* Convert the header - if defined */ 
		if (input->state.new_record){ 

			/* Convert the header */
			error = db_events(input, DBIN_CONVERT_HEADER, header_ptr, header_output_format, END_ARGS);

			if(error){
				err_push(ROUTINE_NAME, ERR_CONVERT, "RECORD HEADER");
				err_disp();
				exit(1);
			}

			/* Fill in the offset, if it is defined, ASSUMPTION: file offset is a long */
			if(offset_var){
				if(header_output_format->type & FFF_BINARY) 
					memcpy((void*)(header_ptr + (int)(offset_var->start_pos - 1)),
						 (void*)&total_data_written, sizeof(long));
	
				else{
					sprintf(offset_variable, offset_format_str, total_data_written);
					memcpy((void*)(header_ptr + (int)(offset_var->start_pos - 1)),
						(void*)offset_variable, (size_t)FF_VAR_LENGTH(offset_var));
  				}
			}

			/* Fill in the extent, if it is defined, ASSUMPTION: file extent is a long */
			if( extent_var ){
				extent = do_get_count( FFF_FORMAT, input->header_format, input->header, input); 
				if (!write_vset)
					extent *= data_output_format_length;
				if(header_output_format->type & FFF_BINARY) 
					memcpy((void*)(header_ptr + (int)(extent_var->start_pos - 1)),
						 (void*)&extent, sizeof(long));
				else{
					sprintf(extent_variable, extent_format_str, extent);
					memcpy((void*)(header_ptr + (int)(extent_var->start_pos - 1)),
						(void*)extent_variable, (size_t)FF_VAR_LENGTH(extent_var));
  				}
			}

			header_ptr += header_output_format_length;
			header_bytes_written += header_output_format_length;

			input->state.new_record = 0;

		} /* End of Header handling */
		
		/* Make sure buffer is large enough for the cache */
		(void)db_show(input, DBIN_BYTE_COUNTS, &input_bytes, &output_bytes, NULL);
		
		if ((unsigned long)output_bytes > (unsigned long)UINT_MAX){
			error = 1;
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "reallocation size too big");
			err_disp();
			break;
		}

		/* Make sure that the data buffer has enough room for this cache. */
		if(output_bytes > data_buffer_remaining){

			/* If not, write out all but the last header */
			header_bytes_written -= header_output_format_length;
			if (write_vset) {
				num_records = header_bytes_written / header_output_format_length;
				VSwrite(vdata_headers, (uint8*)header_buffer, num_records, FULL_INTERLACE);
			}
			else {
				header_bytes = write(std_output, header_buffer, header_bytes_written);
				if(header_bytes != header_bytes_written){
					err_push(ROUTINE_NAME, ERR_WRITE_FILE,"output file");
					err_disp();
					exit(-1);
				}
			}

			/* Write out the data */
			if (write_vset) {
				num_records = data_bytes_written / data_output_format_length; 
				VSwrite(vdata_data, (uint8*)data_buffer, num_records, FULL_INTERLACE);
			}
			else {
				out_count = (unsigned)write(output_file, data_buffer, (size_t)data_bytes_written);
				if(out_count != data_bytes_written){
					err_push(ROUTINE_NAME, ERR_WRITE_FILE,"output file");
					err_disp();
					exit(-1);
				}
			}	

			/* re-initialize the buffers */
			memcpy(header_buffer, header_buffer + header_bytes_written, header_output_format_length);
			header_bytes_written = header_output_format_length;
			header_ptr = header_buffer + header_output_format_length;

			data_bytes_written = 0;
			data_buffer_remaining = LOCAL_BUFFER_SIZE;
			data_ptr = data_buffer;
		}

		/* Convert the cache into the buffer */
		output_bytes = 0L;
		error = db_events(input,
			DBIN_CONVERT_CACHE, data_ptr, (void*)NULL, &output_bytes, END_ARGS);
		if(error) break;

		data_ptr += output_bytes;
		data_bytes_written += output_bytes;
		data_buffer_remaining -= output_bytes;
		if (write_vset)
			total_data_written += (output_bytes / data_output_format_length);
		else
			total_data_written += output_bytes;

		/* Calculate percentage left to process and display */
		perc_left = ((float)input->bytes_available / file_size) * 100; 
		fprintf(stderr,"\r%4d %% left to process...", perc_left);

	}/* End Processing */

	if (error == EOF)
		fprintf(stderr,"  end of input file reached.\n\n");
	/* This else is a cludgy way of checking for control-z's at
	   the end of files. 26 is the decimal equivalent of control-z */
	else if (input->bytes_available == 1) {
		lseek(input->data_file,input->file_location, SEEK_SET);
		read(input->data_file,(void*)&control_z_check,1);
		if (control_z_check == 26) {
			err_clear();
			error = EOF;
			fprintf(stderr,"  end of input file reached.\n\n");
		}
	}

	/* Write remaining headers */
	if (write_vset) {
		num_records = header_bytes_written / header_output_format_length;
		VSwrite(vdata_headers, (uint8*)header_buffer, num_records, FULL_INTERLACE);
	}
	else {
		header_bytes = write(std_output, header_buffer, header_bytes_written);
		if(header_bytes != header_bytes_written){
			err_push(ROUTINE_NAME, ERR_WRITE_FILE,"output file");
			err_disp();
			exit(-1);
		}
	}

	/* Write remaining data */
	if (write_vset) {
		num_records = data_bytes_written / data_output_format_length; 
		VSwrite(vdata_data, (uint8*)data_buffer, num_records, FULL_INTERLACE);
	}
	else {
		out_count = (unsigned)write(output_file, data_buffer, (size_t)data_bytes_written);
		if(out_count != data_bytes_written){
			err_push(ROUTINE_NAME, ERR_WRITE_FILE,"output file");
			err_disp();
			exit(-1);
		}
	}

	if (error && error != EOF){
			err_push(ROUTINE_NAME, ERR_PROCESS_DATA, NULL);
			err_disp();
			exit(1);
	}

	/* If writing a vset, then complete the writing to HDF file */
	if (write_vset) {
		Vinsert(v_group,vdata_headers);
		VSdetach(vdata_headers);
		Vinsert(v_group,vdata_data);
		VSdetach(vdata_data);
		Vdetach(v_group);

		/* Add the designated formats as annontations associated with
	   	the appropriate vdatas */
		while( (vgid = Vgetid(vdata_file,vgid)) != -1) {
	 	   vg = Vattach(vdata_file,vgid,"r");
	  	  if(vg == FAIL) {
			err_push(ROUTINE_NAME, ERR_WRITE_HDF, "attaching to vgroup");
			err_disp();
			exit(0);
	  	  }
	   	 Vinquire(vg,&n, vgname);
	   	 vgotag = VQuerytag(vg);
	   	 vgoref = VQueryref(vg);
	   	 Vgetclass(vg, vgclass); 
	   	 nvg++;
	   	 for (t=0; t< Vntagrefs(vg); t++) {
      			Vgettagref(vg, t, &vstag, &vsid);
      	
      			/* ------ V D A T A ---------- */
		      if (vstag==VSDESCTAG)  {  
	        	vs = VSattach(vdata_file,vsid,"r");
	
		        if(vs == FAIL) {
	        	  fprintf(stderr,"cannot open vs id=%d\n",vsid);
	          	  continue;
	       		 }
	        	VSinquire(vs, &nv,&interlace, fields, &vsize, vsname);
			vsotag = VSQuerytag(vs);
			vsoref = VSQueryref(vs);
        		if (HDstrlen(vsname)==0)  HDstrcat(vsname,"NoName");
        		VSgetclass(vs,vsclass);
			if (os_strcmpi(vsclass, "Index") == 0) {
				DFANputdesc(vdata_file_name, vsotag, vsoref, header_format_buffer, strlen(header_format_buffer));
			} 
			else if (os_strcmpi(vsclass, "Data") == 0) {
				DFANputdesc(vdata_file_name, vsotag, vsoref, data_format_buffer, strlen(data_format_buffer));
			}
              
       			VSdetach(vs);
          	     }
	   	} /* End for */
	    } /* End while */
	    Vclose(vdata_file);
	} /* End if write_vset */
	
	exit(0);
	
}

int define_vset_fields (int32 vdata_set, FORMAT_PTR format, char *set_fields_ptr) {	

	unsigned int i;
	VARIABLE_LIST_PTR	v_list_ptr;
	VARIABLE_PTR	var;
	unsigned int var_length 	= 0;

	for (i=0,v_list_ptr=FFV_FIRST_VARIABLE(format);
	     i<format->num_in_list;
	     i++, v_list_ptr=dll_next(v_list_ptr)) {

		var = FFV_VARIABLE(v_list_ptr);
		var_length = var->end_pos - var->start_pos + 1;
		strcpy(set_fields_ptr, var->name);
		set_fields_ptr += strlen(var->name);
		sprintf(set_fields_ptr, "%s", ",\0");
		set_fields_ptr++;

		switch(var->type){
		case FFV_ULONG:
			VSfdefine(vdata_set, var->name, (long)DFNT_UINT32, 1);
			break;

		case FFV_LONG:
			VSfdefine(vdata_set, var->name, (long)DFNT_INT32, 1);
			break;

		case FFV_SHORT:
			VSfdefine(vdata_set, var->name, (long)DFNT_INT16, 1);
			break;

		case FFV_USHORT:
			VSfdefine(vdata_set, var->name, (long)DFNT_UINT16, 1);
			break;

		case FFV_UCHAR:
			VSfdefine(vdata_set, var->name, (long)DFNT_UINT8, 1);
			break;

		case FFV_CHAR:
			VSfdefine(vdata_set, var->name, (long)DFNT_CHAR, var_length);
			break;

		case FFV_FLOAT:
			VSfdefine(vdata_set, var->name, (long)DFNT_FLOAT32, 1);
			break;

		case FFV_DOUBLE:
			VSfdefine(vdata_set, var->name, (long)DFNT_FLOAT64, 1);
 			break;

		default:	/* Error, Unknown Input Variable type */
			err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE, var->name);
			return(1);
		}
	}
	
	/* Remove last comma */
	*(set_fields_ptr - 1) = '\0';
	return(0);
}

