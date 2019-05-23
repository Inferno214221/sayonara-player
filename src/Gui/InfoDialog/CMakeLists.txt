project(say_gui_info_dialog)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_comp_lyrics
	say_comp_metadata_info
	say_gui_tagedit
	say_gui_covers
)
