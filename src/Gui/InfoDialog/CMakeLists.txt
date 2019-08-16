project(gui_infodialog)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME}	STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_utils
	gui_tagedit
)
