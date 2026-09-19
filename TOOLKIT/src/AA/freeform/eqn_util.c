#include <stdio.h>
#include <string.h>
#include <freeform.h>
#include <databin.h>
#include <dataview.h>
#include <eval_eqn.h>

/*
 * NAME:        ee_check_vars_exist
 *
 * PURPOSE:     To determine if the variables mentioned in the equation in einfo exist
 *                              in eqn_format, which is just a pointer to whatever format it is 
 *                              intended that the ee_ functions use.
 *
 * USAGE: int ee_check_vars_exist(EQUATION_INFO_PTR einfo, FORMAT_PTR eqn_format)
 *
 * RETURNS:     0 if all is well,
 *                              1 on error (with an err_push on the stack)
 *
 * DESCRIPTION: The function checks to see that the variables mentioned in the 
 *                  equation_info structure exist in the passed eqn_format.
 *				It also sets variable_ptr to the appropriate VAR structure, and
 *				mallocs enough room for any string type variables.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    
 *
 * KEYWORDS: equation
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ee_check_vars_exist"
int ee_check_vars_exist(EQUATION_INFO_PTR einfo, FORMAT_PTR eqn_format)
{
	VARIABLE_LIST_PTR       vars    = NULL;
	VARIABLE_PTR            var		= NULL;
	unsigned char           i;
	char					variable_found;
	char					*ch_ptr	= NULL;
	
	assert(einfo && eqn_format);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}
	if (eqn_format != eqn_format->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "eqn_format structure corrupted");
		return(ERR_MEM_CORRUPT);
	}
	
	for(i = 0; i < einfo->num_vars; i++){
		vars = eqn_format->variables;
		vars = dll_first(vars);
	   
		variable_found = 0;
	   
		while ((var = (VARIABLE_PTR)vars->data_ptr) != NULL)
		{
			if(!memStrcmp(einfo->variable[i], var->name,NO_TAG)){ /* Match found */
				/* check to make sure it is of correct data type */
				if(IS_TEXT(var)){
					if(einfo->variable_type[i] != EE_VAR_TYPE_CHAR){
						err_push(ROUTINE_NAME, ERR_EE_DATA_TYPE, var->name);
						return(1);
					}
					/* We have a character type variable */
					if((char *)((long)einfo->eqn_vars[i])){ /* This has been allocated before */
						memFree((char *)((long)einfo->eqn_vars[i]), "ee_check_vars_exist: einfo->eqn_vars[i]");
					}
					ch_ptr = (char *)memMalloc((size_t)(var->end_pos - var->start_pos + 5), "ee_check_vars_exist: ch_ptr");
					if(!ch_ptr)
					{
						err_push(ROUTINE_NAME, ERR_MEM_LACK, "Allocating the character type variable");
						return(1);						
					}
					einfo->eqn_vars[i] = (double)((long)ch_ptr);
				}
				else if(einfo->variable_type[i] == EE_VAR_TYPE_CHAR){
					err_push(ROUTINE_NAME, ERR_EE_DATA_TYPE, var->name);
				}
				einfo->variable_ptr[i] = (void *)var;
				variable_found = 1;
				break;
			}
			vars = dll_next(vars);
		}
		if(!variable_found){
		    err_push(ROUTINE_NAME, ERR_EE_VAR_NFOUND, einfo->variable[i]);
		return(1); /* return an error code */
		}
	}
	return(0);
}

/*
 * NAME:        ee_set_var_values
 *
 * PURPOSE:     To fill the EQUATION_INFO structure with variable values
 *
 * USAGE: int ee_set_var_values(EQUATION_INFO_PTR einfo, void *record, FORMAT_PTR eqn_format, FF_SCRATCH_BUFFER i_am_never_used)
 *
 * RETURNS:     0 if all is well,
 *                              1 on error (with an err_push on the stack)
 *
 * DESCRIPTION: The function fills the EQUATION_INFO structure with variable values in
 *                              eqn_format stored at record.  You MUST call ee_check_vars_exist BEFORE
 *                              calling this function, as this function does NOT check to see if the 
 *                              variables exist.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    
 *
 * KEYWORDS: equation
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ee_set_var_values"        
int ee_set_var_values(EQUATION_INFO_PTR einfo, void *record, FORMAT_PTR eqn_format, FF_SCRATCH_BUFFER i_am_never_used)
{
	unsigned char i;
	char *ch_ptr;
	VARIABLE_PTR var; 
	
	assert(einfo);
	assert(eqn_format);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}
	if (eqn_format != eqn_format->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "eqn_format structure corrupted");
		return(ERR_MEM_CORRUPT);
	}
	
	
	for(i = 0; i < einfo->num_vars; i++){
		var = (VARIABLE_PTR)(einfo->variable_ptr[i]);
		switch(einfo->variable_type[i]){
			case EE_VAR_TYPE_NUMERIC:
				if(ff_get_double(var, (FF_DATA_PTR)((char *)record + var->start_pos - 1),
						&(einfo->eqn_vars[i]), eqn_format->type)){
					err_push(ROUTINE_NAME, ERR_GENERAL, "Filling equation variables");
					return(1);
				}
				break;
			case EE_VAR_TYPE_CHAR:
				ch_ptr = (char *)((long)einfo->eqn_vars[i]);
				memStrncpy(ch_ptr, (char *)((char *)record + var->start_pos - 1), (size_t)(var->end_pos - var->start_pos + 1),NO_TAG);
				ch_ptr[var->end_pos - var->start_pos + 1] = '\0';
				break;
			default:
				err_push(ROUTINE_NAME, ERR_EE_DATA_TYPE, "Unknown data type");
				return(1);
		}
	}
	return(0);
}

/*
 * NAME:        ee_set_var_types
 *
 * PURPOSE:     To fill the EQUATION_INFO structure with variable values
 *
 * USAGE: int ee_set_var_types(char *eqn, FORMAT_PTR eqn_format)
 *
 * RETURNS:     0 if all is well,
 *                              1 on error (with an err_push on the stack)
 *
 * DESCRIPTION: The function inserts a '$' before any variable name it finds which is
 *                              of type FFV_CHAR in format.  This is necessary to run the 
 *                              ee_clean_up_equation function.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    This function assumes that eqn has enough buffer space on the end
 *                              to handle as many inserted '$' as there need to be.
 *
 * KEYWORDS: equation
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ee_set_var_types"
int ee_set_var_types(char *eqn, FORMAT_PTR eqn_format)
{
	int i, j, k;
	char inside_string = 0;
	char var_name[256];
	VARIABLE_LIST_PTR       vars    = NULL;
	VARIABLE_PTR            var             = NULL;
	
	assert(eqn && eqn_format);
	if (eqn_format != eqn_format->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "eqn_format structure corrupted");
		return(ERR_MEM_CORRUPT);
	}
	
    for(i = 0; i < (signed int)strlen(eqn); i++){
	if(eqn[i] == '\"'){
		if(!inside_string) inside_string = 1;
		else{
			if(eqn[i + 1] == '\"'){
				i++;
				continue;
			}
			inside_string = 0;
		}
	}
	if((eqn[i] == '[') && !inside_string){
		/* variable name */
		k = 0;
		for(i++; i < (signed int)strlen(eqn); i++) if(eqn[i] != ' ') break;
			for(j = i; j < (signed int)strlen(eqn); j++){
				if(eqn[j] == ']') break;
				var_name[k++] = eqn[j];
			}
			for(; k > 0; k--) if(var_name[k - 1] != ' ') break;
			var_name[k] = '\0';
			vars = eqn_format->variables;
			vars = dll_first(vars);
	   
			while ((var = (VARIABLE_PTR)vars->data_ptr) != NULL)
			{
				if(!memStrcmp(var_name, var->name,NO_TAG)){ /* Match found */
					if(IS_TEXT(var)){
						for(j = strlen(eqn); j >= i; j--) eqn[j + 1] = eqn[j];
						eqn[i] = '$';
					}
					break;
				}
				vars = dll_next(vars);
			}
		}
	}
	return(0);
}


/*
 * NAME:        ee_make_std_equation
 *
 * PURPOSE:     To set up and verify an EQUATION_INFO structure
 *
 * USAGE: EQUATION_INFO_PTR ee_make_std_equation(char *equation, FORMAT_PTR eqn_format)
 *
 * RETURNS:     NULL on error, or a pointer to the EQUATION_INFO struct
 *
 * DESCRIPTION: Makes a copy of the equation, 
 *				sets variable types (inserts a '$' in front of char type variables, 
 *										according to eqn_format),
 *				calls ee_clean_up_equation to create the EQUATION_INFO structure,
 *				and calls ee_check_vars_exist to confirm that all variables are
 *				accounted for.
 *				
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    
 *
 * KEYWORDS: equation
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ee_make_std_equation"
EQUATION_INFO_PTR ee_make_std_equation(char *equation, FORMAT_PTR eqn_format)
{
    char *scratch;
    EQUATION_INFO_PTR einfo = NULL;
    int error = 0;
    
	assert(equation && eqn_format);
	if (eqn_format != eqn_format->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "eqn_format structure corrupted");
		return(NULL);
	}
	
	scratch = (char *)memMalloc((size_t)(strlen(equation) + 200), "ee_make_std_equation: scratch");
	if(!scratch)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Creating a copy of the query restriction");
		return(NULL);
	}
			
	memStrcpy(scratch, equation,NO_TAG);
	if(ee_set_var_types(scratch, eqn_format)){
		err_push(ROUTINE_NAME, ERR_GENERAL, "Preprocessing equation");
		memFree(scratch, "ee_make_std_equation: scratch");
		return(NULL);
	}
			
	/* call ee_clean_up_equation to generate the EQUATION_INFO struct */
	einfo = ee_clean_up_equation(scratch, &error);
	if(!einfo)
	{
		ee_show_err_mesg(scratch, error); /* retrieve the exact error */
		err_push(ROUTINE_NAME, ERR_PARSE_EQN, scratch);
		memFree(scratch, "ee_make_std_equation: scratch");
		return(NULL);
	}

	/* Check to make sure that all the variables ee_clean_up_equation found
	 * are indeed in the format */
	if(ee_check_vars_exist(einfo, eqn_format)){
		err_push(ROUTINE_NAME, ERR_EE_VAR_NFOUND, "In ee_check_vars_exist");
		memFree(scratch, "ee_make_std_equation: scratch");
		return(NULL);
	}
	memFree(scratch, "ee_make_std_equation: scratch");
	return(einfo);
}

/*
 * NAME:        ee_free_einfo
 *
 * PURPOSE:     To free an EQUATION_INFO structure
 *
 * USAGE: int ee_free_einfo(EQUATION_INFO_PTR einfo)
 *
 * RETURNS:    0 on error, 1 if successful
 *
 * DESCRIPTION: Frees an EQUATION_INFO struct and all pointers underneath it.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none.
 *
 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
 *
 * COMMENTS:    
 *
 * KEYWORDS: equation
 *
 */
int ee_free_einfo(EQUATION_INFO_PTR einfo)
{
	int i;
	
	assert(einfo);
	if (einfo != einfo->check_address) {
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "einfo structure corrupted");
		return(ERR_MEM_CORRUPT);
	}

	memFree(einfo->equation, "ee_free_einfo: einfo->equation");
	
	/* Free string type variables */
	for(i = 0; i < (int)einfo->num_vars; i++){
		if(einfo->variable_type[i] == EE_VAR_TYPE_CHAR)
			if((char *)((long)einfo->eqn_vars[i]))
				memFree((void *)((long)einfo->eqn_vars[i]), "ee_free_einfo: einfo->eqn_vars[i]");
		memFree(einfo->variable[i], "ee_free_einfo: einfo->variable[i]");
	}
	
	/* Free string type constants */
	for(i = einfo->num_vars; i < (int)(einfo->num_vars + einfo->num_strc); i++)
		memFree((void *)((long)einfo->eqn_vars[i]), "ee_free_einfo: einfo->eqn_vars[i]");
	
	memFree(einfo->variable_type, "ee_free_einfo: einfo->variable_type");
	
	for(i = 0; i < (int)einfo->num_vars; i++)
	
	memFree(einfo->variable_ptr, "ee_free_einfo: einfo->variable_ptr");
	memFree(einfo->variable, "ee_free_einfo: einfo->variable");
	memFree(einfo->eqn_vars, "ee_free_einfo: einfo->eqn_vars");
	memFree(einfo, "ee_free_einfo: einfo");
	return(1);
}

