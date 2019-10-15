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
