cmake_minimum_required (VERSION 2.8.10)
PROJECT (HDFEOS C CXX)

#-----------------------------------------------------------------------------
# Instructions for use : Normal Build
#
# For standard build of HDFEOS libraries,tests and tools. 
# Run cmake using the HDFEOS source tree to generate a build tree.
# Enable/Disable options according to requirements and
# set CMAKE_INSTALL_PREFIX to the required install path.
# Make install can be used to install all components for system-wide use.
#
#-----------------------------------------------------------------------------
# Instructions for use : Sub-Project Build
#
# To include HDFEOS as a sub-project within another project. 
# Set HDFEOS_EXTERNALLY_CONFIGURED to 1 in the parent project and
# supply values for the following variables...
#
# HDFEOS_EXPORTED_TARGETS :
#   Set this to the name of the targets variable which controls exports
#   If unset (because parent project does not support/use the 
#   INSTALL (EXPORT target...) syntax), then targets are not configured 
#   for export during install.
#
# HDFEOS_LIB_DEPENDENCIES :
#   If the build of HDFEOS libs is being customized, then rules for the
#   dependencies of the HDFEOS libs may be 'incomplete', add additional
#   dependencies to this variable so that external projects pick them up
#
# HDFEOS_EXTERNAL_LIB_PREFIX :
#   If the pa5rent project needs to install hdf libraries, but avoid 
#   name conflicts with system versions, then a prefix may be added
#   to ensure that the correct versions configured are used.
#
# Consider this example, it builds its own zlib
# library and tells HDFEOS to add it as a dependency - this ensures that
# any project making use of this build of HDFEOS will use the correct zlib
#
#   # Tell HDFEOS that we are manually overriding certain settings
#   SET (HDFEOS_EXTERNALLY_CONFIGURED 1)
#   # Avoid duplicating names of installed libraries
#   SET (HDFEOS_EXTERNAL_LIB_PREFIX "prj")
#   # Targets linking to the HDFEOS libs need to know their names
#   # if they are changed in the sub project, they should be here too
#   SET (HDFEOS_LIB_NAME "prjHDFEOS")
#   SET (HDFEOS_MF_LIB_NAME "prjHDFEOS_mf")
#   SET (HDFEOS_LIBRARY "${HDFEOS_LIB_NAME};${HDFEOS_MF_LIB_NAME}")
#   # Export configuration to this export variable
#   SET (HDFEOS_EXPORTED_TARGETS "project-targets")
#
#   # Setup all necessary overrides for zlib so that HDFEOS uses our
#   # internally compiled zlib rather than any other version
#   IF (HDF_ENABLE_Z_LIB_SUPPORT)
#    # We must tell the main HDFEOS library that it depends on our zlib 
#     SET (HDFEOS_LIB_DEPENDENCIES prjzlib)    
#     # Override the zlib header file
#     IF (PRJ_USE_SYSTEM_ZLIB)
#       SET (HEOS_ZLIB_HEADER "zlib.h")
#     ELSE (PRJ_USE_SYSTEM_ZLIB)
#      SET (HEOS_ZLIB_HEADER "prj_zlib.h")
#       # Set vars that FindZlib would have set if used in sub project
#       SET (ZLIB_INCLUDE_DIRS "${PRJ_ZLIB_INCLUDE_DIRS}")
#       SET (ZLIB_LIBRARIES prjzlib)
#     ENDIF (PRJ_USE_SYSTEM_ZLIB)
#  ENDIF (HDF_ENABLE_Z_LIB_SUPPORT)
#   
#   # Add the sub project
#   ADD_SUBDIRECTORY (Utilities/HDFEOS-1.2)
#   # Add the HDFEOS dirs to our include path
#   SET (HDFEOS_INCLUDE_DIR 
#       ${PROJECT_SOURCE_DIR}/Utilities/HDFEOS-1.2/src
#       ${PROJECT_BINARY_DIR}/Utilities/HDFEOS-1.2
#   )
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Allow Visual Studio solution directories
#-----------------------------------------------------------------------------
# Provide a way for Visual Studio Express users to turn OFF the new FOLDER
# organization feature. Default to ON for non-Express users. Express users must
# explicitly turn off this option to build HDFEOS in the Express IDE...
#
OPTION (HDFEOS_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
MARK_AS_ADVANCED (HDFEOS_USE_FOLDERS)
IF (HDFEOS_USE_FOLDERS)
  SET_PROPERTY (GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF (HDFEOS_USE_FOLDERS)
OPTION (HDFEOS_NO_PACKAGES "CPACK - Disable packaging" OFF)
MARK_AS_ADVANCED (HDFEOS_NO_PACKAGES)

#-----------------------------------------------------------------------------
# Set the core names of all the libraries
#-----------------------------------------------------------------------------
SET (HDFEOS_GCTP_SRC_LIB_CORENAME     "Gctp")
SET (HDFEOS_SRC_LIB_CORENAME          "hdfeos")

#-----------------------------------------------------------------------------
# Set the true names of all the libraries if customized by external project
#-----------------------------------------------------------------------------
SET (HDFEOS_GCTP_SRC_LIB_NAME     "${HDFEOS_EXTERNAL_LIB_PREFIX}${HDFEOS_GCTP_SRC_LIB_CORENAME}")
SET (HDFEOS_SRC_LIB_NAME          "${HDFEOS_EXTERNAL_LIB_PREFIX}${HDFEOS_SRC_LIB_CORENAME}")

#-----------------------------------------------------------------------------
# Set the target names of all the libraries
#-----------------------------------------------------------------------------
SET (HDFEOS_GCTP_SRC_LIB_TARGET     ${HDFEOS_GCTP_SRC_LIB_NAME})
SET (HDFEOS_SRC_LIB_TARGET          ${HDFEOS_SRC_LIB_NAME})

#-----------------------------------------------------------------------------
# Define some CMake variables for use later in the project
#-----------------------------------------------------------------------------
SET (EOS_RESOURCES_DIR          ${HDFEOS_SOURCE_DIR}/config/cmake)
SET (HDFEOS_GCTP_SRC_DIR        ${HDFEOS_SOURCE_DIR}/gctp/src)
SET (HDFEOS_GCTP_INC_DIR        ${HDFEOS_SOURCE_DIR}/gctp/include)
SET (HDFEOS_SRC_DIR             ${HDFEOS_SOURCE_DIR}/src)
SET (HDFEOS_INC_DIR             ${HDFEOS_SOURCE_DIR}/include)
SET (HDFEOS_SAMPLES_SRC_DIR     ${HDFEOS_SOURCE_DIR}/samples)

IF (NOT HDFEOS_INSTALL_BIN_DIR)
  SET (HDFEOS_INSTALL_BIN_DIR bin)
ENDIF (NOT HDFEOS_INSTALL_BIN_DIR)
IF (NOT HDFEOS_INSTALL_LIB_DIR)
  SET (HDFEOS_INSTALL_LIB_DIR lib)
ENDIF (NOT HDFEOS_INSTALL_LIB_DIR)
IF (NOT HDFEOS_INSTALL_INCLUDE_DIR)
  SET (HDFEOS_INSTALL_INCLUDE_DIR include)
ENDIF (NOT HDFEOS_INSTALL_INCLUDE_DIR)
IF (NOT HDFEOS_INSTALL_DATA_DIR)
  IF (NOT WIN32)
    SET (HDFEOS_INSTALL_DATA_DIR share)
    SET (HDFEOS_INSTALL_CMAKE_DIR share/cmake)
  ELSE (NOT WIN32)
    SET (HDFEOS_INSTALL_DATA_DIR ".")
    SET (HDFEOS_INSTALL_CMAKE_DIR cmake)
  ENDIF (NOT WIN32)
ENDIF (NOT HDFEOS_INSTALL_DATA_DIR)

#-----------------------------------------------------------------------------
# parse the full version number
#-----------------------------------------------------------------------------
FILE (READ ${HDFEOS_SRC_DIR}/EHapi.c _ehapi_c_contents)
STRING (REGEX REPLACE ".*#define[ \t]+HDFEOSVERSION1[ \t]+\"([0-9]*.[0-9]*)\".*"
    "\\1" HDFEOS_FULL_VERSION ${_ehapi_c_contents})
STRING (REGEX REPLACE ".*#define[ \t]+HDFEOSVERSION1[ \t]+\"([0-9]*).*$"
    "\\1" HDFEOS_VERS_MAJOR ${_ehapi_c_contents})
STRING (REGEX REPLACE ".*#define[ \t]+HDFEOSVERSION1[ \t]+\"[0-9]*.([0-9]*).*$"
    "\\1" HDFEOS_VERS_MINOR ${_ehapi_c_contents})
MESSAGE (STATUS "VERSION: ${HDFEOS_VERSION}")
#SET (HDFEOS_VERS_MAJOR 1)
#SET (HDFEOS_VERS_MINOR 0)
SET (HDFEOS_VERS_RELEASE 0)
#MESSAGE (STATUS "VERSION: ${HDFEOS_VERSION}")

#-----------------------------------------------------------------------------
# Basic HDFEOS stuff here
#-----------------------------------------------------------------------------
SET (HDFEOS_PACKAGE "hdf-eos2")
SET (HDFEOS_PACKAGE_NAME "hdf-eos2")
SET (HDFEOS_PACKAGE_VERSION "${HDFEOS_VERS_MAJOR}.${HDFEOS_VERS_MINOR}.${HDFEOS_VERS_RELEASE}")
SET (HDFEOS_PACKAGE_VERSION_MAJOR "${HDFEOS_VERS_MAJOR}.${HDFEOS_VERS_MINOR}")
SET (HDFEOS_PACKAGE_VERSION_MINOR "${HDFEOS_VERS_RELEASE}")
SET (HDFEOS_PACKAGE_STRING "${HDFEOS_PACKAGE_NAME} ${HDFEOS_PACKAGE_VERSION}")
SET (HDFEOS_PACKAGE_TARNAME "hdf-eos2")
SET (HDFEOS_PACKAGE_URL "http://www.hdfgroup.org")
SET (HDFEOS_PACKAGE_BUGREPORT "help@hdfgroup.org")

#-----------------------------------------------------------------------------
# Include some macros for reusable code
#-----------------------------------------------------------------------------
INCLUDE (${EOS_RESOURCES_DIR}/EOSMacros.cmake)
INCLUDE (${EOS_RESOURCES_DIR}/HDFEOSMacros.cmake)
INCLUDE (${EOS_RESOURCES_DIR}/HDF4Macros.cmake)

#-----------------------------------------------------------------------------
# Setup output Directories
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
  )
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
  )
  SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
  )
ELSE (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  # if we are externally configured, but the project uses old cmake scripts
  # this may not be set and utilities like Hdetect will fail
  IF (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
  ENDIF (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Targets built within this project are exported at Install time for use
# by other projects using FindEOS.
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXPORTED_TARGETS)
  SET (HDFEOS_EXPORTED_TARGETS "hdfeos-targets")
ENDIF (NOT HDFEOS_EXPORTED_TARGETS)

#-----------------------------------------------------------------------------
# To include a library in the list exported by the project AT BUILD TIME,
# add it to this variable. This is NOT used by Make Install, but for projects
# which include HDFEOS as a sub-project within their build tree
#-----------------------------------------------------------------------------
SET_GLOBAL_VARIABLE (HDFEOS_LIBRARIES_TO_EXPORT "")
SET_GLOBAL_VARIABLE (HDFEOS_APPS_TO_EXPORT "")
SET (EXTERNAL_HEADER_LIST "")
SET (EXTERNAL_LIBRARY_LIST "")
SET (EXTERNAL_LIBRARYDLL_LIST "")

#-----------------------------------------------------------------------------
# Run all the CMake configuration tests for our build environment
#-----------------------------------------------------------------------------
INCLUDE (${EOS_RESOURCES_DIR}/ConfigureChecks.cmake)

#-----------------------------------------------------------------------------
# Option to use legacy naming for windows libs/programs, default is legacy
#-----------------------------------------------------------------------------
IF (WIN32 AND NOT CYGWIN)
  OPTION (HDFEOS_LEGACY_NAMING "Use Legacy Names for Libraries and Programs" ON)
ENDIF (WIN32 AND NOT CYGWIN)

#-----------------------------------------------------------------------------
# Option to Build Shared/Static libs, default is static
#-----------------------------------------------------------------------------
OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  SET (LIB_TYPE SHARED)
  SET (HDFEOS_BUILT_AS_DYNAMIC_LIB 1)
  ADD_DEFINITIONS (-DHDFEOS_BUILT_AS_DYNAMIC_LIB)
  ADD_DEFINITIONS (-DH4_BUILT_AS_DYNAMIC_LIB)
ELSE (BUILD_SHARED_LIBS)
  SET (HDFEOS_BUILT_AS_STATIC_LIB 1)
  ADD_DEFINITIONS (-DHDFEOS_BUILT_AS_STATIC_LIB)
  ADD_DEFINITIONS (-DH4_BUILT_AS_STATIC_LIB)
  IF (NOT WIN32)
    # should this be a user setting : Everyone uses it anyway ?
    ADD_DEFINITIONS (-DPIC)
  ENDIF (NOT WIN32)
ENDIF (BUILD_SHARED_LIBS)

IF (NOT USE_SHARED_LIBS)
  SET (HDFEOS_USE_STATIC_LIBRARIES 1)
ENDIF (NOT USE_SHARED_LIBS)

ADD_DEFINITIONS (-DBIG_LONGS -DSWAP)
ADD_DEFINITIONS (-DHAVE_CONFIG_H)
IF (MSVC)
  ADD_DEFINITIONS (-DVISUAL_CPLUSPLUS)
ENDIF (MSVC)
IF (EOS_HAVE_F2CFORTRAN_MACRO)
  ADD_DEFINITIONS (-Df2cFortran)
ENDIF (EOS_HAVE_F2CFORTRAN_MACRO)

#-----------------------------------------------------------------------------
# Option to use code coverage
#-----------------------------------------------------------------------------
OPTION (HDF_ENABLE_COVERAGE "Enable code coverage for Libraries and Programs" OFF)
IF (HDF_ENABLE_COVERAGE)
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    SET (LDFLAGS "${LDFLAGS} -fprofile-arcs -ftest-coverage")   
ENDIF (HDF_ENABLE_COVERAGE)

#-----------------------------------------------------------------------------
# Include the main src and config directories
#-----------------------------------------------------------------------------
SET (HDFEOS_INCLUDE_DIRECTORIES
    ${HDFEOS_GCTP_INC_DIR} 
    ${HDFEOS_INC_DIR} 
    ${HDFEOS_BINARY_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
INCLUDE_DIRECTORIES (${HDFEOS_INCLUDE_DIRECTORIES} )

#-----------------------------------------------------------------------------
# When building utility executables that generate other (source) files :
# we make use of the following variables defined in the root CMakeLists.
# Certain systems may add /Debug or /Release to output paths
# and we need to call the executable from inside the CMake configuration
#-----------------------------------------------------------------------------
SET (EXE_EXT "")
IF (WIN32)
  SET (EXE_EXT ".exe")
  IF (NOT CYGWIN)
    ADD_DEFINITIONS (-DBIND_TO_CURRENT_VCLIBS_VERSION=1)
    ADD_DEFINITIONS (-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS (-D_CONSOLE)
  ENDIF (NOT CYGWIN)
ENDIF (WIN32)

IF (MSVC)
  SET (CMAKE_MFC_FLAG 0)
ENDIF (MSVC)

SET (MAKE_SYSTEM)
IF (CMAKE_BUILD_TOOL MATCHES "make")
  SET (MAKE_SYSTEM 1)
ENDIF (CMAKE_BUILD_TOOL MATCHES "make")

SET (CFG_INIT "/${CMAKE_CFG_INTDIR}")
IF (MAKE_SYSTEM)
  SET (CFG_INIT "")
ENDIF (MAKE_SYSTEM)

#-----------------------------------------------------------------------------
# Add some definitions for Debug Builds
#-----------------------------------------------------------------------------
IF (CMAKE_BUILD_TYPE MATCHES Debug)
  #-- NMake Makefiles will overwhelm the console with warnings if -Wall is used.
  IF (NOT WIN32)
    ADD_DEFINITIONS (-Wall)
  ENDIF (NOT WIN32)
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)

#-----------------------------------------------------------------------------
# Compiler specific flags : Shouldn't there be compiler tests for these
#-----------------------------------------------------------------------------
IF (CMAKE_COMPILER_IS_GNUCC)
  IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS} -std=c99 -finline-functions -fno-common")
  ELSE (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS} -std=c99 -fomit-frame-pointer -finline-functions -fno-common")
  ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
ENDIF (CMAKE_COMPILER_IS_GNUCC)
IF (CMAKE_COMPILER_IS_GNUCXX)
  IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (CMAKE_CXX_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_CXX_FLAGS} -finline-functions -fno-common")
  ELSE (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (CMAKE_CXX_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_CXX_FLAGS} -fomit-frame-pointer -finline-functions -fno-common")
  ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

#-----------------------------------------------------------------------------
# Option to allow the user to disable compiler warnings
#-----------------------------------------------------------------------------
OPTION (HDFEOS_DISABLE_COMPILER_WARNINGS "Disable compiler warnings" OFF)
IF (HDFEOS_DISABLE_COMPILER_WARNINGS)
  # MSVC uses /w to suppress warnings.  It also complains if another
  # warning level is given, so remove it.
  IF (MSVC)
    SET (HDFEOS_WARNINGS_BLOCKED 1)
    STRING (REGEX REPLACE "(^| )([/-])W[0-9]( |$)" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")
    STRING (REGEX REPLACE "(^| )([/-])W[0-9]( |$)" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
  ENDIF (MSVC)
  IF (WIN32 AND NOT CYGWIN)
    ADD_DEFINITIONS (-D_CRT_SECURE_NO_WARNINGS)
  ENDIF (WIN32 AND NOT CYGWIN)
  # Borland uses -w- to suppress warnings.
  IF (BORLAND)
    SET (HDFEOS_WARNINGS_BLOCKED 1)
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w-")
  ENDIF (BORLAND)

  # Most compilers use -w to suppress warnings.
  IF (NOT HDFEOS_WARNINGS_BLOCKED)
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  ENDIF (NOT HDFEOS_WARNINGS_BLOCKED)
ENDIF (HDFEOS_DISABLE_COMPILER_WARNINGS)

#-----------------------------------------------------------------------------
# This is in here to help some of the GCC based IDES like Eclipse
# and code blocks parse the compiler errors and warnings better.
#-----------------------------------------------------------------------------
IF (CMAKE_COMPILER_IS_GNUCC)
  SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF (CMAKE_COMPILER_IS_GNUCC)
IF (CMAKE_COMPILER_IS_GNUCXX)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

#-----------------------------------------------------------------------------
# Option to Enable MPI Parallel
#-----------------------------------------------------------------------------
SET (CMAKE_MODULE_PATH ${EOS_RESOURCES_DIR} ${CMAKE_MODULE_PATH})
OPTION (HDFEOS_ENABLE_PARALLEL "Enable parallel build (requires MPI)" OFF)
IF (HDFEOS_ENABLE_PARALLEL)
  INCLUDE (FindMPI)
  INCLUDE_DIRECTORIES (${MPI_C_INCLUDE_PATH})
  IF (MPI_C_FOUND)
    SET (HDFEOS_HAVE_PARALLEL 1)
    # MPI checks, only do these if MPI_C_FOUND is true, otherwise they always fail
    # and once set, they are cached as false and not regenerated
    SET (CMAKE_REQUIRED_LIBRARIES "${MPI_C_LIBRARIES}" )
    CHECK_FUNCTION_EXISTS (MPI_File_get_size HDFEOS_HAVE_MPI_GET_SIZE) 
    # Used by Fortran + MPI
    CHECK_SYMBOL_EXISTS (MPI_Comm_c2f "${MPI_C_INCLUDE_PATH}/mpi.h"  HDFEOS_HAVE_MPI_MULTI_LANG_Comm)
    CHECK_SYMBOL_EXISTS (MPI_Info_c2f "${MPI_C_INCLUDE_PATH}/mpi.h"  HDFEOS_HAVE_MPI_MULTI_LANG_Info)
  ENDIF (MPI_C_FOUND)
ENDIF (HDFEOS_ENABLE_PARALLEL)

# Parallel IO usage requires MPI to be Linked and Included
IF (HDFEOS_HAVE_PARALLEL)
  SET (LINK_LIBS ${LINK_LIBS} ${MPI_C_LIBRARIES})
  IF (MPI_C_LINK_FLAGS)
    SET (CMAKE_EXE_LINKER_FLAGS ${MPI_C_LINK_FLAGS} ${CMAKE_EXE_LINKER_FLAGS})
  ENDIF (MPI_C_LINK_FLAGS)
ENDIF (HDFEOS_HAVE_PARALLEL)

INCLUDE (ExternalProject)
OPTION (HDFEOS_ALLOW_EXTERNAL_SUPPORT "Allow External Library Building" "NO")
OPTION (JPEG_USE_EXTERNAL "Use External Library Building for JPEG" 0)
OPTION (ZLIB_USE_EXTERNAL "Use External Library Building for ZLIB" 0)
OPTION (SZIP_USE_EXTERNAL "Use External Library Building for SZIP" 0)
OPTION (HDF4_USE_EXTERNAL "Use External Library Building for HDF4" 0)
IF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
  SET (JPEG_USE_EXTERNAL 1)
  SET (ZLIB_USE_EXTERNAL 1)
  SET (SZIP_USE_EXTERNAL 1)
  SET (HDF4_USE_EXTERNAL 1)
  IF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN")
    SET (JPEG_URL ${JPEG_SVN_URL})
    SET (ZLIB_URL ${ZLIB_SVN_URL})
    SET (SZIP_URL ${SZIP_SVN_URL})
    SET (HDF4_URL ${HDF4_SVN_URL})
  ELSEIF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
    IF (NOT TGZ_PATH)
      SET (TGZ_PATH ${HDFEOS_SOURCE_DIR})
    ENDIF (NOT TGZ_PATH)
    SET (JPEG_URL ${TGZ_PATH}/${JPEG_TGZ_NAME})
    SET (ZLIB_URL ${TGZ_PATH}/${ZLIB_TGZ_NAME})
    SET (SZIP_URL ${TGZ_PATH}/${SZIP_TGZ_NAME})
    SET (HDF4_URL ${TGZ_PATH}/${HDF4_TGZ_NAME})
  ENDIF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN")
ENDIF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")

#-----------------------------------------------------------------------------
# Option for LibJpeg support
#-----------------------------------------------------------------------------
OPTION (HDFEOS_ENABLE_JPEG_LIB_SUPPORT "Enable jpeg" ON)

#-----------------------------------------------------------------------------
# Option for ZLib support
#-----------------------------------------------------------------------------
OPTION (HDFEOS_ENABLE_Z_LIB_SUPPORT "Enable Zlib Filters" ON)

#-----------------------------------------------------------------------------
# Option for SzLib support
#-----------------------------------------------------------------------------
OPTION (HDFEOS_ENABLE_SZIP_SUPPORT "Use SZip Filter" OFF)
IF (HDFEOS_ENABLE_SZIP_SUPPORT)
  OPTION (HDFEOS_ENABLE_SZIP_ENCODING "Use SZip Encoding" OFF)
ENDIF (HDFEOS_ENABLE_SZIP_SUPPORT)

#-----------------------------------------------------------------------------
# HDF4 support
#-----------------------------------------------------------------------------
IF (NOT BUILD_SHARED_LIBS)
  SET (HDF4_USE_STATIC_LIBRARIES 1)
ENDIF (NOT BUILD_SHARED_LIBS)
IF (NOT HEOS_HDF4_HEADER)
  IF (NOT HDF4_USE_EXTERNAL)
    FIND_PACKAGE (HDF4 NAMES ${HDF4_PACKAGE_NAME}${HDFEOS_PACKAGE_EXT})
    IF (NOT HDF4_FOUND)
      FIND_PACKAGE (HDF4) # Legacy find
    ENDIF (NOT HDF4_FOUND)
  ENDIF (NOT HDF4_USE_EXTERNAL)
  IF (HDF4_FOUND)
    MESSAGE (STATUS "HDF4 found: INC=${HDF4_INCLUDE_DIR} TOOLS=${HDF4_TOOLS_DIR}")
    IF ((NOT USE_SHARED_LIBS AND HDF4_BUILD_SHARED_LIBS) OR (USE_SHARED_LIBS AND NOT HDF4_BUILD_SHARED_LIBS))
      MESSAGE (STATUS "HDF4 build type does match requested type")
    ENDIF ((NOT USE_SHARED_LIBS AND HDF4_BUILD_SHARED_LIBS) OR (USE_SHARED_LIBS AND NOT HDF4_BUILD_SHARED_LIBS))
    IF (HDF4_BUILD_SHARED_LIBS)
      ADD_DEFINITIONS (-DH4_BUILT_AS_DYNAMIC_LIB)
    ELSE (HDF4_BUILD_SHARED_LIBS)
      ADD_DEFINITIONS (-DH4_BUILT_AS_STATIC_LIB)
    ENDIF (HDF4_BUILD_SHARED_LIBS)
    SET (HEOS_HAVE_HDF_H 1)
    SET (HEOS_HAVE_HDF4 1)
    SET (HEOS_HDF4_HEADER "hdf.h")
    SET (HDF4_INCLUDE_DIR_GEN ${HDF4_INCLUDE_DIR}${HDF_PACKAGE_EXT})
    SET (HDF4_INCLUDE_DIRS ${HDF4_INCLUDE_DIR})
  ELSE (HDF4_FOUND)
    IF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
      # May need to build JPEG with PIC on x64 machines with gcc
      # Need to use CMAKE_ANSI_CFLAGS define so that compiler test works
      IF (NOT CMAKE_ANSI_CFLAGS)
        SET (CMAKE_ANSI_CFLAGS " ")
      ENDIF (NOT CMAKE_ANSI_CFLAGS)
      IF (BUILD_JPEG_WITH_PIC)
        SET (CMAKE_ANSI_CFLAGS "${CMAKE_ANSI_CFLAGS} -fPIC")
      ENDIF (BUILD_JPEG_WITH_PIC)
        
      EXTERNAL_HDF4_LIBRARY (${HDFEOS_ALLOW_EXTERNAL_SUPPORT} ${LIB_TYPE})
      SET (HEOS_HAVE_HDF_H 1)
      SET (HEOS_HAVE_HDF4 1)
      SET (HEOS_HDF4_HEADER "hdf.h")
      MESSAGE (STATUS "HDF4 is built")
      IF (BUILD_SHARED_LIBS)
        ADD_DEFINITIONS (-DH4_BUILT_AS_DYNAMIC_LIB)
      ELSE (BUILD_SHARED_LIBS)
        ADD_DEFINITIONS (-DH4_BUILT_AS_STATIC_LIB)
      ENDIF (BUILD_SHARED_LIBS)
    ELSE (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
      MESSAGE (FATAL_ERROR " HDF4 is Required for  HDFEOS")
    ENDIF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
  ENDIF (HDF4_FOUND)
ELSE (NOT HEOS_HDF4_HEADER)
  # This project is being called from within another and HDF4 is already configured
  SET (HEOS_HAVE_HDF_H 1)
  SET (HEOS_HAVE_HDF4 1)
ENDIF (NOT HEOS_HDF4_HEADER)
IF (NOT TARGET "hdiff")
  ADD_EXECUTABLE (hdiff IMPORTED)
  IF (USE_SHARED_LIBS AND WIN32)
    SET_PROPERTY (TARGET hdiff PROPERTY IMPORTED_LOCATION "${HDF4_TOOLS_DIR}/hdiffdll")
  ELSE (USE_SHARED_LIBS AND WIN32)
    SET_PROPERTY (TARGET hdiff PROPERTY IMPORTED_LOCATION "${HDF4_TOOLS_DIR}/hdiff")
  ENDIF (USE_SHARED_LIBS AND WIN32)
  SET (HDF4_DIFF_EXECUTABLE $<TARGET_FILE:hdiff>)
ENDIF (NOT TARGET "hdiff")
SET (LINK_LIBS ${LINK_LIBS} ${HDF4_LIBRARIES})
INCLUDE_DIRECTORIES (${HDF4_INCLUDE_DIRS})

#-----------------------------------------------------------------------------
# Option for external libraries
#-----------------------------------------------------------------------------
OPTION (HDFEOS_PACKAGE_EXTLIBS "CPACK - include external libraries" OFF)
IF (HDFEOS_PACKAGE_EXTLIBS)
  IF (HDF4_FOUND)
    PACKAGE_HDF4_LIBRARY (${HDFEOS_ALLOW_EXTERNAL_SUPPORT})
  ENDIF (HDF4_FOUND)
ENDIF (HDFEOS_PACKAGE_EXTLIBS)

#-----------------------------------------------------------------------------
# Build utility to copy and strip X lines of file
#-----------------------------------------------------------------------------
IF (BUILD_TESTING)
  SET (XLATE_UTILITY "xlatefile")
  ADD_EXECUTABLE(${XLATE_UTILITY} ${EOS_RESOURCES_DIR}/xlatefile.c)
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Dashboard and Testing Settings
#-----------------------------------------------------------------------------
OPTION (BUILD_TESTING "Build HDFEOS Unit Testing" OFF)
IF (BUILD_TESTING)
  SET (DART_TESTING_TIMEOUT 1200
      CACHE INTEGER
      "Timeout in seconds for each test (default 1200=20minutes)"
  )
  ENABLE_TESTING ()
  INCLUDE (CTest)
  INCLUDE (${HDFEOS_SOURCE_DIR}/CTestConfig.cmake)
  CONFIGURE_FILE (${EOS_RESOURCES_DIR}/CTestCustom.cmake ${HDFEOS_BINARY_DIR}/CTestCustom.ctest @ONLY)
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Add the HDFEOS Library Target to the build
#-----------------------------------------------------------------------------
ADD_SUBDIRECTORY (${HDFEOS_SOURCE_DIR}/gctp ${PROJECT_BINARY_DIR}/gctp)
ADD_SUBDIRECTORY (${HDFEOS_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src)

IF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
  IF (HDF4_FOUND AND HDF4_USE_EXTERNAL)
    ADD_DEPENDENCIES (${HDFEOS_SRC_LIB_TARGET} HDF4)
  ENDIF (HDF4_FOUND AND HDF4_USE_EXTERNAL)
ENDIF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")

#-----------------------------------------------------------------------------
# Option to build Fortran bindings/tests/samples
#-----------------------------------------------------------------------------
OPTION (HDFEOS_BUILD_FORTRAN "Build FORTRAN support" ON)
IF (HDFEOS_BUILD_FORTRAN AND HDF4_BUILD_FORTRAN)
  INCLUDE (${EOS_RESOURCES_DIR}/HDF4UseFortran.cmake)
ENDIF (HDFEOS_BUILD_FORTRAN AND HDF4_BUILD_FORTRAN)

#-----------------------------------------------------------------------------
# Add the samples tests to the build
#-----------------------------------------------------------------------------
OPTION (HDFEOS_BUILD_SAMPLES  "Build HDFEOS Testdrivers" OFF)
IF (HDFEOS_BUILD_SAMPLES)
  IF (BUILD_TESTING)
    IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
      ADD_SUBDIRECTORY(${HDFEOS_SOURCE_DIR}/samples ${PROJECT_BINARY_DIR}/samples)
    ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  ENDIF (BUILD_TESTING)
ENDIF (HDFEOS_BUILD_SAMPLES)

#-----------------------------------------------------------------------------
# Option to build HDFEOS Testdrivers
#-----------------------------------------------------------------------------
OPTION (HDFEOS_BUILD_TESTDRIVERS  "Build HDFEOS Testdrivers" OFF)
IF (HDFEOS_BUILD_TESTDRIVERS)
  IF (BUILD_TESTING)
    IF (EXISTS "${HDFEOS_SOURCE_DIR}/testdrivers" AND IS_DIRECTORY "${HDFEOS_SOURCE_DIR}/testdrivers")
      ADD_SUBDIRECTORY (${HDFEOS_SOURCE_DIR}/testdrivers ${PROJECT_BINARY_DIR}/testdrivers)
    ENDIF (EXISTS "${HDFEOS_SOURCE_DIR}/testdrivers" AND IS_DIRECTORY "${HDFEOS_SOURCE_DIR}/testdrivers")
  ENDIF (BUILD_TESTING)
ENDIF (HDFEOS_BUILD_TESTDRIVERS)

#-----------------------------------------------------------------------------
# Generate the HE2_config.h file containing user settings needed by compilation
#-----------------------------------------------------------------------------
CONFIGURE_FILE (${EOS_RESOURCES_DIR}/HE2_config.h.in      ${PROJECT_BINARY_DIR}/HE2_config.h @ONLY)

#-----------------------------------------------------------------------------
# Add Target(s) to CMake Install for import into other projects
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  INSTALL (
      EXPORT ${HDFEOS_EXPORTED_TARGETS}
      DESTINATION ${HDFEOS_INSTALL_CMAKE_DIR}/${HDFEOS_PACKAGE}
      FILE ${HDFEOS_PACKAGE}${HDFEOS_PACKAGE_EXT}-targets.cmake
      COMPONENT configinstall
  )
ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Export all exported targets to the build tree for use by parent project
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  EXPORT (
      TARGETS ${HDFEOS_LIBRARIES_TO_EXPORT} ${HDFEOS_LIB_DEPENDENCIES}
      FILE ${HDFEOS_PACKAGE}${HDFEOS_PACKAGE_EXT}-targets.cmake
  )
ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Configure the hdfeos-config.cmake file for the build directory
#-----------------------------------------------------------------------------
SET (HDFEOS_INCLUDES_BUILD_TIME
    ${HDFEOS_SRC_DIR}
    ${HDFEOS_BINARY_DIR}
)
SET (HDFEOS_VERSION_STRING @HDFEOS_PACKAGE_VERSION@)
SET (HDFEOS_VERSION_MAJOR  @HDFEOS_PACKAGE_VERSION_MAJOR@)
SET (HDFEOS_VERSION_MINOR  @HDFEOS_PACKAGE_VERSION_MINOR@)

CONFIGURE_FILE (
    ${EOS_RESOURCES_DIR}/hdfeos-config.cmake.build.in 
    ${HDFEOS_BINARY_DIR}/${HDFEOS_PACKAGE}${HDFEOS_PACKAGE_EXT}-config.cmake @ONLY
)

#-----------------------------------------------------------------------------
# Configure the FindHDFEOS.cmake file for the install directory
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  CONFIGURE_FILE (
      ${EOS_RESOURCES_DIR}/FindEOS.cmake.in 
      ${HDFEOS_BINARY_DIR}/CMakeFiles/FindHDFEOS${HDFEOS_PACKAGE_EXT}.cmake @ONLY
  )
  INSTALL (
      FILES ${HDFEOS_BINARY_DIR}/CMakeFiles/FindHDFEOS${HDFEOS_PACKAGE_EXT}.cmake
      DESTINATION ${HDFEOS_INSTALL_CMAKE_DIR}/${HDFEOS_PACKAGE}
      COMPONENT configinstall
  )
ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Configure the hdfeos-config.cmake file for the install directory
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  CONFIGURE_FILE (
      ${EOS_RESOURCES_DIR}/hdfeos-config.cmake.install.in
      ${HDFEOS_BINARY_DIR}/CMakeFiles/${HDFEOS_PACKAGE}${HDFEOS_PACKAGE_EXT}-config.cmake @ONLY
  )
  INSTALL (
      FILES ${HDFEOS_BINARY_DIR}/CMakeFiles/${HDFEOS_PACKAGE}${HDFEOS_PACKAGE_EXT}-config.cmake
      DESTINATION ${HDFEOS_INSTALL_CMAKE_DIR}/${HDFEOS_PACKAGE}
      COMPONENT configinstall
  )
ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Configure the hdfeos-config-version .cmake file for the install directory
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  CONFIGURE_FILE (
      ${EOS_RESOURCES_DIR}/hdfeos-config-version.cmake.in
      ${HDFEOS_BINARY_DIR}/CMakeFiles/${HDFEOS_PACKAGE}${HDFEOS_PACKAGE_EXT}-config-version.cmake @ONLY
  )
  INSTALL (
      FILES ${HDFEOS_BINARY_DIR}/CMakeFiles/${HDFEOS_PACKAGE}${HDFEOS_PACKAGE_EXT}-config-version.cmake
      DESTINATION ${HDFEOS_INSTALL_CMAKE_DIR}/cmake/${HDFEOS_PACKAGE}
      COMPONENT configinstall
  )
ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Add Document File(s) to CMake Install
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_EXTERNALLY_CONFIGURED)
  IF (EXISTS "${HDFEOS_SOURCE_DIR}/doc" AND IS_DIRECTORY "${HDFEOS_SOURCE_DIR}/doc")
    SET (doc_files
        ${HDFEOS_SOURCE_DIR}/doc/README
    )
    INSTALL (
        FILES ${doc_files}
        DESTINATION ${HDFEOS_INSTALL_DATA_DIR}/release_docs
        COMPONENT hdfeosdocuments
    )
  ENDIF (EXISTS "${HDFEOS_SOURCE_DIR}/doc" AND IS_DIRECTORY "${HDFEOS_SOURCE_DIR}/doc")
ENDIF (NOT HDFEOS_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Set the cpack variables
#-----------------------------------------------------------------------------
SET (CPACK_PACKAGE_VENDOR "HDF_Group")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "HDFEOS Installation")
SET (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/README")
SET (CPACK_PACKAGE_NAME "${HDFEOS_PACKAGE_NAME}")
SET (CPACK_PACKAGE_INSTALL_DIRECTORY "${HDFEOS_PACKAGE_NAME}")
SET (CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${HDFEOS_PACKAGE_NAME}-${HDFEOS_PACKAGE_VERSION}-${LIB_TYPE}")
SET (CPACK_PACKAGE_VERSION "${HDFEOS_PACKAGE_VERSION}")
SET (CPACK_PACKAGE_VERSION_MAJOR "${HDFEOS_PACKAGE_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR "${HDFEOS_PACKAGE_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH "")
SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/doc/README")
SET (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/doc/README")
SET (CPACK_PACKAGE_RELOCATABLE TRUE)

IF (WIN32)
  SET (CPACK_MONOLITHIC_INSTALL ON)
  SET (CPACK_NSIS_CONTACT "${HDFEOS_PACKAGE_BUGREPORT}")
  SET (CPACK_NSIS_MODIFY_PATH ON)
  SET (CPACK_NSIS_PACKAGE_NAME "HDFEOS ${HDFEOS_PACKAGE_VERSION}")
ELSE (WIN32)
  SET (CPACK_PACKAGING_INSTALL_PREFIX "/usr")
  SET (CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)

  SET (CPACK_DEBIAN_PACKAGE_SECTION "Libraries")
  SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "${HDFEOS_PACKAGE_BUGREPORT}")
    
  SET (CPACK_RPM_COMPONENT_INSTALL ON)
  SET (CPACK_RPM_PACKAGE_GROUP "Development/Tools")
  SET (CPACK_RPM_PACKAGE_LICENSE "BSD-style")
  SET (CPACK_RPM_PACKAGE_URL "${HDFEOS_PACKAGE_URL}")
  SET (CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "/sbin/ldconfig")
  SET (CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "/sbin/ldconfig")
  SET (CPACK_RPM_POST_INSTALL_SCRIPT_FILE "/sbin/ldconfig")
  SET (CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "/sbin/ldconfig")
  SET (CPACK_STRIP_FILES "${HDFEOS_LIBRARIES_TO_EXPORT};${HDFEOS_APPS_TO_EXPORT};${EXTERNAL_LIBRARY_LIST}")
ENDIF (WIN32)

INCLUDE(InstallRequiredSystemLibraries)

  SET (CPACK_INSTALL_CMAKE_PROJECTS "${HDFEOS_BINARY_DIR};HDFEOS;ALL;/")
  
  IF (HDFEOS_PACKAGE_EXTLIBS)
    IF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
      IF (HDF4_FOUND AND HDF4_USE_EXTERNAL)
        SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${HDF4_INCLUDE_DIR_GEN};HDF4;libraries;/")
        SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${HDF4_INCLUDE_DIR_GEN};HDF4;headers;/")
        SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${HDF4_INCLUDE_DIR_GEN};HDF4;configinstall;/")
        IF (JPEG_USE_EXTERNAL)
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${JPEG_INCLUDE_DIR_GEN};jpeg;libraries;/")
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${JPEG_INCLUDE_DIR_GEN};jpeg;headers;/")
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${JPEG_INCLUDE_DIR_GEN};jpeg;configinstall;/")
        ENDIF (JPEG_USE_EXTERNAL)
        IF (ZLIB_USE_EXTERNAL)
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ZLIB_INCLUDE_DIR_GEN};zlib;libraries;/")
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ZLIB_INCLUDE_DIR_GEN};zlib;headers;/")
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ZLIB_INCLUDE_DIR_GEN};zlib;configinstall;/")
        ENDIF (ZLIB_USE_EXTERNAL)
        IF (SZIP_USE_EXTERNAL)
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${SZIP_INCLUDE_DIR_GEN};szip;libraries;/")
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${SZIP_INCLUDE_DIR_GEN};szip;headers;/")
          SET (CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${SZIP_INCLUDE_DIR_GEN};szip;configinstall;/")
        ENDIF (SZIP_USE_EXTERNAL)
      ENDIF (HDF4_FOUND AND HDF4_USE_EXTERNAL)
    ENDIF (HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "SVN" OR HDFEOS_ALLOW_EXTERNAL_SUPPORT MATCHES "TGZ")
  ENDIF (HDFEOS_PACKAGE_EXTLIBS)

INCLUDE (CPack)

#-----------------------------------------------------------------------------
# Now list the cpack commands
#-----------------------------------------------------------------------------
CPACK_ADD_COMPONENT (applications 
    DISPLAY_NAME "HDFEOS Applications" 
    DEPENDS libraries
    GROUP Applications
)
CPACK_ADD_COMPONENT (libraries 
    DISPLAY_NAME "HDFEOS Libraries"
    GROUP Runtime
)
CPACK_ADD_COMPONENT (headers 
    DISPLAY_NAME "HDFEOS Headers" 
    DEPENDS libraries
    GROUP Development
)
CPACK_ADD_COMPONENT (hdfeosdocuments 
    DISPLAY_NAME "HDFEOS Documents"
    GROUP Documents
)
CPACK_ADD_COMPONENT (configinstall 
    DISPLAY_NAME "HDFEOS CMake files" 
    DEPENDS libraries
    GROUP Development
)

IF (HDFEOS_BUILD_TESTDRIVERS)
  CPACK_ADD_COMPONENT (toolsapplications 
      DISPLAY_NAME "HDFEOS TESTDRIVERS Applications" 
      DEPENDS toolslibraries
      GROUP Applications
  )
  CPACK_ADD_COMPONENT (toolslibraries 
      DISPLAY_NAME "HDFEOS TESTDRIVERS Libraries" 
      DEPENDS libraries
      GROUP Runtime
  )
  CPACK_ADD_COMPONENT (toolsheaders 
      DISPLAY_NAME "HDFEOS TESTDRIVERS Headers" 
      DEPENDS toolslibraries
      GROUP Development
  )
ENDIF (HDFEOS_BUILD_TESTDRIVERS)
