project(sayonara_soundcloud)

file(GLOB_RECURSE SOURCES . *.cpp)

set(CMAKE_AUTOUIC ON)
set(RESOURCES
	SoundcloudIcons.qrc
)

qt5_add_resources(RESOURCES_RCC ${RESOURCES})

if(WIN32)
	add_library(${PROJECT_NAME} STATIC ${SOURCES} ${RESOURCES_RCC})
	set(SOUNDCLOUD_INSTALL_PATH
		"share"
	)
else()
	add_library(${PROJECT_NAME} SHARED ${SOURCES} ${RESOURCES_RCC})
	set(SOUNDCLOUD_INSTALL_PATH
		"share/sayonara"
	)

endif()

target_link_libraries(${PROJECT_NAME}
	gui_resources
	gui_library
	interface_librarycontainer
)

if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()

if(NOT WIN32 OR NOT DEFINED WIN32)
	install(TARGETS ${PROJECT_NAME} DESTINATION lib${LIB_SUFFIX}/sayonara)
endif()

install(FILES "soundcloud.db" DESTINATION ${SOUNDCLOUD_INSTALL_PATH})

