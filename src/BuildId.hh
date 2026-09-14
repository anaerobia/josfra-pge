/*
************************************************************************************************
**
** File Type:       C++ Header File
**
** Description:     Encapsulates the BuildId Version string. 
**
** References:
**
****************************************** Change log ******************************************
**
** Creator:        A. Tamayo
** Creation date:  2008-05-16
**
** Modification
**      Date: 2005-08-17    Developer: A. Y. Chang
**      Description:        Initial OCO implementation.
**
**      Date:  2020-02-12   Developer: username
**      Description:        Changed from const string to const char* for static initialization.
**
************************************************************************************************
**
**              Copyright 2008, by the California Institute of Technology
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
**
************************************************************************************************
*/

#ifndef BUILDID_HH
#define BUILDID_HH

#include <string>


namespace sndr
{
    class BuildId
    {
    public:
        static const char* VERSION;

    };
}

#endif
