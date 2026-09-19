/*
 * NAME:        db_hdf_tag_to_rowsize
 *              
 * PURPOSE:     db_hdf_tag_to_rowsize is to read data descriptor lists from an
 *              HDF file attached to a data bin. A row size structure is filled
 *              when the designated tag and ref is found.
 *
 * USAGE:       db_hdf_tag_to_rowsize(DATA_BIN_PTR dbin, 
 *                                    unsigned short hdf_tag,
 *                                    unsigned short hdf_ref,
 *                                    unsigned short exact_hdf_ref,
 *                                    ROW_SIZES_PTR row_size_ptr)
 *
 * RETURNS:     0 if all goes well.
 *              -1 if HDF tag and ref not found
 *              #defined error if error occurs (> 0)
 *
 * DESCRIPTION: This function fills a row size structure with the offset and
 *              number of bytes for an hdf object specified by a tag and ref
 *              number. 
 *              
 *              Function Use:
 *                 1) A call with: hdf_tag = specified tag number
 *                                 *hdf_ref = 0
 *                                 exact_hdf_ref = 0
 *                    finds the first object mathching hdf_tag (*hdf_ref is
 *                    stored in a static array)
 *                    A subsequent call with: hdf_tag = specified tag number
 *                                            *hdf_ref != 0
 *                                            exact_hdf_ref = 0
 *                    finds the next object matching hdf_tag. The purpose of
 *                    this is to search an HDF file for only a certain object
 *                    type. One could then possibly fill a row size array with
 *                    all vdatas or all SDS offsets in a file. This also allows
 *                    the ability to pull out only vdatas while searching for 
 *                    a specific class name.
 *
 *                 2) A call with: hdf_tag = specified tag number
 *                                 *hdf_ref = any unsigned short
 *                                 exact_hdf_ref = specified ref number
 *                    looks only for the object with hdf_tag and exact_hdf_ref.
 *
 *                 hdf_ref is a ptr to allow the calling function to know the
 *                 exact HDF object found. Any call with *hdf_ref = 0, clears
 *                 the static hdf_ref array to zero.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR:      Mark VanGorp, NGDC, (303)497-6221, mvg@mail.ngdc.noaa.gov
 *
 * COMMENTS:    This does reads into the HDF file, so a file pointer in a
 *              calling function may need to be reset.
 *
 * KEYWORDS:    databins
 *
*/
#include <freeform.h>
#include <databin.h>
#include <dataview.h>
#ifdef SUNCC
#include <unistd.h>
#endif

/*Prototypes*/
int already_read(unsigned short);

/*****STATIC**********/
static unsigned short static_hdf_read_ref_array[128];

#define DD_BLOCK_SIZE (unsigned short)12
#define MAX_DD_BLOCKS (unsigned short)32
#undef ROUTINE_NAME
#define ROUTINE_NAME "db_hdftag_to_rowsize"

int db_hdf_tag_to_rowsize(DATA_BIN_PTR dbin, unsigned short hdf_tag, unsigned short *hdf_ref,
		 unsigned short exact_hdf_ref, ROW_SIZES_PTR row_size_ptr)
{

	char found = 0;
	char* dd_buffer = NULL;
	char* dd_buffer_ptr= NULL;
	char* offset_ptr = NULL;
	char* num_bytes_ptr = NULL;

	unsigned int num_bytes = 0;
	unsigned short num_data_descriptors;
	unsigned short i;
	unsigned long next_ddh_offset = 4;

	/************************************************************
	Data Descriptor Block: header followed by n data_descriptors:
	(where n = dd_header.num_data_descriptors)
	typedef struct {
		unsigned short num_data_descriptors;
		long next_dd_block_offset;
	} dd_header;

	typedef struct {
		unsigned short tag_number;
		unsigned short ref_number;
		long offset;
		long num_bytes;
	} data_descriptor;
	************************************************************/

	assert(row_size_ptr && dbin);
	
	if (!(dbin->state.open_file || dbin->state.std_input))
		return(err_push(ROUTINE_NAME, ERR_READ_HDF, "HDF file not open"));

	dd_buffer_ptr = dd_buffer = (char*)memMalloc((size_t)(DD_BLOCK_SIZE * MAX_DD_BLOCKS), "db_hdftag_to_rowsize: dd_buffer_ptr");
	if (!dd_buffer_ptr) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "data descriptor buffer");
		return(ERR_MEM_LACK);
	}

	/* Clear array if *hdf_ref = 0 */
	if (*hdf_ref == 0) {
		for (i=0; i<128; i++)
			static_hdf_read_ref_array[i] = 0;
	}
	while (!found && (next_ddh_offset != 0)) {
		lseek(dbin->data_file, next_ddh_offset, SEEK_SET);
		num_bytes = READ_DATA(dbin->data_file, (void*)&num_data_descriptors, 2,NO_TAG);
		if (num_bytes != 2) {
			err_push(ROUTINE_NAME, ERR_READ_HDF, "num_data_descriptors");
			return(ERR_READ_HDF);
		}
		num_bytes = READ_DATA(dbin->data_file, (void*)&next_ddh_offset, 4,NO_TAG);
		if (num_bytes != 4) {
			err_push(ROUTINE_NAME, ERR_READ_HDF, "next_ddh_offset");
			return(ERR_READ_HDF);
		}

#ifdef CCMSC
		byte_swap((char*)&num_data_descriptors, FFV_USHORT);
		byte_swap((char*)&next_ddh_offset, FFV_LONG);
#elif DEC
		byte_swap((char*)&num_data_descriptors, FFV_USHORT);
		byte_swap((char*)&next_ddh_offset, FFV_LONG);
#endif
		num_bytes = READ_DATA(dbin->data_file, (void*)dd_buffer, (unsigned int)(DD_BLOCK_SIZE * num_data_descriptors),NO_TAG);
		if (num_bytes != (unsigned int)(DD_BLOCK_SIZE * num_data_descriptors)) {
			err_push(ROUTINE_NAME, ERR_READ_HDF, "dd_buffer");
			return(ERR_READ_HDF);
		}
	
		for (i=0, dd_buffer_ptr = dd_buffer;
		     i<num_data_descriptors;
		     i++, dd_buffer_ptr+=DD_BLOCK_SIZE) {

#ifdef CCMSC
			byte_swap(dd_buffer_ptr, FFV_USHORT);
#elif DEC
			byte_swap(dd_buffer_ptr, FFV_USHORT);
#endif
			if (*(unsigned short *)dd_buffer_ptr != hdf_tag)
				continue;
			dd_buffer_ptr += 2;
#ifdef CCMSC
			byte_swap(dd_buffer_ptr, FFV_USHORT);
#elif DEC
			byte_swap(dd_buffer_ptr, FFV_USHORT);
#endif
			if (exact_hdf_ref && (*(unsigned short *)dd_buffer_ptr != exact_hdf_ref)) {
				dd_buffer_ptr -= 2;
				continue;
			}
			else if ( !exact_hdf_ref && already_read(*(unsigned short*)dd_buffer_ptr) ) {
				dd_buffer_ptr -= 2;
				continue;
			}
			*hdf_ref = *(unsigned short*)dd_buffer_ptr;
			offset_ptr = dd_buffer_ptr + 2;
			num_bytes_ptr = dd_buffer_ptr + 6;
			found = 1;

#ifdef CCMSC
			byte_swap(offset_ptr, FFV_LONG);
			byte_swap(num_bytes_ptr, FFV_LONG);
#elif DEC
			byte_swap(offset_ptr, FFV_LONG);
			byte_swap(num_bytes_ptr, FFV_LONG);
#endif
	
			break;
		} /* End for */
	} /* End while */

	memFree(dd_buffer, "tag2rs: dd_buffer");

	/* if found, fill the row_size structure */
	if (found) {
		row_size_ptr->start = *(long*)offset_ptr;
		row_size_ptr->num_bytes = *(long*)num_bytes_ptr;
		return(0);
	}
	return(-1);
}

int already_read( unsigned short hdf_ref )
{

	int i = 0;

	while (static_hdf_read_ref_array[i] != 0) {
		if (static_hdf_read_ref_array[i] == hdf_ref)
			break;
		i++;
	}

	if (static_hdf_read_ref_array[i] != 0)
		return 1;
	else {
		static_hdf_read_ref_array[i] = hdf_ref;
		return 0;
	}

}
