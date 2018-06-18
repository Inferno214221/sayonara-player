project(say_interfaces_preference_dialog)

set(SOURCES
	PreferenceAction.cpp
	PreferenceWidget.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_gui_utils
)

if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()

