#define WANT_NCSA_TYPES
#include <freeform.h>
#include <maxmin.h>

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "mm_make"

/*
 * NAME:	mm_make
 *		
 * PURPOSE:	To create a MAX_MIN structure
 *
 * USAGE:	mm_make(VARIABLE_LIST_PTR)
 *
 * RETURNS:	MAX_MIN_PTR  (NULL on error)	
 *
 * DESCRIPTION: Create and initialize a MAX_MIN Structure.
 *				
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	 Ted Habermann, NGDC, (303)497-6221, haber@mail.ngdc.noaa.gov
 *		 Mark VanGorp, mm_make change, mvg@mail.ngdc.noaa.gov
 *
 * 			Rich Fozzard	12/2/94		added void * cast for ThinkC compiler -rf01
 *
 * COMMENTS: 
 *
 * KEYWORDS:	
 *
 */

MAX_MIN_PTR mm_make(VARIABLE_PTR var, unsigned short type)
{

	MAX_MIN_PTR max_min;
	
	max_min = (MAX_MIN_PTR)memCalloc(1, sizeof(MAX_MIN), "mm_make: max_min");
	if (!max_min)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Calloc maxmin struct");
		return((MAX_MIN_PTR)NULL);
	}
	/* A copy of the variable is made even if the address of an existing
           var is sent: If this were not done, mm_free could not be called if
	   the variable still existed in a variable list
	*/
	max_min->var = (VARIABLE_PTR)memMalloc(sizeof(VARIABLE), "mm_make: max_min->var");
	if (!max_min->var)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Malloc variable struct");
		return((MAX_MIN_PTR)NULL);
	}
	
	if (!var) 
		max_min->var->type = type;
	else 
		memMemcpy((void *)max_min->var, (void *)var, sizeof(VARIABLE),NO_TAG);

	max_min->var->check_address = max_min->var;

	/* Switch on the possible types */

	if (IS_TEXT(max_min->var))
	{
		max_min->minimum = (void *)memCalloc(1, (size_t)(FF_VAR_LENGTH(max_min->var) + 1), "max_min->minimum");
		max_min->maximum = (void *)memCalloc(1, (size_t)(FF_VAR_LENGTH(max_min->var) + 1), "max_min->maximum");
		if (!max_min->maximum || !max_min->minimum)
		{
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "Setting missing data");
			return(NULL);
		}

		*(char *)max_min->minimum = CHAR_MAX;
	}
	else
	{
		size_t byte_size;
		
		byte_size = ffv_type_size(FFV_DATA_TYPE(max_min->var));
		if (byte_size)
		{
			max_min->minimum = (void *)memCalloc(1, byte_size, "max_min->minimum");
			max_min->maximum = (void *)memCalloc(1, byte_size, "max_min->maximum");
			if (!max_min->maximum || !max_min->minimum)
			{
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "Setting missing data");
				return(NULL);
			}
		}
		else
			assert(byte_size);
	
		switch (FFV_DATA_TYPE(max_min->var))
		{
			char error_buffer[_MAX_PATH];
			
			case FFV_INT8:
				*(int8 *)max_min->minimum = FFV_INT8_MAX;
				*(int8 *)max_min->maximum = FFV_INT8_MIN;
			break;

			case FFV_UINT8:
				*(uint8 *)max_min->minimum = FFV_UINT8_MAX;
				*(uint8 *)max_min->maximum = FFV_UINT8_MIN;
			break;

			case FFV_INT16:
				*(int16 *)max_min->minimum = FFV_INT16_MAX;
				*(int16 *)max_min->maximum = FFV_INT16_MIN;
			break;

			case FFV_UINT16:
				*(uint16 *)max_min->minimum = FFV_UINT16_MAX;
				*(uint16 *)max_min->maximum = FFV_UINT16_MIN;
			break;

			case FFV_INT32:
				*(int32 *)max_min->minimum = FFV_INT32_MAX;
				*(int32 *)max_min->maximum = FFV_INT32_MIN;
			break;

			case FFV_UINT32:
				*(uint32 *)max_min->minimum = FFV_UINT32_MAX;
				*(uint32 *)max_min->maximum = FFV_UINT32_MIN;
			break;

			case FFV_INT64:
				*(int64 *)max_min->minimum = FFV_INT64_MAX;
				*(int64 *)max_min->maximum = FFV_INT64_MIN;
			break;

			case FFV_UINT64:
				*(uint64 *)max_min->minimum = FFV_UINT64_MAX;
				*(uint64 *)max_min->maximum = FFV_UINT64_MIN;
			break;

			case FFV_FLOAT32:
				*(float32 *)max_min->minimum = FFV_FLOAT32_MAX;
				*(float32 *)max_min->maximum = FFV_FLOAT32_MIN;
			break;

			case FFV_FLOAT64:
				*(float64 *)max_min->minimum = FFV_FLOAT64_MAX;
				*(float64 *)max_min->maximum = FFV_FLOAT64_MIN;
			break;

			default:
				sprintf(error_buffer, "%d, %s:%d",
				        (int)FFV_DATA_TYPE(max_min->var),
				        os_path_return_name(__FILE__), __LINE__);
				err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
				err_disp();
				return(NULL);
		} /* switch on max_min->var type */
	} /* (else) if max_min->var is a string */

	max_min->max_record = max_min->min_record = 0L;
	max_min->check_address = max_min;

	return(max_min);
}

/*
 * NAME:	mm_set
 *		
 * PURPOSE:	Set the attributes and execute the methods of max mins.
 *
 * USAGE:	mm_set(MAX_MIN_PTR, attribute, [args], attribute, args,... NULL)
 *
 * RETURNS:	0 if all goes well.
 *
 * DESCRIPTION:	This function provides access to the attributes and methods
 *		of MAX_MINs. The function processes a list of argument
 *		groups which have the form: attribute [arguments]. The list
 *		of groups is terminated by a NULL, which ends processing.
 *		The presently supported attributes and their arguments are:
 *
 *			case MM_MAX_MIN:		* Argument:  char *data     *
 *			case MM_MISSING_DATA_FLAGS:	* Arguments: void *max_flag *
 *			                        	*            void *min_flag *
 *
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	Mark VanGorp, (303)497-6221, NGDC, mvg@mail.ngdc.noaa.gov
 *
 * 			Rich Fozzard	12/2/94		added void * cast for ThinkC compiler -rf01
 *
 * COMMENTS: Patterned after db_set
 *
 *				   
 * KEYWORDS: max_mins
 *
*/

#undef ROUTINE_NAME
#define ROUTINE_NAME "mm_set"

int mm_set(MAX_MIN_PTR max_min, ...)
{
	va_list	args;

	char	*data;

	double 	d_var = 0.0;
	double 	*d_ptr = &d_var;

	int attribute;
	int var_length;

	long record_number;
	short bytes_to_skip = 0;

	void *max_flag;
	void *min_flag;
	
	char error_buffer[_MAX_PATH];

	if (!max_min)
	{
		va_end (args);
		err_push(ROUTINE_NAME, ERR_PTR_DEF, "max_min");
		return (1);
	}

	if (max_min != max_min->check_address)
	{
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "max_min");
		return(1);
	}	

	va_start(args, max_min);

	while ( (attribute = va_arg (args, int)) != 0 )
	{
		/* Check For Possible Memory Corruption */
		if (max_min != max_min->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "dbin");
			return(1);
		}	

		switch (attribute)
		{
		
		/*
		 * MESSAGE: 	MM_MAX_MIN
		 *
		 * OBJECT TYPE: MAX_MIN
		 *
		 * ARGUMENTS: 	char *data
		 *
		 * PRECONDITIONS:
		 *
		 * CHANGES IN STATE:
		 *
		 * DESCRIPTION: COMPARE_MAX_MIN compares the value pointed to by data
		 *		with the MAX_MIN structure and replaces the appropriate
		 *		members of that structure if necessary
		 *
		 * ERRORS:	unknown variable type
		 *
		 * AUTHOR:	Ted Habermann, NGDC, (303)497-6221, haber@mail.ngdc.noaa.gov
		 *		Mark VanGorp, NGDC, (303)494-6221, mvg@mail.ngdc.noaa.gov
		 *			-- Pre missing data flag checking			 
		 *
		 * COMMENTS:
		 *
		 *
		 * KEYWORDS:	
		 *
		 */
		case MM_MAX_MIN :	/* Argument: char* */
			data = va_arg (args, char *);
			record_number = va_arg(args, long);

			if (!IS_TEXT(max_min->var))
			{ /* align data into d_ptr */
				int error;
				
				error = btype_to_btype((void *)data, FFV_DATA_TYPE(max_min->var),
				                       (void *)d_ptr, FFV_DATA_TYPE(max_min->var));
				if (error)
					return(error);
			}
			
			switch (FFV_DATA_TYPE(max_min->var))
			{
				case FFV_TEXT:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					var_length = FF_VAR_LENGTH(max_min->var);
					
					if (max_min->max_flag &&
						  strncmp(data, (char *)max_min->min_flag, (size_t)var_length) >= 0 &&
							strncmp(data, (char *)max_min->max_flag, (size_t)var_length) >= 0
					   )
						break;
	
					if (strncmp(data, (char *)max_min->maximum, (size_t)var_length) > 0)
					{
						memStrncpy((char *)max_min->maximum, data, (size_t)var_length,NO_TAG);
						max_min->max_record = record_number;
					}
										
					if (strncmp(data, (char *)max_min->minimum, (size_t)var_length) < 0)
					{
						memStrncpy((char *)max_min->minimum, data, (size_t)var_length,NO_TAG);
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_INT8:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(int8 *)d_ptr >= *(int8 *)max_min->min_flag &&
							*(int8 *)d_ptr <= *(int8 *)max_min->max_flag
						 )
						break;
	
					if (*(int8 *)d_ptr > *(int8 *)max_min->maximum)
					{
						*(int8 *)max_min->maximum = *(int8 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(int8 *)d_ptr < *(int8 *)max_min->minimum)
					{
						*(int8 *)max_min->minimum = *(int8 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_UINT8:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(uint8 *)d_ptr >= *(uint8 *)max_min->min_flag &&
							*(uint8 *)d_ptr <= *(uint8 *)max_min->max_flag
						 )
						break;
	
					if (*(uint8 *)d_ptr > *(uint8 *)max_min->maximum)
					{
						*(uint8 *)max_min->maximum = *(uint8 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(uint8 *)d_ptr < *(uint8 *)max_min->minimum)
					{
						*(uint8 *)max_min->minimum = *(uint8 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_INT16:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(int16 *)d_ptr >= *(int16 *)max_min->min_flag &&
							*(int16 *)d_ptr <= *(int16 *)max_min->max_flag
						 )
						break;
	
					if (*(int16 *)d_ptr > *(int16 *)max_min->maximum)
					{
						*(int16 *)max_min->maximum = *(int16 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(int16 *)d_ptr < *(int16 *)max_min->minimum)
					{
						*(int16 *)max_min->minimum = *(int16 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_UINT16:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(uint16 *)d_ptr >= *(uint16 *)max_min->min_flag &&
							*(uint16 *)d_ptr <= *(uint16 *)max_min->max_flag
						 )
						break;
	
					if (*(uint16 *)d_ptr > *(uint16 *)max_min->maximum)
					{
						*(uint16 *)max_min->maximum = *(uint16 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(uint16 *)d_ptr < *(uint16 *)max_min->minimum)
					{
						*(uint16 *)max_min->minimum = *(uint16 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_INT32:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(int32 *)d_ptr >= *(int32 *)max_min->min_flag &&
							*(int32 *)d_ptr <= *(int32 *)max_min->max_flag
						 )
						break;
	
					if (*(int32 *)d_ptr > *(int32 *)max_min->maximum)
					{
						*(int32 *)max_min->maximum = *(int32 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(int32 *)d_ptr < *(int32 *)max_min->minimum)
					{
						*(int32 *)max_min->minimum = *(int32 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_UINT32:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(uint32 *)d_ptr >= *(uint32 *)max_min->min_flag &&
							*(uint32 *)d_ptr <= *(uint32 *)max_min->max_flag
						 )
						break;
	
					if (*(uint32 *)d_ptr > *(uint32 *)max_min->maximum)
					{
						*(uint32 *)max_min->maximum = *(uint32 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(uint32 *)d_ptr < *(uint32 *)max_min->minimum)
					{
						*(uint32 *)max_min->minimum = *(uint32 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_INT64:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(int64 *)d_ptr >= *(int64 *)max_min->min_flag &&
							*(int64 *)d_ptr <= *(int64 *)max_min->max_flag
						 )
						break;
	
					if (*(int64 *)d_ptr > *(int64 *)max_min->maximum)
					{
						*(int64 *)max_min->maximum = *(int64 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(int64 *)d_ptr < *(int64 *)max_min->minimum)
					{
						*(int64 *)max_min->minimum = *(int64 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_UINT64:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(uint64 *)d_ptr >= *(uint64 *)max_min->min_flag &&
							*(uint64 *)d_ptr <= *(uint64 *)max_min->max_flag
						 )
						break;
	
					if (*(uint64 *)d_ptr > *(uint64 *)max_min->maximum)
					{
						*(uint64 *)max_min->maximum = *(uint64 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(uint64 *)d_ptr < *(uint64 *)max_min->minimum)
					{
						*(uint64 *)max_min->minimum = *(uint64 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_FLOAT32:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(float32 *)d_ptr >= *(float32 *)max_min->min_flag &&
							*(float32 *)d_ptr <= *(float32 *)max_min->max_flag
						 )
						break;
	
					if (*(float32 *)d_ptr > *(float32 *)max_min->maximum)
					{
						*(float32 *)max_min->maximum = *(float32 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(float32 *)d_ptr < *(float32 *)max_min->minimum)
					{
						*(float32 *)max_min->minimum = *(float32 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				case FFV_FLOAT64:
					/* If missing data flags are set and data value lies outside
					   the missing data range, then do the max min checking */
					if (max_min->max_flag && 
						  *(float64 *)d_ptr >= *(float64 *)max_min->min_flag &&
							*(float64 *)d_ptr <= *(float64 *)max_min->max_flag
						 )
						break;
	
					if (*(float64 *)d_ptr > *(float64 *)max_min->maximum)
					{
						*(float64 *)max_min->maximum = *(float64 *)d_ptr;
						max_min->max_record = record_number;
					}
	
					if (*(float64 *)d_ptr < *(float64 *)max_min->minimum)
					{
						*(float64 *)max_min->minimum = *(float64 *)d_ptr;
						max_min->min_record = record_number;
					}
				break;
		
				default:
					sprintf(error_buffer, "%d, %s:%d",
					        (int)FFV_DATA_TYPE(max_min->var),
					        os_path_return_name(__FILE__), __LINE__);
					err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
					err_disp();
					return(1);
			}
		break;

		/*
		 * MESSAGE: 	MM_MISSING_DATA_FLAGS
		 *
		 * OBJECT TYPE: MAX_MIN
		 *
		 * ARGUMENTS: 	void *max_flag void *min_flag
		 *
		 * PRECONDITIONS:
		 *
		 * CHANGES IN STATE:
		 *
		 * DESCRIPTION: MISSING_DATA_FLAGS is used to set the _flag void ptrs in the
		 *		MAX_MIN structure to a given max and min range. Any value
		 *             	lying inclusively between the values of min_flag and max_flag
		 *		will be treated as a missing data value and will subsequently not
		 *		be included in determining the max and min for that variable
		 *
		 * ERRORS: No memory for setting the structure flags
		 *
		 * AUTHOR: Mark VanGorp, NGDC, (303)497-6221, mvg@mail.ngdc.noaa.gov
		 *
		 * COMMENTS: When setting a missing data flag for a char, both max_flag
		 *           and min_flag must be NULL terminated	
		 *
		 * KEYWORDS:	
		 *
		 */
		case MM_MISSING_DATA_FLAGS:	/* Argument: void *min_flag, void *max_flag */

			max_flag = va_arg (args, void *);
			min_flag = va_arg (args, void *);

			if (IS_TEXT(max_min->var))
			{
				/* The max_flag and min_flag must be NULL terminated void
				   ptrs. This is necessary to right justify the strings */
				var_length = FF_VAR_LENGTH(max_min->var);

				max_min->max_flag = (void *)memCalloc(1, (size_t)(var_length+1), "max_min->max_flag");
				max_min->min_flag = (void *)memCalloc(1, (size_t)(var_length+1), "max_min->min_flag");
				if (!max_min->max_flag || !max_min->min_flag)
				{
					err_push(ROUTINE_NAME, ERR_MEM_LACK, "Setting missing data");
					return(1);
				}
	
				memMemset(max_min->max_flag, ' ', var_length,"max_min->max_flag,' ',var_length");
				memMemset(max_min->min_flag, ' ', var_length,"max_min->min_flag,' ',var_length");
	
				bytes_to_skip = var_length - strlen((char*)max_flag);
				sprintf(((char*)(max_min->max_flag) + bytes_to_skip),"%s",(char*)max_flag);
	
				bytes_to_skip = var_length - strlen((char*)min_flag);
				sprintf(((char*)(max_min->min_flag) + bytes_to_skip),"%s",(char*)min_flag);
			}
			else if (IS_INTEGER(max_min->var) || IS_REAL(max_min->var))
			{
				size_t byte_size = 0;
				
				byte_size = ffv_type_size(FFV_DATA_TYPE(max_min->var));
				if (byte_size)
				{
					int error;
						
					max_min->max_flag = (void *)memMalloc(byte_size, "max_min->max_flag");
					max_min->min_flag = (void *)memMalloc(byte_size, "max_min->min_flag");
					if (!max_min->max_flag || !max_min->min_flag)
					{
						err_push(ROUTINE_NAME, ERR_MEM_LACK, "Setting missing data");
						return(1);
					}
						
					error = btype_to_btype(max_flag, FFV_DATA_TYPE(max_min->var),
					                       max_min->max_flag, FFV_DATA_TYPE(max_min->var));
					if (error)
						return(error);
						
					error = btype_to_btype(min_flag, FFV_DATA_TYPE(max_min->var),
					                       max_min->min_flag, FFV_DATA_TYPE(max_min->var));
					if (error)
						return(error);
				}
				else
					assert(byte_size); /* should never happen -- would drop into default, below */
			}
			else
				assert(0);

			default:
				sprintf(error_buffer, "%d, %s:%d",
				        (int)attribute,
				        os_path_return_name(__FILE__), __LINE__);
				err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
				err_disp();
				return(1);                              

		}	/* End of Attribute Switch */
	}	/* End of Attribute Processing (while) */
	va_end(args);
	return(0);
}

#ifndef XVT

/*
 * NAME:	mm_print
 *		
 * PURPOSE:	To print the max and mins of a given MAX_MIN structure
 *
 * USAGE:	mm_print(MAX_MIN_PTR)
 *
 * RETURNS:	NULL
 *
 * DESCRIPTION:	mm_free takes a pointer to a MAX_MIN structure and prints to stderr
 *             	the max and min associated with that structure
 *
 * SYSTEM DEPENDENT FUNCTIONS:	Generally mm_make and mm_set
 *
 * AUTHOR:	Ted Habermann, (303) 497-6472, NGDC, haber@mail.ngdc.noaa.gov		
 *        	Mark VanGorp, (303) 497-6221, NGDC, mvg@mail.ngdc.noaa.gov
 *
 * COMMENTS: 
 *				   
 * KEYWORDS: 	max_min
 *
*/
#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "mm_print"

int mm_print(MAX_MIN_PTR max_min)
{
	if (!max_min)
	{
		err_push(ROUTINE_NAME, ERR_PTR_DEF, "max_min");
		return(1);
	}
	/* Switch on the possible types */
	switch (FFV_DATA_TYPE(max_min->var))
	{
		char error_buffer[_MAX_PATH];
		
		case FFV_TEXT:
			printf("Minimum: %s  Maximum: %s\n",
			       *(char *)max_min->minimum,
			       *(char *)max_min->maximum
			      );
		break;

		case FFV_INT8:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_INT8], *(int8 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_INT8], *(int8 *)max_min->maximum);
			printf("\n");
		break;
		
		case FFV_UINT8:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_UINT8], *(uint8 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_UINT8], *(uint8 *)max_min->maximum);
			printf("\n");
		break;

		case FFV_INT16:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_INT16], *(int16 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_INT16], *(int16 *)max_min->maximum);
			printf("\n");
		break;
		
		case FFV_UINT16:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_UINT16], *(uint16 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_UINT16], *(uint16 *)max_min->maximum);
			printf("\n");
		break;

		case FFV_INT32:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_INT32], *(int32 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_INT32], *(int32 *)max_min->maximum);
			printf("\n");
		break;
		
		case FFV_UINT32:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_UINT32], *(uint32 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_UINT32], *(uint32 *)max_min->maximum);
			printf("\n");
		break;

		case FFV_INT64:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_INT64], *(int64 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_INT64], *(int64 *)max_min->maximum);
			printf("\n");
		break;
		
		case FFV_UINT64:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_UINT64], *(uint64 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_UINT64], *(uint64 *)max_min->maximum);
			printf("\n");
		break;

		case FFV_FLOAT32:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_FLOAT32], *(float32 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_FLOAT32], *(float32 *)max_min->maximum);
			printf("\n");
		break;

		case FFV_FLOAT64:
			printf("Minimum: ");
			printf(fft_cnv_flags[FFNT_FLOAT64], *(float64 *)max_min->minimum);
			printf("  Maximum: ");
			printf(fft_cnv_flags[FFNT_FLOAT64], *(float64 *)max_min->maximum);
			printf("\n");
		break;
		
		default:	/* Error, Unknown Input Variable type */
			sprintf(error_buffer, "%d, %s:%d",
			        (int)FFV_DATA_TYPE(max_min->var),
			        os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(1);
	}
	
	return(0);
}

#endif /* (not) XVT */

#undef ROUTINE_NAME
#define ROUTINE_NAME "mm_getmx"

/*
 * NAME:	mm_getmx, mm_getmn
 *		
 * PURPOSE:	Return the max or min respectively from a given MAX_MIN structure
 *
 * USAGE:	mm_getmx(MAX_MIN_PTR), mm_getmn(MAX_MIN_PTR)
 *
 * RETURNS:	The respective maximum or minimum value
 *
 * DESCRIPTION:	These two functions each take a ptr to a MAX_MIN structure
 *             	mm_getmx returns the maximum value associated with that structure
 *             	mm_getmn returns the minimum value associated with that structure
 *
 * SYSTEM DEPENDENT FUNCTIONS:	mm_make and mm_set should generally have been called first
 *
 * AUTHOR:	Ted Habermann, (303) 497-6472, NGDC, haber@mail.ngdc.noaa.gov		
 *        	Mark VanGorp, (303) 497-6221, NGDC, mvg@mail.ngdc.noaa.gov
 *
 * COMMENTS: 	To check for errors pushed on stack, the return value can't be checked
 *          	Use err_state to check for errors pushed by these two functions
 *				   
 * KEYWORDS: 	max_mins
 *
*/

double mm_getmx(MAX_MIN_PTR max_min)
{
	double dbl_var = 0.0;
	int error = 0;
	
	if (!max_min)
	{
		err_push(ROUTINE_NAME, ERR_PTR_DEF, "max_min");
		return((double)1);
	}
	
	if (IS_TEXT(max_min->var))
	{
		/* Return the actual ptr cast to a double */
		return( (double)((long)(max_min->maximum)) );
	}
	
	error = btype_to_btype(max_min->maximum, FFV_DATA_TYPE(max_min->var),
	                       (void *)&dbl_var, FFV_DOUBLE);
	if (error)
		return(1);
	else
		return(dbl_var);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "mm_getmn"

double mm_getmn(MAX_MIN_PTR max_min)
{
	double dbl_var = 0.0;
	int error = 0;
	
	if (!max_min)
	{
		err_push(ROUTINE_NAME, ERR_PTR_DEF, "max_min");
		return((double)1);
	}

	if (IS_TEXT(max_min->var))
	{
		/* Return the actual ptr cast to a double */
		return( (double)((long)(max_min->minimum)) );
	}
	
	error = btype_to_btype(max_min->minimum, FFV_DATA_TYPE(max_min->var),
	                       (void *)&dbl_var, FFV_DOUBLE);
	if (error)
		return(1);
	else
		return(dbl_var);
}


/*
 * NAME:	mm_free
 *		
 * PURPOSE:	free memory associated with a given MAX_MIN structure
 *
 * USAGE:	mm_free(MAX_MIN_PTR)
 *
 * RETURNS:	NULL
 *
 * DESCRIPTION:	mm_free takes a pointer to a MAX_MIN structure and deallocates the memory
 *             	associated with that structure
 *
 * SYSTEM DEPENDENT FUNCTIONS:	(mm_make)
 *
 * AUTHOR:	Ted Habermann, (303) 497-6472, NGDC, haber@mail.ngdc.noaa.gov		
 *        	Mark VanGorp, (303) 497-6221, NGDC, mvg@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *				   
 * KEYWORDS: 	max_min
 *
*/

#undef ROUTINE_NAME
#define ROUTINE_NAME "mm_free"

int mm_free(MAX_MIN_PTR max_min)
{
	if (!max_min)
		return(0);

	if (max_min->maximum)
		memFree(max_min->maximum, "mm_free: max/min");

	if (max_min->minimum)
		memFree(max_min->minimum, "mm_free: max/min");

	if (max_min->max_flag)
		memFree(max_min->max_flag, "mm_free: flag");

	if (max_min->min_flag)
		memFree(max_min->min_flag, "mm_free: flag");

	memFree(max_min->var, "mm_free: Variable");

	memFree(max_min, "mm_free: max_min");

	return(0);
}
