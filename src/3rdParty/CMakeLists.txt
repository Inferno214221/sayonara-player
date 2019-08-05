project(sayonara_3rdParty)

if(WITH_SYSTEM_TAGLIB)
	message("Using system's taglib")
else()

	message("Using Sayonara's taglib")
	configure_file(taglib/taglib_config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/taglib_config.h")

	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/taglib)

	add_subdirectory(taglib)
endif()
