project(say_gui_tagedit)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_comp_tagging
	say_gui_utils
	say_gui_covers
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


