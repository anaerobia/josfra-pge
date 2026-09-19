/*
 * NAME:	ff_file_to_buffer
 *		
 * PURPOSE:	To read a file into a buffer
 *
 * USAGE:	unsigned int ff_file_to_buffer(char *file_name, char *buffer)
 *
 * RETURNS:	The number of bytes read into the buffer.
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * ERRORS:
 *		System File Error,file_name
 *		System File Error,file_name
 *		Problem reading file,"Input File To Buffer"
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS: SIDE EFFECT: If buffer is smaller then file size than
 *							memory beyond the buffer will be corrupted
 *						  this should be checked in the objects
 *	
 * RETURNS: returns 0 on error
 *
 * KEYWORDS:	
 *
 */
/*
 * HISTORY:
 *	r fozzard	4/21/95		-rf01 
 *		comment out struct stat (now part of stat.h included by unix.h)
*/

#include <limits.h>
#include <freeform.h>
#include <os_utils.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_file_to_buffer"

unsigned int ff_file_to_buffer(char *file_name, char *buffer)
{

	FILE *input_file;
	size_t num_read;
	size_t num_to_read;

	/* Error checking on NULL parameters */
	assert(file_name && buffer);

	if ((input_file = fopen(file_name, "rb")) == NULL)
	{
		err_push(ROUTINE_NAME, errno, file_name);
		return(0);
	}

	/* Turn off system buffering on input file */
	setvbuf(input_file, NULL, _IONBF, (size_t)0);
	
	/* Cannot read more than size_t bytes from file */
	num_to_read = (size_t)min(os_filelength(file_name), UINT_MAX);

	num_read = fread(buffer, sizeof(char), num_to_read, input_file);
	fclose(input_file);

	*(buffer + num_read) = STR_END;

	if (num_read != num_to_read)
	{
		err_push(ROUTINE_NAME,ERR_READ_FILE,"Input File To Buffer");
		return(0);
	}

	ff_make_newline(buffer);

	return(num_read);
}

/*
 * NAME:	ff_make_newline
 *		
 * PURPOSE:	To convert all occurrences of EOL strings to newline chars,
 * converting CR's to spaces (in DOS EOL case).
 *
 * USAGE:	int ff_make_newline(char *buffer)
 *
 * RETURNS:	Zero (0)
 *
 * DESCRIPTION:	If buffer contains Mac EOL's then these are replaced with Unix
 * EOL's.  If buffer contains PC EOL's, then these are replaced with the two
 * character string space plus Unix EOL.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	None
 *
 * ERRORS:
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS: 
 *	
 * KEYWORDS:	
 *
 */
int ff_make_newline(char *buffer)
{
	/* buffer_eol_str needs to be 3 bytes for (CR/LF/NULL) PC EOL */
	char buffer_eol_str[3];
	char *position = NULL;
	
	assert(buffer);
	
	ff_get_buffer_eol_str(buffer, buffer_eol_str);
	
	/* Exit if already newline */
	if (buffer_eol_str[0] == UNIX_EOL_1stCHAR)
		return(0);
		
	if (buffer_eol_str[0] == MAC_EOL_1stCHAR)
	{
		if (buffer_eol_str[1])
		{
			/* PC EOL */
			while ((position = strchr(buffer, MAC_EOL_1stCHAR)) != NULL)
				position[0] = ' ';
		}
		else
		{
			/* Mac EOL */
			while ((position = strchr(buffer, MAC_EOL_1stCHAR)) != NULL)
				position[0] = UNIX_EOL_1stCHAR;
		}
	}
	return(0);
}

int ff_file_to_bufsize(char *file_name, FF_BUFSIZE_PTR bufsize)
{
	assert(file_name);
	assert(bufsize);
	
	if (!bufsize || !bufsize->buffer)
		return(ERR_PTR_DEF);
	
	if (!os_file_exist(file_name))
		return(ERR_FILE_NOTEXIST);
	
	if (os_filelength(file_name) >= bufsize->total_bytes)
		return(ERR_DATA_GT_BUFFER);

	bufsize->bytes_used = ff_file_to_buffer(file_name, bufsize->buffer);
	
	if (bufsize->bytes_used)
		return(0);
	else
		return(ERR_READ_FILE);
}

