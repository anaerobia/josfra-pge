/*
 * NAME:	ff_afm2bfm
 *		
 * PURPOSE:	Converts ASCII format to binary
 *
 * USAGE:	FORMAT_PTR ff_afm2bfm(FORMAT_PTR format)
 *
 * RETURNS:	FORMAT_PTR if successful, NULL if not
 *
 * DESCRIPTION:	This function takes a pointer to a format and converts
 *				a binary format with the same variable types and
 *				offsets based on the variable types.
 *
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * ERRORS:	Insufficient Memory for new Format or Variables
 *			Out of memory,"Creating New Binary Format"
 *			Out of memory,"Creating New Variable List Header"
 *			Out of memory,"Creating New Variable List Node"
 *			Out of memory,old_var->name
 *			Unknown variable type,old_var->name
 *
 * COMMENTS:
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
 
#include <freeform.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_afm2bfm"

FORMAT_PTR ff_afm2bfm(FORMAT_PTR format)
{
	FORMAT_PTR new_format = NULL;
	VARIABLE_PTR old_var;
	VARIABLE_PTR new_var;
	VARIABLE_LIST_PTR old_v_list;
	VARIABLE_LIST_PTR new_v_list;

	unsigned short offset = 1;

	if ((new_format = (FORMAT_PTR)memCalloc(1, sizeof(FORMAT), "ff_afm2bfm: new_format")) == NULL) {
		err_push(ROUTINE_NAME,ERR_MEM_LACK,"Creating New Binary Format");
		return(NULL);
	}

	new_format->check_address = (void*)new_format;
	new_format->num_in_list = 0;
	new_format->max_length 	= 0;
	new_format->type = format->type;
	/* Change from ASCII or dbase to BINARY */
	if (new_format->type & FFF_ASCII)
		new_format->type = (new_format->type & ~FFF_ASCII) | FFF_BINARY;
	else if (new_format->type & FFF_DBASE)
		new_format->type = (new_format->type & ~FFF_DBASE) | FFF_BINARY;
	new_format->variables = new_v_list = dll_init();

	if(new_format->variables == NULL) {
		err_push(ROUTINE_NAME,ERR_MEM_LACK,"Creating New Variable List Header");
		return(NULL);
	}
	
	old_v_list = FFV_FIRST_VARIABLE(format);
	while( dll_data(old_v_list) ){
	
		old_var = FFV_VARIABLE(old_v_list);

		/* Add Member to list for new variable */
		new_v_list = dll_add(dll_last(new_format->variables), 0);
		if(!new_v_list){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"Creating New Variable List Node");
			ff_free_format(new_format);
			return(NULL);
		}

		if((new_var = (VARIABLE_PTR)memCalloc(1, sizeof(VARIABLE), "ff_afm2bfm: new_var")) == NULL) {
			err_push(ROUTINE_NAME,ERR_MEM_LACK,old_var->name);
			ff_free_format(new_format);
			return(NULL);
		}
		new_var->start_pos = offset;
		new_var->check_address = (void*)new_var;
		new_var->type = old_var->type;
        	new_var->precision = old_var->precision;
		memStrcpy(new_var->name, old_var->name,NO_TAG);

		switch(FFV_TYPE(old_var)){
		case FFV_LONG:
		case FFV_ULONG:
			offset += sizeof(long) - 1;
			break;

		case FFV_SHORT:
		case FFV_USHORT:
			offset += sizeof(short) - 1;
			break;

		case FFV_UCHAR:
			offset += sizeof(char) - 1;
			break;

		case FFV_CHAR:
			offset += FF_VAR_LENGTH(old_var) - 1;
			break;

		case FFV_FLOAT:
			offset += sizeof(float) - 1;
			break;

		case FFV_DOUBLE:
			offset += sizeof(double) - 1;
			break;

		default:	/* Error, Unknown Input Variable type */
			err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE,old_var->name);
			return(NULL);
		}
		new_var->end_pos = offset++;
		++new_format->num_in_list;

		new_v_list->data_ptr = (void *)new_var;		
		old_v_list = dll_next(old_v_list);
	}
	new_format->max_length 	= offset - 1;
	return(new_format);
}
