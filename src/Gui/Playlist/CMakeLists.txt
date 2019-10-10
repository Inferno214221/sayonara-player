project(gui_playlist)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# InfoDialog, Plugins, Shutdown, Utils
target_link_libraries(${PROJECT_NAME}
	gui_infodialog
	gui_plugins
	gui_shutdown
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

