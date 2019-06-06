project(gui_directories)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	#gui_infodialog, gui_importdialog, gui_utils
	gui_library
)
