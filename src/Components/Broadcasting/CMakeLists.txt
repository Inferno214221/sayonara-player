project(say_comp_broadcasting)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_interfaces_raw_sound_receiver
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
