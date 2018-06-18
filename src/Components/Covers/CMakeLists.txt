project(say_comp_covers)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_db
	Qt5::Gui
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
