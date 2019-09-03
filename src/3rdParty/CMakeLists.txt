project(sayonara_3rdParty)

if(WITH_SYSTEM_TAGLIB)
	message("Using system's taglib")
else()
	message("Using Sayonara's taglib")

	configure_file(Taglib/taglib_config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/Taglib/taglib_config.h)
	file(GLOB_RECURSE HEADERS .
		*.h
		*.tcc
	)

	file(COPY ${HEADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/taglib)

	# utf8-cpp
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})

	add_subdirectory(Taglib)
endif()
