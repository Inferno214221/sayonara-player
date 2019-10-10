project(sayonara_dbus)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_AUTOMOC OFF)

set(DBUS_SOURCES
	DBusAdaptor.cpp
	DBusHandler.cpp
	DBusMPRIS.cpp
	DBusMediaKeysInterface.cpp
	DBusMediaKeysInterfaceGnome.cpp
	DBusMediaKeysInterfaceMate.cpp
	DBusNotifications.cpp
)

set(DBUS_HEADERS
	DBusAdaptor.h
	DBusHandler.h
	DBusMediaKeysInterfaceGnome.h
	DBusMediaKeysInterface.h
	DBusMediaKeysInterfaceMate.h
	DBusMPRIS.h
	DBusNotifications.h
)

qt5_wrap_cpp(DBUS_SOURCES
	DBUS_HEADERS
)

qt5_add_dbus_adaptor(DBUS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/org.mpris.MediaPlayer2.xml
	${CMAKE_SOURCE_DIR}/src/DBus/DBusMPRIS.h
	DBusMPRIS::MediaPlayer2
	org_mpris_media_player2_adaptor
	OrgMprisMediaPlayer2Adaptor
)

qt5_add_dbus_adaptor(DBUS_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/org.mpris.MediaPlayer2.Player.xml
	${CMAKE_SOURCE_DIR}/src/DBus/DBusMPRIS.h
	DBusMPRIS::MediaPlayer2
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

add_library(${PROJECT_NAME} STATIC ${DBUS_SOURCES} ${DBUS_HEADERS})
target_link_libraries(${PROJECT_NAME}
	sayonara_components
	gui_utils
	interface_notifications

	Qt5::Core
	Qt5::DBus
)
