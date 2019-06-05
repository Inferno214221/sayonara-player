project(gui_playlist)

file(GLOB_RECURSE SOURCES . *.cpp)
add_library(${PROJECT_NAME} ${SOURCES})
