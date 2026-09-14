/*
************************************************************************************************
**
** $Id: taiToUtc.c 2456 2007-06-30 01:24:26Z aychang $
**
** Filename:       getBuildId.cpp
**
** Program name:   getBuildId.
**
** Description:    Executable for running stand alone that gets BuildId string.
**
** Command-line parameters: argc[0] : getBuildId
**
** Executable return value: 0 : Success
**
** On success, puts to stdout : Buildid String (e.g, v1.01.02).
**
** References:
**
****************************************** Change log ******************************************
**
** Creator:        A. Y. Chang
** Creation date:  12/28/16
**
** Modification
**    Date:  12/28/16 	Developer: aychang
**    Description:      Initial implementation, Build 1.10.
**
************************************************************************************************
**
**              Copyright 2005, by the California Institute of Technology
**        ALL RIGHTS RESERVED. United States Government Sponsorship acknowledged.
**      Any commercial use must be negotiated with the Office of Technology Transfer 
**                       at the California Institute of Technology.
**
**        This software may be subject to U.S. export control laws and regulations.
**        By accepting this document, the user agrees to comply with all applicable 
**                          U.S. export laws and regulations.
**    User has the responsibility to obtain export licenses, or other export authority 
** as may be required before exporting such information to foreign countries or providing access 
**                                to foreign persons.
*************************************************************************************************
*/

#include <iostream>
#include "BuildId.hh"


int main(int argv, char* argc[])
{
    std::cout << sndr::BuildId::VERSION << std::endl;
    return 0;
}
