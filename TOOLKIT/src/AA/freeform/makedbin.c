/*
 * NAME:        db_make
 *
 * PURPOSE:     To create a data bin in memory.
 *
 * USAGE:       db_make(char *title)
 *
 * RETURNS:     A pointer to the new data bin, else NULL
 *
 * DESCRIPTION: This function allocates memory for and initializes
 *                              a data bin.
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:
 *
 * ERRORS:      out of memory, "Data Bin"
 *
 * SYSTEM DEPENDENT FUNCTIONS: 
 *
 * KEYWORDS:
 *
 */
/*
 * HISTORY:
 *	r fozzard	4/20/95		-rf01 
 *		(char *) for Think C
*/

#include <freeform.h>
#include <databin.h>

/* Operating System Dependent Includes: */
#ifdef CCLSC
#include <unix.h>
#endif
#undef ROUTINE_NAME
#define ROUTINE_NAME "db_make"

DATA_BIN_PTR db_make(char *title)
{
	DATA_BIN_PTR dbin;

	/* Allocate the memory for the dbin */
	dbin = (DATA_BIN_PTR)memMalloc(sizeof(DATA_BIN), "dbin");

	if(dbin == NULL){
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Data Bin");
		return(NULL);
	}

	/* Initialize data_bin */

	dbin->check_address = (void*) dbin;

	if(title){
		dbin->title = (char *) memStrdup(title, "dbin->title"); /* (char *) for Think C -rf01 */
		if(dbin->title == NULL){
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "Data Bin Title");
			memFree(dbin, "dbin");
			return(NULL);
		}
	}

	dbin->file_name                 = NULL;
	dbin->header_file_name  = NULL;
	dbin->input_format              = NULL;
	dbin->output_format             = NULL;
	dbin->header_format             = NULL;
	dbin->format_list               = NULL;
	dbin->header                    = NULL;
	dbin->index                     = NULL;
	dbin->cache                     = NULL;
	dbin->cache_end                 = NULL;
	dbin->data_ptr                  = NULL;
	dbin->buffer                    = NULL;
	dbin->table_list                = NULL;
	dbin->header_delimited_fields   = NULL;
	dbin->input_delimited_fields    = NULL;
	dbin->data_parameters   = NULL;
#ifdef XVT
	dbin->mindex					= NULL;
	dbin->gvmopen					= NULL;
	dbin->gvmselection				= NULL;
#endif /* XVT */

	dbin->data_file                 = 0;
	dbin->header_file               = 0;
	dbin->bytes_to_read             = 0;
	dbin->num_input_vars            = 0;
	dbin->num_header_vars           = 0;
	dbin->header_type               = 0;
	dbin->projection                = 0;
	dbin->type                      = 0;
	
	dbin->cache_size                = 0L;
	dbin->records_in_cache          = 0L;
	dbin->file_location             = 0L;
	dbin->bytes_available           = 0L;
	dbin->data_available            = 0L;
	
	dbin->state.open_file           = 0;
	dbin->state.std_input           = 0;
	dbin->state.cache_defined       = 0;
	dbin->state.cache_filled        = 0;
	dbin->state.header_defined      = 0;
	dbin->state.header_delimited    = 0;
	dbin->state.input_delimited     = 0;
	dbin->state.new_record          = 0;
	dbin->state.read_only           = 0;
	dbin->state.empty2              = 0;
	dbin->state.format_key          = 0;

	/* default data byte_order=native order = endian()
		1=big endian, 0=little endian */
	dbin->state.byte_order          = (unsigned char)endian();
	
	dbin->data_parameters           = 0;

	return(dbin);
}

