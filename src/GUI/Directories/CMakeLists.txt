project(say_gui_directory_widget)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE RAW_UI_FILES . *.ui)

qt5_wrap_ui(UI_FILES ${RAW_UI_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_FILES})
target_link_libraries(${PROJECT_NAME}
	say_gui_info_dialog
	say_gui_import_dialog
	say_comp_directories
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()



