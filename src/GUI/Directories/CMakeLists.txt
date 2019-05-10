project(say_gui_directory_widget)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_gui_info_dialog
	say_gui_import_dialog
	say_comp_directories
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()



