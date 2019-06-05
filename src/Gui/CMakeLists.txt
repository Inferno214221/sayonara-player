project(sayonara_gui)

set(CMAKE_AUTOUIC OFF)

file(GLOB_RECURSE UI_FILES . *.ui)
qt5_wrap_ui(WRAPPED_UI_FILES ${UI_FILES})

add_subdirectory(Resources)

file(GLOB_RECURSE SOURCES Covers *.cpp)
file(GLOB_RECURSE SOURCES Directories *.cpp)
file(GLOB_RECURSE SOURCES ImportDialog *.cpp)
file(GLOB_RECURSE SOURCES InfoDialog *.cpp)
file(GLOB_RECURSE SOURCES Library *.cpp)
file(GLOB_RECURSE SOURCES Player *.cpp)
file(GLOB_RECURSE SOURCES Playlist *.cpp)
file(GLOB_RECURSE SOURCES Plugins *.cpp)
file(GLOB_RECURSE SOURCES Preferences *.cpp)
file(GLOB_RECURSE SOURCES Shutdown *.cpp)
file(GLOB_RECURSE SOURCES TagEdit *.cpp)
file(GLOB_RECURSE SOURCES Utils *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${WRAPPED_UI_FILES})
target_link_libraries(${PROJECT_NAME}
	sayonara_components

	say_interfaces_library_container
	say_interfaces_notifications

	say_utils

	Qt5::Widgets
	Qt5::DBus
)
