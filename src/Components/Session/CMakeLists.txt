project(say_comp_session)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES} )
target_link_libraries(${PROJECT_NAME}
	say_comp_playmanager
	say_db
	say_utils
)
