# FindNanoSVG.cmake
message(STATUS "Looking for NanoSVG")

# Search for NanoSVG library
find_path(NANOSVG_INCLUDE_DIR
	NAMES nanosvg.h nanosvgrast.h
	PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/../deps/NanoSVG/src
	${CMAKE_CURRENT_SOURCE_DIR}/../external/nanosvg/include
	DOC "Path to NanoSVG include directory"
)

find_library(NANOSVG_LIBRARY
	NAMES nanosvg
	PATHS
	${CMAKE_CURRENT_SOURCE_DIR}/../deps/NanoSVG/build
	${CMAKE_CURRENT_SOURCE_DIR}/../external/nanosvg/lib
	DOC "Path to NanoSVG library"
)

message(STATUS "NanoSVG include dir: ${NANOSVG_INCLUDE_DIR}")
message(STATUS "NanoSVG library: ${NANOSVG_LIBRARY}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# Check if NanoSVG was found
if(NANOSVG_INCLUDE_DIR AND NANOSVG_LIBRARY)
	set(NANOSVG_FOUND TRUE)
else()
	set(NANOSVG_FOUND FALSE)
endif()

# Provide variables to the user
if(NANOSVG_FOUND)
	if(NOT NanoSVG_FIND_QUIETLY)
		message(STATUS "Found NanoSVG: ${NANOSVG_LIBRARY}")
	endif()
else()
	if(NOT NanoSVG_FIND_QUIETLY)
		message(ERROR " NanoSVG not found")
	endif()
endif()

# Export variables
set(NANOSVG_INCLUDE_DIRS ${NANOSVG_INCLUDE_DIR} CACHE PATH "NanoSVG include directories")
set(NANOSVG_LIBRARIES ${NANOSVG_LIBRARY} CACHE FILEPATH "NanoSVG libraries")
