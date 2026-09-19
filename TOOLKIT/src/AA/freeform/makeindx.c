/* 
 * FILENAME: makeindx.c
 *
 * CONTAINS: db_make_index()
 *           (MODULE) write_index()
 *           (MODULE) flip_array()
 *           db_free_index()
 */	

/* Operating System Dependent Includes: */
#ifdef CCLSC
#include <unix.h>
#endif

#ifdef CCMSC
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freeform.h>
#include <databin.h>

#include <index.h>
#include <os_utils.h>

#ifdef PROTO
static int flip_array (void *array, size_t element_size, int number_of_elements);
/* not used static int write_index(INDEX *index, char *file_name, char *buffer); */
#else
static int flip_array ();
/* not used static int write_index(); */
#endif

/*
 * NAME:	db_make_index
 *
 * PURPOSE:	This is a routine for moving indexes between disc
 *			and memory:			 
 *			This function takes the name of an index file and creates an
 *			index structure in memory 
 *		
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	db_make_index( char *file_name, char *buffer, int convert, long format_ptr )
 *
 * COMMENTS:This function assumes that the first byte in the binary file
 *			is a character indicating the number of levels in the file.
 *			And that there is one or more LEVEL structures after it.
 *
 * RETURNS:	NULL if there is a failure, else a ptr to the new index structure
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "db_make_index"

INDEX *db_make_index(char *file_name, char *buffer, int convert, long format_ptr)
{
	char 		*ch_ptr		= NULL;		/* ptr to character array */
	char 		response[64];
	
	int 		index_file;				/* file with index format */
	int 		i;
	long nbytes;
	
	int error;

	size_t 		index_bytes = 0;		/* amounts to read and write */

	INDEX_PTR	index 	= NULL;		/* ptr to structure for Index Header */
	LEVEL_PTR 	current = NULL;		/* ptr to current location in level structure */
	FORMAT_PTR 	format	= NULL;



	*response = '\0';
	format = (FORMAT_PTR)format_ptr;

	/*	error checking */
	if (!buffer)
	{
		err_push("db_make_index",ERR_PTR_DEF,"Scratch buffer");
		return(NULL);
	}  


	if ((!file_name) )
	{
		err_push("db_make_index",ERR_PTR_DEF,"NULL filename");
		return(NULL);
	}
	index_file = open(file_name, O_RDONLY | O_BINARY);
	if(index_file == -1)
	{
		err_push("db_make_index",ERR_OPEN_FILE,"Index file");
		return(NULL);
	}

	/* check the file length. If the size is large than 65536, can't handle */
	nbytes=os_filelength(file_name);
	if(nbytes <= 0L || nbytes >= 65536L)
	{
		err_push("db_make_index", ERR_GENERAL, "index file is too big");
		return(NULL);
	}

	/* Allocate the index structure
	*/
	index = (INDEX *)memMalloc(sizeof(INDEX), "index");
	if (!index)
	{
		err_push("db_make_index",ERR_MEM_LACK,"Index");
		return(NULL);
	}
	
	/* Initialize index attributes
	*/

	/* For window version with dialog window  
		get_str_response("What Version Is The Index?", response, 16);
	*/
	index->version = INDEX_VERSION;
	index->offset_count = INDEX_IS_OFFSETS;

	if(!format)
	{
		FORMAT_LIST_PTR f_list = NULL;
		FF_BUFSIZE bufsize;
		PP_OBJECT pp_object;
		
		os_path_put_parts(response, NULL, file_name, "bfm");
		
		bufsize.buffer = buffer;
		bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;

		error = ff_file_to_bufsize(response, &bufsize);
		if (error)
		{
			err_push(ROUTINE_NAME, error, response);
			return(NULL);
		}
		
		pp_object.ppo_type = PPO_FORMAT_LIST;
		pp_object.ppo_object.hf_list = &f_list;

		if (ff_text_pre_parser("bfm", &bufsize, &pp_object))
		{
		}
		else
		{
			format = db_find_format(f_list, FFF_NAME, FORMAT_NAME_INIT, END_ARGS);
		}
	}
	index->record_length = format ? format->max_length : 0;
	
	/* Read the index file:
	This file has a header which contains:
		1 byte character indicating number of levels 
		a variable name for each level 
		min, max, and size for each level
		num_boxes for each level 
	This header is used to determine the number of boxes
	in the regionalization and the size of the index */
	
	/* deterimine number of levels from first byte of index file */
	
	error=memRead(index_file, (char *)&(index->num_levels), 1, "index_file,index->num_levels,1");
	if(error == -1)
	{
		(void) memStrcpy(buffer, "Make_index: Error Reading Index Levels from file", NO_TAG);
		return(NULL);
	}
	
	/* Allocate the levels structures */
	current = (LEVEL *)memMalloc((index->num_levels * sizeof(LEVEL)), "current");
	if(current == NULL)
	{
		memFree(index, "index");
		(void) memStrcpy(buffer, "Make_index: NOT ENOUGH MEMORY FOR INDEX LEVELS\n", NO_TAG);
		return(NULL);
	}
	index->levels = current;
	ch_ptr = buffer;

	/* read index levels from file and print header information to buffer */
/*	ch_ptr += sprintf(ch_ptr,"Region Parameters:\n"); */
	/* note: in sun ANSI C, sprintf return pointers instead of int */
	sprintf(ch_ptr,"Region Parameters:\n");
	ch_ptr += strlen(ch_ptr);	
	error = memRead(index_file, (char *)index->levels, (index->num_levels * sizeof(LEVEL)),NO_TAG );
	if (error == -1)
	{
		(void) memStrcpy(buffer, "Make_index: Error Reading Index\n", NO_TAG);
	 	return(NULL);
	}

	/* Convert the Level information, if necessary */
	if(convert)
	{
		for (i = index->num_levels, current = index->levels; i; ++current, --i)
			flip_array(&(current->min), sizeof(long), 4);
	}
	current = index->levels;	
	
	/* calculate total number of boxes in index file */
	index->total_boxes = current->num_boxes;
	sprintf(ch_ptr,"%s: %lu boxes\n", current->name, current->num_boxes );
	ch_ptr +=strlen(ch_ptr);

	for ( i=1; i < (int)(index->num_levels); i++ )
	{
		current = &(index->levels[i]);
		index->total_boxes = index->total_boxes * current->num_boxes;
		sprintf(ch_ptr,"%s: %lu  boxes\n", current->name, current->num_boxes );
		ch_ptr += strlen(ch_ptr);
	} 
	
	/* Calculate index bytes */
	index_bytes = (size_t)(index->total_boxes + 1 ) * sizeof(long);
	sprintf(ch_ptr,"There are %ld boxes (%u bytes)\n",
		index->total_boxes, index_bytes);
	ch_ptr += strlen(ch_ptr);

	/* Allocate the index of index and read it from the file */
	index->offsets = (long *)memMalloc(index_bytes, "index->offsets");
	if(index->offsets == NULL)
	{
		memFree(index->levels, "index->levels");
		memFree(index, "index");
		(void) memStrcpy(buffer, "Make_index: NOT ENOUGH MEMORY FOR INDEX OFFSETS\n", NO_TAG);
		return(NULL);
	}

	error = memRead(index_file, (char *)index->offsets, index_bytes,"index_file,(char *)index->offsets,index_bytes");
	if(error == -1)
	{
		(void) memStrcpy(buffer, "Make_index: Error Reading Index\n", NO_TAG);
		return(NULL);
	}
	sprintf(ch_ptr, "%u bytes read into index\n", error);
	ch_ptr += strlen(ch_ptr);

	index->last_offset = index->offsets + index->total_boxes;
	
	if(convert)
		flip_array((void *)(index->offsets), sizeof(long), (int)(index->total_boxes));

	if(!((FORMAT_PTR)format_ptr))
	{
		ff_free_format(format);
		format=NULL;
	}
	close(index_file);
	
	return(index);
}

#ifdef WRITE_INDEX_ACTUALLY_USED

/*
 * NAME:	write_index	
 *			
 * PURPOSE:	This is a routine for moving indexes between disc
 *			and memory:			 
 *			This function writes an index into a file
 *			
 * AUTHOR:	Ted Habermann, NGDC, (303)497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	write_index(
 *				INDEX *index, 
 *				char  *file_name, 
 *				char  *buffer)	
 *			
 * COMMENTS:num_boxes for each level is written to the index file
 *
 * RETURNS:	1 if there is an error, else 0
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

static int write_index(INDEX *index, char *file_name, char *buffer)
{

	int index_file;
	int error;
	int i;
	size_t index_bytes;

	index_file = open(file_name, O_WRONLY | O_BINARY | O_CREAT);
	if(index_file == -1)
	{
		err_push("db_make_index",ERR_OPEN_FILE,"Index file");
		return(1);
	}

	index_bytes = (size_t)(index->total_boxes) * sizeof(long);

	/* write index levels to file */
	error = write(index_file, (char *)&(index->num_levels), 1);
	if(error == -1)
	{
		(void) memStrcpy(buffer, "Write_index: Error Writing Index\n", NO_TAG);
		return(1);
	}
	for (i=0; i < (int)(index->num_levels); i++)
	{
		error = write(index_file, (char *)&(index->levels[i]), sizeof(LEVEL) );
		if(error == -1)
		{
			(void) memStrcpy(buffer, "Write_index: Error Writing Index\n", NO_TAG);
			return(1);
		}
	} /* END loop to write levels */

	/* write indexes to file */
	index_bytes = (size_t)((index->total_boxes + 1) * sizeof(long));
	
	error = write(index_file, (char *)index->offsets, index_bytes);
	if(error == -1)
	{
		(void) memStrcpy(buffer, "Write_index: Error Writing Index\n", NO_TAG);
		return(1);
	}
	close(index_file);
	return(0);
}

#endif

/*
 * NAME: flip_array
 *
 * PURPOSE: To swap the bytes in an array
 *
 * USAGE: flip_array(void *, int type, int number_of_elements)
 *
 * RETURNS: The number of numbers flipped
 *
 * DESCRIPTION: This function swaps byte orders for arrays of long or short ints
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * COMMENTS:
 *
 * AUTHOR: Ted Habermann
 *
 */

/* MACROS FOR FLIPPING BYTES */
#ifdef FLIP_4_BYTES
#undef FLIP_4_BYTES
#endif
#define FLIP_4_BYTES(a) (	(((a) & 0x000000FF) << 24) | \
							(((a) & 0x0000FF00) <<  8) | \
							(((a) & 0x00FF0000) >>  8) | \
							(((a) & 0xFF000000) >> 24) )

#ifdef FLIP_2_BYTES
#undef FLIP_2_BYTES
#endif
#define FLIP_2_BYTES(a) ( (((a) & 0xFF00) >> 8) | (((a) & 0x00FF) << 8) )

#ifdef PROTO
static int flip_array (void *array, size_t element_size, int number_of_elements)
#else
static int flip_array (array, element_size, number_of_elements)
	void *array;
	size_t element_size;
	int number_of_elements;
#endif
{
	unsigned	count = 0;
	short		*short_ptr;
	long 		*long_ptr;
	
	switch(element_size){
	
	case 2:
		for(short_ptr = (short *)array; number_of_elements;
			--number_of_elements, ++short_ptr, ++count)
				byte_swap((char *)short_ptr, FFV_SHORT);

			/*	*short_ptr = FLIP_2_BYTES(*short_ptr); */
		break;
		
	case 4:
		for(long_ptr = (long *)array; number_of_elements;
			--number_of_elements, ++long_ptr, ++count)
			byte_swap((char *)long_ptr, FFV_LONG);

			/* the macro is not work for some longs */
/*			*long_ptr = FLIP_4_BYTES(*long_ptr);
*/		break;
		
	default:	/* Error: Illegal element size */
		return(0);
	}
	
	/* No Errors */
	return(count);
}

/*
 * NAME:		db_free_index
 *		
 * PURPOSE:	To free index structure
 *
 * USAGE:	void db_free_index(INDEX_PTR) 
 *
 * RETURNS:	none
 *
 * DESCRIPTION: this function free the index structure	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	none
 *
 * GLOBALS:	none
 *
 * AUTHOR:	Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	index
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_free_index"

void db_free_index(INDEX_PTR index)

{
	if(index==NULL)return;

	if(index->offsets)memFree(index->offsets, "index->offsets");
	if(index->levels)memFree(index->levels, "index->levels");

	memFree(index, "index");
	return;
}

