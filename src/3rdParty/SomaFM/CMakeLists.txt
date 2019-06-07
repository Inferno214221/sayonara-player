project( sayonara_somafm )

file(GLOB_RECURSE SOURCES . *.cpp)
set(CMAKE_AUTOUIC ON)

qt5_add_resources(RESOURCES
    ui/SomaFMIcons.qrc
)

if( WIN32 )
    add_library(${PROJECT_NAME} STATIC ${SOURCES} ${RESOURCES})
else()
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${RESOURCES})
endif()

target_link_libraries(${PROJECT_NAME}
	gui_utils
    interface_librarycontainer
    interface_notifications
)

if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()

if(NOT WIN32 OR NOT DEFINED WIN32)
    install(TARGETS ${PROJECT_NAME} DESTINATION lib${LIB_SUFFIX}/sayonara)
endif()
