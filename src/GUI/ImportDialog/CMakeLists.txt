project(say_gui_import_dialog)

set(SOURCES
	GUI_ImportDialog.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_gui_utils
    say_comp_library
	say_gui_tagedit
)
