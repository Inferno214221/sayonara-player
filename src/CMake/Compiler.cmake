include(CheckCXXCompilerFlag)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#SET(COMMON_FLAGS "-fno-diagnostics-show-caret -ftrack-macro-expansion=0")

message("Build type = ${CMAKE_BUILD_TYPE}")

if(WITH_CCACHE)
	# Configure CCache if available
	find_program(CCACHE_FOUND ccache)
	if(CCACHE_FOUND)
			set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
			set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
			message("Found CCache")
	endif(CCACHE_FOUND)
endif(WITH_CCACHE)
set(COMMON_FLAGS_TEST
		"-Woverloaded-virtual"
		"-Wall"
		"-Wunreachable-code"
		"-Wextra"
		"-Wpedantic"
		"-pthread"
		"-Wno-old-style-cast"
)

if(NOT WIN32 OR NOT DEFINED WIN32)
	set(COMMON_FLAGS_TEST
		${COMMON_FLAGS_TEST}
		"-fPIC"
	)
endif()

foreach(FLAG ${COMMON_FLAGS_TEST})
		CHECK_CXX_COMPILER_FLAG(${FLAG} CXX_COMPILER_FLAG_AVAILABLE)
		if( ${CXX_COMPILER_FLAG_AVAILABLE} )
			message("Add ${FLAG}")
			set(COMMON_FLAGS "${COMMON_FLAGS} ${FLAG}")
		endif()
endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${COMMON_FLAGS}")

if ( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
	message("Debug Mode active")
	add_definitions(-DDEBUG)
endif()

set(CMAKE_CXX_FLAGS_NONE "${CMAKE_CXX_FLAGS_NONE} ${COMMON_FLAGS}")


