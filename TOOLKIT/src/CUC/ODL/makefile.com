$!
$!*********************************************************************
$!$Component                                                          *
$!    odlc_common Makefile                                            *
$!$Abstract                                                           *
$!    Makefile for the PDS Toolbox ODLC common routines.              *
$!$Keywords                                                           *
$!    MAKE                                                            *
$!    SCRIPT                                                          *
$!$Inputs                                                             *
$!    what_to_build:                                                  *
$!        This parameter tells the Makefile which set of dependencies *
$!        it should build.                                            *
$!$Outputs                                                            *
$!    None                                                            *
$!$Returns                                                            *
$!    None                                                            *
$!$Detailed_Description                                               *
$!    This is the Makefile for the PDS Toolbox ODLC common routines.  *
$!    It builds the objcet files that form the ODLC, then creates the *
$!    odlc_lib.a archive.  Please note that environment variables are *
$!    used for referencing dependency files.  This is to allow        *
$!    developers to check out a routine and modify it in their home   *
$!    directory without having to manually edit this makefile.        *
$!$External_References                                                *
$!        Item                  Shared-Data             Access        *
$!     ----------------------------------------------------------     *
$!     A_NODES                   pds_symbols            Read          *
$!     FMTVALUE                  pds_symbols            Read          *
$!     LED2                      pds_symbols            Read          *
$!     LED2_EXE                  pds_symbols            Read          *
$!     LEXACT                    pds_symbols            Read          *
$!     LEXAN                     pds_symbols            Read          *
$!     MACHINE                   pds_symbols            Read          *
$!     ODLC_ARCHIVE              pds_symbols            Read          *
$!     OUTPUT                    pds_symbols            Read          *
$!     P_NODES                   pds_symbols            Read          *
$!     PARSER                    pds_symbols            Read          *
$!     PARSACT                   pds_symbols            Read          *
$!     PDS_INCLUDES              pds_symbols            Read          *
$!     PRTLABEL                  pds_symbols            Read          *
$!     RDLABEL                   pds_symbols            Read          *
$!     V_NODES                   pds_symbols            Read          *
$!     CVTVALUE                  pds_symbols            Read          *
$!     WRTLABEL                  pds_symbols            Read          *
$!     CLASSIFY                  pds_symbols            Read          *
$!     AO_NODES                  pds_symbols            Read          *
$!                                                                    *
$!$Author_and_Institution                                             *
$!    David P. Bernath / J.P.L.                                       *
$!$Version_and_Date                                                   *
$!    2.0   March 28, 1991                                            *
$!$Change_History                                                     *
$!    DPB   02-25-91   Original code.                                 *
$!    MDD   03-28-91   Ported to VMS, added CVTVALUE, CLASSIFY, and   * 
$!                     AO_NODES                                       *
$!*********************************************************************
$
$ on control_y then goto END
$ on error then continue
$ ! get the EOSDIS definition for ODLPrintError which will dump to 
$ ! IK_Syslog instead of to stderr.
$ ! IMS should be a logical which defines where your IK_Syslog.h header file 
$ ! can be found.
$ compile = "cc/nolist/define=(VAX, PDS_TOOLBOX, EOSDIS)/include=(PDS_INCLUDES,IMS)"
$ cpp = "cc/nolist/define=(VAX, PDS_TOOLBOX, EOSDIS)/include=(PDS_INCLUDES,IMS)/preprocess_only="
$
$ if p1 .eqs. "CPP" then goto cpp
$ if p1 .eqs. "ODLC_LIB" then goto lib
$ if p1 .nes. "ODLC_LIB" .and. p1 .nes. "ALL" then goto SINGLEFILE
$
$ 'compile' A_NODES/object=A_NODES
$ 'compile' AO_NODES/object=AO_NODES
$ 'compile' AG_NODES/object=AG_NODES
$ 'compile' COMMENTS/object=COMMENTS
$ 'compile' CVTVALUE/object=CVTVALUE
$ 'compile' P_NODES/object=P_NODES
$ 'compile' V_NODES/object=V_NODES
$ 'compile' LEXAN/object=LEXAN
$ 'compile' PARSER/object=PARSER
$ 'compile' PARSACT/object=PARSACT
$ 'compile' FMTVALUE/object=FMTVALUE
$ 'compile' OUTPUT/object=OUTPUT
$ 'compile' WRTLABEL/object=WRTLABEL
$ 'compile' RDLABEL/object=RDLABEL
$ 'compile' RDVALUE/object=RDVALUE
$ 'compile' PRTLABEL/object=PRTLABEL
$ 'compile' PRTSRC/object=PRTSRC
$ 'compile' WRTSRC/object=WRTSRC
$
$ lib:
$ library/create/log odlc_lib -
   A_NODES, -
   AO_NODES, -
   AG_NODES, -
   COMMENTS, -
   CVTVALUE, -
   P_NODES, -
   V_NODES, -
   LEXAN, -
   PARSER, -
   PARSACT, -
   FMTVALUE, -
   OUTPUT, -
   WRTLABEL, -
   RDLABEL, -
   RDVALUE, -
   PRTLABEL, -
   PRTSRC, -
   WRTSRC
$
$ goto END
$
$ SINGLEFILE:
$
$ 'compile' 'p1'/object='p1'
$ library/replace/log odlc_lib 'p1'
$ goto END
$
$ cpp:
$ 'cpp'A_NODES.cpp A_NODES.c
$ 'cpp'AO_NODES.cpp AO_NODES.c
$ 'cpp'AG_NODES.cpp AG_NODES.c
$ 'cpp'COMMENTS.cpp COMMENTS.c
$ 'cpp'CVTVALUE.cpp CVTVALUE.c
$ 'cpp'P_NODES.cpp P_NODES.c
$ 'cpp'V_NODES.cpp V_NODES.c
$ 'cpp'LEXAN.cpp LEXAN.c
$ 'cpp'PARSER.cpp PARSER.c
$ 'cpp'PARSACT.cpp PARSACT.c
$ 'cpp'FMTVALUE.cpp FMTVALUE.c
$ 'cpp'TOOLOUT.cpp TOOLOUT.c
$ 'cpp'OUTPUT.cpp OUTPUT.c
$ 'cpp'WRTLABEL.cpp WRTLABEL.c
$ 'cpp'RDLABEL.cpp RDLABEL.c
$ 'cpp'RDVALUE.cpp RDVALUE.c
$ 'cpp'PRTLABEL.cpp PRTLABEL.c
$ 'cpp'PRTSRC.cpp PRTSRC.c
$ 'cpp'WRTSRC.cpp WRTSRC.c
$
$ END:
$ exit
