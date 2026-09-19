/* FILENAME:  name_tab.c
 *
 * CONTAINS:  Functions for maintaining name tables for data bins:
 *
 * NAME_TABLE_PTR nt_create(char *filename, FF_SCRATCH_BUFFER buffer)
 *
 * int nt_merge(NAME_TABLE_PTR update_table, NAME_TABLE_HANDLE table)
 *
 * NAME_TABLE_PTR nt_free_name_table(NAME_TABLE_PTR table)
 *
 * char *nt_find_user_name(NAME_TABLE_PTR table, char *geovu_name)
 *
 * int nt_add_constant(NAME_TABLE_HANDLE htable, char *name, FF_TYPES_t var_type,
 * 	void *value)
 *
 * int nt_delete_constant(NAME_TABLE_PTR table, char *name)
 *
 * BOOLEAN nt_askexist(DATA_BIN_PTR dbin, char *name)
 *
 * BOOLEAN nt_askvalue(DATA_BIN_PTR dbin, char *geovu_name, FF_TYPES_t type,
 * 	void *dest, char *buffer)
 *
 * BOOLEAN nt_putvalue(DATA_BIN_PTR dbin, char *geovu_name,
 * 	FF_TYPES_t gvalue_type, void *geovu_value, short header_only)
 *
 * BOOLEAN nt_get_geovu_value(NAME_TABLE_PTR, char *geovu_name, void *user_value,
 * 	FF_TYPES_t uvalue_type, void *geovu_value, FF_TYPES_t *gvalue_type)
 *
 * BOOLEAN nt_get_user_value(NAME_TABLE_PTR table, char *geovu_name,
 *         void *geovu_value, FF_TYPES_t gvalue_type,
 *         void *user_value, FF_TYPES_t *uvalue_type)
 *
 * For Local Use Only:
 *
 * static void *nt_str_to_binary(char *buffer, int type)
 * static BOOLEAN nt_copy_translator_sll(VARIABLE_PTR source_var,
 *        VARIABLE_PTR target_var)
 * static BOOLEAN nt_copy_translator_ugvalue(FF_TYPES_t value_type,
 *        void *source_value, void **target_value)
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	7/31/95		-rf01
 *		CW Mac needs #include <stat.h> for struct stat below
*/

#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <os_utils.h>
#include <freeform.h>
#include <name_tab.h>
#include <databin.h>

#ifdef CCLSC	/* rf01 */
#include <stat.h>	/* CW Mac needs this for struct stat below -rf01 */
#endif /* CCLSC rf01 */

/* Module Functions */
#ifdef PROTO
/*static*/ void *nt_str_to_binary(char *buffer, FF_TYPES_t type);
static BOOLEAN nt_copy_translator_sll(VARIABLE_PTR source_var, VARIABLE_PTR target_var);
static BOOLEAN nt_copy_translator_ugvalue(FF_TYPES_t value_type,
       void *source_value, void **target_value);
static int parse_line_into_tokens_by_case(unsigned char status, char *line,
       char tokens[][MAX_PV_LENGTH + 1], int count_tokens_only);
static void nt_free_trans(TRANSLATOR_PTR trans);
static void nt_show_section(NAME_TABLE_PTR table, char *ch, FF_TYPES_t sect_type);

#else
static void *nt_str_to_binary(buffer, type);
static BOOLEAN nt_copy_translator_sll(source_var, target_var);
static BOOLEAN nt_copy_translator_ugvalue(value_type, source_value, target_value);
static int parse_line_into_tokens_by_case(status, line, tokens, count_tokens_only);
static void nt_free_trans(trans);
static void nt_show_section(table, ch, sect_type);
#endif

/*
 * NAME:  nt_create
 *              
 * PURPOSE:  To create a NAME_TABLE. 
 *
 * USAGE:  NAME_TABLE_PTR nt_create(char *filename, char *buffer)
 *
 * RETURNS:  If successful, return a pointer to the NAME_TABLE, otherwise, NULL;
 *
 * DESCRIPTION:
 *
 * When a FreeForm application such as GeoVu needs information
 * about a given data set it asks for the values of various
 * parameters.  If the data set has a GeoVu header then
 * retrieving this information is straightforward.  However, if
 * the data set has a native header (i.e., native to the
 * application that created it) then the relationships between
 * native and GeoVu parameter-values must be specified.
 * 
 * For example, an IDRISI (a non-FreeForm application) header
 * might contain the following:
 * 
 * rows        : 512
 * columns     : 256
 * min. value  : 124.54
 * max. value  : 234.89
 * data type   : real
 * 
 * whereas its GeoVu equivalent header would contain:
 * 
 * number_of_rows = 512
 * number_of_columns = 256
 * minimum_value = 124.54
 * maximum_value = 134.89
 * data_representation = float
 * 
 * It is the "name table" that specifies the relationship
 * between information in the native header format and the
 * GeoVu format.
 * 
 * Name tables hold two types of information:  constants and
 * parameter equivalencies.  Constants define values for
 * parameters (such as miscellaneous operating parameters for
 * processing the current data set) and parameter equivalencies
 * define correspondences between FreeForm applications'
 * parameter names and a user application's (e.g., IDRISI)
 * names.  Parameter equivalencies can define correspondences
 * between FreeForm and native parameter values as well.
 * 
 * The following Constant section defines the native header
 * file extension, the type of header, and the field delimiter,
 * since the native header has variable length fields.  The
 * variable fields have a fixed tag at the start of each field
 * so the length of each tag is also given.  So, for example:
 * 
 * begin constant
 * 	header_file_ext char .doc
 * 	header_type char header_separated_varied
 * 	_distance short 14
 * 	delimiter_item char \n
 * end constant
 * 
 * Each line has three fields:  the parameter name, its type,
 * and its value.
 * 
 * The following Name Equivalence section gives the
 * correspondences between the IDRISI and the GeoVu headers in
 * the introductory example:
 * 
 * begin name_equiv
 * 	$data_representation data%type
 * 		char float char real
 * 		char short char integer
 * 		char uchar char byte
 * 	$number_of_rows rows
 * 	$number_of_columns columns
 * 	$minimum_value min.%value
 * 	$maximum_value max.%value
 * end name_equiv
 * 
 * The lines containing two fields specify parameter name
 * equivalencies:  the first field (which starts with a dollar
 * sign, i.e., "$") is the GeoVu keyword and the second is the
 * native keyword with "%" substituted for blanks.  The lines
 * containing four fields specify parameter value
 * equivalencies:  the first and third fields are the GeoVu and
 * native parameter value type, respectively, and the second
 * and fourth fields are the parameter values, respectively.
 * Parameter value equivalencies are associated with the
 * immediately preceding parameter name equivalence; it would
 * be invalid to begin a Name Equivalence section with
 * parameter value equivalencies.
 * 
 * Three parameter value equivalencies (more simply called
 * "translators") are listed above under the assumption that
 * the native "data type" may take any one of the three values
 * "real", "integer", or "byte" which would be translated into
 * the GeoVu parameter values "float", "short", or "uchar",
 * respectively.
 * 
 * The name table is used to dynamically translate from native
 * headers to FreeForm (e.g., GeoVu) headers.  Only one name
 * table would be needed to translate any number of native
 * headers into GeoVu headers, which is preferable to
 * statically translating native headers.
 * 
 * The name table is implemented as buffer with an associated
 * format.  The buffer contains the parameter values, and each
 * variable in the format variable list contains the parameter
 * name, type, and location in the buffer.  Translators are
 * currently implemented as a singly linked list of TRANSLATOR
 * structures attached to a variable through its info pointer,
 * but in the future will be implemented as a separate name
 * table.
 * 
 * Creating name tables, and adding and deleting parameters is
 * simply creating buffers and formats, and adding and deleting
 * bytes in the buffer and variables in the format's variable
 * list.  Querying a name table is simply looking up a format's
 * variable by name, and then retrieving the relevant contents
 * of the associated buffer.  Translating a parameter value
 * adds the step of looking up the retrieved value in the list
 * of translators and retrieving the correlated GeoVu value.
 *
 * The name table is created from either a file or a buffer.  If the filename
 * argument is NULL, then the buffer argument must be a text buffer containing
 * the name table text.
 *
 * ERRORS:
 * Problem reading file, ((filename) ? filename : "Empty buffer")
 * Out of memory, "Name Table"
 * Problem defining name table, "\nmismatch >begin name_equiv< with >end constant<"
 * Problem defining name table,"loss >end name_equiv<-"
 * Problem defining name table, value
 * Out of memory,"NAME_EQU"
 * Problem defining name table, value
 * Out of memory,"Translator"
 * Unknown variable type,"Translator"
 * Problem defining name table,"\nwrong geovu value in the translation table"
 * ,"\nwrong user's value in the translation table"
 * Problem defining name table, line
 * Problem defining name table, "nt_create:loss ->end name_equiv<-"
 * Problem defining name table, "\nmismatch >begin constant< with >end name_equiv<"
 * Problem defining name table,"loss ->end constant<-"
 * Problem defining name table, value
 * Out of memory,"NAME_CONST"
 * Unknown variable type, ch
 * Problem in conversion, value
 * Problem defining name table, "nt_create:loss ->end constant<-"
 * Problem defining name table, line
 *
 * ERROR values:
 * ERR_DATA_GT_BUFFER
 * ERR_MEM_LACK
 * ERR_MISPLACED_SECTION_END
 * ERR_MISPLACED_SECTION_START
 * ERR_VARIABLE_NOT_FOUND
 * ERR_UNKNOWN_VAR_TYPE
 * ERR_NT_DEFINE
 * ERR_NUM_TOKENS
 * ERR_GET_VALUE
 * nt_add_constant()
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:  
 *
 * AUTHOR:  Ted Habermann
 * Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:  Strings are stored in the name table with their NULL terminators.
 *
 * KEYWORDS: data dictionary, translation.
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	8/21/95		-rf01
 *		fix newlines first in buffer to deal with Mac EOL
*/
 
#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_create"

#define NEVER_IN_TABLE 3
#define NOT_IN_TABLE 0
#define IN_CONSTANT_TABLE 1
#define IN_EQUIV_TABLE 2

#define COUNT_TOKENS_ONLY 1
#define MAX_TOKENS 4

#define NUM_NAME_EQUIV_TOKENS 2
#define NUM_CONSTANT_TOKENS 3
#define NUM_TRANSLATOR_TOKENS 4

#ifdef PROTO
NAME_TABLE_PTR nt_create(char *filename, char *buffer)
#else
NAME_TABLE_PTR nt_create(filename,buffer)
char *filename;
char *buffer;
#endif
{
	NAME_TABLE_PTR table    = NULL; 
	TRANSLATOR_PTR trans    = NULL;
	
	char *ch_ptr          = NULL;
	char *line            = NULL;
	char *next_line       = NULL;
	char  tokens[MAX_TOKENS][MAX_PV_LENGTH + 1];
	char *geovu_name      = NULL;
	char *gvalue_type_str  = NULL;
	char *geovu_value_str = NULL;
	char *user_name       = NULL;
	char *uvalue_type_str   = NULL;
	char *user_value_str  = NULL;
	char scratch_buffer[MAX_PV_LENGTH + 1];
	
	int num_tokens;
	
	long file_length = 0L;
	
	FF_TYPES_t gvalue_type = 0;
	FF_TYPES_t uvalue_type = 0;
	
	unsigned char status = NEVER_IN_TABLE;
	
	VARIABLE_PTR var = NULL;
  
	if (filename)
	{
		file_length = os_filelength(filename);
		if (file_length == -1)
		{
			err_push(ROUTINE_NAME, ERR_OPEN_FILE, filename);
			return(NULL);
		}

		if (file_length >= UINT_MAX)
		{
			err_push(ROUTINE_NAME, ERR_DATA_GT_BUFFER, "oversized name table");
			return(NULL);
		}
		buffer = (char *)memMalloc((size_t)file_length + 1, "buffer");
		if (!buffer)
		{
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "buffer to read name table");
			return(NULL);
		}
	
		file_length = ff_file_to_buffer(filename, buffer);
	}
	else 
	{
		if (buffer)	/* -rf01 */
			ff_make_newline(buffer);	/* fix newlines first -rf01 */
		file_length = ok_strlen(buffer);
	}

	/* allocate and initialize NAME_TABLE */
	table = (NAME_TABLE *)memMalloc(sizeof(NAME_TABLE), "table");
	if (!table)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Name Table");
		if (filename)
			memFree(buffer, "buffer");
		return((NAME_TABLE_PTR)NULL);
	}
	table->check_address = (void *)table;
	
	/* Allocate NAME_TABLE elements */
	table->data = ff_create_bufsize(NAME_TABLE_QUANTA);
	if (!table->data)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Name Table data");
		memFree(table, "Name Table");
		if (filename)
			memFree(buffer, "buffer");
		return((NAME_TABLE_PTR)NULL);
	}
	
	/* Make a FORMAT */
	table->format = (FORMAT_PTR)memMalloc(sizeof(FORMAT), "table->format");
	if (!table->format)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Name Table Format");
		ff_destroy_bufsize(table->data);
		memFree(table, "table");
		if (filename)
			memFree(buffer, "buffer");
		return((NAME_TABLE_PTR)NULL);
	}
	table->format->variables = dll_init();
	if (!table->format->variables)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK,"Name Table Format Variables");
		if (filename)
			memFree(buffer, "buffer");
		return((NAME_TABLE_PTR)NULL);
	}
	table->format->num_in_list = 0;
	table->format->max_length  = 0;
	table->format->type	= FFF_BINARY;
	table->format->check_address = (void *)table->format;

	ch_ptr = NULL;
	os_path_find_parts(filename, NULL, &ch_ptr, NULL);
	sprintf(table->format->name, "Name Table format created from %s",
	        filename ? ch_ptr : (ok_strlen(buffer) ? "buffer" : "empty template"));

	if (!file_length)
		return(table);

	/* parse the equivalence table */
	line = buffer;
	
	/* make sure that last character is newline */
	if (*(buffer + file_length - 1) != '\n')
	{
		*(buffer + file_length) = '\n';
		file_length++;
	}

	/* Find the end of the line */
	while ( (ch_ptr = strchr(line, '\n')) != NULL &&
	 (ch_ptr - buffer < file_length) )
	{
		*ch_ptr = STR_END;	/* terminate line and set next */
		next_line = ch_ptr + 1;
		
		switch (status)
		{
			case NEVER_IN_TABLE:
			case NOT_IN_TABLE:
				if (strstr(line,"begin name_equiv") != NULL)
				{
					status = IN_EQUIV_TABLE;
					break;
				}
				else if ((strstr(line,"begin constant")) != NULL)
				{
					status = IN_CONSTANT_TABLE;
					break;
				}
			break;
				
			case IN_EQUIV_TABLE:	/* begin the name-equiv block */
	
				/* At the end ? */
				if ((strstr(line,"end name_equiv")) != NULL)
				{
					status = NOT_IN_TABLE;
					break;
				}
	
				/* error check */
				if ((strstr(line,"end constant")) != NULL)
				{
					err_push(ROUTINE_NAME, ERR_MISPLACED_SECTION_END, "out of place \"end constant\"");
					fd_free_format_data(table);
					table = NULL;
					if (filename)
						memFree(buffer, "buffer");
					return((NAME_TABLE_PTR)NULL);
				}
	
				if ((strstr(line,"begin constant")) != NULL)
				{
					err_push(ROUTINE_NAME, ERR_MISPLACED_SECTION_START, "out of place \"begin constant\"");
					fd_free_format_data(table);
					table = NULL;
					if (filename)
						memFree(buffer, "buffer");
					return((NAME_TABLE_PTR)NULL);
				}

				if ((strstr(line,"begin name_equiv")) != NULL)
				{
					err_push(ROUTINE_NAME, ERR_MISPLACED_SECTION_START, "too many \"begin name_equiv\"'s");
					fd_free_format_data(table);
					table = NULL;
					if (filename)
						memFree(buffer, "buffer");
					return((NAME_TABLE_PTR)NULL);
				}
					
				/* The line is name-equivalence table:
				In the lines which contain two fields the first field (which
				may start with a $) is the GeoVu keyword and the second is the
				user keyword with "%" substituted for blanks.
				
				Lines with 4 fields give conversions between values of keywords. */
	
				num_tokens = parse_line_into_tokens_by_case(status, line, tokens, !COUNT_TOKENS_ONLY);

				switch (num_tokens)
				{
					case 0:
						break;
					
					case NUM_NAME_EQUIV_TOKENS:
						trans = NULL;	/* End TRANSLATOR Creation */
						geovu_name = tokens[0];
						if (*geovu_name == '$')
							++geovu_name;
						os_str_replace_unescaped_char1_with_char2('%', ' ', geovu_name);
		
						user_name = tokens[1];
						os_str_replace_unescaped_char1_with_char2('%', ' ', user_name);
						
						/* Create a character type variable with the GeoVu name as
						the variable name and the user name as the value */
						gvalue_type = ff_lookup_number(variable_types, "char");
						if (nt_add_constant(&table, geovu_name, gvalue_type, (void *)user_name))
						{
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return((NAME_TABLE_PTR)NULL);
						}
						var = FFV_VARIABLE(dll_last(table->format->variables));
						var->type |= FFV_EQUIV;
						if (parse_line_into_tokens_by_case(status, next_line, tokens, COUNT_TOKENS_ONLY)
						 == NUM_TRANSLATOR_TOKENS)
						{
							var->type |= FFV_TRANSLATOR;
						}
					break;
						
					case NUM_TRANSLATOR_TOKENS:
						/* In the 4 token case the line gives a correspondence between
						possible values of the user variable and possible values of the
						geovu variable. This case is handled by a TRANSLATOR which is
						attached to the info pointer of the variable */
						gvalue_type_str = tokens[0];
						geovu_value_str = tokens[1];
						uvalue_type_str = tokens[2];
						user_value_str = tokens[3];
						os_str_replace_unescaped_char1_with_char2('%', ' ', geovu_value_str);
						os_str_replace_unescaped_char1_with_char2('%', ' ', user_value_str);
						
						if (trans)
						{ /* Add to existing TRANSLATOR */
							trans->next = (TRANSLATOR_PTR)memMalloc(sizeof(TRANSLATOR), "TRANSLATOR");
							trans = trans->next;
						}
						else
						{
							var = FFV_VARIABLE(dll_last(table->format->variables));
							if (!var)
							{
								err_push(ROUTINE_NAME, ERR_VARIABLE_NOT_FOUND,"Last Variable (for translator)");
								fd_free_format_data(table);
								table = NULL;
								if (filename)
									memFree(buffer, "buffer");
								return(NULL);
							}
							var->info = trans = (TRANSLATOR_PTR)memMalloc(sizeof(TRANSLATOR), "TRANSLATOR");
						}
						if (!trans)
						{
							err_push(ROUTINE_NAME, ERR_MEM_LACK,"Translator");
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return(NULL);
						}
		
						/* Fill the translator fields */
						gvalue_type = ff_lookup_number(variable_types, gvalue_type_str);
						uvalue_type  = ff_lookup_number(variable_types, uvalue_type_str);
						if (gvalue_type == USHRT_MAX || uvalue_type == USHRT_MAX)
						{
							memFree((char *)trans, "trans");
							err_push(ROUTINE_NAME, ERR_UNKNOWN_VAR_TYPE,"Translator");
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return(NULL);
						}
						if ((trans->gvalue = nt_str_to_binary(geovu_value_str, gvalue_type)) == NULL)
						{
							memFree((char *)trans, "trans");
							err_push(ROUTINE_NAME, ERR_NT_DEFINE,"Bad GeoVu value in the translation table");
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return(NULL);
						}
						if ((trans->uvalue = nt_str_to_binary(user_value_str, uvalue_type)) == NULL)
						{
							memFree((char *)trans->gvalue, "trans->gvalue");
							memFree((char *)trans, "trans");
							err_push(ROUTINE_NAME, ERR_NT_DEFINE ,"Bad user value in the translation table");
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return(NULL);
						}
		
						trans->gtype = gvalue_type;
						trans->utype = uvalue_type;
						trans->next = (TRANSLATOR_PTR)NULL;
						trans->check_address = (void *)trans;
					break;
						
					default:
						err_push(ROUTINE_NAME, ERR_NUM_TOKENS, line);
						fd_free_format_data(table);
						table = NULL;
						if (filename)
							memFree(buffer, "buffer");
						return((NAME_TABLE_PTR)NULL);
					} /* end of switch (num_tokens) */
				break;	/* End of IN_EQUIV_TABLE case */
	
			case IN_CONSTANT_TABLE:
				
				if ((strstr(line,"end constant")) != NULL)
				{
					status = NOT_IN_TABLE;
					break;
				}

				/* error check */
				if ((strstr(line,"end name_equiv")) != NULL)
				{
					err_push(ROUTINE_NAME, ERR_MISPLACED_SECTION_END, "out of place \"end name_equiv\"");
					fd_free_format_data(table);
					table = NULL;
					if (filename)
						memFree(buffer, "buffer");
					return((NAME_TABLE_PTR)NULL);
				}
	
				if ((strstr(line,"begin name_equiv")) != NULL)
				{
					err_push(ROUTINE_NAME, ERR_MISPLACED_SECTION_START, "out of place \"begin name_equiv\"");
					fd_free_format_data(table);
					table = NULL;
					if (filename)
						memFree(buffer, "buffer");
					return((NAME_TABLE_PTR)NULL);
				}

				if ((strstr(line,"begin constant")) != NULL)
				{
					err_push(ROUTINE_NAME, ERR_MISPLACED_SECTION_START, "too many \"begin constant\"'s");
					fd_free_format_data(table);
					table = NULL;
					if (filename)
						memFree(buffer, "buffer");
					return((NAME_TABLE_PTR)NULL);
				}
						
				num_tokens = parse_line_into_tokens_by_case(status, line, tokens, !COUNT_TOKENS_ONLY);

				switch(num_tokens)
				{
					case 0:
						break;
					
					case NUM_CONSTANT_TOKENS:
						geovu_name = tokens[0];
						gvalue_type_str = tokens[1];
						geovu_value_str = tokens[2];
						
						gvalue_type = ff_lookup_number(variable_types, gvalue_type_str);
						if (gvalue_type == USHRT_MAX)
						{
							err_push(ROUTINE_NAME, ERR_UNKNOWN_VAR_TYPE, gvalue_type_str);
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return((NAME_TABLE_PTR)NULL);
						}
						os_str_replace_unescaped_char1_with_char2('%', ' ', geovu_value_str);
						if (ff_string_to_binary(geovu_value_str, gvalue_type, scratch_buffer))
						{
							err_push(ROUTINE_NAME, ERR_GET_VALUE, geovu_name);
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return((NAME_TABLE_PTR)NULL);
						}
						if (nt_add_constant(&table, geovu_name, gvalue_type, (void *)scratch_buffer))
						{
							fd_free_format_data(table);
							table = NULL;
							if (filename)
								memFree(buffer, "buffer");
							return((NAME_TABLE_PTR)NULL);
						}
						var = FFV_VARIABLE(dll_last(table->format->variables));
						var->type |= FFV_CONSTANT;
					break;
				
					default:
						err_push(ROUTINE_NAME, ERR_NUM_TOKENS, line);
						fd_free_format_data(table);
						table = NULL;
						if (filename)
							memFree(buffer, "buffer");
						return((NAME_TABLE_PTR)NULL);
				} /* end of switch (num_tokens) */
			break;	/* End of IN_CONSTANT_TABLE case */
		} /* end of switch (status) */
		
		line = next_line;
		
	}	/* End of line parsing loop */

	if (status == IN_CONSTANT_TABLE)
	{
		err_push(ROUTINE_NAME, ERR_EXPECTING_SECTION_END, "expecting \"end constant\"");
		fd_free_format_data(table);
		table = NULL;
		if (filename)
			memFree(buffer, "buffer");
		return((NAME_TABLE_PTR)NULL);
	}
	else if (status == IN_EQUIV_TABLE)
	{
		err_push(ROUTINE_NAME, ERR_EXPECTING_SECTION_END, "expecting \"end name_equiv\"");
		fd_free_format_data(table);
		table = NULL;
		if (filename)
			memFree(buffer, "buffer");
		return((NAME_TABLE_PTR)NULL);
	}
	else if (status == NEVER_IN_TABLE)
	{
		err_push(ROUTINE_NAME, ERR_EXPECTING_SECTION_START, "expecting \"begin constant\" or \"begin name_equiv\"");
		fd_free_format_data(table);
		if (filename)
			memFree(buffer, "buffer");
		return(NULL);
	}

	if (filename)
		memFree(buffer, "buffer");

	return(table);
}

/*
 * NAME:	nt_add_constant
 *              
 * PURPOSE:	To add a constant into name table.
 *
 * USAGE:	NAME_TABLE *table_add_constant(NAME_TABLE_HANDLE htable, char *name, short var_type, void *value_ptr)
 *			htable: a handle (pointer to pointer) to the NAME_TABLE.
 *			If htable points to NULL, this function will create a table.
 *			name: the constant name.
 * 			var_type: the type of the constant, using FREEFORM var type.
 *			value_ptr: a pointer to the value.
 *
 * RETURNS:	Zero if sucessful, an error code if failed
 *
 * DESCRIPTION:  Appends a new variable into the name table's format's
 * variable list.  The contents of value_ptr are appended into the name
 * table's values buffer, which is reallocated as necessary by
 * NAME_TABLE_QUANTA increments.  The name table's format's num_in_list and
 * max_length are incremented by one and according to var_type, respectively.
 *
 * If the named constant to be added already exists in the table, then the
 * previous constant in the table is deleted, effectively being replaced by
 * the added constant.
 *
 * Caution must be used if this function is not called by another name table
 * function.  This function will create a name table if necessary, but
 * functions other than name table functions typically deal with LISTS of
 * name tables, and not just single name tables.
 *
 * The usual paradigm for doing this is given below (though I don't encourage
 * this, because I might change the implementation, you're supposed to be
 * insulated from this internal stuff, and I don't want to have to track down
 * a lot of external, specialized code).  That said,
 *
 *	table = nt_get_name_table(dbin, FFF_INPUT);
 *	if (table)
 *	{
 *		if (nt_add_constant(&table, "histogram_dir", FFV_CHAR, hist_dir))
 *			return(0);
 *	}
 *	else
 *	{
 *		if (nt_add_constant(&table, "histogram_dir", FFV_CHAR, hist_dir))
 *			return(0);
 *		if (nt_put_name_table(dbin, FFF_INPUT, table))
 *			return(0);
 *	}
 *
 * ERRORS:	Unknown variable type, NULL
 *			Undefined error, "unable to create the table"
 *			Out of memory, "allocating name_const"
 *			Unknown variable type, NULL
 *			Out of memory, "fail to allocate the constant"
 *
 * ERROR values:
 * ERR_UNKNOWN_VAR_TYPE
 * ERR_MEM_LACK
 *
 *
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS: This function is used to put both constants and name_equiv's into
 * the name table, though any translators which may be associated with a
 * name_equiv cannot be added with this function.
 *
 * KEYWORDS:    name table.
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_add_constant"

#ifdef PROTO
int nt_add_constant(NAME_TABLE_HANDLE htable, char *name, FF_TYPES_t var_type, void *value_ptr)
#else
int nt_add_constant(htable, name, var_type, value_ptr)
NAME_TABLE_HANDLE htable;
char *name;
FF_TYPES_t var_type;
void *value_ptr;
#endif

{
	size_t var_length = 0;
	VARIABLE_LIST_PTR v_list_ok = NULL;

	VARIABLE_PTR var;

	assert(name);
	assert(value_ptr);
	
	assert(htable);
	if (!htable)
		return(ERR_PTR_DEF);

	if (!*htable)
	{
		*htable = nt_create(NULL, NULL);
		if (!*htable)
			return(0);
		
		assert(*htable == (*htable)->check_address);
	}

	/* Replace constant if it already exists */
						
	if (ff_find_variable(name, (*htable)->format))
		(void)nt_delete_constant(*htable, name);
						
	var	= NULL;

	if (IS_TEXT_TYPE(var_type))
		var_length = strlen((char *)value_ptr) + 1;
	else if (IS_INTEGER_TYPE(var_type) || IS_REAL_TYPE(var_type))
		var_length = ffv_type_size(var_type);
	else
		assert(0);

	while ((*htable)->data->total_bytes < (size_t)(*htable)->format->max_length + var_length)
	{
 		if (ff_resize_bufsize(  (*htable)->data->total_bytes + NAME_TABLE_QUANTA
		                      , &(*htable)->data))
			return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Increase Table buffer size"));
	}

	var = (VARIABLE *)memMalloc(sizeof(VARIABLE), "var");
	if (!var)
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK,"Adding Name Table Variable"));

	strcpy(var->name, name);
	var->start_pos = (*htable)->format->max_length + 1;
	var->end_pos = var->start_pos + var_length - 1;
	var->type = var_type;
	if (IS_REAL(var))
		var->precision = 6;
	else
		var->precision = 0;
	var->check_address = (void*)var;
	var->info = NULL;

	if (var_length > (*htable)->data->total_bytes - (*htable)->data->bytes_used)
	{
		/* this should be impossible, after resizing above */
		assert(var_length <= (*htable)->data->total_bytes - (*htable)->data->bytes_used);
	}
	else
	{
		memcpy((void *)((*htable)->data->buffer + (*htable)->format->max_length),
			(void *)value_ptr, var_length);
		(*htable)->data->bytes_used += var_length;

		++(*htable)->format->num_in_list;
		(*htable)->format->max_length += var_length;
		
		/* Add variable to format */
		v_list_ok = dll_add(dll_last((*htable)->format->variables), 0);
		if (!v_list_ok)
		{
			ff_free_format((*htable)->format);
			(*htable)->format = NULL;
			return(err_push(ROUTINE_NAME, ERR_MEM_LACK,"Table Variable"));
		}
		dll_data(v_list_ok) = (void *)var;
	}

	return(0);
}

int nt_delete_constant(NAME_TABLE_PTR table, char *name)
/*****************************************************************************
 * NAME:  nt_delete_constant()
 *
 * PURPOSE:  Delete the named constant (or name_equiv) from table
 *
 * USAGE:  error = nt_delete_constant(table, name);
 *
 * RETURNS:  zero on success, otherwise a non-zero failure code
 *
 * DESCRIPTION:  Searches table's format's variable list for name and that
 * variable is removed from the variable list.  So as to avoid fragmenting
 * the values buffer, the deleted variable's space in the values buffer,
 * which is now a hole, is filled over with the remaining variables' constant
 * values.  The remaining variables in the variable list have their start
 * and end positions adjusted accordingly.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/
{
	VARIABLE_LIST_PTR v_list = NULL,
	                  var_node = NULL;
	size_t adjustment = 0;

	assert(table);
	assert(table == table->check_address);
	assert(table->format);
	assert(table->format == table->format->check_address);
	
	v_list = FFV_FIRST_VARIABLE(table->format);

	if (name == NULL || strlen(name) == 0)
		return(ERR_MISSING_TOKEN);

	/* This duplicates ff_find_variable(), but I need the node */
	while(FFV_VARIABLE(v_list))
	{
		assert(FFV_VARIABLE(v_list));
		assert(FFV_VARIABLE(v_list) == FFV_VARIABLE(v_list)->check_address);

		if (memStrcmp(name, FFV_VARIABLE(v_list)->name,NO_TAG) == 0)
			break;
		v_list = dll_next(v_list);
	}

	if (FFV_VARIABLE(v_list) == NULL)
		return(ERR_MISSING_VAR);
	
	var_node = v_list;

	adjustment = FFV_VARIABLE(var_node)->end_pos -
	 FFV_VARIABLE(var_node)->start_pos + 1;
	
	/* Variable is last in the list, or isn't */
	
	if (FFV_VARIABLE(dll_next(var_node)) != NULL)
	{ /* variable is in the middle, or start -- move it on down */
		memmove(table->data->buffer + FFV_VARIABLE(var_node)->start_pos - 1,
		 table->data->buffer + FFV_VARIABLE(var_node)->end_pos,
		 table->format->max_length - FFV_VARIABLE(var_node)->end_pos);
		
		table->data->bytes_used -= adjustment;
		
		v_list = dll_next(v_list);
		while (FFV_VARIABLE(v_list))
		{
			assert(FFV_VARIABLE(v_list));
			assert(FFV_VARIABLE(v_list) == FFV_VARIABLE(v_list)->check_address);

			FFV_VARIABLE(v_list)->start_pos -= adjustment;
			FFV_VARIABLE(v_list)->end_pos -= adjustment;
			v_list = dll_next(v_list);
		}
	}
	else
	{ /* variable is last in the list -- do nothing special */
		;
	}
		
	table->format->max_length -= adjustment;
	--table->format->num_in_list;
	
  if (IS_TRANSLATOR(FFV_VARIABLE(var_node)))
  {
  	nt_free_trans((struct t_table *)FFV_VARIABLE(var_node)->info);
  	FFV_VARIABLE(var_node)->info = NULL;
  }
	dll_delete(var_node, (void (*)(void *))ff_free_variable);

	return(0);
}

/*
 * NAME:	nt_askexist
 *              
 * PURPOSE:	To determine whether a named parameter exists in the header or
 * constant section of the name table.
 *
 * USAGE:	BOOLEAN nt_askexist(DATA_BIN_PTR dbin, char *name)
 *
 * RETURNS:	If the named variable exists, return TRUE, otherwise, FALSE.
 *
 * DESCRIPTION: This function performs the same searches as nt_askvalue(),
 * except the operating system environment is not checked.  Use this function
 * to determine if a keyword exists in a file header or name table, but not
 * the operating system environment.
 *
 * Only the constant section of the name table is searched (name equivalances
 * are not found by nt_askexist()).
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS: none
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_askexist"

#ifdef PROTO
BOOLEAN nt_askexist(DATA_BIN_PTR dbin, char *name)
#else
BOOLEAN nt_askexist(dbin, name)
DATA_BIN_PTR dbin;
char *name;
#endif

{
	char *user_name = NULL;
	BOOLEAN found = FALSE;
	NAME_TABLE_PTR table = NULL;
	VARIABLE_PTR var = NULL;

	assert(dbin);
	assert(dbin == dbin->check_address);
	
	(void)fd_get_format_data(dbin->table_list, FFF_INPUT | FFF_TABLE, &table);
	if (table)
	{
		assert(table == table->check_address);
		assert(table->format == table->format->check_address);

		user_name = (char *)nt_find_user_name(table, name);
	}
	/* Try header */
	if (dbin->header && dbin->header_format)
	{
		if (ff_find_variable(name, dbin->header_format))
			found = TRUE;
		else if (user_name && ff_find_variable(user_name, dbin->header_format))
			found = TRUE;
	}	 
  
	/* Try constant table */
  if (!found && table)
  {
  	if ((var = ff_find_variable(name, table->format)) != NULL && IS_CONSTANT(var))
			found = TRUE;
		else if (user_name &&
		         (var = ff_find_variable(user_name, table->format)) != NULL &&
		         IS_CONSTANT(var))
			found = TRUE;
  }

	if (user_name)
		memFree(user_name, "user_name");

	return(found);
}

/*
 * NAME:	nt_askvalue
 *              
 * PURPOSE:	To get the value of a variable using its dictionary name
 *
 * USAGE:	BOOLEAN nt_askvalue(DATA_BIN_PTR dbin, char *geovu_name, FF_TYPES_t request_type, void *dest, char *buffer) 
 *
 * RETURNS:	If success, return TRUE, otherwise, FALSE.
 *
 * DESCRIPTION:
 * The header format, name table, and environment are searched in order
 * until geovu_name is found.  For each of the above searches if geovu_name
 * is not found then user_name (the name equivalent of geovu_name, if it
 * exists) is searched for.  If it is found, then its associated value is
 * retrieved from the header, name table, or environment, as appropriate.
 * The retrieved value is converted into dest according to request_type, with
 * translation, if appropriate.
 *
 * If the value type is unconvertable (i.e. char to numerical value
 * or numerical to char) or geovu_name is not found the function will return
 * FALSE.
 *
 * SYSTEM DEPENDENT FUNCTIONS: none     
 *
 * GLOBALS:
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * ERRORS:		Out of memory, "geovu_value"
 *				Pointer not defined, "header"
 *
 * COMMENTS:  The incoming parameter buffer is no longer being used.
 * A quick kluge for nt_askvalue() to specify either an input or an output
 * name table/header to ask is to piggy back either FFF_INPUT or FFF_OUTPUT
 * along with the variable type in the request_type parameter.  However,
 * FFF_INPUT and FFF_OUTPUT (as format types) confict with FFF_CONSTANT and
 * FFF_INITIAL (as variable types), respectively.
 *
 * KEYWORDS:    dictionary, databin.
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_askvalue"

#ifdef PROTO
BOOLEAN nt_askvalue(DATA_BIN_PTR dbin, char *geovu_name, FF_TYPES_t request_type, void *dest, char *i_am_never_used)
#else
BOOLEAN nt_askvalue(dbin, geovu_name, request_type, dest, i_am_never_used)
DATA_BIN_PTR dbin;
char *geovu_name;
FF_TYPES_t request_type;
void *dest;
char *i_am_never_used;
#endif
{
	char *user_name = NULL;
	char *value_ptr = NULL;
	void *user_value = NULL;
	char scratch_buffer[MAX_PV_LENGTH];
	char geovu_value[MAX_PV_LENGTH];
	FF_TYPES_t uvalue_type, gvalue_type;
	BOOLEAN found = FALSE;

	FORMAT_PTR format = NULL;
	VARIABLE_PTR var = NULL;
	NAME_TABLE_PTR table = NULL;
	
	FF_TYPES_t format_type = FFF_IO & request_type;
	if (!format_type)
		format_type = FFF_INPUT;

	assert(dbin);
	assert(dbin == dbin->check_address);
	assert(dest);
	assert(geovu_name);
	
	(void)fd_get_format_data(dbin->table_list, format_type | FFF_TABLE, &table);
	if (table)
	{
		assert(table == table->check_address);
		assert(table->format == table->format->check_address);
	}

	/* find the user-defined name from equivalence-name table -- will be set
	   to NULL if table doesn't exist */
	user_name = (char *)nt_find_user_name(table, geovu_name);

	/* Check the header */
	if (dbin->header && dbin->header_format)
	{
		format = dbin->header_format;
		assert(format == format->check_address);

		value_ptr = dbin->header;
		
		var = ff_find_variable(geovu_name, format);
		if (!var && user_name)
			var = ff_find_variable(user_name, format);
		if (var && (var->start_pos <= var->end_pos))
		{
			value_ptr += var->start_pos - 1;
	                                               
			/* Check to see if the string "unknown" is in the variable space */
			if ((FF_VAR_LENGTH(var) >= 7) && !IS_TEXT(var))
			{
				strncpy(scratch_buffer, value_ptr, 7);
				*((char *)scratch_buffer + 7) = STR_END;
				if ((os_strcmpi((char *)scratch_buffer, "unknown")) == 0)
				{
					if (user_name)
						memFree(user_name, "user_name");
					return(FALSE);
				}
			}

			if (FF_VAR_LENGTH(var) >= sizeof(scratch_buffer) ||
			    ff_get_value(var, (void *)value_ptr, (void *)scratch_buffer, format->type))
			{
				assert(FF_VAR_LENGTH(var) < sizeof(scratch_buffer));
				err_push(ROUTINE_NAME, ERR_GET_VALUE, "Converting Variable From Header");
				if (user_name)
					memFree(user_name, "user_name");
				return (!ERR_GET_VALUE);
			}
			found = TRUE;
			uvalue_type = FFV_DATA_TYPE(var);
		} /* if found a variable with valid start/end positions */
	} /* if a header and a header format */
	
	/* Check name table */
	if (!found && table)
	{
		format = table->format;
		assert(format);
		assert(format == format->check_address);

		value_ptr = table->data->buffer;

		var = ff_find_variable(geovu_name, format);
		if (!var && user_name)
			var = ff_find_variable(user_name, format);
		if (var && (var->start_pos <= var->end_pos))
		{
			value_ptr += var->start_pos - 1;
	                                               
			/* Check to see if the string "unknown" is in the variable space */
			if ((FF_VAR_LENGTH(var) >= 7) && !IS_TEXT(var))
			{
				strncpy(scratch_buffer, value_ptr, 7);
				*((char *)scratch_buffer + 7) = STR_END;
				if ((os_strcmpi((char *)scratch_buffer, "unknown"))== 0)
				{
					if (user_name)
						memFree(user_name, "user_name");
					return(FALSE);
				}
			}
	
			if (FF_VAR_LENGTH(var) >= sizeof(scratch_buffer) ||
			    ff_get_value(var, (void *)value_ptr, (void *)scratch_buffer, format->type))
			{
				assert(FF_VAR_LENGTH(var) < sizeof(scratch_buffer));
				err_push(ROUTINE_NAME, ERR_GET_VALUE, "Converting Variable From Name Table");
				if (user_name)
					memFree(user_name, "user_name");
				return(!ERR_GET_VALUE);
			}
			found = TRUE;
			uvalue_type = FFV_DATA_TYPE(var);
		} /* if found a variable with valid start/end positions */
	} /* if not found so far and there is a name table */
	
	/* Check the environment */
	if (!found)
	{
		if ((user_value = (void *)os_get_env(geovu_name)) != NULL)
			found = TRUE;

		if (!found)		
		{
			strcpy(scratch_buffer, geovu_name);
			if ((user_value = (void *)os_get_env(os_strupr(scratch_buffer))) != NULL)
				found = TRUE;
		}
		
		if (!found && user_name)
			if ((user_value = (void *)os_get_env(user_name)) != NULL)
				found = TRUE;
		
		if (!found && user_name)
		{
			strcpy(scratch_buffer, user_name);
			if ((user_value = (void *)os_get_env(os_strupr(scratch_buffer))) != NULL)
				found = TRUE;
		}
		
		if (found)
		{
			if (ff_string_to_binary((char *)user_value, FFV_DATA_TYPE_TYPE(request_type), scratch_buffer))
			{
				err_push(ROUTINE_NAME, ERR_GET_VALUE, "Converting Environment Variable String");
				if (user_name)
					memFree(user_name, "user_name");
				memFree(user_value, "user_value");
				return(!ERR_GET_VALUE);
			}

			uvalue_type = FFV_DATA_TYPE_TYPE(request_type);
		}
	} /* if not found so far */

	if (user_name)
		memFree(user_name, "user_name");
	if (user_value)
		memFree(user_value, "user_value");
	
	if (!found)
		return(!ERR_MISSING_VAR);

	/* if it is character string, get rid of the leading and trailing space */
	user_value = (void *)scratch_buffer;
	if (IS_TEXT_TYPE(uvalue_type))
		(void)os_str_trim_whitespace((char *)user_value, (char *)user_value);

	/* get geovu equivalence value from equivalence value table */
	if ((nt_get_geovu_value(table, geovu_name, user_value, uvalue_type,
	 geovu_value, &gvalue_type)) == FALSE)
	{
		if (btype_to_btype(user_value, uvalue_type, dest, FFV_DATA_TYPE_TYPE(request_type)))
			return(FALSE);
		else
			return(TRUE);
	}

	/* convert to required data type */
	if (btype_to_btype(geovu_value, gvalue_type, dest, FFV_DATA_TYPE_TYPE(request_type)))
		return(FALSE);
	else
		return(TRUE);
}                

/*
 * NAME:	nt_find_user_name
 *              
 * PURPOSE:	To find user-defined name using a geovu dictionary name
 *
 * USAGE:	char *nt_find_user_name(NAME_TABLE_PTR table, char *geovu_name)
 *
 * RETURNS:	if success, return a pointer to user-defined name,
 * otherwise return NULL.
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    dictionary, equivalence name.
 *
 */

#ifdef PROTO
char *nt_find_user_name(NAME_TABLE_PTR table, char *geovu_name)
#else
char *nt_find_user_name(table, geovu_name)
NAME_TABLE_PTR table;
char *geovu_name;
#endif
{
	VARIABLE_PTR var;

	if (!table)
		return(NULL);
	
	assert(table == table->check_address);
	assert(table->format);
	assert(table->format == table->format->check_address);

	if (!geovu_name)
		return(NULL);

	var = ff_find_variable(geovu_name, table->format);
	if (!var)
		return((char *)NULL);
	
	if (var && IS_EQUIV(var))
		return((char *)memStrdup(table->data->buffer + var->start_pos - 1, NO_TAG));
	else
		return(NULL);
}

/*
 * NAME:	nt_putvalue
 *              
 * PURPOSE:	To put a value to header or name table
 *
 * USAGE:	BOOLEAN nt_putvalue(DATA_BIN_PTR dbin, char *geovu_name,
 *				FF_TYPES_t gvalue_type, void *value, short header_only)
 *			dbin: data bin;
 *			name: the geovu name of the value;
 *			type: the type of the value to be put;
 *			value: the value to be put;
 *			header_only: NT_HEADER_ONLY, put the value in header only.
 *				NT_ANYWHERE, the value will be put in the constant
 *				table if can not find corresponding variable in the header.
 *
 * RETURNS:     if success, return TRUE, otherwise, FALSE.
 *
 * DESCRIPTION: First convert geovu dictionary name (geovu_name)
 *				to user-defined name.
 *				Uses this name to find the location where the value is stored. and
 *				If found, replace the old value.
 *				If it can't be found, and header_only is false,
 *				the value is stored as a constant
 *
 * SYSTEM DEPENDENT FUNCTIONS: none     
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 *
 * ERRORS:    
 *			Unknown variable type, NULL)
 *			Out of memory, "temp")
 *			Out of memory, "user_value"
 *			Pointer not defined, "header"
 *
 * COMMENTS:    
 *
 * KEYWORDS:  name table, databin.
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_putvalue"

#ifdef PROTO
BOOLEAN nt_putvalue(DATA_BIN_PTR dbin, char *geovu_name,
        FF_TYPES_t gvalue_type, void *geovu_value, short header_only)
#else
BOOLEAN nt_putvalue(dbin, geovu_name, gvalue_type, geovu_value, header_only)
DATA_BIN_PTR dbin;
char *geovu_name;
FF_TYPES_t gvalue_type;
void *geovu_value;
short header_only
#endif

{
	char *user_name = NULL;
	FF_TYPES_t uvalue_type;
	char temp[MAX_PV_LENGTH];
	char user_value[MAX_PV_LENGTH];
	int error;
	NAME_TABLE_PTR table = NULL;

	FORMAT_PTR format;
	VARIABLE_PTR var;

	/* check the input arguments */
	assert(geovu_value);
	assert(geovu_name);
	assert(dbin);
	assert(dbin == dbin->check_address);

	/* get user-defined value */
	(void)fd_get_format_data(dbin->table_list, FFF_INPUT | FFF_TABLE, &table);
	if (nt_get_user_value(table, geovu_name, geovu_value,
		gvalue_type, user_value, &uvalue_type) == FALSE)
	{
		if (btype_to_btype(geovu_value, gvalue_type, (void *)user_value, gvalue_type))
			return(FALSE);
		uvalue_type = gvalue_type;
	}
	
	/* find the user-defined name from equivalence-name table -- will be set
	   to NULL if table doesn't exist */
	user_name = (char *)nt_find_user_name(table, geovu_name);

	/* find the location of the variable */
	error = 0;
	format = dbin->header_format;
	if (format && (var = ff_find_variable(user_name ? user_name : geovu_name, format)) != NULL)
	{
		if (dbin->header == NULL)
		{
			err_push(ROUTINE_NAME,ERR_HEAD_DEFINED, "Contents of header not read");
			return(FALSE);
		}

		if (!ff_binary_to_string(user_value, uvalue_type, temp, 0))
		{
			error = set_var(var, dbin->header, temp, dbin->header_format);
			if (error == -1)
				error = 1;
		}
		else
			error = 1;
	}
	else
	{
		if (header_only)
			error = 1;
		else {
			/* put in the constant table */
			if (nt_add_constant(&table, user_name ? user_name : geovu_name, uvalue_type, (void *)user_value))
				error = 1;
		}
	}
	
	if (user_name)
		memFree(user_name, "user_name");
	
	if (!error)
		return(TRUE);
	else
		return(FALSE);
}

/*
 * NAME:	nt_get_geovu_value
 *              
 * PURPOSE:	To find the GeoVu value given a geovu name and a user-defined value
 *
 * USAGE:	BOOLEAN nt_get_geovu_value(NAME_TABLE_PTR table,
 *					char *geovu_name, void *user_value, FF_TYPES_t uvalue_type,
 *					void *geovu_value, FF_TYPES_t *gvalue_type)
 *
 * RETURNS:	On success, return TRUE:
 * geovu_value and gvalue_type will be set according to
 * translating table when it exist.  Return FALSE if no translators exist for
 * geovu_name or no translator is found with a uvalue that equals user_value.
 *
 * DESCRIPTION:  The list of translators for geovu_name is searched until one
 * is found whose uvalue equals user_value.  If the types for a translator's
 * uvalue and user_value are both numeric but not identical user_value is
 * converted to the same type as the translator's so that a comparison can be
 * made.  If the values are equal the the translator's gvalue and gtype are
 * returned.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS: 
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_get_geovu_value"
 
#ifdef PROTO
BOOLEAN nt_get_geovu_value(NAME_TABLE_PTR table, char *geovu_name,
        void *user_value, FF_TYPES_t uvalue_type,
         void *geovu_value, FF_TYPES_t *gvalue_type)
#else
BOOLEAN nt_get_geovu_value(table, geovu_name, user_value, uvalue_type,
        geovu_value, gvalue_type)
NAME_TABLE_PTR table;
char *name;
void *user_value;
FF_TYPES_t uvalue_type;
void *geovu_value;
FF_TYPES_t *gvalue_type;
#endif

{
	TRANSLATOR_PTR trans = NULL;
	VARIABLE_PTR var = NULL;
	char uvalue_converted[MAX_PV_LENGTH];
	void *vpoint = NULL;

	/* check the input arguments */
	assert(geovu_name);
	assert(user_value);
	assert(uvalue_type);
	assert(geovu_value);
	assert(gvalue_type);

	if (table)
	{
		assert(table == table->check_address);
		assert(table->format);
		assert(table->format == table->format->check_address);
		var = ff_find_variable(geovu_name, table->format);
	}
	else
	{
		*gvalue_type = FFV_NULL;
		return(FALSE);
	}

	if (!IS_TRANSLATOR(var))
	{
		*gvalue_type = FFV_NULL;
		return(FALSE);
	}

	/* translate user_value to geovu_value */
	trans = (TRANSLATOR_PTR)var->info;
	while (trans != NULL)
	{
		assert(trans == trans->check_address);

		if ( FFV_DATA_TYPE_TYPE(trans->utype) == FFV_DATA_TYPE_TYPE(uvalue_type) ||
		     (!IS_TEXT_TYPE(trans->utype) && !IS_TEXT_TYPE(uvalue_type))
		   )
		{
			if (FFV_DATA_TYPE_TYPE(trans->utype) != FFV_DATA_TYPE_TYPE(uvalue_type))
			{
				(void)btype_to_btype(user_value, uvalue_type, uvalue_converted, trans->utype);
				vpoint = uvalue_converted;
			}
			else
				vpoint = user_value;
		}
		else /* Logically equivalent to trans->utype != uvalue_type AND
		        (trans->utype is char string OR uvalue_type is char string).
		        This is a user-error situation, a type mismatch between the
		        native header value and the user-value in the translator,
		        essentially asking for a string-to-numeric or vice versa
		        type conversion.
		        
		        What would be the appropriate error response? */
		{
			assert(0);
			return(FALSE);
		}
		
		if (type_cmp(trans->utype, trans->uvalue, vpoint) == 1)
		{
			*gvalue_type = trans->gtype;
			if (btype_to_btype(trans->gvalue, trans->gtype, geovu_value, trans->gtype))
				return(FALSE);
			else
				return(TRUE);
		}
				
		trans = trans->next;
	}

	*gvalue_type = FFV_NULL;
	return(FALSE);
}

/*
 * NAME:	nt_get_user_value
 *              
 * PURPOSE:	To get equivalent user_defined value by giving a geovu
 *			dictionary name, a geovu type and a geovu value.
 *
 * USAGE:	BOOLEAN nt_get_user_value(DATA_BIN_PTR dbin,
 *			char *geovu_name, FF_TYPES_t gvalue_type, void *geovu_value,
 *			FF_TYPES_t *uvalue_type, void *user_value)
 *
 * RETURNS:	If success, return TRUE, user_value and uvalue_type will be set
 * according to translation table if it exists.  Otherwise, return FALSE.  
 *
 * DESCRIPTION:  The list of translators for geovu_name is searched until one
 * is found whose gvalue equals geovu_value.  If the types for a translator's
 * gvalue and geovu_value are both numeric but not identical geovu_value is
 * converted to the same type as the translator's so that a comparison can be
 * made.  If the values are equal the the translator's uvalue and utype are
 * returned.
 *
 * ERRORS:		
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    name table
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_get_user_value"
 
#ifdef PROTO
BOOLEAN nt_get_user_value(NAME_TABLE_PTR table, char *geovu_name,
	void *geovu_value, FF_TYPES_t gvalue_type, 
	void *user_value, FF_TYPES_t *uvalue_type)
#else
BOOLEAN nt_get_user_value(table, geovu_name, geovu_value, gvalue_type,
        user_value, uvalue_type)
NAME_TABLE_PTR table;
char *geovu_name;
void *geovu_value;
FF_TYPES_t gvalue_type;
void *user_value;
FF_TYPES_t *uvalue_type;
#endif

{
	TRANSLATOR *trans = NULL;
	void *vpoint;
	char gvalue_converted[MAX_PV_LENGTH];
	VARIABLE_PTR var = NULL;

	/* check the input arguments */
	assert(geovu_name);
	assert(geovu_value);
	assert(gvalue_type);
	assert(user_value);
	assert(uvalue_type);

	/* Check for a value translator */
	if (table)
	{
		assert(table == table->check_address);
		assert(table->format);
		assert(table->format == table->format->check_address);
		assert(geovu_value);
		assert(geovu_name);
		assert(gvalue_type);
		assert(user_value);
		assert(uvalue_type);
		
		var = ff_find_variable(geovu_name, table->format);
	}
	else
	{
		*uvalue_type = FFV_NULL;
		return(FALSE);
	}

	if (!IS_TRANSLATOR(var))
	{
		*uvalue_type = FFV_NULL;
		return(FALSE);
	}

	/* translate uvalue to gvalue */
	trans = (TRANSLATOR_PTR)var->info;
	while (trans != NULL)
	{
		assert(trans == trans->check_address);
		
		/* The translator gtype must be the same as the gvalue_type given
		to this function, or, if the types are both numeric, the values must
		be the same */
		if ( FFV_DATA_TYPE_TYPE(gvalue_type) == FFV_DATA_TYPE_TYPE(trans->gtype) ||
		     (!IS_TEXT_TYPE(gvalue_type) && !IS_TEXT_TYPE(trans->gtype))
		   )
		{
			/* If types are not the same, convert the geovu value into gvalue_converted */
			if (FFV_DATA_TYPE_TYPE(gvalue_type) != FFV_DATA_TYPE_TYPE(trans->gtype))
			{
				(void)btype_to_btype(geovu_value, gvalue_type, gvalue_converted, trans->gtype);
				vpoint = gvalue_converted;
			}
			else
				vpoint = geovu_value;
				
			/* check whether the value is the same */
			if (type_cmp(trans->gtype, trans->gvalue, vpoint) == 1)
			{
				*uvalue_type = trans->utype;
				if (btype_to_btype(trans->uvalue, trans->utype, user_value, trans->utype))
					return(FALSE);
				else
					return(TRUE);
			}
		}

		trans = trans->next;
	}

	*uvalue_type = FFV_NULL;
	return(FALSE);
}

#ifdef PROTO
int nt_merge(NAME_TABLE_PTR update_table, NAME_TABLE_HANDLE htable)
#else
int nt_merge(update_table, htable)
NAME_TABLE_PTR update_table;
NAME_TABLE_HANDLE htable;
#endif

/*****************************************************************************
 * NAME: nt_merge()
 *
 * PURPOSE: Merge two name tables
 *
 * USAGE: error = nt_merge(update_table, &table);
 *
 * RETURNS: zero on success, non-zero on failure
 *
 * DESCRIPTION:  The contents of update_table are merged into table, replacing
 * already existing constants and name_equiv's in table when they exist.  The
 * NAME_TABLE contents of table are changed.
 *
 * An nt_copy() can be performed as in the following example:
 * NAME_TABLE_PTR table1, table2;
 *
 * (intervening code in which table1 is now a valid NAME_TABLE)
 *
 * table2 = NULL;
 * error = nt_merge(table1, &table2);
 *
 * Now table2 is a duplicate of table1.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_merge"

{
	VARIABLE_LIST_PTR v_list = NULL;
	VARIABLE_PTR var = NULL;
	
	assert(htable);
	assert(update_table);
	assert(update_table == update_table->check_address);
	assert(update_table->format);
	assert(update_table->format == update_table->format->check_address);
	
	if (*htable == NULL)
		*htable = nt_create(NULL, NULL);
	
	if (*htable == NULL)
		return(ERR_NT_DEFINE);
	
	v_list = dll_first(update_table->format->variables);
	while (FFV_VARIABLE(v_list))
	{
		assert(FFV_VARIABLE(v_list) == FFV_VARIABLE(v_list)->check_address);

		if (nt_add_constant(htable, FFV_VARIABLE(v_list)->name,
		 FFV_DATA_TYPE(FFV_VARIABLE(v_list)),
		 (void *)(update_table->data->buffer + FFV_VARIABLE(v_list)->start_pos - 1)))
		{
			return(1);
		}
		var = FFV_VARIABLE(dll_last((*htable)->format->variables));
		var->type |= FFV_VARIABLE(v_list)->type;
		
		if ((var->type & FFV_TRANSLATOR) == FFV_TRANSLATOR)
		{
			if (nt_copy_translator_sll(FFV_VARIABLE(v_list), var))
			{
				err_push(ROUTINE_NAME,ERR_MEM_LACK,"Translator");
				return(1);
			}
		}
		
		v_list = dll_next(v_list);
	} /* end of while() on variable list */

	return(0);
}

#ifdef PROTO
static BOOLEAN nt_copy_translator_sll(VARIABLE_PTR source_var, VARIABLE_PTR target_var)
#else
static BOOLEAN nt_copy_translator_sll(source_var, target_var)
VARIABLE_PTR source_var;
VARIABLE_PTR target_var;
#endif

/*****************************************************************************
 * NAME:  nt_copy_translator_sll()
 *
 * PURPOSE: Copies the singly linked list of TRANSLATOR's of a translator
 * VARIABLE.
 *
 * USAGE: error = nt_copy_translator_sll(source_var, target_var);
 *
 * RETURNS: 1 on error, zero on success
 *
 * DESCRIPTION: Traverses the singly linked list of source_var's TRANSLATOR's,
 * and creates an identical SLL (except for check_address's, of course) that
 * is attached to target_var.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	TRANSLATOR_PTR source_trans,
	               *target_trans;

	source_trans = (TRANSLATOR_PTR)source_var->info;
	target_trans = (TRANSLATOR_PTR *)(&target_var->info);
	while (source_trans)
	{
		assert(source_trans == source_trans->check_address);

		*target_trans = (TRANSLATOR_PTR)memMalloc(sizeof(TRANSLATOR), "TRANSLATOR");
		if (!*target_trans)
		{
			return(1);
		}

		memMemcpy((void *)*target_trans, (void *)source_trans, sizeof(TRANSLATOR), NO_TAG);
		if ( nt_copy_translator_ugvalue(source_trans->gtype, source_trans->gvalue,
		 &((*target_trans)->gvalue)) )
		{
			return(1);
		}
		if ( nt_copy_translator_ugvalue(source_trans->utype, source_trans->uvalue,
		 &((*target_trans)->uvalue)) )
		{
			return(1);
		}
		(*target_trans)->check_address = (void *)*target_trans;
				
		target_trans = &((*target_trans)->next);
		source_trans = source_trans->next;

	} /* while() on TRANSLATOR SLL */
	
	return(0);
}

#ifdef PROTO
static BOOLEAN nt_copy_translator_ugvalue(FF_TYPES_t value_type,
       void *source_value, void **target_value)

#else
static BOOLEAN nt_copy_translator_ugvalue(value_type, source_value,
       target_value)
FF_TYPES_t value_type;
void *source_value;
void *target_value;
#endif

/*****************************************************************************
 * NAME: nt_copy_translator_ugvalue()
 *
 * PURPOSE:  Copies source_value to target_value, depending on type
 *
 * USAGE: error = nt_copy_translator_ugvalue(source_trans->gtype,
 *                 source_trans->gvalue, target_trans->gvalue);
 * or:
 * error = nt_copy_translator_ugvalue(source_trans->utype,
 *          source_trans->uvalue, target_trans->uvalue);
 *
 * RETURNS: 1 on error, zero on success
 *
 * DESCRIPTION:  Allocates memory for target_value and copies source_value into
 * target_value, depending on value_type
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	*target_value = NULL;
	
	if (IS_TEXT_TYPE(value_type))
		*target_value = (char *)memStrdup((char *)source_value, "*target_value");
	else if (IS_INTEGER_TYPE(value_type) || IS_REAL_TYPE(value_type))
	{
		size_t byte_size = ffv_type_size(value_type);
		
		*target_value = memMalloc(byte_size, "*target_value");
	}
	else
		assert(0);

	if (*target_value == NULL)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "translator: FreeForm value");
		return(1);
	}

	if (!IS_TEXT_TYPE(value_type))
	{
		int error = 0;
		
		error = btype_to_btype(source_value, value_type, target_value, value_type);
		if (error)
			return(1);
		else
			return(0);
	}
	
	return(0);
}

/*
 * NAME:		nt_show
 *		
 * PURPOSE:	To show the NAME_TABLE	
 *
 * USAGE:	int nt_show(NAME_TABLE *, char *buffer)
 *
 * RETURNS:	if sucessful, return 0, otherwise err_no
 *
 * DESCRIPTION: convert the name table to ASCII.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	none
 *
 * GLOBAL:	variable_type[] (defined in freeform.h)
 *
 * AUTHOR:	Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	dictionary, name table.
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_show"

int nt_show(NAME_TABLE *table, char *buffer)
{
	char *ch = buffer;
	
	assert(table);
	assert(table == table->check_address);
	assert(table->format);
	assert(table->format == table->format->check_address);

	if (table == NULL)
	{
		err_push(ROUTINE_NAME, ERR_PTR_DEF, "name table undefined");
		return(ERR_PTR_DEF);
	}

	if (buffer == NULL)
	{
		err_push(ROUTINE_NAME, ERR_PTR_DEF, "buffer undefined");
		return(ERR_PTR_DEF);
	}

	sprintf(ch, "begin name_equiv\n");
	ch += strlen(ch);
	
	nt_show_section(table, ch, FFV_EQUIV);
	ch += strlen(ch);
	sprintf(ch, "end name_equiv\n");
	ch += strlen(ch);

	sprintf(ch, "begin constant\n");
	ch += strlen(ch);
	
	nt_show_section(table, ch, FFV_CONSTANT);
	ch += strlen(ch);
	sprintf(ch, "end constant\n");

	return(0);
} 

void nt_show_section(NAME_TABLE_PTR table, char *ch, FF_TYPES_t sect_type)
/*****************************************************************************
 * NAME:  nt_show_section()
 *
 * PURPOSE:  Writes the name_equiv or constant block body into a text buffer
 *
 * USAGE:  nt_show_section(table, buffer, section_type);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Writes either the name_equiv block body or the constant block
 * body of table into a text buffer.  The variable sect_type should be either
 * FFV_EQUIV or FFV_CONSTANT, and only those variables' type fields matching
 * sect_type will have their contents written into the buffer.  The syntax of
 * the buffer is the same as an eqv file or section.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:  Currently does not support escaping percents
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/


{
	TRANSLATOR *trans;
	char tokens[MAX_TOKENS][MAX_PV_LENGTH + 1];
	VARIABLE_LIST_PTR v_list = NULL;

	v_list = dll_first(table->format->variables);
	while (FFV_VARIABLE(v_list))
	{
		assert(FFV_VARIABLE(v_list) == FFV_VARIABLE(v_list)->check_address);
		
		if ((FFV_VARIABLE(v_list)->type & sect_type) == FFV_CONSTANT)
		{
			strcpy(tokens[0], FFV_VARIABLE(v_list)->name);
			strcpy(tokens[1], ff_lookup_string(variable_types,
			 FFV_DATA_TYPE(FFV_VARIABLE(v_list))));
			(void)ff_binary_to_string(table->data->buffer +
			 FFV_VARIABLE(v_list)->start_pos - 1,
			  FFV_DATA_TYPE(FFV_VARIABLE(v_list)), tokens[2],
			   FF_VAR_LENGTH(FFV_VARIABLE(v_list)));
			
			os_str_replace_char(tokens[0], ' ', '%');
			os_str_replace_char(tokens[2], ' ', '%');
			sprintf(ch, "\t%s %s %s\n", tokens[0], tokens[1], tokens[2]);
/*			os_str_set_unescaping_char1(ch, '%');*/
			ch += strlen(ch);
		}
		else if ((FFV_VARIABLE(v_list)->type & sect_type) == FFV_EQUIV)
		{
			strcpy(tokens[0], FFV_VARIABLE(v_list)->name);
			(void)ff_binary_to_string(table->data->buffer +
			                          FFV_VARIABLE(v_list)->start_pos - 1,
			                          FFV_DATA_TYPE(FFV_VARIABLE(v_list)),
			                          tokens[1],
			                          FF_VAR_LENGTH(FFV_VARIABLE(v_list)));

			os_str_replace_char(tokens[0], ' ', '%');
			os_str_replace_char(tokens[1], ' ', '%');
			sprintf(ch, "\t$%s %s\n", tokens[0], tokens[1]);
/*			os_str_set_unescaping_char1(ch, '%');*/
			ch += strlen(ch);
			if (IS_TRANSLATOR(FFV_VARIABLE(v_list)))
			{
				trans = (struct t_table *)FFV_VARIABLE(v_list)->info;
				while (trans)
				{
					assert(trans == trans->check_address);
					
					strcpy(tokens[0], ff_lookup_string(variable_types,
					 FFV_DATA_TYPE_TYPE(trans->gtype)));
					(void)ff_binary_to_string(trans->gvalue, FFV_DATA_TYPE_TYPE(trans->gtype),
					 tokens[1], 0);
					strcpy(tokens[2], ff_lookup_string(variable_types,
					       FFV_DATA_TYPE_TYPE(trans->utype)));
					(void)ff_binary_to_string(trans->uvalue, FFV_DATA_TYPE_TYPE(trans->utype),
					 tokens[3], 0);
					sprintf(ch, "\t\t%s %s %s %s\n", tokens[0], tokens[1], tokens[2],
					 tokens[3]);
					ch += strlen(ch);
					trans = trans->next;
				} /* end of while (trans) */
			} /* end of if() translator */
		} /* end of else if () name_equiv */
		
		v_list = dll_next(v_list);
	} /* while (v_list) */
}	

/*
 * NAME:  nt_str_to_binary
 *              
 * PURPOSE:  To convert a string to its binary representation, allocating space   
 *
 * USAGE:		void *nt_str_to_binary(char *buffer, int type)
 *
 * RETURNS:  If successfull, return a void pointer to the location where the
 * binary value is stored, otherwise, return NULL. 
 *
 * DESCRIPTION: Allocates memory for the return type, then calls ff_string_to_binary
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * ERRORS: Unknown variable type, NULL
 * Out of memory, "dest"
 *
 * GLOBALS:  none
 *
 * AUTHOR:  Liping Di, NGDC, lpd@ngdc.noaa.gov
 *
 * COMMENTS:  This is a local function to the name table module.
 *
 * KEYWORDS:    dictionary
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_str_to_binary"

#ifdef PROTO
/*static*/ void *nt_str_to_binary(char *buffer, FF_TYPES_t type)
#else
static void *nt_str_to_binary(buffer, type)
char *buffer;
FF_TYPES_t type;
#endif
{
	void *dest = NULL;

	if (!buffer)
	{
		err_push(ROUTINE_NAME,ERR_PTR_DEF, "Variable String");
		return(NULL);
	}
	 
	if (IS_TEXT_TYPE(type))
	{
		dest = (void *)memStrdup(buffer, "dest <-- buffer");
		return(dest);
	}
	else if (IS_INTEGER_TYPE(type) || IS_REAL_TYPE(type))
	{
		size_t byte_size = ffv_type_size(type);
		
		dest = (void *)memMalloc(byte_size, "dest");
	}
	else
		assert(0);

	if (!dest)
	{
		err_push(ROUTINE_NAME,ERR_MEM_LACK, "binary destination");
		return(NULL);
	}
	
	if (ff_string_to_binary(buffer, type, (char *)dest))
	{
		err_push(ROUTINE_NAME, ERR_CONVERT, "Name Table Binary to ASCII");
		return((void *)NULL);
	}
	return(dest);
}

static int parse_line_into_tokens_by_case(unsigned char status, char *line,
       char tokens[][MAX_PV_LENGTH + 1], int count_tokens_only)
/*****************************************************************************
 * NAME: parse_line_into_tokens()
 *
 * PURPOSE:  Parses a text line from an eqv into tokens
 *
 * USAGE:  num_tokens = parse_line_into_tokens(line, tokens);
 *
 * RETURNS:  Number of tokens parsed
 *
 * DESCRIPTION:  Copies the tokens of line, which are separated by whitespace,
 * into the array tokens.  The variable status must be either IN_CONSTANT_TABLE
 * or IN_EQUIV_TABLE -- any other value and this function returns zero and the
 * contents of the array tokens is undefined.
 *
 * In the case IN_CONSTANT_TABLE the first two tokens are copied into the
 * first two elements of the array tokens, respectively, and whatever follows
 * is copied into the third element -- this allows for the defined value in
 * the constant section to contain whitespace itself, e.g.,
 *
 * ff_array char array_name[1 to 2][1 to 80]
 *
 * In the case IN_EQUIV_TABLE either two or four tokens are expected, depending
 * on whether a name equivalence or a translator is being read.
 *
 * A line beginning with a forward slash ('/') indicates a comment line,
 * which is ignored (return with a value of zero; tokens is undefined).
 *
 * If count_tokens_only is non-zero, then the array tokens is not filled,
 * and token counting only is done.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

#define COMMENT_INDICATOR '/'

{
	char *ch_ptr = line;
	int num_tokens = 0, i;
	
	assert(ch_ptr);
	
	while (isspace(*ch_ptr))
		++ch_ptr;
	
	if (*ch_ptr == COMMENT_INDICATOR)
		return(0);

	switch(status)
	{
		case IN_CONSTANT_TABLE:
			while (*ch_ptr != STR_END && *ch_ptr != '\n')
			{
				if (isspace(*ch_ptr))
				{
					++ch_ptr;
				}
				else
				{
					if (num_tokens + 1 < NUM_CONSTANT_TOKENS)
					{
						i = 0;
						while (!isspace(*ch_ptr) && *ch_ptr != STR_END && *ch_ptr != '\n')
						{
							if (count_tokens_only != COUNT_TOKENS_ONLY)
								tokens[num_tokens][i < MAX_PV_LENGTH ? i++ : i] = *ch_ptr;
							++ch_ptr;
						}
						if (count_tokens_only != COUNT_TOKENS_ONLY)
							tokens[num_tokens][i] = STR_END;
					}
					else
					{
						if (count_tokens_only != COUNT_TOKENS_ONLY)
						{
							strncpy(tokens[NUM_CONSTANT_TOKENS - 1], ch_ptr, MAX_PV_LENGTH);
							tokens[NUM_CONSTANT_TOKENS - 1][MAX_PV_LENGTH] = STR_END;
						}
						ch_ptr += strlen(ch_ptr);
					}
					++num_tokens;
				}
			}

		break;

		case IN_EQUIV_TABLE:
			while (*ch_ptr != STR_END && *ch_ptr != '\n')
			{
				if (isspace(*ch_ptr))
				{
					++ch_ptr;
				}
				else
				{
					i = 0;
					while (!isspace(*ch_ptr) && *ch_ptr != STR_END && *ch_ptr != '\n')
					{
						if (count_tokens_only != COUNT_TOKENS_ONLY)
							tokens[num_tokens][i < MAX_PV_LENGTH ? i++ : i] = *ch_ptr;
						++ch_ptr;
					}
					if (count_tokens_only != COUNT_TOKENS_ONLY)
						tokens[num_tokens][i] = STR_END;
					++num_tokens;
				}
			}

		break;

		default:
		break;
	}

	if (count_tokens_only != COUNT_TOKENS_ONLY)
		for (i = 0; i < num_tokens; i++)
			(void)os_str_trim_whitespace(tokens[i], tokens[i]);
	return(num_tokens);
}

static void nt_free_trans(TRANSLATOR_PTR trans)

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_free_trans"

/*****************************************************************************
 * NAME: nt_free_trans()
 *
 * PURPOSE:  To free the SLL translator list
 *
 * USAGE:  nt_free_trans(var->info);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	TRANSLATOR_PTR next = NULL;

	assert(trans);
	
	while (trans)
	{
		next = trans->next;
		assert(trans);
		assert(trans == trans->check_address);

		if (trans->gvalue)
			memFree(trans->gvalue, "trans->gvalue");
		if (trans->uvalue)
			memFree(trans->uvalue, "trans->uvalue");
		memFree(trans, "trans");
   
   trans = next;
	}
}

int fd_get_format_data(FORMAT_DATA_LIST_PTR fd_list, FF_TYPES_t fd_type,
    FORMAT_DATA_HANDLE format_data)
/*****************************************************************************
 * NAME:  fd_get_format_data()
 *
 * PURPOSE:  Find first format data in a format data list that matches the given
 * format data type
 *
 * USAGE:  error = fd_get_format_data(name_table_list, name_table_type, &target_table);
 *
 * RETURNS:  Zero on success and target_table address is set to a format data
 * in format data list, or zero if format data cannot be found and format_data
 * is set to NULL, or an error code on error.
 *
 * DESCRIPTION:  Eligible bit masks include any combination of the following:
 * FFF_TABLE, FFF_DATA, FFF_HD, FFF_INPUT, and FFF_OUTPUT
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	if (!fd_list)
	{
		if (format_data)
			*format_data = NULL;
		return(ERR_UNKNOWN_FORMAT_TYPE);
	}		

	if (!(fd_type & FD_TYPES))
	{
		if (format_data)
			*format_data = NULL;
		return(ERR_UNKNOWN_FORMAT_TYPE);
	}
		
	fd_list = FD_FIRST(fd_list);
	while (FD_FORMAT_DATA(fd_list))
	{
		assert(FD_FORMAT_DATA(fd_list) == FD_FORMAT_DATA(fd_list)->check_address);
		assert(FD_FORMAT_DATA(fd_list)->format == FD_FORMAT_DATA(fd_list)->format->check_address);

		if ( (FD_TYPE(FD_FORMAT_DATA(fd_list)) & fd_type) == (fd_type & FD_TYPES) )
			break;
	
		fd_list = dll_next(fd_list);
	}

	if (format_data)
		*format_data = FD_FORMAT_DATA(fd_list);
	return(0);
}

int fd_put_format_data(FORMAT_DATA_LIST_HANDLE fd_list_hdl, FF_TYPES_t fd_type,
    FORMAT_DATA_PTR format_data)
/*****************************************************************************
 * NAME: fd_put_format_data()
 *
 * PURPOSE:  Put a format data into a list
 *
 * USAGE:  error = fd_put_format_data(&format_data_list, format_data_type,
 *                 format_data);
 *
 * RETURNS:  Zero on success, an error code on failure
 *
 * DESCRIPTION:  If the format data list does not exist, this function
 * creates it, hence the handle.  format data is inserted into the list,
 * and its format->type is set according to format data type.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "fd_put_format_data"
{
	FORMAT_DATA_LIST_PTR new_node;
	
	if (!*fd_list_hdl)
		*fd_list_hdl = dll_init();
	
	if (!*fd_list_hdl)
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "format data list"));
		
	format_data->format->type |= fd_type;
	
	new_node = dll_add(dll_last(*fd_list_hdl), 0);
	if (!new_node)
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "format data list"));

	dll_data(new_node) = format_data;
	
	return(0);
}

int fd_remove_format_data(FORMAT_DATA_LIST_HANDLE fd_list_hdl, FF_TYPES_t fd_type,
    char *f_name)
/*****************************************************************************
 * NAME: fd_remove_format_data()
 *
 * PURPOSE:  Removes a format data from a list
 *
 * USAGE:  error = fd_remove_format_data(&format_data_list, format_data_type,
 *                 format_name);
 *
 * RETURNS:  Zero on success, an error code on failure
 *
 * DESCRIPTION:  If the format data list becomes empty, this function
 * deletes it, hence the handle.  The format data with the matching
 * format_data_type and format_name is deleted from the list.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "fd_remove_format_data"
{
	FORMAT_DATA_LIST_PTR fd_list;
	
	if (!*fd_list_hdl)
		return(ERR_UNKNOWN_FORMAT_TYPE);
	
	fd_list = FD_FIRST(*fd_list_hdl);
	while (FD_FORMAT_DATA(fd_list))
	{
		assert(FD_FORMAT_DATA(fd_list) == FD_FORMAT_DATA(fd_list)->check_address);
		assert(FD_FORMAT_DATA(fd_list)->format == FD_FORMAT_DATA(fd_list)->format->check_address);

		if ( (FD_TYPE(FD_FORMAT_DATA(fd_list)) & fd_type) == (fd_type & FD_TYPES)
		     && !strcmp(f_name, FD_FORMAT_DATA(fd_list)->format->name) )
			break;
	
		fd_list = dll_next(fd_list);
	}

	if (FD_FORMAT_DATA(fd_list))
		dll_delete(fd_list, (void (*)(void *))fd_free_format_data);

	return(0);
}

/*
 * NAME:	fd_free_format_data
 *              
 * PURPOSE:	To free the FORMAT_DATA_PTR structure and associated memory   
 *
 * USAGE:	FORMAT_DATA_PTR fd_free_format_data(FORMAT_DATA_PTR)
 *
 * RETURNS:	NULL
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:	Ted Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    dictionary, name table.
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "fd_free_format_data"

#ifdef PROTO
void fd_free_format_data(FORMAT_DATA_PTR fd)
#else
void fd_free_format_data(fd)
FORMAT_DATA_PTR fd;
#endif
{
	if (!fd)
		return;
	
	assert(fd == fd->check_address);
	
	strncpy(fd->data->buffer, "This FreeForm Buffer has been freed", fd->data->total_bytes);
	ff_destroy_bufsize(fd->data);
	ff_free_format(fd->format);
	memFree(fd, "table");
}

void fd_free_format_data_list(FORMAT_DATA_LIST_PTR fd_list)
/*****************************************************************************
 * NAME:  fd_free_format_data_list()
 *
 * PURPOSE:  Frees a format data list
 *
 * USAGE:  error = fd_free_format_data_list(format_data_list);
 *
 * RETURNS:  Zero on success, an error code on failure
 *
 * DESCRIPTION:
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/
{
	if (!fd_list)
		return;
		
	dll_free(fd_list, (void (*)(void *))fd_free_format_data);
}

NAME_TABLE_PTR nt_get_name_table(NAME_TABLE_LIST_PTR pntl, FF_TYPES_t nt_type)
/*****************************************************************************
 * NAME:  nt_get_name_table()
 *
 * PURPOSE:  Get a name table from the databin according to nt_type
 *
 * USAGE:  table = nt_get_name_table(dbin->table, name_table_type);
 *
 * RETURNS:  a pointer to a name table or NULL
 *
 * DESCRIPTION:  name_table_type should be either FFF_INPUT or FFF_OUTPUT
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	NAME_TABLE_PTR table = NULL;
	
	(void)fd_get_format_data(pntl, nt_type | FFF_TABLE, &table);
	
	return(table);
}

int nt_put_name_table(NAME_TABLE_LIST_HANDLE hntl, FF_TYPES_t nt_type, NAME_TABLE_PTR table)
/*****************************************************************************
 * NAME:  nt_put_name_table()
 *
 * PURPOSE:  Put a name table into the databin according to nt_type
 *
 * USAGE:  error = nt_put_name_table(dbin->table, name_table_type, name_table);
 *
 * RETURNS:  Zero on success, or an error code on failure
 *
 * DESCRIPTION:  name_table_type should be either FFF_INPUT or FFF_OUTPUT
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	if (hntl)
	{
		NAME_TABLE_PTR nt_old;
		
		nt_old = nt_get_name_table(*hntl, nt_type);
		if (nt_old)
		{
			int error;
			
			error = nt_merge(table, &nt_old);
			if (error)
				return(error);
			else
				return(0);
		}
	}
	
	return(fd_put_format_data(hntl, nt_type | FFF_TABLE, table));
}

NAME_TABLE_PTR nt_remove_name_table(NAME_TABLE_LIST_HANDLE hntl, NAME_TABLE_PTR table)
/*****************************************************************************
 * NAME:  nt_remove_name_table()
 *
 * PURPOSE:  Removes from the data bin a name table with the given name table's
 * name and format->type
 *
 * USAGE:  table = nt_remove_name_table(&(dbin->table), table);
 *
 * RETURNS:  NULL
 *
 * DESCRIPTION:  Calls fd_find_format_data() and then fd_remove_format_data()
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	assert(table);
	assert(table == table->check_address);
	assert(table->format == table->format->check_address);
	
	(void)fd_remove_format_data(hntl, FD_TYPE(table),
	 table->format->name);
	
	return(NULL);
}

/*
 * NAME:	nt_free_name_table
 *              
 * PURPOSE:	To free the NAME_TABLE structure and associated memory   
 *
 * USAGE:	NAME_TABLE_PTR nt_free_name_table(NAME_TABLE_PTR)
 *
 * RETURNS:	NULL
 *
 * DESCRIPTION:  Calls fd_free_format_data()
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    dictionary, name table.
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "nt_free_name_table"

#ifdef PROTO
void nt_free_name_table(NAME_TABLE_PTR table)
#else
void nt_free_name_table(table)
NAME_TABLE_PTR table;
#endif
{
	fd_free_format_data(table);
}


