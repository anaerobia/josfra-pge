/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2009, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_ODLToXML.c
 
DESCRIPTION:
         The file contains PGS_MET_ODLToXML.c
	 The function is used to convert data from ODL attribute
	 to XML representation.

AUTHOR:
  	Abe Taaheri / Raytheon

HISTORY:
  	2-May-2009 	AT 	Initial version

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf.h>
#include <mfhdf.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

/* odl include files */

#include <CUC/odldef.h>
#include <CUC/odlinter.h>
#include <PGS_SMF.h>
#include <PGS_PC.h>
#include <PGS_MET.h>
#include <PGS_IO_1.h>
#include <PGS_MEM.h>

#define STYLESHEET 10303
#define TEMP_STYLESHEET 10260

PGSt_integer PGS_MET_IsClass(char *token);

char validateFlag[]="YES";


/* element names used for platform translation*/
char ASSOCIATEDPLATFORMINSTRUMENTSENSOR[] = "ASSOCIATEDPLATFORMINSTRUMENTSENSOR";
char ASSOCIATEDPLATFORMINSTRUMENTSENSORCONTAINER[] = "ASSOCIATEDPLATFORMINSTRUMENTSENSORCONTAINER";
char ASSOCIATEDSENSORSHORTNAME[] = "ASSOCIATEDSENSORSHORTNAME";
char ASSOCIATEDPLATFORMSHORTNAME[] = "ASSOCIATEDPLATFORMSHORTNAME";
char OPERATIONMODE_ODL[] = "OPERATIONMODE";
char ASSOCIATEDINSTRUMENTSHORTNAME[] = "ASSOCIATEDINSTRUMENTSHORTNAME";
char PLATFORM[] = "Platform";
char PLATFORM_SHORTNAME[] = "PlatformShortName";
char INSTRUMENT[] = "Instrument";
char INSTRUMENT_SHORTNAME[] = "InstrumentShortName";
char SENSOR[] = "Sensor";
char SENSOR_SHORTNAME[] = "SensorShortName";
char OPERATIONMODE_XML[] = "OperationMode";

/*declare  "COLLECTIONMETADATA" to only retrieve inventory metadata for cross DAAC ingest*/
char COLLECTIONMETADATA[] = "COLLECTIONMETADATA";
char ARCHIVEDMETADATA[] = "ARCHIVEDMETADATA";

/*declare comment symbols*/
char BEGIN_COMMENT[] = "/*";
char END_COMMENT[] = "*/";

/* regular expression used to split the line*/
char SPLIT_REGEX[] = "=";

/* parameter value delimiters*/
char NON_STRING_DEL[] = ",";
char STRING_DEL[] = "\",\"";

/*
  private Properties styleSheetParams_;
*/

/*declare ODL type*/
char BR[] = "BROWSE";
char DAP[] = "DAP";
char PH[] = "PH";
char QA[] = "QA";
char empty[] = "empty";

/*handle illegal characters in XML*/
char LESS[] = "<";
char LESSESC[] = "&lt;";
char GREAT[] = ">";
char GREATESC[] = "&gt;";
char AMPER[] = "&";
char AMPERESC[] = "&amp;";
char APOSTRESC[] = "&apos;";
char OPEN_PARENTHESIS = '(';
char CLOSE_PARENTHESIS = ')';
char APOSTR[] = "'";

/* XML symbology used to generate the raw xml file*/
char XML_START_START[] = "<";
char XML_END[] = ">";
char XML_END_START[] = "</";
char COMMENT_START[] = "<!--";
char COMMENT_END[] = "-->";
char NUM_VAL_START[] = "<NUM_VAL>";
char NUM_VAL_END[] = "</NUM_VAL>";
char CLASS_START[] = "<CLASS>";
char CLASS_END[] = "</CLASS>";
char VALUE_START[] = "<VALUE>";
char VALUE_END[] = "</VALUE>";
char TYPE_START[] = "<TYPE>";
char TYPE_END[] = "</TYPE>";
char GROUPTYPE_START[] = "<GROUPTYPE>";
char GROUPTYPE_END[] = "</GROUPTYPE>";

char GRINGPOINT[] = "GRINGPOINT";
char GRINGPOINTLONGITUDE[] = "GRINGPOINTLONGITUDE";
char GRINGPOINTLATITUDE[] = "GRINGPOINTLATITUDE";
char GRINGPOINTLATLON[] = "GRINGPOINTLATLON";
char POINT[] = "POINT";


/*declare all the element name used in the raw xml file*/
char odl_GROUP[] = "GROUP";
char odl_OBJECT[] = "OBJECT";
char odl_END_GROUP[] = "END_GROUP";
char odl_END_OBJECT[] = "END_OBJECT";
char odl_NUM_VAL[] = "NUM_VAL";
char odl_CLASS[] = "CLASS";
char odl_VALUE[] = "VALUE";
char odl_TYPE[] = "TYPE";
char odl_GROUPTYPE[] = "GROUPTYPE";
char odl_END[] = "END";

char odl_INPUT_GRANULE[] = "INPUTGRANULE";
char odl_PARAMETER_VALUE[] = "PARAMETERVALUE";


/*OdlToXmlTranlator performs the following functions:
 * 1) create simple raw translation of the ODL file into an xml stream
 * 2) translate the raw xml stream into a Data Pool compliant xml metadata file.
 *    It uses an xsl style sheet which is created by following the m2xt DTD
 */
/**************************************************************************/
/*makes all letters in a string uppercase*/
/**************************************************************************/
char  *PGS_MET_StrToUpper(/*in/out*/ char *stringvariable)
{
  PGSt_integer index;
  PGSt_integer length;

  length = strlen(stringvariable);
  for (index = 0; index < length; index ++)
    {
      if(isalpha(stringvariable[index])&& islower(stringvariable[index]))
	{
	  stringvariable[index] = toupper(stringvariable[index]);
	}
    }
  return stringvariable;
}

/**************************************************************************/
/* case insensitice string compare */
/**************************************************************************/
PGSt_integer PGS_MET_CaseInsensitiveStrcmp(char *s1, char *s2)
{
  char             *temps1=NULL;
  char             *temps2=NULL;
  PGSt_integer     len1;
  PGSt_integer     len2;
  PGSt_SMF_status  returnStatus;
  char *	   funcName = "PGS_MET_CaseInsensitiveStrcmp";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  len1 = strlen(s1);
  len2 = strlen(s2);
  if(len1 != len2) return(-1); /* strings do not match */
  if(temps1 != NULL)
    {
      PGS_MEM_Free(temps1);
      temps1 = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&temps1,(len1+1)*sizeof(char));
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(PGSMET_E_MALLOC_ERR);
    }

  if(temps2 != NULL)
    {
      PGS_MEM_Free(temps2);
      temps2 = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&temps2,(len2+1)*sizeof(char));
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      (void) PGS_MEM_Free(temps1);
      temps1 = (char *) NULL;
      return(PGSMET_E_MALLOC_ERR);
    }

  strcpy(temps1, s1);
  strcpy(temps2, s2);
  PGS_MET_StrToUpper(temps1);
  PGS_MET_StrToUpper(temps2);
  if(strcmp(temps1, temps2) == 0)
    {
      PGS_MEM_Free(temps1);
      PGS_MEM_Free(temps2);
      temps1=NULL;
      temps2=NULL;

      return(0);
    }
  else
    {
      PGS_MEM_Free(temps1);
      PGS_MEM_Free(temps2);
      temps1=NULL;
      temps2=NULL;
      return(-1);
    }
}

/**************************************************************************/
/* Split and trim resulting strings. Return only numStrs and max_elem_size 
   if Num_Elem is NULL*/
/**************************************************************************/
PGSt_integer PGS_MET_SplitTrim(char *inString, char *outStrings[], 
			       char *tag, PGSt_integer *Num_Elem, 
			       PGSt_integer *max_elem_size)
{
  PGSt_integer     loop=0;
  PGSt_integer     i,len;
  PGSt_integer     numStrs;
  char             *ptr = NULL;
  PGSt_integer     temp_max_elem_size=0;
  char             *tempinString=NULL;
  PGSt_SMF_status  returnStatus;
  char *	   funcName = "PGS_MET_SplitTrim";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  len = strlen(inString);
  if(len == 0)
    {
      *max_elem_size = 0;
      return 0;
    }
  if(tempinString != NULL)
    {
      PGS_MEM_Free(tempinString);
      tempinString = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&tempinString,(len+1)* sizeof(char));
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(PGSMET_E_MALLOC_ERR);
    }

  strcpy(tempinString, inString);
  if(Num_Elem == NULL)/* just get number of elements */
    {
      ptr = strtok(tempinString,tag);
      if(ptr == NULL)
	{
	  *max_elem_size = 0;
	  return 0;
	}

      temp_max_elem_size = strlen(ptr);

      for(loop=1;loop<MAX_SPLIT_ELEM;loop++)
	{
	  ptr = strtok('\0',tag);
	  if(ptr == NULL) break;
	  len = strlen(ptr);
	  if( len > temp_max_elem_size) temp_max_elem_size = len;
	}
      numStrs = loop;
      *max_elem_size = temp_max_elem_size;
    }
  else
    {
      ptr = strtok(tempinString,tag);
      if(ptr == NULL)
	{
	  return 0;
	}
      else
	{

	  strcpy(outStrings[0], ptr);
	}

      for(loop=1;loop< MAX_SPLIT_ELEM;loop++)
	{
	  ptr = strtok('\0',tag);
	  if(ptr == NULL)
	    {
	      break;
	    }
	  else
	    {
	      strcpy(outStrings[loop],ptr);
	    }
	}
      
      numStrs = loop;

      for(loop=0;loop<numStrs;loop++)
	{
	  outStrings[loop] = PGS_MET_Trim(outStrings[loop], outStrings[loop]);
	}
    }
  PGS_MEM_Free(tempinString);
  return numStrs;
}


/*******************************************************************************
      Split a value string using a delimiter (in this routine size of each
      element does not exceed 256 characters
********************************************************************************/

PGSt_integer PGS_MET_Split(char *inString, char *tag, char *outStrings[], 
			   PGSt_integer *Num_Elem, PGSt_integer *max_elem_size )
{
  char          *tmp=NULL;
  char          *tmp1;
  PGSt_integer  count = 0;
  PGSt_integer  i,j;
  PGSt_integer  emptyflg = 0;
  PGSt_integer  jj =0;
  char          *newp;
  PGSt_integer  temp_max_elem_size=0;

  /* if tag is one blank space call PGS_MET_SplitTrim routine, 
     otherwise continue */
  if((strlen(tag) == 1) && (tag[0] == ' '))
    {
      if(Num_Elem == NULL)/* just get number of elements */
	{
	  count = PGS_MET_SplitTrim(inString, NULL,tag, NULL, max_elem_size);
	}
      else
	{
	  count = PGS_MET_SplitTrim(inString, outStrings,tag, Num_Elem, max_elem_size);
	}
      return count;
    }

  /* tag is not just one blank space. Try something else ...... */
  /* if inString has no charcters, or just blanks return count=0 */
  if(inString[0] =='\0')
    {
      *max_elem_size = 0;
      count = 0;
      return count;
    }

  for(j =0; j <strlen(inString); j ++)
    {
      if(inString[j] != ' ')
	{
	  break;
	}
      else
	{
	  continue;
	}
    }
  
  if(j == strlen(inString))
    {
      emptyflg = 1;
    }

  if(emptyflg == 1)
    {
      *max_elem_size = 0;
      count = 0;
      return count;
    }

  if(Num_Elem == NULL)
    {
      count = PGS_MET_SplitTrim(inString, NULL,tag, NULL, max_elem_size);
    }
  else
    {
      count = PGS_MET_SplitTrim(inString, outStrings,tag, Num_Elem, max_elem_size);
    }

  return count;
}


/**************************************************************************
      Get rid of leading and trailing spaces in a string 
***************************************************************************/


char *PGS_MET_Trim(const char *str1, char *str2)
{
  char         *ptr;
  PGSt_integer i,j=0;
  PGSt_integer kk;
  char         mmm[4];
  char *temp=NULL;
  char tempbuf[ODLMAXSTMT];
  PGSt_integer length=0, length1=0, length2=0;
  PGSt_SMF_status  returnStatus;
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};
  char *	   funcName = "PGS_MET_Trim";

  if(str1 != NULL)  length1 = strlen(str1);
  if(str2 != NULL)  length2 = strlen(str2);
  length = length1;
  if(length2 > length) length = length2;

  if(length ==0)
    {
      ptr = str2;
      return ptr;
    }
  else if(length > (ODLMAXSTMT -1))
    {
      returnStatus = PGS_MEM_Malloc((void **)&temp, 
				    (length+1)*sizeof(char));
      if(returnStatus != PGS_S_SUCCESS)
	{
	  /* error message is:
	     "Unable to allocate memory for the hdf attribute" */
	  
	  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				  0, errInserts);
	  return(NULL);
	}
      
      ptr = str2;
      
      for(i=0;str1[i]!='\0';i++)
	{
	  if (str1[i] == ' ' ||  str1[i] == '\t' || str1[i] =='\0' )
	    {
	      continue;
	    }
	  else
	    {
	      strcpy(temp, str1+i);
	      break;
	    }
	} 
      
      j = strlen(temp);

      for(i=j; i>0 ;i--)
	{
	  if (temp[i-1] == ' ' || temp[i-1] == '\t')
	    { 
	      temp[i-1]='\0';
	    }
	  else
	    {
	      break;
	    } 
	}
      strcpy(str2,temp);
      free(temp);
      temp = NULL;
      ptr = str2;
    }
  else
    {
     ptr = str2;
      
      for(i=0;str1[i]!='\0';i++)
	{
	  if (str1[i] == ' ' ||  str1[i] == '\t' || str1[i] =='\0' )
	    {
	      continue;
	    }
	  else
	    {
	      strcpy(tempbuf, str1+i);
	      break;
	    }
	} 
      
      j = strlen(tempbuf);

      for(i=j; i>0 ;i--)
	{
	  if (tempbuf[i-1] == ' ' || tempbuf[i-1] == '\t')
	    { 
	      tempbuf[i-1]='\0';
	    }
	  else
	    {
	      break;
	    }
	}
      strcpy(str2,tempbuf);
      ptr = str2;
    }
  return ptr;
}



/**************************************************************************/
/* get rid of all spaces in the string */
/**************************************************************************/

void PGS_MET_TrimAllSpaces(char *str1, char *str2)
{
  char         ptr[ODLMAXSTMT];
  PGSt_integer i,j=0;
  PGSt_integer kk;

  for(kk=0; kk<ODLMAXSTMT; kk++) ptr[kk]='\0';

  for(i=0;str1[i]!='\0';i++)
    {
      if (str1[i] != ' ' && str1[i] != '\t') 
	ptr[j++]=str1[i];
    } 
  ptr[j]='\0';
  str2=ptr;
}


PGSt_integer PGS_MET_FileExists(char *filename)
{
  FILE *tempPtr;

  if((tempPtr = fopen(filename, "r")) != NULL)
    {
      fclose(tempPtr);
      return(PGS_TRUE);
    }
  else
    {
      return(PGS_FALSE);
    }
}


long PGS_MET_Filelength (char *fname)
{
  long rv, here;
  FILE *file;
  
  file=fopen(fname,"rb");
  if(file == NULL)
    {
      return(-1);
    }
  else
    {
      fseek(file,0,SEEK_END);
      rv = ftell(file);
      fclose(file);
      return(rv);
    }
}

/**************************************************************************/
/**************************************************************************/

PGSt_integer PGS_MET_ReplaceStr(char *inbuf, char *outbuf, char *tag, char *replacement)
 {
   char             *hit;
   PGSt_integer     count = 0;
   PGSt_integer	    lengthUpToSsubstitution;
   char             *tempout=NULL, *tempin=NULL, *tempin_temp=NULL;
   PGSt_integer     len, strlendiff;
   PGSt_integer     i;
   PGSt_SMF_status  returnStatus;
   char *	   funcName = "PGS_MET_ReplaceStr";
   char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

   len = strlen(inbuf);
   if(len > 0)
     {
       if(tempin != NULL)
	 {
	   PGS_MEM_Free(tempin);
	   tempin = NULL;
	 }
       returnStatus = PGS_MEM_Malloc((void **)&tempin,(len + 1)*sizeof(char));
       if(returnStatus != PGS_S_SUCCESS)
	 {
	   /* error message is:
	      "Unable to allocate memory for the hdf attribute" */
	   
	   (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				   0, errInserts);
	   return(PGSMET_E_MALLOC_ERR);
	 }

       strcpy(tempin, inbuf);
       
       strlendiff = strlen(replacement) - strlen(tag);
       
       if(strlendiff > 0 )
	 {
	   strlendiff = strlendiff;
	 }
       else
	 {
	   strlendiff = 0;
	 }
       if(tempout != NULL)
	 {
	   PGS_MEM_Free(tempout);
	   tempout = NULL;
	 }
       returnStatus = PGS_MEM_Malloc((void **)&tempout,(len + 1)*sizeof(char));
       if(returnStatus != PGS_S_SUCCESS)
	 {
	   /* error message is:
	      "Unable to allocate memory for the hdf attribute" */
	   
	   (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				   0, errInserts);
	   (void) PGS_MEM_Free(tempin);
	   tempin = (char *) NULL;
	   return(PGSMET_E_MALLOC_ERR);
	 }

       for(i=0; i<(len + 1); i++) tempout[i]='\0';

       while (1) 
	 {
	   /* find tag */
	   if ((hit = strstr(tempin, tag)) != NULL) 
	     {
	       returnStatus = PGS_MEM_Realloc((void **)&tempout,
					     (len + 1 + (count+1) * strlendiff)*sizeof(char));

	       for(i=(len + 1); i<(len + 1 + (count+1)); i++) tempout[i]='\0';

	       /* found - continue */
	       lengthUpToSsubstitution = (PGSt_integer)((hit) - (tempin));
	       
	       /*
		* copy 1st part tempin -> tempout -- not necessary ?
		*/
	       if(count == 0)
		 {
		   strncpy(tempout, tempin, lengthUpToSsubstitution);
		 }
	       else
		 {		  
		   strncat(tempout, tempin, lengthUpToSsubstitution);
		 }
	       /* append data into tempout */
	       strcat(tempout, replacement);
	       
	       /* append remainder of tempin after taglen chars */
	       tempin_temp = hit + strlen(tag);
	       strcat(tempout, tempin_temp);
	       
	       /* loop */
	       count++;
	     }
	   else
	     {
	       /* not found -> return (outbuf is correct) */
	       break;
	     }
	 }
       if(strlen(tempout) != 0)
	 {
	   if(outbuf != NULL)
	     {
	       PGS_MEM_Free(outbuf);
	       outbuf = NULL;
	     }
	   returnStatus = PGS_MEM_Malloc((void **)&outbuf,(strlen(tempout+1))*sizeof(char));
	   if(returnStatus != PGS_S_SUCCESS)
	     {
	       /* error message is:
		  "Unable to allocate memory for the hdf attribute" */
	       
	       (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				       0, errInserts);
	       (void) PGS_MEM_Free(tempin);
	       tempin = (char *) NULL;
	       (void) PGS_MEM_Free(tempout);
	       tempout = (char *) NULL;
	       return(PGSMET_E_MALLOC_ERR);
	     }

	   outbuf[strlen(tempout)]='\0';
	   strcpy(outbuf, tempout);
	 }
       /* clean up ad return */
       PGS_MEM_Free(tempin);
       PGS_MEM_Free(tempout);
       tempin = NULL;
       tempout = NULL;
       return (count);
     }
   else
     {
       if(outbuf != NULL)
	 {
	   PGS_MEM_Free(outbuf);
	   outbuf = NULL;
	 }
       returnStatus = PGS_MEM_Malloc((void **)&outbuf,sizeof(char));
       if(returnStatus != PGS_S_SUCCESS)
	 {
	   /* error message is:
	      "Unable to allocate memory for the hdf attribute" */
	   
	   (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				   0, errInserts);
	   return(PGSMET_E_MALLOC_ERR);
	 }

       strcpy(outbuf, "");
       return (0);
     }
 }

/**************************************************************************/
/**************************************************************************/

PGSt_integer PGS_MET_Setup(char *theOdlFilename, char *odlFilename, 
			   char *temp_rawXml, char *rawXml, char *xmlFilename, 
			   char *odlType, PGSt_integer *type)
{
  PGSt_integer isempty;
  char         *ext = NULL;
  char         errMsg[128];
  char         Msg[128];
  PGSt_integer emptyFile = 0;
  
  strcpy(odlFilename, theOdlFilename);
  
  /*check the odl file name is empty or not*/
  isempty = (PGSt_integer)PGS_MET_Filelength(odlFilename);
  if(isempty == -1)
    {
      sprintf(errMsg,"File %s does not exist. \n",odlFilename);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_FILE_NOEXIST,errMsg,"PGS_MET_Setup");
      return(PGSIO_E_GEN_FILE_NOEXIST);
    }
  else
    {
      if(isempty == 0)
	{
	  sprintf(errMsg,"File %s is empty. \n",odlFilename);
	  PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_OPEN_RECL,errMsg,"PGS_MET_Setup");
	  return(PGSIO_E_GEN_OPEN_RECL);
	}
    }
  
  if ( isempty > 0 )
    {
      emptyFile = 1;
    }

  if((PGS_MET_CaseInsensitiveStrcmp(odlType,BR)) != 0 &&
     (PGS_MET_CaseInsensitiveStrcmp(odlType,PH)) != 0 &&
     (PGS_MET_CaseInsensitiveStrcmp(odlType,QA)) != 0 &&
     (PGS_MET_CaseInsensitiveStrcmp(odlType,DAP)) != 0 )
    {
      *type = 1;
    }
  else /* we will only deal with type = 1 case, so types other than 2
	  are not set below  */
    {
      *type = 2;
    }

     /* construct the xml output file*/
  if ( emptyFile == 1 )
    {
      PGSt_integer index ;
      ext = strrchr( theOdlFilename, '.' );
      if ( strcasecmp( ext, ".met" ) != 0 )
	{
	  index = -1;
	}
      else
	{
	  index = 1;
	}

      if(index != -1)
	{
	  char          tempTemp[512];
	  PGSt_integer  str_length      = 0;
	  PGSt_integer  str1_length     = 0;
	  PGSt_integer  str2_length     = 0;
	  PGSt_integer  j;
	  char          *ptr1;
	  
	  strcpy(tempTemp,theOdlFilename);   
	  str2_length  = strlen(tempTemp);    /* length of the entire tif string */
	  ptr1  = strstr(tempTemp,".met");
	  str1_length  = strlen(ptr1);        /* length of the last part of string */
	  str_length   =  str2_length - str1_length;
	  for(j=0; j<(str_length+1); j++) { xmlFilename[j]='\0';}
	  strncpy(xmlFilename, tempTemp,  str_length);
	  xmlFilename[str_length] = '\0';

	  strcpy(rawXml, xmlFilename);
	  strcat(rawXml,"_RAW.xml");

	  strcpy(temp_rawXml, xmlFilename);
	  strcat(temp_rawXml,"_TempRAW.xml");

	  strcat(xmlFilename, ".xml");
	}
      else
	{

	  strcpy(xmlFilename, theOdlFilename);

	  strcpy(rawXml, xmlFilename);
	  strcat(rawXml,"_RAW.xml");

	  strcpy(temp_rawXml, xmlFilename);
	  strcat(temp_rawXml,"_TempRAW.xml");

	  strcat(xmlFilename,".xml");
	}
      
      /* xmlFilename is the final xml file name */
      /* temp_rawXml is the raw xml file before reprocessing */
      /* Reprocessed raw xml to be written to xmlFilename using stylesheet */


       /* !rawXml.exists() ? */ 

      /* 
      if((output = fopen( rawXml, "r")) == NULL)
	{
	  output = fopen( rawXml, "w");
	  fclose(output);
	}
      else
	{
	  fclose(output);
	  sprintf(Msg, "Output file %s exist. Exiting ......\n",rawXml);
	  sprintf(errMsg, "OdlToXmlTranslator::Setup() ERROR: %s\n", Msg);
	  printf("%s",errMsg);
	  return(-1);
	}
      */
    }
  return (PGS_S_SUCCESS);
}

/**************************************************************************/
/* set the (name, value) pair set for the style sheet as input parameters*/
/**************************************************************************/

void PGS_MET_SetStyleSheetParameters(ssProperty *styleSheetParams, char *dataCenterId)
{
  char             empty[] ="0";
  PGSt_SMF_status  returnStatus;
  
  /* Pass in valid values*/
  strcpy(styleSheetParams->DataCenterId,
	 dataCenterId);
  /* strcpy(styleSheetParams->EmptyBrowse,
     empty);*/
  
}


/*****************************************************************************
       This method will do the actual transform from raw xml to dpl xml file
       based on the type of style sheet
******************************************************************************/

PGSt_integer PGS_MET_TranslateToDplXml(char *xmlFilename, char *rawXml, 
				       char *dataCenterId, PGSt_integer type)
{
  /* now that we have a raw XML representation of the .met file, use XSLT for
     translation into DPL style xml*/
  
  PGSt_SMF_status  returnStatus;
  char             styleSheet[128];
  char             tmpstyleSheet[128];
  char             *errMsg=(char *)NULL;
  char             *msgBuf = (char *)NULL;
  PGSt_integer     msgLen;
  FILE             *fptr;

  PGSt_SMF_boolean  copyStatus;
  PGSt_SMF_status   status = -1;
  ssProperty        styleSheetParams;
  char              *parameters[16+1];
  PGSt_integer      nbparams;
  PGSt_integer      isempty;
  PGSt_integer	    version;
  char		    referenceID[PGSd_PC_FILE_PATH_MAX];
  PGSt_IO_Gen_FileHandle    *fileHandle = NULL;	  /* file pointer to the
						     temporary stylesheet */
  char              *errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};
  char 	            fileIdStr[PGSd_MET_FILE_ID_L]; /*file id value as string */
  PGSt_integer      len_filename, len_count;
  PGSt_uinteger     sum_chars;
  char              pid_string[20];  /* process ID (PID) string */
  char *	   funcName = "PGS_MET_TranslateToDplXml";

  if(errMsg != NULL)
    {
      PGS_MEM_Free(errMsg);
      errMsg = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&errMsg, 128*sizeof(char));
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(PGSMET_E_MALLOC_ERR);
    }

  /* switch case to determine which style sheet to use.*/
  
  switch(type)
    {
      
    case 1:
      {
	version = 1;
	returnStatus = PGS_PC_GetReference(STYLESHEET, &version, referenceID);
	
	if (returnStatus != PGS_S_SUCCESS)
	  {
	    errInserts[0]= "stylesheet filename";
	    (void) PGS_MET_ErrorMsg(PGSMET_E_PCREAD_ERR, "PGS_MET_TranslateToDplXml",
				    1, errInserts);
	    PGS_MEM_Free(errMsg);
	    errMsg =NULL;
	    return(-1);
	  }

	strcpy(styleSheet, referenceID);
	break;
      }
    case 2: /* not implemented */
      {
	sprintf(errMsg,"File %s does not exist, since BROWSE stylesheet type conversion has not been implemented. \n","browse.xsl");
	PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_FILE_NOEXIST,errMsg,"PGS_MET_TranslateToDplXml");
	PGS_MEM_Free(errMsg);
	errMsg =NULL;
	return(-1);
	/*strcpy(styleSheet, "../data/browse.xsl"); */
	break;
      }
      
    case 3: /* not implemented */
      {
	sprintf(errMsg,"File %s does not exist, since PH stylesheet type conversion has not been implemented. \n","ph.xsl");
	PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_FILE_NOEXIST,errMsg,"PGS_MET_TranslateToDplXml");
	PGS_MEM_Free(errMsg);
	errMsg =NULL;
	return(-1);
	/*strcpy(styleSheet, "../data/ph.xsl");*/
	break;
        
      }
    case 4: /* not implemented */
      {
	sprintf(errMsg,"File %s does not exist, since QA stylesheet type conversion has not been implemented. \n","qa.xsl");
	PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_FILE_NOEXIST,errMsg,"PGS_MET_TranslateToDplXml");
	PGS_MEM_Free(errMsg);
	errMsg =NULL;
	return(-1);
	/*strcpy(styleSheet, "../data/qa.xsl");*/
	break;
      }
    case 5: /* not implemented */
      {
	sprintf(errMsg,"File %s does not exist, since DAP stylesheet type conversion has not been implemented. \n","dap.xsl");
	PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_FILE_NOEXIST,errMsg,"PGS_MET_TranslateToDplXml");
	PGS_MEM_Free(errMsg);
	errMsg =NULL;
	return(-1);
	/*strcpy(styleSheet, "../data/dap.xsl");*/
	break;
      }
    }
  /*
  returnStatus = PGS_IO_Gen_Temp_Open(PGSd_IO_Gen_NoEndurance, TEMP_STYLESHEET,
					PGSd_IO_Gen_Write, &fileHandle);
  if(returnStatus != PGS_S_SUCCESS)
    {
      if(returnStatus != PGSIO_W_GEN_ACCESS_MODIFIED)
	{
	  sprintf(fileIdStr, "%d", TEMP_STYLESHEET);
	  errInserts[0] = "temporary";
	  errInserts[1] = fileIdStr;
	  (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_TranslateToDplXml",
				  2, errInserts);
	  PGS_MEM_Free(errMsg);
	  errMsg = NULL;
	  return(-1);
	}
      PGS_IO_Gen_Close(fileHandle);
      version = 1;
      returnStatus = PGS_PC_GetReference(TEMP_STYLESHEET,&version,referenceID);
      if (returnStatus != PGS_S_SUCCESS)
	{
	  errInserts[0]= "temporary stylesheet filename"; 
	  (void) PGS_MET_ErrorMsg(PGSMET_E_PCREAD_ERR, "PGS_MET_TranslateToDplXml",
				  1, errInserts);
	  PGS_MEM_Free(errMsg);
	  errMsg =NULL;
	  return(-1);
	}
      strcpy(tmpstyleSheet, referenceID);
    }
  else
    {
      PGS_IO_Gen_Close(fileHandle);
      version = 1;
      returnStatus = PGS_PC_GetReference(TEMP_STYLESHEET,&version,referenceID);
      if (returnStatus != PGS_S_SUCCESS)
	{
	  errInserts[0]= "temporary stylesheet filename"; 
	  (void) PGS_MET_ErrorMsg(PGSMET_E_PCREAD_ERR, "PGS_MET_TranslateToDplXml",
				  1, errInserts);
	  PGS_MEM_Free(errMsg);
	  errMsg =NULL;
	  return(-1);
	}
      strcpy(tmpstyleSheet, referenceID);
    }
  */

  version = 1;
  returnStatus = PGS_PC_GetReference(TEMP_STYLESHEET,&version,referenceID);
  if (returnStatus != PGS_S_SUCCESS)
    {
      errInserts[0]= "temporary stylesheet filename"; 
      (void) PGS_MET_ErrorMsg(PGSMET_E_PCREAD_ERR, "PGS_MET_TranslateToDplXml",
			      1, errInserts);
      PGS_MEM_Free(errMsg);
      errMsg =NULL;
      return(-1);
    }

  /*get Process ID */
  
  sprintf(pid_string, "%u", getpid());

  len_filename = strlen(xmlFilename);
  sum_chars = 0;
  for(len_count=0; len_count<len_filename; len_count++)
    {
      sum_chars = sum_chars + (PGSt_uinteger)xmlFilename[len_count];
    }

  sprintf(tmpstyleSheet,"%s_%s_%u",referenceID, pid_string, sum_chars);
 

  /* craete a temporary copy of styleSheet */
  
  msgLen = strlen(styleSheet) + strlen(tmpstyleSheet) + strlen("cp %s %s") + 2;
  returnStatus = PGS_MEM_Calloc((void **)&msgBuf,msgLen,sizeof(char));
  
  switch (returnStatus)
    {
    case PGS_S_SUCCESS:
      sprintf(msgBuf,"cp %s %s",styleSheet,tmpstyleSheet);
      break;
    case PGSMEM_E_NO_MEMORY:
      PGS_MEM_Free(errMsg);
      errMsg =NULL;
      return(-1);
    default:
      PGS_SMF_SetUnknownMsg(returnStatus,"PGS_MET_TranslateToDplXml");
      PGS_MEM_Free(errMsg);
      errMsg =NULL;
      return(-1);
    }
  
  if (PGS_SMF_System(msgBuf) == PGS_S_SUCCESS)
    {
      copyStatus = PGS_TRUE;
    }
  else
    {
      copyStatus = PGS_FALSE;
    }
  
  if(copyStatus == PGS_FALSE)
    {
      sprintf(fileIdStr, "%d", TEMP_STYLESHEET);
      errInserts[0] = "temporary";
      errInserts[1] = fileIdStr;
      (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_TranslateToDplXml",
			      2, errInserts);
      PGS_MEM_Free(errMsg);
      errMsg =NULL;
      if (msgBuf != NULL)
	{
	  PGS_MEM_Free(msgBuf);
	}
      return(-1);
    }
  
  /*initialize the output file*/      

  copyStatus = PGS_FALSE;

  if (!PGS_MET_FileExists(xmlFilename))
    {
      /* xmlFilename does not exist. Create a new dpl_output file */

      /* Check if we can create xmlFilename
	 in user workstation. */
      
      if ((fptr = fopen(xmlFilename,"w")) != (FILE *)NULL)
	{
	  /* Okay, the file is not bogus. */
	  
	  fclose(fptr);
	  copyStatus = PGS_TRUE;
	}
      else
	{
	  copyStatus = PGS_FALSE;
	}
      
      if(copyStatus == PGS_FALSE)
	{
	  sprintf(errMsg,"Cannot create file %s. \n", xmlFilename);
	  PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_CREATE_FAILURE,errMsg,
				"PGS_MET_TranslateToDplXml");
	  returnStatus = PGSIO_E_GEN_CREATE_FAILURE;
	  PGS_MEM_Free(errMsg);
	  errMsg =NULL;
	  return (-1);
	}
    }

  if(!PGS_MET_FileExists(rawXml) )
    {
      sprintf(errMsg,"Raw XML File %s Doesn't exist \n", rawXml);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_CREATE_FAILURE,errMsg,
			    "PGS_MET_TranslateToDplXml");
      returnStatus = PGSIO_E_GEN_FILE_NOEXIST;
      PGS_MEM_Free(errMsg);
      errMsg =NULL;
      return (-1);
    }

  PGS_MET_SetStyleSheetParameters(&styleSheetParams, dataCenterId);

  isempty = (PGSt_integer)PGS_MET_Filelength(rawXml);

  if(isempty == -1)
    {
      sprintf(errMsg,"Raw XML File %s Doesn't exist \n", rawXml);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_CREATE_FAILURE,errMsg,
			    "PGS_MET_TranslateToDplXml");
      returnStatus = PGSIO_E_GEN_FILE_NOEXIST;
      PGS_MEM_Free(errMsg);
      errMsg =NULL;
      return (returnStatus);
    }
  else
    {
      if(isempty == 0)
	{
	  sprintf(errMsg,"Raw XML File %s  is empty\n", rawXml);
	  PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_CREATE_FAILURE,errMsg,
				"PGS_MET_TranslateToDplXml");
	  returnStatus = PGSIO_E_GEN_FILE_NOEXIST;
	  PGS_MEM_Free(errMsg);
	  errMsg =NULL;
	  return (returnStatus);
	}
    }
  
  /* Transform raw xml input file to dpl xml output file, using 
     style sheet input parameters.*/

  nbparams = 2; /* only DataCenterId name and value will be passed */
  status = PGS_MET_XslProcessor(tmpstyleSheet, rawXml, xmlFilename,
			styleSheetParams, nbparams);

  if(status == -1)
    {
      sprintf(errMsg,"Failed translating Raw XML to stylesheet format\n");
      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,errMsg,
			    "PGS_MET_TranslateToDplXml");
      returnStatus = PGS_E_TOOLKIT;
      PGS_MEM_Free(errMsg);
      errMsg =NULL;
      return (-1);
      if (msgBuf != (char *)NULL)
	{
	  PGS_MEM_Free(msgBuf);
	  msgBuf = (char *)NULL;
	}
      return (-1);
    }

  /*PGS_IO_Gen_Temp_Delete(TEMP_STYLESHEET);*/

  if(errMsg != NULL)
    {
      PGS_MEM_Free(errMsg);
       errMsg =NULL;
    }

  if (msgBuf != (char *)NULL)
    {
      PGS_MEM_Free(msgBuf);
      msgBuf = (char *)NULL;
    }
  
  return(PGS_S_SUCCESS);
}


/******************************************************************************
     This method will translate ODL file to raw xml file
*******************************************************************************/

PGSt_integer PGS_MET_TranslateToRawXml(char *odlFilename, char *temp_rawXml)
{
  char             errMsg[128]=" ";
  char             Msg[128]=" ";
  char             *Msg_line=NULL;
  PGSt_integer     i, n=0, j, jcount;
  char             *tmp[MAX_SPLIT_ELEM]={NULL};
  PGSt_integer     start_of_tag;
  PGSt_integer     status;
  PGSt_integer     lcount=0;
  PGSt_integer     kk=0;
  char             *newp=NULL;
  char             *name=NULL;
  char             *p=NULL;

  FILE             *file_in=NULL;
  FILE             *file_out=NULL;
  
  char             *line = NULL;
  char             *trimLine = NULL;
  char             *potentialLine = NULL;
  char             *preProcessedLine = NULL;
  char             *processedLine = NULL;
  PGSt_SMF_boolean newFlag = FALSE;
  PGSt_integer     completeFlag = 1;
  PGSt_SMF_status  returnStatus;
  PGSt_integer     unmathched_Parenthesis;
  PGSt_integer     unmathched_doubleQuotes;
  PGSt_integer     InputPointer_flag=0;
  char *	   funcName = "PGS_MET_TranslateToRawXml";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  unmathched_Parenthesis = 0;
  unmathched_doubleQuotes = 0;

  if(name != NULL)
    {
      PGS_MEM_Free(name);
      name = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&name,256*sizeof(char));
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(PGSMET_E_MALLOC_ERR);
    }

  for(i=0; i< 256; i++) name[i] = '\0';

  /*check whether the input ODL file exists or not*/

  if(!PGS_MET_FileExists(odlFilename))
    {
      sprintf(errMsg,"File %s does not exist. \n",odlFilename);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_FILE_NOEXIST,errMsg,"PGS_MET_TranslateToRawXml");
      PGS_MEM_Free(name);
      name = NULL;
      return(-1);
    }

  file_in = fopen(odlFilename,"r");
  
  if(file_in == NULL)
    {
      sprintf(errMsg, "Cannot open odl file %s!", odlFilename);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_OPEN,errMsg,"PGS_MET_TranslateToRawXml");
      PGS_MEM_Free(name);
      name = NULL;
      return(-1);
    }

  file_out = fopen(temp_rawXml,"w");
  
  if(file_out == NULL)
    {
      sprintf(errMsg, "Cannot open rawXML file %s!",temp_rawXml);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_OPEN,errMsg,"PGS_MET_TranslateToRawXml");
      PGS_MEM_Free(name);
      name = NULL;
      return(-1);
    }
  
  if(line != NULL)
    {
      PGS_MEM_Free(line);
      line = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&line,ODLMAXSTMT*sizeof(char));
  line[0]='\0';
  line[1]='\0';
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      (void) PGS_MEM_Free(name);
      name = (char *) NULL;
      return(PGSMET_E_MALLOC_ERR);
    }

  if(preProcessedLine != NULL)
    {
      PGS_MEM_Free(preProcessedLine);
      preProcessedLine = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&preProcessedLine,ODLMAXSTMT*sizeof(char));
  preProcessedLine[0]='\0';
  preProcessedLine[1]='\0';
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      (void) PGS_MEM_Free(line);
      line = (char *) NULL;
      (void) PGS_MEM_Free(name);
      name = (char *) NULL;
      return(PGSMET_E_MALLOC_ERR);
    }

  /* Start translating odl file to raw xml file......*/

  while ( (fgets(line, (ODLMAXSTMT - 1), file_in)) != NULL)
    {
      PGSt_integer i;
      PGSt_integer lenline;
      PGSt_integer max_elem_size;
      

      if((strstr(line,"COLLECTIONMETADATA") != NULL) ||
	 (strstr(line,"ARCHIVEDMETADATA") != NULL) )
	{
	  while ( (fgets(line, (ODLMAXSTMT - 1), file_in)) != NULL)
	    {
	      if((strstr(line,"COLLECTIONMETADATA") != NULL) ||
		 (strstr(line,"ARCHIVEDMETADATA") != NULL) )
		{
		  break;
		}
	    }
	}


      lenline = strlen(line);
      if(lenline > 0) line[lenline - 1] = '\0'; /*get rid of return char at 
						  the end of line */
      if(trimLine != NULL)
	{
	  PGS_MEM_Free(trimLine);
	  trimLine = NULL;
	}
      returnStatus = PGS_MEM_Malloc((void **)&trimLine,(lenline+1)*sizeof(char));
      if(returnStatus != PGS_S_SUCCESS)
	{
	  /* error message is:
	     "Unable to allocate memory for the hdf attribute" */
	  
	  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				  0, errInserts);
	  (void) PGS_MEM_Free(line);
	  line = (char *) NULL;
	  (void) PGS_MEM_Free(name);
	  name = (char *) NULL;
	  (void) PGS_MEM_Free(preProcessedLine);
	  preProcessedLine = (char *) NULL;
	  return(PGSMET_E_MALLOC_ERR);
	}

      lcount++;

       for(kk=0; kk<(lenline+1); kk++) trimLine[kk]='\0';

       newp = PGS_MET_Trim(line, trimLine);
       
       strcpy(trimLine, newp);

       if ((strlen(trimLine)) == 0)
	 {
	   PGS_MEM_Free(trimLine);
	   trimLine = NULL;
	   continue;
	 }
 
       if (completeFlag == 1)
	 {
	   p = strstr(line,SPLIT_REGEX);
	   
	   if( p != NULL)
	     {
	       start_of_tag = (PGSt_integer)((p) - (line));/* -1 is for the return 
							      character at the end 
							      of the line */
	       strncpy(name, line, start_of_tag );
	       
	       n=0;
	       
	       n = PGS_MET_Split(name, " ", tmp, NULL, &max_elem_size);
	       if (n == 1)
		 {
		   newFlag = TRUE;
		 }
	     }
	   else
	     {
	       /* check for end*/
	       
	       if ((PGS_MET_CaseInsensitiveStrcmp(trimLine, odl_END) == 0))
		 {
		   newFlag = TRUE;
		 }
	     }
	   
	   if (newFlag)
	     {
	       if (strcmp(preProcessedLine,"") == 0)
		 {
		   strcpy(preProcessedLine, line);
		 }
	       else
		 {
		   /* Do not read collection and  archive metadata */
		   if((strstr(preProcessedLine,"COLLECTIONMETADATA") != NULL) ||
		      (strstr(preProcessedLine,"ARCHIVEDMETADATA") != NULL) )
		     {
		       break;
		     }

		   /*  invoke processline on the preprocessline*/
		   /* note: processedLine length can be larger than 
		      preProcessedLine length because of converting thjngs
		      like & to &amp
		      So lets make it twice as big!
		   */
		   /* value lines can have many <VALUE></VALUE> pairs, which 
		      has 15 characters.So the processed line can exceed end
		      by number of element * 15..
		      we assume that maximum number of elements in the 
		      value is going to be less than 10000= MAX_SPLIT_ELEM */

		   if(processedLine != NULL)
		     {
		       PGS_MEM_Free(processedLine);
		       processedLine = NULL;
		     }
		   returnStatus = 
		     PGS_MEM_Malloc((void **)&processedLine,((15*MAX_SPLIT_ELEM) + 
					   strlen(preProcessedLine)+1)*sizeof(char));
		   if(returnStatus != PGS_S_SUCCESS)
		     {
		       /* error message is:
			  "Unable to allocate memory for the hdf attribute" */
		       
		       (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
					       0, errInserts);
		       (void) PGS_MEM_Free(line);
		       line = (char *) NULL;
		       (void) PGS_MEM_Free(name);
		       name = (char *) NULL;
		       (void) PGS_MEM_Free(preProcessedLine);
		       preProcessedLine = (char *) NULL;
		       if(trimLine != NULL)
			 {
			   PGS_MEM_Free(trimLine);
			   trimLine= NULL;
			 }
		       return(PGSMET_E_MALLOC_ERR);
		     }

		   processedLine = PGS_MET_ProcessLine(preProcessedLine, file_out, file_in, 
						       processedLine, &InputPointer_flag);

		   /*write it to an output file*/
		   if (processedLine != NULL)
		     {
		       fprintf(file_out,"%s\n",processedLine);
		       /*For debugging info, we need to log 
			 each converted line*/
		       /*
		       returnStatus = PGS_MEM_Malloc((void **)&Msg_line,(strlen(processedLine)+128+1)*sizeof(char));
		       sprintf(Msg_line,"Processing Line :%s\n",processedLine );
		       logger_log(Msg);
		       PGS_MEM_Free(Msg_line);
		       Msg_line = NULL;
		       */
		       if(processedLine != NULL)
			 {
			   PGS_MEM_Free(processedLine);
			   processedLine= NULL;
			 }
		     }
		   else
		     {
		       /* processedLine is NULL */
		     }
		   
		   strcpy(preProcessedLine, line);
		 }
	     }
	   else
	     {
	       strcat(preProcessedLine, line);
	     }
	 }
       else
	 {
	   /* simply add the current line to the incomplete line*/
	   strcat(preProcessedLine, trimLine);
	 }
       
       /* check if the preProcessedLine is a possible complete line
	  i.e. if the \" and () are matched within the line*/
       
       if(PGS_MET_CaseInsensitiveStrcmp(preProcessedLine, odl_END) == 0)
	 {
	   break;
	 }
       
       completeFlag = PGS_MET_IsLineComplete(line, &unmathched_Parenthesis, 
					     &unmathched_doubleQuotes);
       strcpy(line,"");
       
       if(trimLine != NULL)
	 {
	   PGS_MEM_Free(trimLine);
	   trimLine= NULL;
	 }
       
    }/*while loop ends*/
  
  fclose(file_in);
  fclose(file_out);
  
  if(line !=NULL)
    {
      PGS_MEM_Free(line);
      line=NULL;
    }
  
  if(preProcessedLine !=NULL)
    {
      PGS_MEM_Free(preProcessedLine);
      preProcessedLine=NULL;
    }

  if(trimLine!=NULL)
    {
      PGS_MEM_Free(trimLine);
      trimLine=NULL;
    }

  if(name != NULL)
    {
      PGS_MEM_Free(name);
      name= NULL;
    }

  return (PGS_S_SUCCESS);
}


/**************************************************************************
      This routine checks whether the line complete or not. If not, we need keep 
      reading the next line.
***************************************************************************/

PGSt_integer PGS_MET_IsLineComplete(char *line, PGSt_integer *unmathched_Parenthesis,
				    PGSt_integer *unmathched_doubleQuotes)
{
  PGSt_integer test = 0;
  PGSt_integer leftParenthesis = 0;
  PGSt_integer rightParenthesis = 0;
  PGSt_integer doubleQuotes = 0;
  
  /* Check to see if this is a comment line.
     If we have comment markers, then line is complete */

  if((strstr(line, "/*") != NULL) && (strstr(line, "*/") != NULL))
    {
      test = 1;
      return(test);
    }

  leftParenthesis = PGS_MET_NumberOfChar(line, '(');
  rightParenthesis = PGS_MET_NumberOfChar(line, ')');
  
  doubleQuotes = PGS_MET_NumberOfChar(line,'\"');

  if(*unmathched_Parenthesis == 0 && *unmathched_doubleQuotes == 0)
    {
      if( doubleQuotes != 0 && 
	  doubleQuotes % 2 == 0 && 
	  leftParenthesis == 0 && 
	  rightParenthesis == 0   )
	{
	  test = 1;
	  return(test);
	}
      else
	{
	  test = 0;
	}

      if (leftParenthesis == 0 && 
	  rightParenthesis == 0 && 
	  doubleQuotes == 0 )
	{
	  test = 1;
	  return(test);
	}
      else
	{
	  test = 0;
	}
    }

  *unmathched_Parenthesis = (*unmathched_Parenthesis + leftParenthesis - rightParenthesis);
  *unmathched_doubleQuotes = ((*unmathched_doubleQuotes + doubleQuotes) % 2);

  if(*unmathched_Parenthesis == 0 && *unmathched_doubleQuotes == 0)
    {
	  test = 1;
    }
  else
    {
	  test = 0;
    }

  return test;
}


/******************************************************************************
 * Each line shall be tokenized based on the = sign.  Entities on the left and
 * right of the token will be stripped of heading/trailing spaced and entities
 * on the right will be stripped of trailing and ending double quotes.
 * @param line String representing the raw ODL line
 * @return String representing the XML line ready for writing to the output file
 *******************************************************************************/

char *PGS_MET_ProcessLine(char *aline, FILE *out, FILE *in, 
			  char *processedLine, PGSt_integer *InputPointer_flag)
{
  char             *ptr=NULL;
  PGSt_SMF_boolean inputPointer = FALSE;
  PGSt_integer     i,j;
  char             *start=NULL;
  char             *end=NULL;
  char             *tokens[MAX_SPLIT_ELEM]={NULL};
  PGSt_integer     len=0, len0=0;
  PGSt_integer     status=0;
  char             *trimLine = NULL;
  char             *newp=NULL;
  PGSt_integer     aline_len=0;
  char             errMsg[128]=" ";
  PGSt_integer     max_elem_size=0;
  PGSt_SMF_status  returnStatus=0;
  char *	   funcName = "PGS_MET_ProcessLine";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  ptr = processedLine;
  aline_len = strlen(aline);

  if(trimLine != NULL)
    {
      PGS_MEM_Free(trimLine);
      trimLine = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&trimLine,(aline_len + 1)*sizeof(char));
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(NULL);
    }

  if (aline_len > 0)
    {
      /* Check to see if this is a comment line.
	 If there is no equal character in the line and we have comment markers
	 Otherwise it would be syntactically invalid.*/
      if((strstr(aline, "/*") != NULL) && (strstr(aline, "*/") != NULL))
	{
	  /* trim the line of leading and trailing whitespace*/
	  strcpy(trimLine, "");
	  newp = PGS_MET_Trim(aline, trimLine);
	  strcpy(aline, newp);
	  processedLine = PGS_MET_CreateXmlComment(aline, processedLine);
	}
      else
	{

	  len0 = PGS_MET_Split(aline, SPLIT_REGEX, NULL, NULL, &max_elem_size);

	  if(len0 !=0)
	    {
	      for(i=0; i<len0; i++)
		{
		  if(tokens[i] != NULL)
		    {
		      PGS_MEM_Free(tokens[i]);
		      tokens[i] = NULL;
		    }
		  returnStatus = PGS_MEM_Malloc((void **)&tokens[i], 
						(max_elem_size+100)*sizeof(char));
		  if(returnStatus != PGS_S_SUCCESS)
		    {
		      /* error message is:
			 "Unable to allocate memory for the hdf attribute" */
		      
		      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
					      0, errInserts);
		      if(trimLine != NULL)
			{
			  PGS_MEM_Free(trimLine);
			  trimLine = NULL;
			}
		      return(NULL);
		    }
		  for(j=0; j<(max_elem_size+100); j++) tokens[i][j]='\0';
		}
	    }

	  len = PGS_MET_Split(aline, SPLIT_REGEX, tokens, &len0, &max_elem_size);

	  if(start != NULL)
	    {
	      PGS_MEM_Free(start);
	      start = NULL;
	    }
	  returnStatus = PGS_MEM_Malloc((void **)&start,
					(strlen(tokens[0]) + 1)*sizeof(char));
	  if(returnStatus != PGS_S_SUCCESS)
	    {
	      /* error message is:
		 "Unable to allocate memory for the hdf attribute" */
	      
	      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				      0, errInserts);
	      if(trimLine != NULL)
		{
		  PGS_MEM_Free(trimLine);
		  trimLine = NULL;
		}
	      for(i=0; i<len0; i++)
		{
		  if(tokens[i] != NULL)
		    {
		      PGS_MEM_Free(tokens[i]);
		      tokens[i] = NULL;
		    }
		}
	      return(NULL);
	    }

	  strcpy(start,"");
	  newp = PGS_MET_Trim(tokens[0], start);
	  strcpy(start, newp);

	  if(len > 1)
	    {
	      if(len == 2)
		{
		  if (end == NULL)
		    {
		      returnStatus = PGS_MEM_Malloc((void **)&end,
						    (strlen(tokens[1])+1)*sizeof(char));
		      if(returnStatus != PGS_S_SUCCESS)
			{
			  /* error message is:
			     "Unable to allocate memory for the hdf attribute" */
			  
			  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
						  0, errInserts);
			  if(trimLine != NULL)
			    {
			      PGS_MEM_Free(trimLine);
			      trimLine = NULL;
			    }
			  for(i=0; i<len0; i++)
			    {
			      if(tokens[i] != NULL)
				{
				  PGS_MEM_Free(tokens[i]);
				  tokens[i] = NULL;
				}
			    }
			  PGS_MEM_Free(start);
			  start = NULL;
			  return(NULL);
			}
		      strcpy(end,"");
		    }
		  else
		    {
		      PGS_MEM_Free(end);
		      end = NULL;
		      returnStatus = PGS_MEM_Malloc((void **)&end,
						    (strlen(tokens[1])+1)*sizeof(char));
		      if(returnStatus != PGS_S_SUCCESS)
			{
			  /* error message is:
			     "Unable to allocate memory for the hdf attribute" */
			  
			  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
						  0, errInserts);
			  if(trimLine != NULL)
			    {
			      PGS_MEM_Free(trimLine);
			      trimLine = NULL;
			    }
			  for(i=0; i<len0; i++)
			    {
			      if(tokens[i] != NULL)
				{
				  PGS_MEM_Free(tokens[i]);
				  tokens[i] = NULL;
				}
			    }
			  PGS_MEM_Free(start);
			  start = NULL;
			  return(NULL);
			}
		      strcpy(end,"");
		    }

		  newp = PGS_MET_Trim(tokens[1], end);

		  strcpy(end, newp);

		}
	      else if (len > 2)
		{
		  /* Find the location of the first equals character*/
		  char *p;
		  if (end == NULL)
		    {
		      returnStatus = PGS_MEM_Malloc((void **)&end,
						    (aline_len - strlen(tokens[0])+1)*sizeof(char));
		      if(returnStatus != PGS_S_SUCCESS)
			{
			  /* error message is:
			     "Unable to allocate memory for the hdf attribute" */
			  
			  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
						  0, errInserts);
			  if(trimLine != NULL)
			    {
			      PGS_MEM_Free(trimLine);
			      trimLine = NULL;
			    }
			  for(i=0; i<len0; i++)
			    {
			      if(tokens[i] != NULL)
				{
				  PGS_MEM_Free(tokens[i]);
				  tokens[i] = NULL;
				}
			    }
			  PGS_MEM_Free(start);
			  start = NULL;
			  return(NULL);
			}
		    }
		  else
		    {
		      PGS_MEM_Free(end);
		      end = NULL;
		      returnStatus = PGS_MEM_Malloc((void **)&end,
						    (aline_len - strlen(tokens[0])+1)*sizeof(char));
		      if(returnStatus != PGS_S_SUCCESS)
			{
			  /* error message is:
			     "Unable to allocate memory for the hdf attribute" */
			  
			  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
						  0, errInserts);
			  if(trimLine != NULL)
			    {
			      PGS_MEM_Free(trimLine);
			      trimLine = NULL;
			    }
			  for(i=0; i<len0; i++)
			    {
			      if(tokens[i] != NULL)
				{
				  PGS_MEM_Free(tokens[i]);
				  tokens[i] = NULL;
				}
			    }

			  PGS_MEM_Free(start);
			  start = NULL;
			  return(NULL);
			}
		      strcpy(end,"");
		    }

		  for(i=0; i< (aline_len - strlen(tokens[0])+1); i++)
		    {
		      end[i] = '\0';
		    }
		  p = strstr(aline,SPLIT_REGEX);
		  if( p != NULL)
		    {
		      PGSt_integer firstEquals;
		      firstEquals = (PGSt_integer)((p) - (aline));

		      /* Create a string from the next character onwards.*/
		      strncpy(end, aline+firstEquals+strlen(SPLIT_REGEX), (aline_len - firstEquals));
		      /*end[firstEquals + strlen(SPLIT_REGEX)]='\0';*/

		      newp = PGS_MET_Trim(end, end);

		      strcpy(end, newp);
		    }
		}

	      if (PGS_MET_IsXmlElementStart(start))
		{
		  processedLine = PGS_MET_CreateXmlElementStart(end, processedLine, 
								InputPointer_flag);
		}
	      else if (PGS_MET_IsXmlElementEnd(start))
		{
		  processedLine = PGS_MET_CreateXmlElementEnd(end, processedLine, 
							      InputPointer_flag);
		}
	      else if (PGS_MET_IsNumVal(start))
		{
		  processedLine = PGS_MET_CreateNumValElement(end, processedLine);
		}
	      else if (PGS_MET_IsClass(start))
		{
		  processedLine = PGS_MET_CreateClassElement(end, processedLine);
		}
	      else if (PGS_MET_IsXmlElementValue(start))
		{
		  processedLine = PGS_MET_CreateValueElement(end, processedLine, 
							     InputPointer_flag);
		}
	      else if (PGS_MET_IsType(start))
		{
		  processedLine = PGS_MET_CreateTypeElement(end, processedLine);
		}
	      else if (PGS_MET_IsGroupType(start))
		{
		  processedLine = PGS_MET_CreateGroupTypeElement(end, processedLine);
		}
	      else
		{
		  processedLine = PGS_MET_CreateParameterElement(start, end, processedLine);
		}
	    }
	}
    }
  /* Free malloced */
  
  if(start != NULL)
    {
      PGS_MEM_Free(start);
      start = NULL;
    }

  if(end != NULL)
    {
      PGS_MEM_Free(end);
      end = NULL;
    }

  if(len0 > 0)
    {
      for(i=0; i<len0; i++)
	{
	  if(tokens[i] != NULL)
	    {
	      PGS_MEM_Free(tokens[i]);
	      tokens[i] = NULL;
	    }
	}
    }
  if(trimLine != NULL)
    {
      PGS_MEM_Free(trimLine);
      trimLine = NULL;
    }

  return ptr;
}

/*************************************************************************
 * Basic check for GROUP and OBJECT ODL entities
 ************************************************************************/

PGSt_integer PGS_MET_IsXmlElementStart(char *token)
{
  if ((PGS_MET_CaseInsensitiveStrcmp(token, odl_GROUP) == 0) || 
      (PGS_MET_CaseInsensitiveStrcmp(token, odl_OBJECT) == 0))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

/*************************************************************************
 * Basic check for END_GROUP and END_OBJECT ODL entities
 ************************************************************************/

PGSt_integer PGS_MET_IsXmlElementEnd(char *token)
{
  if ((PGS_MET_CaseInsensitiveStrcmp(token, odl_END_GROUP) == 0) || 
      (PGS_MET_CaseInsensitiveStrcmp(token, odl_END_OBJECT) == 0))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

/*************************************************************************
 * Basic check for NUM_VAL ODL entities
 ************************************************************************/

PGSt_integer PGS_MET_IsNumVal(char *token)
{
  if (PGS_MET_CaseInsensitiveStrcmp(token, odl_NUM_VAL) == 0)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

PGSt_integer PGS_MET_IsClass(char *token)
{
  if (PGS_MET_CaseInsensitiveStrcmp(token, odl_CLASS) == 0)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

PGSt_integer PGS_MET_IsXmlElementValue(char *token)
{
  if (PGS_MET_CaseInsensitiveStrcmp(token, odl_VALUE) == 0)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

PGSt_integer PGS_MET_IsGroupType(char *token)
{
  if (strcmp(token, odl_GROUPTYPE) == 0)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

PGSt_integer PGS_MET_IsType(char *token)
{
  if (PGS_MET_CaseInsensitiveStrcmp(token, odl_TYPE) == 0)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

/*************************************************************************
 * Creates the comment element for XML markup in the output file.
 *
 * @param token String representing the raw ODL comment
 * @return String representing the XML element start (i.e. <!-- xyz -->)
 * @see createXmlElementEnd(char *token)
 ************************************************************************/

char *PGS_MET_CreateXmlComment(char *comment, char *ret)
{
  char             *temp_comment=NULL;
  PGSt_integer     len=0;
  char             *ptr=NULL;
  PGSt_SMF_status  returnStatus=0;
  char *	   funcName = "PGS_MET_CreateXmlComment";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};
  int              i;

  ptr = ret;

  len = strlen(comment);

  if(temp_comment != NULL)
    {
      PGS_MEM_Free(temp_comment);
      temp_comment = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&temp_comment,(len + 10)*sizeof(char));
  for(i=0; i<(len + 10); i++) temp_comment[i]='\0';

  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(NULL);
    }
  strcpy(temp_comment,comment);
  
  strcpy(ret,COMMENT_START);
  
  /* If this line starts with a "/*" or a "*" removed it.*/
  if((temp_comment[0] == '/') && (temp_comment[1] == '*'))
    {
      len = strlen(temp_comment) -2;
      memmove(temp_comment, temp_comment+2, len);
    }
  
  /* If this line ends with a "star/" remove it.*/

  len = strlen(temp_comment);
  if((temp_comment[len] == '/') && (temp_comment[len-1] == '*'))
    {
      temp_comment [len-1] = '\0';
      temp_comment [len] = '\0';
    }
  strcat(ret, temp_comment);
  strcat(ret, COMMENT_END);
  return ptr;
}

/************************************************************************
 * Creates the start element for XML markup in the output file.  The end
 * element will also be created.
 *
 * @param token char *representing the XML element name (that will be
 *   included in the start markup i.e. <token>)
 * @return char *representing the XML element start (i.e. <token>)
 * @see createXmlElementEnd(char *token)
 ***********************************************************************/

char *PGS_MET_CreateXmlElementStart(char *token, char *ret, 
				    PGSt_integer *InputPointer_flag)
{
  char *ptr;

  ptr = ret;
  if(strcmp((char *)PGS_MET_StrToUpper(token), "INPUTPOINTER")== 0)
    {
      *InputPointer_flag = 1;
    }
  else
    {
      *InputPointer_flag = 0;
    }
  strcpy(ret, XML_START_START);
  strcat(ret, (char *)PGS_MET_StrToUpper(token));
  strcat(ret, XML_END);
  return ptr;
}

/************************************************************************
 * Creates the end element for XML markup in the output file.  The start
 * element was created before this call.
 *
 * @param token char *representing the XML element name (that will be
 *   included in the end markup i.e. </token>)
 * @return char *representing the XML element end (i.e. </token>)
 * @see createXmlElementStart(char *token)
 ***********************************************************************/
char *PGS_MET_CreateXmlElementEnd(char *token, char *ret, 
				  PGSt_integer *InputPointer_flag)
{
  char *ptr;

  ptr = ret;

  /* the InputPointer_flag has been used in processing (spliting) this attribute into
     indvidual filenames. The spliting using strtok is much faster for this attribute
     rather than using the original method in spliting string values by location first 
     and second quote for each filename. Once we are passed the END_OBJECT for INPUTPOINTER
     we set flag always to zero
  */
  *InputPointer_flag = 0;

  strcpy(ret, XML_END_START);
  strcat(ret, (char *)PGS_MET_StrToUpper(token));
  strcat(ret, XML_END);
  return ptr;
}

/***************************************************************************************
 * Creates the tag and the element for XML markup in the output file.
 * @param name representing the XML tag name and token representing the XML element name
 * @return representing the XML tag and the element (i.e. <name>token</name>) 
 **************************************************************************************/

char *PGS_MET_CreateParameterElement(char *name, char *token, char *ret)
{
  char *ptr;
  ptr = ret;

  strcpy(ret, XML_START_START);
  strcat(ret, (char *)PGS_MET_StrToUpper(name));
  strcat(ret, XML_END);
  strcat(ret, token);
  strcat(ret, XML_END_START);
  strcat(ret, (char *)PGS_MET_StrToUpper(name));
  strcat(ret, XML_END);
  return ptr;
}


/**********************************************************************************
 * Creates the NumVal element for XML markup in the output file.
 * @param representing the XML tag name and token representing the XML element name
 * @return representing the XML tag and the element (i.e. <name>token</name>)
 ***********************************************************************************/

char *PGS_MET_CreateNumValElement(char *token, char *ret)
{
  char *ptr;

  ptr = ret;

  strcpy(ret, NUM_VAL_START);
  strcat(ret,token);
  strcat(ret, NUM_VAL_END);
  return ptr;
}


/************************************************************************************
 * Trims the trainling AND heading " in the given string.
 *
 * @param s String to be trimmed
 * @return String the trimmed string
 ***********************************************************************************/

char *PGS_MET_TrimDoubleQuotes(char *s)
{
  char *temp=NULL;
  PGSt_integer len;
  PGSt_SMF_status  returnStatus;
  char *	   funcName = "PGS_MET_TrimDoubleQuotes";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  len = strlen(s);
  if(temp != NULL)
    {
      PGS_MEM_Free(temp);
      temp = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&temp,(len + 1 ) * sizeof(char));
  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(NULL);
    }
  if(s[0] == ('\"'))
    {
      strncpy(temp, s+1, len-2);
      temp[len-2]='\0';
      strcpy(s, temp);
    }

  PGS_MEM_Free(temp);
  return s;
}

char *PGS_MET_CreateClassElement(char *token, char *ret)
{
  char *ptr;
  ptr = ret;

  strcpy(ret, CLASS_START);
  token = (char *)PGS_MET_TrimDoubleQuotes(token);
  strcat(ret, token);
  strcat(ret, CLASS_END);
  return ptr;
}

char *PGS_MET_CreateValueElement(char *token, char *ret, 
				 PGSt_integer *InputPointer_flag)
{
  char             *ptr;
  PGSt_integer     max_elem_size;
  PGSt_integer     len;
  PGSt_integer     nvalues, nvalues0 ;
  char             *values[MAX_SPLIT_ELEM]  = {NULL};
  PGSt_SMF_boolean isString = FALSE;
  PGSt_integer     i,j;
  PGSt_integer     status;
  PGSt_integer     count=0;
  char             errMsg[128]=" ";
  PGSt_integer     len_token;
  char             *temp_token=NULL;
  PGSt_integer     len_values;
  PGSt_SMF_status  returnStatus;
  char *	   funcName = "PGS_MET_CreateValueElement";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  ptr = ret;
  token = PGS_MET_Trim(token, token);

  len = strlen(token);

  /* Remove parenthesis if present*/
  if(token[len-1] == CLOSE_PARENTHESIS)
    {
      token[len-1] = '\0';
    }
  if(token[0] == OPEN_PARENTHESIS)
    {
      token = token+1;
    }
  strcpy(ret,"");
  len_token = strlen(token);

  /* Is this a string type? If so, split by "," string*/
  if(strstr(token,"\"") !=NULL)
    {
      nvalues0 = PGS_MET_StringValueSplit(token, NULL, NULL, &max_elem_size, 
					  InputPointer_flag);
      
      for(i=0; i<nvalues0; i++)
	{
	  if(values[i] != NULL)
	    {
	      PGS_MEM_Free(values[i]);
	      values[i] = NULL;
	    }
	  returnStatus = PGS_MEM_Malloc((void **)&values[i],
					(max_elem_size+1) *sizeof(char));
	  if(returnStatus != PGS_S_SUCCESS)
	    {
	      /* error message is:
		 "Unable to allocate memory for the hdf attribute" */
	      
	      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				      0, errInserts);
	      return(NULL);
	    }
	  for(j=0; j<(max_elem_size+1); j++) values[i][j]='\0';
	}
      isString = TRUE;

      nvalues = PGS_MET_StringValueSplit(token, values, &nvalues0, 
					 &max_elem_size,InputPointer_flag);
    }
  else/* Else split by , character*/
    {
      nvalues0 = PGS_MET_Split(token, NON_STRING_DEL, NULL, NULL, &max_elem_size);

      for(i=0; i<nvalues0; i++)
	{
	  if(values[i] != NULL)
	    {
	      PGS_MEM_Free(values[i]);
	      values[i] = NULL;
	    }
	  returnStatus = PGS_MEM_Malloc((void **)&values[i],
					(max_elem_size+1) *sizeof(char));
	  if(returnStatus != PGS_S_SUCCESS)
	    {
	      /* error message is:
		 "Unable to allocate memory for the hdf attribute" */
	      
	      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
				      0, errInserts);
	      return(NULL);
	    }
	  for(j=0; j<(max_elem_size+1); j++) values[i][j]='\0';
	}

      isString = FALSE;
       nvalues = PGS_MET_Split(token, NON_STRING_DEL, values, &nvalues0, &max_elem_size);
    }

  if(values[0] != NULL)/* In the looop below the PGS_MET_ReplaceStr calls are slow 
			  for INPUTPOINTER when
			  there are hundreds of large filenames in it */
    {
      for(i = 0; i < nvalues; i++)
	{
	  len_values = strlen(values[i]);
	  if(len_values == 0) continue;

	  if(values[i][len_values - 1] == '\"' )
	    {
	      values[i][len_values - 1] = '\0';
	    }

	  if(values[i][0] == '\"' )
	    {
	      values[i] = values[i] + 1;
	    }

	  status = PGS_MET_ReplaceStr(values[i], values[i],AMPER , AMPERESC);	  
	  status = PGS_MET_ReplaceStr(values[i], values[i],LESS, LESSESC);
	  status = PGS_MET_ReplaceStr(values[i], values[i],GREAT , GREATESC);
	  status = PGS_MET_ReplaceStr(values[i], values[i],APOSTR , APOSTRESC);
	  
	  strcat(ret, VALUE_START);
	  
	  PGS_MET_TrimAllSpaces(values[i], values[i]);
	  
	  if(isString == FALSE)
	    {                   
	      /* Possibly reformat scientific notation*/
	       PGS_MET_ReformatSciNotation(values[i], values[i]);
	    }
	  strcat(ret, values[i]);
	  strcat(ret, VALUE_END);
	}
    }

  for(i=0; i<nvalues; i++)
    {
      PGS_MEM_Free(values[i]);
      values[i]=NULL;
    }

  return ptr;
}

PGSt_integer PGS_MET_MyFormatter(PGSt_double doubleValue, 
				 char *format, char *modified)
{
  return 0;
}


void PGS_MET_ReformatSciNotation(char *original, char *modified)
{
  PGSt_integer index;
  char     *hit;

  if((hit=strchr(original, 'e')) != NULL)
    {
      index = (int)((hit) - (original));
    }
  else
    {
      index = -1;
    }
  
  if (index == -1)
    {
      if((hit=strchr(original, 'E')) != NULL)
	{
	  index = (PGSt_integer)((hit) - (original));
	}
      else
	{
	  index = -1;
	}
    }
  
  if(index == -1)
    {
      /* This string is not in scientific notation, no translation
	 needed*/
      strcpy(modified, original);
    }
  else
    {
      /* Determine the number of digits there will be after converting to*/
      /* decimal notation*/
      PGSt_double     doubleValue;
      PGSt_integer    decimalIndex;
      char            *hit;
      char            *exponent;
      PGSt_integer    e;
      PGSt_integer    extraZeroes;
      PGSt_integer    sigDigits;

      PGSt_integer    maxPrecision = 17;/* Doubles have precision up to 
					   17 decimal places */
      char            *format=NULL;
      PGSt_integer    i;
      PGSt_SMF_status  returnStatus;

      doubleValue = (double)atof(original);
      	if ((hit = strstr(original, ".")) != NULL)
	  {
	    decimalIndex = (PGSt_integer)((hit) - (original));
	  }
	else
	  {
	    decimalIndex = 0;
	  }

	exponent = original + index + 1;
	e = atoi(exponent);

	extraZeroes =  fabs(e);
	sigDigits = index + extraZeroes;

	if (sigDigits > maxPrecision + extraZeroes - decimalIndex)
	  {
	    sigDigits = maxPrecision + extraZeroes - decimalIndex;
	    if (doubleValue < 0)
	      {
		sigDigits++;
	      }
	  }

	if(format != NULL)
	  {
	    PGS_MEM_Free(format);
	    format = NULL;
	  }
	returnStatus = PGS_MEM_Malloc((void **)&format,(sigDigits + 3)*sizeof(char));
	strcpy(format,"");

	/* If the number is less than one add a leading 0.*/
	if (doubleValue < fabs(1)) {
	  strcat (format, "0.");
	}

	for (i = 0; i < sigDigits; i++)
	  {
	    strcat(format, "#");
	  }
	
	/* format now looks something like this :*/
	/* 1.467e-04 yields format of 0.#######*/

	PGS_MET_MyFormatter(doubleValue, format, modified);

	/*
	DecimalFormat PGS_MET_MyFormatter = new DecimalFormat(format);
	modified = PGS_MET_MyFormatter.format(doubleValue);
	*/
	if (format != NULL)
	  {
	    PGS_MEM_Free(format);
	    format = NULL;
	  }
    }
}
    
PGSt_integer PGS_MET_StringValueSplit(char *token, char *values[], 
				      PGSt_integer *nvalues, 
				      PGSt_integer *max_elem_size, 
				      PGSt_integer *InputPointer_flag)
{

  /* Go to first quote.*/
  /* Next char is begining of value*/
  /* Go to next quote.*/
  /* If we hit a comma before next quote then value is between the two*/
  /* captured quotes.*/

  /* Else go to next quote.*/
  
  PGSt_SMF_boolean firstQuoteCaptured = FALSE;
  PGSt_SMF_boolean secondQuoteCaptured = FALSE;
  PGSt_integer     i,j;
  PGSt_integer     count=0;
  PGSt_integer     startIndex = 0;
  PGSt_integer     stopIndex = 0;
  char             *Values[MAX_SPLIT_ELEM] ={NULL};
  PGSt_SMF_status  returnStatus;
  char *	   funcName = "PGS_MET_StringValueSplit";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

   if(*InputPointer_flag == 1)/* we are spliting INPUTPOINTER with COMMA */
    {
      if(nvalues == NULL)
	{
	  *max_elem_size = 0;
	  count = PGS_MET_Split(token, ",", NULL, NULL, max_elem_size);
	  return(count);
	}
      else
	{
	  
	  for(i=0; i< *nvalues; i++)
	    {
	      if(Values[i] != NULL)
		{
		  PGS_MEM_Free(Values[i]);
		  Values[i] = NULL;
		}
	      returnStatus = PGS_MEM_Malloc((void **)&Values[i], (*max_elem_size+10)*sizeof(char));
	      if(returnStatus != PGS_S_SUCCESS)
		{
		  /* error message is:
		     "Unable to allocate memory for the hdf attribute" */
		  
		  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
					  0, errInserts);
		  return(PGSMET_E_MALLOC_ERR);
		}
	      for(j=0; j<(*max_elem_size+1); j++) Values[i][j]='\0';
	    }
	  
	  count = PGS_MET_Split(token, ",", Values, nvalues, max_elem_size);

	  for(i=0; i< *nvalues; i++)
	    {
	      strcpy(values[i],Values[i]);
	    }
	}
    }
  else
    {
      *max_elem_size = 0;
      for(i = 0; i < strlen(token); i++)
	{
	  if(token[i] == '\"')
	    {
	      if(firstQuoteCaptured == FALSE)
		{
		  firstQuoteCaptured = TRUE;
		  startIndex = i + 1;
		}
	      else
		{
		  secondQuoteCaptured = TRUE;
		  stopIndex = i;
		}
	      
	      
	      if(firstQuoteCaptured == TRUE && secondQuoteCaptured == TRUE)
		{
		  if((stopIndex - startIndex) > *max_elem_size)
		    {
		      *max_elem_size = (stopIndex - startIndex);
		    }

		  if(nvalues != NULL)
		    {
		      if(Values[count] != NULL)
			{
			  PGS_MEM_Free(Values[count]);
			  Values[count] = NULL;
			}
		      returnStatus = PGS_MEM_Malloc((void **)&Values[count], 
						    ((stopIndex - startIndex)+1)*sizeof(char));
		      if(returnStatus != PGS_S_SUCCESS)
			{
			  /* error message is:
			     "Unable to allocate memory for the hdf attribute" */
			  
			  (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
						  0, errInserts);
			  return(PGSMET_E_MALLOC_ERR);
			}
		      for(j=0; j<((stopIndex - startIndex)+1); j++) 
			{ 
			  Values[count][j]='\0';
			}
		      strncpy(Values[count], (token+startIndex), (stopIndex - startIndex));
		      strcpy(values[count],Values[count]);
		    }
		  count++;
		  
		  firstQuoteCaptured = FALSE;
		  secondQuoteCaptured = FALSE;
		}
	    }
	}
    }
 if(*InputPointer_flag != 1)
   {
  if(nvalues != NULL)
    {
      for(i=0; i<count; i++)
	{
	  if(Values[i] != NULL)
	    {
	      PGS_MEM_Free(Values[i]);
	      Values[i] = NULL;
	    }
	}
    }
   }
  return(count);
}

char *PGS_MET_CreateTypeElement(char *token, char *ret)
{
  char *ptr;

  ptr = ret;
  strcpy(ret,TYPE_START);
  token = (char *)PGS_MET_TrimDoubleQuotes(token);
  strcat(ret, token);
  strcat(ret, TYPE_END);
  return ret;
}

char *PGS_MET_CreateGroupTypeElement(char *token, char *ret)
{
  char *ptr;

  ptr = ret;
  strcpy(ret, GROUPTYPE_START);
  token = (char *)PGS_MET_TrimDoubleQuotes(token);
  strcat(ret, token);
  strcat(ret,GROUPTYPE_END);
  return ret;
}


/*********************************************************************************
 * Counting the number of special characters in the line.
 *
 * @param line: String need to be checked, match: character needs to be counted
 * @return number of appearance of the character in the line
 ********************************************************************************/

PGSt_integer PGS_MET_NumberOfChar(char *line, char match)
{
  PGSt_integer count = 0 ;
  PGSt_integer i;

  for (i =0; i< strlen(line); i++)
    {
      if (line[i] == match)
	count++;
    }
  
  return count;
}

/*********************************************************************************
 perform custom translation for tough elements 
*********************************************************************************/  
 
PGSt_integer PGS_MET_PerformCustomTranslation(char *temp_rawXml, char *rawXml)
{
  char              errMsg[128];
  char              *line = NULL;
  PGSt_integer      lenline; 
  FILE              *temp_in;
  FILE              *output;
  PGSt_SMF_status   returnStatus;
  char *	   funcName = "PGS_MET_PerformCustomTranslation";
  char *	   errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};

  if((temp_in = fopen( temp_rawXml, "r")) != NULL)
    {
      output = fopen( rawXml, "w");
    }
  else
    {
      sprintf(errMsg, "Cannot open temporary xml file %s!", temp_rawXml);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_OPEN,errMsg,"PGS_MET_PerformCustomTranslation");
      return(-1);
    }
  
  /* read temp_rawXml up to <GPOLYGONCONTAINER> or 
     <ASSOCIATEDPLATFORMINSTRUMENTSENSOR>       */
  
  /* Input pointer may contain 1000 or so filename. Since in the 
     temporary raw xml for inputpointer each filname will cotain 
     <VALUE></VALUE> pair, we allocate extra 15*MAX_SPLIT_ELEM 
     chars to largest sting length in ODL metadata */
  
  if(line != NULL)
    {
      PGS_MEM_Free(line);
      line = NULL;
    }
  returnStatus = PGS_MEM_Malloc((void **)&line,
				(ODLMAXSTMT + 15 * MAX_SPLIT_ELEM)*sizeof(char));
  line[0]='\0';
  line[1]='\0';

  if(returnStatus != PGS_S_SUCCESS)
    {
      /* error message is:
	 "Unable to allocate memory for the hdf attribute" */
      
      (void) PGS_MET_ErrorMsg(PGSMET_E_MALLOC_ERR, funcName,
			      0, errInserts);
      return(PGSMET_E_MALLOC_ERR);
    }

  while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
    {
      lenline = strlen(line);
      if(lenline > 0) line[lenline - 1] = '\0';

      if(strcmp(line, "<GPOLYGONCONTAINER>") == 0 || 
	 strcmp(line, "<ASSOCIATEDPLATFORMINSTRUMENTSENSOR>") == 0 )
	{
	  if(strcmp(line, "<GPOLYGONCONTAINER>") == 0)
	    {
	      PGS_MET_TranslateRingPoints(line, temp_in, output);
	    }
	  else if (strcmp(line, "<ASSOCIATEDPLATFORMINSTRUMENTSENSOR>") == 0 )
	    {
	      PGS_MET_TranslatePlatform(line, temp_in, output);
	    }
	}
      else
	{
	  fprintf(output,"%s\n",line);
	}
    }

  if(line != NULL)
    {
      free(line);
      line = NULL;
    }

  fclose(temp_in);
  fclose(output);
  return(PGS_S_SUCCESS);
}

PGSt_integer PGS_MET_TranslateRingPoints(char *line, FILE *temp_in, FILE *output)
{
  PGSt_integer     i,j,loop;
  PGSt_integer     lon_found, lat_found;
  PGSt_integer     count_lat,count_lon;
  PGSt_SMF_status  returnStatus;
  char             *ptr = NULL;
  char             lon[20][28];
  char             lat[20][28];
  char             errMsg[128];
  char             Msg[128];
  PGSt_integer     lenline; 


  if(strcmp(line, "<GPOLYGONCONTAINER>") == 0)
    {
      lenline = strlen(line);
      if(lenline > 0) line[lenline - 1] = '\0';
      fprintf(output,"%s\n","<GPOLYGONCONTAINER>");
      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
	{
	  lenline = strlen(line);
	  if(lenline > 0) line[lenline - 1] = '\0';
	  if(strcmp(line, "</GPOLYGONCONTAINER>") == 0)
	    {
	      fprintf(output,"%s\n","</GPOLYGONCONTAINER>");
	      break;
	    }
	  else
	    {
	      if(strcmp(line, "<GRINGPOINT>") != 0) fprintf(output,"%s\n",line);
	    }
	  if(strcmp(line, "<GRINGPOINT>") == 0)
	    {
	      lon_found = 0;
	      lat_found = 0;
	      count_lon = 0;
	      count_lat = 0;
	      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
		{
		  lenline = strlen(line);
		  if(lenline > 0) line[lenline - 1] = '\0';
		  if(strcmp(line, "</GRINGPOINT>") == 0) break;
		  if(strcmp(line, "<GRINGPOINTLONGITUDE>") == 0)
		    {
		      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
			{
			  lenline = strlen(line);
			  if(lenline > 0) line[lenline - 1] = '\0';
			  
			  if(line[0] == '<' &&
			     line[1] == 'V' &&
			     line[2] == 'A' &&
			     line[3] == 'L' &&
			     line[4] == 'U' &&
			     line[5] == 'E' &&
			     line[6] == '>' )
			    {
			      ptr = strtok(line,"</VALUE>");
			      if(ptr != NULL)
				{
				  strcpy(lon[0],ptr);
				  count_lon++;
				}
			      for(loop=1;loop<MAX_SPLIT_ELEM;loop++)
				{
				  ptr = strtok('\0',"</VALUE>");
				  if(ptr == NULL) break;
				  strcpy(lon[loop],ptr);
				  count_lon++;
				}
			      break;
			    }
			}
		      lon_found = 1;
		    }
		  if(strcmp(line, "<GRINGPOINTLATITUDE>") == 0)
		    {
		      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
			{
			  lenline = strlen(line);
			  if(lenline > 0) line[lenline - 1] = '\0';
			  if(line[0] == '<' &&
			     line[1] == 'V' &&
			     line[2] == 'A' &&
			     line[3] == 'L' &&
			     line[4] == 'U' &&
			     line[5] == 'E' &&
			     line[6] == '>' )
			    {
			      ptr = strtok(line,"</VALUE>");
			      if(ptr != NULL)
				{
				  strcpy(lat[0],ptr);
				  count_lat++;
				}
			      for(loop=1;loop<MAX_SPLIT_ELEM;loop++)
				{
				  ptr = strtok('\0',"</VALUE>");
				  if(ptr == NULL) break;
				  strcpy(lat[loop],ptr);
				  count_lat++;
				}
			      break;
			    }
			}
		      lat_found = 1;
		    }
		  if(lat_found == 1 && lon_found == 1)
		    {
		      if(count_lat > count_lon)
			{
			  for(j = 0; j<count_lon; j++)
			    {
			      fprintf(output,"%s\n","<GRINGPOINT>");
			      fprintf(output,"%s\n","<GRINGPOINTLATLON>");
			      fprintf(output,"%s\n","<POINT>");
			      fprintf(output,"%s%s%s\n","<GRINGPOINTLONGITUDE>",lon[j],
				      "</GRINGPOINTLONGITUDE>");
			      fprintf(output,"%s%s%s\n","<GRINGPOINTLATITUDE>",lat[j],
				      "</GRINGPOINTLATITUDE>");
			      fprintf(output,"%s\n","</POINT>");
			      fprintf(output,"%s\n","</GRINGPOINTLATLON>");
			      fprintf(output,"%s\n","</GRINGPOINT>");
			    }
			}
		      else
			{
			  for(j = 0; j<count_lat; j++)
			    {
			      fprintf(output,"%s\n","<GRINGPOINT>");
			      fprintf(output,"%s\n","<GRINGPOINTLATLON>");
			      fprintf(output,"%s\n","<POINT>");
			      fprintf(output,"%s%s%s\n","<GRINGPOINTLONGITUDE>",lon[j],
				      "</GRINGPOINTLONGITUDE>");
			      fprintf(output,"%s%s%s\n","<GRINGPOINTLATITUDE>",lat[j],
				      "</GRINGPOINTLATITUDE>");
			      fprintf(output,"%s\n","</POINT>");
			      fprintf(output,"%s\n","</GRINGPOINTLATLON>");
			      fprintf(output,"%s\n","</GRINGPOINT>");
			    }
			}
		      count_lat = 0;
		      lon_found = 0;
		    }
		  else
		    {
		      continue;
		    }
		}
	    }
	}
    }

  return (PGS_S_SUCCESS); 
}
    
/*********************************************************************** 
 * translate the platform element 
 * The following assumption is made in dealing with the Platform:
 * 1- There is at least one Instrument in each Platform.
 * 2- There is at least one Sensor in each Instrument.
 * 3- There is at most one OperationMode for each Instrument.
 * 4- Platform must have exactly one PlatformShortName.
 * 5- Instrument must have exactly one InstrumentShortName.
 * 6- All Sensors in the same Instrument must have the same OperationMode.
 * These assumptions are more strict than the DTD, but it fits the current
 * data model better. It would be nice to fix the DTD as well.
 ***********************************************************************/
    
PGSt_integer PGS_MET_TranslatePlatform(char *line, FILE *temp_in, FILE *output)
{
  Platforms    allPlatforms[MAXPLAT];
  PGSt_integer i,j,k;
  char         platformName[128];
  char         instrumentName[128];
  char         sensorName[128];
  PGSt_integer lenline;
  char         *ptr;
  PGSt_integer ptr_len;
  PGSt_integer line_len;
  PGSt_integer str_len;
  char         operationMode[128];
  PGSt_integer platformCount=0;
  PGSt_integer addNewPlatform;
  PGSt_integer addNewInstrument;
  PGSt_integer addNewSensor;

  for(i=0; i<MAXPLAT; i++)
    {
      strcpy(allPlatforms[i].platformName,"");
      allPlatforms[i].instrumentCount=0;

      for(j=0; j<MAXINST; j++)
	{
	  strcpy(allPlatforms[i].instrument[j].instrumentName,"");
	  strcpy(allPlatforms[i].instrument[j].operationMode,"");
	  allPlatforms[i].instrument[j].sensorCount=0;
	  for(k=0; k<MAXSENS; k++)
	    {
	      strcpy(allPlatforms[i].instrument[j].sensor[k].sensorName,"");
	    }
	}
    }

  if (strcmp(line, "<ASSOCIATEDPLATFORMINSTRUMENTSENSOR>") == 0 )
    {
      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
	{
	  /* initialize sensorName, instrumentName, platformName, and operationMode */
	  strcpy( sensorName, "");
	  strcpy( instrumentName, "");
	  strcpy( platformName, "");
	  strcpy( operationMode, "");

	  lenline = strlen(line);
	  if(lenline > 0) line[lenline - 1] = '\0';
	  if(strcmp(line, "</ASSOCIATEDPLATFORMINSTRUMENTSENSOR>") == 0 ) break;
	  if(strcmp(line, "<ASSOCIATEDPLATFORMINSTRUMENTSENSORCONTAINER>") == 0)
	    {
	      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
		{
		  lenline = strlen(line);
		  if(lenline > 0) line[lenline - 1] = '\0';
		  if(strcmp(line, 
			    "</ASSOCIATEDPLATFORMINSTRUMENTSENSORCONTAINER>") == 0) break;
		  if(strcmp(line, "<ASSOCIATEDSENSORSHORTNAME>") == 0)
		    {
		      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
			{  
			  lenline = strlen(line);
			  if(lenline > 0) line[lenline - 1] = '\0';
			  if(strcmp(line, "</ASSOCIATEDSENSORSHORTNAME>") == 0) break;
			  if(line[0] == '<' &&
			     line[1] == 'V' &&
			     line[2] == 'A' &&
			     line[3] == 'L' &&
			     line[4] == 'U' &&
			     line[5] == 'E' &&
			     line[6] == '>' )
			    {
			      ptr = strstr(line,"</VALUE>");
			      ptr_len = strlen(ptr);
			      line_len = strlen(line);
			      str_len = line_len - ptr_len - 7;
			      strncpy(sensorName, line+7,str_len); 
			      sensorName[str_len] = '\0';
			    }
			}
		    }
		  if(strcmp(line, "<ASSOCIATEDINSTRUMENTSHORTNAME>") == 0)
		    {
		      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
			{  
			  lenline = strlen(line);
			  if(lenline > 0) line[lenline - 1] = '\0';
			  if(strcmp(line, "</ASSOCIATEDINSTRUMENTSHORTNAME>") == 0) break;
			  if(line[0] == '<' &&
			     line[1] == 'V' &&
			     line[2] == 'A' &&
			     line[3] == 'L' &&
			     line[4] == 'U' &&
			     line[5] == 'E' &&
			     line[6] == '>' )
			    {
			      ptr = strstr(line,"</VALUE>");
			      ptr_len = strlen(ptr);
			      line_len = strlen(line);
			      str_len = line_len - ptr_len - 7;
			      strncpy(instrumentName, line+7,str_len); 
			      instrumentName[str_len] = '\0';
			    }
			}
		    }
		  else if (strcmp(line, "<ASSOCIATEDPLATFORMSHORTNAME>") == 0)
		    {
		      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
			{
			  lenline = strlen(line);
			  if(lenline > 0) line[lenline - 1] = '\0';
			  if(strcmp(line, "</ASSOCIATEDPLATFORMSHORTNAME>") == 0) break;
			  if(line[0] == '<' &&
			     line[1] == 'V' &&
			     line[2] == 'A' &&
			     line[3] == 'L' &&
			     line[4] == 'U' &&
			     line[5] == 'E' &&
			     line[6] == '>' )
			    {
			      ptr = strstr(line,"</VALUE>");
			      ptr_len = strlen(ptr);
			      line_len = strlen(line);
			      str_len = line_len - ptr_len - 7;
			      strncpy(platformName, line+7,str_len); 
			      platformName[str_len] = '\0';
			    }
			}
		    }
		  else if (strcmp(line, "<OPERATIONMODE>") == 0)
		    {
		      while ( (fgets(line, (ODLMAXSTMT - 1), temp_in)) != NULL)
			{
			  lenline = strlen(line);
			  if(lenline > 0) line[lenline - 1] = '\0';
			  if(strcmp(line, "</OPERATIONMODE>") == 0) break;
			  if(line[0] == '<' &&
			     line[1] == 'V' &&
			     line[2] == 'A' &&
			     line[3] == 'L' &&
			     line[4] == 'U' &&
			     line[5] == 'E' &&
			     line[6] == '>' )
			    {
			      ptr = strstr(line,"</VALUE>");
			      ptr_len = strlen(ptr);
			      line_len = strlen(line);
			      str_len = line_len - ptr_len - 7;
			      strncpy(operationMode, line+7,str_len); 
			      operationMode[str_len] = '\0';
			    }
			}
		    } 
		}

	      /* add platform, sensor, instrume, operation mode to the structure */

	      if(platformCount == 0)
		{
 		  strcpy(allPlatforms[0].platformName, platformName);
		  strcpy(allPlatforms[0].instrument[0].instrumentName, 
			 instrumentName);
		  strcpy(allPlatforms[0].instrument[0].sensor[0].sensorName, 
			 sensorName);
		  if(strcmp(operationMode, "") != 0) 
		    strcpy(allPlatforms[0].instrument[0].operationMode,operationMode);
		  platformCount = 1;
		  allPlatforms[0].instrumentCount = 1;
		  allPlatforms[0].instrument[0].sensorCount = 1;
		}
	      else
		{
		  addNewPlatform = 0;
		  for(i=0; i< platformCount; i++)
		    {
		      if(strcmp(allPlatforms[i].platformName, platformName) == 0)
			{
			  addNewPlatform = 1;
			  addNewInstrument = 0;

			  for(j=0; j< (allPlatforms[i].instrumentCount); j++)
			    {
			      if(strcmp(allPlatforms[i].instrument[j].instrumentName, 
					instrumentName) == 0 )
				{
				  addNewInstrument = 1;
				  addNewSensor = 0;
				  
				  if(addNewSensor == 0)
				    {
				      strcpy(allPlatforms[i].instrument[j].
					     sensor[allPlatforms[i].instrument[j].sensorCount].
					     sensorName, sensorName);
				      allPlatforms[i].instrument[j].sensorCount += 1;
				    }
				  break;
				}
			    }

			  if(addNewInstrument == 0)
			    {
			      strcpy(allPlatforms[i].instrument[allPlatforms[i].instrumentCount].instrumentName, instrumentName);
			      strcpy(allPlatforms[i].instrument[allPlatforms[i].instrumentCount].sensor[0].sensorName, sensorName);
			      if(strcmp(operationMode, "") != 0) 
				strcpy(allPlatforms[i].instrument[allPlatforms[i].instrumentCount].operationMode,operationMode);
			      allPlatforms[i].instrument[allPlatforms[i].instrumentCount].sensorCount = 1;
			      allPlatforms[i].instrumentCount += 1;

			    }
			  break;
			}
		    }
		  if(addNewPlatform == 0)
		    {
		      strcpy(allPlatforms[platformCount].platformName, platformName);
		      strcpy(allPlatforms[platformCount].instrument[0].instrumentName, 
			     instrumentName);
		      strcpy(allPlatforms[platformCount].instrument[0].sensor[0].sensorName,sensorName);
		      if(strcmp(operationMode, "") != 0) 
			strcpy(allPlatforms[platformCount].instrument[0].operationMode,operationMode);
		      allPlatforms[platformCount].instrumentCount = 1;
		      allPlatforms[platformCount].instrument[0].sensorCount = 1;
		      platformCount++;
		    }
		}
	    }
	}
    }

  if(platformCount > 0)
    {
      for(i=0; i< platformCount; i++)
	{
	  fprintf(output,"%s\n","<Platform>");
	  fprintf(output,"      %s%s%s\n","<PlatformShortName>",
		  allPlatforms[i].platformName,"</PlatformShortName>");
	  for(j=0; j< (allPlatforms[i].instrumentCount); j++)
	    {
	      fprintf(output,"      %s\n","<Instrument>");
	      fprintf(output,"        %s%s%s\n","<InstrumentShortName>",
		      allPlatforms[i].instrument[j].instrumentName,"</InstrumentShortName>");
	      
	      for(k=0; k< (allPlatforms[i].instrument[j].sensorCount); k++)
		{
		  fprintf(output,"        %s\n","<Sensor>");
		  fprintf(output,"          %s%s%s\n","<SensorShortName>",
			  allPlatforms[i].instrument[j].sensor[k].sensorName,"</SensorShortName>");
		  fprintf(output,"        %s\n","</Sensor>");
		}
	      if(strcmp(allPlatforms[i].instrument[j].operationMode, "") != 0)
		{
		  fprintf(output,"        %s%s%s\n","<OperationMode>",
			  allPlatforms[i].instrument[j].operationMode,"</OperationMode>");
		}
	      fprintf(output,"      %s\n","</Instrument>");
	    }
	  fprintf(output,"    %s\n","</Platform>");
	}
    }
  
  return(PGS_S_SUCCESS);
}

/****************************************************************************
 * To be called by PGS_MET_Write()
 ***************************************************************************/

PGSt_integer PGS_MET_ODLToXML(char *theOdlFilename, char *xmlFilename)
{
  PGSt_integer status;
  char         errMsg[128];
  char         *ptr=NULL;
  char         rawXml[512];
  char         temp_rawXml[512];
  char         dataCenterId[6];
  char         odlFilename[512]= " ";

  char         input[512];
  char         odlType[8];
  PGSt_integer type;
  char         cmd[PGSd_PC_FILE_PATH_MAX];

  /*
   * The constructor takes the following parameters:
   * theOdlFilename_: full odl file path
   * theHDFFilename_: full HDF file path
   * dataCenterID = SDPTK: data center id used as input parameter for the stylesheet
   */

  strcpy(dataCenterId, "SDPTK");
  strcpy(odlType,"SCIENCE");

  status = PGS_MET_Setup(theOdlFilename, odlFilename, 
			 temp_rawXml, rawXml, xmlFilename, 
			 odlType, &type);

  if(status == PGSIO_E_GEN_FILE_NOEXIST  || status == PGSIO_E_GEN_OPEN_RECL)
    {
      sprintf(errMsg,"Problem in PGS_MET_Setup()\n");
      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,errMsg,"PGS_MET_ODLToXML");
      return (-1);
    }
  
  status = PGS_MET_TranslateToRawXml(odlFilename, temp_rawXml);
    if(status == -1)
    {
      sprintf(errMsg,"Error: Problem in PGS_MET_TranslateToRawXml()\n");
      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,errMsg,"PGS_MET_ODLToXML");
      return (-1);
    }
  
  /* Perform custom translations that are not well-supported */
  /* via stylesheet.*/
  /* e.g GRINGPOINT */
  
  status = PGS_MET_PerformCustomTranslation(temp_rawXml, rawXml);
  if(status == -1)
    {
      sprintf(errMsg,"Error: Problem in PGS_MET_PerformCustomTranslation()\n");
      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,errMsg,"PGS_MET_ODLToXML");
      return (-1);
    }

  /* delete temp_rawXml file */
  sprintf(cmd,"/bin/rm -f %s",temp_rawXml);
  system(cmd);
  
  status = PGS_MET_TranslateToDplXml(xmlFilename, rawXml, dataCenterId, type);
  if(status == -1)
    {
      sprintf(errMsg,"Problem in PGS_MET_TranslateToDplXm.\n");
      PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,errMsg,"PGS_MET_ODLToXML");
      return (-1);
    }
  
  /* delete rawXml file */
  sprintf(cmd,"/bin/rm -f %s",rawXml);
  system(cmd);

  return(PGS_S_SUCCESS);
}

