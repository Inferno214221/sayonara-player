project(gui_library)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_importdialog
	gui_infodialog
	interface_librarycontainer
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

