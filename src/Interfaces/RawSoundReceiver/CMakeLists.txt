project(say_interfaces_raw_sound_receiver)

set(SOURCES
	RawSoundReceiverInterface.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
)
