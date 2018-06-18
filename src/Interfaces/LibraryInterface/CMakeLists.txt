project(say_interfaces_libraryinterface)

add_subdirectory(LibraryContainer)

set(SOURCES
    LibraryPluginHandler.cpp
	LibraryPluginCombobox.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    say_gui_utils
    say_gui_library
)


