project(sayonara_gui)

set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE GUI_SOURCES . *.cpp)
file(GLOB_RECURSE GUI_HEADERS . *.h)

set(GUI_RESOURCES
	Resources/Icons.qrc
	SomaFM/SomaFMIcons.qrc
	Soundcloud/SoundcloudIcons.qrc
)

if(WIN32)
	set(GUI_RESOURCES
		${GUI_RESOURCES}
		Resources/IconsWindows.qrc
	)
endif()

qt5_add_resources(GUI_RESOURCES_RCC
	${GUI_RESOURCES}
)

add_library(${PROJECT_NAME} STATIC ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_RESOURCES_RCC})
target_link_libraries(${PROJECT_NAME}
	sayonara_components

	Qt5::Gui
	Qt5::Widgets
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
