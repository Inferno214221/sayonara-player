project(gui_library)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	gui_importdialog
	gui_infodialog
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

