/*
 * NAME:	db_embedded_header_length(dbin)	
 *		
 * PURPOSE:	To get the length of a embedded header in the input data
 *
 * USAGE:	short db_embedded_header_length(dbin);
 *
 * RETURNS:	the header length
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:	
 *
 * AUTHOR:	Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	header length
 *
 */
#include <freeform.h>
#include <databin.h>
long db_embedded_header_length(DATA_BIN *dbin)
{long i=0;

	if(nt_askvalue(dbin, "data_bytes_offset", FFV_LONG, &i, dbin->buffer))return(i);

	if(dbin->header_type >= HEADER_SEPARATED){
		if(!nt_askvalue(dbin, "dummy_embedded_header_length", FFV_LONG, &i, dbin->buffer))i=0;
	}
	else if(dbin->header_format){
		i=FORMAT_LENGTH(dbin->header_format);
	}
	return(i);
}
