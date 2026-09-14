/***********************************************************************************************
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
**
** File Type:       C++ Source File
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
**      Date: 2008-05-16    Developer: A. Tamayo
**      Description:        Build ID version set for first PEATE delivery to v2_0_0.
**
**      Date: 2008-07-08    Developer: A. Tamayo
**      Description:        Build ID version updated to v2_0_1 for PGE version testing.
**
**      Date: 2009-11-10    Developer: A. Tamayo
**      Description:        Build ID version updated to v4_0_0 for new SPSS release.
**
**      Date: 2010-03-23    Developer: A. Tamayo
**      Description:        Build ID version updated to v4_0_1 for new SPSS release.
**
**      Date: 2010-06-29    Developer: A. Tamayo
**      Description:        Build ID version updated to v5_0_0 for new SPSS release.
**
**      Date: 2010-08-02    Developer: A. Tamayo
**      Description:        Build ID version updated to v6_0_0 for new SPSS release.
**
**      Date: 2010-09-29    Developer: A. Tamayo
**      Description:        Build ID version updated to v7_0_0 for new SPSS release.
**
**      Date: 2010-10-06    Developer: A. Tamayo
**      Description:        Build ID version fixed back to v6_0_0 for new SPSS release.
**
**      Date: 2010-10-12    Developer: A. Tamayo
**      Description:        Build ID version updated to v7_0_0 for new SPSS release.
**
**      Date: 2011-03-09    Developer: A. Tamayo
**      Description:        Build ID version updated to v6_0_1 for new SPSS release.
**
**      Date: 2010-03-10    Developer: A. Tamayo
**      Description:        Build ID version updated to v7_0_0 for new SPSS release.
**
**      Date: 2011-07-05    Developer: A. Tamayo
**      Description:        Build ID version updated to v7_0_1 for new SPSS release.
**
**      Date: 2011-09-08    Developer: A. Tamayo
**      Description:        Build ID version updated to v8_0_0 for new SPSS release.
**
**      Date: 2011-11-14    Developer: I. Tkatcheva
**      Description:        Build ID version updated to v8_0_1 for new SPSS release.
**
**      Date: 2012-01-30    Developer: I. Tkatcheva
**      Description:        Build ID version updated to v8_0_2 for new SPSS release.
**
**      Date: 2012-04-18    Developer: A. Tamayo
**      Description:        Build ID version updated to v8_0_3 for new SPSS release.
**
**      Date: 2012-04-19    Developer: A. Tamayo
**      Description:        Build ID version updated to v9_0_0 for new SPSS release.
**
**      Date: 2016-12-07    Developer: v. Myers
**      Description:        Build ID version v9_0_0 format to v01.07.00 for new SPSS release.
**
**      Date: 2016-12-12    Developer: v. Myers
**      Description:        Build ID  v01.08.00 for new SPSS release SPDC and GESC release.
**
**      Date: 2017-01-17    Developer: v. Myers
**      Description:        Build ID  v01.09.00 for new SPSS release SPDC and GESC release.
**
**      Date: 2017-01-24    Developer: v.Myers 
**      Description:        Build ID  v01.10.00 for new SPSS release SPDC and GESC release.
**
**      Date: 2017-02-16    Developer: v.Myers 
**      Description:        Build ID for v01.11.00                         
**
**      Date: 2017-02-16    Developer: v.Myers 
**      Description:        Build ID for v01.12.00                         
**
**      Date: 2017-05-02    Developer: v.Myers 
**      Description:        Build ID for v01.13.00                         
**
**      Date: 2017-06-23    Developer: v.Myers 
**      Description:        Build ID for v01.14.00                         
**
**      Date: 2017-08-07    Developer: v.Myers 
**      Description:        Build ID for v01.15.00                         
**
**      Date: 2017-08-20    Developer: v.Myers 
**      Description:        Build ID for v02.00.00 
** 
**      Date: 2017-09-22    Developer: L.Ly-Hollins
**      Description:        Build ID for v02.01.00
**
**      Date: 2017-09-22    Developer: R.Henao
**      Description:        Build ID for v02.02.00
**
**      Date: 2017-10-19    Developer: R.Henao
**      Description:        Build ID for v02.03.00
**
**      Date: 2017-11-29    Developer: R.Henao
**      Description:        Build ID for v02.04.00
**
**      Date: 2018-01-26    Developer: R.Henao
**      Description:        Build ID for v02.05.00
**
**      Date: 2018-02-20    Developer: R.Henao
**      Description:        Build ID for v01.21.00
**
**      Date: 2018-02-22    Developer: R.Henao
**      Description:        Build ID for v01.21.01
**
**      Date: 2018-03-13    Developer: R.Henao
**      Description:        Build ID for v01.22.00
**
**      Date: 2018-03-19    Developer: R.Henao
**      Description:        Build ID for v01.23.00
**
**      Date: 2018-03-22    Developer: R.Henao
**      Description:        Build ID for v01.23.01
**
**      Date: 2018-03-27    Developer: A.Said
**      Description:        Build ID for v01.24.00
**
**      Date: 2018-04-05    Developer: A.Said
**      Description:        Build ID for v01.25.00
**
**      Date: 2018-04-06    Developer: A.Said
**      Description:        Build ID for v01.25.01
**
**      Date: 2018-04-26    Developer: R.Henao
**      Description:        Build ID for v01.26.00
**
**      Date: 2018-05-11    Developer: A.Said
**      Description:        Build ID for v01.27.00
**
**      Date: 2018-05-14    Developer: A.Said
**      Description:        Build ID for v01.27.00
**
**      Date: 2018-05-24    Developer: R.Henao
**      Description:        Build ID for v02.09.00
**
**      Date: 2018-06-04    Developer: R.Henao
**      Description:        Build ID for v01.28.00
**
**      Date: 2018-08-23    Developer: R.Henao
**      Description:        Build ID for v02.10.00
**
**      Date: 2018-09-11    Developer: R.Henao
**      Description:        Build ID for v02.10.01
**
**      Date: 2018-10-17    Developer: R.Henao
**      Description:        Build ID for v02.11.00
**
**      Date: 2018-10-29    Developer: R.Henao
**      Description:        Build ID for v02.11.01
**
**      Date: 2018-11-08    Developer: R.Henao
**      Description:        Build ID for v02.12.00
**
**      Date: 2018-11-30    Developer: R.Henao
**      Description:        Build ID for v02.11.02
**
**      Date: 2018-11-30    Developer: R.Henao
**      Description:        Build ID for v02.13.00
**
**      Date: 2019-01-23    Developer: DaniloAguilar
**      Description:        Build ID for v01.30.00
**
**      Date: 2019-02-07    Developer: DaniloAguilar
**      Description:        Build ID for v01.31.00
**
**      Date: 2019-02-26    Developer: D.Aguilar
**      Description:        Build ID for v02.15.00
**
**      Date: 2019-03-04    Developer: D.Aguilar
**      Description:        Build ID for v02.16.00
**
**      Date: 2019-03-04    Developer: D.Aguilar
**      Description:        Build ID for v01.32.00
**
**      Date: 2019-03-06    Developer: D.Aguilar
**      Description:        Build ID for v01.32.00
**
**      Date: 2019-03-06    Developer: D.Aguilar
**      Description:        Build ID for v02.16.00
**
**      Date: 2019-03-12    Developer: D.Aguilar
**      Description:        Build ID for v01.33.00
**
**      Date: 2019-03-06    Developer: D.Aguilar
**      Description:        Build ID for v02.16.00
**
**      Date: 2019-04-19    Developer: D.Aguilar
**      Description:        Build ID for v02.17.00
**
**      Date: 2019-05-20    Developer: D.Aguilar
**      Description:        Build ID for v01.34.00
**
**      Date: 2019-04-19    Developer: D.Aguilar
**      Description:        Build ID for v02.17.00
**
**      Date: 2019-05-29    Developer: D.Aguilar
**      Description:        Build ID for v02.19.00
**
**      Date: 2019-06-18    Developer: D.Aguilar
**      Description:        Build ID for v02.20.00
**
**      Date: 2019-06-25    Developer: D.Aguilar
**      Description:        Build ID for v02.20.01
**
**      Date: 2019-08-07    Developer: D.Aguilar
**      Description:        Build ID for v02.21.00
**
**      Date: 2019-08-13    Developer: D.Aguilar
**      Description:        Build ID for v02.21.01
**
**      Date: 2019-08-15    Developer: D.Aguilar
**      Description:        Build ID for v01.35.00
**
**      Date: 2019-08-15    Developer: D.Aguilar
**      Description:        Build ID for v02.21.01
**
**      Date: 2019-08-22    Developer: D.Aguilar
**      Description:        Build ID for v02.22.00
**
**      Date: 2019-08-27    Developer: D.Aguilar
**      Description:        Build ID for v01.35.01
**
**      Date: 2019-08-27    Developer: D.Aguilar
**      Description:        Build ID for v02.22.00
**
**      Date: 2019-09-04    Developer: D.Aguilar
**      Description:        Build ID for v02.23.00
**
**      Date: 2019-09-10    Developer: D.Aguilar
**      Description:        Build ID for v02.23.01
**
**      Date: 2019-09-24    Developer: D.Aguilar
**      Description:        Build ID for v02.24.00
**
**      Date: 2019-10-22    Developer: R.Henao
**      Description:        Build ID for v02.25.00
**
**      Date: 2019-11-01    Developer: D.Aguilar
**      Description:        Build ID for v01.36.00
**
**      Date: 2019-11-01    Developer: D.Aguilar
**      Description:        Build ID for v02.25.00
**
**      Date: 2019-11-05    Developer: D.Aguilar
**      Description:        Build ID for v02.27.00
**
**      Date: 2019-11-21    Developer: D.Aguilar
**      Description:        Build ID for v02.28.00
**
**      Date: 2019-11-26    Developer: D.Aguilar
**      Description:        Build ID for v02.28.01
**
**      Date: 2019-12-18    Developer: D.Aguilar
**      Description:        Build ID for v01.37.00
**
**      Date: 2019-12-18    Developer: D.Aguilar
**      Description:        Build ID for v02.28.01
**
**      Date: 2019-12-24    Developer: D.Aguilar
**      Description:        Build ID for v02.28.02
**
**      Date: 2020-01-16    Developer: D.Aguilar
**      Description:        Build ID for v01.37.01
**
**      Date: 2020-01-16    Developer: D.Aguilar
**      Description:        Build ID for v02.28.02
**
**      Date: 2020-01-23    Developer: D.Aguilar
**      Description:        Build ID for v01.37.02
**
**      Date: 2020-01-23    Developer: D.Aguilar
**      Description:        Build ID for v02.28.02
**
**      Date: 2020-02-19    Developer: D.Aguilar
**      Description:        Build ID for v03.00.00
**
**      Date: 2020-02-26    Developer: D.Aguilar
**      Description:        Build ID for v02.29.00
**
**      Date: 2020-03-03    Developer: D.Aguilar
**      Description:        Build ID for v02.29.01
**
**      Date: 2020-03-03    Developer: D.Aguilar
**      Description:        Build ID for v02.30.00
**
**      Date: 2020-03-11    Developer: D.Aguilar
**      Description:        Build ID for v03.00.01
**
**      Date: 2020-03-17    Developer: D.Aguilar
**      Description:        Build ID for v02.31.00
**
**      Date: 2020-03-19    Developer: aychang
**      Description:        Changed from const string to char* for static initialzation.
**
**      Date: 2020-03-31    Developer: D.Aguilar
**      Description:        Build ID for v01.38.00
**
**      Date: 2020-04-14    Developer: D.Aguilar
**      Description:        Build ID for v03.03.00
**
**      Date: 2020-04-14    Developer: D.Aguilar
**      Description:        Build ID for v01.38.01
**
**      Date: 2020-04-29    Developer: D.Aguilar
**      Description:        Build ID for v01.38.02
**
**      Date: 2020-04-29    Developer: D.Aguilar
**      Description:        Build ID for v02.32.00
**
**      Date: 2020-05-19    Developer: D.Aguilar
**      Description:        Build ID for v02.32.01
**
**      Date: 2020-05-19    Developer: D.Aguilar
**      Description:        Build ID for v03.03.01
**
**      Date: 2020-05-20    Developer: D.Aguilar
**      Description:        Build ID for v01.39.00
**
**      Date: 2020-05-21    Developer: D.Aguilar
**      Description:        Build ID for v02.33.00
**
**      Date: 2020-06-01    Developer: D.Aguilar
**      Description:        Build ID for v02.34.00
**
**      Date: 2020-06-03    Developer: D.Aguilar
**      Description:        Build ID for v02.32.02
**
**      Date: 2020-07-01    Developer: D.Aguilar
**      Description:        Build ID for v03.06.00
**
**      Date: 2020-07-06    Developer: D.Aguilar
**      Description:        Build ID for v01.40.00
**
**      Date: 2020-07-08    Developer: D.Aguilar
**      Description:        Build ID for v03.06.01
**
**      Date: 2020-07-15    Developer: D.Aguilar
**      Description:        Build ID for v02.35.00
**
**      Date: 2020-07-16    Developer: D.Aguilar
**      Description:        Build ID for v03.07.00
**
**      Date: 2020-07-20    Developer: D.Aguilar
**      Description:        Build ID for v02.35.01
**
**      Date: 2020-08-03    Developer: D.Aguilar
**      Description:        Build ID for v03.08.00
**
**      Date: 2020-08-10    Developer: D.Aguilar
**      Description:        Build ID for v02.38.00
**
**      Date: 2020-08-17    Developer: D.Aguilar
**      Description:        Build ID for v03.08.01
**
**      Date: 2020-09-10    Developer: D.Aguilar
**      Description:        Build ID for v01.39.00
**
**      Date: 2020-09-10    Developer: D.Aguilar
**      Description:        Build ID for v02.37.00
**
**      Date: 2020-09-10    Developer: D.Aguilar
**      Description:        Build ID for v01.39.01
**
**      Date: 2020-09-16    Developer: D.Aguilar
**      Description:        Build ID for v02.37.01
**
**      Date: 2020-09-18    Developer: D.Aguilar
**      Description:        Build ID for v02.39.00
**
**      Date: 2020-09-29    Developer: D.Aguilar
**      Description:        Build ID for v01.40.01
**
**      Date: 2020-10-01    Developer: D.Aguilar
**      Description:        Build ID for v03.08.02
**
**      Date: 2020-10-01    Developer: D.Aguilar
**      Description:        Build ID for v02.40.00
**
**      Date: 2020-10-01    Developer: D.Aguilar
**      Description:        Build ID for v01.39.02
**
**      Date: 2020-11-12    Developer: D.Aguilar
**      Description:        Build ID for v02.41.00
**
**      Date: 2020-12-03    Developer: R.Henao
**      Description:        Build ID for v02.43.00
**
**      Date: 2020-12-07    Developer: C. Cordell
**      Description:        Build ID for v02.44.00
**
**      Date: 2020-12-17    Developer: C. Cordell
**      Description:        Build ID for v02.45.00
**
**      Date: 2021-01-07    Developer: C. Cordell
**      Description:        Build ID for v02.46.00
**
**      Date: 2021-01-12    Developer: C. Cordell
**      Description:        Build ID for v02.48.00
**
**      Date: 2021-01-19    Developer: C. DSouza
**      Description:        Build ID for v02.47.00
**
**      Date: 2021-01-27    Developer: C. DSouza
**      Description:        Build ID for v02.50.00
**
**      Date: 2021-01-27    Developer: C. DSouza
**      Description:        Build ID for v03.15.00
**
**      Date: 2021-02-05    Developer: C. DSouza
**      Description:        Build ID for v02.49.00
**
**      Date: 2021-02-23    Developer: C. DSouza
**      Description:        Build ID for v03.16.00
**
**      Date: 2021-03-11    Developer: C. DSouza
**      Description:        Build ID for v01.41.00
**
**      Date: 2021-03-16    Developer: C. DSouza
**      Description:        Build ID for v03.18.00
**
**      Date: 2021-04-13    Developer: C. DSouza
**      Description:        Build ID for spdc_v001
**
**      Date: 2021-04-28    Developer: I. Tkatcheva
**      Description:        Set Build ID to a generic string in the expected format
**
**      Date: 2021-05-10    Developer: C. DSouza
**      Description:        Build ID for 01.42.00
**
**      Date: 2021-05-12    Developer: C. DSouza
**      Description:        Build ID for v01.42.00
**
**      Date: 2021-05-12    Developer: C. DSouza
**      Description:        Build ID for v01.42.00
**
**      Date: 2021-08-06    Developer: C. DSouza
**      Description:        Build ID for v02.52.00
**
**      Date: 2021-08-19    Developer: C. DSouza
**      Description:        Build ID for v01.43.00
**
**      Date: 2022-03-08    Developer: C. DSouza
**      Description:        Build ID for spdc_v004
**
**      Date: 2022-03-22    Developer: C. DSouza
**      Description:        Build ID for spdc_v005
**
**      Date: 2022-03-22    Developer: C. DSouza
**      Description:        Build ID for v03.19.00
**
**      Date: 2022-04-18    Developer: C. DSouza
**      Description:        Build ID for v03.20.00
**
**      Date: 2022-04-25    Developer: C. DSouza
**      Description:        Build ID for v02.55.00
**
**      Date: 2022-05-03    Developer: C. DSouza
**      Description:        Build ID for v03.20.01
**
**      Date: 2022-05-19    Developer: C. DSouza
**      Description:        Build ID for v02.56.00
**
**      Date: 2022-05-26    Developer: C. DSouza
**      Description:        Build ID for v02.57.00
**
**      Date: 2022-05-26    Developer: C. DSouza
**      Description:        Build ID for v03.20.02
**
**      Date: 2022-06-28    Developer: C. DSouza
**      Description:        Build ID for v02.57.01
**
**      Date: 2022-07-12    Developer: C. DSouza
**      Description:        Build ID for v02.58.00
**
**      Date: 2022-07-26    Developer: C. DSouza
**      Description:        Build ID for v02.59.00
**
**      Date: 2022-08-02    Developer: C. DSouza
**      Description:        Build ID for v03.21.00
**
**      Date: 2022-08-24    Developer: C. DSouza
**      Description:        Build ID for v02.60.00
**
**      Date: 2022-09-13    Developer: C. DSouza
**      Description:        Build ID for spdc_v012
**
**      Date: 2022-09-19    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2022-09-19    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2022-09-19    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2022-10-04    Developer: C. DSouza
**      Description:        Build ID for v02.61.00
**
**      Date: 2022-10-05    Developer: C. DSouza
**      Description:        Build ID for spdc_v013
**
**      Date: 2022-10-11    Developer: C. DSouza
**      Description:        Build ID for spdc_v014
**
**      Date: 2022-11-17    Developer: C. DSouza
**      Description:        Build ID for spdc_v015
**
**      Date: 2022-11-29    Developer: C. DSouza
**      Description:        Build ID for spdc_v016
**
**      Date: 2022-11-29    Developer: C. DSouza
**      Description:        Build ID for spdc_v016
**
**      Date: 2022-12-08    Developer: C. DSouza
**      Description:        Build ID for v02.63.00
**
**      Date: 2023-01-11    Developer: C. DSouza
**      Description:        Build ID for v02.64.00
**
**      Date: 2023-01-11    Developer: C. DSouza
**      Description:        Build ID for spdc_v017
**
**      Date: 2023-01-25    Developer: C. DSouza
**      Description:        Build ID for v02.65.00
**
**      Date: 2023-01-30    Developer: C. DSouza
**      Description:        Build ID for v03.22.00
**
**      Date: 2023-02-05    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-02-05    Developer: C. DSouza
**      Description:        Build ID for v03.23.00
**
**      Date: 2023-02-07    Developer: C. DSouza
**      Description:        Build ID for v00.00.01
**
**      Date: 2023-02-15    Developer: C. DSouza
**      Description:        Build ID for v00.00.05
**
**      Date: 2023-02-17    Developer: C. DSouza
**      Description:        Build ID for v02.66.00
**
**      Date: 2023-03-01    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-03-01    Developer: C. DSouza
**      Description:        Build ID for v00.00.01
**
**      Date: 2023-03-01    Developer: C. DSouza
**      Description:        Build ID for v03.24.00
**
**      Date: 2023-03-09    Developer: C. DSouza
**      Description:        Build ID for spdc_v018
**
**      Date: 2023-04-20    Developer: C. DSouza
**      Description:        Build ID for v02.67.00
**
**      Date: 2023-06-01    Developer: C. DSouza
**      Description:        Build ID for v02.68.00
**
**      Date: 2023-06-27    Developer: C. DSouza
**      Description:        Build ID for v02.69.00
**
**      Date: 2023-07-06    Developer: C. DSouza
**      Description:        Build ID for v03.25.00
**
**      Date: 2023-07-24    Developer: C. DSouza
**      Description:        Build ID for v02.71.00
**
**      Date: 2023-08-21    Developer: C. DSouza
**      Description:        Build ID for v02.70.00
**
**      Date: 2023-08-28    Developer: C. DSouza
**      Description:        Build ID for spdc_v027
**
**      Date: 2023-08-28    Developer: C. DSouza
**      Description:        Build ID for spdc_v027
**
**      Date: 2023-08-28    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-08-28    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-08-28    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-08-29    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-09-11    Developer: C. DSouza
**      Description:        Build ID for v02.73.00
**
**      Date: 2023-09-15    Developer: C. DSouza
**      Description:        Build ID for v02.74.00
**
**      Date: 2023-09-15    Developer: C. DSouza
**      Description:        Build ID for v02.74.01
**
**      Date: 2023-09-28    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-10-02    Developer: C. DSouza
**      Description:        Build ID for v03.26.00
**
**      Date: 2023-10-31    Developer: C. DSouza
**      Description:        Build ID for v00.00.00
**
**      Date: 2023-11-08    Developer: C. DSouza
**      Description:        Build ID for v03.27.00
**
**      Date: 2023-11-09    Developer: C. DSouza
**      Description:        Build ID for v02.75.00
**
**      Date: 2023-12-13    Developer: C. DSouza
**      Description:        Build ID for v02.72.00
**
**      Date: 2023-12-20    Developer: C. DSouza
**      Description:        Build ID for v02.76.00
**
**      Date: 2023-12-20    Developer: C. DSouza
**      Description:        Build ID for v02.76.00
**
**      Date: 2024-02-02    Developer: R. Bambery
**      Description:        Build ID for v02.78.00
**
**      Date: 2024-02-02    Developer: R. Bambery
**      Description:        Build ID for v02.78.00
**
**      Date: 2024-02-02    Developer: R. Bambery
**      Description:        Build ID for v02.78.00
**
**      Date: 2024-07-31    Developer: R. Bambery
**      Description:        Build ID for v02.83.00
**
**      Date: YYYY-MM-DD    Developer: username
**      Description:        One or two lines, no need for specific details Build Id.
**
***********************************************************************************************/

#include "BuildId.hh"

const char*
sndr::BuildId::
VERSION =
"v00.00.00";

