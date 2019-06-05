project(gui_shutdown)

file(GLOB_RECURSE SOURCES . *.cpp)
add_library(${PROJECT_NAME} ${SOURCES})
