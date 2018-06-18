project(say_comp_playlist)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_db
	say_comp_playmanager
	say_comp_directories
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

