#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define WANT_NCSA_TYPES
#include <freeform.h>

/*
 * NAME:	btype_to_btype
 *              
 * PURPOSE:	Convert data from one binary representation to another, handling
 * alignment
 *
 * USAGE:	int btype_to_btype(void *src_value, unsigned int src_type, void *dest_value, unsigned int dest_type)
 *
 * RETURNS:	On success, return zero, and dest_value is set. Otherwise, return
 * an error value defined in err.h
 *
 * DESCRIPTION: If types are the same, then src_value is memcpy()'d into
 * dest_value, and if both are FFV_CHAR then src_value is strcpy()'d; src_value
 * must be NULL-terminated if it is a string.  If neither type is FFV_CHAR
 * src_value is memcpy()'d into a local double (for alignment) and then
 * dereferenced according to src_type and converted into a double which is
 * ranged checked according to dest_type.  Next, the double is converted into
 * another local double which is cast and dereferenced according to dest_type
 * (again for alignment) and then is memcpy()'d into dest_value.
 * 
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "btype_to_btype"

#ifdef PROTO
int btype_to_btype(void *src_value, FF_TYPES_t src_type, void *dest_value, FF_TYPES_t dest_type)
#else
int btype_to_btype(src_value, src_type, dest_value, dest_type)
void *src_value;
unsigned int src_type;
void *dest_value;
unsigned int dest_type;
#endif

{
	char error_buffer[_MAX_PATH];
		
	big_var_type   big_var;
	align_var_type align_var;

	int error = 0;
	
	size_t  src_type_size;
	size_t dest_type_size;
		
	src_type  = FFV_DATA_TYPE_TYPE(src_type);
	dest_type = FFV_DATA_TYPE_TYPE(dest_type);
	
	src_type_size  = ffv_type_size(src_type);
	dest_type_size = ffv_type_size(dest_type);
	
	if (src_type_size == 0 || dest_type_size == 0)
		return(ERR_UNKNOWN_VAR_TYPE);
	
	/* if the data types are the same, just do memory copy */
	if (src_type == dest_type)
	{
		switch (FFV_DATA_TYPE_TYPE(src_type))
		{
			case FFV_TEXT:
				strcpy((char *)dest_value, (char *)src_value);
			break;
			
			default:
					memcpy(dest_value, src_value, src_type_size);
			break;
		}
		
		return(0);
	}
	else if (IS_TEXT_TYPE(src_type) || IS_TEXT_TYPE(dest_type))
		/* character type and numeric type are not compatible */
		return(ERR_CONVERT);

	/* treat src_value alignment and convert to double */
	
	memcpy((void *)&align_var, src_value, src_type_size);
	
	switch (FFV_DATA_TYPE_TYPE(src_type))
	{
		case FFV_INT8:
			big_var = *((int8 *)&align_var);
		break;
	
		case FFV_UINT8:
			big_var = *((uint8 *)&align_var);
		break;
	
		case FFV_INT16:
			big_var = *((int16 *)&align_var);
		break;
	
		case FFV_UINT16:
			big_var = *((uint16 *)&align_var);
		break;
	
		case FFV_INT32:
			big_var = *((int32 *)&align_var);
		break;
	
		case FFV_UINT32:
			big_var = *((uint32 *)&align_var);
		break;
	
		case FFV_INT64:
			big_var = *((int64 *)&align_var);
		break;
	
		case FFV_UINT64:
			big_var = *((uint64 *)&align_var);
		break;
	
		case FFV_FLOAT32:
			big_var = *((float32 *)&align_var);
		break;
	
		case FFV_FLOAT64:
			big_var = *((float64 *)&align_var);
		break;
	
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)src_type, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(ERR_SWITCH_DEFAULT);
	} 

	/* convert to dest_type and treat dest_value alignment */ 
			
	switch (FFV_DATA_TYPE_TYPE(dest_type))
	{
		case FFV_INT8:
			if (big_var < FFV_INT8_MIN || big_var > FFV_INT8_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(int8 *)&align_var = (int8)ROUND(big_var);
		break;
		
		case FFV_UINT8:
			if (big_var < FFV_UINT8_MIN || big_var > FFV_UINT8_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(uint8 *)&align_var = (uint8)ROUND(big_var);
		break;
		
		case FFV_INT16:
			if (big_var < FFV_INT16_MIN || big_var > FFV_INT16_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(int16 *)&align_var = (int16)ROUND(big_var);
		break;
		
		case FFV_UINT16:
			if (big_var < FFV_UINT16_MIN || big_var > FFV_UINT16_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(uint16 *)&align_var = (uint16)ROUND(big_var);
		break;
		
		case FFV_INT32:
			if (big_var < FFV_INT32_MIN || big_var > FFV_INT32_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(int32 *)&align_var = (int32)ROUND(big_var);
		break;
		
		case FFV_UINT32:
			if (big_var < FFV_UINT32_MIN || big_var > FFV_UINT32_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(uint32 *)&align_var = (uint32)ROUND(big_var);
		break;
		
		case FFV_INT64:
			if (big_var < FFV_INT64_MIN || big_var > FFV_INT64_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(int64 *)&align_var = (int64)ROUND(big_var);
		break;
		
		case FFV_UINT64:
			if (big_var < FFV_UINT64_MIN || big_var > FFV_UINT64_MAX)
				error = ERR_OVERFLOW_UNCHANGE;
			
			*(uint64 *)&align_var = (uint64)ROUND(big_var);
		break;
		
		case FFV_FLOAT32:         
			if (fabs(big_var) > FFV_FLOAT32_MAX)
				error = ERR_OVERFLOW_UNCHANGE;

			*(float32 *)&align_var = (float32)big_var;
		break;
	
		case FFV_FLOAT64:
			*(float64 *)&align_var = (float64)big_var;
		break;
	
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)dest_type, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(ERR_SWITCH_DEFAULT);
	} 
	memcpy(dest_value, (void *)&align_var, dest_type_size);
	
	return(error);
}

BOOLEAN type_cmp(FF_TYPES_t type, void *value0, void *value1)
/*****************************************************************************
 * NAME:  type_cmp()
 *
 * PURPOSE:  Compare two variables depending on FreeForm type
 *
 * USAGE:  is_equal = type_cmp(data_type, (void *)&var1, (void *)&var2);
 *
 * RETURNS:  1 if variables are equal, zero if unequal, or unknown data_type
 *
 * DESCRIPTION:  Strings are strcmp()'d, integers are memcmp()'d and floats
 * and doubles are differenced and compared to FLT/DBL_EPSILON, after attending
 * to alignment.
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

	char error_buffer[_MAX_PATH];
	big_var_type big_var0;
	big_var_type big_var1;
	size_t type_size = ffv_type_size(FFV_DATA_TYPE_TYPE(type));

	switch (FFV_DATA_TYPE_TYPE(type))
	{

		case FFV_TEXT:
			return((BOOLEAN)!strcmp((char *)value0, (char *)value1));
					
		case FFV_INT8:
		case FFV_UINT8:
		case FFV_INT16:
		case FFV_UINT16:
		case FFV_INT32:
		case FFV_UINT32:
		case FFV_INT64:
		case FFV_UINT64:
			return((BOOLEAN)!memcmp(value0, value1, type_size));
		
		case FFV_FLOAT32:
			memcpy((void *)&big_var0, value0, type_size);
			memcpy((void *)&big_var1, value1, type_size);
			if (fabs(*(float32 *)&big_var0 - *(float32 *)&big_var1) < FFV_FLOAT32_EPSILON)
				return(1);
			else
				return(0);
	
		case FFV_FLOAT64:
			memcpy((void *)&big_var0, value0, type_size);
			memcpy((void *)&big_var1, value1, type_size);
			if (fabs(*(float64 *)&big_var0 - *(float64 *)&big_var1) < FFV_FLOAT64_EPSILON)
				return(1);
			else
				return(0);
				
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)type, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(0);
	}
}
