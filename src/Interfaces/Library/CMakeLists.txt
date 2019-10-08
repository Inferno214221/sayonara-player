project(interface_library)

set(SOURCES
	Private/LibraryPluginCombobox.cpp
	Private/LibraryPluginComboBoxDelegate.cpp	
	LibraryPluginHandler.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_utils
)
