#include <stdio.h>	/* needed by assert.h on unix -rf01 */
#include <adtype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define GBEOLS_EOL_CR 13
#define GBEOLS_EOL_LF 10

/*
 * NAME:    queue_stack_op
 *      
 * PURPOSE: to preform the functions of a queue and stack
 *
 * USAGE:   void *queue_stack_op(QSTACK_PTR qs, int op, void *data)
 *
 * RETURNS: void * to data if appropriate, NULL on error (except in case QS_KILL-
 *				NULL is always returned)
 *
 * DESCRIPTION: This function is set to handle all the basic functions of a
 *				queue and a stack (they are so close, they don't even deserve
 *				seperate functions).  The "events" this function handles are:
 *
 *				QS_PUSH- puts the data on the end of the queue/stack
 *					returns: ptr to the new node, NULL on error
 *				QS_POP- pops data off the end of the stack/queue
 *					returns: ptr to the data popped off, NULL on error
 *				QS_PULL- pulls data off front of queue/stack
 *					returns: ptr to the data pulled off, NULL on error
 *				QS_NEW- Creates a new QSTACK descriptor
 *					returns: pointer to new QSTACK descriptor, NULL on error
 *				QS_KILL- Frees all memory taken by the queue/stack,
 *						including the descriptor.
 *					returns: NULL
 *
 *				To initialize a queue/stack, simply use the QS_NEW event.  This
 *				will return a QSTACK ptr to a queue/stack description.  This is
 *				then passed to the queue_stack_op function to specify which queue/
 *				stack is being operated on.
 *					example: queue = (QSTACK_PTR)queue_stack_op(NULL, QS_NEW, NULL);
 *
 *				To push on data, call the QS_PUSH event (both stacks and queues)
 *					example: 
 *						if(!(queue_stack_op(queue, QS_PUSH, (void *)data))){
 *							error code here...
 *						}
 *
 *				To pop off data (stack), call QS_POP.
 *					example: data = (data *)queue_stack_op(stack, QS_POP, NULL);
 *
 *				To pull off data (queue), call QS_PULL.
 *					example: data = (data *)queue_stack_op(queue, QS_PULL, NULL);
 *
 *				When done with the queue/stack, call QS_KILL to free it.
 *					example: queue_stack_op(stack, QS_KILL, NULL);
 *
 *				This function contains no ERR_PUSH statements FOR A REASON:
 *				1) The errors aren't that complex anyway (out of memory is about it),
 *				2) The function is called by things which are not freeform,
 *					and hence have no ERR_PUSH routine.
 *
 *				DO NOT ADD ERR_PUSHES TO THIS FUNCTION!!!!
 *	
 * SYSTEM DEPENDENT FUNCTIONS:  none- completely portable
 *
 * AUTHOR:  Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS: DO NOT ADD ERR_PUSHES TO THIS FUNCTION!!!!
 *
 *      
 * KEYWORDS: queue stack dll
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "queue_stack_op"
void *queue_stack_op(QSTACK_PTR qs, int op, void *data)
{
	QSTACK_PTR node;
	QSTACK_PTR newnode;
	
	switch(op){
		case QS_PUSH:
			if(!qs)
				return(NULL);
			node = qs->next; /* Get rear of queue/stack */
			if(!(newnode = (QSTACK_PTR)malloc(sizeof(QSTACK))))
				return(NULL);
			newnode->data = data;
			newnode->prev = node;
			newnode->next = NULL;
			if(!node){
				qs->prev = newnode;
				qs->next = newnode;
			}
			else{
				qs->next = newnode;
				node->next = newnode;
			}
			return(newnode);
		case QS_POP:
			if(!qs)
				return(NULL);
			node = qs->next;
			if(!node) /* No more to pop */
				return(NULL);
			newnode = node->prev;
			if(!newnode){
				qs->next = NULL;
				qs->prev = NULL;
			}
			else{
				newnode->next = NULL;
				qs->next = newnode;
			}
			data = node->data;
			free(node);
			return(data);
		case QS_PULL:
			if(!qs)
				return(NULL);
			node = qs->prev;
			if(!node) /* No more to pull */
				return(NULL);
			newnode = node->next;
			if(!newnode){
				qs->next = NULL;
				qs->prev = NULL;
			}
			else{
				newnode->prev = NULL;
				qs->prev = newnode;
			}
			data = node->data;
			free(node);
			return(data);
		case QS_NEW:
			if(!(newnode = (QSTACK_PTR)malloc(sizeof(QSTACK))))
				return(NULL);
			newnode->data = data;
			newnode->prev = NULL;
			newnode->next = NULL;
			return(newnode);
		case QS_KILL:
			if(!qs)
				return(NULL);
			node = qs->prev;
			while(node){
				newnode = node;
				node = node->next;
				free(newnode);
			}
			free(qs);
			return(NULL);
		default:
			return(NULL);
	}
}

/*
 * NAME:        ff_get_buffer_eol_str
 *              
 * PURPOSE:     to determine the EOL sequence for a given buffer
 *
 * USAGE:       int ff_get_buffer_eol_str(char *buffer, char *buffer_eol_str)
 *
 * RETURNS:     0 if all is OK, >< 0 on error
 *
 * DESCRIPTION: Determines the EOL sequence for a given file.  If no EOL
 *				sequence is observed, buffer_eol_str is simply set to '\0';
 *				otherwise the first characters of buffer_eol_str are set to 
 *				the EOL sequence (buffer_eol_str must be at least 3 bytes)
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none- completely portable
 *
 * AUTHOR:      Kevin Frender kbf@ngdc.noaa.gov
 *
 * COMMENTS:
 *          
 * KEYWORDS: EOL
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_get_buffer_eol_str"
int ff_get_buffer_eol_str(char *buffer, char *buffer_eol_str)
{
	char *c;
	
	assert(buffer && buffer_eol_str);
	
	c = buffer;
	
	while(c[0]){
		if(c[0] == GBEOLS_EOL_LF){ /* LF */
			/* Must be a unix file */
			buffer_eol_str[0] = (char)GBEOLS_EOL_LF;
			buffer_eol_str[1] = '\0';
			return(0);
		}
		if(c[0] == GBEOLS_EOL_CR){ /* CR */
			c++;
			if(c[0] == GBEOLS_EOL_LF){ /* CR-LF */
				/* Must be a DOS file */
				buffer_eol_str[0] = (char)GBEOLS_EOL_CR;
				buffer_eol_str[1] = (char)GBEOLS_EOL_LF;
				buffer_eol_str[2] = '\0';
				return(0);
			}
			/* Must be a MAC file */
			buffer_eol_str[0] = (char)GBEOLS_EOL_CR;
			buffer_eol_str[1] = '\0';
			return(0);
		}
		c++;
	}
	
	/* Couldn't find any EOL chars */
	buffer_eol_str[0] = '\0';
	return(0);
}

