project(say_gui_playlist)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE RAW_UI_FILES . *.ui)

qt5_wrap_ui(UI_FILES ${RAW_UI_FILES})

set(LINK_LIBS
	say_comp_bookmarks
	say_comp_playlist
	say_comp_preferences
	say_gui_info_dialog
	say_gui_shutdown_dialog
	say_gui_utils
)

if(NOT WIN32 OR NOT DEFINED WIN32)
set(LINK_LIBS
	${LINK_LIBS}
	say_comp_shutdown
)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_FILES})
target_link_libraries(${PROJECT_NAME}
	${LINK_LIBS}
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


