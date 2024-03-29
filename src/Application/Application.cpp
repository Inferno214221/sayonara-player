/* application.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Application/LocalLibraryObserver.h"
#include "Application/MetaTypeRegistry.h"
#include "Components/Bookmarks/Bookmarks.h"
#include "Components/Converter/ConverterFactory.h"
#include "Components/DynamicPlayback/DynamicPlaybackChecker.h"
#include "Components/DynamicPlayback/DynamicPlaybackHandler.h"
#include "Components/DynamicPlayback/LfmSimilarArtistFetcher.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/Equalizer/Equalizer.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/Notification/NotificationHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistChooser.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/PlaylistLibraryInteractor.h"
#include "Components/Playlist/PlaylistLoader.h"
#include "Components/RemoteControl/RemoteControl.h"
#include "Components/Session/Session.h"
#include "Components/Shutdown/Shutdown.h"
#include "Components/SmartPlaylists/SmartPlaylistManager.h"
#include "Components/Streaming/LastFM/LastFM.h"
#include "Components/Streaming/SomaFM/SomaFMLibrary.h"
#include "Components/Streaming/Streams/PodcastHandler.h"
#include "Components/Streaming/Streams/StreamHandler.h"
#include "DBus/DBusMPRIS.h"
#include "DBus/DBusMediaKeysInterfaceGnome.h"
#include "DBus/DBusMediaKeysInterfaceMate.h"
#include "DBus/DBusNotifications.h"
#include "DBus/DBusSessionManager.h"
#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Gui/History/HistoryContainer.h"
#include "Gui/Library/EmptyLibraryContainer.h"
#include "Gui/Library/LocalLibraryContainer.h"
#include "Gui/Lyrics/LyricsLibraryContainer.h"
#include "Gui/Player/GUI_Player.h"
#include "Gui/Plugins/AudioConverter/GUI_AudioConverter.h"
#include "Gui/Plugins/Bookmarks/GUI_Bookmarks.h"
#include "Gui/Plugins/Broadcasting/GUI_Broadcast.h"
#include "Gui/Plugins/Engine/GUI_Crossfader.h"
#include "Gui/Plugins/Engine/GUI_Equalizer.h"
#include "Gui/Plugins/Engine/GUI_LevelPainter.h"
#include "Gui/Plugins/Engine/GUI_SpectrogramPainter.h"
#include "Gui/Plugins/Engine/GUI_Spectrum.h"
#include "Gui/Plugins/Engine/GUI_Speed.h"
#include "Gui/Plugins/PlayerPluginHandler.h"
#include "Gui/Plugins/PlaylistChooser/GUI_PlaylistChooser.h"
#include "Gui/Plugins/SmartPlaylists/GuiSmartPlaylists.h"
#include "Gui/Plugins/Stream/GUI_Podcasts.h"
#include "Gui/Plugins/Stream/GUI_Stream.h"
#include "Gui/Preferences/Broadcast/GUI_BroadcastPreferences.h"
#include "Gui/Preferences/Covers/GUI_CoverPreferences.h"
#include "Gui/Preferences/Engine/GUI_EnginePreferences.h"
#include "Gui/Preferences/FileExtensions/GUI_FileExtensionPreferences.h"
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
#include "Gui/Preferences/StreamRecorder/GUI_StreamRecorderPreferences.h"
#include "Gui/Preferences/Streams/GUI_StreamPreferences.h"
#include "Gui/Preferences/UiPreferences/GUI_UiPreferences.h"
#include "Gui/Preferences/Other/OtherSettings.h"
#include "Gui/SomaFM/SomaFMLibraryContainer.h"
#include "Gui/Soundcloud/SoundcloudLibraryContainer.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"
#include "Utils/FileSystem.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MeasureApp.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"
#include "Utils/Tagging/TagWriter.h"
#include "Utils/Utils.h"
#include "Utils/WebAccess/Proxy.h"
#include "Utils/WebAccess/WebClientFactory.h"

#include <QIcon>
#include <QElapsedTimer>
#include <QSessionManager>

namespace
{
	void initializeSettings()
	{
		auto* db = DB::Connector::instance();
		db->settingsConnector()->loadSettings();

		auto* settings = Settings::instance();
		if(!settings->checkSettings())
		{
			spLog(Log::Error, "SettingInitializer") << "Cannot initialize settings";
			throw std::runtime_error {"Cannot initialize settings"};
		}

		settings->applyFixes();
	}
}

static void initResources() // must be static for some reason
{
	Q_INIT_RESOURCE(Broadcasting);
	Q_INIT_RESOURCE(Database);
	Q_INIT_RESOURCE(Icons);
	Q_INIT_RESOURCE(Lyrics);
	Q_INIT_RESOURCE(Resources);
}

struct Application::Private
{
	Util::FileSystemPtr fileSystem;
	Tagging::TagWriterPtr tagWriter;
	NotificationHandler* notificationHandler;
	PlayManager* playManager;
	Engine::Handler* engine;
	Session::Manager* sessionManager;
	Playlist::Handler* playlistHandler;
	LibraryPlaylistInteractor* libraryPlaylistInteractor;
	Library::Manager* libraryManager;
	Library::PluginHandler* libraryPluginHandler;
	Playlist::LibraryInteractor* playlistLibraryInteractor;
	DynamicPlaybackChecker* dynamicPlaybackChecker;
	DynamicPlayback::Handler* dynamicPlaybackHandler;
	SmartPlaylistManager* smartPlaylistManager;
	Shutdown* shutdown;
	QElapsedTimer* timer;

	GUI_Player* player {nullptr};
	Dbus::SessionManager* dbusSessionManager {nullptr};
	Library::LocalLibraryObserver* localLibraryWatcher {nullptr};
	RemoteControl* remoteControl {nullptr};
	InstanceThread* instanceThread {nullptr};

	bool shutdownTriggered {false};

	explicit Private(Application* app) :
		fileSystem {Util::FileSystem::create()},
		tagWriter(Tagging::TagWriter::create()),
		notificationHandler {NotificationHandler::create(app)},
		playManager {PlayManager::create(notificationHandler, app)},
		engine {new Engine::Handler(fileSystem, tagWriter, playManager)},
		sessionManager {new Session::Manager(playManager)},
		playlistHandler {new Playlist::Handler(playManager, std::make_shared<Playlist::LoaderImpl>(), fileSystem)},
		libraryPlaylistInteractor {LibraryPlaylistInteractor::create(playlistHandler, playlistHandler, playManager)},
		libraryManager {Library::Manager::create(libraryPlaylistInteractor)},
		libraryPluginHandler {Library::PluginHandler::create()},
		playlistLibraryInteractor {new Playlist::LibraryInteractor(libraryManager)},
		dynamicPlaybackChecker {DynamicPlaybackChecker::create(libraryManager)},
		dynamicPlaybackHandler {
			new DynamicPlayback::Handler(playManager, playlistHandler,
			                             std::make_shared<DynamicPlayback::LfmSimilarArtistFetcherFactory>(),
			                             fileSystem, playManager)
		},
		smartPlaylistManager {new SmartPlaylistManager(playlistHandler, fileSystem)},
		shutdown {Shutdown::create(playManager, notificationHandler)},
		timer {Util::startMeasure()}
	{
		Gui::Icons::setDefaultSystemTheme(QIcon::themeName());
		initResources();
	}

	~Private()
	{
		delete timer;
		delete shutdown;
		delete smartPlaylistManager;
		delete dynamicPlaybackHandler;
		delete dynamicPlaybackChecker;
		delete playlistLibraryInteractor;
		delete libraryPluginHandler;
		delete libraryManager;
		delete libraryPlaylistInteractor;
		delete playlistHandler;
		delete sessionManager;
		delete engine;
		delete playManager;
		delete notificationHandler;

		auto* db = DB::Connector::instance();
		db->settingsConnector()->storeSettings();
		db->closeDatabase();
	}
};

Application::Application(int& argc, char** argv) :
	QApplication(argc, argv)
{
	Util::initXdgPaths(QStringLiteral("sayonara"));
	Util::registerMetaTypes();
	initializeSettings();

	m = Pimpl::make<Private>(this);

	QApplication::setQuitOnLastWindowClosed(false);
	QApplication::setApplicationName(QStringLiteral("com.sayonara-player.Sayonara"));
	QApplication::setApplicationVersion(GetSetting(Set::Player_Version));
	QApplication::setWindowIcon(Gui::Icons::icon(Gui::Icons::Logo));
	QApplication::setDesktopFileName(QStringLiteral("com.sayonara-player.Sayonara.desktop"));
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
			Util::sleepMs(100); // NOLINT(readability-magic-numbers)
		}

		m->instanceThread = nullptr;
	}

	delete m->remoteControl;
	delete m->localLibraryWatcher;
	delete m->player;
}

void Application::shutdown()
{
	PlayerPlugin::Handler::instance()->shutdown();

	m->playlistHandler->shutdown();
	m->engine->shutdown();
	m->playManager->shutdown();

	m->shutdownTriggered = true;
}

bool Application::init(const QStringList& filesToPlay, bool forceShow)
{
	SetSetting(Set::Player_Version, QString(SAYONARA_VERSION));

	Util::measure("Theme", m->timer, []() {
		Gui::Icons::changeTheme();
	});

	Proxy::init();

	initPlayer(forceShow);
	initDbusServices();

	ListenSetting(Set::Remote_Active, Application::remoteControlActivated);

	if(GetSetting(Set::Notification_Show))
	{
		m->notificationHandler->notify("Sayonara Player",
		                               Lang::get(Lang::Version) + " " + SAYONARA_VERSION,
		                               QString(":/Icons/logo.png")
		);
	}

	initLibraries();
	initPlugins();
	initPreferences();

	initPlaylist(filesToPlay);
	initSingleInstanceThread();
	spLog(Log::Debug, this) << "Initialized: " << m->timer->elapsed() << "ms";

	ListenSetting(Set::Player_Style, Application::skinChanged);

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

void Application::initPlayer(const bool forceShow)
{
	[[maybe_unused]] auto measureApp = Util::MeasureApp("Player", m->timer);

	if(forceShow)
	{
		SetSetting(Set::Player_StartInTray, false);
	}

	m->player = new GUI_Player(m->playManager,
	                           m->playlistHandler,
	                           m->libraryPluginHandler,
	                           m->engine,
	                           m->shutdown,
	                           m->notificationHandler,
	                           m->dynamicPlaybackChecker,
	                           m->libraryManager,
	                           nullptr);

	m->player->setWindowIcon(Gui::Icons::icon(Gui::Icons::Logo));

	connect(m->player, &GUI_Player::sigClosed, this, &QCoreApplication::quit);
}

void Application::initDbusServices()
{
	[[maybe_unused]] auto measureApp = Util::MeasureApp("Dbus services", m->timer);

	new Dbus::Notifications(m->notificationHandler);
	new Dbus::Mpris::MediaPlayer2(m->player, m->playManager, m->playlistHandler);
	new Dbus::MediaKeysInterfaceGnome(m->playManager);
	new Dbus::MediaKeysInterfaceMate(m->playManager);
	m->dbusSessionManager = new Dbus::SessionManager(m->playManager);
}

void Application::initPlaylist(const QStringList& filesToPlay)
{
	if(!filesToPlay.isEmpty())
	{
		m->playlistHandler->createCommandLinePlaylist(filesToPlay, nullptr);
	}
}

void Application::initPreferences()
{
	[[maybe_unused]] auto measureApp = Util::MeasureApp("Preferences", m->timer);

	auto* preferences = new GUI_PreferenceDialog(m->player);

	const auto canInhibit = m->dbusSessionManager->canInhibit();
	preferences->registerPreferenceDialog(new GUI_PlayerPreferences("application", canInhibit));
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
	preferences->registerPreferenceDialog(new GUI_StreamRecorderPreferences("streamrecorder", m->fileSystem));
	preferences->registerPreferenceDialog(new GUI_BroadcastPreferences("broadcast"));
	preferences->registerPreferenceDialog(new GUI_RemoteControlPreferences("remotecontrol"));
	preferences->registerPreferenceDialog(new GUI_OtherSettings("other-settings"));

	auto* notificationPreferences = new GUI_NotificationPreferences("notifications", m->notificationHandler);
	preferences->registerPreferenceDialog(notificationPreferences);

	auto* lastFmPreferences =
		new GUI_LastFmPreferences("lastfm", new LastFM::Base(m->playManager, m->notificationHandler));
	preferences->registerPreferenceDialog(lastFmPreferences);

	preferences->registerPreferenceDialog(new GUI_FileExtensionPreferences("file-extensions"));

	m->player->registerPreferenceDialog(preferences->action());
}

void Application::initLibraries()
{
	[[maybe_unused]] auto measureApp = Util::MeasureApp("Libraries", m->timer);

	m->localLibraryWatcher = new Library::LocalLibraryObserver(m->libraryManager, m->libraryPluginHandler, this);

	auto libraryContainers = m->localLibraryWatcher->getLocalLibraryContainers();

	auto* soundcloudContainer = new SC::LibraryContainer(m->libraryPlaylistInteractor, m->libraryPluginHandler);
	auto* somafmContainer = new SomaFM::LibraryContainer(new SomaFM::Library(m->playlistHandler, this),
	                                                     m->libraryPluginHandler);
	auto* historyContainer = new HistoryContainer(m->libraryPlaylistInteractor,
	                                              m->sessionManager,
	                                              m->libraryPluginHandler);

	auto* lyricsContainer = new LyricsLibraryContainer(m->playManager, m->libraryPluginHandler);

	libraryContainers << static_cast<Library::LibraryContainer*>(somafmContainer);
	libraryContainers << static_cast<Library::LibraryContainer*>(soundcloudContainer);
	libraryContainers << static_cast<Library::LibraryContainer*>(historyContainer);
	libraryContainers << static_cast<Library::LibraryContainer*>(lyricsContainer);

	m->libraryPluginHandler->init(libraryContainers,
	                              new EmptyLibraryContainer(m->libraryManager, m->libraryPluginHandler));
}

void Application::initPlugins()
{
	[[maybe_unused]] auto measureApp = Util::MeasureApp("Plugins", m->timer);

	auto* playerPluginHandler = PlayerPlugin::Handler::instance();

	auto webClientFactory = std::make_shared<WebClientFactory>();
	auto stationParserFactory = StationParserFactory::createStationParserFactory(webClientFactory, this);
	auto* streamHandler = new StreamHandler(m->playlistHandler, stationParserFactory);
	auto* podcastHandler = new PodcastHandler(m->playlistHandler, stationParserFactory);

	playerPluginHandler->addPlugin(new GUI_LevelPainter(m->engine, m->playManager));
	playerPluginHandler->addPlugin(new GUI_Spectrum(m->engine, m->playManager));
	playerPluginHandler->addPlugin(new GUI_Equalizer(new Equalizer(m->engine)));
	playerPluginHandler->addPlugin(new GUI_Stream(m->playlistHandler, streamHandler));
	playerPluginHandler->addPlugin(new GUI_Podcasts(m->playlistHandler, podcastHandler));
	playerPluginHandler->addPlugin(new GUI_PlaylistChooser(new Playlist::Chooser(m->playlistHandler, this)));
	playerPluginHandler->addPlugin(new GuiSmartPlaylists(m->smartPlaylistManager, m->libraryManager));
	playerPluginHandler->addPlugin(new GUI_AudioConverter(new ConverterFactory(m->playlistHandler)));
	playerPluginHandler->addPlugin(new GUI_Bookmarks(new Bookmarks(m->playManager)));
	playerPluginHandler->addPlugin(new GUI_Speed());
	playerPluginHandler->addPlugin(new GUI_Broadcast(m->playManager, m->engine));
	playerPluginHandler->addPlugin(new GUI_Crossfader());
	playerPluginHandler->addPlugin(new GUI_SpectrogramPainter(m->playManager));
}

void Application::initSingleInstanceThread()
{
	m->instanceThread = new InstanceThread(this);

	connect(m->instanceThread, &InstanceThread::sigPlayerRise, m->player, &GUI_Player::raise);
	connect(m->instanceThread, &InstanceThread::sigCreatePlaylist, this, &Application::createPlaylist);

	m->instanceThread->start();
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
	if(instanceThread)
	{
		const auto paths = instanceThread->paths();
		m->playlistHandler->createCommandLinePlaylist(paths, nullptr);
	}
}

void Application::skinChanged()
{
	Style::applyCurrentStyle(this);
}
