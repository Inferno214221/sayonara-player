project(sayonara_3rdParty)

cmake_minimum_required(VERSION 2.8)

# Taglib needs this policy
if(POLICY CMP0022)
	cmake_policy(SET CMP0022 NEW)
endif()

if(WITH_SYSTEM_TAGLIB)
	message("Using system's taglib")
else()
	message("Using Sayonara's taglib")
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/taglib/taglib)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/taglib/3rdparty)

	add_subdirectory(taglib)
endif()
