#-----------------------------------------------------------------------------
# Include all the necessary files for macros
#-----------------------------------------------------------------------------
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

#-----------------------------------------------------------------------------
# Always SET this for now IF we are on an OS X box
#-----------------------------------------------------------------------------
IF (APPLE)
  LIST(LENGTH CMAKE_OSX_ARCHITECTURES ARCH_LENGTH)
  IF(ARCH_LENGTH GREATER 1)
    set (CMAKE_OSX_ARCHITECTURES "" CACHE STRING "" FORCE)
    message(FATAL_ERROR "Building Universal Binaries on OS X is NOT supported by the EOS project. This is"
    "due to technical reasons. The best approach would be build each architecture in separate directories"
    "and use the 'lipo' tool to combine them into a single executable or library. The 'CMAKE_OSX_ARCHITECTURES'"
    "variable has been set to a blank value which will build the default architecture for this system.")
  ENDIF()
  SET (EOS_AC_APPLE_UNIVERSAL_BUILD 0)
ENDIF (APPLE)

#-----------------------------------------------------------------------------
# This MACRO checks IF the symbol exists in the library and IF it
# does, it appends library to the list.
#-----------------------------------------------------------------------------
SET (LINK_LIBS "")
MACRO (CHECK_LIBRARY_EXISTS_CONCAT LIBRARY SYMBOL VARIABLE)
  CHECK_LIBRARY_EXISTS ("${LIBRARY};${LINK_LIBS}" ${SYMBOL} "" ${VARIABLE})
  IF (${VARIABLE})
    SET (LINK_LIBS ${LINK_LIBS} ${LIBRARY})
  ENDIF (${VARIABLE})
ENDMACRO (CHECK_LIBRARY_EXISTS_CONCAT)

# ----------------------------------------------------------------------
# WINDOWS Hard code Values
# ----------------------------------------------------------------------

SET (WINDOWS)
IF (WIN32)
  IF (MINGW)
    SET (WINDOWS 1) # MinGW tries to imitate Windows
    SET (CMAKE_REQUIRED_FLAGS "-DWIN32_LEAN_AND_MEAN=1 -DNOGDI=1")
  ENDIF (MINGW)
  SET (CMAKE_REQUIRED_LIBRARIES "ws2_32.lib;wsock32.lib")
  IF (NOT UNIX AND NOT CYGWIN AND NOT MINGW)
    SET (WINDOWS 1)
    SET (CMAKE_REQUIRED_FLAGS "/DWIN32_LEAN_AND_MEAN=1 /DNOGDI=1")
  ENDIF (NOT UNIX AND NOT CYGWIN AND NOT MINGW)
ENDIF (WIN32)

IF (WINDOWS)
  SET (EOS_HAVE_STDDEF_H 1)
  SET (EOS_HAVE_SYS_STAT_H 1)
  SET (EOS_HAVE_SYS_TYPES_H 1)
  SET (EOS_HAVE_LIBM 1)
  SET (EOS_HAVE_STRDUP 1)
  SET (EOS_HAVE_SYSTEM 1)
  SET (EOS_HAVE_LONGJMP 1)
  IF (NOT MINGW)
    SET (EOS_HAVE_GETHOSTNAME 1)
  ENDIF (NOT MINGW)
  SET (EOS_HAVE_GETCONSOLESCREENBUFFERINFO 1)
  SET (EOS_HAVE_FUNCTION 1)
  SET (EOS_HAVE_TIMEZONE 1)
  IF (MINGW)
    SET (EOS_HAVE_WINSOCK2_H 1)
  ENDIF (MINGW)
  SET (EOS_HAVE_LIBWS2_32 1)
  SET (EOS_HAVE_LIBWSOCK32 1)
ENDIF (WINDOWS)

# ----------------------------------------------------------------------
# END of WINDOWS Hard code Values
# ----------------------------------------------------------------------

#-----------------------------------------------------------------------------
#  Check for the math library "m"
#-----------------------------------------------------------------------------
IF (NOT WINDOWS)
  CHECK_LIBRARY_EXISTS_CONCAT ("m" ceil     EOS_HAVE_LIBM)
  CHECK_LIBRARY_EXISTS_CONCAT ("ws2_32" WSAStartup  EOS_HAVE_LIBWS2_32)
  CHECK_LIBRARY_EXISTS_CONCAT ("wsock32" gethostbyname EOS_HAVE_LIBWSOCK32)
ENDIF (NOT WINDOWS)

CHECK_LIBRARY_EXISTS_CONCAT ("ucb"    gethostname  EOS_HAVE_LIBUCB)
CHECK_LIBRARY_EXISTS_CONCAT ("socket" connect      EOS_HAVE_LIBSOCKET)
CHECK_LIBRARY_EXISTS ("c" gethostbyname "" NOT_NEED_LIBNSL)

IF (NOT NOT_NEED_LIBNSL)
  CHECK_LIBRARY_EXISTS_CONCAT ("nsl"    gethostbyname  EOS_HAVE_LIBNSL)
ENDIF (NOT NOT_NEED_LIBNSL)

# For other tests to use the same libraries
SET (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${LINK_LIBS})

SET (USE_INCLUDES "")
IF (WINDOWS)
  SET (USE_INCLUDES ${USE_INCLUDES} "windows.h")
ENDIF (WINDOWS)

# For other other specific tests, use this MACRO.
MACRO (HDFEOS_FUNCTION_TEST OTHER_TEST)
  IF ("EOS_${OTHER_TEST}" MATCHES "^EOS_${OTHER_TEST}$")
    SET (MACRO_CHECK_FUNCTION_DEFINITIONS "-D${OTHER_TEST} ${CMAKE_REQUIRED_FLAGS}")
    SET (OTHER_TEST_ADD_LIBRARIES)
    IF (CMAKE_REQUIRED_LIBRARIES)
      SET (OTHER_TEST_ADD_LIBRARIES "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ENDIF (CMAKE_REQUIRED_LIBRARIES)
    
    FOREACH (def ${EOS_EXTRA_TEST_DEFINITIONS})
      SET (MACRO_CHECK_FUNCTION_DEFINITIONS "${MACRO_CHECK_FUNCTION_DEFINITIONS} -D${def}=${${def}}")
    ENDFOREACH (def)

    FOREACH (def
        HAVE_SYS_TIME_H
        HAVE_UNISTD_H
        HAVE_SYS_TYPES_H
        HAVE_SYS_SOCKET_H
    )
      IF ("${EOS_${def}}")
        SET (MACRO_CHECK_FUNCTION_DEFINITIONS "${MACRO_CHECK_FUNCTION_DEFINITIONS} -D${def}")
      ENDIF ("${EOS_${def}}")
    ENDFOREACH (def)
    
    IF (LARGEFILE)
      SET (MACRO_CHECK_FUNCTION_DEFINITIONS
          "${MACRO_CHECK_FUNCTION_DEFINITIONS} -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE"
      )
    ENDIF (LARGEFILE)

    #MESSAGE (STATUS "Performing ${OTHER_TEST}")
    TRY_COMPILE (${OTHER_TEST}
        ${CMAKE_BINARY_DIR}
        ${EOS_RESOURCES_DIR}/EOSTests.c
        CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
        "${OTHER_TEST_ADD_LIBRARIES}"
        OUTPUT_VARIABLE OUTPUT
    )
    IF (${OTHER_TEST})
      SET (EOS_${OTHER_TEST} 1 CACHE INTERNAL "Other test ${FUNCTION}")
      MESSAGE (STATUS "Performing Other Test ${OTHER_TEST} - Success")
    ELSE (${OTHER_TEST})
      MESSAGE (STATUS "Performing Other Test ${OTHER_TEST} - Failed")
      SET (EOS_${OTHER_TEST} "" CACHE INTERNAL "Other test ${FUNCTION}")
      FILE (APPEND
          ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
          "Performing Other Test ${OTHER_TEST} failed with the following output:\n"
          "${OUTPUT}\n"
      )
    ENDIF (${OTHER_TEST})
  ENDIF ("EOS_${OTHER_TEST}" MATCHES "^EOS_${OTHER_TEST}$")
ENDMACRO (HDFEOS_FUNCTION_TEST)

#-----------------------------------------------------------------------------
HDFEOS_FUNCTION_TEST (STDC_HEADERS)
HDFEOS_FUNCTION_TEST (HAVE_F2CFORTRAN_MACRO)
HDFEOS_FUNCTION_TEST (HAVE_F2CFORTRAN_32PTR)

#-----------------------------------------------------------------------------
# Check IF header file exists and add it to the list.
#-----------------------------------------------------------------------------
MACRO (CHECK_INCLUDE_FILE_CONCAT FILE VARIABLE)
  CHECK_INCLUDE_FILES ("${USE_INCLUDES};${FILE}" ${VARIABLE})
  IF (${VARIABLE})
    SET (USE_INCLUDES ${USE_INCLUDES} ${FILE})
  ENDIF (${VARIABLE})
ENDMACRO (CHECK_INCLUDE_FILE_CONCAT)

#-----------------------------------------------------------------------------
#  Check for the existence of certain header files
#-----------------------------------------------------------------------------
CHECK_INCLUDE_FILE_CONCAT ("unistd.h"        EOS_HAVE_UNISTD_H)
CHECK_INCLUDE_FILE_CONCAT ("sys/stat.h"      EOS_HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE_CONCAT ("sys/types.h"     EOS_HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE_CONCAT ("stddef.h"        EOS_HAVE_STDDEF_H)
CHECK_INCLUDE_FILE_CONCAT ("stdint.h"        EOS_HAVE_STDINT_H)

# IF the c compiler found stdint, check the C++ as well. On some systems this
# file will be found by C but not C++, only do this test IF the C++ compiler
# has been initialized (e.g. the project also includes some c++)
IF (EOS_HAVE_STDINT_H AND CMAKE_CXX_COMPILER_LOADED)
  CHECK_INCLUDE_FILE_CXX ("stdint.h" EOS_HAVE_STDINT_H_CXX)
  IF (NOT EOS_HAVE_STDINT_H_CXX)
    SET (EOS_HAVE_STDINT_H "" CACHE INTERNAL "Have includes HAVE_STDINT_H")
    SET (USE_INCLUDES ${USE_INCLUDES} "stdint.h")
  ENDIF (NOT EOS_HAVE_STDINT_H_CXX)
ENDIF (EOS_HAVE_STDINT_H AND CMAKE_CXX_COMPILER_LOADED)

# Windows
IF (NOT CYGWIN)
  CHECK_INCLUDE_FILE_CONCAT ("winsock2.h"      EOS_HAVE_WINSOCK2_H)
ENDIF (NOT CYGWIN)
CHECK_INCLUDE_FILE_CONCAT ("pthread.h"       EOS_HAVE_PTHREAD_H)
CHECK_INCLUDE_FILE_CONCAT ("string.h"        EOS_HAVE_STRING_H)
CHECK_INCLUDE_FILE_CONCAT ("strings.h"       EOS_HAVE_STRINGS_H)
CHECK_INCLUDE_FILE_CONCAT ("stdlib.h"        EOS_HAVE_STDLIB_H)
CHECK_INCLUDE_FILE_CONCAT ("memory.h"        EOS_HAVE_MEMORY_H)
CHECK_INCLUDE_FILE_CONCAT ("dlfcn.h"         EOS_HAVE_DLFCN_H)
CHECK_INCLUDE_FILE_CONCAT ("inttypes.h"      EOS_HAVE_INTTYPES_H)

#-----------------------------------------------------------------------------
#  Check for large file support
#-----------------------------------------------------------------------------

# The linux-lfs option is deprecated.
SET (LINUX_LFS 0)

SET (EOS_EXTRA_FLAGS)
IF (NOT WINDOWS)
  # Linux Specific flags
  IF (CYGWIN)
    SET (EOS_EXTRA_FLAGS -D_BSD_SOURCE)
  ELSE (CYGWIN)
    SET (EOS_EXTRA_FLAGS -D_POSIX_SOURCE -D_BSD_SOURCE)
  ENDIF (CYGWIN)
  OPTION (EOS_ENABLE_LARGE_FILE "Enable support for large (64-bit) files on Linux." ON)
  IF (EOS_ENABLE_LARGE_FILE)
    SET (msg "Performing TEST_LFS_WORKS")
    TRY_RUN (TEST_LFS_WORKS_RUN   TEST_LFS_WORKS_COMPILE
        ${CMAKE_BINARY_DIR}/CMake
        ${EOS_RESOURCES_DIR}/EOSTests.c
        CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=-DTEST_LFS_WORKS
        OUTPUT_VARIABLE OUTPUT
    )
    IF (TEST_LFS_WORKS_COMPILE)
      IF (TEST_LFS_WORKS_RUN  MATCHES 0)
        SET (TEST_LFS_WORKS 1 CACHE INTERNAL ${msg})
        SET (LARGEFILE 1)
        SET (EOS_EXTRA_FLAGS ${EOS_EXTRA_FLAGS} -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE)
        MESSAGE (STATUS "${msg}... yes")
      ELSE (TEST_LFS_WORKS_RUN  MATCHES 0)
        SET (TEST_LFS_WORKS "" CACHE INTERNAL ${msg})
        MESSAGE (STATUS "${msg}... no")
        FILE (APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
              "Test TEST_LFS_WORKS Run failed with the following output and exit code:\n ${OUTPUT}\n"
        )
      ENDIF (TEST_LFS_WORKS_RUN  MATCHES 0)
    ELSE (TEST_LFS_WORKS_COMPILE )
      SET (TEST_LFS_WORKS "" CACHE INTERNAL ${msg})
      MESSAGE (STATUS "${msg}... no")
      FILE (APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
          "Test TEST_LFS_WORKS Compile failed with the following output:\n ${OUTPUT}\n"
      )
    ENDIF (TEST_LFS_WORKS_COMPILE)
  ENDIF (EOS_ENABLE_LARGE_FILE)
  SET (CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${EOS_EXTRA_FLAGS})
ENDIF (NOT WINDOWS)

ADD_DEFINITIONS (${EOS_EXTRA_FLAGS})

#-----------------------------------------------------------------------------
#  Check the size in bytes of all the int and float types
#-----------------------------------------------------------------------------
MACRO (EOS_CHECK_TYPE_SIZE type var)
  SET (aType ${type})
  SET (aVar  ${var})
#  MESSAGE (STATUS "Checking size of ${aType} and storing into ${aVar}")
  CHECK_TYPE_SIZE (${aType}   ${aVar})
  IF (NOT ${aVar})
    SET (${aVar} 0 CACHE INTERNAL "SizeOf for ${aType}")
#    MESSAGE (STATUS "Size of ${aType} was NOT Found")
  ENDIF (NOT ${aVar})
ENDMACRO (EOS_CHECK_TYPE_SIZE)


EOS_CHECK_TYPE_SIZE (char           EOS_SIZEOF_CHAR)
EOS_CHECK_TYPE_SIZE (short          EOS_SIZEOF_SHORT)
EOS_CHECK_TYPE_SIZE (int            EOS_SIZEOF_INT)
EOS_CHECK_TYPE_SIZE (unsigned       EOS_SIZEOF_UNSIGNED)
IF (NOT APPLE)
  EOS_CHECK_TYPE_SIZE (long         EOS_SIZEOF_LONG)
ENDIF (NOT APPLE)
EOS_CHECK_TYPE_SIZE ("long long"    EOS_SIZEOF_LONG_LONG)
EOS_CHECK_TYPE_SIZE (__int64        EOS_SIZEOF___INT64)
IF (NOT EOS_SIZEOF___INT64)
  SET (EOS_SIZEOF___INT64 0)
ENDIF (NOT EOS_SIZEOF___INT64)

EOS_CHECK_TYPE_SIZE (float          EOS_SIZEOF_FLOAT)
EOS_CHECK_TYPE_SIZE (double         EOS_SIZEOF_DOUBLE)
EOS_CHECK_TYPE_SIZE ("long double"  EOS_SIZEOF_LONG_DOUBLE)

EOS_CHECK_TYPE_SIZE (int8_t         EOS_SIZEOF_INT8_T)
EOS_CHECK_TYPE_SIZE (uint8_t        EOS_SIZEOF_UINT8_T)
EOS_CHECK_TYPE_SIZE (int_least8_t   EOS_SIZEOF_INT_LEAST8_T)
EOS_CHECK_TYPE_SIZE (uint_least8_t  EOS_SIZEOF_UINT_LEAST8_T)
EOS_CHECK_TYPE_SIZE (int_fast8_t    EOS_SIZEOF_INT_FAST8_T)
EOS_CHECK_TYPE_SIZE (uint_fast8_t   EOS_SIZEOF_UINT_FAST8_T)

EOS_CHECK_TYPE_SIZE (int16_t        EOS_SIZEOF_INT16_T)
EOS_CHECK_TYPE_SIZE (uint16_t       EOS_SIZEOF_UINT16_T)
EOS_CHECK_TYPE_SIZE (int_least16_t  EOS_SIZEOF_INT_LEAST16_T)
EOS_CHECK_TYPE_SIZE (uint_least16_t EOS_SIZEOF_UINT_LEAST16_T)
EOS_CHECK_TYPE_SIZE (int_fast16_t   EOS_SIZEOF_INT_FAST16_T)
EOS_CHECK_TYPE_SIZE (uint_fast16_t  EOS_SIZEOF_UINT_FAST16_T)

EOS_CHECK_TYPE_SIZE (int32_t        EOS_SIZEOF_INT32_T)
EOS_CHECK_TYPE_SIZE (uint32_t       EOS_SIZEOF_UINT32_T)
EOS_CHECK_TYPE_SIZE (int_least32_t  EOS_SIZEOF_INT_LEAST32_T)
EOS_CHECK_TYPE_SIZE (uint_least32_t EOS_SIZEOF_UINT_LEAST32_T)
EOS_CHECK_TYPE_SIZE (int_fast32_t   EOS_SIZEOF_INT_FAST32_T)
EOS_CHECK_TYPE_SIZE (uint_fast32_t  EOS_SIZEOF_UINT_FAST32_T)

EOS_CHECK_TYPE_SIZE (int64_t        EOS_SIZEOF_INT64_T)
EOS_CHECK_TYPE_SIZE (uint64_t       EOS_SIZEOF_UINT64_T)
EOS_CHECK_TYPE_SIZE (int_least64_t  EOS_SIZEOF_INT_LEAST64_T)
EOS_CHECK_TYPE_SIZE (uint_least64_t EOS_SIZEOF_UINT_LEAST64_T)
EOS_CHECK_TYPE_SIZE (int_fast64_t   EOS_SIZEOF_INT_FAST64_T)
EOS_CHECK_TYPE_SIZE (uint_fast64_t  EOS_SIZEOF_UINT_FAST64_T)
IF (NOT APPLE)
  EOS_CHECK_TYPE_SIZE (size_t       EOS_SIZEOF_SIZE_T)
  EOS_CHECK_TYPE_SIZE (ssize_t      EOS_SIZEOF_SSIZE_T)
  IF (NOT EOS_SIZEOF_SSIZE_T)
    SET (EOS_SIZEOF_SSIZE_T 0)
  ENDIF (NOT EOS_SIZEOF_SSIZE_T)
ENDIF (NOT APPLE)
EOS_CHECK_TYPE_SIZE (off_t          EOS_SIZEOF_OFF_T)
EOS_CHECK_TYPE_SIZE (off64_t        EOS_SIZEOF_OFF64_T)
IF (NOT EOS_SIZEOF_OFF64_T)
  SET (EOS_SIZEOF_OFF64_T 0)
ENDIF (NOT EOS_SIZEOF_OFF64_T)


#-----------------------------------------------------------------------------
# Check a bunch of other functions
#-----------------------------------------------------------------------------
IF (NOT WINDOWS)
  FOREACH (test
      STDC_HEADERS
      HAVE_FUNCTION
  )
    HDFEOS_FUNCTION_TEST (${test})
  ENDFOREACH (test)
ENDIF (NOT WINDOWS)
