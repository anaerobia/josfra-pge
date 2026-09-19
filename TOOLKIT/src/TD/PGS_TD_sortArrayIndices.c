/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include <PGS_TD.h>

PGSt_SMF_status
PGS_TD_sortArrayIndices(
    PGSt_double  inVector[],
    PGSt_integer numValues,
    PGSt_integer sortedIndices[])
{
    PGSt_integer cnt;
    PGSt_integer cntBack;
    PGSt_integer tmp;
    
    if (numValues < 1)
      return PGSTD_E_BAD_ARRAY_SIZE;
    else if(numValues == 1)
      sortedIndices[0] = 0;
	
    sortedIndices[0] = 0;
    
    for (cnt=0;cnt<numValues-1;cnt++)
    {
	if (inVector[sortedIndices[cnt]] <= inVector[cnt+1])
	  sortedIndices[cnt+1] = cnt+1;
	else
	{
	    sortedIndices[cnt+1] = sortedIndices[cnt];
	    sortedIndices[cnt] = cnt + 1;
	    for (cntBack=cnt;cntBack > 0; cntBack--)
	    {
		if (inVector[sortedIndices[cntBack]] <
		    inVector[sortedIndices[cntBack-1]])
		{
		    tmp = sortedIndices[cntBack];
		    sortedIndices[cntBack] = sortedIndices[cntBack-1];
		    sortedIndices[cntBack-1] = tmp;
		}
		else
		  break;
	    }
	}
    }
    return PGS_S_SUCCESS;
}

    
