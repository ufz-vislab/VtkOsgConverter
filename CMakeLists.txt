CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(VtkOsgConverter)
SET(VTKOSGCONVERTER_MAJOR_VERSION 1)
SET(VTKOSGCONVERTER_MINOR_VERSION 1)
SET(VTKOSGCONVERTER_PATCH_VERSION 0)
SET(VTKOSGCONVERTER_VERSION
  ${VTKOSGCONVERTER_MAJOR_VERSION}.${VTKOSGCONVERTER_MINOR_VERSION}.${VTKOSGCONVERTER_PATCH_VERSION})

# Find
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
FIND_PACKAGE(ParaView)
FIND_PACKAGE(OpenSG COMPONENTS OSGBase OSGSystem REQUIRED)
IF(ParaView_FOUND)
  INCLUDE(${PARAVIEW_USE_FILE})
  OPTION(INSTALL_IN_PARAVIEW "This will install the plugin in ParaView." OFF)
ELSE()
  FIND_PACKAGE(VTK)
  IF(OGS_USE_QT AND (VTK_VERSION_MAJOR LESS 5 OR VTK_VERSION_MINOR LESS 8))
    MESSAGE(FATAL_ERROR "VTK not found but is required! Minimum version: VTK 5.8.x")
  ENDIF()
  INCLUDE( ${VTK_USE_FILE} )
ENDIF()

# Offer the user the choice of overriding the installation directories
SET(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
SET(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
SET(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")
IF(WIN32 AND NOT CYGWIN)
  SET(DEF_INSTALL_CMAKE_DIR CMake)
ELSE()
  SET(DEF_INSTALL_CMAKE_DIR lib/CMake/VtkFbxConverter)
ENDIF()
SET(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
FOREACH(p LIB BIN INCLUDE CMAKE)
  SET(var INSTALL_${p}_DIR)
  IF(NOT IS_ABSOLUTE "${${var}}")
    SET(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  ENDIF()
ENDFOREACH()

INCLUDE_DIRECTORIES(
  .
)

# Library
ADD_LIBRARY(VtkOsgConverter
  vtkOsgConverter.h
  vtkOsgConverter.cpp
)

SET_TARGET_PROPERTIES(VtkOsgConverter PROPERTIES
  PUBLIC_HEADER "VtkOsgConverter.h")

USE_OPENSG(VtkOsgConverter)

IF(NOT INSTALL_IN_PARAVIEW)
  INSTALL(TARGETS VtkOsgConverter
    # IMPORTANT: Add the library to the "export-set"
    EXPORT VtkFbxConverterLibraryDepends
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
    ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/VtkOsgConverter"
      COMPONENT dev)
ENDIF()

# Add all targets to the build-tree export set
EXPORT(TARGETS VtkOsgConverter
  FILE "${PROJECT_BINARY_DIR}/VtkOsgConverterLibraryDepends.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
EXPORT(PACKAGE VtkOsgConverter)

# Create a VtkFbxConverterBuildTreeSettings.cmake file for the use from the build tree
CONFIGURE_FILE(VtkOsgConverterBuildTreeSettings.cmake.in
  "${PROJECT_BINARY_DIR}/VtkOsgConverterBuildTreeSettings.cmake" @ONLY)

# Create the VtkFbxConverterConfig.cmake and VtkFbxConverterConfigVersion files
FILE(RELATIVE_PATH CONF_REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
CONFIGURE_FILE(VtkOsgConverterConfig.cmake.in
  "${PROJECT_BINARY_DIR}/VtkOsgConverterConfig.cmake" @ONLY)
CONFIGURE_FILE(VtkOsgConverterConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/VtkOsgConverterConfigVersion.cmake" @ONLY)

IF(ParaView_FOUND)
  ADD_SUBDIRECTORY(pv_plugin)
ENDIF()
