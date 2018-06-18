project(say_gui_covers)

set(SOURCES
	CoverButton.cpp
	AlternativeCoverItemDelegate.cpp
	AlternativeCoverItemModel.cpp
	GUI_AlternativeCovers.cpp
)

qt5_wrap_ui(FORMS_UIC
	GUI_AlternativeCovers.ui
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${FORMS_UIC})
target_link_libraries(${PROJECT_NAME}
    say_comp_covers
    say_gui_utils
)

if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()


