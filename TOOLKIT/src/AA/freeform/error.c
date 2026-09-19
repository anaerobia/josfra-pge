/* 
 *      FILENAME: error.c
 *
 *      CONTAINS: 
 *     err_push() 
 *     err_state() 
 *     err_clear() 
 *     err_disp() 
 *     (MODULE) err_get_msg() 
 *     (MODULE) error_bin_search() 
 *     (MODULE) cb_error 
 */

#ifdef XVT
#include <xvt.h>
static long cb_error(WINDOW, EVENT *);
#define ERROR 1000
#endif

#include <freeform.h>
#include <os_utils.h>

#define NUM_ERROR_ENTRIES (sizeof(local_errlist) / sizeof(ERROR_RECORD))
#define MAX_STACK       10

/* Local variables */

typedef struct  {

	int     error_number;
	pPSTR   error_string;

} ERROR_RECORD;


static int              stack_index = 0;
static ERR_BOOLEAN      err_stack_space = FALSE;
								
static int      error_stack[MAX_STACK + 1];
static pPSTR    str_stack[MAX_STACK + 1];
#ifdef ERR_ROUTINE_NAME
static pPSTR     routine_stack[MAX_STACK + 1];
#endif

/* Local prototypes */
static pPSTR err_get_msg(int);
static pPSTR error_bin_search(int);

ERROR_RECORD local_errlist[] = 
{
	/* NOTE: THESE ERRORS MAY BE CROSS-LISTED
		-- YOU MAY HAVE TO CHECK MORE THAN ONE SECTION TO FIND
		   THE APPROPRIATE ERROR DESCRIPTION */

	/* General Errors */
	ERR_GENERAL,                    "Unable to complete requested command",                 /*500*/
	ERR_OPEN_FILE,                  "Opening file",                                 /*501*/
	ERR_READ_FILE,                  "Reading file",                                 /*502*/
	ERR_WRITE_FILE,                 "Writing to file",                              /*503*/
	ERR_PTR_DEF,                    "Required internal data structure undefined",                                  /*504*/
	ERR_MEM_LACK,                   "Insufficient memory (RAM)",                 /* 505 */
	ERR_UNKNOWN,                    "Undefined error",                                      /*506*/
	ERR_FIND_FILE,                  "Finding file",                                      /*507*/
	ERR_FILE_DEFINED,               "File undefined",                                     /*508*/
	ERR_CLOSE_FILE,                 "Closing file",                                 /*509*/
	ERR_OUT_OF_RANGE,               "Computed value went out of range",                     /*510*/
	ERR_MEM_CORRUPT,                "Possible memory corruption",                           /*511*/
	ERR_STRUCT_FIELD,               "Struct->field not defined",                            /*512*/
	ERR_MAX_LT_MIN,                 "Maximum value < Minimum value",                        /*513*/
	ERR_MAKE_HISTOGRAM,             "Making histogram",                             /*514*/
	ERR_PROCESS_DATA,               "Processing data",                              /*515*/
	ERR_DATA_GT_BUFFER,             "Too much data for buffer",                             /*516*/
	ERR_WRITE_DATA,                 "Writing data",                                 /*517*/
	ERR_FIND_INTERSECT,             "Error finding intersection",                           /*518*/
	ERR_NUM_TOKENS,                 "Incorrect Number of Tokens on Line",                    /* 519 */
	ERR_COMMAND_LINE,               "Parsing command line",                         /* 520 */
	ERR_STRING_TOO_LONG,            "String is too long for destination",                   /* 521 */
	ERR_FILE_EXISTS,                "File already exists (Can you delete or move the file?)",                                  /* 522 */
	ERR_FILE_NOTEXIST,              "File does not exist (Did you type the name correctly?)",                          /* 523 */
	ERR_CREATE_FILE,                "Creating file (Do you have write access to the file and directory?)",    /* 524 */

	/* Freeform Errors */
	ERR_UNKNOWN_VAR_TYPE,           "Unknown variable type",                                /*1000*/
	ERR_UNKNOWN_FORM_TYPE,          "Unknown format type",                                  /*1001*/
	ERR_HEAD_FORM,                  "Problem making header format",                         /*1002*/
	ERR_CONVERT,                    "Problem in conversion",                                /*1003*/
	ERR_MAKE_FORM,                  "Making format",                                /*1004*/
	ERR_FREE_FORM,                  "Destroying format",                          /*1005*/
	ERR_SHOW_FORM,                  "Showing format",                               /*1006*/
	ERR_GET_DELIMITER,              "Getting delimiter",                            /*1007*/
	ERR_PROC_LIST,                  "Processing variable list",                     /*1008*/
	ERR_GET_VALUE,                  "Getting value",                                /*1009*/
	ERR_UNEXPECTED_CR,              "Record Length or CR Problem",                          /*1010*/
	ERR_BINARY_OVERFLOW,            "ASCII->Binary Overflow",                               /*1011*/
	ERR_VARIABLE_NOT_FOUND,         "Variable Not Found",                                   /*1012*/
	ERR_SHOW_VARS,                  "Showing variables",                            /*1013*/
	ERR_VAR_REPLACE,                "No replacement of variable",                           /*1014*/
	ERR_HEAD_LENGTH,                "Calculating header length",                    /*1015*/
	ERR_MAKE_INDEX,                 "Making index",                                 /*1017*/
	ERR_SKIP_HEADER,                "Skipping header",                              /*1018*/
	ERR_VAR_POSITIONS,              "Incorrect positions for variable",                     /*1019*/
	ERR_NO_VARS_SHARED,             "No variables shared by input/output formats",          /*1020*/
	ERR_FIND_FORM,                  "Finding default format",                               /*1021*/
	ERR_CONVERT_VAR,                "Could not determine type of conversion for the given variable",           /*1022*/

	/* Binview Errors */

	ERR_DBIN_EVENT,                 "Performing dbin event",                        /*1500 */
	ERR_BIN_NOT_DEFINED,            "Data bin not defined",                                 /*1501*/ 
	ERR_MAX_BIN_REACHED,            "Maximum number of dbins reached",                      /*1502*/        
	ERR_CACHE_DEFINED,              "Data cache not defined",                               /*1503*/
	ERR_BINFORM_DEFINED,            "Dbin format not defined",                              /*1504*/
	ERR_HEAD_DEFINED,               "Header not defined",                                   /*1505*/
	ERR_UNKNOWN_SECTION,            "Unknown menu section",                                 /*1506*/
	ERR_UNKNOWN_OBJECT,             "Unknown object type",                                  /*1507*/
	ERR_MAKE_DBIN,                  "Making data bin",                              /*1508*/
	ERR_SET_DBIN,                   "Setting data bin",                             /*1509*/
	ERR_SHOW_DBIN,                  "Showing data bin",                             /*1510*/
	ERR_SET_VIEW,                   "Setting view",                                 /*1511*/
	ERR_MAKE_VIEW,                  "Making view",                                  /*1512*/
	ERR_MAX_VIEW_REACHED,           "Maximum number of views reached",                      /*1513*/
	ERR_UNKNOWN_DATA_TYPE,          "Unknown data type",                                    /*1514*/
	ERR_SHOW_VIEW,                  "Showing view",                                 /*1515*/
	ERR_FORMAT_LIST_DEF,            "Undefined Format List",                                /*1516*/
	ERR_FILE_LENGTH,                "File length / Record Length Mismatch",                 /*1517*/
	ERR_DATA_ROUNDUP,               "Data roundup error",                                   /*1518*/
	ERR_OVERFLOW_UNCHANGE,          "Data overflow, no change made",                        /*1519*/
	ERR_NT_DEFINE,                  "Defining name table",                          /*1520*/
	ERR_DVIEW_EVENT,                "Performing dview event",                       /*1521*/
	ERR_MAKE_PLIST,                 "Making parameter list",                        /*1522*/
	ERR_VIEW_UNDEFINED,				      "View has not been defined",                    		/*1523*/
	ERR_UNKNOWN_MESSAGE,			      "Unknown message sent -- Contact Support",                                 /*1524*/
	ERR_UNKNOWN_FORMAT_TYPE,		    "Unknown Format Type",									/*1525*/
	ERR_NAME_TABLE,					        "Error in Name Table Operation",						/*1526*/
	ERR_EVENT_RETIRED,				      "Event has been retired -- Contact Support",								/*1527*/
	
	/* XVT errors */

	ERR_OPEN_DIALOG,                "Opening a dialog",                             /*2000*/
	ERR_OPEN_SLIST,                 "Opening a SLIST",                              /*2001*/
	ERR_ADD_SLIST,                  "Adding an item to a SLIST",                    /*2002*/

	/* String database and menu systems */

	ERR_MAKE_MENU_DBASE,            "Creating menu database",                       /*3000*/
	ERR_NO_SUCH_SECTION,            "No Section with this title",                           /*3001*/
	ERR_GETTING_SECTION,            "Getting text for section",                     /*3002*/
	ERR_MENU,                       "Processing Menu",                              /*3003*/

	ERR_MN_BUFFER_TRUNCATED,        "Menu section buffer truncated",                        /*3500*/
	ERR_MN_SEC_NFOUND,              "Requested menu section not found",                       /*3501*/
	ERR_MN_FILE_CORRUPT,            "Menu file corrupt",                                    /*3502*/
	ERR_MN_REF_FILE_NFOUND,         "Referenced file not found",                            /*3503*/

	/* Interpreter and parse errors */

	ERR_SYNTAX,                     "Syntax error",                                         /* 4000 */
	ERR_MISSING_TOKEN,              "Expected token(s) missing",                               /* 4001 */
	ERR_MISSING_VAR,                "Could not find variable",                              /* 4002 */
	ERR_PARAM_OVERFLOW,             "Too many parameters",                                  /* 4005 */
	ERR_PARAM_VALUE,                "Invalid parameter value",                              /* 4006 */
	ERR_WARNING1,                   "WARNING: ",                                            /* 4010 */
	ERR_BAD_NUMBER_ARGV,            "Error reading number from command line",               /* 4011 */
	ERR_STR_TO_NUMBER,              "Error converting number from string",                  /* 4012 */
	ERR_UNKNOWN_OPTION,             "Unknown option; for usage, run again without any arguments", /* 4013 */
	ERR_IGNORED_OPTION,             "This option not used with this application",             /* 4014 */
	ERR_VARIABLE_DESC,              "Bad syntax for variable description line",               /* 4015 */

	/* HDF errors */
	
	ERR_MAKE_HDF,                   "Making HDF file",                              /* 5000 */
	ERR_DIMENSION,                  "Improper dimension",                                   /* 5001 */
	ERR_MAKE_VSET,                  "Making HDF Vset",                              /* 5002 */
	ERR_SET_SDS_RANGE,              "Setting SDS range",                            /* 5003 */
	ERR_WRITE_HDF,                  "Writing to HDF file",                          /* 5004 */
	ERR_READ_HDF,                   "Reading HDF file",                             /* 5005 */
	ERR_GET_HDF_FMT,                "Getting HDF format",                           /* 5006 */
	ERR_GET_HDF_OBJECT,             "Getting HDF object",                           /* 5007 */

	/* ADTLIB errors */

	ERR_MAKE_MAX_MIN,               "Making max_min struct",                        /* 6000 */
	ERR_FIND_MAX_MIN,               "Finding max and min",                          /* 6001 */
	ERR_PARSE_EQN,                  "Parsing equation",                             /* 6002 */
	ERR_EE_VAR_NFOUND,              "Variable in equation not found",                       /* 6003 */
	ERR_CHAR_IN_EE,                 "Character data type in equation",                      /* 6004 */
	ERR_EE_DATA_TYPE,               "Mismatching data types in equation",                   /* 6005 */
	ERR_NDARRAY,                    "With N-dimensional array",                     /* 6006 */
	ERR_GEN_QUERY,                  "Processing query",                             /* 6007 */
	
	/* NAME_TABLE errors */

	ERR_EXPECTING_SECTION_START, "Expecting Section Start:",
	ERR_EXPECTING_SECTION_END,   "Expecting Section End:",
	ERR_MISPLACED_SECTION_START, "Badly Placed Section Start:",
	ERR_MISPLACED_SECTION_END,   "Badly Placed Section End:",
	ERR_NT_MERGE,                "Error in merging/copying name tables",
	
	/* Programmer's eyes only, or programmer support */
	ERR_MAX_STACK_EXCEEDED,      "Error messaging stack has been exceeded",              /* 7900 */
	ERR_SWITCH_DEFAULT,          "Unexpected default case in switch statement, Contact Support", /* 7901 */
	ERR_DEBUG_MSG,               "Preceding error messages alpha-test ONLY, execution continues...",           /* 8000 */
};

/*
 * NAME:    err_push 
 *
 * PURPOSE: To push an error message and a possible further
 *          error description onto stacks which may later 
 *          be displayed
 *
 * USAGE:   void err_push(PSTR routine_name,
 *                        int error_defined_constant,
 *                        PSTR further_error_description)
 *          further_error_description may be NULL 
 *
 * RETURNS: void
 *              
 *
 * DESCRIPTION: Three simultaneous array-based stacks are used 
 *              to describe an error. Memory is allocated for each
 *              routine name stack element (routine_stack) and for 
 *              each further error description stack element (str_stack).
 *              If the error stacks are full, a stack full message 
 *              is displayed and control is transferred back to the
 *              user in which he\she can display stack errors if
 *              desired. If a memory allocation error occurs, control
 *              is also given to the user with an appropriate explanation.
 *
 * ERR_DEBUG_MSG has a special usage.  It is both an error code and message
 * in its own right, but if added to another error code and sent into err_push()
 * it adds the meaning that this message should be viewed only in a "debugging
 * version", and never in a "release version".  In summary, for programmers'
 * and/or testers' eyes only.  To enable this feature, compile all source code
 * files with the preprocessor macro DEBUG_MSG defined.
 *
 * AUTHOR:  Mark Van Gorp, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS: 
 *  Although no dependent function exists, the following error creating
 *  process must be followed before new errors can be pushed:
 *
 * ALL NEW ERROR ENTRIES MUST START AT INTEGERS > 500. FOR EXAMPLE, A NEW                   
 * BLOCK OF ERROR MESSAGES MAY START AT 1000; THE NEXT BLOCK AT 1500; ETC.      
 * REASON--- ERROR MESSAGES < 500 MAY INTERFERE WITH SYSTEM ERRORS.                                 
 * HOW TO ADD ERROR MESSAGES:                                                                                                       
 *                                                                                                                                                               
 * 1. #define a numeric error descriptive constant (Greater than 500) in err.h                                                                                                                              
 *                                                                                                                                                               
 * 2. Add your defined constant, along with an error message describing it in
 *    local_errlist.                                                               
 *
 * 3. Adding error messages have ONE STIPULATION: They must be
 *    inserted into local_errlist in increasing numeric order of
 *    the constants. (ie 500, 505, 603, ...,n; NOT 500, 603, 505,...n)
 *    This will enable the binary search to work effectively and find
 *    the appropriate error message.
 *
 * To distinguish between an error seen in a debug version versus a release
 * versions the following option is provided:
 * 
 *    Within the err_push() call, add ERR_DEBUG_MSG to the error code, e.g.,
 *    err_push(ROUTINE_NAME, ERR_MY_ERROR + ERR_DEBUG_MSG, some_explanation_str);
 *    This means that in this particular call to err_push(), the debug version
 *    will display ERR_MY_ERROR, but in the release version it will not be
 *    displayed.
 *
 * COMMENTS:
 *
 * KEYWORDS:            error system 
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "err_push"

#ifdef ERR_ROUTINE_NAME
int err_push_unwrapped(pPSTR p_routine, int ercode, pPSTR p_str)
#else
int err_push_unwrapped(int ercode, pPSTR p_str)
#endif
{
#ifdef ERR_ROUTINE_NAME
	assert(p_routine);
#endif
	assert(ercode);

#ifdef DEBUG_MSG
	if (ercode > ERR_DEBUG_MSG)
		ercode -= ERR_DEBUG_MSG;
#else
	if (ercode > ERR_DEBUG_MSG)
		return(ercode - ERR_DEBUG_MSG);
#endif
	
	if (stack_index < MAX_STACK)
	{
		error_stack[stack_index] = ercode;

		/* If sufficient space, assign routine name to the stack; else an
		   error exits in stack allocation space which is assigned TRUE */
				
#ifdef ERR_ROUTINE_NAME
		if (p_routine != NULL &&
		    (routine_stack[stack_index] = (char *)memStrdup(p_routine,"p_routine")) == NULL) 
			err_stack_space = TRUE;
#endif

		if (p_str != NULL && !err_stack_space &&
		    (str_stack[stack_index] = (char *)memStrdup(p_str,"p_str")) == NULL)  
			err_stack_space = TRUE;

		/* If there was an error in stack allocation, then display appropriate
		   error messages and exit */

		if (err_stack_space)
		{
#ifdef XVT
			xvt_dm_post_error("Internal error: insufficient memory for error messaging system.");
#ifdef ERR_ROUTINE_NAME
			xvt_dm_post_error("Present error: %s, %s, %s", p_routine, err_get_msg(ercode), p_str);
#else
			xvt_dm_post_error("Present error: %s, %s", err_get_msg(ercode), p_str);
#endif /* ERR_ROUTINE_NAME */

			if (stack_index)
				xvt_dm_post_note("Remaining error messages will now be displayed");

#else /* XVT */
			fprintf(stderr,"Internal error: insufficient memory for error messaging system.\n");
#ifdef ERR_ROUTINE_NAME
			fprintf(stderr,"Present error: %s, %s, %s\n", p_routine, err_get_msg(ercode), p_str);
#else
			fprintf(stderr,"Present error: %s, %s\n", err_get_msg(ercode), p_str);
#endif /* ERR_ROUTINE_NAME */
		
			fprintf(stderr,"Remaining error messages will now be displayed\n\n");
#endif /* XVT */
			err_disp();
		}       
			
		stack_index++;
	}
	if (stack_index == MAX_STACK)
	{
		error_stack[stack_index] = ERR_MAX_STACK_EXCEEDED;
				
#ifdef ERR_ROUTINE_NAME
		if (p_routine != NULL &&
		    (routine_stack[stack_index] = (char *)memStrdup(ROUTINE_NAME,"ROUTINE_NAME")) == NULL) 
			err_stack_space = TRUE;
#endif

		if (p_str != NULL && !err_stack_space &&
		    (str_stack[stack_index] = (char *)memStrdup("All subsequent errors have been ignored",NO_TAG)) == NULL)  
				err_stack_space = TRUE;

		stack_index++;
	}
	return(ercode);
}

/*
 * NAME:    err_state 
 *
 * PURPOSE: To test if any error mesages have been pushed onto
 *          the error stack
 *
 * USAGE:   ERR_BOOLEAN err_state()
 *
 * RETURNS: TRUE (1) if errors exist; otherwise FALSE (0)
 *
 * DESCRIPTION: Err_state() is a boolean function which evaluates to 
 *              TRUE if errors exist on the stack. Stack_index keeps
 *              track of the stack head and is zero if no errors
 *              have been pushed.                                                               
 *
 * AUTHOR:  Mark Van Gorp, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  None
 *
 * COMMENTS:
 *
 * KEYWORDS:            error system 
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "err_state"

ERR_BOOLEAN  err_state(void)
{
	return(stack_index > 0 ? TRUE : FALSE);
}

/*
 * NAME:    err_clear 
 *
 * PURPOSE: To clear the error message stacks
 *
 * USAGE:   void err_clear()
 *
 * RETURNS: void
 *
 * DESCRIPTION: Clears the string-based stacks (routine_stack and str_stack)
 *              and deallocates the memory. Clear_stack_index denotes the 
 *              number of array elements used on the stack. Err_clear() is
 *              generally called only by err_disp().
 *
 * AUTHOR:  Mark Van Gorp, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  None
 *
 * COMMENTS:
 *
 * KEYWORDS:            error system 
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "err_clear"

void err_clear(void)
{

	int i;

	for (i = 0; i < stack_index; i++)
	{
#ifdef ERR_ROUTINE_NAME
		memFree(routine_stack[i],"routine_stack[i]");
		routine_stack[i] = NULL;
#endif
		if (str_stack[i])
		{
			memFree(str_stack[i],"str_stack[i]");
			str_stack[i] = NULL;
		}
	}
	stack_index = 0;
}

/*
 * NAME:    err_disp 
 *
 * PURPOSE: To display the pushed stack errors
 *
 * USAGE:   void err_disp()
 *
 * RETURNS: void 
 *
 * DESCRIPTION: Err_disp() displays the top error message from the error
 *              stack and queries the user if he\she would like to see more
 *              error messages (if the stack is not empty). If the user
 *              wishes to see more errors, a 'y' or 'Y' must be entered and
 *              the loop is repeated. When the stack is empty or no more 
 *              error messages are needed, err_clear() is called to clear
 *              the stacks. 
 *
 * AUTHOR:  Mark Van Gorp, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  None
 *
 * COMMENTS:
 *
 * KEYWORDS:            error system 
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "err_disp"

void  err_disp(void)
{

#ifndef XVT
	char reply[4]; /* even for alignment/anal purposes */
	int i;
#else
	WINDOW dlg_win = NULL_WIN;
#endif

	/* Only display errors if desired */
	if ( !(os_get_env("NO_ERROR_DISPLAY")) )
	{
#ifdef XVT                                      /* for error display using a dialog */
		if (stack_index > 0)
		{
			dlg_win = xvt_dlg_create_res(WD_MODAL, ERROR, EM_ALL, cb_error, 0L);
			if (dlg_win == NULL_WIN)
				xvt_dm_post_error("Cannot put up error display dialog");
			stack_index = 0;
		}
#else
	
		fprintf(stderr, "\nThere %s %d error message%s!",
		        stack_index == 1 ? "is" : "are", 
		        stack_index,
		        stack_index > 1 ? "s" : "");

		for (i = 0; i < stack_index; i++)
		{
#ifdef ERR_ROUTINE_NAME
			fprintf(stderr, "\nERROR: %d", (i + 1));
#endif
			fprintf(stderr, "\nPROBLEM: %s", err_get_msg(error_stack[i]));
#ifdef ERR_ROUTINE_NAME
			fprintf(stderr, " -- ROUTINE: %s", routine_stack[i]);
#endif
			
			fprintf(stderr, "\nEXPLANATION: %s", str_stack[i] ? str_stack[i] : "none");
	
			if (i < stack_index - 1 && isatty(fileno(stdin)))
			{
				fflush(stdin);
				fprintf(stderr, "\nView next error message? (Y/N) : Y\b");
				fgets(reply, 2, stdin);
				if (toupper(reply[0]) != 'Y' && reply[0] != '\n')
					break;
			}
		}
		if (i == stack_index && isatty(fileno(stdin)))
		{
			fflush(stdin);
			/* The SUN needs a return before a string can be gotten from stdin */
			fprintf(stderr, "\nNo more error messages -- press <RETURN>");
			fgets(reply, 2, stdin);
		}

#endif
	}

	/* Clear error stack */

	err_clear();
}


/*
 * NAME:    cb_error 
 *
 * PURPOSE:     XVT (3) event handler routine for error dialog.
 *
 * USAGE:   long cb_error(WINDOW win, EVENT *ep)
 *
 * RETURNS:     long (XVT(3) EVENT_HANDLER 
 *
 * DESCRIPTION:         Uses XVT to present an error message.
 *
 * AUTHOR:  Liping Di, NGDC, (303)497-6284, lpd@kryton.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  None; XVT(3)
 *
 * COMMENTS:            
 *
 * KEYWORDS:            error system 
 *
 */
#ifdef XVT
#define ROUTINE 6
#define MSGS_LEFT 6
#define ERR     8
#define MESSAGE 10
/* see the file error.url */
#undef ROUTINE_NAME
#define ROUTINE_NAME "cb_error"

long cb_error(WINDOW win, EVENT *ep)
{
#ifndef ERR_ROUTINE_NAME
	char msgs_left_note[40];
#endif

	static int err_num = 0;

	switch(ep->type)
	{
		case E_CREATE:
#ifdef ERR_ROUTINE_NAME
			xvt_vobj_set_title(xvt_win_get_ctl(win, ROUTINE), routine_stack[err_num]);
#else
			sprintf(msgs_left_note, "Error message %d of %d",
			        err_num + 1, stack_index);
			xvt_vobj_set_title(xvt_win_get_ctl(win, MSGS_LEFT), msgs_left_note);
#endif
			xvt_vobj_set_title(xvt_win_get_ctl(win, ERR), err_get_msg(error_stack[err_num]));
			if (str_stack[err_num] != NULL)
				xvt_vobj_set_title(xvt_win_get_ctl(win, MESSAGE), str_stack[err_num]);
			else
				xvt_vobj_set_title(xvt_win_get_ctl(win, MESSAGE), "none");
			if (err_num == stack_index - 1)
				xvt_vobj_set_enabled(xvt_win_get_ctl(win, DLG_OK), FALSE);
			break;
		case E_CLOSE:
			xvt_vobj_destroy(win);
			err_num = 0;
			break;
		case E_DESTROY:
			err_num = 0;
			return((long)0);
			break;
		case E_CONTROL:
			switch(ep->v.ctl.id)
			{
				case DLG_OK:        /* ask for MORE */
					err_num++;
#ifdef ERR_ROUTINE_NAME
					xvt_vobj_set_title(xvt_win_get_ctl(win, ROUTINE), routine_stack[err_num]);
#else
					sprintf(msgs_left_note, "Error message %d of %d",
					        err_num + 1, stack_index);
					xvt_vobj_set_title(xvt_win_get_ctl(win, MSGS_LEFT), msgs_left_note);
#endif
					xvt_vobj_set_title(xvt_win_get_ctl(win, ERR), err_get_msg(error_stack[err_num]));
					if (str_stack[err_num] != NULL)
						xvt_vobj_set_title(xvt_win_get_ctl(win, MESSAGE), str_stack[err_num]);
					else
						xvt_vobj_set_title(xvt_win_get_ctl(win, MESSAGE), "none");
					if (err_num == stack_index - 1)
						xvt_vobj_set_enabled(xvt_win_get_ctl(win, DLG_OK), FALSE);
					break;
				case DLG_CANCEL:            /* CANCEL the dialog, quit messages */
					xvt_vobj_destroy(win);
					err_num = 0;
					break;
			}
	}
	return ((long)0);
}
#endif

/*
 * NAME:    err_get_msg 
 *
 * PURPOSE: To get an error message from the error stack
 *
 * USAGE:   static pPSTR err_get_msg(int error_defined_constant)
 *
 * RETURNS: A pointer to the error message string
 *
 * DESCRIPTION: Uses int msg (error_defined constant) to get the
 *              corresponding error message from the local_errlist
 *              array. If the error is not a system error, a binary
 *              search is called on the local error list. If no error
 *              is found, "Invalid error number" is returned. System
 *              errors are accounted for with a call to the operating
 *              system error list.
 *
 * AUTHOR:  Mark Van Gorp, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  None
 *
 * COMMENTS:
 *
 * KEYWORDS:            error system 
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "err_get_msg"
		
static pPSTR err_get_msg(int msg)
{

	pPSTR p_err_str = NULL;

#if     defined(CCLSC) || defined(SUNCC)
#define sys_nerr 400
#endif

	if (msg < sys_nerr) 
#if defined(CCLSC) || defined(SUNCC)
		p_err_str = strerror(msg);
#else
		p_err_str = sys_errlist[msg];
#endif
	else    
		p_err_str = error_bin_search(msg);

	/* Check if no match was found */
		
	if (!p_err_str)
		p_err_str = "Invalid error number";
		
	return(p_err_str);
}

/*
 * NAME:    error_bin_search 
 *
 * PURPOSE: To search the error array data structure and retrieve
 *          the appropriate error message
 *
 * USAGE:   pPSTR err_binary_search(int error_defined_constant)
 *
 * RETURNS: If successful: a pointer to the appropriate error message
 *          else NULL
 *
 * DESCRIPTION: A non-recursive binary search to find a match 
 *              between int msg (error_defined_constant) and an
 *              error number in local_errlist. If a match is found,
 *              the corresponding error string is returned, else NULL.
 *
 * AUTHOR:  Mark Van Gorp, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  None
 *
 * COMMENTS:
 *
 * KEYWORDS:            error system
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "error_bin_search"

pPSTR error_bin_search(int msg)
{
	int low, high, mid;

	low = 0;
	high = NUM_ERROR_ENTRIES - 1;

	while (low <= high)
	{
		mid = (low + high) / 2;

		if (msg < local_errlist[mid].error_number)
			high = mid - 1;
		else if (msg > local_errlist[mid].error_number)
			low = mid + 1;
		else  /* found match */
			return(local_errlist[mid].error_string);
	}

	/* No match found : Invalid error code */

	return(NULL);

}
			
#ifdef XVT /* leave err_assert() defined even if compiled with NDEBUG */

void err_assert(void *msg, void *file, unsigned line)
{
	switch (xvt_dm_post_ask("CONTINUE", "EXIT", NULL,
	                        "Assertion Failed: %s\nIn File %s, Line %u",
                          msg, file, line))
	{
		case RESP_DEFAULT:
			break;
				
		case RESP_2:
			xvt_dm_post_fatal_exit("Exiting GeoVu");
			break;
				
		default:
			xvt_dm_post_error("Invalid return from ask.");
			break;
	}
}

#endif

