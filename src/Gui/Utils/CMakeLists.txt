project(gui_utils)
set(CMAKE_AUTOUIC OFF)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)
file(GLOB_RECURSE UI_FILES . *.ui)

qt5_wrap_ui(WRAPPED_UI_FILES ${UI_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} ${WRAPPED_UI_FILES})
target_link_libraries(${PROJECT_NAME}
	sayonara_components
	sayonara_utils

	Qt5::Widgets
	Qt5::Gui
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


