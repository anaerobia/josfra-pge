/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_SMF_ManageLogControlList.c

DESCRIPTION:
   This file contains the function PGS_SMF_ManageLogControlList().
   This function manages the list of SMF messages (or catagory of SMF messages)
   for which logging has been disabled.

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   10-Jan-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Manage List of SMF Messages For Which Logging Has Been Disabled

NAME:
   PGS_SMF_ManageLogControlList()

SYNOPSIS:
C:
   #include <PGS_SMF.h>

   PGSt_SMF_status
   PGS_SMF_ManageLogControlList(
       PGSt_SMF_status **list,
       size_t          *length,
       PGSt_SMF_status operation,
       PGSt_SMF_status list_element)

FORTRAN:		  
   N/A

DESCRIPTION:
   This function manages the list of SMF messages (or catagory of SMF messages)
   for which logging has been disabled.  It actually is used to add or delete
   an item from a sorted list (and keep the list sorted).  It is here intended
   to be called only from the function PGS_SMF_LoggingControl().  Although it
   may be useful elsewhere, if it is used for other purposes, the documentation
   herein ought to be modified to reflect a more generic usage.

INPUTS:
   Name           Description
   ----           -----------
   list           address of the pointer to the list

   length         length (i.e. number of elements) of the list

   operation      input command, determines what action this function should
                  take, possible values are:

                     PGSd_ADD_ELEMENT
		     Add a new element to the list.  The list is searched and
		     the new element is inserted in (numeric) order in the list
		     (i.e. the list is ordered).

		     PGSd_DELETE_ELEMENT
		     Remove an element from the list.  The list is searched and
		     if the input element (see "list_element" below) is found it
		     is removed from the list.  If it is not found, no action is
		     taken.

                     PGSd_FREE_LIST
		     free the memory area associated with the list, set the list
		     length to zero and make sure the pointer "list" has a value
		     of NULL

   list_element   list element to be added to or deleted from the list, this
                  variable is ignored if the operation is PGSd_FREE_LIST (see
		  "operation" above)

OUTPUTS:
   length         length (i.e. number of elements) of the list
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGS_E_TOOLKIT               could not perform requested operation

EXAMPLES:
C:

FORTRAN:
  
NOTES:
   None

REQUIREMENTS:
   PGSTK - ????, ????

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <PGS_SMF.h>

/* constants */

#define PGSd_DEFAULT_LIST_SIZE 100  /* default list length (no. of elements) */

PGSt_SMF_status
PGS_SMF_ManageLogControlList(
    PGSt_SMF_status **list,         /* (pointer to) pointer to list */
    size_t          *length,        /* number of elements in list */
    PGSt_SMF_status operation,      /* operation to perform on list */
    PGSt_SMF_status list_element)   /* list element to add/delete from list */
{
    PGSt_SMF_status *tmp_ptr;       /* pointer to space allocated by realloc */

    size_t          index;          /* list element index (used for looping) */
    size_t          tmp_length;     /* temporary variable */
    size_t          copy_size;      /* size of memory region to be copied */
    
    char*           copy_to;        /* index of memory region to be copied to */
    char*           copy_from;      /* index of mem. region to be copied from */
    
    PGSt_boolean    found_element;  /* true if list_element is found in list */
    
    /* if the input list pointer is NULL, initialize it to point to a list with
       intial number of elements = PGSd_DEFAULT_LIST_SIZE */

    if (*list == NULL)
    {
	*list = (PGSt_SMF_status *) malloc( PGSd_DEFAULT_LIST_SIZE*sizeof(PGSt_SMF_status) );
	if (*list == NULL)
	{
	    return PGS_E_TOOLKIT;
	}
    }

    /* perform the requested operation */

    switch (operation)
    {
      case PGSd_ADD_ELEMENT:

	/* Add a new element to the list.  The list is searched and the new
	   element is inserted in (numeric) order in the list (i.e. the list is
	   ordered).  Note that if the addition of the new element causes the
	   number of elements in the list to be larger than the default list
	   size, new memory must be allocated to add the element to the list. */

	/* Temporarily increase the list length by 1 (temporary in case a
	   request for dynamic memory fails and the new element cannot be added
	   to the list). */

	tmp_length = *length + 1;

	/* if the list is now larger than the default, allocate more memory */

	if (tmp_length > PGSd_DEFAULT_LIST_SIZE)
	{
	    tmp_ptr = (PGSt_SMF_status *) realloc(*list, tmp_length);
	    if (tmp_ptr != NULL)
	    {
		/* memory allocation successful, reasign the pointer "list" to
		   point to the new memory area */

	        *list = tmp_ptr;
	    }
	    else
	    {
		/* memory allocation failed, return an "error" status value
		   (note that the pointer "list" should be unaffected) */

		return PGS_E_TOOLKIT;
	    }
	}
	
	/* Search through the list until an element is found that is greater
	   than the new element and insert the new element in the list just
	   before that point.  If the new element is the greater than all other
	   list elements, it will be appended at the end of the list. */

	/* look for a list element greater than the new element */

	for (index=0;index<*length;index++)
	{
	    if (list_element < (*list)[index]) break;
	}
	
	/* copy all list elements greater than the new element "up" one index
	   value (note that if the new element is to be appended to the list,
	   the value of the variable "copy_size" will be zero and therefore
	   nothing will be moved) */

	copy_to = (char*) ((*list)+index+1);
	copy_from = (char*) ((*list)+index);
	copy_size = ((*length)-index)*sizeof(PGSt_SMF_status);
	
	memmove(copy_to, copy_from, copy_size);
	
	/* insert the new element into the list and increment the list length */

	(*list)[index] = list_element;
	*length = tmp_length;
	break;
	
      case PGSd_DELETE_ELEMENT:

	/* Remove an element from the list. 

	   Note that if the element is not memeber of the list nothing is done
	   and this routine will exit without indicating any error. */

	/* search through the list (backwards--though this is not necessary)
	   looking for the element requested to be removed */

	found_element = PGS_FALSE;
	for (index=(*length);index>0;index--)
	{
	    if (list_element == (*list)[index-1])
	    {
		/* Found a match!  Copy all list elements with indices greater
		   than that of the element to be removed "down" one index
		   value.  This will overwrite the element to be removed.  Note
		   that if the element to be removed is at the end of the list,
		   the value of the variable "copy_size" will be zero and
		   therefore nothing will be moved. */

		index = index - 1;
		found_element = PGS_TRUE;

		copy_to = (char*) ((*list)+index);
		copy_from = (char*) ((*list)+index+1);
		copy_size = ((*length)-index-1)*sizeof(PGSt_SMF_status);
		
		memmove(copy_to, copy_from, copy_size);
		break;  /* found the element, leave the "for" loop */
	    }
	}
	
	if (found_element == PGS_TRUE)
	{
	    /* An element was removed from the list, decrement the length of the
	       list.  If the new length is >= the default list size then the old
	       length was certainly greater than the default size, therefore
	       reallocate new memory (i.e. free up some memory).  This is done
	       in case the list grows very large and then shrinks back down.
	       This is an unlikely senario but what the heck, better safe than
	       sorry.  Also this routine is not expected to accessed frequently
	       so all this memory allocating and reallocating will hopefully be
	       harmless.  The default size can be adjusted if reallocating is
	       being done too much. */

	    *length = *length - 1;
	    if (*length >= PGSd_DEFAULT_LIST_SIZE)
	    {
		tmp_ptr = (PGSt_SMF_status *) realloc(*list, *length);
		if (tmp_ptr != NULL)
		{

		    /* memory allocation successful, reasign the pointer "list"
		       to point to the new memory area (if an error occured in
		       reallocating memory, the pointer "list" should be
		       unaffected, so that case is ignored here--this will have
		       the effect of leaving the list size one element larger
		       than we will use, but so what?  Besides future calls to
		       this routine may correct that.) */

		    *list = tmp_ptr;
		}
	    }
	}
	break;
	
      case PGSd_FREE_LIST:

	/* free the memory area associated with the list, set the list length to
	   zero and make sure the pointer "list" has a value of NULL */

	if (*list != NULL)
	{
	    free(*list);
	    *list = NULL;
	}
	*length = 0;
	break;

      default:
	return PGS_E_TOOLKIT;
    }
    
    /* if we made it here, everything was groovy (or close enough for government
       work, which, after all, this is)  (Don't fire me, it's just a joke!).
       The only possible problem may have been in downsizing the memory area,
       which we are ignoring (see the PGSd_REMOVE_ELEMENT case and associated
       comments for details). */

    return PGS_S_SUCCESS;
}
