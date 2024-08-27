cmake_minimum_required(VERSION 3.13)
project(OCCTWrapper)

if (APPLE)
    # TODO: we need to fix notarization with the separate shared library
    add_library(OCCTWrapper STATIC OCCTWrapper.cpp)
else ()
    add_library(OCCTWrapper MODULE OCCTWrapper.cpp)
endif ()

set_target_properties(OCCTWrapper
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
    PREFIX ""
)

include(GenerateExportHeader)

generate_export_header(OCCTWrapper)

find_package(OpenCASCADE 7.8.1 REQUIRED)
if(NOT OpenCASCADE_FOUND)
  pkg_check_modules(OpenCASCADE QUIET)
endif()

if(OCC_INCLUDE_DIR)
    file(STRINGS ${OCC_INCLUDE_DIR}/Standard_Version.hxx OCC_MAJOR
        REGEX "#define OCC_VERSION_MAJOR.*"
    )
    string(REGEX MATCH "[0-9]+" OCC_MAJOR ${OCC_MAJOR})
    file(STRINGS ${OCC_INCLUDE_DIR}/Standard_Version.hxx OCC_MINOR
        REGEX "#define OCC_VERSION_MINOR.*"
    )
    string(REGEX MATCH "[0-9]+" OCC_MINOR ${OCC_MINOR})
    file(STRINGS ${OCC_INCLUDE_DIR}/Standard_Version.hxx OCC_MAINT
        REGEX "#define OCC_VERSION_MAINTENANCE.*"
    )
    string(REGEX MATCH "[0-9]+" OCC_MAINT ${OCC_MAINT})     
    set(OCC_VERSION_STRING "${OCC_MAJOR}.${OCC_MINOR}.${OCC_MAINT}")
endif(OCC_INCLUDE_DIR)

set(OCCT_LIBS
    # TKXDESTEP
    TKFillet
    TKXSBase
    TKOffset
    TKHLR
    TKXCAF
    TKXSBase
    TKVCAF
    TKCAF
    TKLCAF
    TKCDF
    TKV3d
    TKService
    TKMesh
    TKBO
    TKPrim
    TKHLR
    TKShHealing
    TKTopAlgo
    TKGeomAlgo
    TKBRep
    TKGeomBase
    TKG3d
    TKG2d
    TKMath
    TKernel
)

add_librareis(OCCTWrapper PRIVATE ${OCCT_LIBS}) # !?

if(OCC_VERSION_STRING VERSION_LESS 7.8.0)
    list(APPEND OPENCASCADE_LIBRARY_NAMES  TKIGES TKSTEPBase TKSTEPAttr TKSTEP209 TKSTEP)
else(OCC_VERSION_STRING VERSION_LESS 7.8.0)
    list(APPEND OPENCASCADE_LIBRARY_NAMES TKDESTEP TKDEIGES)
endif(OCC_VERSION_STRING VERSION_LESS 7.8.0)

slic3r_remap_configs("${OCCT_LIBS}" RelWithDebInfo Release)

target_include_directories(OCCTWrapper PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(OCCTWrapper PUBLIC ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(OCCTWrapper ${OCCT_LIBS})

include(GNUInstallDirs)

install(TARGETS OCCTWrapper DESTINATION "${CMAKE_INSTALL_BINDIR}")

