project(gui_plugins)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_utils
)
