project(say_db)

set(CMAKE_AUTOMOC OFF)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES} )
target_link_libraries(${PROJECT_NAME}
	sayonara_utils
	Qt5::Sql
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


