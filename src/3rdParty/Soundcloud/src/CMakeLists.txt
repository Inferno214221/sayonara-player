project(sayonara_soundcloud)

set(HEADERS
	SoundcloudData.h
	SoundcloudHelper.h
	SoundcloudLibrary.h
)

set(SOURCES
	SoundcloudData.cpp
	SoundcloudHelper.cpp
	SoundcloudLibrary.cpp
)


add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Xml
	say_comp_library
)



