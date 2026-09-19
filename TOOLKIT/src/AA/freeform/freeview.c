/* FILENAME:  freeview.c
 *
 * CONTAINS:  dv_free()
 *            free_geo_info()
 *
*/

/*
 * NAME:        dv_free
 *              
 * PURPOSE:     To free a view
 *
 * USAGE:       void dv_free(DATA_VIEW *view) 
 *
 * RETURNS:     none
 *
 * DESCRIPTION: dv_free frees a view structure and associated memory
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    view, memory.
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	7/17/95		-rf01
 *		avoid compile errors in THINK C with #undef ROUTINE_NAME
 *
*/

#include <stdlib.h>
#include <freeform.h>
#include <databin.h>
#include <geoinfo.h>

#include <dataview.h>

#undef ROUTINE_NAME		/* needed to avoid compile errors in THINK C -rf01 */
#define ROUTINE_NAME "dv_free"

#ifdef PROTO
	void dv_free(DATA_VIEW *view)
#else
	void dv_free(view)
	DATA_VIEW *view;
#endif
{       
	assert(view && ((void *)view == view->check_address));

	if(view->info != NULL){
		free_geo_info((GEO_INFO_PTR)view->info);
		view->info = (void *)NULL;              /* prevent release twice of view->info */
	}

	if(view->title != NULL) memFree((void *)view->title, "dv_free: view->title");

	if(view->row_sizes != NULL) memFree((void *)view->row_sizes, "dv_free: view->row_sizes");
	
	if(view->eqn_info) ee_free_einfo(view->eqn_info);
	
	/* free view */
	memFree((void *) view, "dv_free: view");
	return;
}

/*
 * NAME:		free_geo_info
 *		
 * PURPOSE:		To free memory of GEO_INFO structure.
 *
 * USAGE:		void free_geo_info(GEO_INFO_PTR info)
 *
 * RETURNS:		void
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	None
 *
 * GLOBAL: 		None	
 *
 * AUTHOR:		Liping Di, NGDC, 303-497-6284, lpd@kryton.ngdc.noaa.gov
 *
 * COMMENTS:	Often found with a "freeview" call.	
 *
 * KEYWORDS:	free memory, GEO_INFO structure
 *
 */
void free_geo_info(GEO_INFO_PTR info)

#undef ROUTINE_NAME
#define ROUTINE_NAME "free_geo_info"
{
  memFree((void *)info->ViewParam, "info->ViewParam");
	memFree(info->info, "info->info");
	memFree((void *)info, "info");
	return;
}
