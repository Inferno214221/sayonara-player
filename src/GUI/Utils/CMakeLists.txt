project(say_gui_utils)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE RAW_UI_FILES . *.ui)

qt5_wrap_ui(UI_FILES ${RAW_UI_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_FILES})
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

