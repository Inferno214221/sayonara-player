project( sayonara_somafm )

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE RAW_UI_FILES . *.ui)

qt5_wrap_ui(UI_FILES ${RAW_UI_FILES})
qt5_add_resources(RESOURCES
    ui/SomaFMIcons.qrc
)

if( WIN32 )
    add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_FILES} ${RESOURCES})
else()
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${UI_FILES} ${RESOURCES})
endif()

target_link_libraries(${PROJECT_NAME}
    say_gui_utils
    say_comp_playlist
    say_interfaces_library_container
)

if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()

if(NOT WIN32 OR NOT DEFINED WIN32)
    install(TARGETS ${PROJECT_NAME} DESTINATION lib${LIB_SUFFIX}/sayonara)
endif()
