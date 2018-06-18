project(say_comp_dbus)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR},
	${CMAKE_CURRENT_SOURCE_DIR})

#file(GLOB_RECURSE HEADERS . *.h)
file(GLOB_RECURSE SOURCES . *.cpp)

QT5_ADD_DBUS_ADAPTOR(SOURCES
	resources/org.mpris.MediaPlayer2.xml
	DBusMPRIS.h
	DBusMPRIS::MediaPlayer2
	org_mpris_media_player2_adaptor
	OrgMprisMediaPlayer2Adaptor
)

QT5_ADD_DBUS_ADAPTOR(SOURCES
	resources/org.mpris.MediaPlayer2.Player.xml
	DBusMPRIS.h
	DBusMPRIS::MediaPlayer2
	org_mpris_media_player2_player_adaptor
	OrgMprisMediaPlayer2PlayerAdaptor
)

QT5_ADD_DBUS_INTERFACE(SOURCES
	resources/org.freedesktop.Notifications.xml
	Notifications
)

QT5_ADD_DBUS_INTERFACE(SOURCES
	resources/org.mate.SettingsDaemon.MediaKeys.xml
	mate_settings_daemon
)

QT5_ADD_DBUS_INTERFACE(SOURCES
	resources/org.gnome.SettingsDaemon.MediaKeys.xml
	gnome_settings_daemon
)

QT5_ADD_DBUS_INTERFACE(SOURCES
	resources/org.gnome.SettingsDaemon.MediaKeys.xml
	gnome_settings_daemon
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	Qt5::Gui
	say_interfaces_notifications
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

