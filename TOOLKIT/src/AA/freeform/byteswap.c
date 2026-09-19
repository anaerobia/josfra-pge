#include <limits.h>
#include <freeform.h>

BOOLEAN endian(void)
/*****************************************************************************
 * NAME: endian()
 *
 * PURPOSE:  Determine native byte order
 *
 * USAGE: is_little_endian = (endian() == 0)
 *
 * RETURNS:  0 if native byte order is little endian, 1 big endian
 *
 * DESCRIPTION:  Tests the left byte of a short initialized to the value of 1.
 * Little endian machines (Least Significant Byte first) will set the left
 * byte to 1 and the right byte to zero, and big endian machines (MSB) will
 * set the left byte to zero and the right byte to 1.
 *
 * AUTHOR:  unknown
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

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "endian"
{
	short s = 1;

	if (*(unsigned char *)&s == 1)
		return(0);
	else
		return(1);
}

#ifdef BYTESWAP_MISCY_FUNCTIONS_NEVER_USED
/* MAO:c GETBYTE: Evaluates as kth byte of n, counting from the left,
                  k=0,1,2,3 */
#define GETBYTE(n, k) (((n) >> (CHAR_BIT * (k))) & 0x00FF)

/* MAO:c PUTBYTE: Evaluates as the rightmost byte of n, shifted into the kth
                  byte, counting from the left, k=0,1,2,3 */
#define PUTBYTE(n, k) (((n) & 0x000000FFl) << (CHAR_BIT * (k)))

int native_to_little(char *dataptr, FF_TYPES_t vartype)
#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "native_to_little"
{
/* MAO:c Seems identical to byte_swap()... */
	long long_var;
	short short_var;

	switch (vartype)
	{
		case FFV_LONG :
		case FFV_ULONG :
			long_var = *(unsigned long *)dataptr;
	
			*(dataptr+0)= (unsigned char)GETBYTE(long_var,0);
			*(dataptr+1)= (unsigned char)GETBYTE(long_var,1);
			*(dataptr+2)= (unsigned char)GETBYTE(long_var,2);
			*(dataptr+3)= (unsigned char)GETBYTE(long_var,3);
		break;

		case FFV_SHORT :
		case FFV_USHORT :
			short_var = *(unsigned short *)dataptr;
	
			*(dataptr+0)= GETBYTE(short_var,0);
			*(dataptr+1)= GETBYTE(short_var,1);
		break;

		default:
			assert(0);
			return(err_push(ROUTINE_NAME, ERR_UNKNOWN_VAR_TYPE, "Unknown representation in little endian for variable"));
		break;
	}
	
	return(0);
}


int little_to_native(char *dataptr,short vartype)
#undef ROUTINE_NAME
#define ROUTINE_NAME "little_to_native"
{
/* MAO:c This also seems to be another variant of byte_swap()... */
	long long_var;
	short short_var;

	switch(vartype)
	{
		case FFV_LONG :
		case FFV_ULONG :
			long_var = PUTBYTE(*(dataptr+0), 0) |
				PUTBYTE(*(dataptr+1), 1) |
				PUTBYTE(*(dataptr+2), 2) |
				PUTBYTE(*(dataptr+3), 3);
	
			memMemcpy(dataptr, (char *)&long_var, sizeof(long),NO_TAG);
	
		break;

		case FFV_SHORT :
		case FFV_USHORT :
			short_var = (short)(PUTBYTE(*(dataptr+0), 0) |
			     	PUTBYTE(*(dataptr+1), 1));
	
			memMemcpy(dataptr, (char *)&short_var, sizeof(short),NO_TAG);
				
		break;

		default:
			assert(0);
			return(err_push(ROUTINE_NAME, ERR_UNKNOWN_VAR_TYPE, "Unknown representation in little endian for variable"));
		break;
	
	}
	return(0);
}

int big_to_native(char *dataptr, FF_TYPES_t vartype)
#undef ROUTINE_NAME
#define ROUTINE_NAME "big_to_native"
{
	long long_var;
	short short_var;

	switch(vartype)
	{
		case FFV_LONG :
		case FFV_ULONG :
			long_var = PUTBYTE(*(dataptr+0), 3) |
				PUTBYTE(*(dataptr+1), 2) |
				PUTBYTE(*(dataptr+2), 1) |
				PUTBYTE(*(dataptr+3), 0);
	
			memMemcpy(dataptr, (char *)&long_var, sizeof(long),NO_TAG);
		break;

		case FFV_SHORT :
		case FFV_USHORT :
			short_var = (short)(PUTBYTE(*(dataptr+0), 1) |
			     	PUTBYTE(*(dataptr+1), 0));
	
			memMemcpy(dataptr, (char *)&short_var, sizeof(short),NO_TAG);
		break;

		default:
			assert(0);
			return(err_push(ROUTINE_NAME, ERR_UNKNOWN_VAR_TYPE, "Unknown representation in little endian for variable"));
		break;
	}

	return(0);
}

#endif
/* a fast way to swap byte for short and long integers */
#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "byte_swap"

/*****************************************************************************
 * NAME: byte_swap()
 *
 * PURPOSE:  To byte-swap various data types
 *
 * USAGE:  error = byte_swap(data_ptr, data_type);
 *
 * RETURNS:
 *
 * DESCRIPTION:  The FLIP_2_BYTES and FLIP_4_BYTES macros are used to swap
 * 2-byte data, i.e., short and unsigned short, and 4-byte data, i.e., long,
 * unsigned long, and float.  The FLIP_4_BYTES macro is used in a staggered
 * fashion on the two halves of doubles, to accomplish an 8-byte swap.
 *
 * The following FreeForm variable types are byte-swapped:
 *
 * FFV_SHORT, FFV_USHORT, FFV_LONG, FFV_ULONG, FFV_FLOAT, FFV_DOUBLE
 *
 * whereas the following FreeForm variable types are passed through:
 *
 * FFV_CHAR, FFV_UCHAR
 *
 * Any other variable types will result in an assertion failure or error return.
 *
 * The fundamental byte-swap algorithm is such that data in a 1-2, 1-2-3-4,
 * and a 1-2-3-4-5-6-7-8 order become a 2-1, 4-3-2-1, and a 8-7-6-5-4-3-2-1
 * order, respectively.
 *
 * CAUTION!  This code makes basic assumptions about the storage sizes of the
 * fundamental C types.  These assumptions are tested by assertions in
 * the DBIN_BYTE_ORDER event inside db_set().
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

void byte_swap(char *dataptr, FFV_TYPE_type var_type)
{
	size_t byte_size = ffv_type_size(var_type);

	char temp;
	char error_buffer[_MAX_PATH];
	static BOOLEAN fuse = FALSE; /* once in the default case is enough */

	switch ((int)byte_size)
	{

		
		case 0:
		case 1:
			/* do nothing */
		break;
		
		case 2:
			temp = *dataptr;
			*dataptr = *(dataptr + 1);
			*(dataptr + 1) = temp;
		break;
		
		case 4:
			temp = *dataptr;
			*dataptr = *(dataptr + 3);
			*(dataptr + 3) = temp;
			temp = *(dataptr + 1);
			*(dataptr + 1) = *(dataptr + 2);
			*(dataptr + 2) = temp;
		break;
		
		case 8:
			temp = *dataptr;
			*dataptr = *(dataptr + 7);
			*(dataptr + 7) = temp;
			temp = *(dataptr + 1);
			*(dataptr + 1) = *(dataptr + 6);
			*(dataptr + 6) = temp;
			temp = *(dataptr + 2);
			*(dataptr + 2) = *(dataptr + 5);
			*(dataptr + 5) = temp;
			temp = *(dataptr + 3);
			*(dataptr + 3) = *(dataptr + 4);
			*(dataptr + 4) = temp;
		break;
		
		default:
			if (!fuse)
			{
				fuse = TRUE;

				sprintf(error_buffer, "%d, %s:%d",
				        (int)byte_size, os_path_return_name(__FILE__), __LINE__);
				err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
				err_disp();
			}
		break;
	}
}
