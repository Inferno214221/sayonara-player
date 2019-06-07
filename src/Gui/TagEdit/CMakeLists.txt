project(gui_tagedit)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_utils
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

