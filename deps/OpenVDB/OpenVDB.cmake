if(BUILD_SHARED_LIBS)
    set(_build_shared ON)
    set(_build_static OFF)
else()
    set(_build_shared OFF)
    set(_build_static ON)
endif()

set (_openvdb_vdbprint ON)
# if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
#     # Build fails on raspberry pi due to missing link directive to latomic
#     # Let's hope it will be fixed soon.
#     set (_openvdb_vdbprint OFF)
# endif ()

prusaslicer_add_cmake_project(OpenVDB
    # 11.0.0
    Git_REPOSITORY https://github.com/AcademySoftwareFoundation/openvdb.git
    Git_TAG 6c044e628a1ac3546eed48b6f5e9c76dfaa2143e
    DEPENDS dep_TBB dep_Blosc dep_OpenEXR dep_Boost
    CMAKE_ARGS
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
        -DOPENVDB_BUILD_PYTHON_MODULE=OFF
        -DUSE_BLOSC=ON
        -DOPENVDB_CORE_SHARED=${_build_shared} 
        -DOPENVDB_CORE_STATIC=${_build_static}
        -DOPENVDB_ENABLE_RPATH:BOOL=OFF
        -DTBB_STATIC=${_build_static}
        -DOPENVDB_BUILD_VDB_PRINT=${_openvdb_vdbprint}
        -DDISABLE_DEPENDENCY_VERSION_CHECKS=ON # Centos6 has old zlib
)

if (MSVC)
    if (${DEP_DEBUG})
        ExternalProject_Get_Property(dep_OpenVDB BINARY_DIR)
        ExternalProject_Add_Step(dep_OpenVDB build_debug
            DEPENDEES build
            DEPENDERS install
            COMMAND ${CMAKE_COMMAND} ../dep_OpenVDB -DOPENVDB_BUILD_VDB_PRINT=OFF
            COMMAND msbuild /m /P:Configuration=Debug INSTALL.vcxproj
            WORKING_DIRECTORY "${BINARY_DIR}"
        )
    endif ()
endif ()