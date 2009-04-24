# Reguire minimum version of CMake
CMake_Minimum_Required(VERSION 2.6)

# AsmJit project - Need to use both C and C++
# Project(BlitJit C CXX)

# If BLITJIT_DIR is not specified, assume that we are building it from here
If(NOT BLITJIT_DIR)
  Set(BLITJIT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
EndIf(NOT BLITJIT_DIR)

# BlitJit C++ sources
Set(BLITJIT_SOURCES
  ${BLITJIT_DIR}/BlitJit/Api.cpp
  ${BLITJIT_DIR}/BlitJit/CodeManager.cpp
  ${BLITJIT_DIR}/BlitJit/Generator.cpp
  ${BLITJIT_DIR}/BlitJit/GeneratorPrivate.cpp
)

# BlitJit C++ headers
Set(BLITJIT_HEADERS
  ${BLITJIT_DIR}/BlitJit/Api.h
  ${BLITJIT_DIR}/BlitJit/BlitJit.h
  ${BLITJIT_DIR}/BlitJit/Build.h
  ${BLITJIT_DIR}/BlitJit/CodeManager.h
  ${BLITJIT_DIR}/BlitJit/Config.h
  ${BLITJIT_DIR}/BlitJit/Generator.h
  ${BLITJIT_DIR}/BlitJit/GeneratorPrivate.h
)

# Include BlitJit to be able to use #include <BlitJit/...>
Include_Directories(${BLITJIT_DIR})

# pthread library is needed for non-windows OSes.
If(NOT WIN32)
  Link_Libraries(pthread)
EndIf(NOT WIN32)

# Debugging support is indicated through _DEBUG macro, add it if needed
If(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  Add_Definitions(-D_DEBUG)
EndIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

# Build BlitJit shared library?
If(BLITJIT_BUILD_LIBRARY)
  Add_Library(BlitJit SHARED ${BLITJIT_SOURCES} ${BLITJIT_HEADERS})
  Target_Link_Libraries(BlitJit AsmJit)

  Install(TARGETS BlitJit DESTINATION "lib")
  
  ForEach(i ${BLITJIT_HEADERS})
    Get_Filename_Component(file ${i} PATH)
    Install(FILES ${i} DESTINATION "include/${file}")
  EndForEach(i)
EndIf(BLITJIT_BUILD_LIBRARY)
