/* application.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "GUI/Utils/Icons.h"

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
#include "Components/StreamPlugins/LastFM/LastFM.h"

#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"
#include "Interfaces/PlayerPlugin/PlayerPluginHandler.h"
#include "Interfaces/Notification/NotificationHandler.h"

#include "GUI/Utils/GuiUtils.h"

#include "GUI/Player/GUI_Player.h"
#include "GUI/Library/LocalLibraryContainer.h"
#include "GUI/Directories/DirectoryWidgetContainer.h"

#include "GUI/Plugins/PlaylistChooser/GUI_PlaylistChooser.h"
#include "GUI/Plugins/Engine/AudioConverter/GUI_AudioConverter.h"
#include "GUI/Plugins/Engine/GUI_LevelPainter.h"
#include "GUI/Plugins/Engine/GUI_Spectrum.h"
#include "GUI/Plugins/Engine/Equalizer/GUI_Equalizer.h"
#include "GUI/Plugins/Engine/Speed/GUI_Speed.h"
#include "GUI/Plugins/Engine/Crossfader/GUI_Crossfader.h"
#include "GUI/Plugins/Stream/GUI_Stream.h"
#include "GUI/Plugins/Stream/GUI_Podcasts.h"
#include "GUI/Plugins/Bookmarks/GUI_Bookmarks.h"
#include "GUI/Plugins/Broadcasting/GUI_Broadcast.h"

#include "GUI/Preferences/Broadcast/GUI_BroadcastSetup.h"
#include "GUI/Preferences/Covers/GUI_Covers.h"
#include "GUI/Preferences/Engine/GUI_EnginePreferences.h"
#include "GUI/Preferences/UiPreferences/GUI_UiPreferences.h"
#include "GUI/Preferences/Language/GUI_LanguageChooser.h"
#include "GUI/Preferences/LastFM/GUI_LastFM.h"
#include "GUI/Preferences/Library/GUI_LibraryPreferences.h"
#include "GUI/Preferences/Notifications/GUI_Notifications.h"
#include "GUI/Preferences/Player/GUI_PlayerPreferences.h"
#include "GUI/Preferences/Playlist/GUI_PlaylistPreferences.h"
#include "GUI/Preferences/PreferenceDialog/GUI_PreferenceDialog.h"
#include "GUI/Preferences/Proxy/GUI_Proxy.h"
#include "GUI/Preferences/RemoteControl/GUI_RemoteControl.h"
#include "GUI/Preferences/Search/GUI_SearchPreferences.h"
#include "GUI/Preferences/Shortcuts/GUI_Shortcuts.h"
#include "GUI/Preferences/Streams/GUI_StreamPreferences.h"
#include "GUI/Preferences/StreamRecorder/GUI_StreamRecorder.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/Proxy.h"
#include "Utils/Macros.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingRegistry.h"

#include "Database/DatabaseConnector.h"
#include "Database/DatabaseSettings.h"

#include <QTime>
#include <QSessionManager>

struct Application::Private
{
	QTime*				timer=nullptr;
	GUI_Player*			player=nullptr;

	Playlist::Handler*	plh=nullptr;
	DB::Connector*		db=nullptr;
	InstanceThread*		instance_thread=nullptr;
	MetaTypeRegistry*	metatype_registry=nullptr;

	bool				settings_initialized;

	Private()
	{
		metatype_registry = new MetaTypeRegistry();
		qRegisterMetaType<uint64_t>("uint64_t");

		/* Tell the settings manager which settings are necessary */
		settings_initialized = SettingRegistry::init();

		db = DB::Connector::instance();
		db->settings_connector()->load_settings();

		Gui::Icons::set_standard_theme(QIcon::themeName());
		Gui::Icons::force_standard_icons(Settings::instance()->get<Set::Icon_ForceInDarkTheme>());

		if( !settings_initialized )
		{
			sp_log(Log::Error, this) << "Cannot initialize settings";
			return;
		}

		Q_INIT_RESOURCE(Icons);

#ifdef Q_OS_WIN
		Q_INIT_RESOURCE(IconsWindows);
#endif
		timer = new QTime();
		plh = Playlist::Handler::instance();
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
	m = Pimpl::make<Private>();
	m->timer->start();

	this->setQuitOnLastWindowClosed(false);
}

Application::~Application()
{
	Engine::Handler::instance()->shutdown();
	Playlist::Handler::instance()->shutdown();
}

bool Application::init(const QStringList& files_to_play)
{
	Settings* settings = Settings::instance();

	QString version = QString(SAYONARA_VERSION);
	settings->set<Set::Player_Version>(version);

	Gui::Icons::change_theme();
	Proxy::instance()->init();

	init_player();

#ifdef WITH_DBUS
	new DBusHandler(m->player, this);
#endif

	new RemoteControl(this);

	if(settings->get<Set::Notification_Show>())
	{
		NotificationHandler::instance()->notify("Sayonara Player",
												Lang::get(Lang::Version) + " " + SAYONARA_VERSION,
												Util::share_path("logo.png"));
	}

	Gui::Icons::change_theme();
	init_single_instance_thread();
	init_engine();
	init_libraries();
	init_plugins();
	init_preferences();

	init_playlist(files_to_play);
Gui::Icons::change_theme();
	sp_log(Log::Debug, this) << "Time to start: " << m->timer->elapsed() << "ms";
	delete m->timer; m->timer=nullptr;

	connect(this, &Application::commitDataRequest, this, &Application::session_end_requested);
Gui::Icons::change_theme();
	return true;
}

void Application::init_player()
{
	m->player = new GUI_Player();
	Gui::Util::set_main_window(m->player);

	connect(m->player, &GUI_Player::sig_player_closed, this, &QCoreApplication::quit);
	sp_log(Log::Debug, this) << "Init player: " << m->timer->elapsed() << "ms";
}


void Application::init_playlist(const QStringList& files_to_play)
{
	if(files_to_play.size() > 0) {
		QString playlist_name = m->plh->request_new_playlist_name();
		m->plh->create_playlist(files_to_play, playlist_name);
	}
}


void Application::init_preferences()
{
	PreferenceDialog* preferences = new GUI_PreferenceDialog(m->player);

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

	sp_log(Log::Debug, this) << "Preference dialogs loaded: " << m->timer->elapsed() << "ms";
}

void Application::init_libraries()
{
	Library::PluginHandler* library_plugin_loader = Library::PluginHandler::instance();

	QList<Library::Container*> library_containers;
	Library::DirectoryContainer* directory_container = new Library::DirectoryContainer(this);

	library_containers << static_cast<Library::Container*>(directory_container);

#ifdef Q_OS_WIN
	SC::LibraryContainer* soundcloud_container = new SC::LibraryContainer(this);
	SomaFM::LibraryContainer* somafm_container = new SomaFM::LibraryContainer(this);
	library_containers << static_cast<Library::ContainerInterface*>(soundcloud_container);
	library_containers << static_cast<Library::ContainerInterface*>(somafm_container);
#endif
	sp_log(Log::Debug, this) << "Libraries init: " << m->timer->elapsed() << "ms";
	library_plugin_loader->init(library_containers);

	sp_log(Log::Debug, this) << "Libraries loaded: " << m->timer->elapsed() << "ms";
}

void Application::init_engine()
{
	Engine::Handler::instance()->init();
}

void Application::init_plugins()
{
	sp_log(Log::Debug, this) << "Init plugins... " << m->timer->elapsed() << "ms";
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

	sp_log(Log::Debug, this) << "Plugins finsihed: " << m->timer->elapsed() << "ms";
}


bool Application::settings_initialized() const
{
	return m->settings_initialized;
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

	if(m->db){
		m->db->settings_connector()->store_settings();
		m->db->close_db();
	}

	if(m->player){
		m->player->request_shutdown();
	};
}

void Application::create_playlist()
{
	InstanceThread* t =	static_cast<InstanceThread*>(sender());
	if(!t){
		return;
	}

	QStringList paths = t->paths();
	QString new_name = Playlist::Handler::instance()->request_new_playlist_name();

	Playlist::Handler::instance()->create_playlist(paths, new_name, true);
}
