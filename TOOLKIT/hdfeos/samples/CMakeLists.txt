cmake_minimum_required (VERSION 2.8.10)
PROJECT (HDFEOS_SAMPLES)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
LINK_DIRECTORIES (
    ${HDFEOS_SRC_BINARY_DIR}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

#-----------------------------------------------------------------------------
# Define Sources
#-----------------------------------------------------------------------------
SET (samples
    SetupSwath
    DefineFields
    WriteFields
    AppendField
    DefineGDflds
    DefineLevels
    InquireGrid
    InquireSwath
    ReadFields
    ReadGDflds
    ReadLevels
    SetupGrid
    SetupPoint
    SubsetGrid
    SubsetPoint
    SubsetSwath
    UpdateLevels
    WriteGDflds
    WriteLevels
)

FOREACH (sample ${samples})
  ADD_EXECUTABLE (${sample} ${PROJECT_SOURCE_DIR}/${sample}.c)
  TARGET_NAMING (${sample} ${LIB_TYPE})
  TARGET_LINK_LIBRARIES (${sample} ${HDFEOS_SRC_LIB_TARGET} ${HDFEOS_GCTP_SRC_LIB_TARGET} ${LINK_LIBS})
ENDFOREACH (sample ${samples})

IF (HDFEOS_BUILD_FORTRAN AND HAVE_F2CFORTRAN_MACRO)
  # make test dir
  FILE (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/fortran")

  SET (fort_samples
    appendfield
    definefields
    definegdflds
    definelevels
    inquiregrid
    inquireswath
    readfields
    readgdflds
    readlevels
    setupgrid
    setuppoint
    setupswath
    subsetgrid
    subsetpoint
    subsetswath
    updatelevels
    writefields
    writegdflds
    writelevels
  )

  FOREACH (sample ${fort_samples})
    ADD_EXECUTABLE (f_${sample} ${PROJECT_SOURCE_DIR}/${sample}.f)
    TARGET_NAMING (f_${sample} ${LIB_TYPE})
    TARGET_FORTRAN_WIN_PROPERTIES (f_${sample} "")
    SET_TARGET_PROPERTIES (f_${sample} PROPERTIES LINKER_LANGUAGE Fortran)
    TARGET_LINK_LIBRARIES (f_${sample} ${HDFEOS_SRC_LIB_TARGET} ${HDFEOS_GCTP_SRC_LIB_TARGET} ${LINK_LIBS})
  ENDFOREACH (sample ${fort_samples})
ENDIF (HDFEOS_BUILD_FORTRAN AND HAVE_F2CFORTRAN_MACRO)

SET (POINT_REFERENCE_FILES
  fixedBuoy0.txt
  fixedBuoy1.txt
  fixedBuoy1s.txt
  floatBuoy0.txt
  floatBuoy1.txt
  simple.txt
)
FOREACH (out_file ${POINT_REFERENCE_FILES})
  SET (outdest "${PROJECT_BINARY_DIR}/${out_file}")
  #MESSAGE (STATUS " Translating ${out_file}")
  ADD_CUSTOM_COMMAND (
      TARGET     SetupPoint
      POST_BUILD
      COMMAND    ${XLATE_UTILITY}
      ARGS       ${PROJECT_SOURCE_DIR}/${out_file} ${outdest}
  )
ENDFOREACH (out_file ${POINT_REFERENCE_FILES})
  
##############################################################################
##############################################################################
###           T H E   T E S T S                                            ###
##############################################################################
##############################################################################
IF (BUILD_TESTING)
  # Remove any output file left over from previous test run
  ADD_TEST (
      NAME swath_demo-clearall-objects
      COMMAND    ${CMAKE_COMMAND} -E remove  SwathFile.hdf
  )
  SET (sd_last_test "swath_demo-clearall-objects")

  SET (swath_demo
    SetupSwath
    DefineFields
    WriteFields
    ReadFields
    InquireSwath
    AppendField
    SubsetSwath
  )
  FOREACH (sample ${swath_demo})
    ADD_TEST (NAME swath_demo_${sample} COMMAND $<TARGET_FILE:${sample}>)
    IF (NOT "${sd_last_test}" STREQUAL "")
      SET_TESTS_PROPERTIES (swath_demo_${sample} PROPERTIES DEPENDS ${sd_last_test})
    ENDIF (NOT "${sd_last_test}" STREQUAL "")
    SET (sd_last_test "swath_demo_${sample}")
  ENDFOREACH (sample ${swath_demo})

  # Remove any output file left over from previous test run
  ADD_TEST (
      NAME grid_demo-clearall-objects
      COMMAND    ${CMAKE_COMMAND} -E remove GridFile.hdf
  )
  SET (gd_last_test "grid_demo-clearall-objects")

  SET (grid_demo
    SetupGrid
    DefineGDflds
    WriteGDflds
    ReadGDflds
    InquireGrid
    SubsetGrid
  )
  FOREACH (sample ${grid_demo})
    ADD_TEST (NAME grid_demo_${sample} COMMAND $<TARGET_FILE:${sample}>)
    IF (NOT "${gd_last_test}" STREQUAL "")
      SET_TESTS_PROPERTIES (grid_demo_${sample} PROPERTIES DEPENDS ${gd_last_test})
    ENDIF (NOT "${gd_last_test}" STREQUAL "")
    SET (gd_last_test "grid_demo_${sample}")
  ENDFOREACH (sample ${grid_demo})

  # Remove any output file left over from previous test run
  ADD_TEST (
      NAME point_demo-clearall-objects
      COMMAND    ${CMAKE_COMMAND} -E remove  PointFile.hdf
  )
  SET (pd_last_test "point_demo-clearall-objects")

  SET (point_demo
    SetupPoint
    DefineLevels
    WriteLevels
    ReadLevels
  )
  FOREACH (sample ${point_demo})
    ADD_TEST (NAME point_demo_${sample} COMMAND $<TARGET_FILE:${sample}>)
    IF (NOT "${pd_last_test}" STREQUAL "")
      SET_TESTS_PROPERTIES (point_demo_${sample} PROPERTIES DEPENDS ${pd_last_test})
    ENDIF (NOT "${pd_last_test}" STREQUAL "")
    SET (pd_last_test "point_demo_${sample}")
  ENDFOREACH (sample ${point_demo})

  IF (HDFEOS_BUILD_FORTRAN AND HAVE_F2CFORTRAN_MACRO)
    # Remove any output file left over from previous test run
    ADD_TEST (
        NAME swath_demo_fortran-clearall-objects
        COMMAND    ${CMAKE_COMMAND} -E remove SwathFile.hdf
    )
    SET_TESTS_PROPERTIES (swath_demo_fortran-clearall-objects PROPERTIES WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/fortran")
    SET (sdf_last_test "swath_demo_fortran-clearall-objects")

    SET (swath_demo_fortran
      setupswath
      definefields
      writefields
      readfields
      inquireswath
      appendfield
      subsetswath
    )

    FOREACH (sample ${swath_demo_fortran})
      ADD_TEST (NAME swath_demo_fortran_${sample} COMMAND $<TARGET_FILE:f_${sample}>)
      SET_TESTS_PROPERTIES (swath_demo_fortran_${sample} PROPERTIES WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/fortran")
      IF (NOT "${sdf_last_test}" STREQUAL "")
        SET_TESTS_PROPERTIES (swath_demo_fortran_${sample} PROPERTIES DEPENDS ${sdf_last_test})
      ENDIF (NOT "${sdf_last_test}" STREQUAL "")
      SET (sdf_last_test "swath_demo_fortran_${sample}")
    ENDFOREACH (sample ${swath_demo_fortran})

    # Remove any output file left over from previous test run
    ADD_TEST (
        NAME grid_demo_fortran-clearall-objects
        COMMAND    ${CMAKE_COMMAND} -E remove GridFile.hdf
    )
    SET_TESTS_PROPERTIES (grid_demo_fortran-clearall-objects PROPERTIES WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/fortran")
    SET (gdf_last_test "grid_demo_fortran-clearall-objects")

    SET (grid_demo_fortran
      setupgrid
      definegdflds
      writegdflds
      readgdflds
      inquiregrid
      subsetgrid
    )
    FOREACH (sample ${grid_demo_fortran})
      ADD_TEST (NAME grid_demo_fortran_${sample} COMMAND $<TARGET_FILE:f_${sample}>)
      IF (NOT "${gdf_last_test}" STREQUAL "")
        SET_TESTS_PROPERTIES (grid_demo_fortran_${sample} PROPERTIES DEPENDS ${gdf_last_test})
      ENDIF (NOT "${gdf_last_test}" STREQUAL "")
      SET (gdf_last_test "grid_demo_fortran_${sample}")
    ENDFOREACH (sample ${grid_demo_fortran})

    # Remove any output file left over from previous test run
    ADD_TEST (
        NAME point_demo_fortran-clearall-objects
        COMMAND    ${CMAKE_COMMAND} -E remove PointFile.hdf
    )
    SET_TESTS_PROPERTIES (point_demo_fortran-clearall-objects PROPERTIES WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/fortran")
    SET (pdf_last_test "point_demo_fortran-clearall-objects")

    SET (point_demo_fortran
      setuppoint
      definelevels
      writelevels
      readlevels
    )
    FOREACH (sample ${point_demo_fortran})
      ADD_TEST (NAME point_demo_fortran_${sample} COMMAND $<TARGET_FILE:f_${sample}>)
      IF (NOT "${pdf_last_test}" STREQUAL "")
        SET_TESTS_PROPERTIES (point_demo_fortran_${sample} PROPERTIES DEPENDS ${pdf_last_test})
      ENDIF (NOT "${pdf_last_test}" STREQUAL "")
      SET (pdf_last_test "point_demo_fortran_${sample}")
    ENDFOREACH (sample ${point_demo_fortran})

  ENDIF (HDFEOS_BUILD_FORTRAN AND HAVE_F2CFORTRAN_MACRO)

ENDIF (BUILD_TESTING)
