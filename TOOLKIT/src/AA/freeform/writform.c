/* FUNCTION WRITE_FORM

	This function writes a format specification to a file.

	RETURNS: An integer (0 if no errors)

	T. Habermann, NGDC, October, 1990
	R. Fozzard		12/6/94		change includes to work on Mac		-rf01
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* -rf01  #include <malloc.h>	*/
#include <ctype.h>
#include <fcntl.h>

#ifdef SUNCC
#include <malloc.h>		/* moved from above... -rf01 */
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef CCLSC    /* LightSpeed C Macintosh (old name) -rf01 */
#include <unix.h>
#endif          /* End of LightSpeed C -rf01 */

#ifdef CCMSC    /* Microsoft C PC -rf01 */
#include <malloc.h>		/* moved from above... -rf01 */
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#endif          /* End of Microsoft C -rf01 */

#include <freeform.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "write_format"

int write_format(FORMAT *form, char *file_name)
{
	FILE *output_file;
	VARIABLE *var;
	VARIABLE_LIST_PTR v_list;

	assert(form && ((void *)form == form->check_address) && file_name);

	output_file = fopen(file_name, "w");
	if (!output_file) {
		err_push(ROUTINE_NAME, ERR_OPEN_FILE, file_name);
		return(1);
	}

	/* Initialize pointer to list */
	v_list=FFV_FIRST_VARIABLE(form);

	while((var=FFV_VARIABLE(v_list))){
		fprintf(output_file,"%s %hd %hd %s %hd\n", var->name,
		var->start_pos, var->end_pos, ff_lookup_string(variable_types, var->type&FFV_TYPES),
		var->precision);
		v_list=dll_next(v_list);
	}
	fclose(output_file);
	return(0);
}
