/* 
 * This file contains conversion functions which are specifically related
 * to earthquake data sets.
 *
 * CONTAINS:
 * 
 *	FUNCTION	CONVERT FROM:			TO:
 *							 
 *	cv_long2mag	longmag					magnitude_mb
 *										magnitude_ms
 *										magnitude_ml
 *										mb-max_like
 *
 *	cv_mag2long	magnitude_mb or			longmag
 *				magnitude_ms or
 *				magnitude_ml
 * 
 *	cv_noaa_eq	NOAA binary				earthquake variables
 *
 *  cv_sea_flags	Seattle Catalog		Special Variables
 *
 *  cv_slu_flags	slu_line2 into			non_tectonic
 *											cultural
 *											intensity
 *											magnitude_ml
 *											scale
 *											ml_authority
 *
 */
 
/*
 * NAME:	cv_long2mag
 *
 * PURPOSE:	This is a conversion function for the FREEFORM system which is
 *			used to get one of the three magnitudes out out the variable
 *			longmag, or to creat longmag from one, two, or three of the
 *			single magnitudes.
 *  	
 *			Longmag is a long which contains three magnitudes:
 *  
 *			ms2 is a magnitude with a precision of 2 and is multiplied by
 *			10,000,000
 *			ms1 is a magnitude with a precision of 2 which is multiplied by
 *			10,000.
 *			mb is a magnitude with a  precision of 1 which is multiplied by 10.
 *
 * AUTHOR:	T. Habermann (303) 497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_long2mag(
 *				VARIABLE_LIST_PTR out_var,
 *				double *mag, 
 *				FORMAT_PTR input, 
 *				FF_DATA_BUFFER input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freeform.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_long2mag"

int cv_long2mag(
	VARIABLE_PTR	 	out_var, 
	double 				*mag, 
	FORMAT_PTR 			input, 
	FF_DATA_BUFFER 		input_buffer)
{

	VARIABLE_PTR 		var_longmag	= NULL;
	unsigned int 		tmp			= 0;
	double 				tmp_mag		= 0;
	unsigned long 		longmag		= 0;
	static char			align[256];

	/* First check the input format to make sure that longmag is present */
	var_longmag = ff_find_variable("longmag", input);
	if(!var_longmag){
		return(0);
	}

	/* Get the input variable */

	memMemcpy(align, input_buffer + var_longmag->start_pos - 1, FF_VAR_LENGTH(var_longmag),NO_TAG);
	(void)ff_get_double(var_longmag, align, &tmp_mag, input->type);

	longmag = (unsigned long)(tmp_mag + .5);

	/* Now make a decision about which magnitude to output */
	 if(memStrcmp(out_var->name, "mb",NO_TAG) == 0 ||
	 	memStrcmp(out_var->name, "magnitude_mb",NO_TAG) == 0){	/* Body Wave Magnitude */
		tmp = (unsigned int)(longmag % 100);
		*mag = tmp / 10.0;
		return(1);
	}
	if(memStrcmp(out_var->name, "ms1",NO_TAG) == 0 ||
	 	memStrcmp(out_var->name, "magnitude_ms1",NO_TAG) == 0){	/* First Surface Wave Magnitude */
		tmp = (unsigned int)(longmag % 100000);
		*mag = tmp / 10000.;
		return(1);
	}
	if(memStrcmp(out_var->name, "ms2",NO_TAG) == 0 ||
	 	memStrcmp(out_var->name, "ml",NO_TAG) 			== 0 ||
	 	memStrcmp(out_var->name, "magnitude_ms2",NO_TAG) 	== 0 ||
	 	memStrcmp(out_var->name, "magnitude_ml",NO_TAG) 	== 0 ||
	 	memStrcmp(out_var->name, "magnitude_local",NO_TAG)== 0){	/* Second Surface Wave Magnitude */
		*mag = longmag / 10000000.;
		return(1);
	}
	if(memStrcmp(out_var->name, "mb-maxlike",NO_TAG) == 0){	/* Magnitude Difference */
		tmp =(unsigned int)( longmag % 100);			/* Get mb */
		*mag = tmp / 10.0;
		tmp = (unsigned int)(longmag / 100000.);
		*mag -= tmp / 100.0;
		return(1);
	}
	return(0);
}

/*
 * NAME:	cv_mag2long
 *
 * PURPOSE:	This is a conversion function for the FREEFORM system which is
 *			used to create a long with three magnitudes from one, two,
 *			or three of the	single magnitudes.
 *
 *			CONVERTS	mb or ms1 or ms2	TO	longmag
 *  	
 *			Longmag is a long which contains three magnitudes:
 *			 
 *			ms2 is a magnitude with a precision of 2 and is multiplied by
 *			10,000,000
 *			ms1 is a magnitude with a precision of 2 which is multiplied by
 *			10,000.
 *			mb is a magnitude with a  precision of 1 which is multiplied by 10.
 *
 * AUTHOR:	T. Habermann (303) 497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_mag2long(
 *				VARIABLE_PTR out_var, 
 *				double		*mag, 
 *				FORMAT_PTR	input, 
 *				FF_DATA_BUFFER	input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_mag2long"

int cv_mag2long(VARIABLE_PTR out_var, double *mag, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{
	VARIABLE_PTR mag_var;
	static char			align[256];

	long longmag = 0L;
	double tmp_mag;
	short got_data = 0;

	/* Check input format to see if mb is present */

	mag_var = ff_find_variable("magnitude_mb", input_format);
	if(!mag_var) mag_var = ff_find_variable("mb", input_format);

	if(mag_var){
		if(!got_data)++got_data;

		/* Get the input variable */
		memMemcpy(align, input_buffer + mag_var->start_pos - 1, FF_VAR_LENGTH(mag_var),NO_TAG);
		(void)ff_get_double(mag_var, align, &tmp_mag, input_format->type);
		longmag = (long)(tmp_mag * 10 + .0001);
	}

	/* Check input format to see if ms1 is present */
	mag_var = 	 ff_find_variable("magnitude_ms1", input_format);
	if(!mag_var) mag_var = ff_find_variable("ms", input_format);
	if(!mag_var) mag_var = ff_find_variable("ms1", input_format);

	if(mag_var){
		if(!got_data)++got_data;

		/* Get the input variable */
		memMemcpy(align, input_buffer + mag_var->start_pos - 1, FF_VAR_LENGTH(mag_var),NO_TAG);
		(void)ff_get_double(mag_var, align, &tmp_mag, input_format->type);
		longmag += (long)(tmp_mag * 1000 + .5);
	}

	/* Check input format to see if ms2 is present */
	mag_var = 	 ff_find_variable("magnitude_ms2", input_format);
	if(!mag_var) mag_var = ff_find_variable("magnitude_ml", input_format);
	if(!mag_var) mag_var = ff_find_variable("magnitude_local", input_format);
	if(!mag_var) mag_var = ff_find_variable("ml", input_format);
	if(!mag_var) mag_var = ff_find_variable("ms2", input_format);

	if(mag_var){
		if(!got_data)++got_data;
		/* Get the input variable */
		memMemcpy(align, input_buffer + mag_var->start_pos - 1, FF_VAR_LENGTH(mag_var),NO_TAG);
		(void)ff_get_double(mag_var, align, &tmp_mag, input_format->type);
		longmag += (long)(tmp_mag * 10000000 + .5);
	}
	*mag = (double)longmag;
	return(got_data);
}

/*
 * NAME:	cv_mag_diff
 *
 * PURPOSE:	This is a conversion function for the FREEFORM system which is
 *			used to calculate differences between various magnitudes.
 *  
 *			CONVERT FROM:				TO:
 *				magnitude_mb and		mb-max_like
 *				magnitude_max_like
 *
 * AUTHOR:	T. Habermann (303) 497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_mag_diff(
 *				VARIABLE_LIST_PTR out_var,
 *				double *mag, 
 *				FORMAT_PTR input, 
 *				FF_DATA_BUFFER input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#include <stdio.h>
#include <string.h>
#include <freeform.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_mag_diff"

int cv_mag_diff(
	VARIABLE_PTR	 	out_var, 
	double 				*mag_diff, 
	FORMAT_PTR 			input, 
	FF_DATA_BUFFER 		input_buffer)
{

	VARIABLE_PTR 		var_mag_1	= NULL;
	VARIABLE_PTR 		var_mag_2	= NULL;
	char				magnitude_1[64] = "magnitude_";
	char				magnitude_2[64] = "magnitude_";
	char				*minus;
	double				mag_1 = 0.0;
	double				mag_2 = 0.0;		
	static char			align[256];

	/* Create the magnitude names from the output variable. That variable
	has the form mag_type_1-mag_type_2. */
	memStrcpy(align, out_var->name,NO_TAG);
	minus = memStrchr(align, '-',NO_TAG);
	if(!minus)return(0);
	
	*minus = '\0';
	memStrcat(magnitude_1, align,NO_TAG);
	memStrcat(magnitude_2, minus + 1,NO_TAG);
	
	/* Find the first magnitude */
	var_mag_1 = ff_find_variable(magnitude_1, input);
	if(!var_mag_1)	return(0);
	
	var_mag_2 = ff_find_variable(magnitude_2, input);
	if(!var_mag_2)	return(0);

	memMemcpy(align, input_buffer + var_mag_1->start_pos - 1, FF_VAR_LENGTH(var_mag_1),NO_TAG);
	(void)ff_get_double(var_mag_1, align, &mag_1, input->type);

	memMemcpy(align, input_buffer + var_mag_2->start_pos - 1, FF_VAR_LENGTH(var_mag_2),NO_TAG);
	(void)ff_get_double(var_mag_2, align, &mag_2, input->type);

	*mag_diff = mag_1 - mag_2;

	return(1);
}

/* 
 *
 * CONTAINS:
 * 
 *	FUNCTION		CONVERT FROM:			TO:
 *							 
 *	cv_noaa_eq		NOAA bit mast			variables
 *	
 */ 
/* 
 * HISTORY:
 *	r fozzard	4/21/95		-rf01
 *		comment out search.h (not needed?)
*/

#include <stdio.h>
#include <string.h>
/*  #include <search.h> comment out search.h (not needed?) -rf01 */
#include <freeform.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "b_strcmp"

int b_strcmp(const void *, const void *);

/*
 * NAME:	cv_noaa_eq
 *
 * PURPOSE:	convert from NOAA bit mask to earthquake parameters
 *	
 * AUTHOR: T. Habermann, NGDC, (303) 497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE: 	cv_noaa_eq(
 *				VARIABLE_PTR var,		Description of Desired variable 
 *				double *converted_value,	Pointer to the Converted value 
 *				FORMAT_PTR input_format,	Input Format Description 
 *				char   *input_buffer)		Input Buffer 
 *								   
 * DESCRIPTION: The NOAA earthquake database includes two files for each
 *				catalog. These files are called the bit mask and the data
 *				mask. The information in both files is held in packed
 *				binary fields. This conversion function reads the bit mask
 *				format and extracts the various earthquake parameters.
 *
 * COMMENTS: 
 *
 * RETURNS:	0 if unsuccessful, 1 if successful
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_noaa_eq"

int cv_noaa_eq(VARIABLE_PTR var,		/* Description of Desired variable */
	double *converted_value,			/* Pointer to the Converted value */
	FORMAT_PTR input_format,			/* Input Format Description */
	FF_DATA_BUFFER input_buffer)		/* Input Buffer */

#define NUM_OUTPUT_NAMES 19

{
	long HUGE	*bit_mask	= NULL;
	long HUGE	*data_mask	= NULL;

	short 		year;		/* Year of Earthquake                   */
	short 		day;		/* Day of Earthquake                    */
	short 		hour;		/* Hour of Earthquake                   */
	short 		minute;		/* minute of Earthquake                 */
	short 		b_mag1;		/* Body Wave Magnitude                  */
	short 		b_mag2;		/* Body Wave Magnitude                  */
	short 		s_mag1;		/* S Wave Magnitude                     */
	short 		s_mag2;		/* S Wave Magnitude                     */
	short 		o_mag1;		/* Other  Magnitude                     */
	short 		o_mag2;		/* Other  Magnitude                     */
	short 		l_mag1;		/* Local  Magnitude                     */
	short 		l_mag2;		/* Local  Magnitude                     */
	short 		fe_reg;		/* Flinn Engdahl Region                 */
	short 		index;		/* Source Code                          */
	short 		intens;		/* FELT Intensity                       */
	short 		zh;			/* Z/H COMPONENT                        */
	long 		no_stat;	/* number of stations                   */
	long 		co_lat;		/* Co-latitude of earthquake		*/
	long 		east_lon;	/* East longitude of earthquake		*/
	long 		time;		/* Time code (contains year and month)  */
	float 		second;		/* second of Earthquake                 */
	float 		depth;		/* depth of Earthquake                  */
	float 		b_mag;		/* Body Wave Magnitude                  */
	float 		ms;			/* S Wave Magnitude                     */
	float 		o_mag;		/* Other Magnitude                    	*/
	float 		l_mag;		/* Local Magnitude                     	*/
	char 		culture;	/* Cultural Effects                     */

	VARIABLE_PTR var_source	= NULL;


 	char *output_names[NUM_OUTPUT_NAMES + 1] = {
		"cultural",		/* 0 */
		"day",			/* 1 */
		"depth",		/* 2 */
		"fe_region",	/* 3 */
		"hour",			/* 4 */
		"intensity",	/* 5 */
		"latitude",		/* 6 */
		"longitude",	/* 7 */
		"magnitude_mb",	/* 8 */
		"magnitude_ml",	/* 9 */
		"magnitude_mo",	/* 10 */
		"magnitude_ms",	/* 11 */
		"minute",		/* 12 */
		"month",		/* 13 */
		"no_station",	/* 14 */
		"second",		/* 15 */
		"source_code",	/* 16 */
		"year",			/* 17 */
		"zh_component",	/* 18 */

		/* TO ADD A NAME: (1)increase number_of_names
						  (2)preserve alphabetical order
						  (3)match case statement to index */
		NULL,
	};

	int 	name_number    	= -1;
	char 	**base  		= output_names;
	char 	*result 		= NULL;

	/* Check to see if a NOAA bit mask exists in the input format */
	var_source = ff_find_variable("cv_noaa_eq_bmask", input_format);
	if (var_source == NULL)
		return(0);

	/*
	** search for the output variable
	*/
	
	result = (char *)bsearch((const void *)var->name, (const void *)base,
	                         (size_t)NUM_OUTPUT_NAMES, sizeof(char *), b_strcmp);

	if (result)
		name_number = (result - (char *)base) / (sizeof(char *));
	else
		return(0);


	/*
	** Initialize the return value and data pointer
	*/

	*converted_value = 0.0;
	bit_mask = (long HUGE *)input_buffer;
	data_mask = (long *)input_buffer+4;

	/*
	** switch on index and convert
	*/

	switch (name_number)
	{
	 	case 0:								/* CULTURAL EFFECTS  */
			culture = (char)((*(data_mask + 2) ) & 0x0007);
			*converted_value = (char)(culture);
		break;

	  case 1:								/* DAY   */
	    day = (short)((*(data_mask + 0) >> 27) & 0x001F);
			*converted_value = (short)(day);
		break;

		case 2:								/* DEPTH */
			depth = (float)((*(bit_mask + 1) >> 15) & 0x03FF);
			if (depth == 1023)
				depth = 0.0F;
			*converted_value = (float)(depth);
		break;

	  case 3:				/* FE_REGION - FLYNN-ENGDAHL REG*/
			fe_reg  = (short)((*(bit_mask + 1) >> 5) & 0x03FF);
			*converted_value = (short)(fe_reg);
		break;
		
		case 4:								/* HOUR  */
			hour = (short)((*(data_mask + 0) >> 22) & 0x001F);
			
			if (hour !=31)
			{
				hour = (short)hour;
				*converted_value = (short)(hour);
			}
		break;

		case 5:								/* FELT INTENSITY  */
			intens  = (short)((*(bit_mask + 0) >> 11) & 0x000F);
			*converted_value = (short)(intens);
		break;
	
		case 6:							/* LATITUDE */
			co_lat = (long)(*(bit_mask + 2) >> 13);
			if (co_lat > 90000L)
			{
				co_lat -= 90000L;
				co_lat *= -1;
			}
			else
				co_lat = 90000L - co_lat;

			*converted_value = co_lat / 1000.0;
		break;

		case 7:							/* LONGITUDE */
			east_lon = (long)((*(bit_mask + 3) >> 12) & 524287L);
			if (east_lon > 180000L)
			{
				*converted_value = (double)(360000L - east_lon);
				*converted_value /= 1000.0;
			}
			else
				*converted_value = (double)(east_lon) / 1000.0;
		break;

		case 8:				/* MAGNITUDE_MB - BODY WAVE MAGNITUDE*/
		  b_mag1  = (short)((*(bit_mask + 2) >> 7) & 0x003F);
		  b_mag2  = (short)((*(data_mask + 1) >> 27) & 0x001F);
		
		if(b_mag1)
			b_mag = (float)(25 *b_mag1 + b_mag2 -300)/100.0F;
		else
			b_mag = 0.0F;
		*converted_value = (float)(b_mag);
		break;
		
		case 9:				/* MAGNITUDE_ML - LOCAL  MAGNITUDE*/
			l_mag1  = (short)((*(bit_mask + 3) >> 6) & 0x003F);
			l_mag2  = (short)((*(data_mask + 2) >> 27) & 0x001F);
						
			if (l_mag1)
				l_mag = (float)(25*l_mag1 + l_mag2 -300)/100.0F;
			else
				l_mag = 0.0F;
			*converted_value = (float)(l_mag);
			break;
						
		case 10:			/* MAGNITUDE_MO - OTHER  MAGNITUDE*/
			o_mag1  = (short)(*(bit_mask + 3)  & 0x003F);
			o_mag2  = (short)((*(data_mask + 1) >> 18) & 0x001F);
					
			if (o_mag1)
				o_mag = (float)(25* o_mag1 + o_mag2 -300)/100.0F;
			else 
				o_mag = 0.0F;
					
			*converted_value = (float)(o_mag);
			break;
									
		case 11:				/* MAGNITUDE_MS - S WAVE MAGNITUDE*/
			s_mag1  = (short)((*(bit_mask + 2) >> 1) & 0x003F);
			s_mag2  = (short)((*(data_mask + 1) >> 3) & 0x000F);
					
			if (s_mag1)
				ms = (float)((25*s_mag1 + s_mag2 *  5 -300) /10) /10.0F;
			else
				ms = 0.0F;
			*converted_value = (float)(ms);
		break;
					
		case 12:							/* MINUTE */
			minute = (short)((*(data_mask + 0) >> 16) & 0x003F);
			if (minute != 63)
				*converted_value = (short)(minute);
		break;
		
		case 13:							/* MONTH */
			time = (long)((*(bit_mask + 0) >> 15) & 0xFFFF);
			year =  (short)(time / 13);
			*converted_value = (short)(time -  year *13);
		break;
						
		case 14:				/* NO_STATION - NUMBER of STATIONS*/
			no_stat = (*(data_mask + 3) >> 17) & 0x03FFF;
						    
			*converted_value = (long)(no_stat);
		break;
		
		case 15:							/* SECOND */
		
			second = (float)((*(data_mask + 0) >> 6) & 0x03FF);
					
			if (second != 1023)
			{
				if (second >600)
					second -=600;
				else second = second/10.0F;
			}
			if (second ==1023)
				second = 0.0F;
			*converted_value = (float)(second);
		break;
						
		case 16:							/* SOURCE CODE       */
			index   = (short)((*(data_mask + 3) >> 8) & 0x01FF);
			*converted_value = (int)(index);
		break;
				
		case 17:							/* YEAR */
			time = (*(bit_mask + 0) >> 15) & 0xFFFF;
					
			*converted_value = (short)((time / 13)-2100);
		break;
						
		case 18:							/* Z/H COMPONENT     */
			switch  ((int)((*(data_mask +3) >>5) & 0x0007))
			{
				case 1 :
					zh = 'H';
				break;

				case 2 :
					zh = 'Z';
				break;

			default: zh =  '\0';
			}
					
			*converted_value = (int)(zh);
			break;
		
		default:
			sprintf(input_buffer,"Problem with switch in NOAAEQ functions\n");

		return(0);
		
	} /* end switch */

	return(1);

} /* End cv_noaa_eq() */

 
/*
 * NAME:	b_strcmp
 *
 * PURPOSE:	compare function for bsearch routine
 *	
 * AUTHOR: TAM
 *
 *
 * RETURNS:	0 if same else the difference between sizes of strings
 *			see strcmp()
 *
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "b_strcmp"

int b_strcmp(const void *s1, const void *s2 )
{
	return(strcmp((char *)s1, *((char **)s2)));
}

/*
 * NAME:	cv_sea_flags
 *
 * PURPOSE:	This is a conversion function for the seismicity data from
 *			the University of Washington.
 *  
 *			Conversions which are included:
 *			 
 *			Convert AType into				cultural
 *			            or 					ngdc_flags
 *			Convert depth_control_code into depth_control
 *			
 * AUTHOR:
 *
 * USAGE:	cv_sea_flags(
 *				VARIABLE_PTR out_var, 
 *				double *dummy, 
 *				FORMAT_PTR input, 
 *				FF_DATA_BUFFER input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_sea_flags"

int cv_sea_flags(VARIABLE_PTR out_var, double *dummy, FORMAT_PTR input, FF_DATA_BUFFER input_buffer)
{
	int i;
	char char_buffer[8];
	char *ch_ptr;

	VARIABLE_PTR in_var;
	FF_DATA_PTR data_source;

	/* Initialize the variable destination (a double) to blanks */
	for(i = 8, ch_ptr = (char *)dummy; i; i--, ++ch_ptr) *ch_ptr = ' ';
	ch_ptr = (char *)dummy;

	if(memStrcmp(out_var->name, "cultural",NO_TAG) == 0 ||
		memStrcmp(out_var->name, "ngdc_flags",NO_TAG) == 0){

		/* Check the input format to make sure that AType is present */
		in_var = ff_find_variable("AType", input);
		if(!in_var){
			return(0);
		}

		/* Get the input variable */
		/* char buffer is a character array which is in the same location as
		the double where the converted variable is to be placed. This allows
		us to use the same space to return a string as would be used to
		return the double. NOTE THAT THIS CAUSES PROBLEMS IF YOU ARE
		TRYING TO GET MORE THAN 8 BYTES!!! */

		data_source = (void HUGE *)(input_buffer + in_var->start_pos - 1);
		(void)ff_get_value(in_var, data_source, &char_buffer, input->type);
	
		/* Event Felt, ch_ptr points to cultural */
		if(memStrcmp(out_var->name, "cultural",NO_TAG) == 0){
			if(memStrcmp(char_buffer, "F",NO_TAG) == 0){
				*ch_ptr = 'F';
				return(1);
			}
		}
		else {		/* setting ngdc_flags */
			if(memStrcmp(char_buffer, "L",NO_TAG) == 0)	/* Volcanic, ch_ptr points to ngdc_flags */
				*(ch_ptr + 3) = 'V';

			if(memStrcmp(char_buffer, "X",NO_TAG) == 0 || memStrcmp(char_buffer, "P",NO_TAG) == 0)
	   			*(ch_ptr + 4) = 'E';
			return(1);
		}
	}
	if(memStrcmp(out_var->name, "depth_control",NO_TAG) == 0){

		/* Check the input format to make sure that depth_control is present */
		in_var = ff_find_variable("depth_control", input);
		if(!in_var){
			return(0);
		}

		/* Get the input variable */
		data_source = (void HUGE *)(input_buffer + in_var->start_pos - 1);
		(void)ff_get_value(in_var, data_source, &char_buffer, input->type);

		if(memStrcmp(char_buffer, "*",NO_TAG) == 0){
				*ch_ptr = 'G';
				return(1);
		}
		if(memStrcmp(char_buffer, "$",NO_TAG) == 0){
				*ch_ptr = '?';
				return(1);
		}
		if(memStrcmp(char_buffer, "#",NO_TAG) == 0){
				*ch_ptr = '?';
				return(1);
		}
	}
	return(0);
}
		
/*
 * NAME:	cv_slu_flags
 *
 * PURPOSE:	This is a conversion function for the seismicity data from
 *			the St. Louis University.
 *
 *			Conversions which are included:
 *			 			
 *			Convert slu_line2 into			non_tectonic
 *			Convert slu_line2 into			cultural
 *			Convert slu_line2 into			intensity
 *			Convert slu_line2 into			magnitude_ml
 *			Convert slu_line2 into			scale
 *			Convert slu_line2 into			ml_authority
 *			
 * AUTHOR:
 *
 * USAGE:	cv_slu_flags(
 *				VARIABLE_PTR out_var, 
 *				double        *dummy, 
 *				FORMAT_PTR    input, 
 *				FF_DATA_BUFFER   input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_slu_flags"

int cv_slu_flags(VARIABLE_PTR out_var, double *dummy, FORMAT_PTR input, FF_DATA_BUFFER input_buffer)
{
	VARIABLE_PTR 	in_var;
	FF_DATA_PTR 		data_source 	= NULL;
	int 			i;
	unsigned 		length;
	char HUGE		*ch_ptr 	= NULL;
	char HUGE		*dummy_ch_ptr 	= NULL;
/*	char _temp[100]; */

	/* Initialize the variable destination (a double) to blanks */
	for(i = 8, dummy_ch_ptr = (char *)dummy; i; i--, ++dummy_ch_ptr) *dummy_ch_ptr = ' ';
	dummy_ch_ptr = (char *)dummy;

	/* Make the non-tectonic flag from the comment line */
	if(memStrcmp(out_var->name, "non_tectonic",NO_TAG) == 0){

		/* Check the input format to make sure that slu_line2 is present */
		in_var = ff_find_variable("slu_line2", input);
		if(!in_var){
			return(0);
		}

		length = in_var->end_pos - in_var->start_pos + 1;

		/* Get the input variable */
		data_source = (void HUGE *)(input_buffer + in_var->start_pos - 1);
		ch_ptr = ff_strnstr("BLAST", (char *)data_source, length);
		if(ch_ptr) *(dummy_ch_ptr) = 'E';
		else *(dummy_ch_ptr) = ' ';
		return (1);
	}

	/* Make the cultural flag from the comment line */
	if(memStrcmp(out_var->name, "cultural",NO_TAG) == 0){

		/* Check the input format to make sure that slu_line2 is present */
		in_var = ff_find_variable("slu_line2", input);
		if(!in_var){
			return(0);
		}

		length = in_var->end_pos - in_var->start_pos + 1;

		/* Get the input variable */
		data_source = (void HUGE *)(input_buffer + in_var->start_pos - 1);
		ch_ptr = ff_strnstr("FELT", (char *)data_source, length);
		if(ch_ptr) *(dummy_ch_ptr) = 'F';
		else *(dummy_ch_ptr) = ' ';
		return (1);
	}

	/* Make the intensity from the comment line */
	if(memStrcmp(out_var->name, "intensity",NO_TAG) == 0){

		/* Check the input format to make sure that slu_line2 is present */
		in_var = ff_find_variable("slu_line2", input);
		if(!in_var){
			return(0);
		}

		length = in_var->end_pos - in_var->start_pos + 1;

		/* Get the input variable */
		data_source = (void HUGE *)(input_buffer + in_var->start_pos - 1);
		ch_ptr = ff_strnstr("MM ", (char *)data_source, length);
		if(ch_ptr) *(dummy_ch_ptr) = *(ch_ptr + 3);
		else *(dummy_ch_ptr) = ' ';
		return (1);
	}

	/* Make the magnitude_ml from the comment line */
	/* There are a number of potential magnitudes in the comments which
	might be selected as ML. The most consistent selection, suggested by Bob
	Hermann 4/30/91, is to take any magnitude with FVM (FVMZ in the original
	catalog). These are magnitudes from the French Village vertical
	instrument. We select these first. If they do not exist we look for
	MD, duration magnitudes calculated by CERI. IF BOTH FVM and MD EXIST
	the MD is ignored.
	*/

	if(memStrcmp(out_var->name, "magnitude_ml",NO_TAG) == 0){

		/* Check the input format to make sure that slu_line2 is present */
		in_var = ff_find_variable("slu_line2", input);
		if(!in_var){
			return(0);
		}

		length = in_var->end_pos - in_var->start_pos + 1;

		/* Get the input variable */
		data_source = (void HUGE *)(input_buffer + in_var->start_pos - 1);

		/* Check for FVM */
		ch_ptr = ff_strnstr("FVM", (char *)data_source, length);
		if(ch_ptr){
#ifdef MEDIUM
			_fmemMemcpy(_temp, ch_ptr-5, 99);
			*(_temp+99)='\0';	
			if(*(ch_ptr - 5) == ' ')*(dummy) = strtod(_temp+1, NULL);
			else *(dummy) = strtod(_temp , NULL);

#else
			if(*(ch_ptr - 5) == ' ')*(dummy) = strtod(ch_ptr - 4, NULL);
			else *(dummy) = strtod(ch_ptr - 5, NULL);
#endif
			return (1);
		}


		ch_ptr = ff_strnstr("MD ", (char *)data_source, length);
		if(ch_ptr)
#ifdef MEDIUM
			_fmemMemcpy(_temp, ch_ptr+3, 99);
			*(_temp+99)='\0';	
			*(dummy) = strtod(_temp, NULL);

#else
			*(dummy) = strtod(ch_ptr + 3, NULL);
#endif

		return (1);
	}

	/* Make the scale from the comment line, (SEE ML NOTES ABOVE) */
	if(memStrcmp(out_var->name, "scale",NO_TAG) == 0){

		/* Check the input format to make sure that slu_line2 is present */
		in_var = ff_find_variable("slu_line2", input);
		if(!in_var){
			return(0);
		}

		length = in_var->end_pos - in_var->start_pos + 1;

		/* Get the input variable */
		data_source = (void  HUGE *)(input_buffer + in_var->start_pos - 1);

		/* Check for FVM */
		ch_ptr = ff_strnstr("FVM", (char *)data_source, length);
		if(ch_ptr){
			*(dummy_ch_ptr) = 'L';
			*(dummy_ch_ptr + 1) = 'G';
			return (1);
		}

		ch_ptr = ff_strnstr("MD ", (char *)data_source, length);
		if(ch_ptr){
			*(dummy_ch_ptr) = 'D';
			*(dummy_ch_ptr + 1) = 'R';
		}
		return (1);
	}

	/* Make the authority from the comment line */
	if(memStrcmp(out_var->name, "ml_authority",NO_TAG) == 0){

		/* Check the input format to make sure that slu_line2 is present */
		in_var = ff_find_variable("slu_line2", input);
		if(!in_var){
			return(0);
		}

		length = in_var->end_pos - in_var->start_pos + 1;

		/* Get the input variable */
		data_source = (void HUGE *)(input_buffer + in_var->start_pos - 1);

		/* Check for FVM */
		ch_ptr = ff_strnstr("FVM", (char *)data_source, length);
		if(ch_ptr){
			*(dummy_ch_ptr) = 'S';
			*(dummy_ch_ptr + 1) = 'L';
			*(dummy_ch_ptr + 2) = 'M';
			return (1);
		}
		ch_ptr = ff_strnstr("MD ", (char *)data_source, length);
		if(ch_ptr){
			*(dummy_ch_ptr) = 'T';
			*(dummy_ch_ptr + 1) = 'E';
			*(dummy_ch_ptr + 2) = 'I';
		}
		return (1);
	}
	return(0);
}
