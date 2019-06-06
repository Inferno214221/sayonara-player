project(gui_utils)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${WRAPPED_UI_FILES} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	sayonara_components
	say_utils
	Qt5::Widgets
)
