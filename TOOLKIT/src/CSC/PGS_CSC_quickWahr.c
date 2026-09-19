/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_TSF.h>

#define EPOCH_DAY            2448988.5 /* TAI Julian day of 0 hrs UTC 1-1-93 */
#define EPOCH_DAY_FRACTION   0.0003125 /* TAI Julian day fraction 0 hrs UTC */
#define SECONDSperDAY        86400.0   /* number of seconds in a day */ 

#define MAX_TIME_DIFF        1200.0    /* maximum time (in seconds) over which
					  to extrapolate nutation angles */
/* name of this function */

#define FUNCTION_NAME "PGS_CSC_quickWahr()"

PGSt_SMF_status 
PGS_CSC_quickWahr(           /* return nutation angles and rates and TDB at
				input TAI time (extrapolating over short 
				periods) */
    PGSt_double secTAI93,    /* TAI (toolkit internal time) */
    PGSt_double jedTDB[2],   /* TDB Julian Ephemeris Date */
    PGSt_double dvnut[4])    /* nutation angles and rates
				dvnut[0] - nutation in longitude (radians)
				dvnut[1] - nutation in obliquity (radians)
				dvnut[2] - nut. rate in longitude (radians/sec)
				dvnut[3] - nut. rate in obliquity (rad/sec) */

{
    
    PGSt_double jdTAI[2];            /* TAI as a Julian Day  */
    PGSt_double jedTDT[2];           /* TDT expressed in Julian days 
					(leap seconds added in, and 32.184) */
    PGSt_double timeDiff;            /* secTAI93 - oldsecTAI93, used to avoid
					repeated setup */
    
    static PGSt_double jedTDBold[2]; /* TDB expressed in Julian days (additional
					periodic correction) */
    static PGSt_double dvnutOld[4];  /* the two nutation angles and their rates,
					output from "PGS_CSC_wahr2" */

    static PGSt_double oldsecTAI93=-1.e50; /* previous value (to avoid repeated
					      setup) (initialized to junk) */

#ifdef _PGS_THREADSAFE

    /* Declare variables used for THREADSAFE version to replace statics
       The local names are appended with TSF and globals are preceded with
       directory and function name    */

    PGSt_double jedTDBoldTSF[2];
    PGSt_double dvnutOldTSF[4];
    PGSt_double oldsecTAI93TSF;

    /* 3  Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_double PGSg_TSF_CSCQuickWahroldsecTAI93[];
    extern PGSt_double PGSg_TSF_CSCQuickWahrdvnutOld[][4];
    extern PGSt_double PGSg_TSF_CSCQuickWahrjedTDBold[][2];
    int masterTSFIndex;
    int x;

    /* Get index    Then test for bad return */

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ( masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* Initialize the variables used for the THREADSAFE version */

    for (x=0; x<2; x++)
    {
        jedTDBoldTSF[x] = PGSg_TSF_CSCQuickWahrjedTDBold[masterTSFIndex][x];
    }
    for (x=0; x<4; x++)
    {
        dvnutOldTSF[x] = PGSg_TSF_CSCQuickWahrdvnutOld[masterTSFIndex][x];
    }
    oldsecTAI93TSF = PGSg_TSF_CSCQuickWahroldsecTAI93[masterTSFIndex];

    /* Almost entire function is duplicated for the THREADSAFE version to
       protect statics  When a value is reassigned the global is updated
                         NO LOCKS  NO KEYS   3 GLOBALS   */


    /* Threadsafe Protect: oldsecTAI93,   reassign its value for next use */

    timeDiff = secTAI93 - oldsecTAI93TSF;
    
    if ( fabs(timeDiff) > MAX_TIME_DIFF ) /* setup */
    {
	oldsecTAI93TSF = secTAI93;    
        PGSg_TSF_CSCQuickWahroldsecTAI93[masterTSFIndex] = oldsecTAI93TSF;

        timeDiff = 0.0;
	
	/* Get TDB - this is needed to get precession and nutation */
	
	/* convert TAI input to Julian date format */
	
	PGS_TD_TAItoTAIjd(secTAI93,jdTAI);
	    	
	/* get TDT from TAI */
	
	PGS_TD_TAIjdtoTDTjed(jdTAI,jedTDT); 
	
	/* compute jedTDB */
	    
        /* Threadsafe Protect: jedTDBold,   reassign its value for next use */

	PGS_TD_TDTjedtoTDBjed(jedTDT,jedTDBoldTSF);
        for (x=0; x<2; x++)
        {
            PGSg_TSF_CSCQuickWahrjedTDBold[masterTSFIndex][x] =
                                                   jedTDBoldTSF[x];
        }
	        
	/*  Obtain the nutation angles and rates  */
	
        /* Threadsafe Protect: dvnutOld,   reassign its value for next use */

	PGS_CSC_wahr2(jedTDT,dvnutOldTSF); 
        for (x=0; x<4; x++)
        {
            PGSg_TSF_CSCQuickWahrdvnutOld[masterTSFIndex][x] = dvnutOldTSF[x];
        }


    } /* end "if ( fabs(timeDiff .." */ 

    /* extrapolate */
    
    /* Threadsafe Protect: dvnutOld, jedTDBold,   */

    dvnut[0] = dvnutOldTSF[0] + dvnutOldTSF[2]*timeDiff;
    dvnut[1] = dvnutOldTSF[1] + dvnutOldTSF[3]*timeDiff;
    dvnut[2] = dvnutOldTSF[2];
    dvnut[3] = dvnutOldTSF[3];
    jedTDB[0] = jedTDBoldTSF[0];
    jedTDB[1] = jedTDBoldTSF[1] + timeDiff/SECONDSperDAY;
#else
    timeDiff = secTAI93 - oldsecTAI93;
    
    if ( fabs(timeDiff) > MAX_TIME_DIFF ) /* setup */
    {
	oldsecTAI93 = secTAI93;    
        timeDiff = 0.0;
	
	/* Get TDB - this is needed to get precession and nutation */
	
	/* convert TAI input to Julian date format */
	
	PGS_TD_TAItoTAIjd(secTAI93,jdTAI);
	    	
	/* get TDT from TAI */
	
	PGS_TD_TAIjdtoTDTjed(jdTAI,jedTDT); 
	
	/* compute jedTDB */
	    
	PGS_TD_TDTjedtoTDBjed(jedTDT,jedTDBold);
	        
	/*  Obtain the nutation angles and rates  */
	
	PGS_CSC_wahr2(jedTDT,dvnutOld); 

    } /* end "if ( fabs(timeDiff .." */ 

    /* extrapolate */
    
    dvnut[0] = dvnutOld[0] + dvnutOld[2]*timeDiff;
    dvnut[1] = dvnutOld[1] + dvnutOld[3]*timeDiff;
    dvnut[2] = dvnutOld[2];
    dvnut[3] = dvnutOld[3];
    jedTDB[0] = jedTDBold[0];
    jedTDB[1] = jedTDBold[1] + timeDiff/SECONDSperDAY;
#endif

    return PGS_S_SUCCESS;
}




