project(say_comp_library)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_db
	say_comp_tagging
	say_comp_playlist
	say_comp_directories
)

