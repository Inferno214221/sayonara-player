project(say_comp_tagging)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_db
	say_utils
	say_comp_metadata_info
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


