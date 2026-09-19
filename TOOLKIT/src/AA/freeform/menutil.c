/*
 *
 * CONTAINS:    checkmen.c      A program for checking menu files.
 *              menutil.c   A program for dealing with menu files.
 */

/*
 * NAME:        menutil.c
 *              
 * PURPOSE:     Check a menu file for errors, strip spaces off of section names, create a menu
 *          index, fill in referenced files, trim out _fmt type sections.
 *
 * USAGE:       checkmen input_file [-trim][-strip][-fill][-index][-d CD_DRIVE_LETTER]
 *                              [-n NEW_FILE_NAME][-nodata][-check]
 *
 * RETURNS:     outputs a new menu file with the requested changes, if any, made.
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  Only in the checking part of the code.
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *          Kevin Frender kbf@kryton.ngdc.noaa.gov (disclaims all of function main)
 *
 * COMMENTS: 
 *          
 * KEYWORDS: stringds
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

#include <dl_lists.h>
#include <name_tab.h>
#include <databin.h>
#include <dataview.h>
#include <menuindx.h>

#ifdef XVT
#include <xvt.h>
#include <geodata.h>
#endif

#define BUFFER_SIZE 32768

#define UNKNOWN_SECTION         0
#define HELP_SECTION            1
#define MENU_SECTION            2
#define EQV_SECTION             3
#define FORMAT_SECTION          4
#define FORMAT_LIST_SECTION     5
#define HEADER_SECTION          6
#define TERM_SECTION            7

#define ulong unsigned long

FFF_LOOKUP section_types[] = {
	{"help", HELP_SECTION},
	{"eqv", EQV_SECTION},
	{"afm", FORMAT_SECTION},
	{"bfm", FORMAT_SECTION},
	{"fmt", FORMAT_LIST_SECTION},
	{"hdr", HEADER_SECTION},
	{(char *)NULL, 0},              
};

#define MAX_BUFFER_SIZE 32000
#define POP 1
#define PUSH 0

typedef struct dll_menu_reference {
	struct dll_menu_reference *next;
	char *file_ref;
	char file_type;
	char *menu_sec;
} menu_ref, *menu_ref_ptr;

typedef struct tree_stack_element {
	struct tree_stack_element *prev;
	char depth;
	ulong offset;
	ulong size;
} tstacke, *tstacke_ptr;

void strip_blanks(char *filename, char *outfnm);
void fillmenu(char *filename, char *newfilename, char *cddrive);
void change_backslash(char *string);
void trim_menu_sec(char *filename, char *outfilename);
void make_menu_tree(char *filename, MENU_INDEX_PTR mindex);
int tree_stack(int op, char *depth, ulong *offset, ulong *size);
char fix_help_titles(char *filename, char *outfilename, MENU_INDEX_PTR mindex);
int make_files_lowercase(MENU_INDEX_PTR mindex, char *outfilename);
static BOOLEAN lookup_keywords(NAME_TABLE_PTR nt);
int compare(const char **, const char **);
int comparei(const char **, const char **);

/* Global variables */
char menu_index_existed = 0;
MENU_INDEX_PTR mindex = NULL;

/*
 * NAME:        menutil main
 *              
 * PURPOSE:     To accept command line arguments and call the appropriate functions;
 *          checking the menu file afterward if so requested.
 *          the old checkmen.c is included in this function.
 *
 * USAGE:       menutil input_file [-trim][-strip][-fill][-index][-d CD_DRIVE_LETTER]
 *                  [-n NEW_FILE_NAME][-nodata][-check][-trimindex][-tree][-fixhelp]
 *
 * RETURNS:     
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *              Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS: 
 *          
 * KEYWORDS: stringds
 *
 */

#define ROUTINE_NAME "Menutil main"
void main(int argc, char *argv[])
{
	FILE *testfile;
	char *text_block = NULL;
	char *ch_ptr = NULL;
	char *point = NULL;
	char *line = NULL;
	char *title = NULL;
	char *file_prefix = NULL;
	char *position;
	char *sections;
	char *uptosection;
	char help_title_1[256];
	char help_title_2[256];
	char help_secs_found = 0;
	char strip_menu = 0;
	char fill_menu = 0;
	char check_menu = 1;
	char check_data = 1;
	char check_help = 1;
	char trim_menu = 0;
	char trim_index = 0;
	char index_menu = 0;
	char menu_tree = 0;
	char lower_menu = 0;
	char fixhelp = 0;
	char fill_menu_name[50];
	char menu_file_name[50];
	 ROW_SIZES rowsize;
	 
	unsigned char found;
		
	int section_type = 0;
	int num_lines;
	int error;
	int i;
	
	unsigned int total_errors = 0;
	unsigned int dbase_length = 0;
	unsigned int num_titles   = 0;
	
	unsigned long fileoffset = 0;

	FFF_LOOKUP special_list[] = {
		{"INTRODUCTION", 0},
		{"SLIDE SHOW", 0},
		{"VOLUME_ID", 0},
		{"ANIMATION", 0},
		{"CD_MENU_NAME", 0},
		{(char *)NULL, 0},              
	};
	FFF_LOOKUP_PTR lookup = NULL;
	
	char *greeting[] = {
#ifdef ALPHA_TEST
{"\nmenutil alpha-test 2.0.2 "__DATE__" -- a GeoVu menu file diagnostic utility\n"},
#elif defined(BETA_TEST)
{"\nmenutil beta-test 2.0.2 "__DATE__" -- a GeoVu menu file diagnostic utility\n"},
#else
{"\nWelcome to menutil release 2.0.2 -- a GeoVu  menu file diagnostic utility\n"},
#endif
{(char *)NULL},
},
       *usage[] = {
{"USAGE: menutil FILE [-d CD_DRIVE] [-n NEW_FILE] [-check] [-index] [-fill]"},
{"                [-trim] [-nodata] [-tree] [-fixhelp] [-trimindex] [-lower]"},
{"FILE:    The menu file to check or make changes to."},
{"-d CD_DRIVE: CD-drive or data's drive"},
{"-n NEW_FILE: Specifies the file to place the modified menu in."},
{"         If ommited, NEW_FILE_NAME is FILE by default."},
{"-check:  Checks the menu file for errors or omissions.  It also checks to"},
{"         see that the files mentioned in the menu file exist."},
{"-nodata: Suppresses check for data files with the -check option."},
{"-nohelp: Suppresses check for help with the -check option."},
{"-fill:   Finds all references to help files in the menu file and inserts those"},
{"         files."},
{"-trim:   removes all sections from the menu file ending in:"},
{"         _eqv, _afm, _bfm, _fmt, and _hdr."},
{"-index:  creates the MENU INDEX section at the front of the menu file."},
{"-tree:   creates a schematic tree of the menu file."},
{"-fixhelp: Changes incorrectly assigned help section titles."},
{"-trimindex: Removes the MENU INDEX section, if one exists."},
{"-lower:  Changes all filenames to lowercase"},
{(char *)NULL},
};
	
	i = 0;
	while (greeting[i])
		fprintf(stderr, "%s\n", greeting[i++]);

	if (argc == 1)
	{
		i = 0;
		while (usage[i])
			fprintf(stderr, "%s\n", usage[i++]);
		exit(1);                                                                    
	}
	
	strcpy(menu_file_name, argv[1]);
	 strcpy(fill_menu_name, argv[1]);
	 if((file_prefix = (char *)malloc(100)) == NULL)
	 {
		 err_push(ROUTINE_NAME, ERR_MEM_LACK, "Creating 100 byte buffer");
		 err_disp();
		 exit(255);
	 }
	 strcpy(file_prefix, "\0");
				
	if(argc >= 3){
		check_menu = 0;
		for(i = 2; i < argc; i++){
			if(strcmp(argv[i], "-d") == 0)
				strcpy(file_prefix, *(argv + i + 1));
			if(strcmp(argv[i], "-check") == 0) check_menu = 1;
			if(strcmp(argv[i], "-nodata") == 0) check_data = 0;
			if(strcmp(argv[i], "-nohelp") == 0) check_help = 0;
			if(strcmp(argv[i], "-n") == 0) strcpy(fill_menu_name, argv[i + 1]);
			if(strcmp(argv[i], "-fill") == 0){
				 fill_menu = 1;
			}
			if(strcmp(argv[i], "-trim") == 0){
				trim_menu = 1;
			}
			if(strcmp(argv[i], "-trimindex") == 0){
				trim_index = 1;
			}
			if(strcmp(argv[i], "-index") == 0){
				index_menu = 1;
			}
			if(strcmp(argv[i], "-fixhelp") == 0){
				fixhelp = 1;
			}        
			if(strcmp(argv[i], "-tree") == 0){
				menu_tree = 1;
			}
			if(strcmp(argv[i], "-lower") == 0){
				lower_menu = 1;
			}
		} /* end for() */
	}
		 
	 if((testfile = fopen(menu_file_name, "r")) == NULL)
	 { /* menu file not found */
		 printf("Menu file '%s' not found.", menu_file_name);
		 exit(1);
	 }
	 fclose(testfile);
		 
	 /* we are going to change the menu file, so make a backup */
	 if(!strcmp(menu_file_name, fill_menu_name)){ 
		/* We weren't specifically told the output file name, so change it */
		position = strrchr(menu_file_name, '.');
		if(position) position[0] = '\0';
		strcat(menu_file_name, ".old");
		remove(menu_file_name);
		if(rename(fill_menu_name, menu_file_name)){
			printf("Error renaming '%s' to '%s'.", fill_menu_name, menu_file_name);
			exit(1);
		}
		printf("The old version of the menu is stored in '%s'.\n\n", menu_file_name);
	}
		 
	/* strip all trailing whitespace off line ends */
	strip_blanks(menu_file_name, fill_menu_name);
	strcpy(menu_file_name, fill_menu_name);
				 
	if(fill_menu){ /* Fill in help sections */
		remove("menutil.tmp");
		if(rename(menu_file_name, "menutil.tmp")){
			printf("Error renaming '%s' to 'menutil.tmp'.", menu_file_name);
			exit(1);
		}
		fillmenu("menutil.tmp", fill_menu_name, file_prefix);
		strcpy(menu_file_name, fill_menu_name);
	}

	if(trim_menu){ /* Trim _eqv, _hdr, etc. sections */
		remove("menutil.tmp");
		if(rename(menu_file_name, "menutil.tmp")){
			printf("Error renaming '%s' to 'menutil.tmp'.", menu_file_name);
			exit(1);
		}
		trim_menu_sec("menutil.tmp", fill_menu_name);
		strcpy(menu_file_name, fill_menu_name);
	}


	if(trim_index){
		remove("menutil.tmp");
		if(rename(menu_file_name, "menutil.tmp")){
			printf("Error renaming '%s' to 'menutil.tmp'.", menu_file_name);
			exit(1);
		}
		mn_index_remove("menutil.tmp", menu_file_name);
	}


ReIndexMenu:
	if(mindex)
		mn_index_free(mindex);
		
	/* the -fixhelp, -tree and -caps options require a menu index */
	if(index_menu){
		printf("Indexing menu...");
		mn_index_remove(menu_file_name, "menutil.tmp");
		if((mindex = mn_index_make("menutil.tmp", MAX_BUFFER_SIZE, menu_file_name)) == NULL)
		{
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu file index");
			err_disp();
			exit(255);
		}
		printf("Done.\n");
	}
	else{
		/* Create a menu index in memory */
		if((mindex = mn_index_make(menu_file_name, MAX_BUFFER_SIZE, NULL)) == NULL)
		{
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu index");
			err_disp();
			exit(255);
		}
		if(mindex->index_corrupt){ /* file index existed, but was corrupt*/
			printf("Automatically reindexing menu to remove corrupt index.\n");
			mn_index_free(mindex);
			mn_index_remove(menu_file_name, "menutil.tmp");
				 
			/* Recreate it correctly */
			if((mindex = mn_index_make("menutil.tmp", MAX_BUFFER_SIZE, menu_file_name)) == NULL)
			{
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu file index");
				err_disp();
				exit(0);
			}
			printf("Done reindexing menu.\n");
			
			mn_index_free(mindex);
			if((mindex = mn_index_make(menu_file_name, MAX_BUFFER_SIZE, NULL)) == NULL)
			{
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu index");
				err_disp();
				exit(255);
			}			
		}
	}
	
	if(lower_menu){ /* Make file names lowercase */
		printf("Converting file names to lowercase...");
		make_files_lowercase(mindex, "menutil.tmp");
		
		if(rename("menutil.tmp", menu_file_name)){
			printf("Error renaming 'menutil.tmp' to '%s'.", menu_file_name);
			exit(1);
		}
		
		if(mindex->file_index_exists){
			mn_index_free(mindex);
			mn_index_remove(menu_file_name, "menutil.tmp");
			
			/* Recreate it correctly */
			if((mindex = mn_index_make("menutil.tmp", MAX_BUFFER_SIZE, menu_file_name)) == NULL)
			{
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu file index");
				err_disp();
				exit(0);
			}
		}
		else{ /* no file index exists */
			mn_index_free(mindex);
			if((mindex = mn_index_make(menu_file_name, MAX_BUFFER_SIZE, NULL)) == NULL)
			{
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu index");
				err_disp();
				exit(0);
			}
		}
		printf("Done.\n");
		lower_menu = 0;		
	}
	
	if(fixhelp){ /* Fix misnamed help section titles */
		remove("menutil.tmp");
		if(rename(menu_file_name, "menutil.tmp")){
			printf("Error renaming '%s' to 'menutil.tmp'.", menu_file_name);
			exit(1);
		}

		fixhelp = fix_help_titles("menutil.tmp", fill_menu_name, mindex);
		strcpy(menu_file_name, fill_menu_name);
						 
		if(fixhelp){ /* we changed something, so re-index... */
			if(mindex->file_index_exists){
				mn_index_free(mindex);
				mn_index_remove(menu_file_name, "menutil.tmp");
			 
				/* Recreate it correctly */
				if((mindex = mn_index_make("menutil.tmp", MAX_BUFFER_SIZE, menu_file_name)) == NULL)
				{
					err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu file index");
					err_disp();
					exit(0);
				}
			}
			else{ /* no file index exists */
				mn_index_free(mindex);
				if((mindex = mn_index_make(menu_file_name, MAX_BUFFER_SIZE, NULL)) == NULL)
				{
					err_push(ROUTINE_NAME, ERR_UNKNOWN, "Creating menu index");
					err_disp();
					exit(0);
				}
			}
		}
		else{ 
			/* we didn't do anything; the file didn't get copied */
			if(rename("menutil.tmp", menu_file_name)){
				printf("Error renaming 'menutil.tmp' to '%s'.\n", menu_file_name);
				exit(1);
			}
		}
		fixhelp = 0;
	}      
		 

	if(menu_tree){
		make_menu_tree(menu_file_name, mindex);
		menu_tree = 0;
	}
		 
	if(!check_menu) exit(0); /* if we want to check, continue on down the code... */
	check_menu = 0;
	
	 
	 /* Now check the menu.
	  * The following things are checked:
	  *
	  * -Data files exist (if -nodata is NOT used)
	  * -referenced sections exist
	  * -Help sections exist
	  * -Help sections are not based on user-interface element
	  * 
	  */
	
	printf("Checking Menu File: %s\n", menu_file_name);
	
	if((title = (char *)malloc((size_t)500)) == NULL)
	{
		printf("Out of memory for title buffer");
		exit(255);
	}
	
	/* a menu index should already exist */

	/* place the section titles in a buffer */
	mn_sec_titles_to_buf(mindex, NULL, &i, &sections);
	 
	printf("The Menu File Contains %u Sections\n", i);
	
	/* Loop the data base to check */
	uptosection = sections;
	strip_menu = 1;
	while(1){
		if(!strip_menu){
			if((uptosection = strstr(uptosection, "\n")) == NULL)
			{
				break;
			}
		}
		else{
			strip_menu = 0;
		}
		
		if(uptosection[1] == '\0') break; /* all done */
		
		 while(uptosection[0] < ' ')
			 uptosection++;       

		/* Determine the type of this section */
		strncpy(title, uptosection, (size_t)490);
		title[490] = '\0';
		position = strstr(title, "\n");
		if(!position){
			printf("FATAL ERROR:\nTitle found which exceeds 490 bytes (the maximum limit):\n%s\n", title);
			exit(255);
		}
		position[0] = '\0';
		
		ch_ptr = strrchr(title,'_');
		found = 0;
		
		if(ch_ptr){
			lookup = section_types;
			while(lookup->string){
				if(strcmp(lookup->string, ch_ptr + 1) == 0){
					section_type = lookup->number;
					++found;
					break;
				}
				++lookup;                       
			}
		}
		if(!found)section_type = MENU_SECTION;
		
		if(mn_index_get_offset(mindex, title, &rowsize)){
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "Finding offset of section");
			err_disp();
			exit(0);
		}
		
		if(text_block)
			free(text_block);
			
		text_block = NULL;
		/* Get text block for this section, but not if it is a help section   */
		/* (45,000 byte help sections cause program slowing & memory errors)  */
		if(section_type != HELP_SECTION){
				
			rowsize.num_bytes *= -1; /* Specify we want title too */
			i = mn_section_get(mindex, NULL, &rowsize, &text_block);
			if(i){ /*error occured*/
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Retrieving section");
				err_disp();
				exit(255);
			}
			if(!text_block){
			  printf("No text for %s\n", title);
			  continue;
			}
		}
		else{
			mindex->max_buffer_size = 1000;
			rowsize.num_bytes *= -1; /* Specify we want title too */
			i = mn_section_get(mindex, NULL, &rowsize, &text_block);
			if((i == 0) || (i == ERR_MN_BUFFER_TRUNCATED))
			{
			}
			else{
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Retreiving help section");
				err_disp();
				exit(0);
			}
			
			position = strstr(text_block, mindex->file_eol_str);
			if(!position){
				printf("%s", text_block);
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Help section line too long");
				err_disp();
				exit(0);
			}
			position = strstr(++position, mindex->file_eol_str);
			if(!position){
				printf("%s", text_block);
				err_push(ROUTINE_NAME, ERR_UNKNOWN, "Help section line too long, or help section nonexistant.");
				err_disp();
				exit(0);
			}
			position[0] = '\0';
			mindex->max_buffer_size = MAX_BUFFER_SIZE;
		}
		
		num_lines = 0;
		line = strstr(text_block, mindex->file_eol_str);
		while(line[0] < ' ') line++; /* Get rid of \n */
		
		/* Check the Text Block */
		switch(section_type){
		
		case MENU_SECTION:

			/* Check to see if this is a special section */
			lookup = special_list;
			found = 0;
			while(lookup->string){
				if(strcmp(lookup->string, title) == 0){
					lookup->number = 1;
					++found;
					break;
				}
				++lookup;                       
			}
			if(found)break;
			
			position = strrchr(title, '_');
			  if(check_help){ /* Check for help for this menu */
				strcpy(help_title_1, "#*");
				strcat(help_title_1, title);
				if(!strcmp(title, "MAIN MENU")) i = 1; else i = 0;
				if(mn_help_sec_find(mindex, help_title_1, &rowsize, NULL)){
					if(i == 0){ /* not in main menu */
					  ++total_errors;
					  printf("Help Section %s Not Found\n", help_title_1);
					}
				}
				else{
					if(i == 1){ /* in main menu */
					  ++total_errors;
						printf("Help section MAIN MENU_help found-\n GeoVu has no means to access this help section.\n");
					}
				}
			}
			
				
			while((line = strtok((num_lines++) ? NULL : line, "\n")) != NULL){
				for(i = strlen(line); i >= 0; i--)
					if(line[i] < ' ')
						line[i] = '\0';
					else
						break;
				
				if(line[0] == '\0')
					break; /*End of section...*/
			
				/* The first line of a menu section is the title of
				another menu which is returned to if ESCAPE is pushed */
				if(num_lines == 1){
					if((!strcmp(title, "MAIN MENU")) && (strcmp(line,"NULL"))){
							++total_errors;
							printf("Parent for MAIN MENU is not NULL\n");
							continue;
					}
					if(strcmp(title, "MAIN MENU"))
						if(mn_index_get_offset(mindex, line, &rowsize)){
							total_errors++;
							printf("Parent Menu %s Not Found\n", line);
						}
				}
								
				/* Subsequent lines should identify data files or sub-menus */
				else {
					ch_ptr = strchr(line,'#') + 1;
					/*if(ch_ptr == (char *)1)continue;*/
					if(ch_ptr[0] == '&') ch_ptr++;
					
					switch(*ch_ptr){
					case 'H':   /* help section */
						break;

						
					case '*':       /* Sub Menu */
						if(mn_index_get_offset(mindex, ch_ptr + 1, &rowsize)){
							printf("Sub-Menu %s Not Found\n", ch_ptr + 1);
							++total_errors;
						}
						/* check to make sure that a help section based on the left side
							of the # does not exist */
						if(check_help){
							strcpy(help_title_1, line);
							position = strchr(help_title_1, '#');
							position--;
							while(position[0] == ' ') position--;
							position[1] = '\0';
							position = line;
							for(i = strlen(position); i > 0; i--)
								if(position[i] <= ' ') position[i] = '\0';
								else break;
							if(!strcmp(help_title_1, ch_ptr + 1)) continue;
							strcat(help_title_1, "_help");
							
							if(!mn_index_get_offset(mindex, help_title_1, &rowsize)){
								/* we did indeed find an illegal help section */
								/* check to see if a duplicate exists */
								
								if(!(mn_help_sec_find(mindex, line, &rowsize, help_title_2))){
									/* we found a duplicate help section */
								  ++total_errors;
								  printf("Multiple (2) help sections exist for menu line\n '%s':\n", line);
								  printf(" Help sections '%s' and\n '%s' exist.\n", help_title_1, help_title_2);
								}
								else{ /* help sec illegally based on left side of # */
									printf("Help section title \"%s\"\n is incorrectly based on the user-interface", help_title_1);
									printf(" element instead of the file name\n");
									printf(" or sub-menu title. Automatically selecting the -fixhelp option.\n");
									if(!fixhelp){
									fixhelp = 1;
									}                                               
								}                                               
							}
						}
						continue;
					case '#':   /* Term file */
						if(check_data){
							ch_ptr++;
							if(*(ch_ptr) == '&') ch_ptr++;
						  help_title_2[0] = '\0';
						  if(file_prefix) strcpy(help_title_2, file_prefix);
						  strcpy(help_title_1, ch_ptr);
						  os_path_make_native(help_title_1, help_title_1);
						  strcat(help_title_2, help_title_1);
						  
						  error = open(help_title_2,O_RDONLY);
						  if(error == -1){
							 ++total_errors;
							 printf("Problem opening file %s\n", help_title_2);
							 continue;
						  }                                             
						  close(error);
						}
						continue;
			 
					case '$':       /* Text data file, increment pointer and check file */
									/* INTENTIONAL FALL THROUGH */
					case '&':
						 ++ch_ptr;
									/* INTENTIONAL FALL THROUGH */                                                               
					case '\\':      /* Data File */
						if(check_data){
							help_title_2[0] = '\0';
							if(file_prefix)strcpy(help_title_2, file_prefix);
							strcpy(help_title_1, ch_ptr);
							os_path_make_native(help_title_1, help_title_1);
							strcat(help_title_2, help_title_1);
							
							error = open(help_title_2,O_RDONLY);
							if(error == -1){
								++total_errors;
								printf("Problem opening data file %s\n", help_title_2);
								/* Commented out to check for help even if data file not found
								continue;  */
							}                                               
							else
								close(error);								
						}
							
						/* Check for data set help */
						if(check_help){
							help_secs_found = 0;
							error = 0;
							i = 0;
							strcpy(help_title_1, line);
							ch_ptr = strchr(help_title_1,'#');
							while(*(ch_ptr - 1) == ' ')--ch_ptr;
							*ch_ptr = '\0';
							strcat(help_title_1, "_help");
							
							if(!mn_index_get_offset(mindex, help_title_1, &rowsize)){
								error = 1;
								i += 4;
								help_secs_found++; /* we found a help section based on the LEFT side of the # */
							}
							
							if(!mn_help_sec_find(mindex, line, &rowsize, help_title_2)){
								help_secs_found++;
							}
							
							if(!help_secs_found){
								++total_errors;
								printf("Help Section for Line:\n%s\nNot Found.\n", line);
							}
							if(help_secs_found > 1){
								++total_errors;
								printf("Multiple (%d) help sections exist for menu line\n '%s':\n", help_secs_found, line);
								printf(" Help sections '%s' and\n '%s' exist.\n", help_title_2, help_title_1);
							}
							if(error){/* we found a help section based on the LEFT side of the # */
								printf("Help section title \"%s\"\n is incorrectly based on the user-interface", help_title_1);
								printf(" element instead of the file name\n");
								printf(" or sub-menu title. Automatically selecting the -fixhelp option.\n");
								if(!fixhelp){
												fixhelp = 1;
								}
							}
						}
						continue;
					default:
						++total_errors;
						printf("Unrecognized key character: %s\n", line);
						continue;
					}       /* switch (ch_ptr)*/
				}
			}    /* while (line) */
			break;
			
		case HEADER_SECTION:
			break;

		case HELP_SECTION:
			/* Check the text block for a pointer to a file */

			if(*line != '>')break;
			/*line[strlen(line) - 1] = '\0';*/

			/* Insert a \0 at the end of the first line */
			while(*line == '&' || *line == '>')++line;
			if(*(line) == '*') { /* Reference to another help section */
				help_title_2[0] = '\0';
				strcat(help_title_2, ++line);
				if(mn_index_get_offset(mindex, help_title_2, &rowsize)){
						++total_errors;
						printf("Referenced Help Section %s Not Found\n", help_title_2);
					}                                    
			}
			else {
				if(check_data){
					help_title_2[0] = '\0';
					if(file_prefix)strcpy(help_title_2, file_prefix);
					strcpy(help_title_1, line);
					os_path_make_native(help_title_1, help_title_1);
					strcat(help_title_2, help_title_1);
					
					error = open(help_title_2,O_RDONLY);
					if(error == -1){
					  ++total_errors;
					  printf("Problem opening help file %s\n", help_title_2);
					  break;
				  }
				  close(error);
				}                                            
			}
			break;

		case EQV_SECTION:    /* Parse the text block through the name table parser */
			ch_ptr = (char *)nt_create((char *) NULL, line);
			if(!ch_ptr)
			{
				err_push(ROUTINE_NAME, ERR_NT_DEFINE, title); 
				err_disp();
				++total_errors;
			}
			else if (lookup_keywords((NAME_TABLE_PTR)ch_ptr))
			{
				err_push(ROUTINE_NAME, ERR_NAME_TABLE, title);
				err_disp();
				++total_errors;
			}
			if (ch_ptr)
				nt_free_name_table((NAME_TABLE_PTR)ch_ptr);
			break;
			
		case FORMAT_SECTION:
			/* Parse the text block through the format parser */
			ch_ptr = (char *)ff_make_format((char *) NULL, line);
			if(!ch_ptr) {
				err_push(ROUTINE_NAME, ERR_MAKE_FORM, title);
				err_disp();
				exit(1);
			}
			ff_free_format((FORMAT_PTR)ch_ptr);
			break;
			
		case FORMAT_LIST_SECTION:
			/* Parse the text block through the eqv parser */
			ch_ptr = (char *)db_make_format_list(FFF_ALL_TYPES, (char *) NULL, line);
			if(!ch_ptr) {
				err_push(ROUTINE_NAME, ERR_FORMAT_LIST_DEF, title);
				err_disp();
				exit(1);
			}
			db_free_format_list((FORMAT_LIST_PTR)ch_ptr);
			break;

		default:
			printf("Unrecognized Section Found: %s\n", title);
			break;
		}
	}

	/* Output Special Section Information */
	if(mn_index_get_offset(mindex, "CD_MENU_NAME", &rowsize)){
		printf("Error: Section CD_MENU_NAME not found. (section is required)\n");
		total_errors++;
	}
	lookup = special_list;
	while(lookup->string){
		printf("Section %s %s\n", lookup->string, (lookup->number) ? "Found" : "Not Found");
		++lookup;                       
	}
	printf("Total Errors: %u\n", total_errors);
	if(fixhelp){
		/* we found some errors in the help section titles... */
		printf("\nAutomatically fixing help section title errors in the menu.\n");
		mn_index_free(mindex);
		goto ReIndexMenu;
	}
	exit(0);
}

/*
 * NAME:        strip_blanks
 *              
 * PURPOSE:     To strip all trailing spaces from all lines in the menufile
 *
 * USAGE:       void strip_blanks(char *filename, char *outputfilename)
 *
 * RETURNS:     void.
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none- completely portable
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS: 
 *          
 * KEYWORDS: 
 *
 */
void strip_blanks(char *filename, char *outfnm)
{
	FILE *infile;
	FILE *outfile;
	char scratch[500];
	char *position = NULL;
	char *file_eol_str;
	int i;

	if((infile = fopen(filename, MENU_FOPEN_R)) == NULL)
	{
		printf("Menu file %s not found.", filename);
		exit(1);
	}
	
	if((file_eol_str = mn_get_file_eol_str(filename)) == NULL)
	{
		printf("Couldn't determine file EOL chars");
		exit(1);
	}

	if((outfile = fopen(outfnm, MENU_FOPEN_W)) == NULL)
	{
		printf("Error opening %s: cause unknown.", outfnm);
		exit(1);
	}

	printf("Removing blanks from line ends...");
	
	while(mn_binary_fgets(scratch, 490, infile, file_eol_str)) {
		for(i = strlen(scratch); i >= 0; i--)
			if(scratch[i] <= ' ')
				scratch[i] = '\0';
			else
				break;

		strcat(scratch, file_eol_str);
		fputs(scratch, outfile);
	}
	
	printf("Done.\n\n");
	fclose(infile);
	fclose(outfile);   
}

/* These definitions are used in fillmenu */
#define TERM_FILE 1
#define HELP_FILE 0

#undef ROUTINE_NAME
#define ROUTINE_NAME "fillmenu"
/*
 * NAME:        fillmenu
 *              
 * PURPOSE:     fillmenu looks for references to files in the help sections of the
 *          menu and includes those files into the menu.
 *
 * USAGE:       void fillmenu(char *inputfilename, char *outputfilename, char *driveletter)
 *
 * RETURNS:     void.
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS: 
 *          
 * KEYWORDS: 
 *
 */
void fillmenu(char *filename, char *newfilename, char *cddrive)
{
	FILE         *infile;
	FILE         *outfile;
	menu_ref      home_node;
	menu_ref_ptr  reference;
	menu_ref_ptr  oldref;
	int           numref      = 0;
	int           numfileref  = 0;
	int           i;
	char          *scratch;
	char          *rfname;
	char          *curr_men;
	char          *position;
	char          *file_eol_str;
	char          prev_ref_found;
	char          in_help_sec = 0;
	 
	if((scratch = (char *)malloc((size_t)(sizeof(char) * 500))) == NULL)
	{
		printf("Out of memory.");
		exit(255);
	}
	if((rfname = (char *)malloc((size_t)(sizeof(char) * 500))) == NULL)
	{
		printf("Out of memory.");
		exit(255);
	}
	if((curr_men = (char *)malloc((size_t)(sizeof(char) * 200))) == NULL)
	{
		printf("Out of memory.");
		exit(255);
	}
	
	home_node.next = NULL;
	reference = &home_node;
	
	if((infile = fopen(filename, MENU_FOPEN_R)) == NULL)
	{
		printf("Menu file '%s' not found.\n", filename);
		exit(1);
	}
	if((file_eol_str = mn_get_file_eol_str(filename)) == NULL)
	{
		printf("Couldn't determine file EOL chars");
		exit(1);
	}
	if((outfile = fopen(newfilename, MENU_FOPEN_W)) == NULL)
	{
		printf("Can't open '%s' for writing.\n", newfilename);
		exit(1);
	}

	printf("Looking for references to other files...");
	while(mn_binary_fgets(scratch, 490, infile, file_eol_str)) {
		if(scratch[0] == '*'){ /* New section */
			strcpy(curr_men, scratch + 1);
			in_help_sec = 0;
			if(strncmp(scratch, "*INTRODUCTION", 13) == 0) in_help_sec = 1;
			if(strncmp(scratch + strlen(scratch) - 5 - strlen(file_eol_str), "_help", 5) == 0){ /* help section */
				in_help_sec = 1;
				fputs(scratch, outfile);
				mn_binary_fgets(scratch, 490, infile, file_eol_str);
				if(scratch[0] == '>') { /* Reference to a file */
					numref++;
					strcpy(rfname, scratch + 2);
					oldref = &home_node;
					prev_ref_found = 0;
					strcpy(scratch, ">*");
					while(oldref = oldref->next)
					{
						if(strcmp(oldref->file_ref, rfname) == 0) { /* referenced previously */
							strcat(scratch, oldref->menu_sec);
							prev_ref_found = 1;
						}
					}
					if(!prev_ref_found) { /* Create a new reference */
						numfileref++;
						oldref = reference;
						if((reference = (menu_ref_ptr)malloc(sizeof(menu_ref))) == NULL)
						{
							printf("Out of memory.");
							exit(255);
						}
						oldref->next = reference;
						reference->next = NULL;
						reference->file_type = HELP_FILE;
						if((reference->file_ref = (char *)malloc((size_t)(strlen(rfname) + 5))) == NULL)
						{
							printf("Out of memory.");
							exit(255);
						}
						strcpy(reference->file_ref, rfname);      
						if((reference->menu_sec = (char *)malloc((size_t)(strlen(rfname) + 10))) == NULL)
						{
							printf("Out of memory.");
							exit(255);
						}
						strcpy(reference->menu_sec, rfname);
						for(i = strlen(reference->menu_sec); i > 0; i--)
							if(reference->menu_sec[i] <= ' ') reference->menu_sec[i] = '\0';
							else break;
						strcat(reference->menu_sec, "_help");
						strcat(reference->menu_sec, file_eol_str);
						change_backslash(reference->menu_sec);
						strcat(scratch, reference->menu_sec);
					}
				} /* end of if(reference to a file)... */
			} /* end of if(help_section)... */
		}
		fputs(scratch, outfile);
	}
	
	fclose(infile);
	
	if(!numref) 
		printf("Done.\nNo references to help files found.\n\n");
	else {
		printf("Done.\n%d references to %d files found.\n", numref, numfileref);
		printf("Adding referenced files to the menu...");
		
		reference = &home_node;
		while(reference = reference->next)
		{
			for(i = strlen(reference->file_ref); i > 0; i--)
				if(reference->file_ref[i] <= ' ') reference->file_ref[i] = '\0';
				else break;
			strcpy(rfname, cddrive);
			strcat(rfname, reference->file_ref);
	 
			if((infile = fopen(rfname, MENU_FOPEN_R)) == NULL)
			{
				printf("\nERROR: Referenced help");
				printf(" file \"%s\" could not be found.\n", rfname);
				exit(1);
			}
	 
			if(reference->file_type == HELP_FILE) { /* add help file */
				fputs(file_eol_str, outfile);
				fputs("*", outfile);
				fputs(reference->menu_sec, outfile);
				while(mn_binary_fgets(scratch, 490, infile, file_eol_str)){
					/* Remove EOF characters */
					while((position = strchr(scratch, '\026')) != NULL)
					{
						position[0] = ' ';
					}
					fputs(scratch, outfile);
				}
			}
			fclose(infile);         
		}
		
		printf("Done.\n");
	}
	fclose(outfile);
		
	oldref = home_node.next;
	while(oldref) {
		free(oldref->file_ref);
		free(oldref->menu_sec);
		reference=oldref;
		oldref=oldref->next;
		free(reference);
	}      
	free(scratch);
	free(rfname);
	printf("Menu file filled successfully.\n\n");
}


/*
 * NAME:        change_backslash
 *              
 * PURPOSE:     converts all '\', ':', '/' to '_' in string
 *
 * USAGE:       void change_backslash(char *string)
 *
 * RETURNS:     void.
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS: 
 *          
 * KEYWORDS: 
 *
 */
void change_backslash(char *string)
{
	char *position;
	while ((position = strchr(string, '\\')) != NULL)
		position[0] = '_';
	while ((position = strchr(string, '/')) != NULL)
		position[0] = '_';
	while ((position = strchr(string, ':')) != NULL)
		position[0] = '_';
}


/*
 * NAME:        trim_menu_sec
 *              
 * PURPOSE:     removes all _eqv, _hdr, _afm, _bfm and _fmt sections from the menu
 *
 * USAGE:       void trim_menu_sec(char *inputfilename, char *outputfilename)
 *
 * RETURNS:     void.
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS: 
 *          
 * KEYWORDS: 
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "trim_menu_sec"
void trim_menu_sec(char *filename, char *outfilename)
{
	FILE *infile;
	FILE *outfile;
	FILE *trimfile;
	char *file_eol_str;
	char scratch[500];
	char scratch_end[6];
	char eat_section = 0;
	int  num_eaten   = 0;

	if((infile = fopen(filename, MENU_FOPEN_R)) == NULL)
	{
		printf("Menu file '%s' not found.\n", filename);
		exit(1);
	}
	if((file_eol_str = mn_get_file_eol_str(filename)) == NULL)
	{
		printf("Could not determine EOL chars for file");
		exit(1);
	}
	if((outfile = fopen(outfilename, MENU_FOPEN_W)) == NULL)
	{
		printf("Can't open '%s' for writing.\n", outfilename);
		exit(1);
	}
	if((trimfile = fopen("menutil.trm", MENU_FOPEN_W)) == NULL)
	{
		printf("Can't open 'menutil.trm' for writing.\n");
		exit(1);
	}
	scratch_end[4] = '\0';
	printf("Trimming sections from the menu...");
	while(mn_binary_fgets(scratch, 490, infile, file_eol_str)) {
		if(scratch[0] == '*'){ /* section name */
			eat_section = 0;
			strncpy(scratch_end, scratch + strlen(scratch) - 4 - strlen(file_eol_str), 4);
			if(
				(strcmp(scratch_end, "_eqv") == 0) ||
				(strcmp(scratch_end, "_afm") == 0) ||
				(strcmp(scratch_end, "_bfm") == 0) ||
				(strcmp(scratch_end, "_fmt") == 0) ||
				(strcmp(scratch_end, "_hdr") == 0) 
			) {eat_section = 1; num_eaten++;}
		}
		if(!eat_section) fputs(scratch, outfile); else fputs(scratch, trimfile);
	}
	printf("Done.\n");
	if(num_eaten) printf("%d sections were trimed from the menu and placed in 'menutil.trm'\n\n", num_eaten);
	else printf("No sections ending in _eqv, _afm, _bfm, _fmt or _hdr were found.\n\n");
	fclose(infile);
	fclose(outfile);
	fclose(trimfile);
}


/*
 * NAME:        make_menu_tree
 *              
 * PURPOSE:     make_menu_tree creates several schematic diagrams of the menu structure
 *
 * USAGE:       void make_menu_tree(char *inputfilename, MENU_INDEX_PTR mindex)
 *
 * RETURNS:     void.
 *
 * DESCRIPTION: creates a diagram of section structure only in file 'MenuTree'
 *              creates a diagram of section structure & files in file 'FileTree'
 *              creates a list of user interface strings and associated files in
 *              file 'FileList'
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS: 
 *          
 * KEYWORDS: menu index
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "make_menu_tree"
void make_menu_tree(char *filename, MENU_INDEX_PTR mindex)
{
	FILE *infile;
	FILE *outfile;
	FILE *outfile2;
	FILE *outfile3;
	char *scratch;
	char *outscratch;
	ulong offset;
	ulong size;
	char  depth;
	char *position;
	int   i;
	ROW_SIZES rowsize;
	
	if((scratch = (char *)malloc((size_t)(sizeof(char) * 500))) == NULL)
	{
		printf("Out of memory.\n");
		exit(255);
	}
	if((outscratch = (char *)malloc((size_t)(sizeof(char) * 500))) == NULL)
	{
		printf("Out of memory.\n");
		exit(255);
	}
	if((infile = fopen(filename, MENU_FOPEN_R)) == NULL)
	{
		printf("Menu file '%s' not found.\n", filename);
		exit(1);
	}
	if((outfile = fopen("MenuTree", MENU_FOPEN_W)) == NULL)
	{
		printf("Can't open 'MenuTree' for writing.\n");
		exit(1);
	}
	if((outfile2 = fopen("FileTree", MENU_FOPEN_W)) == NULL)
	{
		printf("Can't open 'FileTree' for writing.\n");
		exit(1);
	}
	if((outfile3 = fopen("FileList", MENU_FOPEN_W)) == NULL)
	{
		printf("Can't open 'FileList' for writing.\n");
		exit(1);
	}
	
	
	if(mn_index_get_offset(mindex, "MAIN MENU", &rowsize)){
		printf("MAIN MENU could not be found.");
		exit(255);
	}
	
	printf("Creating menu tree...");
	
	depth = 0;
	tree_stack(PUSH, &depth, (unsigned long *)&(rowsize.start), (unsigned long *)&(rowsize.num_bytes));
	while(tree_stack(POP, &depth, (unsigned long *)&offset, (unsigned long *)&size)){
		if(fseek(infile, offset, SEEK_SET)){
			printf("Can't seek position in file.");
			exit(255);
		}
		
		mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str); /* menu name */
		outscratch[0] = '\0';
		for(i = 0; i < depth; i++) strcat(outscratch, ".  ");
		strcat(outscratch, scratch + 1);
		fputs(outscratch, outfile);
		fputs(outscratch, outfile2);
		depth++;
		strncpy(outscratch, scratch + strlen(scratch) - 5 - mindex->file_eollen, 5);
		outscratch[5] = '\0';
		if(strcmp(outscratch, "_help") == 0) continue;
		
		mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str); /* parent menu name */
		while(mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str)){
			if(scratch[0] == '*') break;
			if((position = strstr(scratch, "#*")) != NULL)
			{
				if(mn_index_get_offset(mindex, ++position, &rowsize)){
					for(i = strlen(position); i > 0; i--)
						if(position[i] <= ' ') position[i] = '\0';
						else break;
					sprintf(outscratch, "Menu file bad-\nReferenced section \"%s\" not found.", position);
					printf("%s", outscratch);
					exit(255);
				}
				tree_stack(PUSH, &depth, (unsigned long *)&(rowsize.start), (unsigned long *)&(rowsize.num_bytes));
				continue;
			}
			if((position = strstr(scratch, "#H*")) != NULL)
			{ /* term section */
				++position;
				
				if(mn_index_get_offset(mindex, ++position, &rowsize)){
					for(i = strlen(position); i > 0; i--)
						if(position[i] <= ' ') position[i] = '\0';
						else break;
					sprintf(outscratch, "Menu file bad-\nReferenced section \"%s\" not found.", position);
					printf("%s", outscratch);
					exit(255);
				}
				tree_stack(PUSH, &depth, (unsigned long *)&(rowsize.start), (unsigned long *)&(rowsize.num_bytes));
				continue;
			}
			if(!strchr(scratch, '#')) continue;
			fputs(scratch, outfile3);
			outscratch[0] = '\0';
			for(i = 0; i < depth; i++) strcat(outscratch, ".  ");
			position = strstr(scratch, "#");
			strcat(outscratch, "-");
			strcat(outscratch, ++position);
			fputs(outscratch, outfile2);
		}
	}
	printf("Done.\nTree of sections only is located in the file 'MenuTree'.\n");
	printf("Tree of sections and files is located in the file 'FileTree'.\n");
	printf("List of user interface text and associated files is located\n");
	printf("in the file 'FileList'.\n");
	fclose(infile);
	fclose(outfile);
	fclose(outfile2);
	fclose(outfile3);
	free(scratch);
	free(outscratch);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "tree_stack"
/* tree_stack is a simple stack which is used in make_menu_tree */
int tree_stack(int op, char *depth, ulong *offset, ulong *size)
{  
	static tstacke_ptr top = NULL;
	tstacke_ptr node;
	
	if(op == PUSH){
		if((node = (tstacke_ptr)malloc(sizeof(tstacke))) == NULL)
		{
			printf("Out of memory in tree_stack");
			exit(255);
		}
		node->prev = top;
		node->offset = *(offset);
		node->depth = *(depth);
		node->size = *(size);
		top = node;
		return(1);
	}
	else { /* POP */
		if(!top) return(0);
		*(offset) = top->offset;
		*(depth) = top->depth;
		*(size) = top->size;
		node = top;
		top = top->prev;
		free(node);
		return(1);
	}
}





/*
 * NAME:        fix_help_titles
 *              
 * PURPOSE:     to change all help section titles based on the user-interface element of
 *          a menu line to a title based on the item element of the menu line.
 *
 * USAGE:   fix_help_titles(char *inputfilename, char *outputfilename, MENU_INDEX_PTR mindex)
 *
 * RETURNS:     (char) 1 if mistitled help section(s) were found (and fixed), 0 if not
 *          (note: a return of 0 also means that the file was NOT copied to outputfilename)
 *
 * DESCRIPTION: changes all mistitled help section titles to correct titles.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS:
 *          
 * KEYWORDS: menu index
 *
 */
#undef FUNCT
#define FUNCT "fix_help_titles"
/* fix_help_titles looks for help sections based on the left-hand side of the # and changes
 * their titles to ones based on the right-hand side of the # */
char fix_help_titles(char *filename, char *outfilename, MENU_INDEX_PTR mindex)
{
	FILE *infile;
	FILE *outfile;
	char *scratch;
	char *scratch2;
	char *scratch3;
	char *position;
	char depth            = 0;
	ulong offset          = 0;
	ulong size          = 0;
	int i, j;
	int numchanged        = 0;
	menu_ref_ptr homenode = NULL;
	menu_ref_ptr node     = NULL;
	menu_ref_ptr oldnode  = NULL;
	ROW_SIZES rowsize;

	if((scratch = (char *)malloc((size_t)(sizeof(char) * 500))) == NULL)
	{
		printf("Out of memory in function fix_help_titles\n");
		exit(255);
	}
	if((scratch2 = (char *)malloc((size_t)(sizeof(char) * 500))) == NULL)
	{
		printf("Out of memory in function fix_help_titles\n");
		exit(255);
	}
	if((scratch3 = (char *)malloc((size_t)(sizeof(char) * 200))) == NULL)
	{
		printf("Out of memory in function fix_help_titles\n");
		exit(255);
	}
	if((homenode = (menu_ref_ptr)malloc(sizeof(menu_ref))) == NULL)
	{
		printf("Out of memory in function fix_help_titles\n");
		exit(255);
	}
	homenode->next = NULL;
	node = homenode;
	if((infile = fopen(filename, "rb")) == NULL)
	{
		printf("Menu file '%s' not found.\n", filename);
		exit(1);
	}
	 
	 
	if(mn_index_get_offset(mindex, "MAIN MENU", &rowsize)){
		printf("Can't find main menu.\n");
		exit(255);
	}
	printf("Looking for mistitled help sections...");
	tree_stack(PUSH, &depth, (unsigned long *)&(rowsize.start), (unsigned long *)&(rowsize.num_bytes));
	while(tree_stack(POP, &depth, &offset, &size)){
		if(fseek(infile, offset, SEEK_SET)){
			printf("Error in function fix_help_titles: cannot seek position in file.");
			exit(255);
		}
		mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str); /* menu name */
		mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str); /* parent menu name */
		while(mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str)){
			if(scratch[0] == '*') break; /* new section */
			if((position = strstr(scratch, "#*")) != NULL)
			{
				for(i = strlen(position); i > 0; i--)
					if(position[i] <= ' ') position[i] = '\0';
					else break;
				if(!mn_index_get_offset(mindex, ++position, &rowsize)){
					printf("Cannot find section %s.", position);
					exit(255);
				}
				tree_stack(PUSH, &depth, (unsigned long *)&(rowsize.start), (unsigned long *)&(rowsize.num_bytes));
				if(mn_help_sec_find(mindex, scratch, &rowsize, NULL)){
					strcat(position, "_help");
					strcpy(scratch2, position);
					position = strchr(scratch, '#');
					position[0] = '\0';
					position = scratch;
					for(i = strlen(position); i > 0; i--)
						if(position[i] <= ' ') position[i] = '\0';
						else break;
					strcat(scratch, "_help");
					strcpy(scratch3, "*");
					strcat(scratch3, scratch);
					if(!mn_index_get_offset(mindex, scratch3, &rowsize)){
						/* Illegal help sec title found */
						numchanged++;
						oldnode = node; /* add the title and its replacement to our DLL... */
						if((node = (menu_ref_ptr)malloc(sizeof(menu_ref))) == NULL)
						{
							printf("Out of memory in fix_help_titles");
							exit(0);
						}
						if((node->file_ref = (char *)malloc((size_t)(sizeof(char) * (strlen(scratch) + 3)))) == NULL)
						{
							printf("Out of memory in fix_help_titles");
							exit(0);
						}
						if((node->menu_sec = (char *)malloc((size_t)(sizeof(char) * (strlen(scratch2) + 3)))) == NULL)
						{
							printf("Out of memory in fix_help_titles");
							exit(0);
						}
						strcpy(node->file_ref, scratch3);
						strcpy(node->menu_sec, scratch2);
						node->next = NULL;
						oldnode->next = node;
					}
				}
				continue;
			}
			if((position = strstr(scratch, "#H*")) != NULL)
			{ /* term section */
				continue; /* There should never be any help for a term section */
			}
			if(!strchr(scratch, '#')) continue;
			if(mn_help_sec_find(mindex, scratch, &rowsize, NULL)){
				continue;
			}
			position = strstr(scratch, "#");
			position[0] = '\0';
			position = scratch;
			for(i = strlen(position); i > 0; i--)
				if(position[i] <= ' ') position[i] = '\0';
				else break;
			strcat(position, "_help");
			strcpy(scratch3, "*");
			strcat(scratch3, position);
			if(!mn_index_get_offset(mindex, scratch3, &rowsize)){
				/* Hit an illegal help section title */
				numchanged++;
				oldnode = node; /* add the title and its replacement to our DLL... */
				if((node = (menu_ref_ptr)malloc(sizeof(menu_ref))) == NULL)
				{
					printf("Out of memory in fix_help_titles");
					exit(255);
				}
				if((node->file_ref = (char *)malloc((size_t)(sizeof(char) * (strlen(scratch) + 3)))) == NULL)
				{
					printf("Out of memory in fix_help_titles");
					exit(255);
				}
				if((node->menu_sec = (char *)malloc((size_t)(sizeof(char) * (strlen(scratch2) + 3)))) == NULL)
				{
					printf("Out of memory in fix_help_titles");
					exit(255);
				}
				strcpy(node->file_ref, scratch3);
				strcpy(node->menu_sec, scratch2);
				node->next = NULL;
				oldnode->next = node;
			}
		}
	}
	printf("Done.\n");
	if(numchanged) {
		printf("%d mistitled help sections were found.\n", numchanged);
		/* Now, to swap the illegal titles with the legal ones... */
		printf("Retitling help sections...\n");
		fclose(infile);
		node = homenode;
		while(node = node->next)
			printf("%s -> %s\n", node->file_ref, node->menu_sec);
		if((infile = fopen(filename, MENU_FOPEN_R)) == NULL)
		{
			printf("Unable to re-open menu file.\n");
			exit(255);
		}
		if((outfile = fopen(outfilename, MENU_FOPEN_W)) == NULL)
		{
			printf("Unable to open '%s' for writing.\n", outfilename);
			exit(1);
		}
		while(mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str)){
			if(scratch[0] == '*'){ /* new section */
				position = scratch;
				for(i = strlen(position); i > 0; i--)
					if(position[i] <= ' ') position[i] = '\0';
					else break;
				strncpy(scratch2, scratch + strlen(scratch) - 5, 5);
				scratch2[5] = '\0';
				if(strcmp(scratch2, "_help") == 0){ /* we hit a help section */
					for(i = 0; i < 2; i++){
						node = homenode;
						while(node = node->next)
						{ /* loop through the DLL to see if this is bad sec */
							if(!strcmp(node->file_ref, position)){ /* we found a bad section title */
								strcpy(position, node->menu_sec);
								break;
							}
						}
						if(i == 1) break;
						strcat(scratch, mindex->file_eol_str);
						fputs(scratch, outfile);
						mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str);
						if((scratch[0] == '>') && (scratch[1] == '*')){ /* ref to another help sec */
							position = scratch + 1;
							for(j = strlen(position); j > 0; j--)
								if(position[j] <= ' ') position[j] = '\0';
								else break;
						}
						else{ /* not a ref to another help section */
						/* this is necessary so that we don't get a blank line added in */
							position = scratch; 
							for(j = strlen(position); j > 0; j--)
								if(position[j] <= ' ') position[j] = '\0';
								else break;
							break; /* we don't have to search through again... */
						}
					}
				}
				strcat(scratch, mindex->file_eol_str);
			}
			fputs(scratch, outfile);
		}
		printf("Done.\n\n");
		fclose(outfile);
	}
	else{
		printf("No mistitled help sections were found.\n\n");
	}
	fclose(infile);
	free(scratch);
	free(scratch2);
	oldnode = homenode->next;
	while(oldnode) {
		free(oldnode->file_ref);
		free(oldnode->menu_sec);
		node=oldnode;
		oldnode=oldnode->next;
		free(node);
	}
	free(homenode);
	if(numchanged) return(1); else return(0);
}


/*
 * NAME:        make_files_lowercase
 *              
 * PURPOSE:     to change all file names in a menu to lowercase
 *
 * USAGE:   	int make_files_lowercase(MENU_INDEX_PTR mindex, char *outfilename)
 *
 * RETURNS:     (int) 0 if all is OK, !=0 otherwise
 *
 * DESCRIPTION: changes all file names to lowercase
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS:
 *          
 * KEYWORDS: menu index
 *
 */
#undef FUNCT
#define FUNCT "make_files_lowercase"
#undef ROUTINE_NAME
#define ROUTINE_NAME "make_files_lowercase"
int make_files_lowercase(MENU_INDEX_PTR mindex, char *outfilename)
{
	FILE *outfile;
	FILE *infile;
	char *position;
	char scratch[500];
	char in_help = 0;
	int  i;
	ROW_SIZES_PTR rowsize = NULL;
	QSTACK_PTR queue;
	
	if((infile = fopen(mindex->menu_file, MENU_FOPEN_R)) == NULL)
	{
		printf("Could not open %s for reading\n", mindex->menu_file);
		exit(1);
	}
	if((outfile = fopen(outfilename, MENU_FOPEN_W)) == NULL)
	{
		printf("Could not open %s for writing\n", outfilename);
		exit(1);
	}
	
	if((queue = (QSTACK_PTR)queue_stack_op(NULL, QS_NEW, NULL)) == NULL)
	{
		printf("Out of memory\n");
		exit(1);
	}
	
	while(mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str)){
		if(scratch[0] == '*'){ /* new section */
			in_help = 0;
			if(strstr(scratch, "_help")){ /* help section */
				in_help = 1;
				fputs(scratch, outfile);
				mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str);
				if(scratch[0] == '>'){
					if(scratch[1] != '*'){ /* File reference */
						for(i = strlen(scratch); i > 0; i--)
							scratch[i] = (char)tolower(scratch[i]);
					}
				}
			}
		}
		else{
			if(!in_help){
				position = strstr(scratch, "#\\");
				if(!position)
					position = strstr(scratch, "##");
				if(!position)
					position = strstr(scratch, "#$\\");
						
				if(position){ /* We hit a file name! */
					if(!rowsize){
						if((rowsize = (ROW_SIZES_PTR)malloc(sizeof(ROW_SIZES))) == NULL)
						{
							printf("Out of memory");
							exit(1);
						}
					}
					if(!(mn_help_sec_find(mindex, scratch, rowsize, NULL))){
						/* Help exists for this file name- put the section 
					 	* offset on a stack so we can make it lowercase later */
						if(!(queue_stack_op(queue, QS_PUSH, (void *)rowsize))){
							printf("Out of memory\n");
							exit(1);
						}
						rowsize = NULL;
					}
				
					/* Make the filename lowercase */
					for(i = strlen(position); i > 0; i--)
						position[i] = (char)tolower(position[i]);				
				}
			}
		}
		fputs(scratch, outfile);
	}
	fclose(infile);
	fclose(outfile);
	if(rowsize)
		free(rowsize);
	
	/* Done writing the filenames, now fix help sections */
	if((infile = fopen(outfilename, MENU_FOPEN_U)) == NULL)
	{
		printf("Could not open %s for update\n", outfilename);
		exit(1);
	}
	
	while((rowsize = (ROW_SIZES_PTR)queue_stack_op(queue, QS_PULL, NULL)) != NULL)
	{	
		if(fseek(infile, rowsize->start, SEEK_SET)){ /* error in fseek */
			printf("Cannot seek to position in file\n");
			exit(1);
		}
		
		mn_binary_fgets(scratch, 490, infile, mindex->file_eol_str);
		if(fseek(infile, rowsize->start, SEEK_SET)){ /* error in fseek */
			printf("Cannot seek to position in file\n");
			exit(1);
		}
		
		for(i = strlen(scratch); i > 0; i--){
			scratch[i] = (char)tolower(scratch[i]);
			if(scratch[i] < ' ')
				scratch[i] = '\0';
		}
		fwrite(scratch, (size_t)strlen(scratch), 1, infile);
	}
	
	queue_stack_op(queue, QS_KILL, NULL);
	
	fclose(infile);
	
	return(0);
}

static BOOLEAN lookup_keywords(NAME_TABLE_PTR nt)
/*****************************************************************************
 * NAME:  lookup_keywords()
 *
 * PURPOSE:  Check defined GeoVu name equivalences and constants against
 * recognized keywords.
 *
 * USAGE:  error = lookup_keywords(name_table_ptr);
 *
 * RETURNS:  TRUE if an unrecognized keyword was found
 *
 * DESCRIPTION:  Checks only for the existence and usage of GeoVu keywords
 * AS DOCUMENTED!!!  There are several classes or types of GeoVu keywords:
 * numeric in value, enumerated in value (either numeric or non-numeric),
 * non-numeric in value.  Non-numeric might be slide_show_command or
 * format_dir.  Enumerated might be byter_per_pixel or scaling.  Numeric might
 * be number_of_rows.
 *
 * If a keyword is non-numeric, its type is checked to see if it is type char.
 * If a keyword is enumerated, its value is checked against a lookup table,
 * or by simple constant comparisons. 
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

#define NUM_ConvGeoVuKeywords 89
#define NUM_CharOnlyGeoVuKeywords 40
#define NUM_EnumGeoVuKeywords 23
#define NUM_Data_Representation 7
#define NUM_Data_Type 4
#define NUM_Grid_Origin 8
#define NUM_Header_Type 4
#define NUM_Image_Format 4
#define NUM_Map_Projection 23 /* this needs to be the same as in geodata.h */
#define NUM_Auto_Display 3
#define NUM_Standard_Format_Name 9
#define NUM_Scaling 3
#define NUM_Grid_Cell_Registration 5

#undef ROUTINE_NAME
#define ROUTINE_NAME "EQV_CHECKER"
{

const char *ConvGeoVuKeywords[NUM_ConvGeoVuKeywords + 1] =
{
{"UTM_x_max"},
{"UTM_x_min"},
{"UTM_y_max"},
{"UTM_y_min"},
{"UTM_zone"},
{"_distance"},
{"auto_display"},
{"base_latitude"},
{"bytes_per_pixel"},
{"central_meridian"},

{"data_byte_order"},
{"data_representation"},
{"data_type"},
{"default_band"},
{"delimiter_item"},
{"delimiter_value"},
{"dummy_embedded_header_length"},
{"end_column"},
{"end_row"},
{"false_east"},

{"ff_format_fmt"},
{"ff_header_format"},
{"ff_input_format"},
{"ff_output_format"},
{"file_title"},
{"first_parallel"},
{"format_dir"},
{"grid_cell_registration"},
{"grid_origin"},
{"grid_size"},

{"grid_size(x)"},
{"grid_size(y)"},
{"grid_unit"},
{"hdf_ref"},
{"hdf_tag"},
{"header_dir"},
{"header_file_ext"},
{"header_file_name"},
{"header_file_path"},
{"header_length"},

{"header_type"},
{"histogram_dir"},
{"image_format"},
{"index_file_name"},
{"input_repeat"},
{"keep_preview"},
{"latitude_max"},
{"latitude_min"},
{"left_map_x"},
{"longitude_max"},

{"longitude_min"},
{"lower_map_y"},
{"map_projection"},
{"maximum_value"},
{"minimum_value"},
{"missing_flag"},
{"number_of_bands"},
{"number_of_classes"},
{"number_of_columns"},
{"number_of_display_colors"},

{"number_of_rows"},
{"omit_grid_embedded_header"},
{"output_repeat"},
{"palette"},
/*{"plot_drawing_default},*/
/*{"plot_x_default"},*/
/*{"plot_y_default"},*/
{"preview"},
{"preview_command"},
{"preview_path"},
{"record_count"},
{"record_header_length"},
{"right_map_x"},

{"row_number_of_bytes"},
{"row_start_position"},
{"row_step"},
{"scaling"},
{"second_parallel"},
{"slide_show_command"},
{"slide_show_ext"},
{"slide_show_path"},
{"standard_format_name"},
{"start_column"},

{"start_row"},
{"term_definition"},
{"upper_map_y"},
{"user_define_row_structure"},
{"user_select_data_type"},
{"value_type"},
{"value_unit"},
{"view_command"},
{"view_ext"},
{(char *)NULL},
};

const char *CharOnlyGeoVuKeywords[NUM_CharOnlyGeoVuKeywords + 1] =
{
{"auto_display"},
{"data_byte_order"},
{"data_representation"},
{"data_type"},
{"delimiter_item"},
{"delimiter_value"},
{"ff_format_fmt"},
{"ff_header_format"},
{"ff_input_format"},
{"ff_output_format"},

{"file_title"},
{"format_dir"},
{"grid_cell_registration"},
{"grid_unit"},
{"header_dir"},
{"header_file_ext"},
{"header_file_name"},
{"header_file_path"},
{"header_type"},
{"histogram_dir"},

{"image_format"},
{"index_file_name"},
{"keep_preview"},
{"omit_grid_embedded_header"},
{"palette"},
/*{"plot_drawing_default},*/
{"preview"},
{"preview_command"},
{"preview_path"},
{"scaling"},
{"slide_show_command"},

{"slide_show_ext"},
{"slide_show_path"},
{"standard_format_name"},
{"term_definition"},
{"user_define_row_structure"},
{"user_select_data_type"},
{"value_type"},
{"value_unit"},
{"view_command"},
{"view_ext"},

{(char *)NULL},
};

	char *EnumGeoVuKeywords[NUM_EnumGeoVuKeywords + 1] = 
{
{"auto_display"},
{"bytes_per_pixel"},
{"data_byte_order"},
{"data_representation"},
{"data_type"},
{"ff_format_fmt"},
{"ff_header_format"},
{"ff_input_format"},
{"ff_output_format"},
{"grid_cell_registration"},

{"grid_origin"},
{"header_type"},
{"image_format"},
{"keep_preview"},
{"map_projection"},
{"number_of_display_colors"},
{"omit_grid_embedded_header"},
/*{"plot_drawing_default"},*/
{"preview"},
{"scaling"},
{"standard_format_name"},

{"user_define_row_structure"},
{"user_select_data_type"},
{"value_type"},
{(char *)NULL},
};

/* case insensitive */
const char *auto_display[NUM_Auto_Display + 1] =
{
{"no"},
{"true"},
{"yes"},
{(char *)NULL},
};

/* case insensitive */
const char *data_representation[NUM_Data_Representation + 1] =
{
{"double"},
{"float"},
{"long"},
{"short"},
{"uchar"},
{"ulong"},
{"ushort"},
{(char *)NULL},
};

/* case insensitive */
const char *data_type[NUM_Data_Type + 1] =
{
{"image"},
{"point"},
{"raster"},
{"vector"},
{(char *)NULL},
};

/* case insensitive */
const char *grid_cell_registration[NUM_Grid_Cell_Registration + 1] =
{
{"center"},
{"lowerleft"},
{"lowerright"},
{"upperleft"},
{"upperright"},
{(char *)NULL},
};

/* case insensitive */
const char *grid_origin[NUM_Grid_Origin + 1] =
{
{"lowerleft"},
/*{"lowerleft_x"},*/
{"lowerleft_y"},
{"lowerright"},
/*{"lowerright_x"},*/
{"lowerright_y"},
{"upperleft"},
/*{"upperleft_x"},*/
{"upperleft_y"},
{"upperright"},
/*{"upperright_x"},*/
{"upperright_y"},
{(char *)NULL},
};

/* case insensitive */
const char *header_type[NUM_Header_Type + 1] =
{
{"header_embedded"},
{"header_embedded_varied"},
{"header_separated"},
{"header_separated_varied"},
{(char *)NULL},
};

/* case insensitive */
const char *image_format[NUM_Image_Format + 1] =
{
{"BIL"},
{"BIP"},
{"BIP2"},
{"BSQ"},
{(char *)NULL},
};

/* case insensitive */
const char *map_projection[NUM_Map_Projection + 1] =
{
{"Albers conical equal area"},
{"Azimuthal equidistant"},
{"Equidistant conic"},
{"Equirectangular"},
{"General vertical near_side perspective"},
{"Gnomonic"},
{"Lambert azimuthal equal area"},
{"Lambert conformal conic"},
{"Lat/Lon"},
{"Mercator"},
{"Miller cylindrical"},
{"none"},
{"Oblique Mercator"},
{"Orthographic"},
{"Polar stereographic"},
{"Polyconic"},
{"Sinusoidal"},
{"State plane"},
{"Stereographic"},
{"Transverse Mercator"},
{"User-Defined Projection"},
{"UTM"},
{"Van der Grinten"},
{(char *)NULL},
};

/* case insensitive */
const char *scaling[NUM_Scaling + 1] =
{
{"Frequency"},
{"Histoeq"},
{"Linear"},
{(char *)NULL},
};

/* case insensitive */
const char *standard_format_name[NUM_Standard_Format_Name + 1] =
{
{"erdas_ra"},
{"gac16"},
{"geovu_ra"},
{"idrisi3"},
{"idrisi4"},
{"mcidas_d"},
{"mcidas_u"},
{"pcx"},
{"user"},
{(char *)NULL},
};

	char *cp = NULL;
	char scratch[3 * MAX_NAME_LENGTH];
	VARIABLE_PTR var;
	VARIABLE_LIST_PTR var_list;
	BOOLEAN is_this_found = FALSE;
	BOOLEAN at_least_one_not_found = FALSE;
	BOOLEAN at_least_one_error = FALSE;
	
	short s;
	
#ifdef DEBUG
	int i;
	
	for (i = 0; i < NUM_ConvGeoVuKeywords - 1; i++)
		if (compare(&(ConvGeoVuKeywords[i]), &(ConvGeoVuKeywords[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", ConvGeoVuKeywords[i], ConvGeoVuKeywords[i + 1]);
			exit(EXIT_FAILURE);
		}

	for (i = 0; i < NUM_CharOnlyGeoVuKeywords - 1; i++)
		if (compare(&(CharOnlyGeoVuKeywords[i]), &(CharOnlyGeoVuKeywords[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", CharOnlyGeoVuKeywords[i], CharOnlyGeoVuKeywords[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_EnumGeoVuKeywords - 1; i++)
		if (compare(&(EnumGeoVuKeywords[i]), &(EnumGeoVuKeywords[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", EnumGeoVuKeywords[i], EnumGeoVuKeywords[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Auto_Display - 1; i++)
		if (comparei(&(auto_display[i]), &(auto_display[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", auto_display[i], auto_display[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Data_Representation - 1; i++)
		if (comparei(&(data_representation[i]), &(data_representation[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", data_representation[i], data_representation[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Data_Type - 1; i++)
		if (comparei(&(data_type[i]), &(data_type[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", data_type[i], data_type[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Grid_Cell_Registration - 1; i++)
		if (comparei(&(grid_cell_registration[i]), &(grid_cell_registration[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", grid_cell_registration[i], grid_cell_registration[i + 1]);
			exit(EXIT_FAILURE);
		}

	for (i = 0; i < NUM_Grid_Origin - 1; i++)
		if (comparei(&(grid_origin[i]), &(grid_origin[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", grid_origin[i], grid_origin[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Header_Type - 1; i++)
		if (comparei(&(header_type[i]), &(header_type[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", header_type[i], header_type[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Image_Format - 1; i++)
		if (comparei(&(image_format[i]), &(image_format[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", image_format[i], image_format[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Map_Projection - 1; i++)
		if (comparei(&(map_projection[i]), &(map_projection[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", map_projection[i], map_projection[i + 1]);
			exit(EXIT_FAILURE);
		}
	
	for (i = 0; i < NUM_Scaling - 1; i++)
		if (comparei(&(scaling[i]), &(scaling[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", scaling[i], scaling[i + 1]);
			exit(EXIT_FAILURE);
		}

	for (i = 0; i < NUM_Standard_Format_Name - 1; i++)
		if (comparei(&(standard_format_name[i]), &(standard_format_name[i + 1])) >= 0)
		{
			printf("Out of sequence: %s / %s\n", standard_format_name[i], standard_format_name[i + 1]);
			exit(EXIT_FAILURE);
		}
#endif
	
	assert(nt);
	assert(nt == nt->check_address);
	assert(nt->format);
	assert(nt->format == nt->format->check_address);
	
	var_list = dll_first(nt->format->variables);
	while ((var = FFV_VARIABLE(var_list)) != NULL)
	{
		assert(var == var->check_address);
		
		is_this_found = FALSE;
		cp = var->name;
		
		if (bsearch((char *)&cp, (char *)ConvGeoVuKeywords,
		            NUM_ConvGeoVuKeywords, sizeof(char *),
		            (int(*)(void const *, void const *))compare) != NULL)
			is_this_found = TRUE;
		
		if (is_this_found == FALSE)
		{
			assert(strlen(var->name) < sizeof(scratch));
			
			strncpy(scratch, var->name, sizeof(scratch) - 1);
			scratch[sizeof(scratch) - 1] = STR_END;
		}
		
		/* band_#_unit */
		if (is_this_found == FALSE)
		{
			if (strncmp(scratch, "band_", 5) == 0)
			{
				cp = strstr(scratch, "_unit");
				if (cp)
				{
					if (strlen(cp) == 5)
					{
						memset(cp, '\x20', 5);
						(void)os_str_trim_whitespace(scratch, scratch);
						
						(void)strtol(scratch + 5, &cp, 10);
						if (ok_strlen(cp) == 0)
							is_this_found = TRUE;
					}
				}
			}
		}
		
		/* category# */
		if (is_this_found == FALSE)
		{
			if (strncmp(scratch, "category", 8) == 0)
			{
				(void)strtol(scratch + 8, &cp, 10);
				if (ok_strlen(cp) == 0)
					is_this_found = TRUE;
			}
		}
		
		/* class_# */
		if (is_this_found == FALSE)
		{
			if (strncmp(scratch, "class_", 6) == 0)
			{
				(void)strtol(scratch + 6, &cp, 10);
				if (ok_strlen(cp) == 0)
					is_this_found = TRUE;
			}
		}
		
		/* var_#_max or var_#_min */
		if (is_this_found == FALSE)
		{
			if (strncmp(scratch, "var_", 4) == 0)
			{
				cp = strstr(scratch, "_max");
				if (!cp)
					cp = strstr(scratch, "_min");
				if (cp)
				{
					if (strlen(cp) == 4)
					{
						(void)strtol(scratch + 4, &cp, 10);
						if (ok_strlen(cp) == 0)
							is_this_found = TRUE;
					}
				}
			}
		}
		
		/* var_name_max or var_name_min */
		if (is_this_found == FALSE)
		{
			cp = strstr(scratch, "_min");
			if (!cp)
				cp = strstr(scratch, "_max");
			if (ok_strlen(cp) == 4)
				/* Read above comment */
				is_this_found = TRUE;
		}
		
		/* var_name_unit */
		if (is_this_found == FALSE)
		{
			cp = strstr(scratch, "_unit");
			if (ok_strlen(cp) == 5)
				/* Read above comment */
				is_this_found = TRUE;
		}
		
		if (is_this_found == FALSE)
		{
			sprintf(scratch, "Unrecognized (mistyped?) GeoVu keyword: %s\n==> %s %s ",
			        var->name, var->name,
              ff_lookup_string(variable_types, FFV_TYPE(var)));
			if (IS_CHAR(var))
				strcat(scratch, nt->data->buffer + var->start_pos - 1);
			else
				ff_binary_to_string(nt->data->buffer + var->start_pos - 1,
				                    FFV_TYPE(var), scratch + strlen(scratch), 0);
			strcat(scratch, " <==");
			err_push(ROUTINE_NAME, ERR_UNKNOWN, scratch);
			at_least_one_not_found = TRUE;
		}
		else /* begin table lookups */
		{
			if (IS_CONSTANT(var))
			{
				cp = var->name;
				
				if (   !IS_CHAR(var)
				    && bsearch( (char *)&cp, (char *)CharOnlyGeoVuKeywords
				               , NUM_CharOnlyGeoVuKeywords, sizeof(char *)
				               , (int(*)(void const *, void const *))compare
				              ) != NULL
				   )
				{
					at_least_one_error = TRUE;
					sprintf(scratch, "%s must be type char\n==> %s %s ",
					        var->name, var->name,
						      ff_lookup_string(variable_types, FFV_TYPE(var)));
					ff_binary_to_string(nt->data->buffer + var->start_pos - 1,
					                    FFV_TYPE(var), scratch + strlen(scratch), 0);
					strcat(scratch, " <==");
					err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
				}
				else
				{
					cp = var->name;
						
					if (bsearch((char *)&cp, (char *)EnumGeoVuKeywords,
					            NUM_EnumGeoVuKeywords, sizeof(char *),
					            (int(*)(void const *, void const *))compare) != NULL)
	        {
						cp = nt->data->buffer + var->start_pos - 1;
							
	        	if (strcmp(var->name, "auto_display") == 0)
	        	{
							if (bsearch((char *)&cp, (char *)auto_display,
							            NUM_Auto_Display, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal yes, true, or no\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
	        	}
						else if (strcmp(var->name, "bytes_per_pixel") == 0)
						{
							if (btype_to_btype(cp, FFV_TYPE(var), &s, FFV_SHORT))
							{
								at_least_one_error = TRUE;
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, var->name);
							}
							else
							{
								if (s != 1 && s != 2 && s != 4 && s != 8)
								{
									at_least_one_error = TRUE;
									sprintf(scratch, "%s must equal 1, 2, 4, or 8\n==> %s %s %dh <==",
									        var->name, var->name,
									        ff_lookup_string(variable_types, FFV_TYPE(var)), s);
									err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
								}
							}
						}
						else if (strcmp(var->name, "data_byte_order") == 0)
						{
							if (strcmpi(cp, "big_endian") && strcmpi(cp, "little_endian"))
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal big_endian or little_endian\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "data_representation") == 0)
						{
							if (bsearch((char *)&cp, (char *)data_representation,
							            NUM_Data_Representation, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal uchar, short, ushort, long, ulong, float, or double\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "data_type") == 0)
						{
							if (bsearch((char *)&cp, (char *)data_type,
							            NUM_Data_Type, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal point, vector, image, or raster\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "ff_header_format") == 0)
						{
							if (   strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_afm" : ".afm"
							               , 4)
							    && strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_bfm" : ".bfm"
							               , 4)
							    && strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_dfm" : ".dfm"
							               , 4)
							   )
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "Improper %s (should be %cafm, %cbfm, or %cdfm)\n==> %s %s %s <==",
								        cp[0] == '*' ? "suffix" : "extension",
								        cp[0] == '*' ? '_' : '.',
								        cp[0] == '*' ? '_' : '.',
								        cp[0] == '*' ? '_' : '.',
								        var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "ff_input_format") == 0)
						{
							if (   strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_afm" : ".afm"
							               , 4)
							    && strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_bfm" : ".bfm"
							               , 4)
							    && strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_dfm" : ".dfm"
							               , 4)
							   )
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "Improper %s (should be %cafm, %cbfm, or %cdfm)\n==> %s %s %s <==",
								        cp[0] == '*' ? "suffix" : "extension",
								        cp[0] == '*' ? '_' : '.',
								        cp[0] == '*' ? '_' : '.',
								        cp[0] == '*' ? '_' : '.',
								        var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "ff_output_format") == 0)
						{
							if (   strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_afm" : ".afm"
							               , 4)
							    && strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_bfm" : ".bfm"
							               , 4)
							    && strncmp(  cp + strlen(cp) - 4
							               , cp[0] == '*' ? "_dfm" : ".dfm"
							               , 4)
							   )
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "Improper %s (should be %cafm, %cbfm, or %cdfm)\n==> %s %s %s <==",
								        cp[0] == '*' ? "suffix" : "extension",
								        cp[0] == '*' ? '_' : '.',
								        cp[0] == '*' ? '_' : '.',
								        cp[0] == '*' ? '_' : '.',
								        var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "ff_format_fmt") == 0)
						{
							if (strncmp(  cp + strlen(cp) - 4
							            , cp[0] == '*' ? "_fmt" : ".fmt"
							            , 4)
							   )
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "Improper %s (should be %cfmt)\n==> %s %s %s <==",
								        cp[0] == '*' ? "suffix" : "extension",
								        cp[0] == '*' ? '_' : '.',
								        var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "grid_cell_registration") == 0)
						{
							if (bsearch((char *)&cp, (char *)grid_cell_registration,
							            NUM_Grid_Cell_Registration, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal upperleft, lowerleft, ..., or center\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "grid_origin") == 0)
						{
							if (IS_CHAR(var))
							{
								if (bsearch((char *)&cp, (char *)grid_origin,
								            NUM_Grid_Origin, sizeof(char *),
								            (int(*)(void const *, void const *))comparei) == NULL)
								{
									at_least_one_error = TRUE;
									sprintf(scratch, "%s must equal upperleft, lowerleft, ..., or upperright_y\n==> %s %s %s <==",
									        var->name, var->name,
										      ff_lookup_string(variable_types, FFV_TYPE(var)),
										      nt->data->buffer + var->start_pos - 1);
									err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
								}
							}
							else if (IS_SHORT(var))
							{
								if (btype_to_btype(cp, FFV_TYPE(var), &s, FFV_SHORT))
								{
									at_least_one_error = TRUE;
									err_push(ROUTINE_NAME, ERR_PARAM_VALUE, var->name);
								}
								else
								{
									if (s < 1 || s > NUM_Grid_Origin)
									{
										at_least_one_error = TRUE;
										sprintf(scratch, "%s must equal 1, 2, ..., or %d\n==> %s %s %dh <==",
										        var->name, NUM_Grid_Origin, var->name,
										        ff_lookup_string(variable_types, FFV_TYPE(var)), s);
										err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
									}
								}
							}
							else
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must be type char or short", var->name);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "header_type") == 0)
						{
							if (bsearch((char *)&cp, (char *)header_type,
							            NUM_Header_Type, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal header_embedded[_varied] or header_separated[_varied]\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "image_format") == 0)
						{
							if (bsearch((char *)&cp, (char *)image_format,
							            NUM_Image_Format, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal BSQ, BIL, BIP, or BIP2\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}	
						else if (strcmp(var->name, "keep_preview") == 0)
						{
							if (strcmpi(cp, "yes") != 0 && strcmpi(cp, "no") != 0)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal yes or no\n==> %s %s %s <==",
								        var->name, var->name,
									      ff_lookup_string(variable_types, FFV_TYPE(var)),
									      nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "map_projection") == 0)
						{
							if (IS_CHAR(var))
							{
								cp = strncpy(scratch, nt->data->buffer + var->start_pos - 1,
								             sizeof(scratch) - 1);
								scratch[sizeof(scratch) - 1] = STR_END;
								os_str_replace_unescaped_char1_with_char2('%', ' ', scratch);
									
								if (bsearch((char *)&cp, (char *)map_projection,
								            NUM_Map_Projection, sizeof(char *),
								            (int(*)(void const *, void const *))comparei) == NULL)
								{
									at_least_one_error = TRUE;
									sprintf(scratch, "%s has an invalid value (consult GeoVu documentation)\n==> %s %s %s <==",
									        var->name, var->name,
									        ff_lookup_string(variable_types, FFV_TYPE(var)),
									        nt->data->buffer + var->start_pos - 1);
									err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
								}
							}
							else if (IS_SHORT(var))
							{
								if (btype_to_btype(cp, FFV_TYPE(var), &s, FFV_SHORT))
								{
									at_least_one_error = TRUE;
									err_push(ROUTINE_NAME, ERR_PARAM_VALUE, var->name);
								}
								else
								{
									if (s < 0 || s > NUM_Map_Projection)
									{
										at_least_one_error = TRUE;
										sprintf(scratch, "%s must equal 0, 1, ..., or %d\n==> %s %s %dh <==",
										        var->name, NUM_Map_Projection, var->name,
										        ff_lookup_string(variable_types, FFV_TYPE(var)), s);
										err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
									}
								}
							}
							else
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must be type char or short", var->name);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "number_of_display_colors") == 0)
						{
							if (btype_to_btype(cp, FFV_TYPE(var), &s, FFV_SHORT))
							{
								at_least_one_error = TRUE;
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, var->name);
							}
							else
							{
								if (s < 16 || s > 256)
								{
									at_least_one_error = TRUE;
									sprintf(scratch, "%s must be in the range [16..256]\n==> %s %s %dh <==",
									        var->name, var->name,
									        ff_lookup_string(variable_types, FFV_TYPE(var)), s);
									err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
								}
							}
						}
						else if (strcmp(var->name, "omit_grid_embedded_header") == 0)
						{
							if (strcmp(cp, "Yes") != 0 && strcmp(cp, "No") != 0)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal Yes or No\n==> %s %s %s <==",
								        var->name, var->name,
								        ff_lookup_string(variable_types, FFV_TYPE(var)),
								        nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
	/*
						else if (!strcmpi(var->name, "plot_drawing_default"))
						{
							if (strcmp(cp, "connected_lines"))
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal connected_lines\n==> %s %s %s <==",
								        var->name, var->name,
								        ff_lookup_string(variable_types, FFV_TYPE(var)),
								        nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
	
	*/
						else if (strcmp(var->name, "preview") == 0)
						{ /* actual values are "same", "no_preview" or file extension */
							if (strcmp(cp, "Yes") != 0 && (strcmp(cp, "No") != 0))
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal Yes or No\n==> %s %s %s <==",
								        var->name, var->name,
								        ff_lookup_string(variable_types, FFV_TYPE(var)),
								        nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "scaling") == 0)
						{
							if (bsearch((char *)&cp, (char *)scaling,
							            NUM_Scaling, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s has an invalid value (consult GeoVu documentation)\n==> %s %s %s <==",
								        var->name, var->name,
								        ff_lookup_string(variable_types, FFV_TYPE(var)),
								        nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "standard_format_name") == 0)
						{
							if (bsearch((char *)&cp, (char *)standard_format_name,
							            NUM_Standard_Format_Name, sizeof(char *),
							            (int(*)(void const *, void const *))comparei) == NULL)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s has an invalid value (consult GeoVu documentation)\n==> %s %s %s <==",
								        var->name, var->name,
								        ff_lookup_string(variable_types, FFV_TYPE(var)),
								        nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "user_select_data_type") == 0)
						{
							if (strcmp(cp, "Yes") != 0 && strcmp(cp, "No") != 0)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal Yes or No\n==> %s %s %s <==",
								        var->name, var->name,
								        ff_lookup_string(variable_types, FFV_TYPE(var)),
								        nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else if (strcmp(var->name, "user_define_row_structure") == 0)
						{ /* actual value is simply to have a definition */
							if (strcmp(cp, "Yes") != 0 && strcmp(cp, "No") != 0)
							{
								at_least_one_error = TRUE;
								sprintf(scratch, "%s must equal Yes or No\n==> %s %s %s <==",
								        var->name, var->name,
								        ff_lookup_string(variable_types, FFV_TYPE(var)),
								        nt->data->buffer + var->start_pos - 1);
								err_push(ROUTINE_NAME, ERR_PARAM_VALUE, scratch);
							}
						}
						else
						{
							at_least_one_error = TRUE;
							sprintf(scratch, "Internal Error -- overlooked enumerated keyword");
							err_push(ROUTINE_NAME, ERR_UNKNOWN, scratch);
						}
					} /* if enumerated keyword */
				} /* (else) if not a char, but is supposed to be */
			} /* end if() name constant variable (vs. name equivalence) */
		} /* end of table lookups */
		
		var_list = dll_next(var_list);
	} /* while() on variables in name table */
	
	return(at_least_one_not_found || at_least_one_error);
}

int compare(const char **a, const char **b)
{
	return(strcmp(*a, *b));
}


int comparei(const char **a, const char **b)
{
	return(strcmpi(*a, *b));
}

