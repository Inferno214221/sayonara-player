project(say_gui_library)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_comp_library


	say_gui_utils
	say_gui_info_dialog
	say_gui_import_dialog


)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

