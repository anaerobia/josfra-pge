/* 
 *
 * CONTAINS:	Functions that deal with conversions between different units:
 *
 *				cv_meters_to_feet
 *				cv_feet_to_meters
 *				cv_abs_sign_to_value
 *				cv_multiply_value
 *				cv_units are the defaults:
 *					v_name to v_name_scaled
 *					v_name_abs and v_name_sign to v_name
 *					v_name to v_name_abs and v_name_sign
 *					v_name_ft to v_name_m
 *					v_name_m to v_name_ft
 *					v_name_base and v_name_exp to value
 *					v_name to v_name_base and v_name_exp
 *
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <freeform.h>

/* External Variables for these functions: */
static unsigned 		name_length		= 0;
static FF_DATA_BUFFER 	ch 				= NULL;
static char				*eform_set		= "Ee+ ";
static char 			*last_underscore= NULL;
static char				align[256];

/*
 * NAME:	cv_meters_to_feet
 *
 * PURPOSE:	Convert v_name_m to v_name_ft 
 *		   								  
 * AUTHOR:	T. Habermann (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_meters_to_feet(
 *				VARIABLE_PTR var,		Description of Desired variable
 *				double *converted_value,Pointer to the Converted value
 *				FORMAT_PTR input_format,	Input Format Description 
 *				FF_DATA_BUFFER input_buffer)	Input Buffer 			   			
 *										   			  									
 * COMMENTS:																							
 *
 * RETURNS:	Conversion functions return 0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_meters_to_feet"

int cv_meters_to_feet(
	VARIABLE_PTR 	var,				/* Description of Desired variable */
	double 				*converted_value,	/* Pointer to the Converted value */
	FORMAT_PTR 			input_format,		/* Input Format Description */
	FF_DATA_BUFFER 		input_buffer)		/* Input Buffer */
{
	if (cv_multiply_value(var, converted_value, 3.28084, "_m",
	                      input_format, input_buffer)
	   )
		return(1);
	else
		return(0);
}

/*
 * NAME:	cv_feet_to_meters
 *			
 * PURPOSE: Convert v_name_ft to v_name_m 
 *		   								  
 * AUTHOR:	T. Habermann (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_feet_to_meters(
 *				VARIABLE_PTR var,		Description of Desired variable
 *				double *converted_value,Pointer to the Converted value 	 
 *				FORMAT_PTR input_format,	Input Format Description	   	
 *				FF_DATA_BUFFER input_buffer)	Input Buffer
 *													   			 
 * COMMENTS:																							
 *
 * RETURNS:	Conversion functions return 0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_feet_to_meters"

int cv_feet_to_meters(
	VARIABLE_PTR 	var,				/* Description of Desired variable */
	double 				*converted_value,	/* Pointer to the Converted value */
	FORMAT_PTR 			input_format,		/* Input Format Description */
	FF_DATA_BUFFER 		input_buffer)		/* Input Buffer */
{
	if (cv_multiply_value(var, converted_value, 0.3048, "_ft",
	                      input_format, input_buffer)
	   )
		return(1);
	else
		return(0);
}

#ifdef CV_ABL_SIGN_TO_VALUE_NEVER_CALLED
/*
 * NAME:	cv_abs_sign_to_value	
 *			
 * PURPOSE: convert v_name_abs and v_name_sign  to  v_name
 *		   								  
 * AUTHOR:	T. Habermann (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_abs_sign_to_value(
 *				VARIABLE_PTR var,		Description of Desired variable
 *				double *converted_value,Pointer to the Converted value 	 
 *				FORMAT_PTR input_format,	Input Format Description	   	
 *				FF_DATA_BUFFER input_buffer)	Input Buffer 			  
 *													   			 
 * COMMENTS:																							
 *
 * RETURNS:	Conversion functions return 0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME 
#define ROUTINE_NAME "cv_abs_sign_to_value"

int cv_abs_sign_to_value(VARIABLE_PTR var,	/* Description of Desired variable */
	double *converted_value,					/* Pointer to the Converted value */
	FORMAT_PTR input_format,					/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */
{
	char		v_name[MAX_NAME_LENGTH + 24];	/* Variable Name */

	VARIABLE_PTR var_source;

	name_length = strlen(var->name);

	/* find the value */
	memcpy(v_name, var->name, name_length);
	*(v_name + name_length) = STR_END;

	var_source = ff_find_variable(strcat(v_name,"_abs"), input_format);
	if (!var_source)
		return(0);

	memcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source));
	(void)ff_get_double(var_source, align, converted_value, FFF_TYPE(input_format));

	/* Now Get The Sign */
	*(v_name + name_length) = STR_END;

	var_source = ff_find_variable(strcat(v_name,"_sign"), input_format);
	if (!var_source)
	{
		if (*converted_value <= 0.0)
			*converted_value *= -1.0;
		return(1);
	}

	ch = input_buffer + var_source->start_pos - 1;
	if (*ch == '-')
		*converted_value *= -1.0;

	return(1);
}
#endif /* CV_ABL_SIGN_TO_VALUE_NEVER_CALLED */


/*
 * NAME:	cv_multiply_value		
 *			
 * PURPOSE: Multiply the variable by a constant	
 *		   								  		
 * AUTHOR:	T. Habermann (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_multiply_value(
 *				VARIABLE_PTR var,			Description of Desired variable
 *				double *converted_value,	Pointer to the Converted value 	 
 *				double conversion_factor,	Factor to multiply to value	   	
 *				char   *var_extension,		Look for input var with extension 
 *				FORMAT_PTR input_format,		Input Format Description		   	  
 *				FF_DATA_BUFFER input_buffer)	Input Buffer 			  
 *													   	   		 
 * COMMENTS:																							
 *
 * RETURNS:	Conversion functions return 0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_multiply_value"

int cv_multiply_value(
	VARIABLE_PTR 	var,				/* Description of Desired variable */
	double 				*converted_value,	/* Pointer to the Converted value */
	double 				conversion_factor,	/* Factor to multiply to value */
	char 				*var_extension,		/* Look for input var with extension */   
	FORMAT_PTR 			input_format,		/* Input Format Description */
	FF_DATA_BUFFER 		input_buffer)		/* Input Buffer */

{
	char 	v_name[16];

	double 	double_value = 0;
	double 	*double_ptr	 = NULL;

	VARIABLE_PTR in_var = NULL;


	/* Initialize the return value to zero */
	double_ptr = &double_value;
   	*converted_value = 0;

	strcpy(v_name, var->name);
	last_underscore = strrchr(v_name,'_');
	if (last_underscore)
		*last_underscore = STR_END;

	in_var = ff_find_variable(strcat(v_name, var_extension), input_format);

	if (in_var)
	{
		memcpy(align, input_buffer + (int)(in_var->start_pos - 1), FF_VAR_LENGTH(in_var));
		(void)ff_get_double(in_var, align, double_ptr,	FFF_TYPE(input_format));
		*converted_value = double_value * conversion_factor;
	}

	if (in_var)
		return(1);
	else
		return(0);
}

/*
 * NAME:	cv_units	
 *			
 * PURPOSE: miscellaneous unit conversions, called as last resort
 *		   								  
 * AUTHOR:	T. Habermann (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_units(
 *				VARIABLE_PTR var,		Description of Desired variable
 *				double *converted_value,Pointer to the Converted value 	 
 *				FORMAT_PTR input_format,	Input Format Description	   	
 *				FF_DATA_BUFFER input_buffer)	Input Buffer
 *													   			 
 * COMMENTS:																							
 *
 * RETURNS:	Conversion functions return 0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_units"

int cv_units(
	VARIABLE_PTR 	var,				/* Description of Desired variable */
	double 				*converted_value,	/* Pointer to the Converted value */
	FORMAT_PTR 			input_format,		/* Input Format Description */
	FF_DATA_BUFFER 		input_buffer)		/* Input Buffer */

{
	char 	v_name[MAX_NAME_LENGTH + 24];	/* Variable Name */
	char 	numstring[80];

	double 	*double_ptr	 = NULL;
	double 	double_value = 0.0;

	int count = 0;
	int i = 0; 
        unsigned variable_length = 0;

	VARIABLE_PTR var_source = NULL;

	name_length = strlen(var->name);
	memcpy(v_name, var->name, name_length);
	*(v_name + name_length) = STR_END;

	last_underscore = strrchr(v_name, '_');

#ifdef SCALED_CONVERSION_NOT_WORKING
/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name				v_name_scaled
 *		   				 
*/
	if (last_underscore && !strcmp(last_underscore, "_scaled"))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(v_name, input_format);
		if (var_source)
		{
    	/* get the scale and offset */
/*			scale_var=var_source->next; */
			double_ptr=(double *)scale_var->name;
			scale=*double_ptr;
			offset=*(double_ptr+1);

			/* initialize the return value to 0 */
			double_ptr = &double_value;
			*converted_value=0;

			memcpy(align, input_buffer + var_source->start_pos - 1, FF_VAR_LENGTH(var_source));
			ff_get_double(var_source, align, double_ptr, FFF_TYPE(input_format));
			*converted_value = double_value*scale + offset;
			return(1);
		}
		else
			return(0);
	}			
#endif /* SCALED_CONVERSION_NOT_WORKING */

/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name_m				v_name_ft
 *		   				 
*/
	if (last_underscore && !strcmp(last_underscore, "_ft"))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(strcat(v_name,"_m"), input_format);
		if (var_source)
		{		/*  value in meters found */

			/* Initialize the return value to zero */
			double_ptr = &double_value;
			*converted_value = 0; 

			memcpy(align, input_buffer + var_source->start_pos - 1, FF_VAR_LENGTH(var_source));
			(void)ff_get_double( var_source, align, double_ptr, FFF_TYPE(input_format));
			*converted_value = double_value * 3.28084;
			return(1);
		}
		else
			return(0);
	}

/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name_ft				v_name_m
 *		   				 
*/

	if (last_underscore && !strcmp(last_underscore, "_m"))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(strcat(v_name,"_ft"), input_format);
		if (var_source)
		{		/*  value in feet found */
			/* Initialize the return value to zero */
			double_ptr = &double_value;
   			*converted_value = 0; 
			
			memcpy(align, input_buffer + var_source->start_pos - 1, FF_VAR_LENGTH(var_source));
			(void)ff_get_double( var_source, align, double_ptr, FFF_TYPE(input_format));
			*converted_value = double_value * 0.3048;
			return(1);
		}
		else
			return(0);
	}
 
/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name					v_name_base and
 *							v_name_exp
 *
 * COMMENTS:				The following conventions are used:
 *
 *							since large numbers are use as non-data flags
 *							***'s are converted to 0.0 E+00
 *
 *							the decimal is place behind the first digit and
 *							the exponent is calculated from there. This results
 *							in 3 possible cases:
 *								1) decimal is in correct position(1.234)
 *								-> _base is 1.234 _exp is E+00
 *								2) decimal is beyond position (123.4)
 *								-> _base is 1.234 _exp is E+02
 *								3) decimal is 1st digit (.1234)
 *								-> _base is 0.1234 _exp is E+00 and
 *								the leading zero is added in processing
 *							
 *							If E is not used as constant, write E
 *							otherwise it is already there.
 *
 *
*/

	if (last_underscore && !strcmp(last_underscore + 1, "base"))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(v_name, input_format);
		if (var_source)
		{
			/* source has been found, put into numstring */
			ff_get_string(var_source,
			              input_buffer + var_source->start_pos - 1,
			              numstring,
			              FFF_TYPE(input_format)
			             );
	
			/* Detirmine current location of decimal to find count number of moves
			to put decimal behind the first digit, if *'s then value is 0  */

			ch = strchr(numstring, '.');
			if (ch == NULL)
				count = 0;
			else
				count = ((char HUGE *)ch - (char HUGE *)numstring) - 1;
		
			ch = numstring;
			if (*ch == '-')
				count--;
			else if (*ch == '.')
				count = 0;
			else if (*ch == '*')
				double_value = 0.0;
			else
				double_value = atof(numstring);

			if (count > 0)
				double_value /= pow(10.0, count);

			*converted_value = double_value;
			return(1);
		}
		else
			return(0);
	}/* end v_name to v_name_base */

	if (last_underscore && !strcmp(last_underscore + 1, "exp"))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(v_name, input_format);
		if (var_source)
		{
			/* source has been found, put into numstring */
			(void)ff_get_string(var_source, input_buffer + var_source->start_pos - 1, numstring, FFF_TYPE(input_format));

			/* find position of the decimal in the _base value
			then detirmine how many places it must move. The decimal
			will not move if count = 0. This occurs when the field
			if filled with *'s, it is already in front or in position */

			if (*numstring == '.' || *numstring == '*')
				count = 0;
			else
			{
				ch = strchr(numstring, '.');
				if (ch == NULL)
					count = 0;
				else
					count = ((char HUGE *)ch - (char HUGE *)numstring) - 1;
			}
			if (*numstring == '-')
				count--;

			/* If _exp is a char string we can create the exp string with +%id
			where i is the field width and d is count, left justified and zero filled 
			*/
			i = (int)(FF_VAR_LENGTH(var)) - 1;
			sprintf(numstring,"+%0*d", i, count);

			memcpy((char *)converted_value, (char *)numstring, i + 1);
			return(1);
		}
		else
			return(0);
	}/* end v_name to exp */

/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name					v_name_abs and
 *							v_name_sign
*/

	if (last_underscore && !strcmp(last_underscore + 1, "abs"))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(v_name, input_format);
		if (var_source)
		{
			/* source has been found, determine value */
			memcpy(align, input_buffer + var_source->start_pos - 1, FF_VAR_LENGTH(var_source));
			(void)ff_get_double(var_source, align, &double_value, FFF_TYPE(input_format));
	
			*converted_value = fabs(double_value);
			return(1);
		}
		else
			return(0);
	}

	if (last_underscore && !strcmp(last_underscore + 1, "sign"))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(v_name, input_format);
		if (var_source)
		{
			/* source has been found, determine value */
			memcpy(align, input_buffer + var_source->start_pos - 1, FF_VAR_LENGTH(var_source));
			(void)ff_get_double(var_source, align, &double_value, FFF_TYPE(input_format));
	
			ch = (FF_DATA_BUFFER)converted_value;
			*ch = (char)((double_value >= 0.0) ? '+' : '-');
			return(1);
		}
		else
			return(0);
	}

/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name					v_name_negate (negate input value)
 */

	if (last_underscore && !strncmp(last_underscore + 1, "negate", 6))
	{
		*last_underscore = STR_END;

		var_source = ff_find_variable(v_name, input_format);
		if (var_source)
		{
			memcpy(align, input_buffer + var_source->start_pos - 1, FF_VAR_LENGTH(var_source));
			(void)ff_get_double(var_source, align, &double_value, FFF_TYPE(input_format));
	
			*converted_value = -double_value;
			return(1);
		}
		else
			return(0);
	}
	
/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name_base				v_name
 *  v_name_exp
 *
 * COMMENTS:	There may or may not be an 'E' or 'e' in the exp part
 *				The range of E format is E+/-999, but can not get a
 *					double => E+/-32, so should we truncate to 31? 
 *
 *				A common error may be an output field that is too small
 *				for the resulting value
 *				
 *				In order that the exponent appear correct when we convert
 *				the vname_exp from ASCII -> binary -> ASCII:
 *					the number must be treated as a character string
 *					the 'E' may be treated as a constant so it doesn't go into binary
 *				The only way around this is to add code in get_string to check if
 *				current variable is _exp and then fill with +/- and zeros
 *
 */							

	*(v_name + name_length) = STR_END;

	var_source = ff_find_variable(strcat(v_name,"_exp"), input_format);
	if (var_source)
	{ /* exp found */
		/* put the exponent string into numstring */
		ff_get_string(var_source,
		              input_buffer + var_source->start_pos - 1,
		              numstring,
		              FFF_TYPE(input_format)
		             );

		/* Now Get The base as a double */
		*(v_name + name_length) = STR_END;

		var_source = ff_find_variable(strcat(v_name,"_base"), input_format);
		if (!var_source)
			return(0);

		variable_length = FF_VAR_LENGTH(var_source);
		memcpy(align, input_buffer + var_source->start_pos - 1, variable_length);
		ff_get_double(var_source, (char *)align, &double_value, FFF_TYPE(input_format));

		/* Increment ch past eform characters then get the value of the exponent
		If ch is pointing at 1st digit, i is 0, else i is number of places skipped */
		i = strspn(numstring, eform_set);
		if ((unsigned int)i == strlen(numstring))
			count = 0;
		else
		{
			ch = numstring + i;
			count = atoi(ch);
		}

		double_value *= pow(10.0, count);
		
		*converted_value = double_value;
	 	return(1);
	}/* end base_exp to v_name */

/*	CONVERT FROM:			TO:
 *		   				 
 *	v_name_abs and			v_name
 *	v_name_sign
 *		   				 
*/

	*(v_name + name_length) = STR_END;

	var_source = ff_find_variable(strcat(v_name,"_abs"), input_format);
	if (var_source)
	{		/* absolute value found */
		memcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source));
		(void)ff_get_double(var_source, align, &double_value, FFF_TYPE(input_format));
		*converted_value = double_value;
		
		/* Now Get The Sign */
		*(v_name + name_length) = STR_END;

		var_source = ff_find_variable(strcat(v_name,"_sign"), input_format);
		if (!var_source)
		{
			if (*converted_value <= 0.0)
				*converted_value *= -1.0;
			return(1);
		}

		ch = input_buffer + var_source->start_pos - 1;
		if (*ch == '-')
			*converted_value *= -1.0;
		return(1);
	}

/*	CONVERT FROM:			TO:
 *		   				 
 *  v_name_negate   v_name
 */

	*(v_name + name_length) = STR_END;

	var_source = ff_find_variable(strcat(v_name, "_negate"), input_format);
	if (var_source)
	{
		memcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source));
		(void)ff_get_double(var_source, align, &double_value, FFF_TYPE(input_format));

		*converted_value = -double_value;
		return(1);
	}

	/* Check for generic date string conversion */

	*(v_name + name_length) = STR_END;

	if (!strncmp(v_name,"date", 4))
	{
		return(cv_date_string(var, converted_value, input_format,input_buffer));
	}
	
	/* else no conversion found */
	return(0);
}  /* End cv_units() */
