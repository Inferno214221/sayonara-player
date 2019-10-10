project(interface_rawsoundreceiver)

set(SOURCES
	RawSoundReceiverInterface.cpp
)

set(HEADERS
	RawSoundReceiverInterface.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
)
