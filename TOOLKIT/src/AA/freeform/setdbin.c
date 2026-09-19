/*
 * FILENAME:  setdbin
 * CONTAINS:          db_set()
 *           (MODULE) ff_make_header_format()
 *           (MODULE) strascii()
 *           (MODULE) eval_eqv_title()
 *           (MODULE) do_change_input_img_format()
 *           (MODULE) dbs_search_menu_formats()
 *           (MODULE) dbs_set_menu_fmt_locus()
 *           (MODULE) dbs_set_menu_fmt_loci()
 *           (MODULE) ff_set_format_locus()
 *           (MODULE) ff_set_flist_loci()
 */

/*
 * NAME:        db_set
 *              
 * PURPOSE:     Set the attributes and execute the methods of data bins.
 *
 * USAGE:       db_set(DATA_BIN_PTR, attribute, [args], attribute, args,... NULL)
 *
 * RETURNS:     0 if all goes well.
 *
 * DESCRIPTION: This function provides access to the attributes and methods
 *				of the DATA_BINs. The function processes a list of argument
 *				groups which have the form: attribute [arguments]. The list
 *				of groups is terminated by a NULL, which ends processing.
 *				The presently supported attributes and their arguments are:
 *				case BUFFER:    * Argument: char *buffer *
 *				case BUFFER_IN: * Argument: char *buffer *
 *				case DBIN_BUFFER_SIZE: Argument: size_t buffer_size *
 *				case CHECK_FILE_SIZE:   * Argument: None *
 *				case DBIN_BUFFER_TO_INDEX:      * Argument: none *
 *				case DBIN_CACHE:                * Argument: char *cache, long cache_size *
 *				case DBIN_CACHE_SIZE:   * Argument: long cache_size *
 *				case DBIN_DATA_AVAILABLE:
 *				case DBIN_FILE_HANDLE:  * Argument: none *
 *        case DBIN_SET_TOTAL_RECORDS:  * Argument: long records_to_read *
 *				case DBIN_FILE_POSITION:       * Argument: long new_position *
 *				case DBIN_FILE_NAME:    * Argument: char *file_name *
 *				case DBIN_FLAGS:
 *				case DBIN_HEADER_FILE_NAME:     * Argument:  *
 *				case DBIN_OUTPUT_SIZE:  * Argument: none *
 *				case DBIN_SET_HEADER:
 *				case DBIN_SET_SCALE:
 *				case DBIN_SET_STRDB:    * Argument: DLL_NODE_PTR strdb *
 *				case DBIN_STD_FORMAT:
 *				case DEFINE_HEADER:     * Argument: char *header_format_file,  char * buffer] *
 *					case HEADER_NONE:
 *					case HEADER_EMBEDDED:
 *					case HEADER_EMBEDDED_VARIED:
 *					case HEADER_SEPARATED:
 *					case HEADER_SEPARATED_VARIED:
 *				case INPUT_FORMAT:      * Argument: char *file_name, char *buffer *
 *				case DBIN_INDEX:        * Argument: char *index_name *
 *				case FORMAT_LIST:       * Argument: char *file_name, char *buffer *
 *        case FORMAT_BUFFER:        * Argument: char *format_buffer, char *input_format_title, char *output_format_title *
 *				case MAKE_NAME_TABLE:   * Argument: char *file_name *
 *				case OUTPUT_FORMAT:
 *
 * SYSTEM DEPENDENT FUNCTIONS:  Processing of RETURN formats assumes EOL = 2 bytes
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:    Only one header format is checked for in a format list.
 *                              MEDIUM model:   check that allocations are allocating
 *                                                              the correct type(near or huge)
 *                                 
 * KEYWORDS: databins
 *
*/
/*
 * HISTORY:
 *	r fozzard	4/21/95		-rf01 
 *		(char *) for Think C
 *	r fozzard	7/28/95		-rf02
 *		comment out strerror(); conflicts with CW #define
 *		#include <stat.h> - CW needs this
*/


#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include <freeform.h> 

#include <os_utils.h>
#include <databin.h>
#include <dataview.h>

static int dbs_error_verbose_on = TRUE; /* not yet implemented, just a default */

#ifdef SUNCC
#define MEM_CHAR char
#else
#define MEM_CHAR unsigned char
#endif

/* module functions */

BOOLEAN is_a_variable_description_file(char *file_name)
{
	char *ext = os_path_return_ext(file_name);

	if (!ext)
		return(FALSE);

	return((BOOLEAN)((!strcmp(ext, "afm") ||
	                  !strcmp(ext, "bfm") ||
	                  !strcmp(ext, "dfm")
	                 )
	                 ? TRUE : FALSE
	                )
	      );
}

void ff_set_format_locus(const char *locus, FORMAT_PTR format)
{
	assert(format);
	
	if (format)
		strncpy(format->locus, locus, sizeof(format->locus) - 1);
}

void ff_set_flist_loci(const char *locus, FORMAT_LIST_PTR flist)
{
	assert(flist);
	
	if (flist)
	{
		flist = FFF_FIRST(flist);
		while (FFF_FORMAT_FMT(flist))
		{
			ff_set_format_locus(locus, FFF_FORMAT_FMT(flist));
			flist = FFF_NEXT(flist);
		}
	}
}

/*
 * NAME:	ff_make_header_format
 *
 * PURPOSE:	To create a header format.
 *
 * USAGE:	FORMAT_PTR ff_make_header_format(FORMAT_PTR form, FF_SCRATCH_BUFFER buffer)
 *
 * RETURNS:	A pointer to the header format, else NULL
 *
 * DESCRIPTION:	This function checks a format specification for a header.
 *				If it exists, the file given as the name of the FFV_HEADER
 *				variable is read to create a header format. If the file does
 *				not exist, a format with a single char variable with the
 *				length of the header is created.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * ERRORS:
 *		Pointer not defined,"format"
 *		Possible memory corruption, "format"
 *		Pointer not defined,"buffer"
 *		Out of memory,"Header Format"
 *		Out of memory,"Header Format Variable"
 *
 * COMMENTS:	
 *
 * KEYWORDS:	
 *
 */

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "ff_make_header_format"

FORMAT_PTR ff_make_header_format(FORMAT_PTR format, FF_SCRATCH_BUFFER buffer)
{
	VARIABLE_PTR		var 	= NULL;
	VARIABLE_LIST_PTR	v_list	= NULL;
	FORMAT_PTR		header_format = NULL;
	
	/* Error checking on NULL passed parameters */
	assert(format && buffer &&
		(format == format->check_address) );

	v_list = FFV_FIRST_VARIABLE(format);

	while(dll_data(v_list))
	{
		if (IS_HEADER(FFV_VARIABLE(v_list)))
			break;
		v_list = dll_next(v_list);
	}

	if (dll_data(v_list))
	{
		int error;
		
		FORMAT_LIST_PTR f_list = NULL;
		FF_BUFSIZE bufsize;
		PP_OBJECT pp_object;
		
		bufsize.buffer = buffer;
		bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;

		error = ff_file_to_bufsize(FFV_VARIABLE(v_list)->name, &bufsize);
		if (error)
		{
			err_push(ROUTINE_NAME, error, FFV_VARIABLE(v_list)->name);
			return(NULL);
		}

		pp_object.ppo_type = PPO_FORMAT_LIST;
		pp_object.ppo_object.hf_list = &f_list;

		if (ff_text_pre_parser(os_path_return_ext(FFV_VARIABLE(v_list)->name),
		                       &bufsize,
		                       &pp_object))
		{
		}
		else
		{
			header_format = db_find_format(f_list, FFF_NAME, FORMAT_NAME_INIT, END_ARGS);
		}

		if (header_format)
		{	
			if (IS_BINARY(header_format))
				header_format->type = FFF_BINARY_FILE_HD;
			else
				header_format->type = FFF_ASCII_FILE_HD;
		}
		else
		{
			/* An error occurred creating the header format, presumably
			the file does not exist. Create a format with a single variable
			with the length of the header */

			header_format->num_in_list = 1;

			if (IS_BINARY(format))
				header_format->type = FFF_BINARY_FILE_HD;
			else
				header_format->type = FFF_ASCII_FILE_HD;
			 
			header_format->max_length = FFV_VARIABLE(v_list)->end_pos;

			/* Create variable for header format */
			if ((var = (VARIABLE_PTR)memCalloc(1, sizeof(VARIABLE), "Creating Variable")) == NULL) {
				err_push(ROUTINE_NAME,ERR_MEM_LACK,"Header Format Variable");
				return(NULL);
			}
			var->check_address = (void*)var;
			var->start_pos	= 1;
			var->end_pos	= header_format->max_length;
			var->type		= FFV_TEXT;
			header_format->variables = dll_init();
			v_list = dll_add(dll_last(header_format->variables), 0);
			v_list->data_ptr = (void *)var;
		}

		return(header_format);
	}
	return(NULL);
}

/*
 * NAME:        strascii        
 *              
 * PURPOSE:     To convert non-printable character to it's ASCII value. Only used in setdbin
 *
 * USAGE:       int strascii(char *ch);
 *                      *ch: pointer to the non-printable character 
 *
 * RETURNS:     ASCII code of the character
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * GLOBALS:     
 *
 * AUTHOR:      Liping Di 303-497-6284  lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */

static char strascii(char *ch)
{
	if (*ch != '\\')
		return(*ch);

	switch(*++ch)
	{
		case 'n':
			return('\n');
	
		case 't':
			return('\t');
	
		case '0':
			return(STR_END);
			
		case 'r':
			return('\r');
	
		default:
			return(*ch);
	}
}       

/* GeoVu-specific module functions */
#ifdef XVT
#include <xvt.h>
#include <data_par.h>
#include <geodata.h> /* for CD_MENU_PTR, Source_CD */
extern void change_input_img_format(DATA_BIN_PTR dbin);

static void dbs_set_menu_fmt_locus(const char *menu_name, const char *sect_title, FORMAT_PTR format)
{
	char locus[SZ_FNAME];
	
	assert(format);
	assert(sizeof(locus) > strlen(menu_name) + 1 + strlen(sect_title) + 3);
	
	if (format)
	{
		strncpy(locus, menu_name, sizeof(locus) - 1);
		locus[sizeof(locus) - 1] = STR_END;
		strncat(locus, " (", sizeof(locus) - strlen(locus) - 1);
		strncat(locus, sect_title, sizeof(locus) - strlen(locus) - 1);
		strncat(locus, ")", sizeof(locus) - strlen(locus) - 1);
	
		strncpy(format->locus, locus, sizeof(format->locus) - 1);
		format->locus[sizeof(format->locus) - 1] = STR_END;
	}
}

static void dbs_set_menu_fmt_loci(const char *menu_name, const char *sect_title, FORMAT_LIST_PTR flist)
{
	assert(flist);
	
	if (flist)
	{
		flist = FFF_FIRST(flist);
		while (FFF_FORMAT_FMT(flist))
		{
			dbs_set_menu_fmt_locus(menu_name, sect_title, FFF_FORMAT_FMT(flist));
			flist = FFF_NEXT(flist);
		}
	}
}

/*
 * NAME:        eval_eqv_title
 *              
 * PURPOSE:     To determine whether or not the equivalence section name belongs to
 *                      the file.
 *
 * USAGE:       BOOLEAN eval_eqv_title(char *sect, char *fname)
 *
 * RETURNS:     TRUE if the sect is a eqv table and belongs to the file.
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.gov
 *
 * COMMENTS:    (kbf, 6/26/95) This function used to be nt_eval_eqv, but seemed
 *				out of place there.
 *
 * KEYWORDS:    name table, menu system.
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "eval_eqv_title"
static BOOLEAN eval_eqv_title(char *sect, char *fnm)
{
	BOOLEAN result=FALSE, current;
	char temp[MAX_PV_LENGTH];
	char *ch;
	int i,j, last;
	
	if(sect == NULL)return(FALSE);
	if(fnm == NULL)return(FALSE);

	if(*sect != '#')return(FALSE);

	if(strlen(sect) > 80)return(FALSE);
	strcpy(temp, sect+1);

	if((ch=strstr(temp, "_eqv")) == NULL)
		return(FALSE);
	*ch=STR_END;
	ch=temp;
	i=0;
	last=2; 

	while(1){
		j=0;
		if(temp[i] == '&')j=1;
		if(temp[i] == '|')j=2;
		if(temp[i] == STR_END)j=3;

		if(j > 0){
			temp[i]=STR_END;
			if(strlen(ch) > 0){

				if(memStrstr(fnm, ch,"fnm,ch"))
					current=TRUE;
				else
					current=FALSE;
	
				if(last == 1)
					result=result & current;
				else
					result=result | current;

				ch=&temp[i+1];
			}
			last=j;
		}

		if(j == 3)break;
		i++;
	}
	return(result);
}

static int do_change_input_img_format(DATA_BIN_PTR dbin)
/*****************************************************************************
 * NAME: do_change_input_img_format()
 *
 * PURPOSE:  Change the input format according to data_representation and
 * bytes_per_pixel, if data file is an image file.
 *
 * USAGE:  error = do_change_input_img_format(dbin);
 *
 * RETURNS: Zero on success, ERR_MEM_LACK on failure
 *
 * DESCRIPTION:  Checks value
 * of GeoVu keyword "data_type" for a substring containing "image", "raster",
 * or "grid".  Initializes dbin->data_parameters to an
 * IMAGE structure (remembering the previous pointer, to be restored later),
 * and calls change_input_img_format().
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:  I can't distinguish between newform/checkvar calling this event,
 * or GeoVu, so there is redundancy when this function gets called again in
 * set_image_limit().  However, this call cannot be removed from set_image_limit(),
 * as bytes_per_pixel get set in other ways by that calling code.  This also
 * precludes newform() and checkvar() running on .pcx files.  This can be fixed
 * by making this function more intelligent (set_image_limit() less?).
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "do_change_input_img_format"
{						
	void *point;
	char temp[MAX_PV_LENGTH];

	if (!IS_BINARY(dbin->input_format))
		return(0);
	
	if (!os_strcmpi(os_path_return_ext(dbin->file_name), "pcx"))
		return(0);
	if (!os_strcmpi(os_path_return_ext(dbin->file_name), "bmp"))
		return(0);

	if (nt_askvalue(dbin, "data_type", FFV_CHAR, temp, NULL) && 
	    (strstr(temp, "image") || strstr(temp, "raster") ||
	     strstr(temp, "grid")))
	{
		point = dbin->data_parameters;
				
		if ((dbin->data_parameters = (void *)memMalloc(sizeof(IMAGE), "dbin->data_parameters")) == NULL)
			return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Unable to initialize image information"));
		
		memset(dbin->data_parameters, '\xcc', sizeof(IMAGE));
				
		change_input_img_format(dbin);
				
		memFree(dbin->data_parameters, "dbin->data_parameters");
		dbin->data_parameters = point;
	}
	return(0);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "dbs_search_menu_formats"

/*****************************************************************************
 * NAME: dbs_search_menu_formats()
 *
 * PURPOSE:  Search a GeoVu menu for formats
 *
 * USAGE:  found = dbs_search_menu_formats(fff_type, dbin, dbin->strdb, dbin->buffer, &hflist);
 *
 * RETURNS: Zero on success, else an error code
 *
 * DESCRIPTION:  Check if "ff_header_format", "ff_input_format", or
 * "ff_output_format", according to fff_type, is defined.  If not defined,
 * check if GeoVu keyword "ff_format_fmt" is defined.  If one of the above
 * keywords is defined, create a format list from the menu section or format
 * file so named.  If this generates an error, in which case no format list
 * is generated, the user is alerted. 
 *
 * If either "ff_format_fmt" or "ff_{header, input, output}_format" is defined then
 * TRUE is returned, regardless of whether a format list was successfully
 * generated or not.  This has the effect of preventing a default format file
 * search (which might succeed) when clearly the intention is for the format
 * to be taken from the menu file.
 *
 * If the format is contained in a menu, then the menu file name and the
 * format section title are copied into the format locus.  This assumes
 * that the external variable Source_CD exists and has been properly defined.
 * If an external format file is referenced within a menu file, then only that
 * format file (and not the menu file name) are copied into the format locus.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:  Source_CD
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

static int dbs_search_menu_formats(FF_TYPES_t fff_type, DATA_BIN_PTR dbin, MENU_INDEX_PTR mindex, FF_SCRATCH_BUFFER buffer, FORMAT_LIST_HANDLE hflist)
{
	char          *sect_text = NULL;
	int            error;
	char           gv_keyword[17] = {"ff_output_format"};
	char           gv_key_def[MAX_PV_LENGTH];
	FORMAT_PTR     format = NULL;
	DLL_NODE_PTR   section = NULL;
	ROW_SIZES  rowsize;

	char *file_name = NULL;

	assert(dbin);
	assert(dbin == dbin->check_address);
	assert(buffer);
	assert(hflist);

	if (!mindex)
		return(ERR_MN_SEC_NFOUND);

	if (!dbin || !buffer || !hflist)
		return(ERR_PTR_DEF);

	switch (fff_type & (FFF_INPUT | FFF_OUTPUT | FFF_HD))
	{
		case FFF_INPUT:
			strcpy(gv_keyword, "ff_input_format");
		break;
		
		case FFF_OUTPUT:
			strcpy(gv_keyword, "ff_output_format");
		break;
		
		case FFF_HD:
			strcpy(gv_keyword, "ff_header_format");
		break;
		
		default:
			sprintf(buffer, "%s, %s:%d",
			        ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, buffer);
			err_disp();
			return(ERR_SWITCH_DEFAULT);
		break;
	}
	
	 error = nt_askvalue(dbin,
	                     gv_keyword,
	                     FFV_CHAR | FFF_INPUT,
	                     gv_key_def, NULL
	                    ) ? 0 : ERR_GENERAL;
		
	if (error)
	{
		 error = nt_askvalue(dbin,
		                     "ff_format_fmt",
		                     FFV_CHAR | FFF_INPUT,
		                     gv_key_def, NULL
		                    ) ? 0 : ERR_GENERAL;
	}
	if (!error)
  {
		if (gv_key_def[0] == '*')
		{
			sect_text = NULL;
			
			if (mn_index_get_offset(mindex, gv_key_def, &rowsize))
			{
				err_push(ROUTINE_NAME, ERR_MENU, "Retrieving section");
				return(ERR_MENU);
			}
			
			/* Special case to include the title in the section */
			rowsize.num_bytes *= -1;
			
			if (mn_section_get(mindex, NULL, &rowsize, &sect_text))
			{
				err_push(ROUTINE_NAME, ERR_MENU, "Retrieving section");
				return(ERR_MENU);
			}

			file_name = NULL;
		}
		else
		{
			if (os_path_prepend_special(gv_key_def, cd_device, buffer))
			{ /* the format file is located in the file system */
				strcpy(gv_key_def, buffer);
				file_name = gv_key_def;
			}
		}
		
		if (sect_text || file_name)
		{
			FF_BUFSIZE bufsize;
			PP_OBJECT pp_object;
			
			bufsize.buffer = sect_text ? sect_text : buffer;
			
			if (file_name)
			{
				bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;

				error = ff_file_to_bufsize(file_name, &bufsize);
				if (error)
					return(err_push(ROUTINE_NAME, error, file_name));
			}
			else
			{
				bufsize.total_bytes = strlen(buffer) + 1;
				bufsize.bytes_used = bufsize.total_bytes;
			}

			pp_object.ppo_type = PPO_FORMAT_LIST;
			pp_object.ppo_object.hf_list = hflist;

			error = ff_text_pre_parser(file_name ? os_path_return_ext(file_name) :
			                                       NULL,
			                           &bufsize,
			                           &pp_object);
			if (!error)
			{
				format = db_find_format(*hflist, FFF_NAME, FORMAT_NAME_INIT, END_ARGS);
				if (format && file_name)
				{
					if (is_a_variable_description_file(file_name))
						format->type |= fff_type;
				}
				else if (format)
							format->type |= fff_type;
			
				if (*hflist)
				{
					if (file_name)
						ff_set_flist_loci(gv_key_def, *hflist);
					else
						dbs_set_menu_fmt_loci(mindex ? mindex->menu_file : "",
						                      gv_key_def,
						                      *hflist);
				}
			} /* if !error */

			if (sect_text)
				free(sect_text);
		} /* if section text or file name */
	} /* if !error */

	if (error && !*hflist)
	{
		if (err_state())
		{
			if (dbs_error_verbose_on)
			{
				xvt_dm_post_error("Cannot create format\n\nThe %s %s referenced in menu file %s is in error"
				                  , gv_key_def[0] == '*' ? "section" 
				                                         : "file"
				                  , gv_key_def
								  , mindex ? mindex->menu_file : "<unknown>"
				                 );
				err_disp();
			}
			else
				err_clear();
		}
		
		return(ERR_MAKE_FORM);
	}

	return(error);
}

#endif /* XVT */

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "make_format_eqv_list"

#define dbs_IS_DATA(fff_type) (fff_type & FFF_DATA)
#define dbs_IS_INPUT_FORMAT(fff_type) (fff_type & FFF_INPUT)

static int make_format_eqv_list
	(
	 FF_TYPES_t fff_type,
	 char *fmt_fname,
	 char *format_buffer,
	 FORMAT_LIST_HANDLE hf_list,
	 NAME_TABLE_LIST_HANDLE hnt_list,
	 DATA_BIN_PTR dbin
	)
{
	int number;
	char **found_files;
	int error;
			
	
	*hf_list = NULL;
	number = 0; /* to hold return value of db_find_format_file() */

	if (!fmt_fname && !format_buffer)
	{
		if (dbin->file_name == NULL)
			return(err_push(ROUTINE_NAME, ERR_FILE_DEFINED, "databin file name -- Cannot default format search"));

#ifdef XVT /* only search GeoVu menu if this is an XVT app */

		/* create input format from GeoVu menu */
		if (dbs_search_menu_formats(fff_type,
		                            dbin,
		                            dbin->mindex,
		                            dbin->buffer,
		                            hf_list
		                           )
		    )

#endif /* XVT */
		{
			number = db_find_format_files(dbin, dbin->file_name, &found_files);
			if (number)
      {
      	if (dbs_IS_INPUT_FORMAT(fff_type))
      		fmt_fname = found_files[0];
      	else
      	{
	      	if (strcmp(os_path_return_ext(found_files[0]), "fmt") == 0)
	      		fmt_fname = found_files[0];
	      	else if (number == 2)
	      		fmt_fname = found_files[1];
	      	else
	      	{
	      		/* No output variable description file --
	      		   not necessarily an error.
	      		*/
	      	}
	      }
      }
    }
	} /* if no file name and no buffer */
			
	if (fmt_fname || format_buffer)
	{
		FF_BUFSIZE bufsize;
		PP_OBJECT pp_object;
				
		if (fmt_fname)
		{
			bufsize.buffer = dbin->buffer;
			bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
			
			error = ff_file_to_bufsize(fmt_fname, &bufsize);
			if (error)
				return(err_push(ROUTINE_NAME, error, fmt_fname));
		}
		else
		{
			bufsize.buffer = format_buffer;
			bufsize.total_bytes = strlen(format_buffer) + 1;
			bufsize.bytes_used = bufsize.total_bytes;
		}

		pp_object.ppo_type = PPO_FORMAT_LIST;
		pp_object.ppo_object.hf_list = hf_list;
		
		if (!ff_text_pre_parser(fmt_fname ? os_path_return_ext(fmt_fname) :
		                                    NULL,
		                        &bufsize,
		                        &pp_object))
		{
			if (fmt_fname)
			{
				ff_set_flist_loci(fmt_fname, *hf_list);
						
				/* Must manually add OUTPUT type if format was generated from a
				   variable description file
				*/
				if (is_a_variable_description_file(fmt_fname))
				{
					FORMAT_PTR format;
					
					format = db_find_format(*hf_list,
					                        FFF_NAME,
					                        FORMAT_NAME_INIT,
					                        END_ARGS
					                       );
					if (format)
						format->type |= fff_type;
				} /* if file is a variable description file */
			} /* if fmt_fname */
		} /* (else) if ff_text_pre_parser... */
		
		if (dbs_IS_DATA(fff_type))
		{
			pp_object.ppo_type = PPO_NT_LIST;
			pp_object.ppo_object.nt_list.nt_io_type = fff_type;
			pp_object.ppo_object.nt_list.hnt_list = hnt_list;
			
			(void)ff_text_pre_parser(NULL, &bufsize, &pp_object);
		}

		if (number)
		{
			int i;
			
			for (i = 0; i < number; i++)
				memFree(found_files[i], "found_files[i]");
			memFree(found_files, "found_files");
		}
	}
			
	if (!*hf_list)
		return(ERR_MAKE_FORM);
	else
		return(0);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_set"

int db_set(DATA_BIN_PTR dbin, ...)
{
	va_list args;

	SCRATCH_BUFFER  ch_ptr                  = NULL;
	SCRATCH_BUFFER  format_buffer           = NULL;
	char            *file_name              = NULL;
	char            *point                  = NULL;
	char            *ch                     = NULL;
	char            temp[MAX_PV_LENGTH];
	char            *env_dir             = NULL;
	int delim_item, delim_value, i;

/* -rf02	char	*strerror();	not used, and conflicts with CW #define -rf02 */

	char	error_buf[MAX_ERRSTR_BUFFER];
	char	*title_specified;

	unsigned	num_in_records  = 0;
	unsigned	bytes_to_read   = 0;
	unsigned	bytes_read		= 0;


	long	l_size	= 0L;
	long	total_bytes	= 0L;
	long	file_size	= 0L;

	int	dflt	= 0;
	int	error	= 0;
	int	attribute	= 0;
	int	number = 0;
	int	file = -1;
	
	FORMAT_PTR        format = NULL,
	           header_format = NULL;

	FORMAT_LIST_PTR f_list	= NULL;

	VARIABLE_PTR    var             = NULL;
	VARIABLE_PTR    var_scale       = NULL;
	VARIABLE_LIST_PTR v_list        = NULL;

	double inmin, inmax, outmin, outmax, factor;
	double *double_ptr;
    
	NAME_TABLE_PTR new_table	= NULL;
	NAME_TABLE_PTR old_table	= NULL;
	
	FF_TYPES_t format_type;
	
	char **found_files;
	FF_BUFSIZE generic_bufsize;
	PP_OBJECT pp_object;
				
	

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set NO_MESSAGE"

	assert(dbin && ((void *)dbin == dbin->check_address));
	
	format_buffer = NULL;
	ch_ptr = dbin->buffer;

	va_start(args, dbin);

	while ( (attribute = va_arg (args, int)) != END_ARGS )
	{
		/* Check For Possible Memory Corruption */
		if (dbin != dbin->check_address)
			return(err_push(MESSAGE_NAME, ERR_MEM_CORRUPT, "dbin"));

		if (!dbin)
			return(err_push(ROUTINE_NAME, ERR_BIN_NOT_DEFINED, ""));

		switch (attribute) {
		
		/*
		 * MESSAGE:     BUFFER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET) char *buffer
		 *
		 * PRECONDITIONS:       buffer must be allocated externally.
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: This message is used to set the scratch space for
		 *				the databin. The scratch space is allocated by the
		 *				application. 
		 *				Sets dbin->buffer to argument and initializes
		 *				first character to STR_END.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    This call may become vestigial when the memory
		 *				management moves into the bin library.
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set BUFFER"
		
		case BUFFER:    /* Argument: char *buffer */
			ch_ptr = dbin->buffer = va_arg (args, char *);
			*(ch_ptr) = STR_END;
			break;

		/*
		 * MESSAGE:     BUFFER_IN
		 *
		 * OBJECT TYPE:	DATA_BIN
		 *
		 * ARGUMENTS:	(TO SET)char *buffer
		 *
		 * PRECONDITIONS:
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: This message is used to get a buffer into a databin
		 *				to use as temporary input for some subsequent
		 *				message. A pointer is set to the argument and no
		 *				changes are made to the contents of the buffer (the
		 *				message does not initialize first character to STR_END).
		 *				This message does not change the location of the
		 *				databin buffer.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set BUFFER_IN"
		
		case BUFFER_IN: /* Argument: char *buffer */
			ch_ptr = va_arg (args, char *);
			break;

		/*
		 * MESSAGE:     DBIN_BUFFER_SIZE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET) size_t buffer_size
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:    Allocates (or reallocates) memory for buffer
		 *						Sets dbin->buffer
		 *                
		 *
		 * DESCRIPTION: This message creates a buffer for dbin with a given
		 *				size and sets a pointer to the beginning of the buffer.
		 *
		 * ERRORS:      Lack of Memory
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set BUFFER_SIZE"
		
		case DBIN_BUFFER_SIZE:  /* Argument: size_t buffer_size */

			bytes_to_read = va_arg (args, size_t);

			/* ptr to realloc must be malloced at least once  */
			if(dbin->buffer)        
				dbin->buffer = (FF_SCRATCH_BUFFER)memRealloc((void *)dbin->buffer, bytes_to_read, "dbin->buffer");
			else 
				dbin->buffer = (FF_SCRATCH_BUFFER)memMalloc(bytes_to_read, "dbin->buffer");

			if (!dbin->buffer)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "Data Bin Buffer"));

			ch_ptr = dbin->buffer;

			break;

		/*
		 * MESSAGE:     CHECK_FILE_SIZE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:	File must be opened.
		 *					Input Format must be defined.
		 *					Format list must be defined.
		 *					File must stat() without error.
		 *
		 * CHANGES IN STATE:	None
		 *
		 * DESCRIPTION: Checks to make sure file size is consistent with
		 *				the format length. This check includes file_headers
		 *				(or input_file_headers) but it can not be done if
		 *				the file includes record headers.
		 *
		 * ERRORS:	File not defined, dbin->title
		 *			Dbin format not defined,"Input Format"
		 *			Undefined Format List, dbin->title
		 *			System File Error, dbin->file_name
		 *			Dbin format not defined, "Finding dbin->header_format"
		 *			Dbin format not defined, "Finding dbin->input_format"
		 *			File length / Record Length Mismatch,"Input File"
		 *
		 * AUTHOR:	TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set CHECK_FILE_SIZE"
		
		case CHECK_FILE_SIZE:   /* Argument: None */

			if(!(dbin->state.open_file || dbin->state.std_input))
			{ /* ERROR: NO FILE DEFINED FOR BIN */
				va_end(args);
				return(err_push(MESSAGE_NAME, ERR_FILE_DEFINED, dbin->title));
			}
			if(dbin->input_format == NULL){
				va_end(args);
				return(err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED,"Input Format"));
			}

			if(dbin->format_list == NULL){
				va_end(args);
				return(err_push(MESSAGE_NAME, ERR_FORMAT_LIST_DEF,dbin->title));
			}

			total_bytes = os_filelength(dbin->file_name);
			if (total_bytes == -1)
				return(err_push(MESSAGE_NAME, errno, dbin->file_name));
			
			/* Record headers in the same file make it impossible to predict
			the file length. Check for a record header (or an input_record_header)
			and exit if it is not separate */
			format = db_find_format(dbin->format_list, FFF_GROUP, FFF_REC | FFF_HD, END_ARGS);
			if(IS_OUTPUT(format))
				format = db_find_format(dbin->format_list, FFF_GROUP, FFF_INPUT | FFF_REC | FFF_HD, END_ARGS);
			if(format && !(IS_SEPARATE(format)))
				break;

			/* Check for an input file header format */
			format = db_find_format(dbin->format_list, FFF_GROUP, FFF_FILE | FFF_HD, END_ARGS);
			if(IS_OUTPUT(format))
				format = db_find_format(dbin->format_list, FFF_GROUP, FFF_INPUT | FFF_FILE | FFF_HD, END_ARGS);
			if(format && !(IS_SEPARATE(format))){
				total_bytes -= FORMAT_LENGTH(format);

				/* the macro FORMAT_LENGTH adds EOL_SPACE to ascii headers 
				but headers in a binary file do not have an EOL at the end */
				if (IS_ASCII(format) && IS_BINARY(dbin->input_format)){
					total_bytes += EOL_SPACE;
				}
			}/* if header defined */

			format = db_find_format(dbin->format_list, FFF_GROUP, FFF_INPUT | FFF_DATA, END_ARGS);
			if (!format)
				return(err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED, "Finding dbin->input_format"));

			/* The specification for dbase files (.dbf) includes an end of file character
			1AH which seems to be optional, check for its existence */
			if(IS_DBASE(format) && memStrstr(dbin->file_name, ".dbf", "dbin->file_name,\".dbf\"")){
				l_size = lseek(dbin->data_file, 0L, SEEK_CUR);
				lseek(dbin->data_file,-1L, SEEK_END);
				read(dbin->data_file, temp, sizeof(char));
				if(temp[0] == '\032')
					total_bytes--;
				lseek(dbin->data_file,l_size, SEEK_SET);  
			}

			if (total_bytes % (long)(FORMAT_LENGTH(format)))
			{
				va_end(args);
				return(err_push(MESSAGE_NAME, ERR_FILE_LENGTH,"Input File"));
			}

			break;

		/*
		 * MESSAGE:     DBIN_BUFFER_TO_INDEX
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)None
		 *
		 * PRECONDITIONS:       dbin->buffer must be defined.
		 *
		 * CHANGES IN STATE:    Makes Index
		 *
		 * DESCRIPTION: Makes an index for the dbin from dbin->buffer.
		 *
		 * ERRORS:      Failure to make index
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    This can not work at present because db_make_index
		 *				does not accept a NULL file name.
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set BUFFER_TO_INDEX"

		case DBIN_BUFFER_TO_INDEX:      /* Argument: none */

			if(dbin->index)
				db_free_index(dbin->index); 
			
			/* the line above used to be the one below (and it was commented out):	
			  dbin->input_format = free_index(dbin->input_format) */

			/* this call is doomed to fail, because db_make_index returns with an
			   error on a NULL file name, as below */
			dbin->index = db_make_index(NULL, ch_ptr, 0, 0L);

			if(dbin->index == NULL)
				return(err_push(MESSAGE_NAME, ERR_MAKE_INDEX, "case DBIN_BUFFER_TO_INDEX"));

			ch_ptr += strlen(ch_ptr);

			break;

		/*
		 * MESSAGE:     DBIN_CACHE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET) DATA_BUFFER cache, long cache_size
		 *
		 * PRECONDITIONS:       The memory block must be allocated and its
		 *						size must be known.
		 *
		 * CHANGES IN STATE:    Sets dbin->cache
		 *						= dbin->cache_end
		 *						= dbin->data_ptr
		 *						Sets dbin->cache_size
		 *						Sets dbin->state.cache_defined = 1
		 *						Initialize first character of cache to STR_END
		 *                
		 *
		 * DESCRIPTION:  Set dbin->cache to a pre-existing block of memory
		 *
		 * ERRORS:		None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    Should cache_size be adjusted to multiple of input record?
		 *				Should dbin->records_in_cache be set?
		 *				Why initialize the first character of the cache?
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_CACHE"

		case DBIN_CACHE:	/* Argument: char *cache, long cache_size */
			dbin->data_ptr = dbin->cache_end = dbin->cache = va_arg (args, DATA_BUFFER);
			dbin->cache_size = va_arg (args, long);
			*(dbin->cache) = STR_END;
			dbin->state.cache_defined = 1;
			break;

		/*
		 * MESSAGE:     DBIN_CACHE_SIZE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET) long cache_size
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:	ALLOCATES MEMORY FOR CACHE
		 *						Sets dbin->cache
		 *						= dbin->cache_end
		 *						= dbin->data_ptr
		 *						Sets dbin->bytes_to_read to record length
		 *						Sets dbin->cache_size
		 *						Sets dbin->records_in_cache 
		 *						Sets dbin->state.cache_defined = 1
		 *						Initialize first character of cache to STR_END
		 *
		 * DESCRIPTION: Create cache for dbin with a given size.
		 *				If an input or output format is defined, the cache size
		 *				is adjusted to be the largest integral multiple
		 *				of the record size of the larger of input or output format.
		 *
		 * ERRORS:	Lack of Memory:
		 *				Sets dbin->state.cache_defined = 0 
		 *				Sets dbin->state.cache_filled  = 0 
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:	Why initialize first cache character to STR_END?
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_CACHE_SIZE"  

		case DBIN_CACHE_SIZE:   /* Argument: long cache_size */
			dbin->cache_size = va_arg (args, long);
			
			/* MAO:d downsize cache_size if necessary */
			dbin->cache_size = min(dbin->cache_size, UINT_MAX);

			/* Adjust cache size to be an even multiple of the longer record */
			if(dbin->input_format || dbin->output_format){
				dbin->bytes_to_read = max(FORMAT_LENGTH(dbin->input_format),
						FORMAT_LENGTH(dbin->output_format));
				dbin->records_in_cache = dbin->cache_size/dbin->bytes_to_read;
				dbin->cache_size = dbin->records_in_cache * dbin->bytes_to_read;
			}

			/* MAO:c dbin->records_in_cache is now cache size in units of records
			   (the larger of input or output formats) */

			/* ptr to realloc must be malloced at least once  */
			if (dbin->cache)
			{
				dbin->data_ptr = (char *)memRealloc((void *)dbin->cache, (size_t)dbin->cache_size, "DBIN_CACHE_SIZE: HUGE dbin->cache");
				if (!dbin->data_ptr)
					return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "reallocation of data cache"));
				else
					dbin->cache = dbin->cache_end = dbin->data_ptr;
			}
			else 
				dbin->data_ptr =
				dbin->cache_end =
				dbin->cache = (char *)memMalloc((size_t)dbin->cache_size, "DBIN_CACHE_SIZE: HUGE dbin->cache");

			/* MAO:d try a halving reduction approach 

			if(dbin->cache == NULL){
				// Try a smaller cache size before giving up 
				dbin->bytes_to_read = FORMAT_LENGTH(dbin->input_format);
				dbin->records_in_cache = DEFAULT_CACHE_SIZE/dbin->bytes_to_read; 
				dbin->cache_size = dbin->records_in_cache * dbin->bytes_to_read;

				dbin->data_ptr =
				dbin->cache_end =
				dbin->cache = (char HUGE *)memMalloc((size_t)dbin->cache_size, "DBIN_CACHE_SIZE: HUGE dbin->cache");
			}
*/			

			while (dbin->cache == NULL && dbin->records_in_cache > 0)
			{
				dbin->records_in_cache /= 2;
				dbin->cache_size = dbin->records_in_cache * dbin->bytes_to_read;
				dbin->cache = (char HUGE *)memMalloc((size_t)dbin->cache_size, "DBIN_CACHE_SIZE: HUGE dbin->cache");
			}

			if (!dbin->cache){
				dbin->state.cache_defined = dbin->state.cache_filled = 0;
				sprintf(error_buf, "\nCache in bin %s of size %ld", dbin->title, dbin->cache_size);
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, error_buf));
			}

			*(dbin->cache) = STR_END;
			dbin->state.cache_defined = 1;
			break;

		/*
		 * MESSAGE:     DBIN_DATA_AVAILABLE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)long data_available
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:    Sets dbin->data_available
		 *
		 * DESCRIPTION: The databins use data_available to keep track of how
		 * 				much actual data is available for processing. This is
		 *				particularly useful in the case where record headers
		 *				give the number of data records to be processed before
		 *				the next record header.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_DATA_AVAILABLE"

		case DBIN_DATA_AVAILABLE:
			dbin->data_available = va_arg (args, long);
			break;

		/*
		 * MESSAGE:     DBIN_FILE_HANDLE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:		dbin->file_name must be defined
		 *
		 * CHANGES IN STATE:    Sets dbin->data_available to file length
		 *						Sets dbin->file_location to 0L
		 *						Sets dbin->state.open_file = 1
		 *						Sets dbin->bytes_available to file length.
		 *						dbin->data_available is redefined in DEFINE_HEADER
		 *						if a RECORD header is defined
		 *
		 * DESCRIPTION: This message opens the file given by dbin->file_name.
		 *				If a file is already opened, it is closed prior to
		 *				opening the new file.
		 *
		 * ERRORS:		dbin->file_name not defined.
		 *				Unable to open file.
		 *				Unable to determine file length.
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_FILE_HANDLE"

		case DBIN_FILE_HANDLE:  /* Argument: none */

			/* This option opens a file for the data bin.
				If a file is presently open, it is closed.
				The state of the data_bin is set to DBS_OPEN_FILE.
				Errors go into buffer */

			if (!dbin->state.std_input && dbin->data_file)
				close(dbin->data_file);

			if(dbin->file_name == NULL)
				return(err_push(MESSAGE_NAME, ERR_FILE_DEFINED, "No file name"));
			
			/* open file and check read only */
			dbin->state.read_only = 0;
			dbin->data_file = open(dbin->file_name, O_RDWR | O_BINARY);
			
			if(dbin->data_file == -1){
				dbin->data_file=open(dbin->file_name, O_RDONLY | O_BINARY);
				if(dbin->data_file != -1)
					dbin->state.read_only = 1;
			}

			if(dbin->data_file == -1){
				dbin->data_file = 0;
				sprintf(error_buf, "Problem opening-> %s", dbin->file_name);
				return(err_push(MESSAGE_NAME, errno, error_buf));
			}

			dbin->state.open_file = 1;
			dbin->state.std_input = 0;
			dbin->file_location = 0L;
                                                          
			if((file_size = os_filelength(dbin->file_name)) < 0)
				return(err_push(MESSAGE_NAME, errno, dbin->file_name));

			dbin->data_available = dbin->bytes_available = file_size;
			break;

		/*
		 * MESSAGE:     DBIN_FILE_POSITION
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)long new_position
		 *
		 * PRECONDITIONS:       dbin->data_file must be opened
		 *
		 * CHANGES IN STATE:	Sets dbin->file_location to new_position.
		 * Sets dbin->bytes_available and dbin->data_available to difference
		 * between file_length and new_position.
		 *
		 * DESCRIPTION: Sets the file position for the bin.
		 *
		 * ERRORS:      File not open.
		 *				Unable to seek to location.
		 *				Unable to determine file length.
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_FILE_POSITION"
		 
		 case DBIN_FILE_POSITION:       /* Argument: long new_position */

			dbin->file_location = va_arg (args, long);

			if(!(dbin->data_file || dbin->state.std_input))
			{ /* ERROR: NO FILE DEFINED FOR BIN */
				va_end(args);
				err_push(MESSAGE_NAME, ERR_FILE_DEFINED, dbin->title);
				return(4);
			}
			if (!(dbin->state.open_file || dbin->state.std_input))
			{
				sprintf(error_buf, "File not open for %s", dbin->title);
				va_end(args);
				return(err_push(MESSAGE_NAME, ERR_FILE_DEFINED, error_buf));
			}

			if((file_size = os_filelength(dbin->file_name)) < 0)
				return(err_push(MESSAGE_NAME, errno, dbin->file_name));

			if (lseek(dbin->data_file, dbin->file_location, SEEK_SET) == -1L)
				return(err_push(MESSAGE_NAME, errno, "lseek dbin->data_file"));

			dbin->data_available = dbin->bytes_available = file_size - dbin->file_location;

			break;

		/*
		 * MESSAGE:     DBIN_SET_TOTAL_RECORDS
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:  long records_to_read
		 *
		 * PRECONDITIONS:  dbin->data_file must be opened
		 *                 dbin->input_format must be defined
		 *
		 * CHANGES IN STATE:  dbin->file_location
		 *                    dbin->bytes_available
		 *                    dbin->data_available
		 *
		 * DESCRIPTION:  records_to_read may be either positive or negative, indicating
		 * that that many records should be process from the head of the file (positive)
		 * or the tail of the file (negative).  bytes_available, data_available and
		 * file_location attributes are reset accordingly.
		 *
		 * ERRORS: 
		 *
		 * AUTHOR: Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_SET_TOTAL_RECORDS"
		 
		case DBIN_SET_TOTAL_RECORDS:    /* Argument: long records_to_read */

			l_size = va_arg(args, long); /* (records to read) */
			
			assert(dbin->input_format);
			if (!dbin->input_format)
			{
				err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED, "input format record length unknown");
				break;
			}
			
			if (db_set(dbin, DBIN_FILE_POSITION, 0L, END_ARGS))
				return(err_push(MESSAGE_NAME, ERR_SET_DBIN, "Problem setting data file position"));
		
			if (!l_size)
				break;

			total_bytes = FORMAT_LENGTH(dbin->input_format) * labs(l_size);
		
			if (total_bytes > dbin->bytes_available)
				return(err_push(MESSAGE_NAME, ERR_PARAM_VALUE, "records to read exceeds file length"));
		
			if (l_size /* (records to read) */ < 0)
			{
				if (db_set(dbin, DBIN_FILE_POSITION, dbin->bytes_available - total_bytes, END_ARGS))
					return(err_push(MESSAGE_NAME, ERR_SET_DBIN, "Problem setting data file position"));
			}
			else
			{
				if (dbin->header_format && !(dbin->header_format->type & FFF_SEPARATE))
					total_bytes += FORMAT_LENGTH(dbin->header_format);
				dbin->data_available = dbin->bytes_available = total_bytes;
			}
		
		break;

		/*
		 * MESSAGE:     DBIN_FILE_NAME
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *file_name
		 *
		 * PRECONDITIONS:       dbin->data_file must be opened
		 *
		 * CHANGES IN STATE:    ALLOCATES NEAR MEMORY FOR dbin->file_name.
		 *
		 * DESCRIPTION: Sets the file name for the bin.
		 *
		 * ERRORS:      Lack of memory
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_FILE_NAME"
		 
		case DBIN_FILE_NAME:    /* Argument: char *file_name */
			file_name = va_arg (args, char *);

			if(file_name){
				/* add (char *) for Think C -rf01 */
				dbin->file_name = (char *)memStrdup(file_name, "DBIN_FILE_NAME: dbin->file_name"); /* -rf01 */
				if(dbin->file_name == NULL)
					return(err_push(MESSAGE_NAME, ERR_MEM_LACK, dbin->title));
			}
			break;

		/*
		 * MESSAGE:     DBIN_FLAGS
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)int new_flags
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:    Replaces state information
		 *
		 * DESCRIPTION: Sets the flags for the data bin
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_FLAGS"
		 
		case DBIN_FLAGS:
			dflt = va_arg(args,int); /* use dflt as a temp variable */
			if(dflt & DBS_OPEN_FILE)
				dbin->state.open_file = 1;
			if(dflt & DBS_CACHE_DEFINED)
				dbin->state.cache_defined = 1;
			if(dflt & DBS_FILLED_CACHE)
				dbin->state.cache_filled = 1;
			if(dflt & DBS_HEADER_DEFINED)
				dbin->state.header_defined = 1;
			dflt = 0;
			break;

		/*
		 * MESSAGE:     DBIN_HEADER_FILE_NAME
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *file_name
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:    ALLOCATES MEMORY FOR HEADER FILE NAME
		 *
		 * DESCRIPTION: Sets the header file name for the dbin.
		 *
		 * ERRORS:      Lack of Memory
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_HEADER_FILE_NAME"
		 
		case DBIN_HEADER_FILE_NAME:     /* Argument:  */
			file_name = va_arg (args, char *);

			if(file_name){
				/* add (char *) for Think C -rf01 */
				dbin->header_file_name = (char *)memStrdup(file_name, /* -rf01 */
					"DBIN_HEADER_FILE_NAME: dbin->header_file_name");
				if(dbin->header_file_name == NULL){
					err_push(MESSAGE_NAME, ERR_MEM_LACK, dbin->title);
					break;
				}
			}
			break;

		/*
		 * MESSAGE:     DBIN_OUTPUT_SIZE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)None
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:    REALLOCATES MEMORY FOR THE DBIN BUFFER
		 *
		 * DESCRIPTION: Determines the number of input records in the
		 *				cache and adjusts the size of the output buffer
		 *				to hold that many output records.
		 *
		 * ERRORS:      No Cache Defined
		 *                      No input format defined
		 *                      No output format defined
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    There is an assumption that the dbin->buffer is the
		 *				output buffer. This could clearly cause problems
		 *				as the buffer is near and the output buffer is far.
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_OUTPUT_SIZE"
		 
		case DBIN_OUTPUT_SIZE:  /* Argument: none */

			if (!dbin->cache)
				return(err_push(MESSAGE_NAME, ERR_CACHE_DEFINED, "case DBIN_OUTPUT_SIZE"));
			else if (!dbin->input_format)
				return(err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED, "input_format in DBIN_OUTPUT_SIZE"));
			else if (!dbin->output_format)
				return(err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED, "output_format in DBIN_OUTPUT_SIZE"));

			l_size = FORMAT_LENGTH(dbin->input_format);
			num_in_records =(unsigned)((dbin->cache_end - dbin->cache) / l_size + 0.5);
			l_size = FORMAT_LENGTH(dbin->output_format);

			l_size *= num_in_records;
			if( ((unsigned long)(l_size)) > ((unsigned long)UINT_MAX) )
				return(-1);

			/* ptr to realloc must be malloced first */
			if(dbin->buffer == NULL)         
				dbin->buffer =(char HUGE *)memMalloc((size_t) l_size, "DBIN_OUTPUT_SIZE: HUGE dbin->buffer");
			else
				dbin->buffer =(char HUGE *)memRealloc((void HUGE *)dbin->buffer, (size_t) l_size, "DBIN_OUTPUT_SIZE: HUGE dbin->buffer");
			if(dbin->buffer == NULL)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "Reallocation of dbin->buffer"));

			break;

		/*
		 * MESSAGE:     DBIN_SET_HEADER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *header
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:    Defines dbin-header
		 *
		 * DESCRIPTION: Sets dbin->header to point at the argument.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    This call will become vestigial when the memory
		 *				management moves into the bin library.
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_SET_HEADER"
		
		case DBIN_SET_HEADER:
			dbin->header = va_arg (args, char *);
			break;

		/*
		 * MESSAGE:     DBIN_SET_SCALE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:	VARIABLE_PTR var,
		 *				double inmin,
		 *				double inmax,
		 *				double outmin,
		 *				double outmax
		 *
		 * PRECONDITIONS:       dbin->input_format must be defined for default to work.
		 *
		 * CHANGES IN STATE:    if the format_list is NULL, create the format_list.
		 *                      if scale_format is NULL, create the scale_format.
		 *                      if the variable to be changed is in the scale format, just 
		 *                      the scale factor and offset. Otherwise create the variable
		 *                      and associated scale variable.
		 *	
		 * DESCRIPTION: Defines an scale format for a data bin. The scale format
		 *				is a special structure which is used ONLY in the cv_units
		 *				function when variable is being converted to
		 *				variable_scaled. This conversion is of the form
		 *				var_scaled = (var - offset) * scale factor
		 *				The scale_factor and offset are held in the space for the
		 *				name of the variable after var in the scale format.
		 *
		 * ERRORS:      Lack of memory for temporary file name
		 *                      Problems making format, data error
		 *
		 * AUTHOR:      LPD NGDC,(303)497-6284, lpd@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_SET_SCALE"
		 
		case DBIN_SET_SCALE:
			if(!dbin->input_format)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "dbin->input_format"));

			var = va_arg (args, VARIABLE_PTR);
			if(var == NULL)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "var"));

			inmin=va_arg(args, double);
			inmax=va_arg(args, double);
			outmin=va_arg(args,double);
			outmax=va_arg(args,double);

			if(inmax < inmin || outmax < outmin)
				return(err_push(MESSAGE_NAME, ERR_MAX_LT_MIN, "case DBIN_SET_SCALE"));

			/* create an empty format */
			format = ff_create_format();
			if(!format)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "Scale Format"));

			/* Set the attributes of the scale format from the input format */
			format->num_in_list = 1;
			memStrcpy(format->name, "scale format", "DBIN_SET_SCALE:format->name,\"scale format\"");
			format->max_length = dbin->input_format->max_length;
			format->type = FFF_SCALE | (dbin->input_format->type & FFF_FORMAT_TYPES);

			/* create an empty variable in the scale format */
			format->variables = dll_init();
			if(!format->variables)
				return(err_push(MESSAGE_NAME,ERR_MEM_LACK,"Initial Variable"));
			
			v_list = dll_add(dll_last(format->variables), 0);
			if(!v_list)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "scale var"));
			
			var_scale = (VARIABLE *)memMalloc(sizeof(VARIABLE), "DBIN_SET_SCALE: var_scale");

			if(!var_scale)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "scale var"));

			v_list->data_ptr = (void *)var_scale;
	    
			/* Copy attributes of input variable to output variable */
			memMemcpy((void *)var_scale, (void *)var, sizeof(VARIABLE), "DBIN_SET_SCALE:var_scale,var");
			memStrcpy(var_scale->name, var->name, "DBIN_SET_SCALE:var_scale->name");

			/* Create the variable for holding the offset and scale factor */
			v_list = dll_add(dll_last(format->variables), 0);
			if(!v_list)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "scale var"));
			
			var_scale = (VARIABLE *)memMalloc(sizeof(VARIABLE), "DBIN_SET_SCALE: var_scale");

			if(!var_scale)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "scale var"));

	    v_list->data_ptr = (void *)var_scale;

			double_ptr=(double *)var_scale->name;
			var_scale->name[20]=STR_END;               /* make sure that the name string is termined */
			/* calculate the factor */
			if(inmax == inmin)
				factor=0;
			else
				factor=(outmax-outmin)/(inmax-inmin);
			
			*double_ptr=factor;
			double_ptr++;

			*double_ptr=outmin-inmin*factor;

			/* Replace scale format in format list */
			f_list = db_replace_format(dbin->format_list, format);
			if(!f_list)
				return(err_push(MESSAGE_NAME, ERR_MAKE_FORM, "Replacing Header Format"));

			dbin->format_list = f_list;
			
			break;

		/*
		 * MESSAGE:     DBIN_SET_STRDB
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)DLL_NODE_PTR strdb
		 *
		 * PRECONDITIONS:       strdb must be created externally.
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Sets dbin->strdb to argument.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      lpd, NGDC,(303)497-6284, lpd@kryton.ngdc.noaa.gov
		 *
		 * COMMENTS:    (kbf, 6/23/95) This event is going away, to be
		 *				replaced by DBIN_SET_MENU_INDEX
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_SET_STRDB"

		case DBIN_SET_STRDB:    /* Argument: DLL_NODE_PTR strdb */

			return(err_push(MESSAGE_NAME, ERR_EVENT_RETIRED, "This event has been replaced"));
			
		/*
		 * MESSAGE:     DBIN_SET_MENU_INDEX
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)MENU_INDEX_PTR mindex
		 *
		 * PRECONDITIONS:       the menu index must be created externally.
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Sets dbin->mindex to argument.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      Kevin Frender, kbf@ngdc.noaa.gov
		 *
		 * COMMENTS:    This event replaces DBIN_SET_STRDB with the advent of
		 *				the menu index.
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_SET_MENU_INDEX"
		 
		case DBIN_SET_MENU_INDEX:    /* Argument: MENU_INDEX_PTR mindex */
#ifdef XVT
			dbin->mindex = va_arg(args, MENU_INDEX_PTR);
#endif
			break;
			
		/*
		 * MESSAGE:     DBIN_SET_MENU_SELECTION
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)MENU_SELECTION_PTR mselection
		 *
		 * PRECONDITIONS:       the menu selection struct must be created externally,
		 *						it's extra_info pointer must point to a MENU_OPEN
		 *						struct.
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Sets dbin->gvmselection to argument;
		 *		Sets dbin->gvmopen to (MENU_OPEN_PTR)dbin->gvmselection->extra_info;
		 *		Sets dbin->mindex to dbin->gvmopen->mindex.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      Kevin Frender, kbf@ngdc.noaa.gov
		 *
		 * COMMENTS:    This event replaces DBIN_SET_STRDB with the advent of
		 *				the menu index.
		 *           
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_SET_MENU_SELECTION"
		 
		case DBIN_SET_MENU_SELECTION:    /* Argument: MENU_SELECTION_PTR mselection */
#ifdef XVT
			dbin->gvmselection = va_arg(args, MENU_SELECTION_PTR);
			if(dbin->gvmselection){
				dbin->gvmopen = (MENU_OPEN_PTR)dbin->gvmselection->extra_info;
				if(dbin->gvmopen)
					dbin->mindex = dbin->gvmopen->mindex;
			}

#endif
			break;

		/*
		 * MESSAGE:     DBIN_STD_FORMAT
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)input_format and name_table etc.
		 *
		 * PRECONDITIONS:       
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Set input format, and headerformat from the
		 *				list of standard format.
		 *
		 * ERRORS:      no std_format
		 * (MAO) Need to check ch re: reassignments/multiple free()'s
		 * Looks like ch gets free()'d twice if dbin->file_name is neither a
		 * .pcx nor a .hdf and standard format is "unknown" or "cancel".
		 * Memory leak (ch) if dbin->file_name is an .hdf 
		 *
		 * AUTHOR:      lpd, NGDC,(303)497-6284, lpd@kryton.ngdc.noaa.gov
		 *
		 * COMMENTS:  	GIF is not a "standard format" since there is no fmt
		 *				description for it.
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_STD_FORMAT"
		 
		case DBIN_STD_FORMAT:

#ifdef XVT

			if (!dbin->file_name)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "file name undefined"));

			ch = (char *)memMalloc(120, "DBIN_STD_FORMAT:ch");
			if (!ch)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "allocating temp memory"));

			Keep_User_Choice = FALSE;

			point = os_path_return_ext(dbin->file_name);
			if (point)
			{
				/* put the standard format here */
				if (os_strcmpi(point, "pcx") == 0)
				{
					dbin->type=DATA_TYPE_IMAGE;
					os_path_prepend_special("pcx", cd_device, ch);
				}
				else if (os_strcmpi(point, "bmp") == 0)
				{
					dbin->type=DATA_TYPE_IMAGE;
					os_path_prepend_special("bmp", cd_device, ch);
				}
				else if (os_strcmpi(point, "hdf") == 0)
				{
					if (db_hdf_fmt_to_buffer(dbin, NULL, NULL, &hdf_tag, &hdf_ref))
						return(err_push(ROUTINE_NAME, ERR_GET_HDF_FMT, "hdf format to buffer"));

					if (db_set(dbin, INPUT_FORMAT, NULL, dbin->buffer, END_ARGS))
						return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "Setting header and input format"));

					if (db_set(dbin, OUTPUT_FORMAT, NULL, dbin->buffer, END_ARGS))
						return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "Setting output format"));

					db_set(dbin, DBIN_BYTE_ORDER, END_ARGS);
					break;
				} /* if dbin->file_name extension is hdf */
				else
				{ /* ask user */
					if (!nt_askvalue(dbin, "standard_format_name", FFV_CHAR, temp, NULL))
					{
						memFree(ch, "DBIN_STD_FORMAT:ch");  
						ch = stdform(dbin->buffer);  /* get the standard file format name */
						(void)os_str_trim_whitespace(ch, ch);
						Keep_User_Choice = TRUE;
					}
					else
						os_path_prepend_special(temp, cd_device, ch);
				} /* if extension is neither pcx nor hdf */
			} /* if extension in dbin->file_name */
			else
			{
				if (!nt_askvalue(dbin, "standard_format_name", FFV_CHAR, temp, NULL))
				{
					memFree(ch, "DBIN_STD_FORMAT:ch");
					ch = stdform(dbin->buffer);  /* get the standard file format name */
					(void)os_str_trim_whitespace(ch, ch);
					Keep_User_Choice = TRUE;
				}
				else
					os_path_prepend_special(temp, cd_device, ch);
			} /* if no extension in dbin->file_name */

/* MAO:c Status of ch:
   (1) if dbin->file_name extension is pcx then ch is "$GEOVU_DIR{/\:}pcx"
   (2) if dbin->file_name extension is hdf then ch is allocated but not
       free()'d, a break is executed and next set dbin event flag is processed
   (3) if dbin->file_name extension is neither pcx nor hdf then:
       (i) if "standard_format_name" is found in header then ch is the
           translation and prepended with $GEOVU_DIR
       (ii) else user is prompted, and apparently no prepending with $GEOVU_DIR
   (4) if dbin->file_name has no extension do same as (3)
   
   NOTE: in steps 3ii and 4ii ch is free()'d and reassigned!
*/
			if (!ch)
			{
				err_push(MESSAGE_NAME, ERR_GENERAL, "Cannot set a standard format");
				return(1);
			}

			if (os_strcmpi(ch, "unknown") == 0)
			{
				err_push(MESSAGE_NAME, ERR_UNKNOWN_FORM_TYPE, "data format");
				memFree(ch, "DBIN_STD_FORMAT:ch");
				return(2);
			}

			if (os_strcmpi(ch, "cancel") == 0)
			{
				memFree(ch, "DBIN_STD_FORMAT:ch");
				return(3);
			}
			if (Keep_User_Choice)
				if (!nt_putvalue(dbin, "standard_format_name", FFV_CHAR, ch, 0))
					err_push(MESSAGE_NAME, ERR_DEBUG_MSG + ERR_NAME_TABLE, "data format");

			/* get format and name-equivalence table */
			
			os_path_put_parts(temp, NULL, ch, "eqv");
			
			/* remove old name table, but reserve */
			old_table = NULL;
			/* use new_table as a temporary intermediary */
			new_table = nt_get_name_table(dbin->table_list, FFF_INPUT);
			if (new_table)
			{
				if (nt_merge(new_table, &old_table)) /* copy new_table to old_table */
					return(err_push(ROUTINE_NAME, ERR_NT_MERGE, "updating name table"));
				
				/* now we remove the old ("new_table") name table */
				new_table = nt_remove_name_table(&(dbin->table_list), new_table);
			}
			/* now old_table contains dbin's old name table, if any */

			(void)db_set(dbin, MAKE_NAME_TABLE, temp, END_ARGS);

			if (err_state())
			{
				if (dbs_error_verbose_on)
					err_disp();
				else
					err_clear();
			}

			/* connect the old table with newly created table */

			if(((new_table = nt_get_name_table(dbin->table_list, FFF_INPUT)) != NULL) && old_table)
			{ /* merge the two name tables */
				if (nt_merge(old_table, &new_table))
					return(err_push(ROUTINE_NAME, ERR_NT_MERGE, "updating name table"));
				
				nt_free_name_table(old_table);
			}
		
			/* see if new name table has a new definition for ff_input_format,
			   ff_header_format, or ff_format_fmt */
			(void)db_set(dbin, INPUT_FORMAT, NULL, NULL, END_ARGS);
			(void)db_set(dbin, OUTPUT_FORMAT, NULL, NULL, END_ARGS);

			if (dbin->input_format == NULL)
			{
				os_path_put_parts(temp, NULL, temp, "fmt");
				if (os_file_exist(temp))
				{
					(void)db_set(dbin, INPUT_FORMAT, temp, NULL, END_ARGS);
					(void)db_set(dbin, OUTPUT_FORMAT, temp, NULL, END_ARGS);
				}
			}

			if (dbin->input_format == NULL)
			{
				os_path_put_parts(temp, NULL, temp, "bfm");
				if (os_file_exist(temp))
					(void)db_set(dbin, INPUT_FORMAT, temp, NULL, END_ARGS);
			}

			if(dbin->input_format == NULL)
			{
				os_path_put_parts(temp, NULL, temp, "afm");
				if (os_file_exist(temp))
					(void)db_set(dbin,  INPUT_FORMAT, temp, NULL, END_ARGS);
			}
	
			/* create header file by using defaults */
			if(dbin->input_format && (!dbin->header))
			{
				v_list = FFV_FIRST_VARIABLE(dbin->input_format);
				while(FFV_VARIABLE(v_list)){
					if(IS_HEADER(FFV_VARIABLE(v_list)))
						break;
					v_list = dll_next(v_list);
				}

/* MAO:c is var even defined here? -- it's only set in DBIN_SET_SCALE! */
/* MAO:d This looks like a mistake, so I'm removing var here -- CAUTION:
   I am next using var in INPUT_FORMAT and OUTPUT_FORMAT, so if something
   magical is really happening here with incarnations of var, probably ought
   to undo using var as a temp variable in INPUT_FORMAT and OUTPUT_FORMAT
   
				if(var || dbin->header_format)
*/
				if(dbin->header_format)
				{
					if(db_set(dbin, DEFINE_HEADER, NULL, NULL, END_ARGS))
					{
						memFree(ch, "DBIN_STD_FORMAT:ch");
						return(err_push(MESSAGE_NAME, ERR_SET_DBIN, "defining header from STD form"));
					}
				}
			}
	
			if(!dbin->input_format)
			{
				memFree(ch, "DBIN_STD_FORMAT:ch");
				return(err_push(MESSAGE_NAME,ERR_MAKE_FORM, "Standard Format"));
			}
				
			memFree(ch, "DBIN_STD_FORMAT:ch");

#else /* XVT */
			return(err_push(MESSAGE_NAME, ERR_EVENT_RETIRED, "This event only used by GeoVu applications"));
#endif /* XVT */


		/*
		 * MESSAGE:     DEFINE_HEADER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *file_name char * header_file_name
		 *
		 * PRECONDITIONS:       None
		 *
		 * CHANGES IN STATE:    Checks for delimited fields in header.
		 *						ALLOCATES MEMORY FOR HEADER
		 *		                ALLOCATES MEMORY FOR header_delimited_fields ARRAY.
		 *		                Sets dbin->data_available = 0 if a record header        
		 *		                Sets dbin->state.header_defined = 1
		 *		                Sets dbin->state.header_delimited = 1
		 *
		 * DESCRIPTION: Defines a header for a data bin.
		 *				The FREEFORM system presently recognizes three
		 *				types of headers:
		 *				File Headers:
		 *			        Only one file header can be associated with
		 *			        a DATA_BIN.
		 *			        It either occurs at the beginning of the data
		 *			        file or in a file of it's own (SEPARATE).
		 *				Record Headers:
		 *			        Record headers occur once for every record
		 *			        of a data file. They can either be interspersed
		 *			        with the data or in a file of their own (SEPARATE);
		 *				Index Headers:
		 *			        see index.h for format
		 *				At present only one INPUT and OUTPUT header format is
		 *					allowed / format_list.
		 *				The HEADER event processing includes the following
		 *					steps:
		 *		        1) Check for header format in format list.
		 *	                If list exists with no header format: ERROR (NO!!!)
		 *		        2) Make header format from file given as first
		 *	                argument.
		 *		        3) Define header file (second argument if SEPARATE).
		 *		        3) Read Header if it is file header
		 *
		 *				In order that PROCESS_FORMAT_LIST can process both
		 *				a file header and record headers:
		 *		        1) make sure there is enough room in the header buffer
		 *
		 *
		 * ERRORS:
		 *                      both the filename and header being defined
		 *                      Format list with no header format
		 *                      Problems making header format
		 *                      No Header file found
		 *                      Lack of memory for header
		 *
		 * AUTHOR:      LPD,TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    This call will become vestigial when the memory
		 *				management moves into the bin library.
		 *				Why is the header_delimited_fields array not set?
		 *
		 * KEYWORDS:    
		 *
		 */                 		 
		/*
		 * HISTORY:
		 *	Rich Fozzard	9/26/95		-rf01
		 *		make header_file_path native before using
		*/
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DEFINE_HEADER"
		
		case DEFINE_HEADER:     /* Argument: char *header_format_file,  char * buffer] */
			
			file_name = va_arg (args, char *);
			format_buffer = va_arg (args, char *);
			
			format_type = FFF_INPUT;
			
			/* check the dbin->buffer -- but is this really necessary? */
			if (dbin->buffer == NULL)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "dbin->buffer undefined"));

			/* if format is set, a new header format has been created and needs
			   to be added into the format list, otherwise the header format is
			   already in the format list */
			format = NULL;
						
			if (file_name || format_buffer)
			{
				assert(!(file_name && format_buffer));
				
				if (file_name)
				{
					generic_bufsize.buffer = dbin->buffer;
					generic_bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
					
					error = ff_file_to_bufsize(file_name, &generic_bufsize);
					if (error)
						return(err_push(ROUTINE_NAME, error, file_name));
				}
				else
				{
					generic_bufsize.buffer = format_buffer;
					generic_bufsize.total_bytes = strlen(format_buffer) + 1;
					generic_bufsize.bytes_used = generic_bufsize.total_bytes;
				}
				
				f_list = NULL;

				pp_object.ppo_type = PPO_FORMAT_LIST;
				pp_object.ppo_object.hf_list = &f_list;

				if (ff_text_pre_parser(file_name ? os_path_return_ext(file_name) :
				                                   NULL,
				                       &generic_bufsize,
				                       &pp_object))
				{
				}
				else
				{
					format = 
					header_format = db_find_format(f_list,
					                               FFF_GROUP,
					                               FFF_FILE | FFF_HD,
					                               END_ARGS
					                              );
					format->type |= FFF_HD;
				}

				
				if (err_state())
				{
					if (dbs_error_verbose_on)
						err_disp();
					else
						err_clear();
				}
			} /* if file name or format buffer */
			
#ifdef XVT

			else if (!dbs_search_menu_formats(FFF_HD | FFF_FILE, dbin, dbin->mindex, dbin->buffer, &f_list))
			{
				header_format =
				format = db_find_format(f_list, FFF_GROUP, FFF_HD, END_ARGS);
			}

#endif /* XVT */

			else if (dbin->format_list)
			{  /* Get format from list */
				header_format = db_find_format_is_isnot(dbin->format_list,
				                 FFF_GROUP, FFF_FILE | FFF_HD, FFF_OUTPUT, END_ARGS);
				if (!header_format)
					header_format = db_find_format_is_isnot(dbin->format_list,
					                 FFF_GROUP, FFF_REC | FFF_HD, FFF_OUTPUT, END_ARGS);
			}

			if (!header_format)
				/* Check for old style header variable in input */
				header_format = format = ff_make_header_format(dbin->input_format, dbin->buffer);
			
			/* if the format is newly created, put in the flist */
			if (format)
			{
				format->type |= (FFF_HD | format_type);
				f_list = db_replace_format(dbin->format_list, format);
				if (!f_list)
					return(err_push(MESSAGE_NAME, ERR_MAKE_FORM, "Replacing Header Format"));
				dbin->format_list = f_list;
				format = NULL;
			}

			if (!header_format)
				break;

			/* get the header type */
			if (nt_askvalue(dbin, "header_type", FFV_CHAR, temp, NULL))
			{
				if (!os_strcmpi(temp, "header_embedded"))
				{
					header_format->type &= ~FFF_SEPARATE;
					dbin->header_type = HEADER_EMBEDDED;
				}
				else if (!os_strcmpi(temp, "header_embedded_varied"))
				{
					header_format->type &= ~FFF_SEPARATE;
					header_format->type |= FFF_VARIED;
					dbin->header_type = HEADER_EMBEDDED_VARIED;
				}
				else if (!os_strcmpi(temp, "header_separated"))
				{
					header_format->type |= FFF_SEPARATE;
					dbin->header_type = HEADER_SEPARATED;
				}
				else if (!os_strcmpi(temp, "header_separated_varied"))
				{
					header_format->type |= FFF_SEPARATE;
					header_format->type |= FFF_VARIED;
					dbin->header_type = HEADER_SEPARATED_VARIED;
				}
			}
			else if (dbin->header_type >= HEADER_EMBEDDED &&
			         dbin->header_type <= HEADER_SEPARATED_VARIED)
				switch (dbin->header_type)
				{
					case HEADER_EMBEDDED:
						header_format->type &= ~FFF_SEPARATE;
					break;

					case HEADER_EMBEDDED_VARIED:
						header_format->type &= ~FFF_SEPARATE;
						header_format->type |= FFF_VARIED;
					break;

					case HEADER_SEPARATED:
						header_format->type |= FFF_SEPARATE;
					break;

					case HEADER_SEPARATED_VARIED:
						header_format->type |= FFF_SEPARATE;
						header_format->type |= FFF_VARIED;
					break;
				}

			/* Save the original number of variables */
			dbin->num_header_vars = header_format->num_in_list;

			/* Initialize an array of unsigned chars, one for each
			variable in the format. The elements of this array are
			set to 1 if that variable is delimited and 0 if it is not.
			This setting occurs in the function ff_get_delimiters */

			/* ptr to realloc must be malloced first */
			if(dbin->header_delimited_fields == NULL)
				dbin->header_delimited_fields =
				(unsigned char *)memMalloc((size_t)header_format->num_in_list, "DEFINE_HEADER:dbin->header_delimted_fields");
			else
				dbin->header_delimited_fields =
				(unsigned char *)memRealloc((void *)(dbin->header_delimited_fields),
				(size_t)header_format->num_in_list, "DEFINE_HEADER:dbin->header_delimted_fields");

			if (!dbin->header_delimited_fields)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "Realloc dbin->header_delimited_fields"));

			memMemset((MEM_CHAR *)dbin->header_delimited_fields,STR_END, header_format->num_in_list,"dbin->header_delimited_fields,'\\0',header_format->num_in_list");

			/* Formats with delimited fields are identified by having
			variables with start and end positions both set to 0. Check
			this format to see if it is delimited. */

			dbin->state.header_delimited = 0;

			v_list = FFV_FIRST_VARIABLE(header_format);
			while (!dbin->state.header_delimited && FFV_VARIABLE(v_list))
			{ /* Check to see if it is a delimited variable */
				if (FFV_VARIABLE(v_list)->start_pos == 0 && !IS_CONVERT(FFV_VARIABLE(v_list)))
				{
					dbin->state.header_delimited = 1;
					break;
				}
				v_list = dll_next(v_list);
			}

			/* set the header type in case the header type is read from FMT */
			if (IS_SEPARATE(header_format))
			{
				if (dbin->state.header_delimited)
				{
					dbin->header_type = HEADER_SEPARATED_VARIED;
					header_format->type |= FFF_VARIED;
				}
				else
					dbin->header_type = HEADER_SEPARATED;
			}
			else
			{
				if (dbin->state.header_delimited)
				{
					dbin->header_type = HEADER_EMBEDDED_VARIED;
					header_format->type |= FFF_VARIED;
				}
				else
					dbin->header_type = HEADER_EMBEDDED;
			}
			if (dbin->header_type == HEADER_NONE)
				return(err_push(MESSAGE_NAME, ERR_UNKNOWN_FORM_TYPE, "header format"));

			/* get the header in */
			file = -1;        
			switch (dbin->header_type)
			{
				default:
				case HEADER_NONE:                       /* no header */
					dbin->header = NULL;
				break;

				case HEADER_EMBEDDED:
				case HEADER_EMBEDDED_VARIED:
					file = dbin->header_file = dbin->data_file;
					dbin->header_file_name = dbin->file_name;
					bytes_read = header_format->max_length;
	
					/* Check to see if we have both a file header and record header */
					if (IS_FILE(header_format))
					{
						format = db_find_format(dbin->format_list, FFF_GROUP, FFF_REC, END_ARGS);
						if (format)
						{
							bytes_read += format->max_length;
							dbin->data_available = 0;
							format = NULL;
						}
					}
	
					if (bytes_read == 0)
					/* the header for embedded varied header must be less than 1000 bytes */
						bytes_read = header_format->max_length = 1000;

				case HEADER_SEPARATED:
				case HEADER_SEPARATED_VARIED:

				/* In this case the header is in a separate file from the data.
				This file must be found and opened */
					number = 0; /* number of default files found */
					ch_ptr = dbin->buffer;
					if (file == -1)
					{ /* seperated header file */
						/* make the seperated header file name */
						if (nt_askvalue(dbin, "header_file_name", FFV_CHAR, temp, NULL))
						{
							*ch_ptr = STR_END;
#ifdef XVT
							if (dbin->mindex)
								memStrcpy(ch_ptr,cd_device, "DEFINE_HEADER:ch_ptr,cd_device");
#endif /* XVT */

							memStrcat(ch_ptr, temp, "DEFINE_HEADER:ch_ptr,temp");
						}
						else
						{
						/* construct the header filename from file path and extension,
						   assuming that the header file has same name as data file but with different extension */
						   
/* MAO:d Much of this is redundant with db_find_header_file(), however, there
   some differences in logic, regarding querying the name table for
   header_file_name and header_file_ext -- this is problematic. 
   db_find_header_file() currently only looks for .hdr files, whereas this
   code block needs to look for header files with a custom extension, e.g.,
   .doc   A useful extension to db_find_*_file() would be the ability to
   search with non-default values, i.e., search directories and extensions. */
   
							i = 0;
							dbin->header_file = -1;
							while(i < 2 && dbin->header_file == -1)
							{
								i++;
								ch = NULL;
								error = 0;

/* MAO:c first try env. var. HEADER_DIR, then try header_file_path in eqv */							
								if (i == 1)
								{
									ch=os_get_env("HEADER_DIR");
									if(ch){
										strcpy(temp, ch);
										error=1;
									}
								}
								else {
									if (nt_askvalue(dbin, "header_file_path", FFV_CHAR, temp, NULL))
									{
										error=1;
										os_path_make_native(temp,temp); /* make this native before using -rf01 */
									}
								}
	
								if (error)
								{ 
									/* HEADER_DIR (i==1) or header_file_path (i==2) defined */
									*ch_ptr=STR_END;
#ifdef XVT
									if (dbin->mindex && !ch)
										/* if the header is in header_dir, not copy cd driver */
										memStrcpy(ch_ptr, cd_device, "DEFINE_HEADER:ch_ptr,cd_device");
#endif /* XVT */
	
									memStrcat(ch_ptr, temp, "DEFINE_HEADER:ch_ptr,temp");
									os_path_find_parts(dbin->file_name, NULL, &ch, NULL);
									os_path_put_parts(ch_ptr, ch_ptr, ch, NULL);
								} /* error == 1 */
								else /* couldn't get a path */
									os_path_get_parts(dbin->file_name, NULL, ch_ptr, NULL);
	
								if (nt_askvalue(dbin, "header_file_ext", FFV_CHAR, temp, NULL))
									os_path_put_parts(ch_ptr, NULL, ch_ptr, temp);
								else
									os_path_put_parts(ch_ptr, NULL, ch_ptr, "hdr");
	
								dbin->header_file = open(ch_ptr,O_RDONLY | O_BINARY);
								if(dbin->header_file != -1)
								{
									close(dbin->header_file);
									break;
								}
							} /* while */
						} /* header_file_name not defined in name table */

/* here ends the gruesome in-line header file finding code */

						/* open the header file */
						dbin->state.read_only = 0;
						if ((dbin->header_file = open(ch_ptr, O_RDWR | O_BINARY)) == -1)
						{
							dbin->header_file = open(ch_ptr, O_RDONLY | O_BINARY);
							if (dbin->header_file != -1)
								dbin->state.read_only = 1;
						}       
	
						/* check the existance of the header file, and get the size */
						if (dbin->header_file == -1)
						{
							if (nt_askvalue(dbin, "header_file_ext", FFV_CHAR, temp, NULL))
								os_path_put_parts(ch_ptr, NULL, dbin->file_name, temp);
							else
								os_path_put_parts(ch_ptr, NULL, dbin->file_name, "hdr");
	
							dbin->state.read_only = 0;
							if ((dbin->header_file = open(ch_ptr, O_RDWR | O_BINARY)) == -1)
							{
								dbin->header_file = open(ch_ptr, O_RDONLY | O_BINARY);
								if (dbin->header_file != -1)
									dbin->state.read_only = 1;
							}       
						}
	
						if (dbin->header_file == -1)
						{
							env_dir = os_get_env("HEADER_DIR");
							if ((number = ff_find_files(dbin->file_name, "hdr", env_dir, &found_files)) != 0)
							{
								dbin->state.read_only = 0;
								if ((dbin->header_file=open(found_files[0],O_RDWR | O_BINARY)) == -1)
								{
									dbin->header_file = open(found_files[0], O_RDONLY | O_BINARY);
									if (dbin->header_file != -1)
										dbin->state.read_only = 1;
								}       
							}
						}               
	
						if (dbin->header_file == -1)
						{
								memFree(dbin->header, "DEFINE_HEADER:dbin->header");
								dbin->header = NULL;
								return(err_push(MESSAGE_NAME, ERR_OPEN_FILE, number ? found_files[0] : ch_ptr));
						}
						else
						{
							db_set(dbin, DBIN_HEADER_FILE_NAME, number ? found_files[0] : ch_ptr, END_ARGS);
							file = dbin->header_file;
							bytes_read = (unsigned int)os_filelength(dbin->header_file_name);
						}
					}
				break;
			} /* switch header_type */
			
			if (dbin->header_type != HEADER_NONE) /* this if() test can go away */
			{
				/* read the file header */
				if (lseek(file, 0L, SEEK_SET) == -1L)
					return(err_push(MESSAGE_NAME, errno, "lseek dbin->header_file"));
				number = 3;
				if (dbin->header_type == HEADER_EMBEDDED_VARIED ||
				    dbin->header_type == HEADER_SEPARATED_VARIED)
					number = 501; /* favorite Levis? */
				dbin->header = (char *)memMalloc((size_t)(bytes_read + number), "DEFINE_HEADER:dbin->header");
				error = memRead(file, dbin->header, bytes_read, NO_TAG);
				*(dbin->header + bytes_read) = STR_END;
/* MAO:d header_format->max_length is now set in ff_header_to_format()
					if (dbin->header_type == HEADER_SEPARATED ||
					    dbin->header_type == HEADER_SEPARATED_VARIED)
						if (!(header_format->type & FFF_REC))
							header_format->max_length = (short)bytes_read;
*/    
			}

			/* make variable length, variable position header format, need to be modified */
			if (dbin->header_type == HEADER_SEPARATED_VARIED ||
			 dbin->header_type == HEADER_EMBEDDED_VARIED)
			 { /* get the delimiter 1, delimiter 2 and/or distance */
				number = 0;
				delim_item = STR_END;
				delim_value = STR_END;
				if (nt_askvalue(dbin, "delimiter_item", FFV_CHAR, temp, NULL))
					delim_item = strascii(temp);

				if (delim_item == STR_END)
					delim_item = '\n';

				if (nt_askvalue(dbin, "_distance", FFV_SHORT, temp, NULL))
					number = *((short *)temp);

				if (number == 0)
				{
					if (nt_askvalue(dbin, "delimiter_value", FFV_CHAR, temp, NULL))
						delim_value = strascii(temp);
					else
						delim_value= '=';
				}

				if (ff_header_to_format(header_format, dbin->header, delim_item,
				    delim_value, number))
				{
					return(err_push(ROUTINE_NAME, ERR_HEAD_FORM, "Unable to create header format"));
				}
			} /* if header is varied (separated or embedded) */

			/* redefine the header length for the embedded varied header */
			if (dbin->header_type == HEADER_EMBEDDED_VARIED)
			{
				if (nt_askvalue(dbin, "header_length", FFV_USHORT, &number, NULL))
					header_format->max_length = *(unsigned short *)&number;
				else
					return(err_push(MESSAGE_NAME, ERR_HEAD_LENGTH, "unknown length"));
			}

			/* the header and data files need to be at 0L so that
			PROCESS_FORMAT_LIST can do its thing */

			if (lseek(dbin->header_file, 0L, SEEK_SET) == -1L)
				return(err_push(MESSAGE_NAME, errno, "lseek dbin->header_file"));
			if (lseek(dbin->data_file, 0L, SEEK_SET) == -1L)
				return(err_push(MESSAGE_NAME, errno, "lseek dbin->data_file"));

			if (header_format->type & FFF_REC)
				dbin->data_available = 0;       
			if (dbin->header_file == -1)
				return(err_push(MESSAGE_NAME, errno, "Header File"));

			dbin->state.header_defined = 1;

			dbin->header_format = header_format;

			/* define data_byte order */
			db_set(dbin, DBIN_BYTE_ORDER, END_ARGS);

			/* convert the header to native format */
			if (dbin->header)
				db_events(dbin, DBIN_DATA_TO_NATIVE, dbin->header,
				 dbin->header + header_format->max_length - 1,
				 header_format, END_ARGS);

			break;
			
		/*
		 * MESSAGE:     INPUT_FORMAT
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *file_name, char *format_buffer
		 *
     * file_name is the name of a format file (.afm, .dfm, .bfm, and .fmt
     * extensions allowed) and format_buffer is a character pointer to text
     * containing a format specification block (i.e., the contents of a .afm,
     * .dfm, or .bfm file).  If both arguments are NULL, this provokes a
     * search for defaulting format (detailed under DESCRIPTION).  If both
     * arguments are non-NULL, then format_buffer is taken not as a format
     * specification block, but as the desired format title (assuming that more
     * than one format specification in the located format file is eligible).
     *
     * If file_name is set to FORMAT_BUFFER_IS_A_TITLE 
     * then a search for a defaulting format file ensues, with the contents
     * of format_buffer taken as the desired format title.
     *
		 * PRECONDITIONS: dbin->file_name must be defined for default searches
		 *
		 * CHANGES IN STATE:    ALLOCATES MEMORY FOR TEMPORARY FILE NAME
		 * dbin->bytes_to_read set to input format record length
		 * dbin->records_in_cache set according to cache size and input format
		 *                        record length
		 *                
		 * DESCRIPTION: 
		 *
		 * If file_name is NULL and format_buffer is not, then format_buffer is
		 * assumed to contain a format specification block (i.e., the contents of
		 * a .afm, .bfm, or .dfm file).
		 * NOTE:  This is different from the FORMAT_LIST event, which assumes
		 * format_buffer to contain a format description block (i.e., the contents
		 * of a .fmt file).
		 * format_buffer containing a format specification is used to create
		 * a format, which is added into dbin->format_list and assigned to
		 * dbin->input_format.
		 * Currently the format created in this way is automatically given the
		 * binary file
		 * format descriptor type.  This apparently is an oversight and should be
		 * corrected.  My suggestion (MAO) is to use the dbin->file_name extension
		 * and the default extension rules, (i.e., .dat means ASCII, .dab means
		 * dBASE, and anything else means binary).
		 *
		 * If both file_name and format_buffer are non-NULL (and file_name
		 * has not been set to FORMAT_BUFFER_IS_A_TITLE) then file_name is
		 * assumed to be the name of a .fmt file and format_buffer the desired
		 * format title.  The contents of the format file are used to create a
		 * temporary format list, but all data formats except the first occurring
		 * format matching the given format title are deleted.  The first matching
		 * format is added into dbin->format_list and assigned to dbin->input_format.
		 *
		 * If file_name has been set to FORMAT_BUFFER_IS_A_TITLE and format_buffer
		 * is not NULL then the format_buffer is assumed to be the desired format
		 * title.  Even though file_name is set to FORMAT_BUFFER_IS_A_TITLE the
		 * menu is searched first for formats.  This means that format titles can
		 * be applied to format sections in menu files.  If the menu does not
		 * produce a format then
		 * a default file search is conducted.  If a format file is found, then the
		 * steps above for file_name and format_buffer being non-NULL are
		 * performed.
		 *
		 * If file_name is non-NULL and format_buffer is NULL, then file_name
		 * is assumed to be the name of a format file (.?fm or .fmt).  If a .?fm
		 * file, then the contents are used to create a format, which is assigned
		 * a file format descriptor type based on extension (.afm => ASCII,
		 * .dfm => dBASE, and .bfm => binary).  The format is added into
		 * dbin->format_list and assigned to dbin->input_format.  If a .fmt file,
		 * then the contents are used to create a temporary format list.
		 * 
		 * If both file_name and format_buffer are NULL then the GeoVu menu file
		 * is searched.  See the DESCRIPTION for dbs_search_menu_formats() for
		 * more information.
		 *
		 * If both file_name and format_buffer are NULL and the menu search fails
		 * then a default format
		 * file search is conducted.  If a format file is found, then the contents
		 * are used to create a temporary format list, and the first input format
		 * is selected to be added into dbin->format_list and assigned to
		 * dbin->input_format.  All output and read/write ambiguous data formats are
		 * deleted from the temporary format list.  All other input formats which
		 * do not have the same format title also deleted.  If multiple input
		 * formats with identical titles exist in the same .fmt file, problems
		 * may result, and no measures are taken to detect or prevent this.  If no
		 * input format exists, then ambiguous formats are marked as either input
		 * or output according to the extension of dbin->file_name and the format
		 * file type (i.e., .dat/ASCII => input, .dat/binary/dBASE => output,
		 * .bin/binary => input, .bin/ASCII/dBASE => output, .dab/dBASE => input,
		 * .dab/ASCII/binary => output).  After this ambiguity resolution, the
		 * first input format is selected to be added into dbin->format_list and
		 * assigned to dbin->input_format.  All other data formats are deleted from
		 * the temporary format list.
		 * 
		 * If an input format cannot be created above, this event returns an error
		 * code.  Pending events are NOT processed.
		 *
		 * However, if an input format has been created in one of the above
		 * scenarios, it is then checked for an "old style" header format variable.
		 * If such exists, the header format is created via the input format and
		 * added into the format list.
		 *
		 * Whether an old style header variable exists in the input format
		 * or not, the DEFINE_HEADER event is then called.  After the return of
		 * this event, the input format is checked to see if it is a delimited
		 * format (meaning that the variable start and end positions must be
		 * determined dynamically).
		 *
		 * Lastly, dbin->bytes_to_read is set to the input record length, and
		 * dbin->records_in_cache is set to the largest whole multiple of input
		 * records that will fit into dbin->cache, according to dbin->cache_size.
		 *
		 * ERRORS:      Lack of memory for temporary file name
		 *                      Problems making input format
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 * modified substantially by: Kevin Frender, kbf@ngdc.noaa.gov
		 *
		 * COMMENTS:    Things are written to ch_ptr without checking to see if
		 *				it is initialized. This seems unsafe.
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set INPUT_FORMAT"
		 
		case INPUT_FORMAT:      /* Argument: char *file_name, char *buffer */
			file_name = va_arg (args, char *);
			format_buffer = va_arg (args, char *);

			title_specified = NULL;

			if (file_name && format_buffer)
			{ /* title is specified in format_buffer */
				if (!strcmp(file_name, FORMAT_BUFFER_IS_A_TITLE))
					file_name = NULL;

				title_specified = format_buffer;
				format_buffer = NULL;
			}
			
		   	  
			error = make_format_eqv_list(FFF_INPUT | FFF_DATA,
			                             file_name,
			                             format_buffer,
			                             &f_list,
			                             &(dbin->table_list),
			                             dbin
			                            );
			if (error)
				return(error);
			
			if (dbin->state.std_input)
				/* Remove all separate formats from dbin->format_list */
				(void)db_delete_all_formats(f_list, FFF_SEPARATE);
				
			if (title_specified)
			{
				/* there will be only one surviving input data format --
				   ensure that the user-designated format survives by marking
				   all data formats as FFF_OUTPUT, and then marking title-
				   selected format as FFF_INPUT.  All FFF_OUTPUT formats will
				   be deleted from f_list prior to merging with
				   dbin->format_list */
							 
				while ((format = db_find_format_is_isnot(f_list, FFF_GROUP,
				                 FFF_DATA, FFF_OUTPUT, END_ARGS)) != NULL)
				{
					format->type &= ~FFF_INPUT;
					format->type |= FFF_OUTPUT;
				}
				
				format = db_find_format(f_list, FFF_NAME, title_specified, END_ARGS);
				if (format)
				{
					format->type &= ~FFF_OUTPUT;
					format->type |= FFF_INPUT;
				}
				else
					return(err_push(ROUTINE_NAME, ERR_FIND_FORM, title_specified));
			} /* if title_specified */
			else
			{
				format = db_find_format(f_list, FFF_GROUP, FFF_INPUT | FFF_DATA, END_ARGS);
				db_format_list_mark_io(f_list, dbin->file_name);
	
				if (!format)
					format = db_find_format(f_list, FFF_GROUP, FFF_INPUT | FFF_DATA, END_ARGS);
							
				if (format)
					db_delete_rivals(f_list, format);
			}
			
			if (format)
				dbin->input_format = format;
			else
				error = ERR_MAKE_FORM;
			
			/* Remove any output formats from the list */
			(void)db_delete_all_formats(f_list, FFF_OUTPUT);
			/* This is done to avoid confusion if the input format file
			 * contained output formats, but a seperate output format
			 * file was defined. */
			
			/* And merge the list we have into dbin->format_list */

			dbin->format_list = db_merge_format_lists(dbin->format_list, f_list);
			if (!dbin->format_list)
				return(ERR_MAKE_FORM);

			/* The db_merge_format_list function frees f_list */
			f_list = NULL;
			format = NULL;
			
			/* End checking for input format. */
			
			/* MAO:c following test can only be FALSE only if dbin->header_format
			   was set manually, or DEFINE_HEADER event was called prior to the
			   INPUT_FORMAT event (this event) */
			if (!dbin->header_format)
			{ /* dbin->header_format is set only within DEFINE_HEADER.  The first
			     time through INPUT_FORMAT dbin->header_format will be NULL, and so
			     the DEFINE_HEADER event will be called.
			  */
				/* create header format and get header in by default */
				(void)db_set(dbin, DEFINE_HEADER, NULL, NULL, END_ARGS);
			}

			if (dbin->input_format)
			{
				v_list = FFV_FIRST_VARIABLE(dbin->input_format);
				while (FFV_VARIABLE(v_list))
				{ /* Check to see if it is a delimited variable */
					if (FFV_VARIABLE(v_list)->start_pos == 0 && !IS_HEADER(FFV_VARIABLE(v_list)))
					{
						dbin->state.input_delimited = 1;
						break;
					}
					v_list = dll_next(v_list);
				}

				/* Set records_in_cache for new input format length */
				dbin->bytes_to_read = FORMAT_LENGTH(dbin->input_format);
				dbin->records_in_cache = dbin->cache_size / dbin->bytes_to_read; 
			}

#ifdef XVT
			/*
			Determine if input format needs to be dynamically changed.  This is
			done under the following circumstances:
			1) We are running under GeoVu (e.g., newform, checkvar)
			2) Data file is an image, as determined by value of data_type
			3) Data file is binary.
			4) Data file is not a .pcx file
				
			This is also done in the FORMAT_LIST event.  This code duplicates in
			small part what is done in set_image_limit().
			*/

			if (dbin->input_format)
			{
				if (do_change_input_img_format(dbin))
					return(ERR_MEM_LACK);
				else
					break;
			}
			
#endif /* XVT */

			if (error)
				return(error);

			break; /* End of INPUT_FORMAT */

		/*
		 * MESSAGE:     DBIN_INDEX
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *file_name
		 *				If file name is NULL, use dbin file name to create
		 *				index from file_name.ind.
		 *
		 * PRECONDITIONS:       dbin->file_name must be defined for default to work.
		 *
		 * CHANGES IN STATE:    ALLOCATES MEMORY FOR TEMPORARY FILE NAME
		 *
		 * DESCRIPTION: Defines an index for a data bin.
		 *				The index is created by a call to db_make_index().
		 *
		 * ERRORS:      Lack of memory for temporary file name
		 *                      Problems making index
		 *
		 * AUTHOR:      LPD, TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_INDEX"
		 
		case DBIN_INDEX:        /* Argument: char *index_name */
			file_name = va_arg (args, char *);
			dflt=0;
			if(file_name == NULL){  /* Use Default */
				dflt = 1;
				file_name = (char *)memMalloc(strlen(dbin->file_name) + 5, "DBIN_INDEX:file_name");
				if (!file_name)
					return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "file_name in case DBIN_INDEX"));

				os_path_put_parts(file_name, NULL, dbin->file_name, "ind");
			}
			if(dbin->index)
				db_free_index(dbin->index);
			if(endian() == (int)dbin->state.byte_order)
				dbin->index = db_make_index(file_name, dbin->buffer, 0, (long)dbin->input_format);
			else
				dbin->index = db_make_index(file_name, dbin->buffer, 1, (long)dbin->input_format);

			if(dflt)
				memFree(file_name, "DBIN_INDEX:file_name");

			if(!dbin->index)
				return(err_push(MESSAGE_NAME, ERR_MAKE_INDEX, "case DBIN_INDEX"));

			break;

		/*
		 * MESSAGE:     MAKE_NAME_TABLE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *file_name
		 *				If file name is NULL, use dbin file name to create
		 *				index from file_name.eqv.
		 *
		 * PRECONDITIONS:       dbin->buffer must be defined
		 *
		 * CHANGES IN STATE:    Name table is created
		 *
		 * DESCRIPTION: Defines a name table for a data bin from a file.
		 *
		 * ERRORS:      Lack of memory for temporary file name
		 *                      Problems making name table (in nt_create)
		 *
		 * AUTHOR:      LD, NGDC,(303)497-6472, lpd@kryton.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set MAKE_NAME_TABLE"
		 
		case MAKE_NAME_TABLE:   /* Argument: char *file_name */

			file_name = va_arg (args, char *);

			old_table = new_table = NULL;

			if (file_name && os_file_exist(file_name))
			{
#ifndef DO_NOT_MERGE_NAME_TABLES_HERE_IN_SETDBIN_YES_RIGHT_HERE
				if ((old_table = nt_get_name_table(dbin->table_list, FFF_INPUT)) != NULL)
					old_table = nt_remove_name_table(&(dbin->table_list), old_table);
				
				if ((new_table = nt_create(file_name, NULL)) == NULL)
					return(err_push(ROUTINE_NAME, ERR_NT_DEFINE, file_name));
				
				if (nt_put_name_table(&(dbin->table_list), FFF_INPUT, new_table))
					return(err_push(ROUTINE_NAME, ERR_NT_MERGE/*ADD*/, "adding name table to data bin"));
			
#else /* this would seem to be the preferred code -- substitute when ready */
				if ((new_table = nt_create(file_name, NULL)) == NULL)
					return(err_push(ROUTINE_NAME, ERR_NT_DEFINE, file_name));
        
        old_table = nt_get_name_table(dbin->table_list, FFF_INPUT);
				if (old_table && nt_merge(new_table, &old_table))
					return(err_push(ROUTINE_NAME, ERR_NT_MERGE, file_name));
				
				nt_free_name_table(new_table);
				
#endif
				if (!new_table)
				{
					if (err_state())
					{
						if (dbs_error_verbose_on)
						{
#ifdef XVT
							xvt_dm_post_error("%s\n\nError creating name table from this file.", file_name);
#else
							err_push(ROUTINE_NAME, ERR_NAME_TABLE, file_name);
#endif
							err_disp();
						}
						else
							err_clear();
					}
				}

				db_set(dbin, DBIN_BYTE_ORDER, END_ARGS);
				break;
			} /* if file_name (entrant to this event) */

/* If execution reaches this point there was no given file name */

/* file_name variable will be reused */
			   
			if ((file_name = (char *)memMalloc(_MAX_PATH, "MAKE_NAME_TABLE:file_name")) == NULL)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "file_name in MAKE_NAME_TABLE"));

#ifdef XVT

			/* create name_table from a menu _eqv section */
			if (dbin->mindex != NULL)
			{
				ch_ptr = NULL;
				if(!mn_index_get_offset(dbin->mindex, "default_eqv", &rowsize))
				{
					if(mn_section_get(dbin->mindex, NULL, &rowsize, &ch_ptr))
						return(err_push(MESSAGE_NAME, ERR_MENU, "Retrieving default_eqv section"));
					/* Create the name table */
					if((old_table = nt_create(NULL, ch_ptr)) == NULL)
						return(err_push(MESSAGE_NAME, ERR_NT_DEFINE, "from buffer"));
					/* We have created a name table */
					free(ch_ptr);
				}
				
				/* Find directory-specific eqv section */
				
/*  Insert UNIX and Mac band-aid -- menu section titles may contain non-native
   paths (using MENU_DIR_SEPARATOR aka DOS_DIR_SEPARATOR) but dbin->file_name
   (should) be a native path -- make a copy of dbin->file_name and translate
   back to a DOS native path
*/
#ifdef CCLSC
				if (os_path_is_native(dbin->file_name))
					os_str_replace_char(dbin->file_name, MAC_DIR_SEPARATOR, MENU_DIR_SEPARATOR);
#endif          

#ifdef SUNCC
				if (os_path_is_native(dbin->file_name))
					os_str_replace_char(dbin->file_name, UNIX_DIR_SEPARATOR, MENU_DIR_SEPARATOR);
#endif	

				position = NULL;
				rowsize.num_bytes = 0;
				rowsize.start = 0;
				while(1)
				{
					result = mn_index_find_title(dbin->mindex, "_eqv", &rowsize, &position);
					if(result)
					{
						if(result == ERR_MN_SEC_NFOUND)
							break;
						err_push(MESSAGE_NAME, ERR_MENU, "Finding _eqv sections");
					}
					/* See if this title and the file name match */
					if(eval_eqv_title(position, dbin->file_name) == TRUE)
					{
						/* They do match- get the section */
						/* Find its offset */
						if(mn_index_get_offset(dbin->mindex, position, &rowsize))
							/* We already know the section exists- there should never be
							 * an error retrieving its offset and size, but check anyway... */
							return(err_push(MESSAGE_NAME, ERR_MENU, "Retrieving _eqv section"));

						ch_ptr = NULL;
						if(mn_section_get(dbin->mindex, NULL, &rowsize, &ch_ptr))
							return(err_push(MESSAGE_NAME, ERR_MENU, "Retrieving default_eqv section"));
					     
					    /* Create the name table */
						if((new_table = nt_create(NULL, ch_ptr)) == NULL)
							return(err_push(MESSAGE_NAME, ERR_NT_DEFINE, "from buffer"));
						
						free(ch_ptr); /* Free the section */
                        
            /* We want to merge the newly created table, new_table, into
             * old_table, replacing any previously defined things in
             * old_table with the newly defined things in new_table */
						if(old_table && nt_merge(new_table, &old_table))
							/* Error merging tables */
							return(err_push(MESSAGE_NAME, ERR_NT_MERGE, "merging name tables"));
						break;
						/* No need to search further... */
					}
				} /* Done looking for directory-specific eqv section */

#ifdef CCLSC
				if (!os_path_is_native(dbin->file_name))
					(void)os_path_make_native(dbin->file_name, dbin->file_name);
#endif

#ifdef SUNCC
				if (!os_path_is_native(dbin->file_name))
					(void)os_path_make_native(dbin->file_name, dbin->file_name);
#endif


				/* find the specific equivalence table for the file */
				memStrcpy(file_name, dbin->file_name,"MAKE_NAME_TABLE:file_name,dbin->file_name");
				/* delete the cd driver */
				if (!os_strncmpi(file_name, cd_device, strlen(cd_device)))
					memStrcpy(file_name, file_name+strlen(cd_device),"MAKE_NAME_TABLE:file_name,file_name+strlen(cd_device)");

				point = os_path_return_ext(file_name);
				if (point)
					*--point = STR_END;
				memStrcat(file_name, "_eqv","MAKE_NAME_TABLE:file_name,\"_eqv\"");
				
				ch_ptr = NULL;
				if(!mn_index_get_offset(dbin->mindex, file_name, &rowsize))
				{
					if(mn_section_get(dbin->mindex, NULL, &rowsize, &ch_ptr))
						return(err_push(MESSAGE_NAME, ERR_MENU, "Retrieving _eqv section"));
					/* Create the name table */
					new_table = nt_create(NULL, ch_ptr);
					if (!new_table)
						return(err_push(MESSAGE_NAME, ERR_NT_DEFINE, "Creating name table"));
					/* We have created a name table */
					free(ch_ptr);
					
/* We want to merge the newly created table, new_table, into
* old_table, replacing any previously defined things in
* old_table with the newly defined things in new_table */
					if(old_table && nt_merge(new_table, &old_table))
							/* Error merging tables */
						return(err_push(MESSAGE_NAME, ERR_NT_MERGE, "merging name tables"));
				}
				if (old_table)
					nt_put_name_table(&(dbin->table_list), FFF_INPUT, old_table);
					db_set(dbin, DBIN_BYTE_ORDER, END_ARGS);
					break;
			} /* if no (?) prior name table and there is a menu */

#endif /* XVT */
			
/* If execution reaches this point no name table has yet been created */

/* Try data file name variations */

			os_path_find_parts(dbin->file_name, NULL, &point, NULL);

			if (point)
			{
				if ((env_dir = os_get_env("FORMAT_DIR")) != NULL)
				{
					/* Try data filename.eqv in FORMAT_DIR */
					os_path_put_parts(file_name, env_dir, point, "eqv");
					if (!os_file_exist(file_name))
						/* Try data filename.eqv in default directory (FORMAT_DIR failed)*/
						os_path_put_parts(file_name, NULL, point, "eqv");
				}
	      else
					/* Try data filename.eqv in default directory (no FORMAT_DIR) */
					os_path_put_parts(file_name, NULL, point, "eqv");
	
				if (!os_file_exist(file_name))
					/* Try data filename.eqv in file's home directory */
					os_path_put_parts(file_name, NULL, dbin->file_name, "eqv");
			} /* end data file name variations */

/* try data file ext.eqv variations */

			if (!os_file_exist(file_name))
			{
				point = os_path_return_ext(dbin->file_name);
	
				if (point)
				{
					if ((env_dir = os_get_env("FORMAT_DIR")) != NULL)
					{
						/* Try ext.eqv in FORMAT_DIR */
						os_path_put_parts(file_name, env_dir, point, "eqv");
						if (!os_file_exist(file_name))
							/* Try ext.eqv in default directory (FORMAT_DIR failed)*/
							os_path_put_parts(file_name, NULL, point, "eqv");
					}
		      else
						/* Try ext.eqv in default directory (no FORMAT_DIR) */
						os_path_put_parts(file_name, NULL, point, "eqv");
		
					if (!os_file_exist(file_name))
						/* Try ext.eqv in file's home directory */
						os_path_put_parts(file_name, NULL, dbin->file_name, "eqv");
				} /* data file name has an extension */
			} /* if no file found under data file name variations */
			
			if (os_file_exist(file_name))
			{
#ifndef DO_NOT_MERGE_NAME_TABLES_HERE_IN_SETDBIN_YES_RIGHT_HERE
			if ((old_table = nt_get_name_table(dbin->table_list, FFF_INPUT)) != NULL)
				old_table = nt_remove_name_table(&(dbin->table_list), old_table);
				
			if ((new_table = nt_create(file_name, NULL)) == NULL)
				return(err_push(ROUTINE_NAME, ERR_NT_DEFINE, file_name));
				
			if (nt_put_name_table(&(dbin->table_list), FFF_INPUT, new_table))
				return(err_push(ROUTINE_NAME, ERR_NT_MERGE/*ADD*/, "adding name table to data bin"));
#else /* this would seem to be the preferred code -- substitute when ready */
				if ((new_table = nt_create(file_name, NULL)) == NULL)
					return(err_push(ROUTINE_NAME, ERR_NT_DEFINE, file_name));
						
        old_table = nt_get_name_table(dbin->table_list, FFF_INPUT);
				if (old_table && nt_merge(new_table, &old_table))
					return(err_push(ROUTINE_NAME, ERR_NT_MERGE, file_name));
					
				nt_free_name_table(new_table);
					
#endif
			} /* if constructed file name exists */

/* MAO:c if no table is NULL this is probably an error that should be
         treated -- "old" SDB MENU code */

			memFree(file_name, "MAKE_NAME_TABLE:file_name");
			
			break;
            
 		/*
		 * MESSAGE:     DBIN_BYTE_ORDER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:	In order to get the user-defined data_byte_order value,
		 * the MAKE_NAME_TABLE and DEFINE_HEADER events be called, unless
		 * data_byte_order is defined in the environment.
		 *
		 * CHANGES IN STATE:    state.byte_order
		 *                                              
		 * DESCRIPTION:  Sets the data bin's byte_order state vector according
		 * to the user-defined value of "data_byte_order".
		 *
		 * Calls nt_askvalue to search header, name table, and environment for
		 * data_byte_order.  If found, the dbin byte_order state vector is set to
		 * 0 for a value of "little_endian", 1 for a value of "big_endian", or
		 * return with error if the value of data_byte_order is neither.
		 *
		 * ERRORS:                   
		 *
		 * AUTHOR: Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set DBIN_BYTE_ORDER"
		 
		case DBIN_BYTE_ORDER:
			if (nt_askvalue(dbin, "data_byte_order", FFV_CHAR, temp, NULL))
			{
				if (!os_strcmpi(temp, "little_endian"))
					dbin->state.byte_order = 0;
				else if (!os_strcmpi(temp, "big_endian"))
					dbin->state.byte_order = 1;
				else
					return(err_push(MESSAGE_NAME, ERR_PARAM_VALUE, temp));
			}
		break;
				
		/*
		 * MESSAGE:     OUTPUT_FORMAT
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET)char *file_name, char *format_buffer
		 *				If file name and buffer are NULL, use dbin file name to create
		 *				format from file_name.afm.
		 *              Revision by KBF, 3/29/95: if file_name AND format_buffer are
		 *              both defined, format_buffer is assumed to be the title of the output format
		 *              to be used.  If file_name is a ".fmt" file, and is different from the 
		 *              input format file, the contents of file_name are appended to the
		 *              dbin->format_list, with all formats specifically designated to be input
		 *              removed.  The argument FORMAT_BUFFER_IS_A_TITLE can be sent in 
		 *				as the file name if a format title, but not a format file,
		 *				was specified by the user.  The FORMAT_BUFFER_IS_A_TITLE argument
		 *				is essentially treated as if no filename were sent in.
		 *
		 * PRECONDITIONS:       dbin->file_name must be defined for default to work.
		 *
		 * CHANGES IN STATE:    ALLOCATES MEMORY FOR TEMPORARY FILE NAME
		 *                
		 * DESCRIPTION: Defines an output format for a data bin.  The output
		 * format search parallels the search for an input format.
		 * See the DESCRIPTION for INPUT_FORMAT for more information.
		 *
		 * ERRORS:      Lack of memory for temporary file name
		 *                      Problems making input format
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:
		 * MAO: I am uncertain that appropriate extensions are being looked for.
		 *
		 * KEYWORDS:    
		 *
		 */
		 
#undef MESSAGE_NAME
#define MESSAGE_NAME "db_set OUTPUT_FORMAT"
		 
		case OUTPUT_FORMAT: /* Arguments: filename, buffer */
			file_name = va_arg (args, char *);
			format_buffer = va_arg (args, char *);

			title_specified = NULL;
            
			if (file_name && format_buffer)
			{ /* title is specified in format_buffer */
				if (!strcmp(file_name, FORMAT_BUFFER_IS_A_TITLE))
					file_name = NULL;

				title_specified = format_buffer;
				format_buffer = NULL;
			}

			error = make_format_eqv_list(FFF_OUTPUT | FFF_DATA,
			                             file_name,
			                             format_buffer,
			                             &f_list,
			                             &(dbin->table_list),
			                             dbin
			                            );
			if (error)
				return(error);
			
			if (dbin->state.std_input)
				/* Remove all separate formats from dbin->format_list */
				(void)db_delete_all_formats(f_list, FFF_SEPARATE);
				
			/* Read/write ambiguous (file?) header formats must be marked
			   unconditionally as FFF_INPUT in the OUTPUT_FORMAT event.
			   db_format_list_io() marks ambiguous data formats according to
			   data file extension, but this is not appropriate (currently...)
			   for header formats.  The assumption here is that unless the user
			   specifically lists input and output file header formats, conversion
			   of file headers is not desired.  This also insures that ambiguous
			   file header formats do not create two instances in
			   dbin->format_list, as ambigous formats get merged into the format
			   list in both the INPUT_FORMAT and OUTPUT_FORMAT events.
	
			   This is worth elaboration.  The header format and the input format
			   are assigned to the data bin in the INPUT_FORMAT event, but when
			   formats are ambiguous in BOTH the INPUT_FORMAT and
			   OUTPUT_FORMAT event, and when the format lists are merged in the
			   OUTPUT_FORMAT event, ambiguous formats are replaced.  Thus
			   dbin->input_format and dbin->output_format are free()'d w/o
			   being replaced!
			 */
	
			while ((format = db_find_format_is_isnot(f_list, FFF_GROUP, FFF_HD,
			                 FFF_INPUT | FFF_OUTPUT, END_ARGS)) != NULL)
				format->type |= FFF_INPUT;
							 
			if (title_specified)
			{
				/* there will be only one surviving output data format --
				   ensure that the user-designated format survives by marking
				   all data formats as FFF_INPUT, and then marking title-
				   selected format as FFF_OUTPUT.  All FFF_INPUT formats will
				   be deleted from f_list prior to merging with
				   dbin->format_list */
							
				while ((format = db_find_format_is_isnot(f_list, FFF_GROUP,
				                 FFF_DATA, FFF_INPUT, END_ARGS)) != NULL)
				{
					format->type &= ~FFF_OUTPUT;
					format->type |= FFF_INPUT;
				}
							
				format = db_find_format(f_list, FFF_NAME, title_specified, END_ARGS);
				if (format)
				{
					format->type &= ~FFF_INPUT;
					format->type |= FFF_OUTPUT;
				}
				else
					return(err_push(ROUTINE_NAME, ERR_FIND_FORM, title_specified));
			} /* if title_specified */
			else
			{
				format = db_find_format(f_list, FFF_GROUP, FFF_OUTPUT | FFF_DATA, END_ARGS);
				db_format_list_mark_io(f_list, dbin->file_name);
	
				if (!format)
					format = db_find_format(f_list, FFF_GROUP, FFF_OUTPUT | FFF_DATA, END_ARGS);
							
				if (format)
					db_delete_rivals(f_list, format);
			}
		
			if (format)
				dbin->output_format = format;
			else
				error = ERR_MAKE_FORM;
			
			/* Remove any input formats from the list */
			(void)db_delete_all_formats(f_list, FFF_INPUT);
			/* This is done to avoid confusion if the output format file
			 * contained input formats, but a seperate input format
			 * file was defined. */
			
			/* And merge the list we have into dbin->format_list */

			dbin->format_list = db_merge_format_lists(dbin->format_list, f_list);
			if (!dbin->format_list)
				return(ERR_MAKE_FORM);

			/* The db_merge_format_list function frees f_list */
			f_list = NULL;
			format = NULL;
			
			/* End checking for output format. */
			
			if (error)
				return(error);

			break; /* End of OUTPUT_FORMAT */

		default:
			sprintf(dbin->buffer, "%s, %s:%d",
			        ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, dbin->buffer);
			err_disp();
			return(4);

		}       /* End of Attribute Switch */
	}       /* End of Attribute Processing (while) */
	va_end(args);
	return(0);
}

