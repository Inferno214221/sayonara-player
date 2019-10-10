project(interface_library)

set(SOURCES
	Private/LibraryPluginCombobox.cpp
	Private/LibraryPluginComboBoxDelegate.cpp	
	LibraryPluginHandler.cpp
)

set(HEADERS
	Private/LibraryPluginCombobox.h
	Private/LibraryPluginComboBoxDelegate.h
	LibraryPluginHandler.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	gui_utils
)
