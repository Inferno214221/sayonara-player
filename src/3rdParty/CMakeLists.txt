project(sayonara_3rdParty)

if(WITH_SYSTEM_TAGLIB)
	message("Using system's taglib")
else()
	message("Using Sayonara's taglib")
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/taglib/taglib)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/taglib/3rdparty)

	add_subdirectory(taglib)
endif()
