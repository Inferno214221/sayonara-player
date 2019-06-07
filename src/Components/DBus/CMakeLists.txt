file(GLOB_RECURSE DBUS_SOURCES . *.cpp)
project(comp_dbus)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt5_add_dbus_adaptor(DBUS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/org.mpris.MediaPlayer2.xml
	${CMAKE_SOURCE_DIR}/src/Components/DBus/DBusMPRIS.h
	DBusMPRIS::MediaPlayer2
	org_mpris_media_player2_adaptor
	OrgMprisMediaPlayer2Adaptor
)

qt5_add_dbus_adaptor(DBUS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/org.mpris.MediaPlayer2.Player.xml
	${CMAKE_SOURCE_DIR}/src/Components/DBus/DBusMprisMp2Player.h
	DBusMPRIS::MediaPlayer2Player
	org_mpris_media_player2_player_adaptor
	OrgMprisMediaPlayer2PlayerAdaptor
)

qt5_add_dbus_interface(DBUS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/org.freedesktop.Notifications.xml
	Notifications
)

qt5_add_dbus_interface(DBUS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/org.mate.SettingsDaemon.MediaKeys.xml
	mate_settings_daemon
)

qt5_add_dbus_interface(DBUS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/org.gnome.SettingsDaemon.MediaKeys.xml
	gnome_settings_daemon
)

set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)
#target_sources(${PROJECT_NAME} PRIVATE ${DBUS_SOURCES})
add_library(${PROJECT_NAME} STATIC ${DBUS_SOURCES})
