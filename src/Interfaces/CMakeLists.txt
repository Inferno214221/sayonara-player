project(sayonara_interfaces)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} PlaylistInterface.h)
target_link_libraries(${PROJECT_NAME}
	sayonara_utils
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
