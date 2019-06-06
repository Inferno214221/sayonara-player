project(say_interfaces_libraryinterface)

add_subdirectory(LibraryContainer)

set(SOURCES
	LibraryPluginCombobox.cpp
	LibraryPluginHandler.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	sayonara_utils
)
