/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_TD.h>
#include <PGS_IO.h>


PGSt_SMF_status
PGS_IO_L0_EDOS_hdr_Sim(
    unsigned char    scID,
    unsigned char    numAPIDs,
    PGSt_integer     appID[],
    PGSt_integer     dataLength[],
    unsigned char    numFiles,
    unsigned long    sizePDS,
    unsigned int     numPkts,
    PGSt_double      startTAI93,
    PGSt_double      stopTAI93,
    PGSt_double      timeInterval,
    char             *filename,
    PGSt_tag          spacecraftTag)
{
    long             hdr_size;
    unsigned char    *eosamFileHdr;
    char             asciiUTC[28];
    FILE             *hdr_file_ptr;
    PGSt_integer     filePkts;
    PGSt_integer     granuleSize;
    PGSt_double      jdUTC[2];
    PGSt_boolean     onLeap;
    PGSt_SMF_status  returnStatus;
    unsigned int     beginAPID;
    unsigned int     endAPID;
    unsigned int     pkt_offset;
    unsigned long    numThisAPID;
    unsigned long    sizeThisAPID;
    unsigned long    offset;
    int              cnt;
    int              cnt2;
    PGSt_IO_L0_SecPktHdrEOS_PM_GIRD eospmSecHdr2;
    PGSt_IO_L0_SecPktHdrEOS_AURA eosauraSecHdr;
    char             asciiUTCtemp[28];

    /* header for EOS_AM */
if(spacecraftTag == PGSd_EOS_AM || spacecraftTag == PGSd_EOS_AURA || spacecraftTag == PGSd_EOS_PM_GIIS || spacecraftTag == PGSd_EOS_PM_GIRD)
{
   
    if (spacecraftTag == PGSd_EOS_AM)
    {
	
    hdr_size = 152 + 96*numAPIDs + (44 + 24*numAPIDs)*(numFiles);
    eosamFileHdr = (unsigned char*) calloc(1,hdr_size);
    eosamFileHdr[2] = 1;
    eosamFileHdr[40] = 1;
    eosamFileHdr[51] = 1;

    memcpy((void*) (eosamFileHdr+4), (void*) filename, 36);
    
    returnStatus = PGS_TD_TAItoUTCjd(startTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
	onLeap = PGS_TRUE;
    }
    else
    {
	onLeap = PGS_FALSE;
    }    
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+53);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+97);
    if (returnStatus != PGS_S_SUCCESS &&
	returnStatus != PGSTD_W_PRED_LEAPS)
    {
	free(eosamFileHdr);
	return returnStatus;
    }
    
    returnStatus = PGS_TD_TAItoUTCjd(stopTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
	onLeap = PGS_TRUE;
    }
    else
    {
	onLeap = PGS_FALSE;
    }
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+61);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+105);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+133);
    if (returnStatus != PGS_S_SUCCESS &&
	returnStatus != PGSTD_W_PRED_LEAPS)
    {
	free(eosamFileHdr);
	return returnStatus;
    }
    
    PGS_TD_TAItoUTC(startTAI93, asciiUTC);
    PGS_TD_UTCtoEOSAM(asciiUTC, eosamFileHdr+80);
    PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
    PGS_TD_UTCtoEOSAM(asciiUTC, eosamFileHdr+88);

    memcpy((void*)(eosamFileHdr+97), (void*)(eosamFileHdr+53), 7);
    memcpy((void*)(eosamFileHdr+105), (void*)(eosamFileHdr+61), 7);
    
    eosamFileHdr[116] = numPkts/0x1000000;
    eosamFileHdr[117] = (numPkts/0x10000)%0x100;
    eosamFileHdr[118] = (numPkts/0x100)%0x100;
    eosamFileHdr[119] = numPkts%0x100;
    
    eosamFileHdr[124] = sizePDS/0x1000000;
    eosamFileHdr[125] = (sizePDS/0x10000)%0x100;
    eosamFileHdr[126] = (sizePDS/0x100)%0x100;
    eosamFileHdr[127] = sizePDS%0x100;

    memcpy((void*)(eosamFileHdr+133), (void*)(eosamFileHdr+61), 7);

    eosamFileHdr[147] = numAPIDs;
    
    eosamFileHdr[151+numAPIDs*96] = numFiles;
    
    /* tricky new stuff */

    granuleSize = numPkts/(numFiles-1);

    offset = 152 + 96*numAPIDs;
    
    for (cnt=0; cnt<(int)numFiles; cnt++)
    {
	memcpy((void*)(eosamFileHdr+offset), (void*)(filename), 36);
	sprintf((char*) (eosamFileHdr+offset+34), "%02d.PDS", cnt);
	if (cnt != 0)
	{
	    filePkts = (cnt == numFiles) ? numPkts-(cnt-1)*granuleSize :
	                                   granuleSize;
	    eosamFileHdr[offset+43] = numAPIDs;
	    beginAPID = granuleSize*(cnt-1);
	    endAPID = beginAPID + filePkts - numAPIDs;
	    for (cnt2=0;cnt2<(int)numAPIDs;cnt2++)
	    {
		eosamFileHdr[offset+45+(24*cnt2)] = scID;
		eosamFileHdr[offset+46+(24*cnt2)] = appID[beginAPID]/256;
		eosamFileHdr[offset+47+(24*cnt2)] = appID[cnt2]%256;
		PGS_TD_TAItoUTC(startTAI93+(beginAPID+cnt2)*timeInterval,
				asciiUTC);
		PGS_TD_UTCtoEOSAM(asciiUTC, eosamFileHdr+(offset+48+(24*cnt2)));
		PGS_TD_TAItoUTC(startTAI93+(endAPID+cnt2)*timeInterval,
				asciiUTC);
		PGS_TD_UTCtoEOSAM(asciiUTC, eosamFileHdr+(offset+56+(24*cnt2)));
	    }
	}
	else
	{
	    numAPIDs = 1;
	}
	
	offset += (44 + 24*numAPIDs);
    }
    
    offset = 148;
    
    for (cnt=0; cnt<(int)numAPIDs; cnt++)
    {
	eosamFileHdr[offset+1] = scID;
	eosamFileHdr[offset+2] = appID[cnt]/256;
	eosamFileHdr[offset+3] = appID[cnt]%256;
	pkt_offset = 0;
	for (cnt2=1;cnt2<cnt;cnt2++)
	{
	    pkt_offset += dataLength[cnt2-1] + (PGSd_IO_L0_PrimaryPktHdrSize +
						PGSd_IO_L0_SecPktHdrSizeEOS_AM);
	}
	eosamFileHdr[offset+8] = pkt_offset/0x1000000;
	eosamFileHdr[offset+9] = (pkt_offset/0x10000)%0x100;
	eosamFileHdr[offset+10] = (pkt_offset/0x100)%0x100;
	eosamFileHdr[offset+11] = pkt_offset%0x100;
	eosamFileHdr[offset+15] = 1;
	eosamFileHdr[offset+18] = scID/64;
	eosamFileHdr[offset+19] = scID%64 << 2;
	PGS_TD_TAItoUTC(startTAI93+cnt*timeInterval, asciiUTC);
	PGS_TD_UTCtoEOSAM(asciiUTC, eosamFileHdr+(offset+40));
	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	{
	    onLeap = PGS_TRUE;
	}
	else
	{
	    onLeap = PGS_FALSE;
	}
	PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+57));
	
	PGS_TD_TAItoUTC(startTAI93+(numPkts-numAPIDs+cnt)*timeInterval, 
			asciiUTC);
	PGS_TD_UTCtoEOSAM(asciiUTC, eosamFileHdr+(offset+48));
	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	{
	    onLeap = PGS_TRUE;
	}
	else
	{
	    onLeap = PGS_FALSE;
	}
	PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+65));

	numThisAPID = numPkts/numAPIDs;
	if ((cnt+1) <= numPkts%numAPIDs)
	{
	    numThisAPID += 1;
	}
	sizeThisAPID = numThisAPID*(dataLength[cnt]+PGSd_IO_L0_PrimaryPktHdrSize
				    +PGSd_IO_L0_SecPktHdrSizeEOS_AM);

	
	eosamFileHdr[offset+76] = numThisAPID/0x1000000;
	eosamFileHdr[offset+77] = (numThisAPID/0x10000)%0x100;
	eosamFileHdr[offset+78] = (numThisAPID/0x100)%0x100;
	eosamFileHdr[offset+79] = numThisAPID%0x100;

	eosamFileHdr[offset+84] = sizeThisAPID/0x1000000;
	eosamFileHdr[offset+85] = (sizeThisAPID/0x10000)%0x100;
	eosamFileHdr[offset+86] = (sizeThisAPID/0x100)%0x100;
	eosamFileHdr[offset+87] = sizeThisAPID%0x100;

	offset += 96;
    }
    
    hdr_file_ptr = fopen(filename, "w");
    fwrite(eosamFileHdr, hdr_size, 1, hdr_file_ptr);
    fclose(hdr_file_ptr);
    free(eosamFileHdr);
    return PGS_S_SUCCESS;
    }

    /* header for EOS_AURA */
 
    if (spacecraftTag == PGSd_EOS_AURA)
    {
    hdr_size = 152 + 96*numAPIDs + (44 + 24*numAPIDs)*(numFiles);
    eosamFileHdr = (unsigned char*) calloc(1,hdr_size);
    eosamFileHdr[2] = 1;
    eosamFileHdr[40] = 1;
    eosamFileHdr[51] = 1;
 
    memcpy((void*) (eosamFileHdr+4), (void*) filename, 36);
 
    returnStatus = PGS_TD_TAItoUTCjd(startTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
        onLeap = PGS_TRUE;
    }
    else
    {
        onLeap = PGS_FALSE;
    }
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+53);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+97);
    if (returnStatus != PGS_S_SUCCESS &&
        returnStatus != PGSTD_W_PRED_LEAPS)
    {
        free(eosamFileHdr);
        return returnStatus;
    }
 
    returnStatus = PGS_TD_TAItoUTCjd(stopTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
        onLeap = PGS_TRUE;
    }
    else
    {
        onLeap = PGS_FALSE;
    }
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+61);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+105);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+133);
    if (returnStatus != PGS_S_SUCCESS &&
        returnStatus != PGSTD_W_PRED_LEAPS)
    {
        free(eosamFileHdr);
        return returnStatus;
    }

    PGS_TD_TAItoUTC(startTAI93, asciiUTC);
    PGS_TD_UTCtoEOSAURAGIRD(asciiUTC, eosauraSecHdr.scTime);
    PGS_TD_EOSAURAtoUTC(eosauraSecHdr.scTime,asciiUTCtemp);
    PGS_TD_UTCtoEOSAURAGIIS(asciiUTCtemp, eosamFileHdr+80);
    PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
    PGS_TD_UTCtoEOSAURAGIRD(asciiUTC, eosauraSecHdr.scTime);
    PGS_TD_EOSAURAtoUTC(eosauraSecHdr.scTime,asciiUTCtemp);
    PGS_TD_UTCtoEOSAURAGIIS(asciiUTCtemp, eosamFileHdr+88);

    memcpy((void*)(eosamFileHdr+97), (void*)(eosamFileHdr+53), 7);
    memcpy((void*)(eosamFileHdr+105), (void*)(eosamFileHdr+61), 7);
 
    eosamFileHdr[116] = numPkts/0x1000000;
    eosamFileHdr[117] = (numPkts/0x10000)%0x100;
    eosamFileHdr[118] = (numPkts/0x100)%0x100;
    eosamFileHdr[119] = numPkts%0x100;
 
    eosamFileHdr[124] = sizePDS/0x1000000;
    eosamFileHdr[125] = (sizePDS/0x10000)%0x100;
    eosamFileHdr[126] = (sizePDS/0x100)%0x100;
    eosamFileHdr[127] = sizePDS%0x100;
 
    memcpy((void*)(eosamFileHdr+133), (void*)(eosamFileHdr+61), 7);
 
    eosamFileHdr[147] = numAPIDs;
 
    eosamFileHdr[151+numAPIDs*96] = numFiles;
 
    /* tricky new stuff */
 
    granuleSize = numPkts/(numFiles-1);
 
    offset = 152 + 96*numAPIDs;
 
    for (cnt=0; cnt<(int)numFiles; cnt++)
    {
        memcpy((void*)(eosamFileHdr+offset), (void*)(filename), 36);
        sprintf((char*) (eosamFileHdr+offset+34), "%02d.PDS", cnt);
        if (cnt != 0)
        {
            filePkts = (cnt == numFiles) ? numPkts-(cnt-1)*granuleSize :
                                           granuleSize;
            eosamFileHdr[offset+43] = numAPIDs;
            beginAPID = granuleSize*(cnt-1);
            endAPID = beginAPID + filePkts - numAPIDs;
            for (cnt2=0;cnt2<(int)numAPIDs;cnt2++)
            {
                eosamFileHdr[offset+45+(24*cnt2)] = scID;
                eosamFileHdr[offset+46+(24*cnt2)] = appID[beginAPID]/256;
                eosamFileHdr[offset+47+(24*cnt2)] = appID[cnt2]%256;
                PGS_TD_TAItoUTC(startTAI93+(beginAPID+cnt2)*timeInterval,
                                asciiUTC);
                PGS_TD_UTCtoEOSAURAGIRD(asciiUTC, eosauraSecHdr.scTime);
                PGS_TD_EOSAURAtoUTC(eosauraSecHdr.scTime,asciiUTCtemp);
                PGS_TD_UTCtoEOSAURAGIIS(asciiUTCtemp, eosamFileHdr+(offset+48+(24*cnt2)));
                PGS_TD_TAItoUTC(startTAI93+(endAPID+cnt2)*timeInterval,
                                asciiUTC);
                PGS_TD_UTCtoEOSAURAGIRD(asciiUTC, eosauraSecHdr.scTime);
                PGS_TD_EOSAURAtoUTC(eosauraSecHdr.scTime,asciiUTCtemp);
                PGS_TD_UTCtoEOSAURAGIIS(asciiUTCtemp, eosamFileHdr+(offset+56+(24*cnt2)));
            }
        }
        else
        {
            numAPIDs = 1;
        }
 
        offset += (44 + 24*numAPIDs);
    }
 
    offset = 148;
 
    for (cnt=0; cnt<(int)numAPIDs; cnt++)
    {
        eosamFileHdr[offset+1] = scID;
        eosamFileHdr[offset+2] = appID[cnt]/256;
        eosamFileHdr[offset+3] = appID[cnt]%256;
        pkt_offset = 0;
        for (cnt2=1;cnt2<cnt;cnt2++)
        {
            pkt_offset += dataLength[cnt2-1] + (PGSd_IO_L0_PrimaryPktHdrSize +
                                                PGSd_IO_L0_SecPktHdrSizeEOS_AURA);
        }
        eosamFileHdr[offset+8] = pkt_offset/0x1000000;
        eosamFileHdr[offset+9] = (pkt_offset/0x10000)%0x100;
        eosamFileHdr[offset+10] = (pkt_offset/0x100)%0x100;
        eosamFileHdr[offset+11] = pkt_offset%0x100;
        eosamFileHdr[offset+15] = 1;
        eosamFileHdr[offset+18] = scID/64;
        eosamFileHdr[offset+19] = scID%64 << 2;
        PGS_TD_TAItoUTC(startTAI93+cnt*timeInterval, asciiUTC);
        PGS_TD_UTCtoEOSAURAGIRD(asciiUTC, eosauraSecHdr.scTime);
        PGS_TD_EOSAURAtoUTC(eosauraSecHdr.scTime,asciiUTCtemp);
        PGS_TD_UTCtoEOSAURAGIIS(asciiUTCtemp, eosamFileHdr+(offset+40));
        returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
        if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
        {
            onLeap = PGS_TRUE;
        }
        else
        {
            onLeap = PGS_FALSE;
        }
        PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+57));
 
        PGS_TD_TAItoUTC(startTAI93+(numPkts-numAPIDs+cnt)*timeInterval,
                        asciiUTC);
        PGS_TD_UTCtoEOSAURAGIRD(asciiUTC, eosauraSecHdr.scTime);
        PGS_TD_EOSAURAtoUTC(eosauraSecHdr.scTime,asciiUTCtemp);
        PGS_TD_UTCtoEOSAURAGIIS(asciiUTCtemp, eosamFileHdr+(offset+48));
        returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
        if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
        {
            onLeap = PGS_TRUE;
        }
        else
        {
            onLeap = PGS_FALSE;
        }
        PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+65));
 
        numThisAPID = numPkts/numAPIDs;
        if ((cnt+1) <= numPkts%numAPIDs)
        {
            numThisAPID += 1;
        }
        sizeThisAPID = numThisAPID*(dataLength[cnt]+PGSd_IO_L0_PrimaryPktHdrSize
                                    +PGSd_IO_L0_SecPktHdrSizeEOS_AURA);
 
 
        eosamFileHdr[offset+76] = numThisAPID/0x1000000;
        eosamFileHdr[offset+77] = (numThisAPID/0x10000)%0x100;
        eosamFileHdr[offset+78] = (numThisAPID/0x100)%0x100;
        eosamFileHdr[offset+79] = numThisAPID%0x100;
 
        eosamFileHdr[offset+84] = sizeThisAPID/0x1000000;
        eosamFileHdr[offset+85] = (sizeThisAPID/0x10000)%0x100;
        eosamFileHdr[offset+86] = (sizeThisAPID/0x100)%0x100;
        eosamFileHdr[offset+87] = sizeThisAPID%0x100;
 
        offset += 96;
    }

 
    hdr_file_ptr = fopen(filename, "w");
    fwrite(eosamFileHdr, hdr_size, 1, hdr_file_ptr);
    fclose(hdr_file_ptr);
    free(eosamFileHdr);
    return PGS_S_SUCCESS;
    }

    /* header for EOS_PM_GIIS */

    if (spacecraftTag == PGSd_EOS_PM_GIIS)
    {
	
    hdr_size = 152 + 96*numAPIDs + (44 + 24*numAPIDs)*(numFiles);
    eosamFileHdr = (unsigned char*) calloc(1,hdr_size);
    eosamFileHdr[2] = 1;
    eosamFileHdr[40] = 1;
    eosamFileHdr[51] = 1;

    memcpy((void*) (eosamFileHdr+4), (void*) filename, 36);
    
    returnStatus = PGS_TD_TAItoUTCjd(startTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
	onLeap = PGS_TRUE;
    }
    else
    {
	onLeap = PGS_FALSE;
    }    
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+53);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+97);
    if (returnStatus != PGS_S_SUCCESS &&
	returnStatus != PGSTD_W_PRED_LEAPS)
    {
	free(eosamFileHdr);
	return returnStatus;
    }
    
    returnStatus = PGS_TD_TAItoUTCjd(stopTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
	onLeap = PGS_TRUE;
    }
    else
    {
	onLeap = PGS_FALSE;
    }
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+61);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+105);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+133);
    if (returnStatus != PGS_S_SUCCESS &&
	returnStatus != PGSTD_W_PRED_LEAPS)
    {
	free(eosamFileHdr);
	return returnStatus;
    }
    
    PGS_TD_TAItoUTC(startTAI93, asciiUTC);
    PGS_TD_UTCtoEOSPMGIIS(asciiUTC, eosamFileHdr+80);
    PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
    PGS_TD_UTCtoEOSPMGIIS(asciiUTC, eosamFileHdr+88);

    memcpy((void*)(eosamFileHdr+97), (void*)(eosamFileHdr+53), 7);
    memcpy((void*)(eosamFileHdr+105), (void*)(eosamFileHdr+61), 7);
    
    eosamFileHdr[116] = numPkts/0x1000000;
    eosamFileHdr[117] = (numPkts/0x10000)%0x100;
    eosamFileHdr[118] = (numPkts/0x100)%0x100;
    eosamFileHdr[119] = numPkts%0x100;
    
    eosamFileHdr[124] = sizePDS/0x1000000;
    eosamFileHdr[125] = (sizePDS/0x10000)%0x100;
    eosamFileHdr[126] = (sizePDS/0x100)%0x100;
    eosamFileHdr[127] = sizePDS%0x100;

    memcpy((void*)(eosamFileHdr+133), (void*)(eosamFileHdr+61), 7);

    eosamFileHdr[147] = numAPIDs;
    
    eosamFileHdr[151+numAPIDs*96] = numFiles;
    
    /* tricky new stuff */

    granuleSize = numPkts/(numFiles-1);

    offset = 152 + 96*numAPIDs;
    
    for (cnt=0; cnt<(int)numFiles; cnt++)
    {
	memcpy((void*)(eosamFileHdr+offset), (void*)(filename), 36);
	sprintf((char*) (eosamFileHdr+offset+34), "%02d.PDS", cnt);
	if (cnt != 0)
	{
	    filePkts = (cnt == numFiles) ? numPkts-(cnt-1)*granuleSize :
	                                   granuleSize;
	    eosamFileHdr[offset+43] = numAPIDs;
	    beginAPID = granuleSize*(cnt-1);
	    endAPID = beginAPID + filePkts - numAPIDs;
	    for (cnt2=0;cnt2<(int)numAPIDs;cnt2++)
	    {
		eosamFileHdr[offset+45+(24*cnt2)] = scID;
		eosamFileHdr[offset+46+(24*cnt2)] = appID[beginAPID]/256;
		eosamFileHdr[offset+47+(24*cnt2)] = appID[cnt2]%256;
		PGS_TD_TAItoUTC(startTAI93+(beginAPID+cnt2)*timeInterval,
				asciiUTC);
		PGS_TD_UTCtoEOSPMGIIS(asciiUTC, eosamFileHdr+(offset+48+(24*cnt2)));
		PGS_TD_TAItoUTC(startTAI93+(endAPID+cnt2)*timeInterval,
				asciiUTC);
		PGS_TD_UTCtoEOSPMGIIS(asciiUTC, eosamFileHdr+(offset+56+(24*cnt2)));
	    }
	}
	else
	{
	    numAPIDs = 1;
	}
	
	offset += (44 + 24*numAPIDs);
    }
    
    offset = 148;
    
    for (cnt=0; cnt<(int)numAPIDs; cnt++)
    {
	eosamFileHdr[offset+1] = scID;
	eosamFileHdr[offset+2] = appID[cnt]/256;
	eosamFileHdr[offset+3] = appID[cnt]%256;
	pkt_offset = 0;
	for (cnt2=1;cnt2<cnt;cnt2++)
	{
	    pkt_offset += dataLength[cnt2-1] + (PGSd_IO_L0_PrimaryPktHdrSize +
						PGSd_IO_L0_SecPktHdrSizeEOS_PM);
	}
	eosamFileHdr[offset+8] = pkt_offset/0x1000000;
	eosamFileHdr[offset+9] = (pkt_offset/0x10000)%0x100;
	eosamFileHdr[offset+10] = (pkt_offset/0x100)%0x100;
	eosamFileHdr[offset+11] = pkt_offset%0x100;
	eosamFileHdr[offset+15] = 1;
	eosamFileHdr[offset+18] = scID/64;
	eosamFileHdr[offset+19] = scID%64 << 2;
	PGS_TD_TAItoUTC(startTAI93+cnt*timeInterval, asciiUTC);
	PGS_TD_UTCtoEOSPMGIIS(asciiUTC, eosamFileHdr+(offset+40));
	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	{
	    onLeap = PGS_TRUE;
	}
	else
	{
	    onLeap = PGS_FALSE;
	}
	PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+57));
	
	PGS_TD_TAItoUTC(startTAI93+(numPkts-numAPIDs+cnt)*timeInterval, 
			asciiUTC);
	PGS_TD_UTCtoEOSPMGIIS(asciiUTC, eosamFileHdr+(offset+48));
	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	{
	    onLeap = PGS_TRUE;
	}
	else
	{
	    onLeap = PGS_FALSE;
	}
	PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+65));

	numThisAPID = numPkts/numAPIDs;
	if ((cnt+1) <= numPkts%numAPIDs)
	{
	    numThisAPID += 1;
	}
	sizeThisAPID = numThisAPID*(dataLength[cnt]+PGSd_IO_L0_PrimaryPktHdrSize
				    +PGSd_IO_L0_SecPktHdrSizeEOS_PM);

	
	eosamFileHdr[offset+76] = numThisAPID/0x1000000;
	eosamFileHdr[offset+77] = (numThisAPID/0x10000)%0x100;
	eosamFileHdr[offset+78] = (numThisAPID/0x100)%0x100;
	eosamFileHdr[offset+79] = numThisAPID%0x100;

	eosamFileHdr[offset+84] = sizeThisAPID/0x1000000;
	eosamFileHdr[offset+85] = (sizeThisAPID/0x10000)%0x100;
	eosamFileHdr[offset+86] = (sizeThisAPID/0x100)%0x100;
	eosamFileHdr[offset+87] = sizeThisAPID%0x100;

	offset += 96;
    }
    
    hdr_file_ptr = fopen(filename, "w");
    fwrite(eosamFileHdr, hdr_size, 1, hdr_file_ptr);
    fclose(hdr_file_ptr);
    free(eosamFileHdr);
    return PGS_S_SUCCESS;
    }



    /* header for EOS_PM_GIRD */
    /* Note: before changing UTC times to GIIS and writing to header file,
       we have converted UTC to GIRD and from GIRD to UTC again.This will take 
       care of rounding problem that made first packet's time off by a few
       microsecond from the corresponding GISS time that is written to the
       header file */

    if (spacecraftTag == PGSd_EOS_PM_GIRD)
    {
    hdr_size = 152 + 96*numAPIDs + (44 + 24*numAPIDs)*(numFiles);
    eosamFileHdr = (unsigned char*) calloc(1,hdr_size);
    eosamFileHdr[2] = 1;
    eosamFileHdr[40] = 1;
    eosamFileHdr[51] = 1;

    memcpy((void*) (eosamFileHdr+4), (void*) filename, 36);
    
    returnStatus = PGS_TD_TAItoUTCjd(startTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
	onLeap = PGS_TRUE;
    }
    else
    {
	onLeap = PGS_FALSE;
    }    
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+53);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+97);
    if (returnStatus != PGS_S_SUCCESS &&
	returnStatus != PGSTD_W_PRED_LEAPS)
    {
	free(eosamFileHdr);
	return returnStatus;
    }
    
    returnStatus = PGS_TD_TAItoUTCjd(stopTAI93, jdUTC);
    if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
    {
	onLeap = PGS_TRUE;
    }
    else
    {
	onLeap = PGS_FALSE;
    }
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+61);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+105);
    returnStatus = PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+133);
    if (returnStatus != PGS_S_SUCCESS &&
	returnStatus != PGSTD_W_PRED_LEAPS)
    {
	free(eosamFileHdr);
	return returnStatus;
    }
    
    PGS_TD_TAItoUTC(startTAI93, asciiUTC);
    PGS_TD_UTCtoEOSPMGIRD(asciiUTC, eospmSecHdr2.scTime);
    PGS_TD_EOSPMGIRDtoUTC(eospmSecHdr2.scTime,asciiUTCtemp);
    PGS_TD_UTCtoEOSPMGIIS(asciiUTCtemp, eosamFileHdr+80);
    PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
    PGS_TD_UTCtoEOSPMGIRD(asciiUTC, eospmSecHdr2.scTime);
    PGS_TD_EOSPMGIRDtoUTC(eospmSecHdr2.scTime,asciiUTCtemp);
    PGS_TD_UTCtoEOSPMGIIS(asciiUTCtemp, eosamFileHdr+88);

    memcpy((void*)(eosamFileHdr+97), (void*)(eosamFileHdr+53), 7);
    memcpy((void*)(eosamFileHdr+105), (void*)(eosamFileHdr+61), 7);
    
    eosamFileHdr[116] = numPkts/0x1000000;
    eosamFileHdr[117] = (numPkts/0x10000)%0x100;
    eosamFileHdr[118] = (numPkts/0x100)%0x100;
    eosamFileHdr[119] = numPkts%0x100;
    
    eosamFileHdr[124] = sizePDS/0x1000000;
    eosamFileHdr[125] = (sizePDS/0x10000)%0x100;
    eosamFileHdr[126] = (sizePDS/0x100)%0x100;
    eosamFileHdr[127] = sizePDS%0x100;

    memcpy((void*)(eosamFileHdr+133), (void*)(eosamFileHdr+61), 7);

    eosamFileHdr[147] = numAPIDs;
    
    eosamFileHdr[151+numAPIDs*96] = numFiles;
    
    /* tricky new stuff */

    granuleSize = numPkts/(numFiles-1);

    offset = 152 + 96*numAPIDs;
    
    for (cnt=0; cnt<(int)numFiles; cnt++)
    {
	memcpy((void*)(eosamFileHdr+offset), (void*)(filename), 36);
	sprintf((char*) (eosamFileHdr+offset+34), "%02d.PDS", cnt);
	if (cnt != 0)
	{
	    filePkts = (cnt == numFiles) ? numPkts-(cnt-1)*granuleSize :
	                                   granuleSize;
	    eosamFileHdr[offset+43] = numAPIDs;
	    beginAPID = granuleSize*(cnt-1);
	    endAPID = beginAPID + filePkts - numAPIDs;
	    for (cnt2=0;cnt2<(int)numAPIDs;cnt2++)
	    {
		eosamFileHdr[offset+45+(24*cnt2)] = scID;
		eosamFileHdr[offset+46+(24*cnt2)] = appID[beginAPID]/256;
		eosamFileHdr[offset+47+(24*cnt2)] = appID[cnt2]%256;
		PGS_TD_TAItoUTC(startTAI93+(beginAPID+cnt2)*timeInterval,
				asciiUTC);
		PGS_TD_UTCtoEOSPMGIRD(asciiUTC, eospmSecHdr2.scTime);
		PGS_TD_EOSPMGIRDtoUTC(eospmSecHdr2.scTime,asciiUTCtemp);
		PGS_TD_UTCtoEOSPMGIIS(asciiUTCtemp, eosamFileHdr+(offset+48+(24*cnt2)));
		PGS_TD_TAItoUTC(startTAI93+(endAPID+cnt2)*timeInterval,
				asciiUTC);
		PGS_TD_UTCtoEOSPMGIRD(asciiUTC, eospmSecHdr2.scTime);
		PGS_TD_EOSPMGIRDtoUTC(eospmSecHdr2.scTime,asciiUTCtemp);
		PGS_TD_UTCtoEOSPMGIIS(asciiUTCtemp, eosamFileHdr+(offset+56+(24*cnt2)));
	    }
	}
	else
	{
	    numAPIDs = 1;
	}
	
	offset += (44 + 24*numAPIDs);
    }
    
    offset = 148;
    
    for (cnt=0; cnt<(int)numAPIDs; cnt++)
    {
	eosamFileHdr[offset+1] = scID;
	eosamFileHdr[offset+2] = appID[cnt]/256;
	eosamFileHdr[offset+3] = appID[cnt]%256;
	pkt_offset = 0;
	for (cnt2=1;cnt2<cnt;cnt2++)
	{
	    pkt_offset += dataLength[cnt2-1] + (PGSd_IO_L0_PrimaryPktHdrSize +
						PGSd_IO_L0_SecPktHdrSizeEOS_PM);
	}
	eosamFileHdr[offset+8] = pkt_offset/0x1000000;
	eosamFileHdr[offset+9] = (pkt_offset/0x10000)%0x100;
	eosamFileHdr[offset+10] = (pkt_offset/0x100)%0x100;
	eosamFileHdr[offset+11] = pkt_offset%0x100;
	eosamFileHdr[offset+15] = 1;
	eosamFileHdr[offset+18] = scID/64;
	eosamFileHdr[offset+19] = scID%64 << 2;
	PGS_TD_TAItoUTC(startTAI93+cnt*timeInterval, asciiUTC);
	PGS_TD_UTCtoEOSPMGIRD(asciiUTC, eospmSecHdr2.scTime);
	PGS_TD_EOSPMGIRDtoUTC(eospmSecHdr2.scTime,asciiUTCtemp);
	PGS_TD_UTCtoEOSPMGIIS(asciiUTCtemp, eosamFileHdr+(offset+40));
	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	{
	    onLeap = PGS_TRUE;
	}
	else
	{
	    onLeap = PGS_FALSE;
	}
	PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+57));
	
	PGS_TD_TAItoUTC(startTAI93+(numPkts-numAPIDs+cnt)*timeInterval, 
			asciiUTC);
	PGS_TD_UTCtoEOSPMGIRD(asciiUTC, eospmSecHdr2.scTime);
	PGS_TD_EOSPMGIRDtoUTC(eospmSecHdr2.scTime,asciiUTCtemp);
	PGS_TD_UTCtoEOSPMGIIS(asciiUTCtemp, eosamFileHdr+(offset+48));
	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	{
	    onLeap = PGS_TRUE;
	}
	else
	{
	    onLeap = PGS_FALSE;
	}
	PGS_TD_UTCjdtoPB5C(jdUTC, onLeap, eosamFileHdr+(offset+65));

	numThisAPID = numPkts/numAPIDs;
	if ((cnt+1) <= numPkts%numAPIDs)
	{
	    numThisAPID += 1;
	}
	sizeThisAPID = numThisAPID*(dataLength[cnt]+PGSd_IO_L0_PrimaryPktHdrSize
				    +PGSd_IO_L0_SecPktHdrSizeEOS_PM);

	
	eosamFileHdr[offset+76] = numThisAPID/0x1000000;
	eosamFileHdr[offset+77] = (numThisAPID/0x10000)%0x100;
	eosamFileHdr[offset+78] = (numThisAPID/0x100)%0x100;
	eosamFileHdr[offset+79] = numThisAPID%0x100;

	eosamFileHdr[offset+84] = sizeThisAPID/0x1000000;
	eosamFileHdr[offset+85] = (sizeThisAPID/0x10000)%0x100;
	eosamFileHdr[offset+86] = (sizeThisAPID/0x100)%0x100;
	eosamFileHdr[offset+87] = sizeThisAPID%0x100;

	offset += 96;
    }
    
    hdr_file_ptr = fopen(filename, "w");
    fwrite(eosamFileHdr, hdr_size, 1, hdr_file_ptr);
    fclose(hdr_file_ptr);
    free(eosamFileHdr);
    return PGS_S_SUCCESS;
    }
}
else
 {
   returnStatus = PGSIO_E_L0_BAD_SPACECRAFT_TAG;
   return returnStatus;
 }
  return returnStatus;
}
