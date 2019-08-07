project(interface_library)

add_subdirectory(LibraryContainer)

set(SOURCES
	Private/LibraryPluginCombobox.cpp
	Private/LibraryPluginComboBoxDelegate.cpp	
	LibraryPluginHandler.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_utils
)
