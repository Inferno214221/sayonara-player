project(say_comp_engine)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_BASE_APP_LIBRARIES}

	say_comp_playmanager
	say_utils
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
