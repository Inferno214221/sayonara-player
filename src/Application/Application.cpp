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
#include "Helper/Macros.h"
#include "Helper/Language.h"
#include "Helper/Settings/Settings.h"

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

#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"
#include "Interfaces/PlayerPlugin/PlayerPluginHandler.h"
#include "Interfaces/Notification/NotificationHandler.h"

#include "GUI/Helper/GUI_Helper.h"

#include "GUI/Player/GUI_Player.h"
#include "GUI/Library/LocalLibraryContainer.h"
#include "GUI/DirectoryWidget/DirectoryWidgetContainer.h"

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

#include "GUI/Preferences/Fonts/GUI_FontConfig.h"
#include "GUI/Preferences/Notifications/GUI_Notifications.h"
#include "GUI/Preferences/LastFM/GUI_LastFM.h"
#include "GUI/Preferences/Language/GUI_LanguageChooser.h"
#include "GUI/Preferences/Broadcast/GUI_BroadcastSetup.h"
#include "GUI/Preferences/PlaylistPreferences/GUI_PlaylistPreferences.h"
#include "GUI/Preferences/StreamRecorder/GUI_StreamRecorder.h"
#include "GUI/Preferences/RemoteControl/GUI_RemoteControl.h"
#include "GUI/Preferences/LibraryPreferences/GUI_LibraryPreferences.h"
#include "GUI/Preferences/Shortcuts/GUI_Shortcuts.h"
#include "GUI/Preferences/PlayerPreferences/GUI_PlayerPreferences.h"
#include "GUI/Preferences/PreferenceDialog/GUI_PreferenceDialog.h"
#include "GUI/Preferences/Covers/GUI_Covers.h"
#include "GUI/Preferences/Icons/GUI_IconPreferences.h"

#include "Helper/FileHelper.h"
#include "Helper/Helper.h"
#include "Helper/Logger/Logger.h"

#include "Database/DatabaseConnector.h"

#include <QTime>
#include <QTranslator>

struct Application::Private
{
	QTime*				timer=nullptr;
	Settings*			settings = nullptr;
	GUI_Player*			player=nullptr;

	PlaylistHandler*	plh=nullptr;
	DatabaseConnector*	db=nullptr;
	InstanceThread*		instance_thread=nullptr;

	Private()
	{
		timer = new QTime();
		settings = Settings::getInstance();
	}

	~Private()
	{
		if(timer){
			delete timer; timer = nullptr;
		}

		if(instance_thread){
			instance_thread->stop();
			while(instance_thread->isRunning()){
				Helper::sleep_ms(100);
			}
		}

		if(plh){
			plh->save_all_playlists();
		}

		if(player){
			delete player; player=nullptr;
		}

		if(db){
			db->store_settings();
			db->close_db();
		}
	}
};

static InstanceMessage instance_message=InstanceMessageNone;

#ifdef Q_OS_UNIX

	#include <csignal>

	void new_instance_handler(int sig_num)
	{
		switch(sig_num)
		{
			case SIGUSR1:
				instance_message = InstanceMessageWithFiles;
				break;
			case SIGUSR2:
				instance_message = InstanceMessageWithoutFiles;
				break;
			default:
				break;
		}
	}

#else
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
	_m = Pimpl::make<Private>();
	_m->timer->start();

	instance_message = InstanceMessageNone;

	this->setQuitOnLastWindowClosed(false);
}

/*void Application::check_for_crash()
{
	QString error_file = Helper::get_error_file();

	if(!QFile::exists(error_file)) return;

	QString info_text;
	QString mail;

	if(!Helper::File::read_file_into_str(error_file, mail)){
		mail = "";
		mail.prepend("mailto:luciocarreras@gmail.com?subject=Sayonara Crash&amp;body=Hi Lucio,\n\nhere is the trace for a Sayonara crash\n\n");
	}

	else{
		mail.prepend("mailto:luciocarreras@gmail.com?subject=Sayonara Crash&amp;body=Hi Lucio,\n\nhere is the trace for a Sayonara crash\n\n");
		mail.append("\n\nI hope this will not happen again...");
	}

	info_text = QString("Sayonara seems to have crashed the last time<br />") +
			"Please send " +
			Helper::create_link(error_file, false) +
			" in " + Helper::create_link(Helper::get_sayonara_path(), false) +
			" to " + Helper::create_link("luciocarreras@gmail.com", false, mail);

	GlobalMessage::getInstance()->info(info_text);

	QFile f(error_file);
	f.open(QIODevice::ReadOnly);
	if(!f.isOpen()){
		sp_log(Log::Warning) << "Cannot oopen " << error_file;
		return;
	}

	f.remove();
	f.close();

	return;
}
*/

bool Application::init(QTranslator* translator, const QStringList& files_to_play)
{
	LibraryPluginHandler* library_plugin_loader = LibraryPluginHandler::getInstance();
	_m->db = DatabaseConnector::getInstance();
	_m->plh = PlaylistHandler::getInstance();

	sp_log(Log::Debug, this) << "Init application: " << _m->timer->elapsed() << "ms";

	bool success = this->installTranslator(translator);
	if(!success){
		sp_log(Log::Warning) << "Cannot install translator";
	}

	//check_for_crash();

	QString version = QString(SAYONARA_VERSION);
	_m->settings->set(Set::Player_Version, version);

	sp_log(Log::Debug, this) << "Start player: " << _m->timer->elapsed() << "ms";
	_m->player = new GUI_Player(translator);
	GUI::set_main_window(_m->player);

	connect(_m->player, &GUI_Player::sig_player_closed, this, &QCoreApplication::quit);

	sp_log(Log::Debug, this) << "Init player: " << _m->timer->elapsed() << "ms";

#ifdef WITH_DBUS
	DBusHandler* dbus	= new DBusHandler(_m->player, this);
	Q_UNUSED(dbus)

#endif

	RemoteControl* rmc = new RemoteControl(this);
	Q_UNUSED(rmc)

	if(_m->settings->get(Set::Notification_Show)){
		NotificationHandler::getInstance()->notify("Sayonara Player",
												   Lang::get(Lang::Version) + " " + SAYONARA_VERSION,
												   Helper::share_path("logo.png"));
	}

	sp_log(Log::Debug, this) << "Init plugins: " << _m->timer->elapsed() << "ms";
	PlayerPluginHandler* pph = new PlayerPluginHandler(this);

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

	sp_log(Log::Debug, this) << "Plugins finsihed: " << _m->timer->elapsed() << "ms";

	QList<LibraryContainerInterface*> library_containers;
	DirectoryLibraryContainer* directory_container = new DirectoryLibraryContainer(this);


	library_containers << static_cast<LibraryContainerInterface*>(directory_container);

#ifdef Q_OS_WIN
	SoundcloudLibraryContainer* soundcloud_container = new SoundcloudLibraryContainer(this);
	SomaFMLibraryContainer* somafm_container = new SomaFMLibraryContainer(this);
	library_containers << static_cast<LibraryContainerInterface*>(soundcloud_container);
	library_containers << static_cast<LibraryContainerInterface*>(somafm_container);
#endif
	library_plugin_loader->init(library_containers);

	sp_log(Log::Debug, this) << "Libraries loaded: " << _m->timer->elapsed() << "ms";

	GUI_PreferenceDialog* preferences = new GUI_PreferenceDialog(_m->player);

	_m->player->register_preference_dialog(preferences);

	preferences->register_preference_dialog(new GUI_LanguageChooser());
	preferences->register_preference_dialog(new GUI_FontConfig());
	preferences->register_preference_dialog(new GUI_PlayerPreferences());
	preferences->register_preference_dialog(new GUI_PlaylistPreferences());
	preferences->register_preference_dialog(new GUI_LibraryPreferences());
	preferences->register_preference_dialog(new GUI_Covers());
	preferences->register_preference_dialog(new GUI_StreamRecorder());
	preferences->register_preference_dialog(new GUI_BroadcastSetup());
	preferences->register_preference_dialog(new GUI_Shortcuts());
	preferences->register_preference_dialog(new GUI_Notifications());
	preferences->register_preference_dialog(new GUI_RemoteControl());
	preferences->register_preference_dialog(new GUI_LastFM());
	preferences->register_preference_dialog(new GUI_IconPreferences());

	EngineHandler::getInstance()->init();

	sp_log(Log::Debug, this) << "Preference dialogs loaded: " << _m->timer->elapsed() << "ms";

	_m->player->register_player_plugin_handler(pph);
	_m->player->ui_loaded();

	if(files_to_play.size() > 0) {
		QString playlist_name = _m->plh->request_new_playlist_name();
		_m->plh->create_playlist(files_to_play, playlist_name);
	}

	init_single_instance_thread();

	sp_log(Log::Debug, this) << "Time to start: " << _m->timer->elapsed() << "ms";
	delete _m->timer; _m->timer=nullptr;

	return true;
}


Application::~Application() {}


void Application::init_single_instance_thread()
{
#ifdef Q_OS_UNIX
	signal(SIGUSR1, new_instance_handler);
	signal(SIGUSR2, new_instance_handler);
#endif

	_m->instance_thread = new InstanceThread(&instance_message, this);

	connect(_m->instance_thread, &InstanceThread::sig_player_raise, _m->player, &GUI_Player::raise);
	connect(_m->instance_thread, SIGNAL(sig_create_playlist(const QStringList&, const QString&, bool)),
			_m->plh, SLOT(create_playlist(const QStringList&, const QString&, bool)));

	_m->instance_thread->start();
}
