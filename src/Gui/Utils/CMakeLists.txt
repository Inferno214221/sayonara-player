project(say_gui_utils)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	Qt5::Widgets
	Qt5::DBus
	say_utils
	say_comp_directories
	say_comp_preferences
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

