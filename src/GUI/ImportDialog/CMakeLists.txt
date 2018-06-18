project(say_gui_import_dialog)

set(SOURCES
	GUI_ImportDialog.cpp
)

qt5_wrap_ui(FORMS_UIC
	GUI_ImportDialog.ui
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${FORMS_UIC})
target_link_libraries(${PROJECT_NAME}
	say_gui_utils
    say_comp_library
	say_gui_tagedit
)
