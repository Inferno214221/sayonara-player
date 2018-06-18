project(say_comp_remote_control)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Network
	Qt5::Gui

	say_comp_playmanager
	say_comp_playlist
	say_comp_covers
)
