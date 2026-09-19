/* AVLINS.C */
/*
 * HISTORY:
 *	Rich Fozzard	12/22/95		-rf01
 *		Mac compiler needs unix.h instead of malloc.h
*/


#include <stdio.h>
#include <avltree.h>
#include <memtrack.h>

/*----------------------------------------------------------------------
 * Externally accessible routines:
 *
 * HEADER	*insert( rootp, newnode, cmp )   Insert newnode in tree
 * HEADER	*talloc( size )			 Allocate a tree node
 * void		tfree( p )			 Free a tree node
 * 
 *----------------------------------------------------------------------
 */
static int	(*Cmp)();

static  HEADER	*Newnode;
static  HEADER	*Conflicting;

/*----------------------------------------------------------------------*/
#ifdef CCLSC /* Mac compiler needs unix.h instead of malloc.h -rf01 */
#include <unix.h>
#else
#include <malloc.h>
#endif

#ifdef PROTO
HEADER *talloc(int size)
#else
HEADER *talloc(size)
int size;
#endif
{					  
	HEADER	*p;
	if ((p = (HEADER *)memMalloc( size + sizeof(HEADER),"in talloc")) != NULL)
	{			   
		p->left  = NULL;
		p->right = NULL;
		p->size  = size;
		p->bal	 = B;
	}	
	return p;
}

/*----------------------------------------------------------------------*/

#ifdef PROTO
void tfree(HEADER *p)
#else
void	tfree( p )
HEADER	*p;
#endif
{
	memFree( --p, "in tfree" );
}

/*----------------------------------------------------------------------*/

#ifdef PROTO
static void ins(HEADER **pp)
#else
static void ins( pp )
HEADER	**pp;
#endif
{
	HEADER	        *p;
	HEADER  	*p1, *p2;
	int     	relation;  /* relation >  0  <==> p >  Newnode
				    * relation <  0  <==> p <  Newnode
				    * relation == 0  <==> p == Newnode
				    */

	static int     h = 0;	   /* Set by recursive calls to search to
				    * indicate that the tree has grown.
				    * It will magically change its value
				    * everytime ins() is called recursively.
				    */
	if( !(p = *pp) )
	{
		p = Newnode ;	   /* insert node in tree	*/
		h = 1;
	}
	else if( (relation = (* Cmp)( p+1, Newnode+1)) == 0 )
	{
		Conflicting = p + 1;
		h = 0;
	}
	else if( relation > 0 )
	{
		ins( &p->left );

		if( h )				   /* left branch has grown */
		{
			switch( p->bal )
			{
			case  R: p->bal = B ;  h = 0;	break;
			case  B: p->bal = L ;		break;

			case  L:				/* rebalance */
				p1 = p->left;
				if( p1->bal == L )		/* Single LL */
				{
					p->left   = p1->right;
					p1->right = p;
					p->bal    = B;
					p	  = p1;
				}
				else				/* Double LR */
				{
					p2	  = p1->right;
					p1->right = p2->left;
					p2->left  = p1;
					p->left   = p2->right;
					p2->right = p;
					p->bal    = (p2->bal == L) ?  R : B ;
					p1->bal   = (p2->bal == R) ?  L : B ;
					p	  = p2;
				}
				p->bal = B;
				h      = 0;
			}
		}
	}
	else
	{
		ins( &p->right );

		if( h )				  /* right branch has grown */
		{
			switch( p->bal )
			{
			case L: p->bal = B;   h = 0;	break;
			case B: p->bal = R;		break;

			case R:				      /* rebalance: */
				p1 = p->right;
				if( p1->bal == R )	      /* Single RR  */
				{
					p->right = p1->left;
					p1->left = p;
					p->bal   = B;
					p	 = p1;
				}
				else			      /* Double RL  */
				{
					p2	  = p1->left;
					p1->left  = p2->right;
					p2->right = p1;
					p->right  = p2->left;
					p2->left  = p;
					p->bal    = (p2->bal == R) ? L : B ;
					p1->bal   = (p2->bal == L) ? R : B ;
					p	  = p2;
				}
				p->bal = B;
				h      = 0;
			}
		}
	}

	*pp = p;
}

/*----------------------------------------------------------------------*/
#ifdef PROTO

HEADER *insert(HEADER **rootp, HEADER *newnode, int (*cmp)())

#else

HEADER	*insert( rootp, newnode, cmp )
HEADER	**rootp;
HEADER	*newnode;
int		(*cmp)();

#endif

{
	/*  Insert newnode into tree pointed to by *rootp. Cmp is passed
	 *  two pointers to HEADER and should work like strcmp().
	 *  Return NULL on success or a pointer to the conflicting node
	 *  on error.
	 */

	Cmp 	    = cmp;
	Newnode     = newnode - 1;
	Conflicting = NULL;

	ins(rootp);

	return Conflicting;
}
