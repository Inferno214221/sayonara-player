project(say_gui_tagedit)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE RAW_UI_FILES . *.ui)

qt5_wrap_ui(UI_FILES ${RAW_UI_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_FILES})
target_link_libraries(${PROJECT_NAME}
	say_comp_tagging
	say_gui_utils
	say_gui_covers
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()


