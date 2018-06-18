project(say_interfaces_notifications)

set(SOURCES
	NotificationInterface.cpp
	NotificationHandler.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_utils
)


if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()

