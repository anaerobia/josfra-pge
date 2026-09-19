/*
 * FILENAME: dl_lists.c
 * 
 * CONTAINS:
 *		DLL_NODE_PTR 	dll_init
 *		DLL_NODE_PTR 	dll_insert
 *		DLL_NODE_PTR 	dll_add
 *				void 	dll_delete
 *				int		dll_free
 *		DLL_NODE_PTR 	dll_init_contiguous
 *				int		make_dll_title_list
 *
 */

#include <freeform.h>
#include <limits.h>

#ifdef DLL_CHECKS_ON
#ifdef NDEBUG
#undef NDEBUG
#endif

/* Private */

#define HEAD_NODE 0x8000
#define FREED     0x4000

#define dll_size(n) ((n)->length)
#define dll_count(h) dll_size(h)

#define dll_set_head_node(h) ((h)->status|=HEAD_NODE)
#define dll_mark_freed(n) ((n)->status|=FREED)
#define dll_is_freed(n) ((n)->status&FREED)
#define dll_set_new(n) ((n)->status=0)

#define dll_is_head_node(n) ((n)->status&HEAD_NODE)
#define dll_is_data_trojan(n) (!dll_is_head_node(n) && dll_size(n))

static DLL_NODE_PTR dll_node_create(DLL_NODE_PTR link_node, unsigned int dsize);
static DLL_NODE_PTR find_head_node(DLL_NODE_PTR head);
#else
static DLL_NODE_PTR dll_node_create(unsigned int dsize);
#endif

/*
 * NAME: dll_init
 *		
 * PURPOSE: To Initialize a generic linked list
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	NODE_PTR dll_init()
 *
 * DESCRIPTION: Memory is allocated for the header node (of type NODE).
 *				No memory is allocated for a data element, because the
 * 				data-element pointer in the header node points to NULL.
 *				The previous and next node pointers are set to point to
 *				the header node.
 *
 * dll_count(head) is initialized to zero.  Every time a new node is added
 * (inserted) into the list, dll_count() is incremented.  Every time a node is
 * removed from the list, dll_count() is decremented. dll_is_head_node(head)
 * is set to 1 (DLL_YES).
 *
 * RETURNS:	If successful: A pointer to header node.
 *			If Error: NULL
 *
 * SYSTEM DEPENDENT FUNCTIONS: None
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "dll_init"

DLL_NODE_PTR dll_init(void)
{
	/*  Allocate the header node */
#ifdef DLL_CHECKS_ON
	DLL_NODE_PTR head = (DLL_NODE_PTR)dll_node_create(NULL, 0);
#else
	DLL_NODE_PTR head = (DLL_NODE_PTR)dll_node_create(0);
#endif

	if (head)
	{	/* Successful Allocation */
#ifdef DLL_CHECKS_ON
		dll_count(head) = 0;
		dll_set_head_node(head);
#endif
		dll_next(head) = dll_previous(head) = head;
	}
	return(head);
}

/*
 * NAME: dll_insert
 *		
 *
 * PURPOSE: Allocate memory for and install a new node in a linked list.
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE: DLL_NODE_PTR dll_ins(DLL_NODE_PTR next_node, unsigned dsize)
 *
 * DESCRIPTION: dll_insert() inserts a new node in a generic linked
 *				list before the node pointed to by next_node.
 *				A new node is allocated and linked into the node list.
 *
 * RETURNS:	If successful: A pointer to the new node.
 *			If Error: NULL
 *
 * SYSTEM DEPENDENT FUNCTIONS: None
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "dll_insert"

DLL_NODE_PTR dll_insert(DLL_NODE_PTR next_node, unsigned int dsize)
{
#ifdef DLL_CHECKS_ON
	DLL_NODE_PTR node = dll_node_create(next_node, dsize);
#else
	DLL_NODE_PTR node = dll_node_create(dsize);
#endif
	
	if (node == NULL)
		return(NULL);

	/* make links for new node:
		next pointer of new node is next_node,
		previous pointer of new node is previous pointer of next_node */

	dll_next(node) = next_node;
	dll_previous(node) = dll_previous(next_node);

	/* adjust links for pre-existing nodes */
	dll_previous(next_node) = node;
	dll_next(dll_previous(node)) = node;
	
	return(node);
}

/*
 * NAME: dll_add
 *		
 *
 * PURPOSE: Allocate memory for and install a new node in a linked list.
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE: DLL_NODE_PTR dll_add(DLL_NODE_PTR previous_node, unsigned dsize)
 *
 * DESCRIPTION: dll_insert() inserts a new node in a generic linked
 *				list after the node pointed to by previous_node.
 *				A new node is allocated and linked into the node list.
 *
 * RETURNS:	If successful: A pointer to the new node.
 *			If Error: NULL
 *
 * SYSTEM DEPENDENT FUNCTIONS: None
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "dll_add"

DLL_NODE_PTR dll_add(DLL_NODE_PTR previous_node, unsigned int dsize)
{
#ifdef DLL_CHECKS_ON
	DLL_NODE_PTR node = dll_node_create(previous_node, dsize);
#else
	DLL_NODE_PTR node = dll_node_create(dsize);
#endif
	
	if (node == NULL)
		return(NULL);

	/* make links for new node:
		next pointer of new node is next_node of previous_node,
		previous pointer of new node is previous_node_pointer  */

	dll_previous(node) = previous_node;
	dll_next(node) = dll_next(previous_node);

	/* adjust links for pre-existing nodes */
	dll_previous(dll_next(node)) = node;
	dll_next(previous_node) = node;

	return(node);
}


/*
 * NAME: dll_delete
 *		
 * PURPOSE: To Delete a node from a standard list.
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE: void dll_delete(DLL_NODE_PTR)
 *
 * DESCRIPTION:	dll_delete() deletes the node from the linked list and resets
 *				the next and previous node pointers for both the previous and next
 *				nodes.  dll_delete() releases the memory allocated for the
 *				node.  If the data storage had been allocated in dll_ins(),
 *				then this storage is freed as well. If the data element
 *				pointed to by the node contains pointers to memory allocated
 *				by the user, this memory should be freed before deleting the
 *				node.
 *
 * The header node's dll_count() is decremented.  Assertions are used to verify
 * that the deletion node is not the header node, and that if the data block
 * associated with the deletion node is to be free()'d, it was allocated
 * separate from the node itself.  This is based on the dll_is_data_trojan()
 * bitfield being set to 0 (DLL_NO) for separate allocations.
 *
 * Note:  Cannot perform last assertion, thanks to strdb abuse.
 *
 * RETURNS:	None
 *
 * SYSTEM DEPENDENT FUNCTIONS: None
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "dll_delete"

void dll_delete(DLL_NODE_PTR node, void (*free_dll_data)(void *))
{
	assert(node);
	
#ifdef DLL_CHECKS_ON
	assert(!dll_is_head_node(node));
	assert(!dll_is_freed(node));
	
	dll_mark_freed(node);
	dll_count(find_head_node(node))--;
#endif

	dll_next(dll_previous(node)) = dll_next(node);     /* set next_node of prev_node */
	dll_previous(dll_next(node)) = dll_previous(node);     /* Set prev_node of next_node */

	if (free_dll_data)
	{
#ifdef DLL_CHECKS_ON
		assert(!dll_is_data_trojan(node));
		assert(dll_data(node) != NULL);
#endif

		(*free_dll_data)((void *)dll_data(node));
		dll_data(node) = NULL;
	}
	
	dll_previous(node) = dll_next(node) = NULL;
	memFree(node, "List Node");

	return;
}

/*
 * NAME:		dll_free
 *		
 * PURPOSE:		To delete all nodes of a doublely linked list and the data
 *				the list points to [optionally].
 *
 * USAGE:    	int dll_free(DLL_NODE_PTR node, short delete_data)
 *
 * RETURNS:		number of nodes deleted
 *
 * DESCRIPTION:	dll_free() deletes all nodes from a standard doubly
 *				linked list. If the delete_data flag is TRUE,
 *				then this storage is freed as well. If the data element
 *				pointed to by the node contains pointers to memory allocated
 *				by the user, this memory should be freed before deleting the
 *				node.
 *
 * Assertions are used to verify that if delete_data is FALSE, that the
 * dll_data(node) is the NULL pointer.  Assertions are used to verify that
 * if the dll_data(node) is to be free()'d, it was separately allocated from
 * the node.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	none
 *
 * GLOBAL:	none
 *
 * AUTHOR:	Ted Habermann, NGDC, 303-497-6284, haber@mail.ngdc.noaa.gov
 * 			Liping Di, NGDC, 303-497-6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	dll
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "dll_free"

int dll_free(DLL_NODE_PTR head, void (*free_dll_data)(void *))
{
	DLL_NODE_PTR node = NULL;
	int count = 0;

	assert(head);
	if (head == NULL)
		return(0);
	
#ifdef DLL_CHECKS_ON
	assert(dll_is_head_node(head));
#endif

	node = dll_first(head);

	/* normalize pointers below for address comparison */
	while ((void HUGE *)node != (void HUGE *)head)
	{
#ifdef DLL_CHECKS_ON
		assert(!dll_is_head_node(node));
#endif

		dll_delete(node, free_dll_data);

		count++;
		node = dll_first(head);
	}

#ifdef DLL_CHECKS_ON
	assert(dll_count(head) == 0);
#endif
	memFree(node, "Header Node");          

	return(count);
}
		
/*
 * NAME: dll_init_contiguous
 *		
 * PURPOSE: Fill a generic linked list from a contiguous array
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE: DLL_NODE_PTR dll_init_contiguous(DLL_NODE_PTR head,
 *			void *array_start,
 *			void *array_end,
 *			unsigned int dsize)
 *
 * DESCRIPTION:	dll_init_contiguous() creates a generic linked list which points at
 *				the members of an array which is stored in a contiguous block of memory.
 *
 * RETURNS:	If successful: A pointer to header node.
 *			If Error: NULL
 *
 * SYSTEM DEPENDENT FUNCTIONS: None
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "dll_init_contiguous"

DLL_NODE_PTR dll_init_contiguous(void *array_start,	void *array_end, unsigned int dsize)
{
	DLL_NODE_PTR	dll_head;
	DLL_NODE_PTR	node;
	char 			*ptr;

	/* First Initialize the head element */
	node = dll_head = dll_init();

	ptr = (char *)array_start;

	while((void HUGE *)ptr <= (void HUGE *)array_end)
	{	/* Add a new Node to the list without allocating memory for data */
		node = dll_add(node, 0);
		dll_data(node) = (void *)ptr;
		ptr += dsize;
	}
	return(dll_head);
}

/*
 * NAME:	make_dll_title_list-  This function has moved to stringdb.c, as
 *                it really was a sdb_ function anyway (lots of phantom strings and
 *                other nasty stuff)
 */


#ifdef DLL_CHECKS_ON
static DLL_NODE_PTR dll_node_create(DLL_NODE_PTR link_node, unsigned int dsize)
#else
static DLL_NODE_PTR dll_node_create(unsigned int dsize)
#endif
/*****************************************************************************
 * NAME: dll_node_create()
 *
 * PURPOSE:  Allocate a memory block for a new DLL_NODE, plus dsize for data
 *
 * USAGE:  node = dll_node_create(link_node, dsize);
 *
 * RETURNS:  NULL if operation fails, otherwise a pointer to an allocated
 * node/data block
 *
 * DESCRIPTION:  Allocates space for DLL_NODE plus dsize.  dsize is the size
 * in bytes of the data block to which the dll_data(node) will point to.  If
 * dsize is zero, space is allocated only for the DLL_NODE, and the
 * dll_data(node) is assigned the NULL pointer.
 *
 * The header node's dll_size() is incremented.  Incoming parameter
 * link_node is used solely to located the header node, which is based on
 * the dll_is_head_node() bit field being set to 1 (DLL_YES) for the
 * header node and 0 (DLL_NO) for all other nodes (the data nodes).
 * dll_is_head_node() is set to 0 (DLL_NO) for the new node.  If dsize is zero.
 * dll_is_data_trojan() is set to 0 (DLL_NO) for the new node, otherwise 1 (DLL_YES),
 * and dll_size() is set to dsize.
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
{
	DLL_NODE_PTR node;

	/* Get memory for data element and set dll_data(node)
	to point to it */

	node = (DLL_NODE_PTR)memMalloc(sizeof(DLL_NODE) + dsize, "List Node");

	if (node == NULL)
		return(NULL);

#ifdef DLL_CHECKS_ON
	dll_set_new(node);
#endif
	
	dll_next(node) = dll_previous(node) = NULL;
	
	if (dsize)
  {
		dll_data(node) = (void *)(node + 1);
#ifdef DLL_CHECKS_ON
		dll_size(node) = dsize;
#endif
	}
	else
	{
		dll_data(node) = NULL;
#ifdef DLL_CHECKS_ON
		dll_size(node) = 0;
#endif
	}

	/* Find header */
#ifdef DLL_CHECKS_ON
	if (link_node)
		dll_size(find_head_node(link_node))++;
#endif

	return(node);
}

#ifdef DLL_CHECKS_ON
static DLL_NODE_PTR find_head_node(DLL_NODE_PTR head)
{
	int i = 0;

	assert(head);
	while (!dll_is_head_node(head))
	{
		if (i++ == INT_MAX)
			break;
		head = dll_previous(head);
	}
	
	assert(dll_data(head) == NULL);
	return(head);
}
#endif
