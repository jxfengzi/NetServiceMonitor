CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(NetServiceMonitor CXX)

#---------------------------------------------------------------------------------------
#
# cxx flags
#
#---------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------
#
# include dir
#
#---------------------------------------------------------------------------------------
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/3rdpart/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

#--------------------------------------------------------------------------------------
#
# 3rdpart library dir
#
#--------------------------------------------------------------------------------------
IF (WIN32)
  LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/3rdpart/lib/Win32)
  MESSAGE(STATUS "3rdpart Library Dir : ${PROJECT_SOURCE_DIR}/3rdpart/lib/Win32")
ELSE (WIN32)
  LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/3rdpart/lib/Linux64)
  MESSAGE(STATUS "3rdpart Library Dir : ${PROJECT_SOURCE_DIR}/3rdpart/lib/Linux64")
ENDIF (WIN32)

#--------------------------------------------------------------------------------------
#
# OS library
#
#--------------------------------------------------------------------------------------
IF (WIN32)
  SET(OS_LIB ws2_32.lib)
    MESSAGE(STATUS "OS_LIB for Win32: ${OS_LIB}")
  SET(DNSSD_LIB dnssd.lib)
    MESSAGE(STATUS "DNSSD_LIB for Win32: ${DNSSD_LIB}")
ELSE (WIN32)
  SET(OS_LIB pthread)
    MESSAGE(STATUS "OS_LIB for Linux: ${OS_LIB}")
  SET(DNSSD_LIB dns_sd)
    MESSAGE(STATUS "DNSSD_LIB for Linux: ${DNSSD_LIB}")
ENDIF (WIN32)

#-------------------------------------------------------------------------------------
# set output dir
#-------------------------------------------------------------------------------------
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)
MESSAGE(STATUS "executable output:  ${EXECUTABLE_OUTPUT_PATH}")
MESSAGE(STATUS "library output: ${LIBRARY_OUTPUT_PATH}")

#-------------------------------------------------------------------------------------
#
# project
#
#-------------------------------------------------------------------------------------
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)
