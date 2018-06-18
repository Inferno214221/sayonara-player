project(say_interfaces_library_container)

set(SOURCES
	LibraryContainer.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Widgets
)
