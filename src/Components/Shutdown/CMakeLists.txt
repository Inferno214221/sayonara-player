project(say_comp_shutdown)

set(SOURCES
	Shutdown.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_comp_playmanager
	say_interfaces_notifications
	Qt5::DBus
)
