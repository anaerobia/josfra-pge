/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2009, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_XslProcessor.c
 
DESCRIPTION:
         The file contains PGS_MET_XslProcessor.c
	 The function is used to transform raw xml input file to dpl xml output file

AUTHOR:
  	Abe Taaheri / Raytheon

HISTORY:
  	2-May-2009 	AT 	Initial version

END_FILE_PROLOG:
***************************************************************************/

/**************************************************************************
 * Transform raw xml input file to dpl xml output file, using stylesheet.
 *
 * @param inputXmlFile - raw XML input file
 * @param outputXmlFile - DPL XML output file - result of transformation
 * @param parms Properties - Parameters to be passed in to stylesheet
 *                           during the transformation
 *************************************************************************/

/*----- includes ------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ATT_NAMES_DEFINED
#include <PGS_MET.h>
#include <PGS_IO_1.h>

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>



extern int xmlLoadExtDtdDefaultValue;
/*extern void logger_log(char *str);*/

PGSt_SMF_status PGS_MET_XslProcessor(char *stylesheet, 
				     char *Raw_XMLFile, 
				     char *OUT_XMLFile, 
				     ssProperty styleSheetParams, 
				     PGSt_integer nbparams)
{
  int               i;
  xsltStylesheetPtr *stylesheetsPtr = NULL; /* struct in memory that contains 
					       the stylesheet tree and other 
					       information about the stylesheet
					    */
  xmlDocPtr    inputXmlFile, outputXmlFile; /* structS in memory that contains 
					    the document tree for input xml
					    and output xml */
  const char   *params[16 + 1]; /* params can be used to pass XSLT parameters 
				 to the stylesheet. It is a NULL-terminated 
				 array of name/value pairs of const char's.*/
  char         *parameters[16 + 1]; 
  int          arg_indx;
  int          params_indx = 0;
  FILE         *output_file = NULL; /* pointer for OUT_XMLFile */
  xmlDocPtr    *files = NULL;
  int          return_value = 0;
  char         errMsg[128]=" ";

  /*logger_log("Transforming Raw Xml to DPL Xml.... \n");*/

  parameters[0] = (char *)malloc(13*sizeof(char));
  parameters[1] = (char *)malloc(13*sizeof(char));

  /*
  parameters[0] = "DataCenterId";
  parameters[1] = styleSheetParams.DataCenterId;
  */

  strcpy(parameters[0],"DataCenterId");
  strcpy(parameters[1],styleSheetParams.DataCenterId);

  /*
  parameters[2] = "SdsrvDbID";
  parameters[3] = styleSheetParams.SdsrvDbID;
  parameters[4] = "SizeOfGranule";
  parameters[5] = styleSheetParams.SizeOfGranule;
  parameters[6] = styleSheetParams.DataCenterId;
  parameters[7] = styleSheetParams.ProductionDateTime;
  parameters[7] = "2009-04-23";
  parameters[8] = "InsertTime";
  parameters[9] = styleSheetParams.InsertTime;
  parameters[9] = "2009-04-23";
  parameters[10] = "UpdateTime";
  parameters[11] = styleSheetParams.UpdateTime;
  parameters[11] = "2009-04-23";
  parameters[12] = "EmptyBrowse";
  parameters[13] = styleSheetParams.EmptyBrowse;
  parameters[12] = 0;
  */

  files = (xmlDocPtr *) calloc(1, sizeof(xmlDocPtr));
  stylesheetsPtr = (xsltStylesheetPtr *) calloc(1, sizeof(xsltStylesheetPtr));

  /*
  for(arg_indx=0; arg_indx<(nbparams; arg_indx++)
    {
      params[params_indx++] = parameters[arg_indx];

      if (params_indx >= 16) {
	fprintf(stderr, "too many params\n");
	fprintf(logger, "too many params\n");
	return_value = -1;
	goto finish;
      }
    }
    params[params_indx] = NULL;
  */

  params[0] = parameters[0];
  params[1] = parameters[1];
  params[nbparams] = NULL;

  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;

  /*Pars StylesheetFile */
  stylesheetsPtr[0] = xsltParseStylesheetFile((const xmlChar *)stylesheet);

  /* Parsed Raw_XMLFile */
  files[0] = xmlParseFile(Raw_XMLFile);

  inputXmlFile = files[0];
  outputXmlFile = inputXmlFile;

  /* Entering xsltApplyStylesheet ....*/
  outputXmlFile = xsltApplyStylesheet(stylesheetsPtr[0], inputXmlFile, params);

  xmlFreeDoc(inputXmlFile);
  inputXmlFile = outputXmlFile;

  output_file = fopen(OUT_XMLFile, "w");
  if(output_file == NULL)
    {
      sprintf(errMsg, "Cannot open file %s!", OUT_XMLFile);
      PGS_SMF_SetDynamicMsg(PGSIO_E_GEN_OPEN,errMsg,"PGS_MET_XslProcessor");
      return(-1);
    }

  xsltSaveResultToFile(output_file, outputXmlFile, stylesheetsPtr[0]);
  xmlFreeDoc(outputXmlFile);

  fclose(output_file);

  xsltFreeStylesheet(stylesheetsPtr[0]);
  
  xsltCleanupGlobals();
  xmlCleanupParser();

 finish:
  free(parameters[0]);
  free(parameters[1]);
  free(stylesheetsPtr);
  free(files);
  return(return_value);
}
