/*
*
* Description: This program takes OrbSim output (24 hours) as input, 
*              calls Toolkit function PGS_EPH_EphemAttit to get 
*              spacecraft position, velocity, attitude body rate, and
*              attitude quaternion, fill these values (every second)
*              and status word 2 value (every 8 seconds) into 12 2-hour
*              carryout files that will be used to perform DPREP testing. 
*
* History:
*     Date        name         short note
*     ----------  -----------  -----------
*     2000Mar17   swang        creation
*     2000Apr22   pnoerdli     enhanced for table read and IRU time tag
*     2000July    pnoerdli     altered order of quaternion components, etc
*                              for compatibility with Aqua standards
*     2000Sept26  pnoerdli     changed spacecraft tag to EOSPM1 (from PM_GIIS)
*                              for compatibility with new Toolkit standards
*     2000Nov13   pnoerdli     fixed to avoid negative IRU time tag seconds
*                              field
*     2001Sep05   kummerer     change status word 2 fine-pointing from 6 to
*                              24576
*     2001Dec04   kummerer     change sign of quaternion 4 to match EMOS
*                              definition
*     2002Mar08   kummerer     change PGS_EPH_EphemAttit call from numOffsets-1
*                              to numOffsets to avoid quarternions from not
*                              being filled at the end of the 24-hour simulation
*     2002Sep05   kummerer     allow for GBAD modes other than fine-pointing;
*                              perform both Aqua and Aura attitude simulations
*                              under a single simulator.
*     2002Sep26   kummerer     correct error in BDYRATETIN calculation.
*
*/

#include <stdio.h>
#include <math.h>
#include <PGS_TD.h>
#include <PGS_TD_Prototypes.h>
#include <PGS_EPH.h>
#include <string.h>
#define  EPOCH_TIME         1104537627.0  /* (julian date 12AM 1/1/1993) -
                                             (julian date 12AM 1/1/1958)
                                             in seconds */

/*  orbsim output of 24 hours 1.024 second interval contains 84375 records. 
    orbsim output of 24 hours 1.000 second interval contains 86400 records. 
*/
#define ARRAY_SIZE    86402
#define ASCII_UTC_SIZE 28     /* Format YYYY-MM-DDTHH:MM:SS.ssssssZ */
#define FINE_POINT 3          /* fine point value for status word 2: */

/*
* number of headers in carry-out file. 
*   if this #define changed, 2 places must be changed corrspondingly.
*     1. gHeaderRecords definition
*     2. gMnemonic definition
*/
#define NB_HEADERS  16
#define NB_GBADEVENTS 64

/*
* some max length, see carryout file Format. 
*/
#define LEN_MNEMONIC        20

/* 
* global data structure definition.
*/
struct {
  double  D1_offset;
  short   D2_emosID;
  short   D3_pdbID;
  char    D4_dummy;   /* not used by us */
  double  D5_double;
  int     D5_integer; /* will hold Status Word 2 value. */
  char    D6_dummy;   /* not used by us */ 
  unsigned D7_statWd;
} gDataRecord;


enum { PosX, PosY, PosZ,
       VeloX, VeloY, VeloZ,
       Quat1, Quat2, Quat3, Quat4,
       IRUtag1,IRUtag2,
       AttRateX, AttRateY, AttRateZ, StatWd2
} gMnemonic;


struct {
  char   H3_mnemonic[LEN_MNEMONIC + 1];
  short  H4_emosID;
  short  H5_pdbID;
  char   H6_type;

} gHeaderRecords[NB_HEADERS] = {

   /*  default  emosID PDB ID Data */
   /*  mnemonic               Type */
   /* --------  ------ ----- ------*/ 
    "PositionX",     1, 101, 'R', 
    "PositionY",     2, 102, 'R',   
    "PositionZ",     3, 103, 'R',
    "VelocityX",     4, 104, 'R',
    "VelocityY",     5, 105, 'R',
    "VelocityZ",     6, 106, 'R',
    "Quaternion0",   7, 107, 'R',
    "Quaternion1",   8, 108, 'R',
    "Quaternion2",   9, 109, 'R',
    "Quaternion3",  10, 110, 'R',
    "IRUtimeTag1",  11, 111, 'R',
    "IRUtimeTag2",  12, 112, 'R',
    "AttitudeRateX",13, 113, 'R',
    "AttitudeRateY",14, 114, 'R',
    "AttitudeRateZ",15, 115, 'R',
    "StatusWord2",  16, 116, 'I'
   /* --------  ------ ----- -------- */
};

struct {
  double offset;
  int    attMode;
  int    gbadMode;
} gbadEvents[NB_GBADEVENTS];

/*
* utility function prototype.
*/
void print_header_records(FILE *);
void print_data_record(FILE *);
void print_data_record_statWd2(FILE *);
void print_secFrom1958(FILE *);
void time_converter(double, char*);

/*----------------- Main -----------------*/
int main (int argc, char *argv[])
{
  /*-------------- Local Variable Declaration ----------------*/

  /* variables used by function: PGS_EPH_EphemAttit */
  char            asciiUTC[ASCII_UTC_SIZE];
  char            cFilenameTime[ASCII_UTC_SIZE];
  char            templateTime[ASCII_UTC_SIZE] = "1993-01-01T00:00:00.000000Z\0";
  char            transTime[ASCII_UTC_SIZE];
  char            platformId[5];
  char            hours[3];
  PGSt_double     offsets[ARRAY_SIZE];
  PGSt_integer    qualityFlags[ARRAY_SIZE][2];
  PGSt_double     positionECI[ARRAY_SIZE][3];
  PGSt_double     velocityECI[ARRAY_SIZE][3];
  PGSt_double     eulerAngles[ARRAY_SIZE][3];
  PGSt_double     xyzRotRates[ARRAY_SIZE][3];
  PGSt_double     attitQuat[ARRAY_SIZE][4];
  PGSt_double     cFilenameBaseTime;
  PGSt_double     segmentDuration = 7200;     /* segment duration in seconds */
  PGSt_double     transTAITime;
  PGSt_tag        spacecraftTag;
  PGSt_SMF_status returnStatus;
  /* end of variables used by function: PGS_EPH_EphemAttit */

  int    numOffsets,numCheck;
  char   cFilename[ASCII_UTC_SIZE], cFilenameRoot[ASCII_UTC_SIZE];
  int    iFilenameExt;
  PGSt_double qSIGN; /* QUATERNION   */
  PGSt_double scrat; /*  scratch item for seconds fluctuation */

  char   CarryoutTime[ASCII_UTC_SIZE];
  double TAITime, BaseTAITime,seconds;

  double TwoHoursBoundary, TwoHoursBoundaryOld;
  double tempDouble,interval,temP;
  PGSt_SMF_status code;             /* returned from PGS_SMF_GetMsg() */

  char mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* error mnemonic */
  char msg[PGS_SMF_MAX_MSG_SIZE];           /* error message */

  unsigned int numTries;        /* number of times the user has attempted
                                   to input something */

  FILE   *CarryoutFile;
  FILE   *fp;
  int    i,j, iCtr;
  char   scratchC[10];
  char   inputBuffer[100];    /* input buffer */
  char   *newlinePtr;         /* pointer to newline character in the
                                         input buffer */
 
  PGSt_boolean gotData;             /* used to determine if an input value as
                                       been successfully ingested */

  int             lengthC;

  int timeSpecMode;
  int gbadMode, gbadIndex;
  int attMode, prevAttMode;
  short bitsStatusWord;
  double offset;

  /*-------------- End of Variable Declaration ----------------*/

  /* parse the command line */
  /* check number of arguments in command line */
  if (argc != 3)
  {
    printf(" Usage:   %s PLATFORM YYYY-MM-DDTHH:MM:SS.ddddddZ\n\n", argv[0]);
    printf("    *** PLATFORM must either be Aqua or Aura.             ***\n");
    printf("    *** Provide the simulation start time in this format! ***\n\n");
    printf(" Example: %s Aqua 1998-02-05T13:05:45.341250Z\n", argv[0]);
    exit(1);
  }

  /* get spacecraft ID */
  strcpy(platformId, argv[1]);
  if (strcmp(platformId,"Aqua") != 0 && strcmp(platformId,"Aura") != 0)
  {
    printf(" Usage:   %s PLATFORM YYYY-MM-DDTHH:MM:SS.ddddddZ\n\n", argv[0]);
    printf("    *** PLATFORM must either be Aqua or Aura.             ***\n");
    printf("    *** Provide the simulation start time in this format! ***\n\n");
    printf(" Example: %s Aqua 1998-02-05T13:05:45.341250Z\n", argv[0]);
    exit(1);
  }

  /* get asciiUTC time */
  strcpy(asciiUTC, argv[2]);
  strncat(asciiUTC, &templateTime[strlen(asciiUTC)], strlen(templateTime)-strlen(asciiUTC));
  printf(" Simulation start time: %s\n\n",asciiUTC);
  returnStatus = PGS_TD_UTCtoTAI(asciiUTC, &BaseTAITime);
  if (returnStatus != PGS_S_SUCCESS)
  {
     printf("PGS_TD_UTCtoTAI error: %d\n", returnStatus);
     exit(1);
  }

  /* parse an initial time offset */
  numTries=0;
  gotData = PGS_FALSE;
  printf(" To offset the initial time from the whole hour\n");
  printf(" enter a floating point number of seconds < 1.00\n");
  printf(" Else enter a carriage return to continue or a 'q' to quit. \n");
  do 
  {
     fgets(inputBuffer,99,stdin);
     if (inputBuffer[0] == 'q')
       exit(0);
     newlinePtr = strchr(inputBuffer,'\n');
     if (newlinePtr != NULL)
       *newlinePtr = '\0';
         
     if (inputBuffer[0] != '\n')
     {
        numCheck = sscanf(inputBuffer,"%le",&tempDouble);
        if (numCheck == 0) continue;
        seconds = tempDouble;
     }

     if(seconds >= 0.0 && seconds < 1.0)
     {
        gotData = PGS_TRUE;
        break;
     }
     else
     {
        printf(" To offset the initial time from the whole hour\n");
        printf(" enter a floating point number of seconds < 1.00\n");
        printf(" Else enter a carriage return to continue or a 'q' to quit. \n");
     }
  }
  while (gotData == PGS_FALSE && ++numTries < 4);
  if (gotData == PGS_FALSE) exit(0);
  printf(" An initial time offset of %f seconds used.\n\n",seconds);

  /* parse GBAD mnemonics name table */
  interval = 1.000;

  if( (fp = fopen("NameTable.txt","r")) == NULL)
  {
     printf(" *** No NameTable.txt input file found; default mnemonics used. ***\n");
  }
  else
  {
     for(j=0;j<NB_HEADERS;j++)
     {
        fscanf(fp,"%s",gHeaderRecords[j].H3_mnemonic);
        fscanf(fp,"%hd",&gHeaderRecords[j].H4_emosID);
        fscanf(fp,"%hd",&gHeaderRecords[j].H5_pdbID);
        fscanf(fp,"%s",scratchC);
        lengthC = strlen(scratchC);
        gHeaderRecords[j].H6_type= scratchC[lengthC-1];
     }
     fclose(fp);
  }

  /* parse the attitude mode transition table */
  gbadIndex = 0;
  for(j=0;j<NB_GBADEVENTS;j++)
  {
     gbadEvents[j].offset = 99999;
     gbadEvents[j].attMode = FINE_POINT;
     gbadEvents[j].gbadMode = FINE_POINT << 13;
  }
  attMode = FINE_POINT;
  gbadMode = FINE_POINT << 13;

  /* only Aura simulation uses the attitude mode transition table */
  if (strcmp(platformId,"Aura") == 0)
  {
     if( (fp = fopen("GbadMode.txt","r")) == NULL)
     {
        printf(" *** No GbadMode.txt input file found; fine-pointing used throughout simulation. ***\n");
        for(j=0;j<NB_GBADEVENTS;j++)
        {
           gbadEvents[j].offset = 99999;
           gbadEvents[j].attMode = FINE_POINT;
           gbadEvents[j].gbadMode = FINE_POINT << 13;
        }
        attMode = FINE_POINT;
        gbadMode = FINE_POINT << 13;
     }
     else
     {
        /* initialize */
        attMode = FINE_POINT;
        gbadMode = FINE_POINT << 13;
        for(j=0;j<NB_GBADEVENTS;j++)
        {
           gbadEvents[j].offset = 99999;
           gbadEvents[j].attMode = FINE_POINT;
           gbadEvents[j].gbadMode = FINE_POINT;
        }

        /* parse time entry mode */
        fscanf(fp,"%i",&timeSpecMode);

        /* transition times specified as offsets */
        if (timeSpecMode == 0)
        {
           j=0;
           while ( j<NB_GBADEVENTS && feof(fp)==0 )
           {
              fscanf(fp,"%le",&offset);
              if ( feof(fp)==0 )
                fscanf(fp,"%hd",&bitsStatusWord);
              if ( feof(fp)==0 )
              {
                 gbadEvents[j].offset = offset + seconds;
                 gbadEvents[j].attMode = bitsStatusWord;
                 j++;
              }
           }
        }

        /* transition times specified as timestamps */
        else if (timeSpecMode == 1)
        {
           j=0;
           while ( j<NB_GBADEVENTS && feof(fp)==0 )
           {
              fscanf(fp,"%s",transTime);
              if (feof(fp)==0)
                fscanf(fp,"%hd",&bitsStatusWord);
              if (feof(fp)==0)
              {
                 returnStatus = PGS_TD_UTCtoTAI(transTime, &transTAITime);
                 if (returnStatus != PGS_S_SUCCESS)
                 {
                    printf("PGS_TD_UTCtoTAI error: %d\n", returnStatus);
                    exit(1);
                 }
                 gbadEvents[j].offset = (transTAITime - BaseTAITime) + seconds;
                 gbadEvents[j].attMode = bitsStatusWord;
                 j++;
              }
           }
        }

        /* unrecognized time entry format */
        else
        {
           printf(" *** Unrecognized attitude mode transition time entry format. ***\n");
           exit(1);
        }
        fclose(fp);

        /* convert attitude mode to the encoded attitude mode (GBAD formatted) */
        for(j=0;j<NB_GBADEVENTS;j++)
        {
           gbadEvents[j].gbadMode = gbadEvents[j].attMode << 13;
        }

        /* display attitude mode transition events */
        printf(" Requested attitude mode transition events:\n");
        j=0;
        while ( j<NB_GBADEVENTS && gbadEvents[j].offset != 99999 )
        {
           TAITime = BaseTAITime + gbadEvents[j].offset;
           PGS_TD_TAItoUTC(TAITime, transTime);
           printf("   Offset Time: %10.3f    UTC Time: %s    Attitude Mode: %i    Encoded Attitude Mode: %i\n",gbadEvents[j].offset,transTime,gbadEvents[j].attMode,gbadEvents[j].gbadMode);
           j++;
        }
        printf("\n");
     }
  }

  /*
  * initialize offsets. Note that we want 1.024 second offset.
  */
  i=0;
  offsets[0]=seconds;
  while(offsets[i]  < 86400.0 && i < 86400 ) 
  {
    offsets[i+1] = seconds + interval * (PGSt_double) (i+1);
    i++;
  }
  numOffsets = i;

  /* 
   * Call function PGS_EPH_EphemAttit to do conversion.
   * To use this function, Toolkit environment should be set up,
   * also, a PCF should be created, so that toolkit can be used 
   * and input data can be read in. 
   */
  printf(" Exercising PGS_EPH_EphemAttit()\n");
  printf(" If no response in 2 minutes, do Ctrl-C and examine LogStatus file.\n");

  if (strcmp(platformId,"Aqua") == 0)
  {
     spacecraftTag = PGSd_EOS_PM1;   /* this is a PM1 specific */
  }
  else if (strcmp(platformId,"Aura") == 0)
  {
     spacecraftTag = PGSd_EOS_PM1;   /* this is a PM1 specific */
/*     spacecraftTag = PGSd_EOS_AURA;   this is a AURA specific */
  }

  returnStatus = PGS_EPH_EphemAttit(spacecraftTag,numOffsets,asciiUTC,
                                    offsets,PGS_TRUE,PGS_TRUE,
                                    qualityFlags,positionECI,velocityECI,
                                    eulerAngles,xyzRotRates,attitQuat);

  /*
  returnStatus = PGS_EPH_EphemAttit(PGSd_EOS_PM1,numOffsets-1,asciiUTC, 2002Mar08
  returnStatus = PGS_EPH_EphemAttit(PGSd_EOS_PM1,numOffsets-2,asciiUTC,
   */
   
  if (returnStatus != PGS_S_SUCCESS)
  {
     PGS_SMF_GetMsg(&code,mnemonic,msg);
     if (code != returnStatus)
       PGS_SMF_GetMsgByCode(returnStatus,msg);

     printf(" *** PGS_EPH_EphemAttit error. Toolkit return status is %d. ***\n", returnStatus);
     exit(1);
  }
  else
  {
     printf(" Toolkit OK. Toolkit return status is %d.\n\n", returnStatus);
  }

  /* We used numOffsets-1 so as not to run to end of orbsim data; now extrapolate! */

  for(i=0;i<3;i++)
  {
     positionECI[numOffsets][i] = 2.0*positionECI[numOffsets-1][i] - positionECI[numOffsets-2][i];
     velocityECI[numOffsets][i] = 2.0*velocityECI[numOffsets-1][i] - velocityECI[numOffsets-2][i];
  }
  for(i=0;i<4;i++)
  {
     attitQuat[numOffsets][i] = 2.0*attitQuat[numOffsets-1][i] - attitQuat[numOffsets-2][i];
  }
  scrat = 0.0;
  for(i=0;i<4;i++)
  {
    scrat+= attitQuat[numOffsets][i]* attitQuat[numOffsets][i];
  }
  scrat = sqrt(scrat);
  for(i=0;i<4;i++)
  {
    attitQuat[numOffsets][i] /= scrat;
  }
  /*
   * take year-month-date-hour part of asciiUTC  as carryout file name root.
   */
   strncpy(cFilenameTime, asciiUTC, 13);
   strncat(cFilenameTime, &templateTime[strlen(cFilenameTime)], strlen(templateTime)-strlen(cFilenameTime));
   PGS_TD_UTCtoTAI(cFilenameTime, &cFilenameBaseTime);
 
  /*
   * initialization 
   */
  TwoHoursBoundary = seconds;

  /*
   * fill in carryout files.
   */
  for (iCtr = 0; iCtr < numOffsets; iCtr++)
  {
    /* fill in header part. */
    if (offsets[iCtr] >= TwoHoursBoundary)
    {
      fclose(CarryoutFile);

      PGS_TD_TAItoUTC(cFilenameBaseTime, cFilenameTime);
      strcpy(cFilenameRoot, cFilenameTime);
      for (i=0; cFilenameRoot[i] != 'T'; i++);
      cFilenameRoot[i] = '\0';
      strncpy(hours, &cFilenameTime[11], 2);
      iFilenameExt = atoi(hours);

      cFilenameBaseTime = cFilenameBaseTime + segmentDuration;

      /* get new carryout file name */
      sprintf(cFilename, "%s.%02d", cFilenameRoot, iFilenameExt);
      CarryoutFile = fopen(cFilename, "w");

      printf(" Filling %s GBAD simulation granule %s.\n", platformId,cFilename);

      /* fill in Item H1 of carryout file */
      TAITime = BaseTAITime + offsets[iCtr];
      time_converter(TAITime, CarryoutTime);
      fprintf(CarryoutFile, "%s\n", CarryoutTime);

      /* fill in Items H2 to H6 of carryout file */
      print_header_records(CarryoutFile);

      /* reset TwoHoursBoundary and iFilenameExt */ 
      iFilenameExt += 2;
      TwoHoursBoundaryOld = TwoHoursBoundary;
      TwoHoursBoundary = TwoHoursBoundaryOld + 2.0 * 3600.0;
    }

      
    /* fill in data record part. */
    qSIGN = (attitQuat[iCtr][0] < 0.000) ? -1.000 : 1.0000;
     
    gDataRecord.D1_offset = offsets[iCtr] - TwoHoursBoundaryOld;

    gDataRecord.D2_emosID = gHeaderRecords[PosX].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[PosX].H5_pdbID;
    gDataRecord.D5_double = positionECI[iCtr][0];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[PosY].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[PosY].H5_pdbID;
    gDataRecord.D5_double = positionECI[iCtr][1];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[PosZ].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[PosZ].H5_pdbID;
    gDataRecord.D5_double = positionECI[iCtr][2];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[VeloX].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[VeloX].H5_pdbID;
    gDataRecord.D5_double = velocityECI[iCtr][0];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[VeloY].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[VeloY].H5_pdbID;
    gDataRecord.D5_double = velocityECI[iCtr][1];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[VeloZ].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[VeloZ].H5_pdbID;
    gDataRecord.D5_double = velocityECI[iCtr][2];
    print_data_record(CarryoutFile);


    gDataRecord.D2_emosID = gHeaderRecords[Quat1].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[Quat1].H5_pdbID;
    /* the following minus reverses the quat from 
       Spacecraft -> J2000 to J2000 -> Spacecraft */
    gDataRecord.D5_double = -qSIGN * attitQuat[iCtr][1];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[Quat2].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[Quat2].H5_pdbID;
    /* the following minus reverses the quat from 
       Spacecraft -> J2000 to J2000 -> Spacecraft */
    gDataRecord.D5_double = -qSIGN * attitQuat[iCtr][2];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[Quat3].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[Quat3].H5_pdbID;
    /* the following minus reverses the quat from 
       Spacecraft -> J2000 to J2000 -> Spacecraft */
    gDataRecord.D5_double = -qSIGN * attitQuat[iCtr][3];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[Quat4].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[Quat4].H5_pdbID;
    gDataRecord.D5_double = -qSIGN * attitQuat[iCtr][0];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[IRUtag1].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[IRUtag1].H5_pdbID;
    temP = TAITime +offsets[iCtr] -TwoHoursBoundaryOld;
    gDataRecord.D5_double = EPOCH_TIME + (double) ((int) temP);
    print_secFrom1958(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[IRUtag2].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[IRUtag2].H5_pdbID;
    scrat = 0.002 * cos(offsets[iCtr]);
    scrat = (scrat >= 0.0 ) ? scrat : -scrat;
    gDataRecord.D5_double = temP -(double) ((int) temP)+  scrat;

    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[AttRateX].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[AttRateX].H5_pdbID;
    gDataRecord.D5_double = xyzRotRates[iCtr][0];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[AttRateY].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[AttRateY].H5_pdbID;
    gDataRecord.D5_double = xyzRotRates[iCtr][1];
    print_data_record(CarryoutFile);

    gDataRecord.D2_emosID = gHeaderRecords[AttRateZ].H4_emosID;
    gDataRecord.D3_pdbID  = gHeaderRecords[AttRateZ].H5_pdbID;
    gDataRecord.D5_double = xyzRotRates[iCtr][2];
    print_data_record(CarryoutFile);

    /* fill in Status Word 2 every 8 seconds. */
    if ((int)offsets[iCtr] % 8 == 0) 
    {
      if (gbadIndex<NB_GBADEVENTS && gbadEvents[gbadIndex].offset <= offsets[iCtr])
      {
        prevAttMode = attMode;
        attMode = gbadEvents[gbadIndex].attMode;
        gbadMode = gbadEvents[gbadIndex].gbadMode;
        transTAITime = BaseTAITime + offsets[iCtr];
        PGS_TD_TAItoUTC(transTAITime, transTime);
#if 0
        TAITime = BaseTAITime + offsets[iCtr];
        PGS_TD_TAItoUTC(TAITime, transTime);
#endif
        printf("    Attitude mode transitioning from %i to %i at %s.\n",prevAttMode,attMode,transTime);
        gbadIndex++;
      }
      gDataRecord.D2_emosID  = gHeaderRecords[StatWd2].H4_emosID;
      gDataRecord.D3_pdbID   = gHeaderRecords[StatWd2].H5_pdbID;
      gDataRecord.D5_integer = gbadMode;
      print_data_record_statWd2(CarryoutFile);
    }
  }

  fclose(CarryoutFile);

  exit(0);

} /* end of main */


/*================= utility function =================*/
void print_header_records(FILE *ptrFile)
{
  int i;

  fprintf(ptrFile, "%d\n", NB_HEADERS);

  for (i=0; i < NB_HEADERS; i++)
  {
    fprintf(ptrFile, "%s|%d|%d|%c||\n",
            gHeaderRecords[i].H3_mnemonic,
            gHeaderRecords[i].H4_emosID,
            gHeaderRecords[i].H5_pdbID,
            gHeaderRecords[i].H6_type);
  }
} /* end of print_header_records */


void print_data_record(FILE *ptrFile)
{
    fprintf(ptrFile, "%f|%d|%d||%14.11E||%d\n", 
            gDataRecord.D1_offset,
            gDataRecord.D2_emosID,
            gDataRecord.D3_pdbID,
            gDataRecord.D5_double,
            gDataRecord.D7_statWd);
} /* end of print_data_record */

void print_secFrom1958(FILE *ptrFile)
{
    fprintf(ptrFile, "%f|%d|%d||%12.9E||%d\n", 
            gDataRecord.D1_offset,
            gDataRecord.D2_emosID,
            gDataRecord.D3_pdbID,
            gDataRecord.D5_double,
            gDataRecord.D7_statWd);
} /* end of print_secFrom1958 */


void print_data_record_statWd2(FILE *ptrFile)
{
    fprintf(ptrFile, "%f|%d|%d||%05u||%d\n",
            gDataRecord.D1_offset,
            gDataRecord.D2_emosID,
            gDataRecord.D3_pdbID,
            gDataRecord.D5_integer,
            gDataRecord.D7_statWd);
} /* end of print_data_record_statWd2 */
 

void time_converter(double TAITime, char *CarryoutTime)
{
  char asciiUTC_A[ASCII_UTC_SIZE], asciiUTC_B[ASCII_UTC_SIZE];
  PGSt_SMF_status returnStatus;

  returnStatus = PGS_TD_TAItoUTC((PGSt_double)TAITime, asciiUTC_A);
  if (returnStatus != PGS_S_SUCCESS)
  {
     printf("PGS_TD_TAItoUTC error: %d\n", returnStatus);
     exit(1);
  }

  returnStatus = PGS_TD_ASCIItime_AtoB(asciiUTC_A, asciiUTC_B);
  if (returnStatus != PGS_S_SUCCESS)
  {
     printf("PGS_TD_ASCIItime_AtoB error: %d\n", returnStatus);
     exit(1);
  }

  /* 
   * Now asciiUTC_B should be in format such as YYYY-DDDTHH:MM:SS.ssssssZ
   * We need to change it to format YYYY/DDD HH:MM:SS.mmm
   */
  asciiUTC_B[4] = '/';
  asciiUTC_B[8] = ' ';
  asciiUTC_B[21] = '\0';
   
  strcpy(CarryoutTime, asciiUTC_B);
}
