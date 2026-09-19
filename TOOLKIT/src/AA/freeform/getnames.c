/* 
 * NAME:		getnames.c
 * PURPOSE:	This program creates a dbin structure with only a 
 *                      name_table and header and then retrieves keywords
 *                      The format of the header file is:
 *                      parameter_name=parameter_value
 *                              .
 *                              .
 *                              .
 * 
 * USAGE:       getnames file_name
 *                      file_name: the header file containing PVL.
 *
 * DESCRIPTION:
 *
 * SYSTEM DEPENDENT FUNCTIONS: none
 *
 * GLOBALS:     none
 *
 * AUTHOR: Liping Di, NOAA-NGDC 303-497-6284. lpd@ngdc.noaa.gov
 * 
 * COMMENTS:
 * 
 * KEYWORDS: header, name table, data bin.
 *
 */
             
#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA
#include <databin.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "GetNames"
#define HEADER_FORMAT_INIT "ASCII_input_file_header_separate header_format\ncreate_format_from_data_file 0 0 char 0\n"

void main(int argc, char *argv[])
{
		char *file_name;
		char *buffer;
        DATA_BIN_PTR dbin=NULL;
        int data_error= 0;
        char *ch;
        long long_var;

		file_name = argv[1]; 
		buffer = (char *)malloc(20480);
		
        /* set a format for the header */
        if ((ch = (char *)malloc(1024)) == NULL) {
        	err_push(ROUTINE_NAME, ERR_MEM_LACK, "malloc failure");
        	err_disp();
        	exit(1);
        }
        strcpy(ch, HEADER_FORMAT_INIT);

        /* allocate and initialize dbin */
        if((dbin = db_make(file_name)) == NULL){
                err_push(ROUTINE_NAME, ERR_MAKE_DBIN, file_name);
                err_disp();
                exit(1);
        }

        /* use the header file name as the data file name, and create a name table.  */
        if(db_set(dbin,
			BUFFER, buffer,
            DBIN_FILE_NAME, file_name,
            MAKE_NAME_TABLE, (void *)NULL,
            END_ARGS)
		){
			err_push(ROUTINE_NAME, ERR_SET_DBIN, file_name);
			err_disp();
			exit(1);
		}

        /* set the header file name in the name table so that DEFINE_HEADER
           can get the header file name */
        nt_putvalue(dbin, "header_file_name", FFV_CHAR, file_name, 0);

        /* Create format and define the header so that the header can be retrieved by nt_askvalue.
        At present, the FORMAT_LIST event also reads in the header.
        Philosophically, this seems incorrect and it will be changed as part
        of our review of the libraries for this project. */
        if(db_set(dbin,
            FORMAT_LIST, (void *)NULL, ch,
            /* DEFINE HEADER,						event will be necessary here to read the header */
            END_ARGS)
		){
			err_push(ROUTINE_NAME, ERR_SET_DBIN, file_name);
			err_disp();
			exit(1);
		}

		/* Show the variables from the header */
		ff_show_variables(dbin->header, buffer, dbin->header_format);
		printf("%s", buffer);
		
		while((ch = gets(ch)) != NULL){
			nt_askvalue(dbin, ch, FFV_LONG, &long_var, buffer);
			printf("%d\n", long_var);
		}
}
