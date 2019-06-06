project(gui_library)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_importdialog
	gui_infodialog
	say_interfaces_library_container
)
