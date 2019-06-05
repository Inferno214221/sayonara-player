project(sayonara_engine)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	${GSTREAMER_LIBRARIES}
    ${GSTREAMER_BASE_LIBRARIES}
    ${GSTREAMER_BASE_APP_LIBRARIES}
)

