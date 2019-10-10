project(gui_somafm)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

qt5_add_resources(RESOURCES SomaFMIcons.qrc)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} ${RESOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_utils
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

