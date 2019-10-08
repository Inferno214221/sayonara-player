project(gui_library)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_importdialog
	gui_infodialog
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

