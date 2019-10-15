project(interface_notifications)

set(SOURCES
	NotificationInterface.cpp
	NotificationHandler.cpp
)

set(HEADERS
	NotificationInterface.h
	NotificationHandler.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	sayonara_utils
)

if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()

