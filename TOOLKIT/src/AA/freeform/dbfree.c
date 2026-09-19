/*
 * NAME:                db_free
 *              
 * PURPOSE:     To free dbin and associated data structure
 *
 * USAGE:       db_free(DATA_BIN_PTR dbin)
 *
 * RETURNS:     void
 *
 * DESCRIPTION: This function frees the data bin structure and its associated
 *                              data structure, closes all opened input, output, and header file.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    dbin
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "Data Bin: Free"

#if defined(CCLSC) || defined(SUNCC)
#include <errno.h>
#endif

#include <freeform.h>
#include <databin.h>

void db_free(DATA_BIN_PTR dbin)
{
	assert(dbin && ((void*)dbin == dbin->check_address));
	
	if (dbin->buffer)
		memFree(dbin->buffer, "dbin->buffer");
	
	if (dbin->cache)
		memFree(dbin->cache, "dbin->cache");

	if (dbin->title)
		memFree(dbin->title, "dbin->title");

	if (dbin->file_name)
		memFree(dbin->file_name, "dbin->file_name");

	if (dbin->header_file_name)
		memFree(dbin->header_file_name, "dbin->header_file_name");

	if (dbin->header)
		memFree(dbin->header, "dbin->header");

	if (dbin->data_parameters)
		memFree(dbin->data_parameters, "dbin->data_paramaters");

	if (dbin->header_delimited_fields)
		memFree(dbin->header_delimited_fields, "dbin->header_delimited_fields");

	if (dbin->input_delimited_fields)
		memFree(dbin->input_delimited_fields, "dbin->input_delimited_fields");

	if (dbin->index)
		db_free_index(dbin->index);

	if (dbin->format_list)
		db_free_format_list(dbin->format_list);

	if (dbin->table_list)
		fd_free_format_data_list(dbin->table_list);

	/* close opened files */
	if (!dbin->state.std_input && dbin->data_file)
		close(dbin->data_file);

	if ((dbin->header_file) && (dbin->header_file != dbin->data_file))
		close(dbin->header_file);

#ifdef XVT
	/* If inside GeoVu, see if there is a MENU_OPEN struct attached to the dbin */
	if(dbin->gvmopen){
		/* There is; decrement the usage count since we are destroying this dbin */
		dbin->gvmopen->usage_count--;
	}
	
	if(dbin->gvmselection){
		/* There was a MENU_SELECTION struct hanging off the dbin- free it up */
		mn_selection_free(dbin->gvmselection);
	}
	
	/* If there was a dbin->mindex MENU_INDEX structure hanging off the dbin,
	 * we should not free it- other things may be using it. */
#endif

	memFree(dbin, "dbin");
	return;
}

