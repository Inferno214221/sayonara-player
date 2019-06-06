project(gui_utils)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	sayonara_components
	sayonara_utils

	Qt5::Widgets
	Qt5::Gui
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


