/* application.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

// need this here because of SAYONARA ENV variables
#include "Utils/Macros.h"

#include "Application/Application.h"
#include "Application/InstanceThread.h"
#include "Application/MetaTypeRegistry.h"
#include "Application/LocalLibraryWatcher.h"

#ifdef SAYONARA_WITH_DBUS

#include "DBus/DBusHandler.h"

#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include "3rdParty/SomaFM/ui/SomaFMLibraryContainer.h"
#include "3rdParty/Soundcloud/ui/GUI_SoundcloudLibrary.h"
#endif

#include "Components/Bookmarks/Bookmarks.h"
#include "Components/Converter/ConverterFactory.h"
#include "Components/DynamicPlayback/DynamicPlaybackHandler.h"
#include "Components/DynamicPlayback/DynamicPlaybackCheckerImpl.h"
#include "Components/Equalizer/Equalizer.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistLoader.h"
#include "Components/Playlist/PlaylistChooser.h"
#include "Components/Playlist/LibraryPlaylistInteractorImpl.h"
#include "Components/PlayManager/PlayManagerImpl.h"
#include "Components/RemoteControl/RemoteControl.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/Streaming/LastFM/LastFM.h"
#include "Components/Streaming/SomaFM/SomaFMLibrary.h"
#include "Components/Session/Session.h"
#include "Components/Shutdown/Shutdown.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Interfaces/Notification/NotificationHandler.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Icons.h"

#include "Gui/Player/GUI_Player.h"
#include "Gui/Library/LocalLibraryContainer.h"
#include "Gui/Library/EmptyLibraryContainer.h"
#include "Gui/Soundcloud/SoundcloudLibraryContainer.h"
#include "Gui/SomaFM/SomaFMLibraryContainer.h"

#include "Gui/History/HistoryContainer.h"

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
#include "Gui/Plugins/Engine/GUI_SpectrogramPainter.h"
#include "Gui/Plugins/Stream/GUI_Stream.h"
#include "Gui/Plugins/Stream/GUI_Podcasts.h"

#include "Gui/Preferences/Broadcast/GUI_BroadcastPreferences.h"
#include "Gui/Preferences/Covers/GUI_CoverPreferences.h"
#include "Gui/Preferences/Engine/GUI_EnginePreferences.h"
#include "Gui/Preferences/UiPreferences/GUI_UiPreferences.h"
#include "Gui/Preferences/Language/GUI_LanguagePreferences.h"
#include "Gui/Preferences/LastFM/GUI_LastFmPreferences.h"
#include "Gui/Preferences/Library/GUI_LibraryPreferences.h"
#include "Gui/Preferences/Notifications/GUI_NotificationPreferences.h"
#include "Gui/Preferences/Player/GUI_PlayerPreferences.h"
#include "Gui/Preferences/Playlist/GUI_PlaylistPreferences.h"
#include "Gui/Preferences/PreferenceDialog/GUI_PreferenceDialog.h"
#include "Gui/Preferences/Proxy/GUI_ProxyPreferences.h"
#include "Gui/Preferences/RemoteControl/GUI_RemoteControlPreferences.h"
#include "Gui/Preferences/Search/GUI_SearchPreferences.h"
#include "Gui/Preferences/Shortcuts/GUI_ShortcutPreferences.h"
#include "Gui/Preferences/Streams/GUI_StreamPreferences.h"
#include "Gui/Preferences/StreamRecorder/GUI_StreamRecorderPreferences.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/Proxy.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/StandardPaths.h"

#include "Database/Connector.h"
#include "Database/Settings.h"

#include <QIcon>
#include <QTime>
#include <QSessionManager>

class MeasureApp
{
		QTime* mTime;
		QString mComponent;
		int mStart;

	public:
		MeasureApp(const QString& component, QTime* t) :
			mTime(t),
			mComponent(component)
		{
			mStart = mTime->elapsed();
			spLog(Log::Debug, this) << "Init " << mComponent << ": " << mStart << "ms";
		}

		~MeasureApp()
		{
			int end = mTime->elapsed();
			spLog(Log::Debug, this) << "Init " << mComponent << " finished: " << end << "ms (" << end - mStart
			                        << "ms)";
		}
};

#define measure(c) MeasureApp mt(c, m->timer); Q_UNUSED(mt);

struct Application::Private
{
	MetaTypeRegistry* metatypeRegistry;
	DB::Connector* db;
	PlayManager* playManager;
	Engine::Handler* engine;
	Session::Manager* sessionManager;
	Playlist::Handler* playlistHandler;
	LibraryPlaylistInteractor* libraryPlaylistInteractor;
	Library::Manager* libraryManager;
	DynamicPlaybackChecker* dynamicPlaybackChecker;
	QTime* timer;

	GUI_Player* player = nullptr;
	DBusHandler* dbusHandler = nullptr;
	DynamicPlayback::Handler* dynamicPlaybackHandler = nullptr;
	Library::LocalLibraryWatcher* localLibraryWatcher = nullptr;
	RemoteControl* remoteControl = nullptr;
	InstanceThread* instanceThread = nullptr;

	bool shutdownTriggered;

	Private(Application* app) :
		shutdownTriggered{false}
	{
		QApplication::setApplicationName("sayonara");
		Util::copyFromLegacyLocations();

		metatypeRegistry = new MetaTypeRegistry();

		/* Tell the settings manager which settings are necessary */
		db = DB::Connector::instance();
		db->settingsConnector()->loadSettings();

		auto* settings = Settings::instance();
		if(!settings->checkSettings())
		{
			spLog(Log::Error, this) << "Cannot initialize settings";
			return;
		}

		settings->applyFixes();

		playManager = new PlayManagerImpl(app);
		engine = new Engine::Handler(playManager);
		sessionManager = new Session::Manager(playManager);
		playlistHandler = new Playlist::Handler(playManager, std::make_shared<Playlist::LoaderImpl>());
		libraryPlaylistInteractor = new LibraryPlaylistInteractorImpl(playlistHandler, playManager);
		libraryManager = new Library::Manager(libraryPlaylistInteractor);
		dynamicPlaybackChecker = new DynamicPlaybackCheckerImpl(libraryManager);

		Shutdown::instance()->registerPlaymanager(playManager);

		Gui::Icons::setSystemTheme(QIcon::themeName());
		Gui::Icons::forceStandardIcons(GetSetting(Set::Icon_ForceInDarkTheme));

		Private::initResources();

		timer = new QTime();
		timer->start();
	}

	~Private()
	{
		delete timer;
		delete dynamicPlaybackChecker;
		delete libraryManager;
		delete playlistHandler;
		delete sessionManager;
		delete engine;
		delete playManager;

		if(db)
		{
			db->settingsConnector()->storeSettings();
			db->closeDatabase();
			db = nullptr;
		}

		delete metatypeRegistry;
	}

	static void initResources()
	{
		Q_INIT_RESOURCE(Broadcasting);
		Q_INIT_RESOURCE(Database);
		Q_INIT_RESOURCE(Icons);
		Q_INIT_RESOURCE(Lyrics);
		Q_INIT_RESOURCE(Resources);

#ifdef Q_OS_WIN
		Q_INIT_RESOURCE(IconsWindows);
#endif
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

Application::Application(int& argc, char** argv) :
	QApplication(argc, argv)
{
	m = Pimpl::make<Private>(this);

	QApplication::setQuitOnLastWindowClosed(false);
	QApplication::setApplicationName("sayonara");
}

Application::~Application()
{
	if(!m->shutdownTriggered)
	{
		shutdown();
	}

	if(m->instanceThread)
	{
		m->instanceThread->stop();
		while(m->instanceThread->isRunning())
		{
			Util::sleepMs(100);
		}

		m->instanceThread = nullptr;
	}

	if(m->remoteControl)
	{
		delete m->remoteControl;
	}

	delete m->localLibraryWatcher;
	delete m->dynamicPlaybackHandler;
	if(m->dbusHandler)
	{
		delete m->dbusHandler;
	}

	delete m->player;
}

void Application::shutdown()
{
	PlayerPlugin::Handler::instance()->shutdown();
	Library::PluginHandler::instance()->shutdown();

	m->playlistHandler->shutdown();
	m->engine->shutdown();
	m->playManager->shutdown();

	m->shutdownTriggered = true;
}

bool Application::init(const QStringList& filesToPlay, bool forceShow)
{
	{
		measure("Settings")
		SetSetting(Set::Player_Version, QString(SAYONARA_VERSION));
	}

	{
		measure("Theme")
		Gui::Icons::changeTheme();
	}

	{
		measure("Proxy")
		Proxy::init();
	}

	initPlayer(forceShow);

#ifdef SAYONARA_WITH_DBUS
	{
		measure("DBUS")
		m->dbusHandler = new DBusHandler(m->player, m->playManager, m->playlistHandler, this);
	}
#endif

	{
		ListenSetting(Set::Remote_Active, Application::remoteControlActivated);
	}

	if(GetSetting(Set::Notification_Show))
	{
		NotificationHandler::instance()->notify("Sayonara Player",
		                                        Lang::get(Lang::Version) + " " + SAYONARA_VERSION,
		                                        QString(":/Icons/logo.png")
		);
	}

	m->dynamicPlaybackHandler = new DynamicPlayback::Handler(m->playManager, m->playlistHandler, this);

	initLibraries();
	initPlugins();
	initPreferences();

	initPlaylist(filesToPlay);
	initSingleInstanceThread();
	spLog(Log::Debug, this) << "Initialized: " << m->timer->elapsed() << "ms";

	ListenSetting(SetNoDB::Player_MetaStyle, Application::skinChanged);

	if(!GetSetting(Set::Player_StartInTray))
	{
		m->player->show();
	}

	else
	{
		m->player->hide();
	}

	return true;
}

void Application::initPlayer(bool force_show)
{
	measure("Player")

	if(force_show)
	{
		SetSetting(Set::Player_StartInTray, false);
	}

	m->player = new GUI_Player(m->playManager,
	                           m->playlistHandler,
	                           m->engine,
	                           m->dynamicPlaybackChecker);

	Gui::Util::setMainWindow(m->player);

	connect(m->player, &GUI_Player::sigClosed, this, &QCoreApplication::quit);
}

void Application::initPlaylist(const QStringList& filesToPlay)
{
	if(!filesToPlay.isEmpty())
	{
		m->playlistHandler->createCommandLinePlaylist(filesToPlay);
	}
}

void Application::initPreferences()
{
	measure("Preferences")

	auto* preferences = new GUI_PreferenceDialog(m->player);

	preferences->registerPreferenceDialog(new GUI_PlayerPreferences("application"));
	preferences->registerPreferenceDialog(new GUI_LanguagePreferences("language"));
	preferences->registerPreferenceDialog(new GUI_UiPreferences("user-interface"));
	preferences->registerPreferenceDialog(new GUI_ShortcutPreferences("shortcuts"));

	preferences->registerPreferenceDialog(new GUI_PlaylistPreferences("playlist"));
	preferences->registerPreferenceDialog(new GUI_LibraryPreferences(m->libraryManager, "library"));
	preferences->registerPreferenceDialog(new GUI_CoverPreferences("covers"));
	preferences->registerPreferenceDialog(new GUI_EnginePreferences("engine"));
	preferences->registerPreferenceDialog(new GUI_SearchPreferences("search"));

	preferences->registerPreferenceDialog(new GUI_ProxyPreferences("proxy"));
	preferences->registerPreferenceDialog(new GUI_StreamPreferences("streams"));
	preferences->registerPreferenceDialog(new GUI_StreamRecorderPreferences("streamrecorder"));
	preferences->registerPreferenceDialog(new GUI_BroadcastPreferences("broadcast"));
	preferences->registerPreferenceDialog(new GUI_RemoteControlPreferences("remotecontrol"));

	preferences->registerPreferenceDialog(new GUI_NotificationPreferences("notifications"));
	preferences->registerPreferenceDialog(new GUI_LastFmPreferences("lastfm", new LastFM::Base(m->playManager)));

	m->player->registerPreferenceDialog(preferences->action());
}

void Application::initLibraries()
{
	measure("Libraries")

	m->localLibraryWatcher = new Library::LocalLibraryWatcher(m->libraryManager, this);

	auto libraryContainers = m->localLibraryWatcher->getLocalLibraryContainers();

	auto* soundcloudContainer = new SC::LibraryContainer(m->libraryPlaylistInteractor, this);
	auto* somafmContainer = new SomaFM::LibraryContainer(new SomaFM::Library(m->playlistHandler, this), this);
	auto* historyContainer = new HistoryContainer(m->sessionManager, this);

	libraryContainers << static_cast<Library::AbstractContainer*>(somafmContainer);
	libraryContainers << static_cast<Library::AbstractContainer*>(soundcloudContainer);
	libraryContainers << static_cast<Library::AbstractContainer*>(historyContainer);

	Library::PluginHandler::instance()->init(libraryContainers, new EmptyLibraryContainer(m->libraryManager));
}

void Application::initPlugins()
{
	measure("Plugins")

	auto* playerPluginHandler = PlayerPlugin::Handler::instance();

	playerPluginHandler->addPlugin(new GUI_LevelPainter(m->engine, m->playManager));
	playerPluginHandler->addPlugin(new GUI_Spectrum(m->engine, m->playManager));
	playerPluginHandler->addPlugin(new GUI_Equalizer(new Equalizer(m->engine)));
	playerPluginHandler->addPlugin(new GUI_Stream(m->playlistHandler));
	playerPluginHandler->addPlugin(new GUI_Podcasts(m->playlistHandler));
	playerPluginHandler->addPlugin(new GUI_PlaylistChooser(new Playlist::Chooser(m->playlistHandler, this)));
	playerPluginHandler->addPlugin(new GUI_AudioConverter(new ConverterFactory(m->playlistHandler)));
	playerPluginHandler->addPlugin(new GUI_Bookmarks(new Bookmarks(m->playManager)));
	playerPluginHandler->addPlugin(new GUI_Speed());
	playerPluginHandler->addPlugin(new GUI_Broadcast(m->playManager, m->engine));
	playerPluginHandler->addPlugin(new GUI_Crossfader());
	playerPluginHandler->addPlugin(new GUI_SpectrogramPainter(m->playManager));

	spLog(Log::Debug, this) << "Plugins finished: " << m->timer->elapsed() << "ms";
}

void Application::initSingleInstanceThread()
{
	m->instanceThread = new InstanceThread(this);

	connect(m->instanceThread, &InstanceThread::sigPlayerRise, m->player, &GUI_Player::raise);
	connect(m->instanceThread, &InstanceThread::sigCreatePlaylist, this, &Application::createPlaylist);

	m->instanceThread->start();
}

void Application::sessionEndRequested(QSessionManager& manager)
{
	Q_UNUSED(manager)
	shutdown();

	if(m->db)
	{
		m->db->settingsConnector()->storeSettings();
		m->db->closeDatabase();
	}

	if(m->player)
	{
		m->player->requestShutdown();
	}
}

void Application::remoteControlActivated()
{
	if(GetSetting(Set::Remote_Active) && !m->remoteControl)
	{
		m->remoteControl = new RemoteControl(m->playlistHandler, m->playManager, this);
	}
}

void Application::createPlaylist()
{
	auto* instanceThread = dynamic_cast<InstanceThread*>(sender());
	if(!instanceThread)
	{
		return;
	}

	const QStringList paths = instanceThread->paths();
	m->playlistHandler->createPlaylist(paths);
}

void Application::skinChanged()
{
	Style::applyCurrentStyle(this);
}
