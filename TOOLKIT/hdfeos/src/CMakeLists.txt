cmake_minimum_required (VERSION 2.8.10)
PROJECT (HDFEOS_SRC C CXX)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
LINK_DIRECTORIES (
    ${HDFEOS_SRC_BINARY_DIR}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

SET (HDFEOS_SRC_CSRCS
    ${HDFEOS_SRC_SOURCE_DIR}/EHapi.c
    ${HDFEOS_SRC_SOURCE_DIR}/GDapi.c
    ${HDFEOS_SRC_SOURCE_DIR}/PTapi.c
    ${HDFEOS_SRC_SOURCE_DIR}/SWapi.c
)

SET (HDFEOS_SRC_CHDRS
#    ${HDFEOS_INC_DIR}/HE2_config.h
    ${HDFEOS_INC_DIR}/HdfEosDef.h
    ${HDFEOS_INC_DIR}/HDFEOSVersion.h
    ${HDFEOS_INC_DIR}/cfortHdf.h
    ${HDFEOS_INC_DIR}/ease.h
)

IF (H5_USE_HDFEOS2)
  ADD_DEFINITIONS (-DHAVE_LIBHDFEOS)
ENDIF (H5_USE_HDFEOS2)

ADD_LIBRARY (${HDFEOS_SRC_LIB_TARGET} ${LIB_TYPE} ${HDFEOS_SRC_CSRCS} ${HDFEOS_SRC_CHDRS})
IF (MSVC)
  TARGET_LINK_LIBRARIES (${HDFEOS_SRC_LIB_TARGET} "ws2_32.lib")
ELSE (MSVC)
  SET_TARGET_PROPERTIES (${HDFEOS_SRC_LIB_TARGET} PROPERTIES LINK_FLAGS "-Wl,-single_module")
ENDIF (MSVC)
TARGET_LINK_LIBRARIES (${HDFEOS_SRC_LIB_TARGET} ${LINK_LIBS})
SET_GLOBAL_VARIABLE (HDFEOS_LIBRARIES_TO_EXPORT "${HDFEOS_LIBRARIES_TO_EXPORT};${HDFEOS_SRC_LIB_TARGET}")
HDFEOS_SET_LIB_OPTIONS (${HDFEOS_SRC_LIB_TARGET} ${HDFEOS_SRC_LIB_NAME} ${LIB_TYPE})

#-----------------------------------------------------------------------------
# Add file(s) to CMake Install
#-----------------------------------------------------------------------------
IF (NOT HDFEOS_INSTALL_NO_DEVELOPMENT)
  INSTALL (
      FILES
          ${HDFEOS_SRC_CHDRS}
      DESTINATION
          ${HDFEOS_INSTALL_INCLUDE_DIR}
      COMPONENT
          headers
  )
ENDIF (NOT HDFEOS_INSTALL_NO_DEVELOPMENT)

#-----------------------------------------------------------------------------
# Add Target(s) to CMake Install for import into other projects
#-----------------------------------------------------------------------------
IF (HDFEOS_EXPORTED_TARGETS)

  INSTALL_TARGET_PDB (${HDFEOS_SRC_LIB_TARGET} ${HDFEOS_INSTALL_LIB_DIR} libraries)
  
  INSTALL (
      TARGETS 
          ${HDFEOS_SRC_LIB_TARGET}
      EXPORT
          ${HDFEOS_EXPORTED_TARGETS}
      LIBRARY DESTINATION ${HDFEOS_INSTALL_LIB_DIR} COMPONENT libraries 
      ARCHIVE DESTINATION ${HDFEOS_INSTALL_LIB_DIR} COMPONENT libraries
      RUNTIME DESTINATION ${HDFEOS_INSTALL_BIN_DIR} COMPONENT libraries
  )
ENDIF (HDFEOS_EXPORTED_TARGETS)
