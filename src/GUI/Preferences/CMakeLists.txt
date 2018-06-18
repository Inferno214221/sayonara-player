project( say_gui_preferences )

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE RAW_UI_FILES . *.ui)

qt5_wrap_ui(UI_FILES ${RAW_UI_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_FILES})
target_link_libraries(${PROJECT_NAME}
	say_comp_preferences
	say_comp_streamplugins
	say_comp_covers
	say_comp_library
	say_interfaces_preference_dialog
)


if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

