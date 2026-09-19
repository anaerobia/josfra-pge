/* 
 * FILE:		stringdb.c
 *
 * CONTAINS:	The Functions for the string data base library:
 *
 *				DLL_NODE_PTR sdb_make_string_db(char *buffer);
 *				DLL_NODE_PTR sdb_find_section(char *, DLL_NODE_PTR);
 *				DLL_NODE_PTR sdb_free(char *buffer);
 *				unsigned int sdb_show_section_titles(DLL_NODE_PTR, char *);
 *				char 		*sdb_get_dll_data(DLL_NODE_PTR, char *);
 *				int 		 make_dll_title_list(char **name, DLL_NODE_PTR node);
 *				
 * 
 */

#ifdef CDGUIDE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <dl_lists.h>
int sdb_free(DLL_NODE_PTR, short);
#else
#include <freeform.h>
#include <databin.h>
#endif

#define MAX_LINE_LENGTH 256

/*
 * NAME:	sdb_make_string_db.c
 *		
 * PURPOSE:	Create a list for accessing a string database
 *
 * USAGE:	DLL_NODE_PTR sdb_make_string_db(char *buffer, int file)
 *
 * RETURNS:	Pointer to list or NULL
 *
 * DESCRIPTION:	Create a specialized doubly-linked list for storing a
 *				number of text blocks with associated titles called a
 *				string data base. The data structure used to do this is
 *				actually a special case of the DLL_NODE_PTR. This structure
 *				contains three pointers:
 *				typedef struct list_node
 *				{
 *					struct	list_node *previous;
 *					struct	list_node *next;
 *					void	*data_ptr;
 *				} DLL_NODE, *DLL_NODE_PTR;
 *				The string data base implementation allocates these structures
 *				with a character pointer appended:
 *				section = dll_add(dll_last(sections), sizeof(char *));
 *				This character pointer points to the title of the text block,
 *				thus statements that look like this:
 *				ppc = (char **)(section + 1);
 *				where section is a DLL_NODE_PTR are actually setting ppc
 *				to point at the title of the block of data.
 *  
 *				The data pointer in the DLL_NODE points to the block itself.
 *
 *				If buffer length is 0, and file is larger than 0,
 *				use file, and void pointer in the
 *				dll is actually an offset from beginning of file;
 *
 * SYSTEM DEPENDENT FUNCTIONS:	Processing of RETURN formats assumes EOL = 2 bytes
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS: stringdb
 *
 * 
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "sdb_make_string_db"

DLL_NODE_PTR sdb_make_string_db(char *buffer, char *source_file)
{
	unsigned int num_lines	  = 0;
	DLL_NODE_PTR section = NULL, sections = NULL;
	int nline;
	char in[MAX_LINE_LENGTH], *title, *ch=NULL, **ppc;
	FILE *stream;
	FILE_DLL_DATA *file_data;

/*	int i; unreferenced local variable */
/*	int nbyte; unreferenced local variable */
/*	fpos_t *pos; unreferenced local variable */

	/* Read data base from a source_file */
	if(buffer == NULL && source_file != NULL){
		stream=fopen(source_file, "r");
		if(stream == NULL){
			err_push(ROUTINE_NAME,ERR_OPEN_FILE, source_file);
			return(NULL);
		}

		while((fgets(in, MAX_LINE_LENGTH, stream) != NULL)){
		
			if(*in == '*'){			/* Define a section */

				/* Trim non-printing characters and blanks from end of title */
				ch = in + strlen(in);
				while(*(--ch) <= ' ') *ch = '\0';

				title = (char*)memMalloc((size_t)(strlen(in+1) + 1), "title");
				if(!title){
					if(sections)
						sdb_free(sections, 1);
					err_push(ROUTINE_NAME,ERR_MEM_LACK, in + 1);
					return(NULL);
				}
				(void*)memStrcpy(title,(char*)(in+1),NO_TAG);

				if(sections == NULL){
					sections = dll_init();
					if(sections == NULL){
						err_push(ROUTINE_NAME, ERR_MEM_LACK, "Initializing String Data Base");
						return(NULL);
					}
				}

				else {	/* Define end of last section */
					section = dll_last(sections);
					file_data = (FILE_DLL_DATA *)dll_data(section);
					file_data->nline = nline;
				}
	
				file_data = (FILE_DLL_DATA *)memMalloc(sizeof(FILE_DLL_DATA), "sdb_make_string_db: Data Pointer");
				if(file_data == NULL){
					if(sections)
						sdb_free(sections, 1);
					err_push(ROUTINE_NAME, ERR_MEM_LACK, "Making String Data Base");
					return(NULL);
				}
				fgetpos(stream, &(file_data->pos));
				file_data->fileid=-9999;
				file_data->stream=stream;
					
				section = dll_add(dll_last(sections), sizeof(char *));

				if(section == NULL){
					if(sections)
						sdb_free(sections, 1);
					err_push(ROUTINE_NAME, ERR_MEM_LACK, "Adding String Data Base Section");
					return(NULL);
				}

				dll_data(section) = file_data;
	
				/* title */
				ppc = (char **)(section + 1);
				*ppc = title;
				
				nline = 0;
			}
			else
				nline++;
		}	
		
		if(!sections)
			return(NULL);	
		section =dll_last(sections);
		file_data=(FILE_DLL_DATA *)dll_data(section);
		file_data->nline=nline;
	}
	
	else {
		assert(0); /* code below guarantees problems if dll ever gets free()'d */
		ch = buffer;
		while((ch = strtok((num_lines++) ? NULL : ch, "\n")) != NULL){
	 
			/* Skip Section Bodies */
			if(*ch != '*'){
				if(num_lines > 1)
					*(ch - 1) = '\012';
				continue;
			}
	 
			/* A section header has been found */
	
			/* Set the CR character at the end of the section title to '\0' */
			*(strchr(ch,'\015')) = '\0';
	
			if(sections == NULL){
				sections = dll_init();
				if(sections == NULL){
					err_push(ROUTINE_NAME, ERR_MEM_LACK, "Initializing String Data Base");
					return(NULL);
				}
			}
	
			/* Allocate a node + space for a character pointer */
			section = dll_add(dll_last(sections), sizeof(char *));
			if(section == NULL){
				if(sections)
					sdb_free(sections, 1);
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "Adding String Data Base Section");
				return(NULL);
			}
	
			/* Set the pointers for the new node:
				dll_add allocates space for the data and points the data
				pointer for the node at that space. In this case that space
				is used to store a pointer to the section title. */
	
			section = dll_last(sections);
	
			ppc = (char **)dll_data(section);
			*ppc = ch + 1;
	
			/* The data pointer of the node points at the beginning
			of the section */
			dll_data(section) = strchr(ch, '\0') + 2;
		}
	}
	return(sections);
	
}


/*
 * NAME:	sdb_find_section
 *		
 * PURPOSE:	Create a list for accessing a string database
 *
 * USAGE:	DLL_NODE_PTR sdb_find_section(char *buffer, DLL_NODE_PTR sections)
 *
 * RETURNS:	Pointer to section or NULL
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	Processing of RETURN formats assumes EOL = 2 bytes
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	title may consist of a path-filename that may have been
 * translated into a native path, i.e., using native directory separators
 * and not MENU_DIR_SEPARATOR's.  Thus a string compare is performed in which
 * leniency is given to directory separators.
 *
 * KEYWORDS: stringdb
 *
$Log:   K:/support/archive/stringdb.c_v  $
 * 
 *    Rev 1.11   12 Jan 1996 09:46:42   globepc
 * Moved dll_make_title_list out of 
 * dl_lists.c and into stringdb.c- this function
 * is really a string data base function, as
 * it is only used on string DBs and is 
 * expecting the SDB setup of phantom
 * strings.  It is not necessary with the 
 * new menu functions in GeoVu
 * 
 *    Rev 1.10   Oct 21 1995 21:04:54   mao
 * Various and sundry cosmetic changes, including adding
 * a bit more safe code.
 * 
 *    Rev 1.9   04/03/95 16:04:22   pvcs
 * changed whitespace, misc.changed sdb_find_section() -- changed a strcmp() to os_path_cmp_paths()
 * 
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "sdb_find_section"

DLL_NODE_PTR sdb_find_section(char *title, DLL_NODE_PTR sections)
{
	DLL_NODE_PTR section = NULL;
	char **ppc = NULL;
	
	if (!title) {
		err_push(ROUTINE_NAME,ERR_PTR_DEF,"String Data Base Section Title");
		return(0);
	}

	if (!sections) {
		err_push(ROUTINE_NAME,ERR_PTR_DEF,"String Data Base");
		return(0);
	}

	section = dll_first(sections);
	while(dll_data(section)){
		ppc = (char **)(section + 1);
		if(os_path_cmp_paths(title, *ppc)){
			section = dll_next(section);
			continue;
		}
		return(section);
	}
	return(NULL);
}

/*
 * NAME:	sdb_show_section_titles.c
 *		
 * PURPOSE:	Create a list of section titles
 *
 * USAGE:	char *sdb_show_section_titles(DLL_NODE_PTR sections, char *buffer)
 *
 * RETURNS:	Number of titles
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	Processing of RETURN formats assumes EOL = 2 bytes
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS: stringdb
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "sdb_show_section_titles"

unsigned int sdb_show_section_titles(DLL_NODE_PTR sections, char *buffer)
{
	unsigned int num_titles = 0;
	char *ch = NULL, **ppc = NULL;
	DLL_NODE_PTR section = NULL;
	
	if (!sections) {
		err_push(ROUTINE_NAME,ERR_PTR_DEF,"String Data Base");
		return(0);
	}

	section = dll_first(sections);
	ch = buffer;

	while(dll_data(section)){
		ppc = (char **)(section + 1);
		sprintf(ch, "%s\n", *ppc);
		ch += strlen(ch);
		++num_titles;
		section = dll_next(section);
	}
	*ch = '\0';

	return(num_titles);
}


/*
 * NAME:	sdb_get_dll_data
 *		
 * PURPOSE:	get data from a particular DLL section
 *
 * USAGE:	char *sdb_get_all_data(DLL_NODE_PTR section)
 *
 * RETURNS:	buffer containing the data
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 *
 * COMMENTS:	
 *	    
 * KEYWORDS: 
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "sdb_get_dll_data"

char *sdb_get_dll_data(DLL_NODE_PTR section, char *buffer)
{
	FILE_DLL_DATA *file_data;
	int i;
	char io[200];
	
	if (!section) {
		err_push(ROUTINE_NAME,ERR_PTR_DEF,"String Data Base Section");
		return(0);
	}

	if (!buffer) {
		err_push(ROUTINE_NAME,ERR_PTR_DEF,"Destination Buffer");
		return(0);
	}

	file_data=(FILE_DLL_DATA *)dll_data(section);
	if(file_data->fileid != -9999)
		return((char *)dll_data(section));
	else {
		*buffer='\0';
		fsetpos(file_data->stream, &(file_data->pos));
		for (i =0; i<file_data->nline; i++){
			fgets(io, 200, file_data->stream);
			memStrcat(buffer, io,NO_TAG);
		}
		return(buffer);
	}
}

/*
 * NAME:		sdb_free
 *		
 * PURPOSE:		To delete a string data base
 *
 * USAGE:    	int sdb_free(DLL_NODE_PTR node, short delete_title)
 *
 * RETURNS:		Number of nodes deleted
 *
 * DESCRIPTION:	sdb_free() deletes all nodes from a string data base.
 *				If the data delete_title flag is 1, the title and the
 *				data which is pointed to by the data_ptr in the nodes
 *				are freed as well.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	none
 *
 * GLOBAL:	none
 *
 * AUTHOR:	Ted Habermann, NGDC, 303-497-6284, haber@ngdc.noaa.gov
 * 			Liping Di, NGDC, 303-497-6284, lpd@ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	dll
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "sdb_free"

int sdb_free(DLL_NODE_PTR head, short delete_title)
{
	DLL_NODE_PTR current = NULL;
	char **ppc		 = NULL;
	int i				 = 0;

	if (head == NULL)
		return(0);

	current = dll_first(head);

	while (current != head)
	{
		/* Free the title pointer */
		if(delete_title)
		{
			ppc = (char **)(current + 1);
			memFree(*ppc, "sdb_free: Title Pointer");
			if (dll_data(current))
				memFree(dll_data(current), "sdb_free: Data Pointer");
		}

		current = dll_next(current);
		
		dll_data(dll_previous(current)) = NULL;
		dll_previous(dll_previous(current)) = NULL;
		dll_next(dll_previous(current)) = NULL;
		
		memFree(dll_previous(current), "sdb_free: Node Pointer");

		i++;
	}
	memFree(head, "sdb_free: Head Node Pointer");
	head = NULL;
	return(i);
}

/*
 * NAME:	make_dll_title_list
 *		
 * PURPOSE:	To make a list of section titles from a dll of strings...
 *
 * USAGE:	int make_dll_title_list(char **name, DLL_NODE_PTR node)
 *
 * RETURNS:	number of dll titles found
 *
 * DESCRIPTION:	The **name must be allocated large enough before this call
 *
 * SYSTEM DEPENDENT FUNCTIONS:	none
 *
 * GLOBAL:	none
 *
 * AUTHOR:	Liping Di, NGDC 497-6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	Double-Linked Lists, 	
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "make_dll_title_list"

int make_dll_title_list(char **name, DLL_NODE_PTR node)
{
	int i=0;
	DLL_NODE_PTR section;
	char *ch_ptr, **ch;

	if (node == NULL)
		return(0);

	section = dll_first(node);
	ch = name;

	while (dll_data(section))
	{
		ch_ptr = *(char **)(section + 1);
		*ch = ch_ptr;
		ch++;
		i++;
		section = dll_next(section);
	}
	return(i);
}

