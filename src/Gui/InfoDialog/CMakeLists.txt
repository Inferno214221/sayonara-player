project(gui_infodialog)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

add_library(${PROJECT_NAME}	STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	gui_utils
	gui_tagedit
)
