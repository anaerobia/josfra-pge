/* 
 *	This file contains functions which are used to convert between
 *	various representations of time.
 *
 *	CONTAINS:
 *
 *	FUNCTION			Convert From:			to:
 *							 
 *	cv_ymd2ser		year, month, day		serial_day_1980 (1980 = 0)
 *						hour, minute, second    OR year_decimal
 *							
 *	cv_ser2ymd		serial_day_1980				year, month, day,
 *	                                                                hour, minute, second
 *
 * cv_ydec2ymd    year_decimal                   year, month, day,
 *	                                                                hour, minute, second
 *
 *	cv_ipe2ser		ipe_date				serial_day_1980 (1980 = 0)
 *                  	
 *	cv_ser2ipe		serial_day_1980 (1980 = 0)	ipe_date
 *
 *	cv_date_string	day, month, year OR		date_m/d/y	OR
 *						date_m/d/y 		 OR		date_yymmdd
 *						date_yymmdd
 *
 *
 *	cv_time_string	hour, minute, second OR	time_h:m:s
 *						time_h:m:s			 OR	time_hhmmss
 *						time_hhmmss
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <freeform.h>

/* Variables Used in All Functions:	*/
VARIABLE_PTR	in_var		= NULL;
static char	*last_underscore	= NULL;
static char scratch_buffer[256];
static double last_input_value = -989898.989898;

static short days_per_month[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static short days_per_leap_month[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

#define FF_TIME_VAR_YEAR 0
#define FF_TIME_VAR_MONTH 1
#define FF_TIME_VAR_DAY 2
#define FF_TIME_VAR_HOUR 3
#define FF_TIME_VAR_MINUTE 4
#define FF_TIME_VAR_SECOND 5
#define FF_TIME_VAR_CENTURY_AND_YEAR 6
#define FF_TIME_VAR_CENTURY 7


/*
 * NAME:  	cv_ymd2ser
 *
 * PURPOSE:	This function converts a time given as year, month, day, hour,
 *			minute, second into a serial_day_1980 data with January 1, 1980 as 0,
 *			or into year_decimal.
 * 
 * AUTHOR:	The code was modified from that given by Michael Covington,
 *			PC Tech Journal, December, 1985, PP. 136-142.
 *			Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_ymd2ser(VARIABLE_PTR out_var, 
 *			double		*serial_day_1980, 
 *			FORMAT_PTR	input_format,
 *			FF_DATA_BUFFER	input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_ymd2ser"

int cv_ymd2ser(VARIABLE_PTR out_var, double *serial_day_1980, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{
	short	century = 0;
	short	year = 0;
	short	month = 0;
	short	day = 0;
	short	hour = 0;
	short	minute = 0;
	int		jul_mon = 0;
	int		centuries = 0;
	int		real_leap_yrs = 0;
	int		correct = 0;
	long	jul_year = 0;
	double second = 0.0;
	double decimal, jul_num;

	short got_data = 0;

	/* Define the Variables */
	
	/* Check input format to see if variables are present */

	in_var = ff_find_variable("century", input_format);	/* Get the century */
	if(in_var){
		++got_data;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
		scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
		(void) ff_get_double(in_var, scratch_buffer, &decimal, input_format->type);
		century = (short)decimal;
	}
	
	in_var = ff_find_variable("year", input_format);	/* Get the year */
	if(in_var){
		++got_data;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
		scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
		(void) ff_get_double(in_var, scratch_buffer, &decimal, input_format->type);
		year = (short)decimal;
		if (year < 100)
		{
			if (century == 0)
				; /* no change */
			else if (century > 0)
				year += century * (short)100;
			else
				year = century * 100 - year;
		}
	}
	else
	{		/* Year not found */
		in_var = ff_find_variable("century_and_year", input_format);	/* Get the century_and_year */
		if(in_var){
			++got_data;
			memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
			scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
			(void) ff_get_double(in_var, scratch_buffer, &decimal, input_format->type);
			year = (short)decimal;
		}
		else	return(0);	/* Can't Do Conversion */
	}

	in_var = ff_find_variable("month", input_format);	/* Get the month */
	if(in_var){
		++got_data;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
		scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
		(void) ff_get_double(in_var, scratch_buffer, &decimal, input_format->type);
		month = (short)decimal;
	}
	in_var = ff_find_variable("day", input_format);	/* Get the day */
	if(in_var){
		++got_data;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
		scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
		(void) ff_get_double(in_var, scratch_buffer, &decimal, input_format->type);
		day = (short)decimal;
	}

	in_var = ff_find_variable("hour", input_format);	/* Get the hour */
	if(in_var){
		++got_data;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
		scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
		(void) ff_get_double(in_var, scratch_buffer, &decimal, input_format->type);
		hour = (short)decimal;
	}

	in_var = ff_find_variable("minute", input_format);	/* Get the minute */
	if(in_var){
		++got_data;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
		scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
		(void) ff_get_double(in_var, scratch_buffer, &decimal, input_format->type);
		minute = (short)decimal;
	}

	in_var = ff_find_variable("second", input_format);	/* Get the second */
	if(in_var){
		++got_data;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
		scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
		(void) ff_get_double(in_var, scratch_buffer, &second, input_format->type);
	}
	
	/* Several of the time variables can not have values of 0 (i.e. month and day). If these
		variables do have these values, it is assumed that they are unspecified. For example,
		we know that an event occurred during a specific year (month), but do not know
		any more. This case can also occur if we know the year, month, and day,
		but not the time of day. In these cases, all unknown variables are set to 1. */

	if (   day == 0
	    || (hour == 0 && minute == 0 && second == 0)
	   )
	{
		if (century == 0) century = 1;
		if (year == 0) year = 1;
		if (month == 0) month = 1;
		if (day == 0) day = 1;
		hour = 1;
		minute = 1;
		second = 1.0;
	}
	
	decimal  = second / 60.0;
	decimal += minute;
	decimal /= 60.0;
	decimal += hour;
	decimal /= 24.0;
	
 	/* Calculate decimal year if that is the output variable name */
 	if (!strcmp(out_var->name, "year_decimal"))
 	{
 		day += days_per_month[month - 1];
 		decimal += day - 1;
 		
 		/* Leap years */
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		{
			if (month >= 3)
 				decimal += 1.0;
			*serial_day_1980 = year + ((year >= 0) ? 1 : -1) * (decimal / 366.0);
 		}
 		else
 			*serial_day_1980 = year + ((year >= 0) ? 1 : -1) * (decimal / 365.0);

 		return(got_data);
 	}

	if (0 < month && month < 3)
	{
		month += 12;
		year  -= 1;
	}

	jul_year = (long)(365.25 * year + 0.1);

	jul_mon = (int)(30.6001 * (month + 1) - 63);

	jul_num = jul_year + jul_mon + day - 723182.0;

	/* convert to Gregorian calendar */
	centuries = (int)floor(year / 100.0); /* The number of centuries counted (including the year 0)*/
	real_leap_yrs = (int)floor(centuries / 4.0); /* The number of centuries that were leap years (divisible by 400) */
	correct = 2 - centuries + real_leap_yrs;
	*serial_day_1980 = jul_num + correct + decimal;

	return(got_data);
}


/*
 * NAME:	cv_ser2ymd
 *
 * PURPOSE:	This function converts a time given as either a serial_day_1980 date
 *			with January 1, 1980 as 0 (serial_day_1980) or an IPE date in minutes AD
 *			into year, month, day, hour, minute, second.
 *
 * AUTHOR:	The code was modified from that given by Michael Covington,
 *			PC Tech Journal, December, 1985, PP. 136-142.
 *			Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_ser2ymd(
 *				VARIABLE_PTR out_var,
 *				double        *conv_var, 
 *				FORMAT        *input_format,
 *				char          *input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_ser2ymd"

int cv_ser2ymd(VARIABLE_PTR out_var, double *conv_var, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{
	char *output_names[] = {
		"year",
		"month",
		"day",
		"hour",
		"minute",
		"second",
		"century_and_year",
		"century",

		/* DON'T EVEN THINK OF ADDING SOMETHING HERE WITHOUT INCREASING
		THE number_of_outputs. */
	};

	unsigned char number_of_outputs = 8;
	
	unsigned char ipe = 0;
	unsigned char leap = 0;
	unsigned char output_variable = 0;
	
	int		centuries = 0;
	int		real_leap_yrs = 0;
	int		correct = 0;
		
	double serial_day_1980;
	double decimal;
	double fprior_days;
	double raw_mon;
	double year, month, day, hour, minute, second;
    		
	short prior_days, extra_days, mon_const;
    short int_year, int_month, int_hour, int_minute, century;
	
	long adj_serial_day_1980, base_days, whole_serial_day_1980;  
	
/*	int int_day; */

	/* Define the Variables */
	
	/* Check input format to see if variables are present */

	in_var = ff_find_variable("serial_day_1980", input_format);	/* Get the serial date */

	if(!in_var){
		/* Check for old variable name */
		in_var = ff_find_variable("serial", input_format);
		if(!in_var)	return (0);
	}
	
	if(!in_var){
		/* Check for IPE date */
		in_var = ff_find_variable("ipe_date", input_format);
		if(!in_var)	return (0);
		ipe = 1;
	}

	memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
	scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
	(void)ff_get_double(in_var, scratch_buffer, (double *)&serial_day_1980, input_format->type);

	if(ipe){		/* Convert ipe date to serial_day_1980 date
					Define time: 1,040,874,840 = number of minutes
					to end of 1979 1440 minutes / day */
		serial_day_1980 = (serial_day_1980 - 1040874840) / 1440.0;
	}

	whole_serial_day_1980 = (long)(floor(serial_day_1980));

	/* Now the serial_day_1980 number is known, define the output variable */
	for(output_variable = 0; output_variable < number_of_outputs; ++output_variable){
		if (!strcmp(out_var->name, output_names[output_variable]))
			break;
	}

	/* Now begin the extraction of the output variables and exit
	when appropriate. There are two major branches: one deals with
	portions of a day and one with units larger than one day */
	
	switch(output_variable)
	{
	case FF_TIME_VAR_YEAR:
	case FF_TIME_VAR_CENTURY_AND_YEAR:
	case FF_TIME_VAR_MONTH:
	case FF_TIME_VAR_DAY:
	case FF_TIME_VAR_CENTURY:
        
        /* 28855 days in this century before 1980 */
		adj_serial_day_1980 = whole_serial_day_1980 + 28855;

		year = 1901+ adj_serial_day_1980 / 365.25;
		
		/* Now year is known, apply Gregorian correction */
		centuries = (int)floor((1980 - year) / 100.0); /* The number of centuries counted (including the year 0)*/
		real_leap_yrs = (int)floor(centuries / 4.0); /* The number of centuries that were leap years (divisible by 400) */
		correct = centuries - real_leap_yrs;
		adj_serial_day_1980 -= correct;
		
		int_year = (short) (year + DOUBLE_UP);
		base_days = (long)((int_year - 1901) * 365.25 + DOUBLE_UP);
		extra_days = (short) (adj_serial_day_1980 - base_days);	/* days since beginning of year */
		if (int_year % 4 == 0 && int_year % 100 != 0 || int_year % 400 == 0)leap = 1; 
		if ((int)extra_days > 59 + (int)leap){ 	/*  month is not Jan or Feb */
			mon_const = 1;
			extra_days -= (short)leap;
		}
		else{					/* if month is Jan or Feb, add 12 to month so
								that Feb 29 will be last day of year */
			extra_days += 365;
			mon_const = 13;
		}

		raw_mon = (extra_days + 63) / 30.6001;
		month = raw_mon - mon_const;
		int_month = (short)(month + DOUBLE_UP);

		fprior_days = (float)((int_month + mon_const) * 30.6001 - 63);
		prior_days = (short)fprior_days; /* days between beg. of yr & beg. of current mon.*/
		day = extra_days - prior_days;

		if(day == 0 && int_month == 13){	/* Dec 31 of leap years is treated specially */
			day = 31;
			month = 12.0;
			int_month = 12;
		}

		/* Times during the last minute of the year sometimes have problems */
		if(day == 31 && int_month == 0){
			month = 12.0;
			int_month = 12;
			year -= 1.0;
			int_year -= 1;
		}

		/* The time variables are special conversions.
		If they are integer variables with a precision of 0,
		truncate at this point.  */
		
		switch(output_variable){
		case FF_TIME_VAR_CENTURY:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  (short)(year / 100.0 + DOUBLE_UP);
			else
				*conv_var = year/100.0;
			return(1);
			
		case FF_TIME_VAR_YEAR:
			century =  (short)(year / 100.0) * (short)100;
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_year - century;
			else
				*conv_var = year - century;
			return(1);
			
		case FF_TIME_VAR_CENTURY_AND_YEAR:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_year;
			else
				*conv_var = year;
			return(1);
			
		case FF_TIME_VAR_MONTH:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_month;
			else
				*conv_var = month;
			return(1);
			
		case FF_TIME_VAR_DAY:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  (short)(day + DOUBLE_UP);
			else
				*conv_var = day;
			return(1);
		}

	case FF_TIME_VAR_HOUR:
	case FF_TIME_VAR_MINUTE:
	case FF_TIME_VAR_SECOND:

		decimal = serial_day_1980 - whole_serial_day_1980;

		hour = decimal * 24.0;
		int_hour = (short)(hour + DOUBLE_UP);
		if(int_hour == 24){
			hour = 0.0;
			int_hour = 0;
		}

        decimal = hour - int_hour;
		minute = decimal * 60.0;
		int_minute = (short)(minute + DOUBLE_UP);
				
        decimal = minute - int_minute;
        second = decimal * 60.0;
        if(second < 0.0) second = 0.0;

		switch(output_variable){
		case FF_TIME_VAR_HOUR:	/* hour */
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  int_hour;
			else
				*conv_var = hour;
			break;
			
		case FF_TIME_VAR_MINUTE: /* minute */
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_minute;
			else
				*conv_var = minute;
			break;
			
		case FF_TIME_VAR_SECOND: /* second */
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  (short)(second + DOUBLE_UP);
			else
				*conv_var = second;
			break;
		}
		return(1);
	}
	return(0);
}

/*
 * NAME:	cv_ydec2ymd
 *
 * PURPOSE:	This function converts a time given as decimal years
 *			into year, month, day, hour, minute, second.
 *
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_ydec2ymd(
 *				VARIABLE_PTR out_var,
 *				double        *conv_var, 
 *				FORMAT        *input_format,
 *				char          *input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_ydec2ymd"

int cv_ydec2ymd(VARIABLE_PTR out_var, double *conv_var, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{
	char *output_names[] = {
		"year",
		"month",
		"day",
		"hour",
		"minute",
		"second",
		"century_and_year",
		"century",

		/* DON'T EVEN THINK OF ADDING SOMETHING HERE WITHOUT INCREASING
		THE number_of_outputs. */
	};

	unsigned char number_of_outputs = 8;
	unsigned char output_variable = 0;

	double decimal, year_decimal;

	static double century_and_year, century, year, month, day, hour, minute, second;
	static short int_century_and_year, int_century, int_year, int_month, int_day, int_hour, int_minute;
	
	/* Define the Variables */
	/* Check input format to see if variables are present */
	in_var = ff_find_variable("year_decimal", input_format);	/* Get the serial date */
	if(!in_var)	return (0);

	memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
	scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
	(void)ff_get_double(in_var, scratch_buffer, (double *)&year_decimal, input_format->type);

	/* These functions keep track of the last value that they dealt with and only compute
	conversion variables if they do not match. */
	if(year_decimal != last_input_value){	/* Extract output variables. */

		last_input_value = year_decimal;
		
		int_century_and_year = (short)(year_decimal + DOUBLE_UP);
		century_and_year = (double)int_century_and_year;

		int_century = (short)century_and_year / (short)100;
		century =  (double) int_century;
		
		int_year = int_century_and_year - int_century * (short)100;
		if(century < 0) int_year *= -1;
		year =  (double) int_year;

		decimal = fabs(year_decimal - century_and_year);
		int_month = 0;
		
		/* determine if this is a leap year */
	 	if(int_century_and_year % 4 == 0 && int_century_and_year % 100 != 0 || int_century_and_year % 400 == 0){
			decimal *= 366.0;	/* Convert Decimal to days */
			while(decimal > days_per_leap_month[int_month])
				int_month++;
			if(int_month > 1)
				decimal -= days_per_leap_month[int_month - 1];
		} else {
			decimal *= 365.0;	/* Convert Decimal to days */
			while(decimal > days_per_month[int_month])
				int_month++;
			if(int_month > 1)
				decimal -= days_per_month[int_month - 1];
		}
		month = (double)int_month;

		/* 1 must be added to the day because there is no day 0 */
		day = decimal + 1;
		int_day = (short)(day + DOUBLE_UP);
		decimal = (day - int_day > 0.0) ? (day - int_day) * 24.0 : 0.0;	/* Convert decimal to days */
			
		hour = decimal;
		int_hour = (short) (hour + DOUBLE_UP);
		decimal = (hour - int_hour > 0.0) ? (hour - int_hour) * 60.0 : 0.0; /* Convert decimal to minutes */
	
		minute = decimal;
		int_minute = (short) (minute +DOUBLE_UP) ;
		decimal = (minute - int_minute > 0.0) ? (minute - int_minute) * 60.0 : 0.0; /* Convert decimal to seconds */
		
		second = decimal;
	} /* Done calculating times */
		
	/* Define the output variable */
	for(output_variable = 0; output_variable < number_of_outputs; ++output_variable){
		if(!memStrcmp(out_var->name, output_names[output_variable],NO_TAG))
			break;
	}
	switch(output_variable){
		case FF_TIME_VAR_CENTURY_AND_YEAR:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_century_and_year;
			else
				*conv_var = century_and_year;
			break;

		case FF_TIME_VAR_CENTURY:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  int_century;
			else
				*conv_var = century;
			break;
			
		case FF_TIME_VAR_YEAR:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_year;
			else
				*conv_var = year;
			break;
			
		case FF_TIME_VAR_MONTH:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_month;
			else
				*conv_var = month;
			break;
			
		case FF_TIME_VAR_DAY:
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  (short)(day + DOUBLE_UP);
			else
				*conv_var = day;
			break;

		case FF_TIME_VAR_HOUR:	/* hour */
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  int_hour;
			else
				*conv_var = hour;
			break;
			
		case FF_TIME_VAR_MINUTE: /* minute */
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var = int_minute;
			else
				*conv_var = minute;
			break;
			
		case FF_TIME_VAR_SECOND: /* second */
			if(IS_INTEGER(out_var) && out_var->precision == 0)
				*conv_var =  (short)(second + DOUBLE_UP);
			else
				*conv_var = second;
			break;
			
		default:
			return(0);
	}
	return(1);
}

/*
 * NAME:  	cv_ymd2ipe
 *
 * PURPOSE:	This function converts a time given as year, month, day, hour,
 *			minute, second into a date used by the Institute of the Physics
 *			of the Earth in Moscow. The time is in minutes A.D.
 * 
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_ymd2ipe(
 *				VARIABLE_PTR out_var, 
 *				double        *serial_day_1980, 
 *				FORMAT        *input_format,
 *				char          *input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_ymd2ipe"

int cv_ymd2ipe(VARIABLE_PTR out_var, double *serial_day_1980, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{

	/* This function uses the cv_ymd2ser function to create a serial_day_1980 date in
	days after January, 1980 and then converts that to the IPE Date.
	*/

	int error;

	error = cv_ymd2ser(out_var, serial_day_1980, input_format, input_buffer);

	if(error == 0) return(0);

	/* There are 1440 minutes in a day, so convert serial_day_1980 into minutes */
	*serial_day_1980 *= 1440.0;

	/* 1040874840 = number of minutes to end of 1979 */

	*serial_day_1980 += 1040874840;

	return(1);
}

/*
 * NAME:  	cv_ser2ipe
 *
 * PURPOSE:	This function converts a time given as serial_day_1980 date (1980 = 0)
 *			into a date used by the Institute of the Physics
 *			of the Earth in Moscow. The time is in minutes A.D.
 * 
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_ser2ipe(
 *				VARIABLE_PTR out_var, 
 *				double      *serial_day_1980, 
 *				FORMAT      *input_format,
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
#define ROUTINE_NAME "cv_ser2ipe"

int cv_ser2ipe(VARIABLE_PTR out_var_never_used, double *serial_day_1980, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{

	/* Find the serial_day_1980 variable */
	in_var = ff_find_variable("serial_day_1980", input_format);	/* Get the serial date */

	if(!in_var){
		/* Check for old variable name */
		in_var = ff_find_variable("serial", input_format);
		if(!in_var)	return (0);
	}

	if(!in_var) return (0);

	memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
	scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
	(void)ff_get_double(in_var, scratch_buffer, (double *)serial_day_1980, input_format->type);
	
	/* There are 1440 minutes in a day, so convert serial_day_1980 into minutes */
	*serial_day_1980 *= 1440.0;

	/* 1040874840 = number of minutes to end of 1979 */

	*serial_day_1980 += 1040874840;
	
	/* Truncate to avoid rounding */
	*serial_day_1980 = (long)*serial_day_1980;

	return(1);
}

/*
 * NAME:  	cv_ipe2ser
 *
 * PURPOSE:	This function converts a date used by the Institute of the
 * 			Physics	of the Earth in Moscow into a time given as serial_day_1980
 *			date (1980 = 0).
 * 
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_ipe2ser(
 *				VARIABLE_PTR out_var, 
 *				double		*serial_day_1980, 
 *				FORMAT_PTR	input_format,
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
#define ROUTINE_NAME "cv_ipe2ser"

int cv_ipe2ser(VARIABLE_PTR out_var_never_used, double *serial_day_1980, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{

	/* Find the ipe_date variable */
	in_var = ff_find_variable("ipe_date", input_format);	/* Get the ipe date */

	if(!in_var) return (0);

	memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, FF_VAR_LENGTH(in_var));
	scratch_buffer[FF_VAR_LENGTH(in_var)] = STR_END;
	(void)ff_get_double(in_var, scratch_buffer, (double *)serial_day_1980, input_format->type);

	/* 1040874840 = number of minutes to end of 1979 */

	*serial_day_1980 -= 1040874840;

	/* There are 1440 minutes in a day, so convert serial_day_1980 into minutes */
	*serial_day_1980 /= 1440.0;

	return(1);
}

/*
 * NAME:  	cv_date_string
 *
 * PURPOSE:	This function converts to various string representations
 *			of dates. The position of the date and time elements in the
 *			string are controlled by the variable name:
 *				cc = century
 *				yy = year
 *				mm = month
 *				dd = day
 *				hh = hour
 *				mi = minute
 *				ss = second
 *			The presently supported strings are:
 *				day/month/year	(date_d/m/y)
 *				mmddyy			(date_ddmmyy)
 *
 * 
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	cv_date_string(
 *				VARIABLE_PTR out_var, 
 *				double        *serial_day_1980, 
 *				FORMAT        *input_format,
 *				char          *input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_date_string"

int cv_date_string(VARIABLE_PTR out_var, double *output, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{
	enum {DATE_NAMES_COUNT = 3};
	
	/* Must match an input variable name with one of the following
	*/
	char *date_names[DATE_NAMES_COUNT + 1] =
	{
		"date_m/d/y", /* date_m/d/y is replaced by date_mm/dd/yy */
		"date_mm/dd/yy",
		"date_yymmdd",

		/* DON'T EVEN THINK OF ADDING SOMETHING HERE WITHOUT INCREASING
		THE number_of_names. */

		NULL,
	};

	char *ch_ptr;
	char *target;

	int variable_length;
	unsigned int string_number;
	int i;

	double yy_mm_dd;
	
	char sec_str[4]= {STR_END};
	char min_str[4]= {STR_END};
	char hour_str[4]= {STR_END};
	char day_str[4]= {STR_END};
	char month_str[4]= {STR_END};
	char year_str[5]= {STR_END};
	char century_str[5]= {STR_END};

	VARIABLE tmp_var;
	
	assert(sizeof(*output) == sizeof(double));
	memset(output, ' ', sizeof(*output));

	in_var = NULL;
	
	/* Search the Input Format to Find the Date String */
	/* The first time this function is called it is an experimental call, to
	see if the conversion can be done. If this call succeeds, the desired
	conversion variable is added to the input format. The next time it is
	called, to do the actual conversions, this variable is there as a FFV_CONVERT
	variable. We are not interested in finding the convert variable here, we
	need the input variable. */
	
	string_number = DATE_NAMES_COUNT;
	for (i = DATE_NAMES_COUNT; i; i--)
	{
		in_var = ff_find_variable(date_names[i - 1], input_format);
		if (in_var && in_var->type != FFV_CONVERT)
		{
			string_number = i - 1;
			break;
		}
	}
	
	if (string_number == DATE_NAMES_COUNT)
	{
		err_push(ROUTINE_NAME, ERR_CONVERT_VAR, out_var->name);
		return(0);
	}

	/* The kludge, which makes the alias to the obsolete date_m/d/y
	   blech blech blech blech blech blech blech blech blech blech
	*/
	if (string_number)
		--string_number;
	if (!strcmp(out_var->name, "date_m/d/y"))
		strcpy(out_var->name, "date_mm/dd/yy");
	/* Don't touch the above kludge unless you are sure you know exactly what
	   I'm doing!
	*/

/*
	while(   (   date_names[string_number]
	          && (in_var = ff_find_variable(date_names[string_number], input_format)) == NULL
	         )
	      || in_var->type == FFV_CONVERT)
		++string_number;
*/
  
	if(string_number < DATE_NAMES_COUNT)
	{		/* Input variable is data string */
		/* Copy The String to buffer */
		variable_length = in_var->end_pos - in_var->start_pos + 1;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, variable_length);
		scratch_buffer[variable_length] = STR_END;

		/* Skip leading blanks */
		ch_ptr = scratch_buffer;
		while (*ch_ptr == ' ')
			++ch_ptr;
	
		switch(string_number)
		{
		case 0:		/* Date String is mm/dd/yy */
		 	sprintf(month_str, "%02s", strtok(ch_ptr, "/:, "));
		 	sprintf(day_str, "%02s", strtok(NULL, "/:, "));
		 	sprintf(year_str, "%02s", strtok(NULL, "/:, "));
/*		 	
		 	memStrcpy(month_str,strtok(ch_ptr, "/:, "),NO_TAG);
		 	memStrcpy(day_str  ,strtok(NULL, "/:, "),NO_TAG);
		 	memStrcpy(year_str ,strtok(NULL, "/:, "),NO_TAG);
*/
			break;
	
		case 1:		/* Date String is yymmdd */
			if (strlen(ch_ptr) == 5)
			{
				memMemmove(ch_ptr + 1, ch_ptr, 6,NO_TAG);
				*ch_ptr = '0';
			}
		 	memMemmove(year_str ,ch_ptr, 2,NO_TAG);
		 	memMemmove(month_str,ch_ptr + 2, 2,NO_TAG);
		 	memMemmove(day_str  ,ch_ptr + 4, 2,NO_TAG);
			*(year_str + 2) = STR_END;
			*(month_str + 2) = STR_END;
			*(day_str + 2) = STR_END;
			break;
		
		default:
			sprintf(  scratch_buffer, "%s, %s:%d"
			        , ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, scratch_buffer);
			err_disp();
			return(0);
		}
	}
	else
	{		/* Input is data, not a string */
		in_var = ff_find_variable("second", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, sec_str, input_format->type);

		in_var = ff_find_variable("minute", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, min_str, input_format->type);

		in_var = ff_find_variable("hour", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, hour_str, input_format->type);

		in_var = ff_find_variable("day", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, day_str, input_format->type);

		in_var = ff_find_variable("month", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, month_str, input_format->type);

		in_var = ff_find_variable("year", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, year_str, input_format->type);
		
		in_var = ff_find_variable("century", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, century_str, input_format->type);
		
		in_var = ff_find_variable("serial_day_1980", input_format);
		if(in_var){	/* Convert serial_day_1980 to year, month, day */

			/* In converting these real time values to integers I try to
			have correct truncation for times down to 100th of a second.
			In other words, if a time is within 100th of a second of the
			next major unit, it may be incorrectly rounded up.
			
			0.01 second = 0.001666 minute
						= 0.000003 hour
						= 0.00000012 day
			*/			
			memStrcpy(tmp_var.name, "second",NO_TAG);
			cv_ser2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(sec_str, "%02d", (short)(yy_mm_dd + 0.5));	/* Round Seconds */
			
			memStrcpy(tmp_var.name, "minute",NO_TAG);
			cv_ser2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(min_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "hour",NO_TAG);
			cv_ser2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(hour_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "day",NO_TAG);
			cv_ser2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(day_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "month",NO_TAG);
			cv_ser2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(month_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "year",NO_TAG);
			cv_ser2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(year_str, "%02d", (short)yy_mm_dd);

			memStrcpy(tmp_var.name, "century",NO_TAG);
			cv_ser2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(century_str, "%02d", (short)yy_mm_dd);			
		}
		
		/* Unsuccessful so far, try year_decimal for input */
		in_var = ff_find_variable("year_decimal", input_format);
		if(in_var){	/* Convert year_decimal to year, month, day */
			/* In converting these real time values to integers I try to
			have correct truncation for times down to 100th of a second.
			In other words, if a time is within 100th of a second of the
			next major unit, it may be incorrectly rounded up.
			
			0.01 second = 0.001666 minute
						= 0.000003 hour
						= 0.00000012 day
			*/			
			memStrcpy(tmp_var.name, "second",NO_TAG);
			cv_ydec2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(sec_str, "%02d", (short)(yy_mm_dd + 0.5));	/* Round Seconds */
			
			memStrcpy(tmp_var.name, "minute",NO_TAG);
			cv_ydec2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(min_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "hour",NO_TAG);
			cv_ydec2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(hour_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "day",NO_TAG);
			cv_ydec2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(day_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "month",NO_TAG);
			cv_ydec2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(month_str, "%02d", (short)(yy_mm_dd + DOUBLE_UP));
			
			memStrcpy(tmp_var.name, "year",NO_TAG);
			cv_ydec2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(year_str, "%02d", (short)yy_mm_dd);

			memStrcpy(tmp_var.name, "century",NO_TAG);
			cv_ydec2ymd(&tmp_var, &yy_mm_dd, input_format, input_buffer);
			sprintf(century_str, "%02d", (short)yy_mm_dd);			
		}
	}		
	if (!in_var)
		return(0);

	/* Now the input data is known, Determine the string positions in the
		output string */

/*	string_number = 0;
	while(memStrcmp(out_var->name, date_names[string_number],NO_TAG))++string_number;
*/
	last_underscore = memStrrchr(out_var->name,'_', "out_var->name,'_'");
	if (!last_underscore)return(0);	/* Invalid output variable name */
	++last_underscore;
	if (strlen(last_underscore) > sizeof(double))
		return(0); /* Output string too long */
	
	/* Search for tokens in the output variable name */
	ch_ptr = memStrstr(last_underscore, "ss", "ss");
	if(ch_ptr){
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, sec_str, 2, "second");
	}

	ch_ptr = memStrstr(last_underscore, "mi", "mi");
	if(ch_ptr){
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, min_str, 2, "minute"); 
	}

	ch_ptr = memStrstr(last_underscore, "hh", "hh");
	if(ch_ptr){
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, hour_str, 2, "hour"); 
	}

	ch_ptr = memStrstr(last_underscore, "dd", "dd");
	if(ch_ptr){
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, day_str, 2, "day"); 
	}

	ch_ptr = memStrstr(last_underscore, "mm", "mm");
	if(ch_ptr){
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, month_str, 2, "month"); 
	}

	ch_ptr = memStrstr(last_underscore, "yy", "yy");
	if(ch_ptr){
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, year_str, 2, "year"); 
	}

	ch_ptr = memStrstr(last_underscore, "cc", "cc");
	if(ch_ptr){
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, century_str, 2, "century"); 
		if(*target == ' ')*target = '0'; 
	}

	/* Search for /'s in the output variable name */
	ch_ptr = last_underscore;
	while( (ch_ptr = strchr(++ch_ptr, '/')) != NULL)
	{
		target = (char *)((char *)output + (ch_ptr - last_underscore));
		memMemcpy(target, "/", 1, "Slash"); 
	}
	
	/* convert leading zero's to spaces */
	
	for (target = (char *)output; *target == '0' /* zero */; target++)
		*target = ' ';
	
	/* Should right justify strings in field */

	return(1);
}

/*
 * NAME:  	cv_time_string
 *
 * PURPOSE:	This function converts bewteen various string representations
 *			of times. The presently supported strings are:
 *				hhmmss					(time_hhmmss)
 *				hh:mm:ss				(time_hh:mm:ss)
 *			These also work for strings in which seconds are not present.
 *
 * 
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *			modified by Terry Miller
 *
 * USAGE:	cv_time_string(
 *				VARIABLE_PTR out_var, 
 *				double        *serial_day_1980, 
 *				FORMAT        *input_format,
 *				char          *input_buffer)
 *
 * COMMENTS:
 *
 * RETURNS:	1 if successful, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_time_string"

int cv_time_string(VARIABLE_PTR out_var, double *output, FORMAT_PTR input_format, FF_DATA_BUFFER input_buffer)
{
	char *time_names[] = {
		  "time_h:m:s",
		  "time_hhmmss",

		/* DON'T EVEN THINK OF ADDING SOMETHING HERE WITHOUT INCREASING
		THE number_of_names. */

		NULL,
	};
	unsigned int number_of_names = 2;

	char *ch_ptr = NULL;

	int variable_length = 0;
	int string_length = 0;
	unsigned int string_number = 0;

	char hour[4];
	char minute[4];
	char second[5];

	*hour = STR_END;
	*minute = STR_END;
	*second = STR_END;

	assert(sizeof(*output) == sizeof(double));
	memset(output, ' ', sizeof(*output));

	in_var = NULL;

	/* Search the Input Format to Find the Time String */
	/* The first time this function is called it is an experimental call, to
	see if the conversion can be done. If this call succeeds, the desired
	conversion variable is added to the input format. The next time it is
	called, to do the actual conversions, this variable is there as a FFV_CONVERT
	variable. We are not interested in finding the convert variable here, we
	need the input variable. */
	while((((in_var = ff_find_variable(time_names[string_number], input_format)) == NULL) &&
		time_names[string_number]) ||
		in_var->type == FFV_CONVERT)
		++string_number;

	if(string_number < number_of_names){		/* Input variable is data string */

		/* Copy The String to buffer */
		variable_length = in_var->end_pos - in_var->start_pos + 1;
		memcpy(scratch_buffer, input_buffer + in_var->start_pos - 1, variable_length);
		scratch_buffer[variable_length] = STR_END;

		/* Skip leading blanks */
		ch_ptr = scratch_buffer;
		while(*ch_ptr == ' ')
			++ch_ptr;

		string_length = strlen(ch_ptr);
			
	
		switch(string_number)
		{
		case 0:		/* Time String is hh:mm:ss */
		 	sprintf(hour, "%02s", strtok(ch_ptr, "/:|, "));
		 	sprintf(minute, "%02s", strtok(NULL, "/:|, "));
			if (string_length < 6)
				*second = STR_END;
			else
		 		memStrcpy(second,strtok(NULL, "/:|, "),NO_TAG);
			break;
	
		case 1:		/* Time String is hhmmss */
			if(string_length == 5){
				memMemmove(ch_ptr + 1, ch_ptr, 6,NO_TAG);
				*ch_ptr = '0';
			}
		 	memMemmove(hour ,ch_ptr, 2,NO_TAG);
		 	memMemmove(minute,ch_ptr + 2, 2,NO_TAG);
		 	memMemmove(second  ,ch_ptr + 4, 2,NO_TAG);
			*(hour + 2) = STR_END;
			*(minute + 2) = STR_END;
			*(second + 2) = STR_END;
			break;

			default:
			sprintf(  scratch_buffer, "%s, %s:%d"
			        , ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, scratch_buffer);
				err_disp();
				return(0);
		}
	}
	else {		/* Input is data, not a string */
		in_var = ff_find_variable("hour", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, hour, input_format->type);

		in_var = ff_find_variable("minute", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, minute, input_format->type);

		in_var = ff_find_variable("second", input_format);
		if(in_var)ff_get_string(in_var, input_buffer + in_var->start_pos - 1, second, input_format->type);
	}		
	if(!in_var) return(0);

	/* Now the input data is known, Determine the output type */

	string_number = 0;
	while (strcmp(out_var->name, time_names[string_number]))
		++string_number;

	ch_ptr = (char *)output;

	switch(string_number)
	{
	case 0:		/* Time String is hh:mm:ss */
	 	sprintf(ch_ptr,"%s:%s:%s", hour, minute, second);
		break;

	case 1:		/* Date String is hhmmss */
		if(*(second + 1) == STR_END){
			*(second + 1) = *second;
			*(second + 2) = STR_END;
			*second = '0';
		}
		if(*(minute + 1) == STR_END){
			*(minute + 1) = *minute;
			*(minute + 2) = STR_END;
			*minute = '0';
		}

	 	sprintf(ch_ptr,"%s%s%s", hour, minute, second);
		break;

		default:
			sprintf(  scratch_buffer, "%s, %s:%d"
			        , ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, scratch_buffer);
			err_disp();
			return(0);
	}

	/* convert leading zero's to spaces */
	
	for (ch_ptr = (char *)output; *ch_ptr == '0' /* zero */; ch_ptr++)
		*ch_ptr = ' ';
	
	return(1);
}
