/*  
 * FILENAME: latlon.c 
 *
 * CONTAINS:
 * 
 *	FUNCTION	CONVERT FROM			TO
 *							 
 *	cv_abs		v_name or				v_name_abs or
 *										v_name_sign
 *							
 *	cv_deg		v_name_deg				v_name
 *				v_name_min	
 *				v_name_sec
 *							-- OR --
 *				v_name_deg_abs			v_name
 *				v_name_min	  
 *				v_name_sec
 *				v_name_ns
 *				v_name_ew 
 *				v_name_sign	   
 *				geo_quad_code	   
 *
 *  cv_lon_east	longitude_east			longitude
 *							-- OR --
 *				longitude				longitude_east
 *
 *	cv_deg_nsew	v_name_abs				v_name
 *				v_name_ns
 *				v_name_ew
 *				
 *	cv_deg_abs	v_name_deg				v_name_abs
 *				v_name_min
 *				v_name_sec
 *							-- OR --
 *				v_name_deg_abs			v_name_abs
 *				v_name_min	  
 *				v_name_sec
 *
 *	cv_nsew		v_name					v_name_ns	
 *										v_name_ew 
 *					  					geog_quad_code
 *							-- OR --				
 *				v_name_deg				v_name_ns
 *										v_name_ew
 *					  					geog_quad_code
 *						  				
 *						  				
 *	cv_dms		v_name					v_name_deg
 *							-- OR --
 *				v_name_abs				v_name_min
 *				v_name_ns				v_name_sec
 *				v_name_ew		 
 *
 *  cv_degabs_nsew
 *				v_name_deg_abs			(v-name)_deg
 *				v_name_ns
 *				v_name_ew
 *  				  
 *	cv_degabs	v_name 					v_name_deg_abs
 *										v_name_min_abs
 *										v_name_sec_abs
 *						-- OR --
 *				v_name_abs				v_name_deg_abs, _min_abs, sec_abs
 *
 *
 *	
 *	cv_geog_quad	latitude				geog_quad_code
 *				and longitude
 *							-- OR --				
 *				geog_quad_code			latitude_ns
 *										longitude_ew
 *										latitude_sign
 *										longitude_sign
 *
 *  cv_geog_sign	v_name_ns or			v_name_sign
 *				v_name_ew
 *							-- OR --				
 *				v_name_sign				v_name_ns
 *										v_name_ew
 *					  					geog_quad_code
 */ 
#include <stdio.h>
#include <math.h>
#include <freeform.h>

/* External Variables Used in These Functions: */

static char		v_name[MAX_NAME_LENGTH + 24];	/* Variable Name */
static char		align[256];

static FF_DATA_BUFFER 	ch			= NULL;

static char		*first_underscore 	= NULL;
static char 		*last_underscore  	= NULL;
static char		*underscore 		= NULL;

static int			nsew_char 			= 0;

static double 		double_value 		= 0.0;
static double		*double_ptr  		= &double_value;


static unsigned		name_length = 0;

static VARIABLE_PTR	var_deg		= NULL;
static VARIABLE_PTR	var_min		= NULL;
static VARIABLE_PTR	var_sec		= NULL;
static VARIABLE_PTR	var_source	= NULL;

#define WEST	87		/* ASCII W */
#define EAST	69		/* ASCII E */
#define SOUTH	83		/* ASCII S */
#define NORTH	78		/* ASCII N */



/*
 * NAME:	cv_abs
 *
 * PURPOSE:	convert from v_name  to  v_name_abs OR v_name_sign
 *	
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE: 	cv_abs(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_abs"

int cv_abs(VARIABLE_PTR var,			/* Description of Desired variable */
	double 				*converted_value,	/* Pointer to the Converted value */
	FORMAT_PTR 			input_format,		/* Input Format Description */
	FF_DATA_BUFFER 		input_buffer)		/* Input Buffer */

{
	/* Initialize the return value to zero */
	*converted_value = 0.0;

	/*	See if v_name exists	*/
	(void) memStrcpy(v_name, var->name,NO_TAG);

	last_underscore  = memStrrchr(v_name, '_', "v_name,'_'");
	if(last_underscore) *last_underscore = '\0';

	var_source = ff_find_variable(v_name, input_format);

	if (var_source) {

		/* source has been found, determine value */
		memMemcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source), NO_TAG);
		(void) ff_get_double(var_source, align, double_ptr,input_format->type);

		if(memStrcmp(last_underscore + 1, "abs", "last_underscore+1,\"abs\"") == 0)
			*converted_value = fabs(*double_ptr);

		if(memStrcmp(last_underscore + 1, "sign", "last_underscore+1, \"sign\"") == 0){
			ch = (char HUGE *)converted_value;
			*ch = (char)((*double_ptr >= 0.0) ? '+' : '-');
		}

		return(1);
	}
	return(0);
}


/*
 * NAME:	cv_deg
 *			
 * PURPOSE:	CONVERT FROM			TO
 *		   				 
 *			v_name_deg				v_name
 *			v_name_min
 *			v_name_sec
 *						-- OR --
 *			v_name_deg_abs			v_name
 *			v_name_min	  
 *			v_name_sec
 *			v_name_ns
 *			v_name_ew 
 *			geo_quad_code
 *			WMO_quad
 *	
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:  	cv_deg(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_deg"

int cv_deg(VARIABLE_PTR var,		/* Description of Desired variable */
	double *converted_value,		/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */

{
	VARIABLE_PTR	var_nsew 	= NULL;
	signed char 				sign 		= 1;


	/* Initialize return value to zero */
	*converted_value = 0; 

	/* For each extension, see if exists in input_format.
	   If it does, compute the appropriate value */

	name_length = strlen(var->name);
	*(v_name + name_length) = '\0';

	/* find the degrees */
	(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
	first_underscore = memStrchr(v_name, '_', "v_name, '_'");
	if(!first_underscore) first_underscore = v_name + name_length;

	var_deg = ff_find_variable(memStrcat(v_name,"_deg", NO_TAG), input_format);

	if(!var_deg) var_deg = ff_find_variable(memStrcat(v_name,"_abs", NO_TAG), input_format);

	if(var_deg){
		memMemcpy(align, input_buffer + (int)(var_deg->start_pos - 1), FF_VAR_LENGTH(var_deg), NO_TAG);
		(void) ff_get_double(var_deg, align, double_ptr,input_format->type);
		*converted_value += *double_ptr;
		if(*converted_value < 0.0) sign = -1;
	}

	/* Converted value is now in degrees, now find the minutes */
	(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
	*(v_name + name_length) = '\0';

	var_min = ff_find_variable( memStrcat(v_name,"_min", NO_TAG), input_format);
	if(!var_min) var_min = ff_find_variable(memStrcat(v_name,"_abs", NO_TAG), input_format);

	if (var_min) {
		memMemcpy(align, input_buffer + (int)(var_min->start_pos - 1), FF_VAR_LENGTH(var_min), NO_TAG);
		(void) ff_get_double(var_min, align, double_ptr,input_format->type);

		*converted_value += *double_ptr / (sign * 60.);
		if(*converted_value < 0.0) sign = -1;
	}


	/* Converted value is now in degrees and minutes, now find the seconds */
	(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
	*(v_name + name_length) = '\0';

	var_sec = ff_find_variable( memStrcat(v_name,"_sec", NO_TAG), input_format);
	if(!var_sec) var_sec = ff_find_variable(memStrcat(v_name,"_abs", NO_TAG), input_format);


	if (var_sec) {
		memMemcpy(align, input_buffer + (int)(var_sec->start_pos - 1), FF_VAR_LENGTH(var_sec), NO_TAG);
		(void) ff_get_double(var_sec, align, double_ptr,input_format->type);

		*converted_value += *double_ptr / (sign * 3600.);
	}

	
	/* Check the sign of the value if var_nsew is found */

	if (var_deg) {
		(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
		*(v_name + name_length) = '\0';
		var_nsew = ff_find_variable( memStrcat(v_name,"_ns", NO_TAG), input_format);

		if (!var_nsew) {
			*(v_name + name_length) = '\0';
			var_nsew = ff_find_variable( memStrcat(v_name,"_ew", NO_TAG), input_format);
		}

		if (!var_nsew) {
			*(v_name + name_length) = '\0';
			var_nsew = ff_find_variable(memStrcat(v_name,"_sign", NO_TAG), input_format);
		}

		if (!var_nsew) {
			var_nsew = ff_find_variable("geog_quad_code", input_format);
		}

		if (!var_nsew) {
			var_nsew = ff_find_variable("WMO_quad_code", input_format);
		}

		if (var_nsew) {
			ch = input_buffer + var_nsew->start_pos - 1;

			/* Take care of the nsew case: Southern and Western
			Hemispheres are negative */
			if(*ch == 'S' || *ch == 's' || *ch == 'W' || *ch == 'w'){
				*converted_value = -*converted_value;
				return(1);
			}

			/* The geographic quad codes of DMA indicate:
				1 = Northeast
				2 = Northwest
				3 = Southeast
				4 = Southwest
			*/

			if(memStrcmp(var_nsew->name, "geog_quad_code", "var_nsew->name,\"geog_quad_code\"") == 0){
				*(first_underscore) = '\0';
				if(memStrcmp(v_name, "latitude", "v_name,\"latitude\"") == 0 && ((*ch == '3' || *ch == '4') || *ch == '-'))
					*converted_value = -*converted_value;
	
				if(memStrcmp(v_name, "longitude", "v_name,\"longitude\"") == 0 && ((*ch == '2' || *ch == '4') || *ch == '-'))
					*converted_value = -*converted_value;

			}

			/* The geographic quad codes of WMO indicate:
				1 = Northeast
				7 = Northwest
				3 = Southeast
				5 = Southwest
			*/

			if(memStrcmp(var_nsew->name, "WMO_quad_code", "var_nsew->name,\"WMO_quad_code\"") == 0){
				*(first_underscore) = '\0';
				if(memStrcmp(v_name, "latitude", "v_name,\"latitude\"") == 0 && (*ch == '3' || *ch == '5'))
					*converted_value = -*converted_value;
	
				if(memStrcmp(v_name, "longitude", "v_name,\"longitude\"") == 0 && (*ch == '5' || *ch == '7'))
					*converted_value = -*converted_value;

			}
			
		}

	}

	/* Has a conversion been found? */

	if (var_deg || var_min || var_sec ) return(1);
	else return(0);

}
/*
 * NAME:	cv_lon_east
 *			
 * PURPOSE:	CONVERT FROM			TO
 *		   				 
 *			longitude_east 			longitude
 *						-- OR --
 *			longitude 				longitude_east
 *			v_name_min	  
 *			v_name_sec
 *			v_name_ns
 *			v_name_ew 
 *			geo_quad_code
 *	
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:  	cv_lon_east(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS: This function deals with longitude_east
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_lon_east"

int cv_lon_east(VARIABLE_PTR var,		/* Description of Desired variable */
	double *converted_value,		/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */

{
	*converted_value = 0;  	/* Initialize the return value to zero */

	/* Check to see which way the conversion is going */
	
	if(memStrcmp(var->name, "longitude", "var->name,\"longitude\"") == 0){

		/* Conversion of longitude_east to longitude */
		var_source = ff_find_variable("longitude_east", input_format);
		if(!var_source) return(0);

		memMemcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source), NO_TAG);
		(void) ff_get_double(var_source, align, double_ptr,input_format->type);

		if(*double_ptr < 180.0)	*converted_value += *double_ptr;
		else *converted_value = *double_ptr - 360.0;
		return(1);
	}

	/* Conversion of longitude to longitude_east */
	var_source = ff_find_variable("longitude", input_format);
	if(!var_source) return(0);

	memMemcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source), NO_TAG);
	(void) ff_get_double(var_source, align, double_ptr,input_format->type);
	if(*double_ptr > .000000000000001)	*converted_value += *double_ptr;
	else *converted_value = 360 + *double_ptr;
	return(1);
}

/*
 * NAME:	cv_deg_nsew
 *			
 * PURPOSE:	CONVERT FROM			TO
 * 		   				 
 *			v_name_abs				v_name
 *			v_name_ns
 *			v_name_ew
 *			
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_deg_nsew(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_deg_nsew"

int cv_deg_nsew(VARIABLE_PTR var,		/* Description of Desired variable */
	double *converted_value,			/* Pointer to the Converted value */
	FORMAT_PTR input_format,				/* Input Format Description */
	FF_DATA_BUFFER input_buffer)					/* Input Buffer */

{
	VARIABLE_PTR var_abs;
	VARIABLE_PTR var_nsew;

	/* Initialize the return value to zero */

	double_ptr = &double_value;
	*converted_value = 0;

	/* See if v_name_abs and v_name_ns (or ew) exist in the input format */

	(void) memStrcpy(v_name, var->name, NO_TAG);
	var_abs = ff_find_variable( memStrcat(v_name,"_abs", NO_TAG), input_format);

	(void) memStrcpy(v_name, var->name, NO_TAG);
	var_nsew = ff_find_variable( memStrcat(v_name,"_ns", NO_TAG), input_format);
	if (!var_nsew) {
		(void) memStrcpy(v_name, var->name, NO_TAG);
		var_nsew = ff_find_variable( memStrcat(v_name,"_ew", NO_TAG), input_format);
	}

	/* If both var_abs and var_nsew exist, compute the converted value */


	if (var_abs && var_nsew) {

		memMemcpy(align, input_buffer + (int)(var_abs->start_pos - 1), FF_VAR_LENGTH(var_abs), NO_TAG);
		(void) ff_get_double(var_abs, align, double_ptr,input_format->type);
		*converted_value = *double_ptr;

		ch = input_buffer + var_nsew->start_pos - 1;

	/* Change the sign of converted value depending on
		the value of nsew_char */

		nsew_char = toupper( (int)((* (char HUGE *) ch ) % 128));
		if (nsew_char == NORTH || nsew_char == EAST)
			*converted_value = fabs(*converted_value);
		if (nsew_char == SOUTH || nsew_char == WEST)
			*converted_value = -fabs(*converted_value);

	}

	if (var_abs && var_nsew ) return(1);
	else return(0);

}


/*
 * NAME:	cv_deg_abs
 *				   
 * PURPOSE:	CONVERT FROM			TO
 *		   				 
 *			v_name_deg				v_name_abs
 *			v_name_min
 *			v_name_sec
 *						-- OR --
 *			v_name_deg_abs			v_name_abs
 *			v_name_min	  
 *			v_name_sec
 *
 *
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_deg_abs(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *										
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_deg_abs"

int cv_deg_abs(VARIABLE_PTR var,
								/* Description of Desired variable */
	double *converted_value,		/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */


{
	/* Initialize the return value to zero */
	*converted_value = 0.0;

	/* For each extension, see if exists in input_format.
	If it does, compute the appropriate value, also check for _abs extensions */

	/* Look for v_name_deg and v_name_deg_abs */
	(void) memStrcpy(v_name, var->name, NO_TAG);
	underscore = memStrrchr(v_name, '_', "v_name, '_'");
	*(underscore++) = '\0';

	var_deg = ff_find_variable( memStrcat(v_name,"_deg", NO_TAG), input_format);
	if (!var_deg) var_deg = ff_find_variable( memStrcat(v_name,"_abs", NO_TAG), input_format);

	if (var_deg) {

		memMemcpy(align, input_buffer + (int)(var_deg->start_pos - 1), FF_VAR_LENGTH(var_deg), NO_TAG);
		(void) ff_get_double(var_deg, align, double_ptr,input_format->type);
		*converted_value = fabs(*double_ptr);
	}

	/* Look for v_name_min and v_name_min_abs */
	(void) memStrcpy(v_name, var->name, NO_TAG);
	underscore = memStrrchr(v_name, '_', "v_name, '_'");
	*(underscore++) = '\0';

	var_min = ff_find_variable( memStrcat(v_name,"_min", NO_TAG), input_format);
	if (!var_min) var_min = ff_find_variable( memStrcat(v_name,"_abs", NO_TAG), input_format);

	if (var_min) {
		memMemcpy(align, input_buffer + (int)(var_min->start_pos - 1), FF_VAR_LENGTH(var_min), NO_TAG);
		(void) ff_get_double(var_min, align, double_ptr,input_format->type);
		*converted_value += fabs(*double_ptr / 60.0);
	}

	/* Look for v_name_sec and v_name_sec_abs */
	(void) memStrcpy(v_name, var->name, NO_TAG);
	underscore = memStrrchr(v_name, '_', "v_name, '_'");
	*(underscore++) = '\0';

	var_sec = ff_find_variable( memStrcat(v_name,"_sec", NO_TAG), input_format);
	if (!var_sec) var_sec = ff_find_variable( memStrcat(v_name,"_abs", NO_TAG), input_format);

	if (var_sec) {
		memMemcpy(align, input_buffer + (int)(var_sec->start_pos - 1), FF_VAR_LENGTH(var_sec), NO_TAG);
		(void) ff_get_double(var_sec, align, double_ptr,input_format->type);
		*converted_value += fabs(*double_ptr / 3600.0);

	}

	/* Has a conversion been found? */

	if (var_deg || var_min || var_sec ) return(1);
	else return(0);

}


/*
 * NAME:	cv_nsew
 *				   	   	   
 * PURPOSE:	CONVERT FROM			TO
 *		   				 
 *				v_name					v_name_ns	
 *										v_name_ew 
 *							-- OR --				
 *				v_name_deg				v_name_ns
 *										v_name_ew
 *							-- OR --				
 *				v_name_sign				v_name_ns
 *										v_name_ew
 *					  				
 *
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_nsew(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *		 		double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   		
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_nsew"

int cv_nsew(VARIABLE_PTR var,		/* Description of Desired variable */
	double *converted_value,   		/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */

{
	char *nsew;

	*converted_value = 0;	/* Initialize the return value to zero */
	nsew = (char *)converted_value;

	/*  See if v_name exists in the input format	*/

	name_length = strlen(var->name);

	/* define the first and last underscores */
	(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
	*(v_name + name_length) = '\0';
	first_underscore = memStrchr(v_name, '_', "v_name, '_'");
	last_underscore  = memStrrchr(v_name, '_', "v_name, '_'");

	*(first_underscore) = '\0';

	var_source = ff_find_variable(v_name, input_format);

	/* If v_name does not exist, try v_name_deg	*/
	if (!var_source) var_source = ff_find_variable(memStrcat(v_name, "_deg", NO_TAG), input_format);

	if (!var_source) return(0);
	
	(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
	*(v_name + name_length) = '\0';

	memMemcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source), NO_TAG);
	(void) ff_get_double(var_source, align, double_ptr,input_format->type);

	if (!memStrcmp(last_underscore,"_ns", NO_TAG))
		*nsew = (char)((*double_ptr < 0.0) ? SOUTH : NORTH);
	if (!memStrcmp(last_underscore,"_ew", NO_TAG))
		*nsew = (char)((*double_ptr < 0.0) ? WEST  : EAST);

	return(1);
}

/*
 * NAME:	cv_dms
 *				   	   	   
 * PURPOSE:	CONVERT FROM			TO
 *		   				 			
 *			v_name					v_name_deg
 *			v_name_abs				v_name_min
 *			v_name_ns				v_name_sec
 *			ew		 
 *
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_dms(VARIABLE_PTR var,	Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *										   
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 *			If the degrees are between -1 and 0       minutes are negated
 *			If the minutes are also between -1 and 0  seconds are negated
 *
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *		   
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_dms"		

int cv_dms(VARIABLE_PTR var,	/* Description of Desired variable */
	double *converted_value,	/* Pointer to the Converted value */
	FORMAT_PTR input_format,		/* Input Format Description */
	FF_DATA_BUFFER input_buffer)			/* Input Buffer */


{
	VARIABLE_PTR var_nsew = NULL;
	short abs_marker = 0;
	short negate_value = 0;

 	*converted_value = 0;  	/* Initialize the return value to zero */

	name_length = strlen(var->name);

	/* define the first and last underscores */
	(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
	*(v_name + name_length) = '\0';
	first_underscore = memStrchr(v_name, '_', "v_name, '_'");
	last_underscore  = memStrrchr(v_name, '_', "v_name, '_'");

	/* Check to see if the base variable exists */
	*(first_underscore) = '\0';
	var_source = ff_find_variable(v_name, input_format);

	/* If v_name does not exist, try v_name_abs	*/

	if (!var_source) {

		(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
		(void) memStrcpy(last_underscore, "_abs", NO_TAG);
	
		var_source = ff_find_variable(v_name, input_format);

		if (var_source){
			abs_marker++;
			(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
		}

	}

	/* If neither the base name or abs can be found, return */
	if(!var_source) return(0);

	/* At this point either
		1) the variable base (abs_marker == 0) or
		2) _abs 			 (abs_marker == 1)
		is selected as var_source.
	*/

	/* Restore v_name */
	(void) memMemcpy(v_name, var->name, name_length, NO_TAG);

	/* Define the location of the input variable and get it's value */
	memMemcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source), NO_TAG);
	(void) ff_get_double(var_source, align, double_ptr,input_format->type);

	/* If the variable being constructed is _deg, check for sign conversion */
	if (!memStrcmp(last_underscore,"_deg", NO_TAG)) {

		if (abs_marker) {
	
			(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
			*(first_underscore) = '\0';

			var_nsew = ff_find_variable( memStrcat(v_name,"_ns", NO_TAG),	input_format);

			if (!var_nsew) {
				(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
				*(first_underscore) = '\0';
				var_nsew = ff_find_variable( memStrcat(v_name,"_ew", NO_TAG), input_format);
			}
	
			if (var_nsew) {
	
				ch = input_buffer + var_nsew->start_pos - 1;
				nsew_char = toupper( (int)((* (char HUGE *) ch ) % 128));
	
				if (nsew_char == NORTH || nsew_char == EAST)
						*converted_value = fabs(*converted_value);
	
				if (nsew_char == SOUTH || nsew_char == WEST)
						*converted_value = -fabs(*converted_value);
			}
		}

		*converted_value = (double) ( (int) *double_ptr);
		return(1);
	}

	if (!memStrcmp(last_underscore,"_min", NO_TAG)) {

		/* Check to see if the degrees are between -1 and 0,
		negate the minutes if they are */

		if (*double_ptr < 0.0 && *double_ptr > -1.0) negate_value = 1;
		*converted_value = fabs(*double_ptr);

		*converted_value = 60.0 * fmod(*converted_value, 1.0);

		/* If output minutes needs precision, look for seconds
			and convert to minutes */

		if (var->precision) {

			(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
			*(first_underscore) = '\0';
			var_sec = ff_find_variable(memStrcat(v_name, "_sec", NO_TAG),	input_format);

			if (var_sec) {
				memMemcpy(align, input_buffer + (int)(var_sec->start_pos - 1), FF_VAR_LENGTH(var_sec), NO_TAG);
				(void) ff_get_double(var_sec, align, double_ptr,input_format->type);

				*converted_value += fabs(*double_ptr / 60.0);
			}

		}
		else *converted_value = (double) ( (int) *converted_value);

		if (*converted_value && negate_value) *converted_value *= -1.0;

		return(1);		/* Done with _min */
	}

	if (!memStrcmp(last_underscore,"_sec", NO_TAG)) {

		/* Check to see if the minutes are between -1 and 0,
		 negate the seconds if they are */

		if (*double_ptr < 0.0 && *double_ptr > -1.0) negate_value = 1;
		*converted_value = fabs(*double_ptr);

		/* convert to minutes */
		*converted_value = 60.0 * (*converted_value -
			(double) ( (int) *converted_value));

		if (negate_value)
			if (*converted_value >= 1.0) negate_value = 0;


		/* convert to seconds */
		*converted_value = 60.0 * (*converted_value -
			(double) ( (int) *converted_value));
	}
 
	if (negate_value) *converted_value *= -1.0;
	return(1);
}


/*
 * NAME:	cv_degabs_nsew
 *			   
 * PURPOSE:	CONVERT FROM			TO
 *						 
 *			v_name_deg_abs			(v-name)_deg
 *			v_name_ns	  
 *			v_name_ew
 *			
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_degabs_nsew(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_degabs_nsew"

int cv_degabs_nsew(VARIABLE_PTR var,
								/* Description of Desired variable */
	double *converted_value,			/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */

{
	VARIABLE_PTR var_abs; 
	VARIABLE_PTR var_nsew;
	char *extension;

	*converted_value = 0;	/* Initialize the return value to zero */

	/* See if v_name_deg_abs and v_name_ns (or ew) exist in the input format */

	(void) memStrcpy(v_name, var->name, NO_TAG);
	extension = memStrchr(v_name, '_', "v_name, '_'");
	*extension = '\0';
	var_abs = ff_find_variable( memStrcat(v_name,"_deg_abs", NO_TAG), input_format);

	(void) memStrcpy(v_name, var->name, NO_TAG);
	extension = memStrchr(v_name, '_', "v_name, '_'");
	*extension = '\0';
	var_nsew = ff_find_variable( memStrcat(v_name,"_ns", NO_TAG), input_format);

	if (!var_nsew) {
		(void) memStrcpy(v_name, var->name, NO_TAG);
		extension = memStrchr(v_name, '_', "v_name, '_'");
		*extension = '\0';
		var_nsew = ff_find_variable( memStrcat(v_name,"_ew", NO_TAG), input_format);
	}

	/* If both var_abs and var_nsew exist, compute the converted value */

	if (var_abs && var_nsew) {

		memMemcpy(align, input_buffer + (int)(var_abs->start_pos - 1), FF_VAR_LENGTH(var_abs), NO_TAG);
		(void) ff_get_double(var_abs, align, double_ptr,input_format->type);
		*converted_value = *double_ptr;

		ch = input_buffer + var_nsew->start_pos - 1;

		/* Change the sign of converted value depending on
		the value of nsew_char */

		nsew_char = toupper( (int)((* (char HUGE *) ch ) % 128));

		if (nsew_char == NORTH || nsew_char == EAST)
			*converted_value = fabs(*converted_value);

   		if (nsew_char == SOUTH || nsew_char == WEST)
			*converted_value = -fabs(*converted_value);

	}

	if (var_abs && var_nsew ) return(1);
	else return(0);

}


/*
 * NAME:	cv_degabs
 *			   		 
 * PURPOSE:	CONVERT FROM			TO
 *									
 *			v_name 					v_name_deg_abs, _min_abs, _sec_abs
 *						-- OR --
 *			v_name_abs				v_name_deg_abs, _min_abs, _sec_abs
 *
 *				  
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_degabs(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS: This function deals with degree / minute / second and NSEW
 *				conversions.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_degabs"

int cv_degabs(VARIABLE_PTR var,	/* Description of Desired variable */
	double *converted_value,		/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */

{
	
	/* Initialize the return value to zero */
	*converted_value = 0.0;

	/*  See if v_name exists in the input format	*/

	name_length = strlen(var->name);

	(void) memStrcpy(v_name, var->name, NO_TAG);
	first_underscore = memStrchr(v_name, '_', "v_name, '_'");
	*first_underscore = '\0';

	var_source = ff_find_variable(v_name, input_format);
	if (!var_source) {		/* Check for v_name_abs */
		memStrcat(v_name, "_abs", NO_TAG);
		var_source = ff_find_variable(v_name, input_format);
	}

	if(!var_source) return(0);	/* Give up */

	/* Restore v_name and get value of the input v_name */
	(void)memMemcpy(v_name, var->name, name_length, NO_TAG);

	memMemcpy(align, input_buffer + (int)(var_source->start_pos - 1), FF_VAR_LENGTH(var_source), NO_TAG);
	(void) ff_get_double(var_source, align, double_ptr,input_format->type);


	/* construct _deg_abs */
	if (!memStrcmp(first_underscore, "_deg_abs", NO_TAG)){

		*converted_value = fabs( (double) ( (int) *double_ptr) );
	
		return(1);
	}

	/* construct _min_abs */
	if (!memStrcmp(first_underscore, "_min_abs", NO_TAG)){

		*converted_value = fabs(*double_ptr);

		*converted_value = 60.0 * fmod(*converted_value, 1.0);

		/* If output minutes needs precision, look for seconds
			and convert to minutes */

		if (var->precision) {

			(void) memMemcpy(v_name, var->name, name_length, NO_TAG);
			*(first_underscore) = '\0';
			var_sec = ff_find_variable(memStrcat(v_name, "_sec", NO_TAG),	input_format);
			if (!var_sec) var_sec = ff_find_variable(memStrcat(v_name, "_sec_abs", NO_TAG),	input_format);

			if (var_sec) {
				memMemcpy(align, input_buffer + (int)(var_sec->start_pos - 1), FF_VAR_LENGTH(var_sec), NO_TAG);
				(void) ff_get_double(var_sec, align, double_ptr,input_format->type);

				*converted_value += fabs(*double_ptr / 60.0);
			}

		}
		else *converted_value = (double) ( (int) *converted_value);


		return(1);		/* Done with _min */

	}

	/* construct _sec_abs */
	if (!memStrcmp(first_underscore, "_sec_abs", NO_TAG)){

		*converted_value = fabs(*double_ptr);

		/* convert to minutes */
		*converted_value = 60.0 * (*converted_value -
			(double) ( (int) *converted_value));

		/* convert to seconds */
		*converted_value = 60.0 * (*converted_value -
			(double) ( (int) *converted_value));
	
	}

  	return(1);
} /* End cv_degabs() */


/*
 * NAME:	cv_geog_quad	
 *			   		 
 * PURPOSE:	The geographic quad codes of DMA indicate:
 *		   		1 = Northeast
 *				2 = Northwest
 *				3 = Southeast
 *				4 = Southwest
 *
 *			The geographic quad codes of WMO indicate:
 *				1 = Northeast
 *				7 = Northwest
 *				3 = Southeast
 *				5 = Southwest
 *	
 *			CONVERT FROM			TO
 *  					 
 *			latitude and longitude	geog_quad_code
 *						WMO_quad_code
 *
 *						-- OR --				
 *
 *			latitude_ns and
 *			longitude_ew			geog_quad_code
 *							WMO_quad_code
 *
 *						-- OR --
 *
 *			latitude_sign and
 *			longitude_sign			geog_quad_code
 *							WMO_quad_code
 *				  
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_geog_quad(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS: This function creates a geographic quadrant code used by
 *			 DMA in their gravity data.
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_geog_quad"

int cv_geog_quad(VARIABLE_PTR var,	/* Description of Desired variable */
	double *converted_value,		/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */
{
	VARIABLE_PTR var_latlon = NULL;

	unsigned char ns = 0, ew = 0;
	char *nsew;

	*converted_value = 0;	/* Initialize the return value to zero */
	nsew = (char *)converted_value;

	/* First search for latitude */
	var_latlon = ff_find_variable("latitude", input_format);

	if(var_latlon){
		ch = input_buffer + var_latlon->start_pos -1;
		memMemcpy(align, ch, FF_VAR_LENGTH(var_latlon), NO_TAG);
		(void) ff_get_double(var_latlon, align, double_ptr,input_format->type);

		if(*double_ptr <= 0.0) ns = 1;
	}

	else {
		/* try for latitude_ns */
		var_latlon = ff_find_variable("latitude_ns", input_format);
	
		if(var_latlon){
			ch = input_buffer + var_latlon->start_pos - 1;
			if(*ch == 'S' || *ch == 's') ns = 1;
		}

		else {

			/* try for latitude_sign */
			var_latlon = ff_find_variable("latitude_sign", input_format);
			if(var_latlon){
				ch = input_buffer + var_latlon->start_pos - 1;
				if(*ch == '-') ns = 1;
			}
		}
	}
		
	if(!ch) return (0);
	ch = NULL;

	/* Next search for longitude */
	var_latlon = ff_find_variable("longitude", input_format);

	if(var_latlon){
		ch = input_buffer + var_latlon->start_pos -1;
		memMemcpy(align, ch, FF_VAR_LENGTH(var_latlon), NO_TAG);
		(void) ff_get_double(var_latlon, align, double_ptr,input_format->type);

		if(*double_ptr <= 0.0) ew = 1;
	}

	else {	/* try for longitude_ew */
		var_latlon = ff_find_variable("longitude_ew", input_format);
	
		if(var_latlon){
			ch = input_buffer + var_latlon->start_pos - 1;
			if(*ch == 'W' || *ch == 'w') ew = 1;
		}

		else {
			var_latlon = ff_find_variable("longitude_sign", input_format);
		
			if(var_latlon){
				ch = input_buffer + var_latlon->start_pos - 1;
				if(*ch == '-') ew = 1;
			}

		}
	}

	if(!ch)return (0);

	if(memStrcmp(var->name, "geog_quad_code", "var->name,\"geog_quad_code\"") == 0){
		if(ns == 0){
			if(ew == 0) *nsew = '1';	/* Northeast */
			else *nsew = '2';			/* Northwest */
		}
		else {
			if(ew == 0) *nsew = '3';	/* Southeast */
			else *nsew = '4';			/* Southwest */
		}
	}
	
	if(memStrcmp(var->name, "WMO_quad_code", "var->name,\"WMO_quad_code\"") == 0){
		if(ns == 0){
			if(ew == 0) *nsew = '1';	/* Northeast */
			else *nsew = '7';			/* Northwest */
		}
		else {
			if(ew == 0) *nsew = '3';	/* Southeast */
			else *nsew = '5';			/* Southwest */
		}
	}

	return(1);
}

/*
 * NAME:	cv_geog_sign	
 *			   		 
 * PURPOSE:	Convert between various lat/lon signs
 *	
 *			CONVERT FROM			TO
 *  					 
 *			latitude_ns				latitude_sign
 *			longitude_ew			longitude_sign
 *						-- OR --				
 *			latitude_sign			latitude_ns
 *			longitude_sign			longitude_ew
 *				  
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 *
 * USAGE:	cv_geog_sign(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)	Input Buffer 
 *								   
 * COMMENTS:
 *			
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_geog_sign"

int cv_geog_sign(VARIABLE_PTR var,	/* Description of Desired variable */
	double *converted_value,		/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* Input Buffer */
{
	char *nsew;

	*converted_value = 0;	/* Initialize the return value to zero */
	nsew = (char *)converted_value;

	last_underscore  = memStrrchr(var->name, '_', "var->name, '_'");

	if(!last_underscore) return(0);

	/* Determine what is being created */
	if(memStrcmp(last_underscore + 1, "sign", "last_underscore+1,\"sign\"") == 0){	/* Creating a sign */

		if(*(var->name + 1) == 'o'){		/* Longitude */
			var_source = ff_find_variable("longitude_ew", input_format);
	
			if(var_source){
				ch = input_buffer + var_source->start_pos - 1;
				if(*ch == 'W' || *ch == 'w') *nsew = '-';
				else *nsew = '+';
				return(1);
			}
			else return(0);
		}

		/* Latitude */
		var_source = ff_find_variable("latitude_ns", input_format);
	
		if(var_source){
			ch = input_buffer + var_source->start_pos - 1;
			if(*ch == 'S' || *ch == 's') *nsew = '-';
			else *nsew = '+';
			return(1);
		}
		else return(0);
	}

	if(memStrcmp(last_underscore + 1, "ew", "last_underscore+1,\"ew\"") == 0){	/* Creating ew from a sign */
		var_source = ff_find_variable("longitude_sign", input_format);
	
		if(var_source){
			ch = input_buffer + var_source->start_pos - 1;
			if(*ch == '+' || *ch == ' ') *nsew = 'E';
			else *nsew = 'W';
			return(1);
		}
		else return(0);
		
	}

	if(memStrcmp(last_underscore + 1, "ns", "last_underscore+1,\"ns\"") == 0){	/* Creating ns from a sign */

		var_source = ff_find_variable("latitude_sign", input_format);
	
		if(var_source){
			ch = input_buffer + var_source->start_pos - 1;
			if(*ch == '-') *nsew = 'S';
			else *nsew = 'N';
			return(1);
		}
		else return(0);
	}

	return(0);
}



