project(say_comp_tagging)

file(GLOB_RECURSE SOURCES . *.cpp)

include_directories(${TAGLIB_INCLUDE_DIRS})
link_directories(${TAGLIB_LIBRARY_DIRS})

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_db
	say_utils
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


