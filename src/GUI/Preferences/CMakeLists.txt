project( say_gui_preferences )

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_comp_preferences
	say_comp_streamplugins
	say_comp_covers
	say_comp_library
	say_interfaces_preference_dialog
)


if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

