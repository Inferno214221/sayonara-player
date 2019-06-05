project(say_gui)

set(CMAKE_AUTOUIC ON)

add_subdirectory(Resources)

file(GLOB_RECURSE SOURCES . *.cpp)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(${PROJECT_NAME}
	sayonara_components

	say_interfaces_library_container
	say_interfaces_notifications

	say_utils

	Qt5::Widgets
	Qt5::DBus
)
