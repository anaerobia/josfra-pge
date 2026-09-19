/* 
 *
 * CONTAINS: 	Lookup functions for FREEFORM lookup lists
 *				ff_lookup_string(FFF_LOOKUP_PTR, FF_TYPES_t);
 *				ff_lookup_number(FFF_LOOKUP_PTR, char *);
 *
 * The FREEFORM system uses lookup structures in a number of ways.
 * This file contains the functions used to extract information from those
 * structures. The structures look like this:
 * typedef struct {
 *	char		*string;
 *	unsigned int	number;
 * }FFF_LOOKUP, *FFF_LOOKUP_PTR;
 *  
 * and are stored in an array. The last member of the array has a NULL
 * pointer for the string. 
*/

#include <stdio.h>
#include <freeform.h>
#include <os_utils.h>
#include <limits.h>

/*
 * NAME:	ff_lookup_string
 *		
 * PURPOSE: Lookup string for a given number in a FREEFORM lookup list
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	char *ff_lookup_string(FFF_LOOKUP_PTR, FF_TYPES_t);
 *
 * RETURNS:	A pointer to the string, or NULL
 *
 * DESCRIPTION:	The FREEFORM lookup structure includes strings and numbers:
 * 				struct {
 *				char			*string;
 *				unsigned int	number;
 *				}FFF_LOOKUP, *FFF_LOOKUP_PTR;
 * 
 *				This function returns the string associated with the number
 *				given as the argument.
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * ERRORS:	
 *
 * SYSTEM DEPENDENT FUNCTIONS: 
 *
 * KEYWORDS:
 *
 */

char *ff_lookup_string(FFF_LOOKUP_PTR lookup, FF_TYPES_t int_key)
{
	while(lookup->string){
		if(int_key == lookup->number)return(lookup->string);
		++lookup;
	}
	return(NULL);
}

/*
 * NAME:	ff_lookup_number
 *		
 * PURPOSE: Lookup number for a given string in a FREEFORM lookup list
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	FF_TYPES_t ff_lookup_number(FFF_LOOKUP_PTR, char *);
 *
 * RETURNS:	An unsigned int  if the string is found or USHRT_MAX if not
 *
 * DESCRIPTION:	The FREEFORM lookup structure includes strings and numbers:
 * 				struct {
 *				char			*string;
 *				unsigned int	number;
 *				}FFF_LOOKUP, *FFF_LOOKUP_PTR;
 * 
 *				This function returns the number associated with the string
 *				given as the argument. The string comparisons are independent
 *				of case.
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * ERRORS:	
 *
 * SYSTEM DEPENDENT FUNCTIONS: 
 *
 * KEYWORDS:
 *
 */

FF_TYPES_t ff_lookup_number(FFF_LOOKUP_PTR lookup, char *str_key)
{
	while(lookup->string){
		if(os_strcmpi(str_key, lookup->string) == 0)
			return(lookup->number);
		++lookup;
	}
	return(USHRT_MAX);
}
