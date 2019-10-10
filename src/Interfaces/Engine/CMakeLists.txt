project(interface_audiodatareceiver)

set(SOURCES
	AudioDataReceiverInterface.cpp
)

set(HEADERS
	AudioDataReceiverInterface.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
)
