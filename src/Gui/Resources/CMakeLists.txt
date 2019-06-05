project(gui_resources)

add_subdirectory(Icons)

set(RESOURCES Icons.qrc)

if(WIN32)
	set(RESOURCES
		${RESOURCES}
		IconsWindows.qrc
	)
endif()

qt5_add_resources(RESOURCES_RCC
	${RESOURCES}
)

add_library(${PROJECT_NAME} STATIC ${RESOURCES_RCC})

if(WIN32)
	set(INSTALL_TARGET_PATH
		"share"
	)
else()
	set(INSTALL_TARGET_PATH
		"share/sayonara"
	)
endif()

install(FILES "standard.css" DESTINATION ${INSTALL_TARGET_PATH})
install(FILES "dark.css" DESTINATION ${INSTALL_TARGET_PATH})


