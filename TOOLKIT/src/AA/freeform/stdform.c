/*
 * NAME:                stdform
 *              
 * PURPOSE:     To list standard form of formats for user to select
 *
 * USAGE:       char *stdform(char *buffer)
 *                      argument: buffer--temporary data buffer
 *
 * RETURNS:     if success, return a pointer to a file name of standard file format
 *                      otherwise, return NULL.
 *
 * DESCRIPTION: This functions reads a file called stdform.nam under the directory
 *                              defined by environmental variable GEOVUDIR. In this file, every
 *                              line has two iterms: the first item is format name, and the second
 *                              item is the format file name. The function displays the first
 *                              part in a dialog as a list box for user to select.
 *
 * ERRORS:
 *              File name error,"Can't get file name for STD format"
 *              Problem opening file, "Standard Format File"
 *              Problem with standard format, "Can't make slist"
 *              Problem reading file, "Standard Format File"
 *              ,"Can't open the data_type dialog"
 *              Problem reading file, "Standard Format File"
 *              Out of memory, "name"
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    standard format
 *
 */
/*
 * HISTORY:
 *	r fozzard	4/21/95		-rf01 
 *		(char *) for Think C
*/

#ifdef XVT

#include <xvt.h>
#include <freeform.h>
#include <os_utils.h>
#include <databin.h>
#include <dataview.h>
#include <geodata.h>

#else

#include <freeform.h>
#include <os_utils.h>
#include <databin.h>

#endif

#undef ROUTINE_NAME
#define ROUTINE_NAME "stdform"


char *stdform(char *buffer)
{
	char *ch = NULL;
	char *text = NULL;
	char *name = NULL;
	char *non_blank = NULL;
	char *file_name[128];

	int data_error;
	int select, cancel;
	
	unsigned int num_lines = 0;

#ifdef XVT
	DLG_RES_DATA list;
	WINDOW dlg_win = NULL_WIN;
#endif

	assert(buffer);

	/* get the standard file format*/
	if(!(os_path_prepend_special("stdform.nam",NULL, buffer))){
		err_push(ROUTINE_NAME, ERR_FIND_FILE, "GEOVUDIR undefined");
		return(NULL);
	} 

	/* Read the file into the buffer */
	ch=buffer+128;
	data_error = ff_file_to_buffer(buffer, ch);
	
	cancel=0;
	if(data_error == 0){
		err_push(ROUTINE_NAME, ERR_GENERAL, "Cannot collect standard formats");
		return(NULL);
	}

#ifdef XVT      
	list.select = 0;            /* default to first type */
	list.title = "FORMAT TYPE";   /* dialog title */
	/* the original message is too long & causes "dots..." */
	/* ORIGINAL: "Does the format of the data file belong to one of following:" */
	list.message = "Set the format of the data from the following: "; 
	list.cancel_enabled = TRUE;    /* enable the "cancel" button, user *
					       * must enter one of them(include not known */  
	/* make an empty SLIST */
	if ((list.x=xvt_slist_create()) == NULL)
	{
		err_push(ROUTINE_NAME,ERR_MEM_LACK, "Can't make string list");
		return(NULL);
	}
 
	/* make the name list of standard formats */
	data_error=0;
	while ((ch = strtok((data_error) ? NULL : ch, DOS_EOL_STRING)) != NULL)
	{
		/* Skip Comment Lines in the Format Files */
		data_error++;
		if (*ch == '/')
			continue;

		non_blank = memStrchr(ch, ' ', "stdform");
		*non_blank = '\0';
		while(*(++non_blank) == ' ')
			;
		
		file_name[num_lines] = non_blank;
	       
		xvt_slist_add_at_elt(list.x, NULL,  ch,  0L);
		num_lines++;
	}
			
	if (num_lines == 0)
	{
		err_push(ROUTINE_NAME,ERR_READ_FILE, "Missing EOL marker(s)");
		return(NULL);
	}
	
	/* put the select dialog */
	dlg_win = xvt_dlg_create_res(WD_MODAL, DLG_RES, EM_ALL, cb_res, PTR_LONG(&list));
	if (dlg_win == NULL_WIN)
	{
		err_push(ROUTINE_NAME,ERR_OPEN_DIALOG ,"Can't open the data_type dialog");
		return(NULL);
	}

	if(list.select == -1){                     /* cancel button, equal to unknown */
		list.select=num_lines-1;
		cancel=-1;
	}
	select=list.select+1;
	xvt_slist_destroy(list.x);                /* free the slist */

#else
	/* make the name list of standard formats */
	data_error=strlen(ch);
	text=ch+data_error+1;
	*text='\0';
	data_error=0;
	while ((ch = strtok((data_error) ? NULL : ch, DOS_EOL_STRING)) != NULL)
	{
		/* Skip Comment Lines in the Format Files */
		data_error++;
		if (*ch == '/')
			continue;

		non_blank = memStrchr(ch, ' ', "stdform");
		*non_blank = '\0';
		while(*(++non_blank) == ' ')
			;
		
		file_name[num_lines] = non_blank;
	       

		memStrcat(text, ch,NO_TAG);
		memStrcat(text, "\n",NO_TAG);

		num_lines ++;
	}
			
	if(num_lines == 0){
		err_push(ROUTINE_NAME,ERR_READ_FILE, "Missing EOL marker(s)");
		return(NULL);
	}
	
	/* put the selection */
	data_error=1;
	while(data_error){
		printf("\n\nStandard Format\n%s\n\nPlease select one using the number: ", text);
		scanf("%s",name);
		if(strlen(name)){
			select=atoi(ch);
			if(select > 0 && (unsigned int)select <= num_lines)break;
		}
		putchar('\a');
		printf("\n\nYou must select a number between 1 and %d\n\nReturn for continue",num_lines);
		scanf("%s", name);
	}

#endif

	/* (char *) for Think C -rf01 */
	name = (char *)memMalloc(strlen(file_name[select-1]) + 30, "stdform: name"); /* -rf01 */
	if(!name){
		err_push(ROUTINE_NAME,ERR_MEM_LACK, "name");
		return(NULL);
	}

	/* get the std_format file name */
	os_path_prepend_special(file_name[select-1], NULL, name);
	
	/* the file format is't belongs to one of listed on the list box, or unknown */
	if(memStrstr(name,"unknown",NO_TAG))memStrcpy(name, "unknown",NO_TAG);
	if(cancel == -1)memStrcpy(name, "cancel",NO_TAG);

	return(name);
}
