/* FILENAME:  formlist.c
 *
 * CONTAINS: 
 * ff_text_pre_parser()
 * make_format() (MODULE)
 */
#include <stdlib.h>
#include <limits.h>

#define WANT_FF_TYPES
#include <freeform.h>
#include <name_tab.h>

#include <os_utils.h>

#if defined(CCLSC) || defined(SUNCC)
#include <errno.h>
#endif

typedef enum sect_types_enum
{
 RESERVED     = 0,
 IN_SECT      = 1,
 FMT_SECT     = 2,
 INPUT_EQV_SECT  = 3,
 OUTPUT_EQV_SECT = 4,
 LAST_SECT    = 5,
 VAR_SECT     = 6 /* VAR_SECT cannot equal zero */
} sect_types_t;

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "make_format"

char *skip_lead_whitespace(char *s)
{
	assert(s);

	while (*s && isspace(*s) && strcspn(s, UNION_EOL_CHARS))
		s++;
	
	return(s);
}

#define is_comment_line(str) (*(str) == '/' ? TRUE : FALSE)

static BOOLEAN get_format_type_and_name
	(
	 char *sect_start, 
	 FF_TYPES_t *fmt_type,
	 char **fmt_name
	)
{
	char *cp = sect_start;
	char *format_type_str;
	char c = STR_END;
	
		/* See if a format type string is at start of line */
		/* Parse candidate format type string into a NULL-terminated string
		   for ff_lookup_number
		*/
	cp = sect_start;
	while (!isspace(*cp) && *cp != '\0')
		cp++;
		
	if (*cp != STR_END)
	{
		c = *cp;
		*fmt_name = skip_lead_whitespace(cp + 1);

		*cp = STR_END;
	}
	else
		*fmt_name = NULL;

	*fmt_type = ff_lookup_number(format_types, sect_start);
		
	if (*fmt_type == USHRT_MAX)
	{
		/* See if format type is expressed as a number */
		if (isdigit(*sect_start))
		{
			*fmt_type = (FF_TYPES_t)atol(sect_start);
			format_type_str = ff_lookup_string(format_types, *fmt_type);
			if (!format_type_str)
				*fmt_type = USHRT_MAX;
		}
	}
	
	if (c != STR_END)
		*cp = c;
	
	return((BOOLEAN)(*fmt_type != USHRT_MAX));
}

static char *find_eol(char *s)
{
	size_t spn;
	
	if (!ok_strlen(s))
		return(NULL);

	spn = strcspn(s, UNION_EOL_CHARS);
	
	if (spn == strlen(s))
		return(NULL);
	else
		return(s + spn);
}

static char *find_last_word_on_line(char *text_line)
{
	int lead_count;
	char *cp = find_eol(text_line);
	
	if (cp)
		--cp;
	else
		cp = text_line + strlen(text_line) - 1;

	lead_count = (char HUGE *)cp - (char HUGE *)text_line;

	while (lead_count && isspace(text_line[lead_count]))
	{
		lead_count--;
		cp--;
	}
	
	while (lead_count && !isspace(text_line[lead_count]))
	{
		lead_count--;
		cp--;
	}
	
	return(cp);
}
		
static BOOLEAN is_input_equiv_section(char *text_line)
{
	char *cp;
	
	cp = find_last_word_on_line(text_line);

	if (!strncmp(cp, "input_eqv", 9))
		return(TRUE);
	else
		return(FALSE);
}

static BOOLEAN is_output_equiv_section(char *text_line)
{
	char *cp;
	
	cp = find_last_word_on_line(text_line);

	if (!strncmp(cp, "output_eqv", 10))
		return(TRUE);
	else
		return(FALSE);
}

static sect_types_t is_last_sect(char *text_line)
{
	return (sect_types_t) (find_eol(text_line) ? FALSE : TRUE);
}

static FF_TYPES_t menu_format_section_type(char *text_line)
{
	char *cp;
	int word_span;
	
	if (!ok_strlen(text_line))
		return(FFF_NULL);
	
	cp = find_last_word_on_line(text_line);

	word_span = 0;
	while (*cp && !isspace(*cp))
	{
		word_span++;
		cp++;
	}
	
	if (word_span > 4)
	{
		cp -= 4;
		if (!strncmp(cp, "_afm", 4))
			return(FFF_ASCII);
		else if (!strncmp(cp, "_bfm", 4))
			return(FFF_BINARY);
		else if (!strncmp(cp, "_dfm", 4))
			return(FFF_DBASE);
	}
	
	return(FFF_NULL);
}

static BOOLEAN is_variable_description(char *text_line)
{
	char name[MAX_PV_LENGTH];
	unsigned short start_pos;
	unsigned short end_pos;
	char var_type_str[MAX_PV_LENGTH];
	short precision;
	
	if (sscanf(text_line, "%s %hu %hu %s %hd",
	           name,
	           &start_pos,
	           &end_pos,
	           var_type_str,
	           &precision
	          ) == 5
	   )
	{
		if (ff_lookup_number(variable_types, var_type_str) == USHRT_MAX)
			return(FALSE);
		else
			return(TRUE);
	}
	else
		return(FALSE);
		
}

static BOOLEAN is_format
	(
	 char *text_line 
	)
{
	FF_TYPES_t fmt_type;
	char *fmt_name;
	
	if (get_format_type_and_name(text_line, &fmt_type, &fmt_name))
		return(TRUE);
	else if (is_variable_description(text_line))
		return(TRUE);
	else if (menu_format_section_type(text_line) != FFF_NULL)
		return(TRUE);
	else
		return(FALSE);
}

static sect_types_t get_section_type
	(
	 char *text_line
	)
{
	sect_types_t sect_type;
	
	if (is_variable_description(text_line))
		sect_type = VAR_SECT;
	else if (is_format(text_line))
		sect_type = FMT_SECT;
	else if (is_input_equiv_section(text_line))
		sect_type = INPUT_EQV_SECT;
	else if (is_output_equiv_section(text_line))
		sect_type = OUTPUT_EQV_SECT;
	else if (is_last_sect(text_line))
		sect_type = LAST_SECT;
	/*
	else if (add_your_BOOLEAN_test_here())
		sect_type = ADD_YOUR_SECTION_TYPE_HERE;
	*/
	else
		sect_type = IN_SECT;
	
	return(sect_type);
}

static char *get_first_line(char *s)
{
	return(skip_lead_whitespace(s));
}

/* Return pointer to first non-whitespace character of next line (point
   to newline if blank line).  Return pointer to NULL-terminator if last
   line.
*/
static char *get_next_line(char *s)
{
	char *t;
	
	assert(s);
	
	t = find_eol(s);
	if (t)
	{
		t += strspn(t, UNION_EOL_CHARS);
		t = skip_lead_whitespace(t);
	}
	else
		t = s + strlen(s);
	return(t);
}

static int make_variable_list(char *text_line, FORMAT_PTR format)
{
	char errorbuff[2 * MAX_ERRSTR_BUFFER];

	int num_read     = 0;
	short adjustment = 0;

	VARIABLE_PTR         var = NULL;
	VARIABLE_LIST_PTR	v_list = NULL;

	char           name[2 * sizeof(var->name)];
	unsigned short start_pos = 0; 
	unsigned short end_pos   = 0;
	short          precision = 0;
	FF_TYPES_t     type      = 0;
	char           var_type_str[64];

	num_read = sscanf(text_line,"%s %hu %hu %s %hd", name, &start_pos, &end_pos, var_type_str, &precision);
		
	if (num_read == 5)
	{
		/* Okay, continue processing */
	}
	else if (num_read < 0)
	{
		return(err_push(ROUTINE_NAME, ERR_MAKE_FORM, "Premature end of string"));
	}
	else if (num_read == 0)
	{
		assert(num_read);
		
		return(0);
	}
	else
	{
		sprintf(errorbuff,"%s\n", text_line);
		return(err_push(ROUTINE_NAME,ERR_VARIABLE_DESC,errorbuff));
	}

	if (end_pos < start_pos)
	{
		sprintf(errorbuff," End Position < Start Position\n%s", text_line);
		return(err_push(ROUTINE_NAME,ERR_VARIABLE_DESC,errorbuff));
	}

	os_str_replace_unescaped_char1_with_char2('%', ' ', name);

	/* Determine The Variable Type */
	if (isdigit(*var_type_str))
		type = (unsigned short)atoi(var_type_str);
	else /* Convert From String to Variable Type */
		type = ff_lookup_number(variable_types, var_type_str);

	if (type == USHRT_MAX)
	{
		sprintf(errorbuff,"Unknown Variable Type\n%s", text_line);
		return(err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE,errorbuff));
	}
		
	/* If a binary format, confirm that given start and end byte positions
	   are compatible with the native machine size for that type.  If start
	   and end positions are both zero, this indicates a delimited data format.
	   (Delimited binary data?  Hmmm...)  If not FFV_DATA_TYPE_TYPE(type), then
	   this is a header, initial, constant, array, or some other "non-primitive"
	   type.
	*/
	if ( IS_BINARY(format) && end_pos && start_pos &&
	     FFV_DATA_TYPE_TYPE(type) == type
	   )
	{
		if (!IS_TEXT_TYPE(type) &&
		    (ffv_type_size(type) != end_pos - start_pos + 1)
		   )
		{
			sprintf(errorbuff, "Size error in binary format\n==> %s <==", os_str_trim_whitespace(text_line, text_line));
			return(err_push(ROUTINE_NAME,ERR_VARIABLE_DESC,errorbuff));
		}
	}

	/* END OF ERROR CHECKING */

	if (format->num_in_list == 0)
	{	/* First Variable: Initialize format and variable list */
		format->num_in_list = 1;

		if (type != FFV_HEADER)
			format->max_length = end_pos;
		else
			format->max_length = 0;

		/* Initialize the list */
		format->variables = v_list = dll_init();

		if (format->variables == NULL)
			return(err_push(ROUTINE_NAME,ERR_MEM_LACK,"Initial Variable"));
	}
	else
	{					/* Subsequent Variables */
		format->num_in_list++;
		if (type != FFV_HEADER && end_pos > format->max_length)
			format->max_length = end_pos;
	}

	/* Add Member to list for new variable */
	v_list = dll_add(dll_last(format->variables), 0);
	if (!v_list)
	{
		dll_free(format->variables, (void (*)(void *))ff_free_variable);
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK, name));
	}
		
	var = (VARIABLE_PTR)memMalloc(sizeof(VARIABLE), "var");
	if (var == NULL)
	{
		dll_free(dll_next(v_list), (void (*)(void *))ff_free_variable);
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK, name));
	}
	dll_data(dll_last(format->variables)) = var;

	/* EOL's will have different lengths dependent on system,
	adjust if needed */
	if (!strcmp(name, "EOL"))
	{
		assert(strcmp(name, "EOL")); /* I want to catch these types of variables */
		
		adjustment = (unsigned short)EOL_SPACE - (end_pos - start_pos + 1);
		start_pos -= adjustment;
	}

	if (strlen(name) > sizeof(var->name) - 1)
		err_push(ROUTINE_NAME, ERR_STRING_TOO_LONG, "var->name constant character array");
	strncpy(var->name, name, sizeof(var->name) - 1);
	var->name[sizeof(var->name) - 1] = STR_END;
	var->check_address = (void *)var;
	var->start_pos = start_pos += adjustment;
	var->end_pos = end_pos += adjustment;
	var->precision = precision;
	var->type = type;
	var->info = NULL;
	
	return(0);
}

/*
 * NAME:	make_format
 *
 * PURPOSE:	Parse variable descriptions, update format info.
 *
 * USAGE:	error = make_format(format_type, bufsize, format);
 *
 * RETURNS:	An error code on failure, zero otherwise
 *
 * DESCRIPTION:	
 * A format description is a block of text whose first line is a format type,
 * one or more separating spaces, and then a quoted format title.
 * Subsequent lines are one or more variable descriptions.
 * This function parses either single format descriptions or variable description
 * blocks.  A variable description block becomes a format description when
 * it is preceded by a line containing a format type and title.
 *
 * For example:
 *
 * binary_data "default binary format"
 * data 1 1 uchar 0
 *
 * The above format description contains only one variable description, but
 * might contain several.
 *
 * Parse each line of the buffer.  For each line, create and initialize
 * a variable structure, and add it to the format's variable list.  Set each
 * variable's name, type, start_pos, end_pos, and precision fields according
 * to the information on each line.  If the format is binary and is not
 * delimited (indicated by zero start and ending positions), check that the
 * size given by start and ending positions match the native type size.
 * 
 * Blank lines end formats; blank lines are not allowed within format sections.
 *
 * AUTHOR: Mark A. Ohrenschall, NGDC, (303) 497 - 6124 mao@ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * ERRORS:
 *
 * SYSTEM DEPENDENT FUNCTIONS: 
 *
 * KEYWORDS:	freeform, format
 *
 */

static int make_format
	(
	 FF_TYPES_t format_type,
	 FF_BUFSIZE_PTR bufsize,
	 FORMAT_PTR format
	)
{
	char *text_line = NULL;
	char *fmt_name;
	int error = 0;
	
	text_line = get_first_line(bufsize->buffer);

	if (!text_line)
		return(err_push(ROUTINE_NAME, ERR_MISSING_TOKEN, "Expecting a newline"));
	else
	{
		FF_TYPES_t check_format_type;
		
		if (get_format_type_and_name(text_line, &check_format_type, &fmt_name))
		{
			char *cp;
			
			format->type = check_format_type;
			strncpy(format->name, fmt_name, sizeof(format->name) - 1);
			format->name[sizeof(format->name) - 1] = STR_END;
			cp = find_eol(format->name);
			if (cp)
				*cp = STR_END;
			if (format->name[0] == '\"')
				format->name[0] = ' ';
			cp = strrchr(format->name, '\"');
			if (cp)
				*cp = ' ';
			os_str_trim_whitespace(format->name, format->name);

			text_line = get_next_line(text_line);
			if (!text_line)
				return(err_push(ROUTINE_NAME, ERR_VARIABLE_DESC, "Expecting a variable description"));

		}
		else
		{
			check_format_type = menu_format_section_type(text_line);
			if (check_format_type != FFF_NULL)
			{
				format->type = check_format_type;
				text_line = get_next_line(text_line);
			}
			else
				format->type = format_type;
		}
	}

	while (strlen(text_line))
	{
		char  save_char;
		char *cp;
						
		cp = find_eol(text_line);
		if (cp)
		{
			save_char = *cp;
			*cp = STR_END;
		}
		else
			save_char = STR_END;
							
		if (!is_comment_line(text_line) && strlen(text_line))
			error = make_variable_list(text_line, format);
						
		if (save_char != STR_END)
			*cp = save_char;
						
		if (error)
			return(error);
					
		text_line = get_next_line(text_line);
	}
	
/*	format->max_length += adjustment; */
	return(0);
}

static int parse_fmt_section
	(
	 FF_TYPES_t format_type,
	 FF_BUFSIZE_PTR var_desc_buffer,
	 FORMAT_HANDLE hformat
	)
{
	*hformat = ff_create_format();
	if (!*hformat)
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Creating format"));
			
	return(make_format(format_type, var_desc_buffer, *hformat));
}

static int update_format_list
	(
	 FF_TYPES_t format_type,
	 FF_BUFSIZE_PTR var_desc_buffer,
	 FORMAT_LIST_HANDLE hf_list
	)
{
	FORMAT_LIST_PTR f_list_end;
	FORMAT_PTR format = NULL;
	
	int error;
	
	error = parse_fmt_section(format_type, var_desc_buffer, &format);
	if (!error)
	{
		if (!*hf_list)
		{
			f_list_end = *hf_list = dll_init();
			if (!f_list_end)
			{
				dll_free(*hf_list, NULL);
				return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Creating format list"));
			}
		}
		else
			f_list_end = dll_last(*hf_list);

		f_list_end = dll_add(f_list_end, 0);
		if (!f_list_end)
		{
			dll_free(*hf_list, NULL);
			return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Creating format list"));
		}
							
		dll_data(f_list_end) = format;
	} /* (else) if error in parsing format section */
	
	return(error);
}

static int update_name_table_list(FF_BUFSIZE_PTR nt_buffer, NAME_TABLE_LIST_HANDLE hnt_list)
{
	NAME_TABLE_PTR nt = NULL;
	FF_TYPES_t nt_type = 0;
	char *cp;
	
	cp = get_next_line(nt_buffer->buffer);
	
	nt = nt_create(NULL, cp);
	if (nt)
	{
		cp = skip_lead_whitespace(nt_buffer->buffer);
		if (!strncmp(cp, "input_eqv", 9))
			nt_type = FFF_INPUT;
		else if (!strncmp(cp, "output_eqv", 10))
			nt_type = FFF_OUTPUT;
		else
			return(ERR_GENERAL);
		
		nt_put_name_table(hnt_list, nt_type, nt);
		return(0);
	}
	else
		return(ERR_GENERAL);
}

#define SAME_IO_CONTEXT(sect, obj) (\
(sect == INPUT_EQV_SECT && (obj->ppo_object.nt_list.nt_io_type & FFF_INPUT)) || \
(sect == OUTPUT_EQV_SECT && (obj->ppo_object.nt_list.nt_io_type & FFF_OUTPUT)))

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "ff_text_pre_parser"

/*
 * NAME: ff_text_pre_parser
 *
 * PURPOSE:  Parse a text buffer, determine contents, parse each
 * part as appropriate.
 *
 * USAGE: error = ff_text_pre_parser(file_ext, bufsize, pp_object)
 *
 * RETURNS: An error code on failure, zero on success
 *
 * DESCRIPTION:  This function is intended to be a dispatcher for other
 * parsers (e.g., nt_create, make_format).  Text files or text buffers
 * can be parsed by this function to determine the type of each element
 * (e.g., equivalence table, format) and each element (section) is then
 * parsed as appropriate.  Since this is not the only point of creation
 * for a newly created data structure, it is added to a list, which is passed
 * as an argument by the calling routine.  It is up to the update_*_list
 * functions called by the specific parser to manage the lists of old and
 * new data structures.
 *
 * The start of any new section must be uniquely identifiable, and preferably
 * unique in occurrence.  This means that a section start should not be a
 * legitimate occurrence within its own, or another section.  An exception
 * to this are variable descriptions, each line of which can not only signal
 * the beginning of a new section, but also compose the contents of a section.
 * This necessitates a kludge specific to variable description sections,
 * otherwise, every variable description line would begin a new section.
 *
 * The general scheme then is that each section begin with a unique identifier
 * (which may itself be a part of the section it is indicating) and each
 * section ends with the start of a new section, or the end of the buffer,
 * indicated by the NULL-terminator.
 *
 * Initially the state is outside of a section.  Each line is examined in
 * turn until a beginning of a section is found, then the state is within
 * a section.  Each line is further examined until a new section beginning or
 * the NULL-terminator, is found.  At this point the state becomes outside of
 * a section, and the previous section is dispatched to the appropriate parser.
 *
 * The above is repeated until there are no more sections.
 *
 * Currently recognized strings beginning a section are:
 * For formats:
 * 1) a recognized format format type string or number
 * 2) a variable description line
 * 3) a string ending with "_afm", "_bfm", or "_dfm"
 *
 * For equivalance tables:
 * 1) "input_eqv"
 * 2) "output_eqv"
 * 
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:  The file extension is passed in to determine the file
 * descriptor of the format type when parsing the contents of variable
 * description files.  When variable description files are no more, this
 * argument should be removed.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

int ff_text_pre_parser
	(
	 char *file_ext,
	 FF_BUFSIZE_PTR fmt_buffer,
	 PP_OBJECT_PTR pp_object
	)
{
	int error     = ERR_GENERAL;
	int new_error =           0;

	char *current_sect_start;
	char *text_line;
	
	char error_buffer[MAX_ERRSTR_BUFFER];
	
	sect_types_t current_sect_type; /* type of the previous section */
	sect_types_t sect_type;
	BOOLEAN first_section = TRUE;
	
	assert(fmt_buffer);

	text_line = get_first_line(fmt_buffer->buffer);

	first_section      =       TRUE;
	current_sect_start =       NULL;
	current_sect_type  =    IN_SECT;
	sect_type          =  (sect_types_t) !LAST_SECT;

	/* MAIN LOOP that parses each line of format buffer
	*/

	while (sect_type != LAST_SECT)
	{
		char save_char;
		
		sect_type = get_section_type(text_line);
		
		switch (sect_type)
		{
			case IN_SECT:
			break;

			case VAR_SECT:
				/* The kludge:  If the current line is a variable description,
				   and we are already in a variable description block or a
				   format description, then do not treat this line as if it were
				   the start of a new section.
				*/
				
				if (current_sect_type == VAR_SECT ||
				    current_sect_type == FMT_SECT
				   )
					break;
						
			/* intentional fall-through */
			case FMT_SECT:
			case INPUT_EQV_SECT:
			case OUTPUT_EQV_SECT:
			case LAST_SECT:
				if (first_section)
				{
					first_section = FALSE;
				}
				else
				{
					FF_BUFSIZE bufsize;

					/* Process current (previous) section */
					bufsize.buffer = current_sect_start;
					bufsize.bytes_used  =
					(size_t)(bufsize.total_bytes = (char HUGE *)text_line -
					                               (char HUGE *)current_sect_start
					        );
				
					save_char = *text_line;
					*text_line = STR_END;
								
					switch (current_sect_type)
					{
						FF_TYPES_t format_type;
						
						case VAR_SECT:
							if (file_ext)
							{
								if (!strcmp(file_ext, "afm"))
									format_type = FFF_ASCII;
								else if (!strcmp(file_ext, "dfm"))
									format_type = FFF_DBASE;
								else
									format_type = FFF_BINARY;
							}
						
            /* intentional fall-through */
						case FMT_SECT:
							if (pp_object->ppo_type == PPO_FORMAT_LIST)
							{
								if (current_sect_type == FMT_SECT)
									format_type = FFF_NULL;
	
								error = update_format_list(format_type, &bufsize, pp_object->ppo_object.hf_list);
								new_error = error;
							}
						break;
									
						case INPUT_EQV_SECT:
						case OUTPUT_EQV_SECT:
							if (pp_object->ppo_type == PPO_NT_LIST &&
							    SAME_IO_CONTEXT(current_sect_type, pp_object)
							   )
							{
								error = update_name_table_list(&bufsize, pp_object->ppo_object.nt_list.hnt_list);
								new_error = error;
							}
						break;
						
						/*
						case ADD_YOUR_SECTION_TYPE_HERE:
						*/
									
						default:
							sprintf(error_buffer, "%s, %s:%d",
							        ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
							err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
							err_disp();
							return(ERR_SWITCH_DEFAULT);
					} /* switch on current section type */
					
					if (new_error)
						break;
				
					*text_line = save_char;
				} /* if in_section */

				current_sect_type  = sect_type;
				current_sect_start = text_line;
			break;
			
			default:
				sprintf(error_buffer, "%s, %s:%d",
				        ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
				err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
				err_disp();
				return(ERR_SWITCH_DEFAULT);
		} /* switch on section type */
		
		if (new_error)
			break;

		if (sect_type != LAST_SECT) 
		if (sect_type != LAST_SECT)
			text_line = get_next_line(text_line);
		
  } /* while not the last section */

	return(error);
}

