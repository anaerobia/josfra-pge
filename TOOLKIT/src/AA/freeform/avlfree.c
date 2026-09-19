#include <stdio.h>		/* AVLFREE.C */
#include <avltree.h>
#include <memtrack.h>

#ifdef PROTO
static void fa(HEADER *root)
#else
static void	fa( root )
HEADER		*root;
#endif
{
	/* Delete the entire tree pointed to by root. Note that unlike
	 * tfree(), this routine is passed a pointer to a HEADER rather
	 * than to the memory just below the header.
	 */

	if( root )
	{
		fa( root->left  );
		fa( root->right );
		memFree( root, "In fa avlfree.c" );
	 
	}
}

/*----------------------------------------------------------------------*/

#ifdef PROTO
void freeall(HEADER **root)
#else
void	freeall( root )
HEADER **root;
#endif
{
	fa( *root );
	*root = NULL;
}
