project(sayonara_application)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR},
	${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

set(APPLICATION_LIBRARIES
	sayonara_gui
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	${APPLICATION_LIBRARIES}
	sayonara_dbus
	sayonara_utils
)

