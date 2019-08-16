project(sayonara_3rdParty)

if(WITH_SYSTEM_TAGLIB)
	message("Using system's taglib")
else()

	set(TAGLIB_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/taglib/taglib_config.h)
	configure_file(taglib/taglib_config.h.cmake ${TAGLIB_CONFIG_OUT})

	set(TAGLIB_INCLUDE_BIN_PATH ${CMAKE_CURRENT_BINARY_DIR}/taglib)

	file(GLOB_RECURSE HEADERS .
		*.h
		*.tcc
	)

	file(COPY ${HEADERS} DESTINATION ${TAGLIB_INCLUDE_BIN_PATH})

	message("Using Sayonara's taglib")

	# utf8-cpp
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})

	add_subdirectory(taglib)
endif()
