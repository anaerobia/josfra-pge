/*
 * FILENAME: mkformat.c
 *
 * CONTAINS: ff_header_to_format
 *           delete_white_space
 *           (MODULE) alphanum_type
 *           (MODULE) findpos()
 *
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	7/17/95		-rf01
 *		avoid compile errors in THINK C with #undef ROUTINE_NAME
 *
*/

#include <limits.h>
#include <stdlib.h>
#include <freeform.h>
#include <databin.h>

#ifdef PROTO
char *delete_white_space(char *);
static int alphanum_type(char *, int, int);
static BOOLEAN findpos(char *header, char *item, int delim_line, int delim_value, int dist, 
				 int *start_pos, int *end_pos);
#else
char *delete_white_space();
static int alphanum_type();
static BOOLEAN findpos();
#endif

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME		/* needed to avoid compile errors in THINK C -rf01 */
#endif
#define ROUTINE_NAME "ff_header_to_format"
/*
 * NAME:                ff_header_to_format
 *              
 * PURPOSE:     To make a FREEFORMAT list(actually the position) from
 *                      header contents
 *
 * USAGE:       BOOLEAN mkformat(FORMAT *form, char *header, char delim_item, char delim_value, int dist)
 *
 * RETURNS:     If success, return 0, otherwise return an error code
 *
 * DESCRIPTION: This function is used to prepare header format for
 * variable-length variable-position header which usually
 * has formats: variable-name delim_value variable-value delim_item, or
 * variable-name variable-value delim_item where the distance between 
 * variable-name and variable-value is dist, or variable-value delim_item.
 * The function selects format according to the passed arguments. The 
 * delim_item must be none-NULL character. If delim_value is NULL (STR_END)
 * and dist is not equal to zero, it is second format. if both delim_value== STR_END, 
 * and dist==0, the third format. 
 *                              
 * For the first two cases, if the fisrt variable name in the 
 * format is "create_format_from_data_file", this function will
 * create the format according to the data in the header.
 *
 * ERRORS:
 *                                              Out of memory,"format->variables"
 *                                      Out of memory,"var->next"
 *                                      Out of memory,"can not create dll node"
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    variable-length header, format
 *
 */
#ifdef PROTO
	int ff_header_to_format(FORMAT *form, char *header, int delim_line, int delim_value, int dist)
#else
	int ff_header_to_format(form, header, delim_line, delim_value, dist)
	FORMAT *form;
	char *header;
	int delim_line;
	int delim_value;
	int dist;
#endif
{       
	VARIABLE *var;
	VARIABLE_LIST_PTR v_list;
	int num_in_list = 0;
	int start_pos, end_pos;
	char *hline; /* each "line" in the header */
	int p_offset, /* parameter offset from start of header */
	    v_shift; /* variable shift, due to whitespace trims, or variable length */

	if (delim_line == STR_END)
		return(ERR_HEAD_FORM);    

	if (form == NULL)
		return(ERR_BINFORM_DEFINED);
	if (header == NULL)
		return(ERR_HEAD_DEFINED);

	p_offset = 0;
	hline = header;

	v_list = FFV_FIRST_VARIABLE(form);
	if ((var = FFV_VARIABLE(v_list)) != NULL &&
	    (strcmp(var->name, "create_format_from_data_file") == 0 ||
	     strcmp(var->name, "create_format_from_header_file") == 0))
/* I (MAO) would like to transition out *_data_* in favor of *_header_* */
	{
		dll_free(form->variables, (void (*)(void *))ff_free_variable);
		form->variables = NULL;

		while (1)
		{
			if (findpos(hline, NULL, delim_line, delim_value, dist, &start_pos, &end_pos))
			{
				assert(start_pos > 0);
				assert(end_pos > 0);
				assert(start_pos <= end_pos);
				
				if (num_in_list == 0)
				{   
					if ((form->variables = dll_init()) == NULL)
						return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "format variable list"));
				}
				if ((var = (VARIABLE_PTR)memCalloc(1, sizeof(VARIABLE), "ff_header_to_format")) == NULL)
				{
					dll_free(form->variables, (void (*)(void *))ff_free_variable);
					form->variables = NULL;
					return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "next variable in format variable list"));
				}

				v_list = dll_add(dll_last(form->variables), 0);
				if (!v_list)
				{
					dll_free(form->variables, (void (*)(void *))ff_free_variable);
					form->variables = NULL;   
					return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Unable to add to format variable list"));
				}

				v_list->data_ptr = var;
				var->check_address = var;

				/* Set variable start and end positions */
				var->start_pos = start_pos + p_offset;
				while (isspace(header[var->start_pos - 1]) && var->start_pos < USHRT_MAX)
					var->start_pos++;
				var->end_pos = end_pos + p_offset;
				while (isspace(header[var->end_pos - 1]) && var->end_pos > 0)
					var->end_pos--;

				assert(var->end_pos > 0);
				assert(var->start_pos <= var->end_pos);
				
				form->max_length = max(form->max_length, var->end_pos);

				/* Set variable name same as parameter name.
				   Reuse start_pos and end_pos for variable name substring */
				end_pos = start_pos - 2;
				start_pos = 1;

				while (isspace(hline[start_pos - 1]) && start_pos < USHRT_MAX)
					start_pos++;
				
				while (isspace(hline[end_pos - 1]) && end_pos > 0)
					end_pos--;
				
				v_shift = end_pos - start_pos + 1;
				
				assert(v_shift > 0);
				assert(v_shift < sizeof(var->name));
				v_shift = min(v_shift, sizeof(var->name) - 1);
				strncpy(var->name, hline + start_pos - 1, v_shift);
				var->name[v_shift] = STR_END;
        
				/* Set variable type and precision */
				switch (alphanum_type(header, var->start_pos, var->end_pos))
				{
					case 0:
						var->type = FFV_TEXT;
						var->precision = 0;
					break;
        	
					case 1:
						var->type = FFV_INT32;
						var->precision = 0;
					break;
        	
					case 2:
						var->type = FFV_FLOAT64;
						
						v_shift = strcspn(header + var->start_pos - 1, ".");
						if (v_shift + (int)var->start_pos < (int)var->end_pos)
						{ /* count trailing characters after decimal point */
							var->precision = var->end_pos - var->start_pos - v_shift;
							v_shift = strcspn(header + var->start_pos - 1, "eE");
							if (v_shift + (int)var->start_pos < (int)var->end_pos)
								/* E-notation characters were added into precision, subtract
								them off, including the 'E' or 'e' */
								var->precision -= var->end_pos - var->start_pos - v_shift + 1;
														
							assert(var->precision >= 0);
							assert((unsigned)var->precision < var->end_pos - var->start_pos + 1);
						}
						else
							var->precision = 0;
					break;
        	
					default:
						err_push(ROUTINE_NAME, ERR_UNKNOWN, "Internal Error, unexpected function return value");
					break;
				} /* switch alphanum_type() */

				/* The following assertion relates to basic assumptions in the name
				   table module, and possibly elsewhere...
				*/
				if (FF_VAR_LENGTH(var) >= MAX_PV_LENGTH)
				{
					assert(FF_VAR_LENGTH(var) < MAX_PV_LENGTH);
					err_push(ROUTINE_NAME, ERR_HEAD_FORM, "Parameter value string is too long");
					dll_delete(v_list, (void (*)(void *))ff_free_variable);
				}
				else
					num_in_list++;
			}
			hline = strchr(hline, delim_line);
			if (!hline)
				break;
			hline++;
			p_offset = (int)((char HUGE *)hline - (char HUGE *)header);
			if (*hline == STR_END)
				break;
		}
	} /* create format from header file */
	else
	{
		while ((var = FFV_VARIABLE(v_list)) != NULL)
		{
			if (findpos(hline, var->name, delim_line, delim_value, dist, &start_pos, &end_pos))
			{
				var->start_pos = start_pos + p_offset;
				var->end_pos = end_pos + p_offset;
				form->max_length = max(form->max_length, var->end_pos);
				while (isspace(*(header + var->end_pos - 1)))
					var->end_pos -= 1;
				if (delim_value == STR_END && dist == 0)
				{
					hline = header + var->end_pos + 1;
					p_offset = var->end_pos + 1;
#ifdef CCMSC
					if (delim_line == '\n')
					{ /* return key has two bytes */
						hline++;
						p_offset++;
					}
#endif
				}
				num_in_list++;
				v_list = dll_next(v_list);
			}
			else
			{
				v_list = dll_next(v_list);
				dll_delete(dll_previous(v_list), (void (*)(void *))ff_free_variable);
			}
		}
	}

	form->num_in_list = num_in_list;

	if (num_in_list == 0)
		return (ERR_HEAD_FORM);
	else
		return (0);
}

/*
 * NAME:                delete_white_space
 *              
 * PURPOSE: To delete the head and tail no-visible(include space)characters.
 *
 * USAGE:       char *delete_white_space(char *str)
 *
 * RETURNS: If the input str is all white, return NULL, otherwise, return 
 * the point to the no-white string (still part of argument str)
 *
 * DESCRIPTION: This function deletes all characters which is less that
 * 0x21 or larger than 0x7E in the head or tail part of 
 * the str, and return then return the str.
 *                              
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    utility
 *
 */

char *delete_white_space(char *str)
{
	char *ch;
	int i;

	ch = str;
	while(*ch != STR_END)
	{
		if (isspace(*ch))
			ch++;
		else
			break;
	}

	i = strlen(ch);
	while (i > 0)
	{
		if (isspace(ch[i - 1]))
			i--;
		else
		{
			ch[i] = STR_END;
			break;
		}
	}
	
	if (i > 0)
		return(ch);
	else
		return(NULL);
}

/*
 * NAME:                alphanum_type
 *              
 * PURPOSE:     To parse a string to determine whether or not it is a
 *                      numerical string.
 *
 * USAGE:       int alphanum_type(char *header, int start, int end)
 *
 * RETURNS:     If 0, not a numerical string, 1 integer, 2 real number (include
 *                      E-type data) 
 *
 * DESCRIPTION: This function uses a finite state technique to parse
 * a string to determine whether or not it is a numerical string.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    utility
 *
 */
static int alphanum_type(char *header, int start, int end)
{
	int status = 0;

	/* states       =0, initial
			=1, initial sign
			=2, inital decimal point
			=3, integer &
			=4, float &
			=5, has E part
			=6, finish interger &
			=7, finish float &
			=8, E part has sign
			=9, E digital   &
			=10, E finish &
	*/

	while (start <= end)
	{
		switch (header[start - 1])
		{
			case '+':
			case '-':
	
				if (status == 0)
					status = 1;
				else if (status == 5)
					status = 8;
				else
					return(0);
			break;
				
			case 'E':
			case 'e':
			
				if (status == 3 || status == 4)
					status = 5;
				else
					return(0);
			break;
	
			case '.':
				if (status == 0 || status == 1)
					status = 2;
				else if (status == 3)
					status = 4;
				else 
					return(0);
			break;
	
			default:
				if (isdigit(header[start - 1]))
				{
					if (status == 0 || status == 1 || status == 3)
						status = 3;
					else if (status == 2 || status == 4)
						status = 4;
					else if (status ==5 || status == 8 || status == 9)
						status = 9;
					else
						return(0);
				}
				else if (isspace(header[start - 1]))
				{ /* space */
					if (status == 0)
						;
					else if ((status == 3) || (status == 6))
						status = 6;
					else if ((status == 4) || (status == 7))
						status = 7;
					else if ((status == 9) || (status == 10))
						status = 10;
					else 
						return(0);
				}
				else
					return(0);
		} /* switch header[start - 1] */
		start++;
	}
	if (status == 6 || status == 3)
		return(1);
	else if (status == 4 || status == 7 || status == 9 || status == 10)
		return(2);
	else
		return(0);
}

/*
 * NAME:                findpos
 *              
 * PURPOSE:     To find position of a variable value from a character string
 *
 * USAGE:       BOOLEAN findpos(char *header, char *item, char token1, char toke2, int dist, 
 *                                              unsigned int *start_pos, unsigned int *end_pos)
 *
 * RETURNS:     If success, return TRUE, otherwise, FALSE
 *
 * DESCRIPTION: This function is used to find an item's value in a variable-length,
 * variable_location character string. The string should look
 * like following:
 * case 1: item_name1 delim_value item_value1 delim_item item_name2 delim_value item_value2 delim_item... 
 * case 2: item_name1 item_value1 delim_item item_name1 item_value2 delim_item. and the distance
 *         between the first char of item_name and that of the item_value is dist.
 * case 3: item_value1 delim_item item_value2 delim_item...
 *
 * Argument:
 * header--pointer to a string
 * item--pointer to an item name to be found in the string
 * delim_value-- a deliminator seperating item name from item value.
 * delim_item-- a delimnator seperating one item from another
 * distance--number of characters between the first character of 
 *           item name and of item value. If token != STR_END, this value
 *           will not be used.
 * value-- pointer to contain the item value
 *
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * GLOBALS:
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "findpos"

#ifdef PROTO
static BOOLEAN findpos(char *header, char *item, int delim_line, int delim_value, int dist, 
				 int *start_pos, int *end_pos)
#else
static BOOLEAN findpos(header,item,delim_line,delim_value, dist, start_pos, end_pos)
	char *header;
	char *item;
	int delim_line;
	int delim_value;
	int dist;
	int *start_pos, *end_pos;
#endif
{
	char *ch1,*ch2;
#ifdef CCMSC
	int end=0;
#endif
	assert(header);

	if (delim_value == STR_END && dist == 0)
	{                   /* case 3 */
		ch2 = memStrchr(header, delim_line, "header,delim_line");
		if (ch2 == NULL)
			return(FALSE);
		*start_pos = 1;
		*end_pos = (int)((char HUGE *)ch2 - (char HUGE *)header);
		if (delim_line == '\n')
		 *end_pos -= 1;    /* return key has two bytes */
	}
	else
	{                                                                  /* case 1 and 2 */
		if (item)
			ch1 = memStrstr(header, item, "header,item");  /* find item location */
		else
			ch1 = header;
						
		if (!ch1)
			return(FALSE);

		ch2 = strchr(ch1, delim_line);     /* find token location */
		if (!ch2)
		{
			ch2 = strrchr(ch1, STR_END);  /* set it to end of the string */
#ifdef CCMSC
			end = 1;
#endif
		}
		if (delim_value != STR_END)
		{ /* case 1 */
			if ((ch1 = strchr(ch1,delim_value)) == NULL)
				return(FALSE);
			*start_pos = (int)((char HUGE *)ch1 - (char HUGE *)header + 2);
		}
		else
		{ /* case 2 */
			*start_pos = (int)((char HUGE *)ch1 - (char HUGE *)header + dist + 1);
			if (((char HUGE *)ch2 - (char HUGE *)ch1) < dist)
				return(FALSE);
		}
		
		*end_pos = (int)((char HUGE *)ch2 - (char HUGE *)header);
		ch1 = ch2;
		ch2 -= 1;
#ifdef CCMSC
		if (delim_line == '\n' && !end)
			*end_pos -= 1;
#endif
	}
	return(TRUE);
}


	
