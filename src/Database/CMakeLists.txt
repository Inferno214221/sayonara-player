project(sayonara_database)

# No moc files in database
set(CMAKE_AUTOMOC OFF)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	sayonara_utils
	Qt5::Sql
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


