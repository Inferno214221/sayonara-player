/* application.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Application.h"
#include "InstanceThread.h"
#include "MetaTypeRegistry.h"
#include "LibraryWatcher.h"

#include "Gui/Utils/Icons.h"

#ifdef WITH_DBUS
#include "Components/DBus/DBusHandler.h"
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include "3rdParty/SomaFM/ui/SomaFMLibraryContainer.h"
#include "3rdParty/Soundcloud/ui/GUI_SoundcloudLibrary.h"
#endif

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/RemoteControl/RemoteControl.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Streaming/LastFM/LastFM.h"
#include "Components/Session/Session.h"

#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"
#include "Interfaces/Notification/NotificationHandler.h"

#include "Gui/Utils/GuiUtils.h"

#include "Gui/Player/GUI_Player.h"
#include "Gui/Library/LocalLibraryContainer.h"
#include "Gui/Library/EmptyLibraryContainer.h"
#include "Gui/Directories/DirectoryWidgetContainer.h"

#include "Gui/Plugins/PlayerPluginHandler.h"
#include "Gui/Plugins/PlaylistChooser/GUI_PlaylistChooser.h"
#include "Gui/Plugins/AudioConverter/GUI_AudioConverter.h"
#include "Gui/Plugins/Bookmarks/GUI_Bookmarks.h"
#include "Gui/Plugins/Broadcasting/GUI_Broadcast.h"
#include "Gui/Plugins/Engine/GUI_LevelPainter.h"
#include "Gui/Plugins/Engine/GUI_Spectrum.h"
#include "Gui/Plugins/Engine/GUI_Equalizer.h"
#include "Gui/Plugins/Engine/GUI_Speed.h"
#include "Gui/Plugins/Engine/GUI_Crossfader.h"
#include "Gui/Plugins/Stream/GUI_Stream.h"
#include "Gui/Plugins/Stream/GUI_Podcasts.h"

#include "Gui/Preferences/Broadcast/GUI_BroadcastSetup.h"
#include "Gui/Preferences/Covers/GUI_Covers.h"
#include "Gui/Preferences/Engine/GUI_EnginePreferences.h"
#include "Gui/Preferences/UiPreferences/GUI_UiPreferences.h"
#include "Gui/Preferences/Language/GUI_LanguageChooser.h"
#include "Gui/Preferences/LastFM/GUI_LastFM.h"
#include "Gui/Preferences/Library/GUI_LibraryPreferences.h"
#include "Gui/Preferences/Notifications/GUI_Notifications.h"
#include "Gui/Preferences/Player/GUI_PlayerPreferences.h"
#include "Gui/Preferences/Playlist/GUI_PlaylistPreferences.h"
#include "Gui/Preferences/PreferenceDialog/GUI_PreferenceDialog.h"
#include "Gui/Preferences/Proxy/GUI_Proxy.h"
#include "Gui/Preferences/RemoteControl/GUI_RemoteControl.h"
#include "Gui/Preferences/Search/GUI_SearchPreferences.h"
#include "Gui/Preferences/Shortcuts/GUI_Shortcuts.h"
#include "Gui/Preferences/Streams/GUI_StreamPreferences.h"
#include "Gui/Preferences/StreamRecorder/GUI_StreamRecorder.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/Proxy.h"
#include "Utils/Macros.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Database/Connector.h"
#include "Database/Settings.h"

#include <QIcon>
#include <QTime>
#include <QDateTime>
#include <QSessionManager>

class MeasureApp
{
	QTime*		m_t;
	QString		m_component;
	int			m_start;

public:
	MeasureApp(const QString& component, QTime* t) :
		m_t(t),
		m_component(component)
	{
		m_start = m_t->elapsed();
		sp_log(Log::Debug, this) << "Init " << m_component << ": " << m_start << "ms";
	}

	~MeasureApp()
	{
		int end = m_t->elapsed();
		sp_log(Log::Debug, this) << "Init " << m_component << " finished: " << end << "ms (" << end - m_start << "ms)";
	}
};

#define measure(c) MeasureApp mt(c, m->timer); Q_UNUSED(mt);

struct Application::Private
{
	QTime*				timer=nullptr;
	GUI_Player*			player=nullptr;

	RemoteControl*		remote_control=nullptr;
	Playlist::Handler*	plh=nullptr;
	DB::Connector*		db=nullptr;
	InstanceThread*		instance_thread=nullptr;
	MetaTypeRegistry*	metatype_registry=nullptr;
	Session*			session=nullptr;

	bool				was_shut_down;

	Private(Application* app)
	{
		metatype_registry = new MetaTypeRegistry();
		qRegisterMetaType<uint64_t>("uint64_t");

		/* Tell the settings manager which settings are necessary */
		db = DB::Connector::instance();
		db->settings_connector()->load_settings();

		session = new Session(app);

		Gui::Icons::set_standard_theme(QIcon::themeName());
		Gui::Icons::force_standard_icons(GetSetting(Set::Icon_ForceInDarkTheme));

		if( !Settings::instance()->check_settings() )
		{
			sp_log(Log::Error, this) << "Cannot initialize settings";
			return;
		}

		Settings::instance()->apply_fixes();

		Q_INIT_RESOURCE(Icons);

#ifdef Q_OS_WIN
		Q_INIT_RESOURCE(IconsWindows);
#endif
		timer = new QTime();
		plh = Playlist::Handler::instance();
		was_shut_down = false;
	}

	~Private()
	{
		if(timer){
			delete timer; timer = nullptr;
		}

		if(instance_thread)
		{
			instance_thread->stop();
			while(instance_thread->isRunning()){
				Util::sleep_ms(100);
			}

			instance_thread = nullptr;
		}

		plh = nullptr;

		if(player){
			delete player;
			player=nullptr;
		}

		if(db){
			db->settings_connector()->store_settings();
			db->close_db();

			db = nullptr;

			QStringList connection_names = QSqlDatabase::connectionNames();
			for(const QString& connection_name : connection_names)
			{
				QSqlDatabase database = QSqlDatabase::database(connection_name);
				if(database.isOpen()){
					database.close();
				}
			}
		}

		if(metatype_registry)
		{
			delete metatype_registry; metatype_registry = nullptr;
		}
	}
};

#ifdef Q_OS_WIN
void global_key_handler()
{
	if(!RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_MEDIA_PLAY_PAUSE)){
		return false;
	}

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		if (msg.message == WM_HOTKEY)
		{
			UINT modifiers = msg.lParam;
			UINT key = msg.wParam;
		}
	}
}
#endif

Application::Application(int & argc, char ** argv) :
	QApplication(argc, argv)
{
	m = Pimpl::make<Private>(this);
	m->timer->start();

	this->setQuitOnLastWindowClosed(false);
}

Application::~Application()
{
	if(!m->was_shut_down){
		shutdown();
	}
}

bool Application::init(const QStringList& files_to_play)
{
	{
		measure("Settings")

		QString version = QString(SAYONARA_VERSION);
		SetSetting(Set::Player_Version, version);
	}

	{
		measure("Theme")
		Gui::Icons::change_theme();
	}

	{
		measure("Proxy")
		Proxy::instance()->init();
	}

	init_engine();
	init_player();

#ifdef WITH_DBUS
	{
		measure("DBUS")
		new DBusHandler(m->player, this);
	}
#endif

	{
		ListenSetting(Set::Remote_Active, Application::remote_control_activated);
	}

	if(GetSetting(Set::Notification_Show))
	{
		NotificationHandler::instance()->notify("Sayonara Player",
												Lang::get(Lang::Version) + " " + SAYONARA_VERSION,
												Util::share_path("logo.png"));
	}

	init_libraries();
	init_plugins();
	init_preferences();

	init_playlist(files_to_play);
	init_single_instance_thread();
	sp_log(Log::Debug, this) << "Initialized: " << m->timer->elapsed() << "ms";
	delete m->timer; m->timer=nullptr;

	//connect(this, &Application::commitDataRequest, this, &Application::session_end_requested);

	return true;
}

void Application::init_player()
{
	measure("Player")

	m->player = new GUI_Player();
	Gui::Util::set_main_window(m->player);

	connect(m->player, &GUI_Player::sig_player_closed, this, &QCoreApplication::quit);
}


void Application::init_playlist(const QStringList& files_to_play)
{
	if(!files_to_play.isEmpty())
	{
		QString playlist_name = m->plh->request_new_playlist_name();
		m->plh->create_playlist(files_to_play, playlist_name);
	}
}


void Application::init_preferences()
{
	measure("Preferences")

	auto* preferences = new GUI_PreferenceDialog(m->player);

	preferences->register_preference_dialog(new GUI_PlayerPreferences("application"));
	preferences->register_preference_dialog(new GUI_LanguageChooser("language"));
	preferences->register_preference_dialog(new GUI_UiPreferences("user-interface"));
	preferences->register_preference_dialog(new GUI_Shortcuts("shortcuts"));

	preferences->register_preference_dialog(new GUI_PlaylistPreferences("playlist"));
	preferences->register_preference_dialog(new GUI_LibraryPreferences("library"));
	preferences->register_preference_dialog(new GUI_Covers("covers"));
	preferences->register_preference_dialog(new GUI_EnginePreferences("engine"));
	preferences->register_preference_dialog(new GUI_SearchPreferences("search"));

	preferences->register_preference_dialog(new GUI_Proxy("proxy"));
	preferences->register_preference_dialog(new GUI_StreamPreferences("streams"));
	preferences->register_preference_dialog(new GUI_StreamRecorder("streamrecorder"));
	preferences->register_preference_dialog(new GUI_BroadcastSetup("broadcast"));
	preferences->register_preference_dialog(new GUI_RemoteControl("remotecontrol"));

	preferences->register_preference_dialog(new GUI_Notifications("notifications"));
	preferences->register_preference_dialog(new GUI_LastFM("lastfm", new LastFM::Base()));

	m->player->register_preference_dialog(preferences->action());
}

void Application::init_libraries()
{
	measure("Libraries")

	auto* local_library_plugin_handler = new Library::LocalLibraryPluginHandler(this);
	auto* library_plugin_loader = Library::PluginHandler::instance();

	QList<Library::Container*> library_containers = local_library_plugin_handler->get_local_library_containers();
	auto* directory_container = new Library::DirectoryContainer(this);

	library_containers << static_cast<Library::Container*>(directory_container);

#ifdef Q_OS_WIN
	SC::LibraryContainer* soundcloud_container = new SC::LibraryContainer(this);
	SomaFM::LibraryContainer* somafm_container = new SomaFM::LibraryContainer(this);
	library_containers << static_cast<Library::ContainerInterface*>(soundcloud_container);
	library_containers << static_cast<Library::ContainerInterface*>(somafm_container);
#endif

	library_plugin_loader->init(new EmptyLibraryContainer(), library_containers);
}

void Application::init_engine()
{
	measure("Engine")
	Engine::Handler::instance()->init();
}

void Application::init_plugins()
{
	measure("Plugins")

	PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();

	pph->add_plugin(new GUI_LevelPainter());
	pph->add_plugin(new GUI_Spectrum());
	pph->add_plugin(new GUI_Equalizer());
	pph->add_plugin(new GUI_Stream());
	pph->add_plugin(new GUI_Podcasts());
	pph->add_plugin(new GUI_PlaylistChooser());
	pph->add_plugin(new GUI_AudioConverter());
	pph->add_plugin(new GUI_Bookmarks());
	pph->add_plugin(new GUI_Speed());
	pph->add_plugin(new GUI_Broadcast());
	pph->add_plugin(new GUI_Crossfader());

	sp_log(Log::Debug, this) << "Plugins finished: " << m->timer->elapsed() << "ms";
}

void Application::init_single_instance_thread()
{
	m->instance_thread = new InstanceThread(this);

	connect(m->instance_thread, &InstanceThread::sig_player_raise, m->player, &GUI_Player::raise);
	connect(m->instance_thread, &InstanceThread::sig_create_playlist, this, &Application::create_playlist);

	m->instance_thread->start();
}

void Application::session_end_requested(QSessionManager& manager)
{
	Q_UNUSED(manager)
	shutdown();

	if(m->db){
		m->db->settings_connector()->store_settings();
		m->db->close_db();
	}

	if(m->player){
		m->player->request_shutdown();
	};
}

void Application::shutdown()
{
	Engine::Handler::instance()->shutdown();
	Playlist::Handler::instance()->shutdown();
	PlayManager::instance()->shutdown();

	m->was_shut_down = true;
}

void Application::remote_control_activated()
{
	if(GetSetting(Set::Remote_Active) && !m->remote_control)
	{
		m->remote_control = new RemoteControl(this);
	}
}

void Application::create_playlist()
{
	auto* instance_thread =	static_cast<InstanceThread*>(sender());
	if(!instance_thread){
		return;
	}

	QStringList paths = instance_thread->paths();
	QString new_name = Playlist::Handler::instance()->request_new_playlist_name();

	Playlist::Handler::instance()->create_playlist(paths, new_name, true);
}
