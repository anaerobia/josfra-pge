/* 
 *
 * CONTAINS:	cv_geo44tim
 *
 */

/*
 * NAME:	cv_geo44tim
 *
 * PURPOSE:	This file contains the function which is used to convert the time in
 *			arbitrary seconds to a time offset for the GEO44 data. The offset is
 *			calculated from the time which is in the header record.
 *								
 * AUTHOR:	T. Habermann (303) 497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	Time is converted in arbitrary seconds to a time offset for the
 *			GEO44 data. The offset is calculated from the time which is in
 *			the header record.
 *
 *			Header records are identified by a 1000 in the gravity_uncertainty
 *			variable.
 *
 *			If a header record is being processed, change variables: 
 *
 *				The uncertainty needs to be set to 10 so that the integer
 *				representation becomes 1000.
 *
 *				The gravity anomaly needs to be reduced by a factor of 100 so
 *				that its integer value is correct.
 *
 *			cv_geo44tim(
 *				VARIABLE_PTR out_var,
 *				double        *offset, 
 *				FORMAT_PTR    input, 
 * 				FF_DATA_BUFFER   input_buffer)
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
#define ROUTINE_NAME "cv_geo44tim"

int cv_geo44tim(VARIABLE_PTR out_var, double *offset, FORMAT_PTR input, FF_DATA_BUFFER input_buffer)
{
	VARIABLE_PTR 	var			= NULL;
	FF_DATA_PTR 		data_source = NULL;

	char HUGE 			*decimal_pt	= NULL;

	char 				*ten 		= {"  10.00"};

	static double 		start_time 	= 0.0;
	double 				d_value		= 0.0;
	
	/* Get the value of the time */
	var = ff_find_variable("time_seconds", input);
	data_source = (void HUGE *)(input_buffer + var->start_pos - 1);
	(void) ff_get_value(var, data_source, (void HUGE *)&d_value, input->type);

	/* Get the value of the key */
	
	var = ff_find_variable("gravity_uncertainty", input);
	if(!var)return(0);

	data_source = (void HUGE *)(input_buffer + var->start_pos - 1);

	if(memStrncmp((char *)data_source,(char HUGE *)"1000", 4,NO_TAG)){		/* Not header */
		*offset = (d_value - start_time) / 0.489;
		return(1);
	}

	/* Deal with header */

	start_time = d_value;
	*offset = 0.0;

	var = ff_find_variable("gravity_uncertainty", input);
	memMemcpy(input_buffer + var->start_pos - 1, ten, 7,NO_TAG);

	var = ff_find_variable("gravity_anom", input);

#ifdef MEDIUM
	decimal_pt = _fmemStrchr(input_buffer + var->start_pos - 1, '.',NO_TAG);

#else
	decimal_pt = memStrchr(input_buffer + var->start_pos - 1, '.',NO_TAG);
#endif

	memMemmove(decimal_pt + 1, decimal_pt - 2, 2,NO_TAG);
	memMemmove(decimal_pt - 2, decimal_pt - 4, 2,NO_TAG);
	*(decimal_pt - 4) = *(decimal_pt - 3) = ' ';
	if(*(decimal_pt + 1) == ' ') *(decimal_pt + 1) = '0';

	return(1);
}
