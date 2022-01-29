/* GUI_Player.cpp */

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

#include "GUI_Player.h"
#include "GUI_Logger.h"
#include "GUI_TrayIcon.h"
#include "GUI_PlayerMenubar.h"
#include "GUI_Controls.h"
#include "GUI_ControlsNew.h"
#include "VersionChecker.h"
#include "Translator.h"

#include "Interfaces/CoverDataProvider.h"
#include "Interfaces/DynamicPlayback.h"
#include "Interfaces/PlaylistInterface.h"
#include "Interfaces/PlayManager.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/LibraryManagement/AbstractLibraryContainer.h"

#include "Gui/Covers/CoverButton.h"
#include "Gui/Player/ui_GUI_Player.h"
#include "Gui/Plugins/PlayerPluginBase.h"
#include "Gui/Plugins/PlayerPluginHandler.h"

#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaData.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/EventFilter.h"

#include <QApplication>
#include <QAction>
#include <QDataStream>
#include <QKeySequence>
#include <QTimer>

namespace
{
	QString extractTrackName(const MetaData& track)
	{
		const auto trimmedTitle = track.title().trimmed();
		return (!trimmedTitle.isEmpty())
		       ? trimmedTitle
		       : Util::File::getFilenameOfPath(track.filepath());
	}

	QString getWindowTitle(const MetaData& track)
	{
		const auto title = extractTrackName(track);
		if(!title.isEmpty())
		{
			const auto artist = track.artist().trimmed();
			return (!artist.isEmpty())
			       ? QString("%1 - %2").arg(artist, title)
			       : title;
		}

		return QString("Sayonara %1").arg(GetSetting(Set::Player_Version));
	}

	void changeWindowTitle(QWidget* widget, PlayManager* playManager)
	{
		const auto windowTitle = getWindowTitle(playManager->currentTrack());
		widget->setWindowTitle(windowTitle);
	}
}

struct GUI_Player::Private
{
	Menubar* menubar {nullptr};
	std::shared_ptr<GUI_Logger> logger {nullptr};
	GUI_TrayIcon* trayIcon {nullptr};
	GUI_ControlsBase* controls {nullptr};
	CoverDataProvider* coverProvider;
	PlayManager* playManager;
	bool shutdownRequested {false};

	Private(PlayManager* playManager, PlaylistCreator* playlistCreator, CoverDataProvider* coverProvider,
	        GUI_Player* parent) :
		menubar {new Menubar(playlistCreator, parent)},
		logger {std::make_shared<GUI_Logger>(parent)},
		coverProvider {coverProvider},
		playManager {playManager} {}
};

GUI_Player::GUI_Player(PlayManager* playManager, Playlist::Handler* playlistHandler, CoverDataProvider* coverProvider,
                       DynamicPlaybackChecker* dynamicPlaybackChecker, QWidget* parent) :
	Gui::MainWindow(parent),
	MessageReceiverInterface("Player Main Window")
{
	m = Pimpl::make<Private>(playManager, playlistHandler, coverProvider, this);

	languageChanged();

	ui = std::make_shared<Ui::GUI_Player>();
	ui->setupUi(this);
	ui->retranslateUi(this);
	ui->playlistWidget->init(playlistHandler, playManager, dynamicPlaybackChecker);

	ui->pluginWidget->setVisible(false);

	Message::registerReceiver(this);

	this->setMenuBar(m->menubar);
	this->setWindowIcon(Gui::Icons::icon(Gui::Icons::Logo));
	this->setAttribute(Qt::WA_DeleteOnClose, false);

	initControls();
	initLibrary();
	initFontChangeFix();
	initGeometry();        // init geometry before initConnections
	initConnections();
	initTrayActions();

	changeWindowTitle(this, m->playManager);

	if(GetSetting(Set::Player_NotifyNewVersion))
	{
		auto* versionChecker = new VersionChecker(this);
		connect(versionChecker, &VersionChecker::sigFinished, versionChecker, &QObject::deleteLater);
	}

	ListenSettingNoCall(Set::Player_Fullscreen, GUI_Player::fullscreenChanged);
	ListenSettingNoCall(Set::Lib_Show, GUI_Player::showLibraryChanged);
	ListenSettingNoCall(SetNoDB::Player_Quit, GUI_Player::reallyClose);
	ListenSettingNoCall(Set::Player_ControlStyle, GUI_Player::controlstyleChanged);
}

GUI_Player::~GUI_Player()
{
	spLog(Log::Debug, this) << "Player closed.";
}

static int16_t getGeometryVersion(const QByteArray& geometry)
{
	auto dataStream = QDataStream(geometry);
	int32_t ignoreThis;
	int16_t ret;

	dataStream >> ignoreThis >> ret;
	return ret;
}

void GUI_Player::initGeometry()
{
	const auto geometry = GetSetting(Set::Player_Geometry);
	if(!geometry.isEmpty())
	{
		// newer version of qt store more values than older versions
		// older version have trouble using the new representation,
		// so we have to trim it
		const auto ourGeometryVersion = getGeometryVersion(this->saveGeometry());
		const auto dbGeometryVersion = getGeometryVersion(geometry);
		if(ourGeometryVersion < dbGeometryVersion)
		{
			Gui::Util::placeInScreenCenter(this, 0.8f, 0.8f);
		}

		else
		{
			this->restoreGeometry(geometry);
		}
	}

	else
	{
		Gui::Util::placeInScreenCenter(this, 0.8f, 0.8f);
	}

	if(GetSetting(Set::Player_StartInTray))
	{
		this->setHidden(true);
	}

	else if(GetSetting(Set::Player_Fullscreen))
	{
		this->showFullScreen();
	}

	else if(GetSetting(Set::Player_Maximized))
	{
		this->showMaximized();
	}

	else
	{
		this->showNormal();
	}

	this->initMainSplitter();
	this->initControlSplitter();
}

void GUI_Player::initMainSplitter()
{
	ui->libraryWidget->setVisible(GetSetting(Set::Lib_Show));

	const auto splitterState = GetSetting(Set::Player_SplitterState);
	if(!splitterState.isEmpty())
	{
		ui->splitter->restoreState(splitterState);
	}

	else
	{
		const auto newWidthLeft = width() / 3;
		const auto newWidthRight = width() - newWidthLeft;
		ui->splitter->setSizes({newWidthLeft, newWidthRight});
	}
}

void GUI_Player::initControlSplitter()
{
	const auto splitterState = GetSetting(Set::Player_SplitterControls);
	if(!splitterState.isEmpty())
	{
		ui->splitterControls->restoreState(splitterState);
	}

	QApplication::processEvents();
	this->checkControlSplitter();
}

void GUI_Player::initFontChangeFix()
{
	auto* filter = new Gui::GenericFilter(QEvent::Paint, this);
	connect(filter, &Gui::GenericFilter::sigEvent, this, [=](const auto eventType) {
		if(eventType == QEvent::Type::Paint)
		{
			this->removeEventFilter(filter);
			this->skinChanged();
			this->update();
		}
	});

	installEventFilter(filter);
}

void GUI_Player::initConnections()
{
	auto* lph = Library::PluginHandler::instance();
	connect(lph, &Library::PluginHandler::sigCurrentLibraryChanged, this, &GUI_Player::currentLibraryChanged);

	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, [&](const auto& /* track */) {
		changeWindowTitle(this, m->playManager);
	});
	connect(m->playManager, &PlayManager::sigCurrentMetadataChanged, this, [&]() {
		changeWindowTitle(this, m->playManager);
	});
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, [&](const auto /* playstate */) {
		changeWindowTitle(this, m->playManager);
	});
	connect(m->playManager, &PlayManager::sigError, this, &GUI_Player::playError);

	connect(ui->splitter, &QSplitter::splitterMoved, this, &GUI_Player::splitterMainMoved);
	connect(ui->splitterControls, &QSplitter::splitterMoved, this, &GUI_Player::splitterControlsMoved);

	connect(m->menubar, &Menubar::sigCloseClicked, this, &GUI_Player::reallyClose);
	connect(m->menubar, &Menubar::sigLoggerClicked, m->logger.get(), &GUI_Logger::show);
	connect(m->menubar, &Menubar::sigMinimizeClicked, this, &GUI_Player::minimize);

	auto* pph = PlayerPlugin::Handler::instance();
	connect(pph, &PlayerPlugin::Handler::sigPluginAdded, this, &GUI_Player::pluginAdded);
	connect(pph, &PlayerPlugin::Handler::sigPluginActionTriggered, this, &GUI_Player::pluginActionTriggered);

	auto* dblClickFilter = new Gui::GenericFilter(QEvent::MouseButtonDblClick, ui->splitterControls);
	connect(dblClickFilter, &Gui::GenericFilter::sigEvent, this, [=](QEvent::Type) {
		this->checkControlSplitter();
	});

	ui->splitterControls->handle(1)->installEventFilter(dblClickFilter);
}

void GUI_Player::registerPreferenceDialog(QAction* dialog_action)
{
	m->menubar->insertPreferenceAction(dialog_action);
}

void GUI_Player::initTrayActions()
{
	auto* trayIcon = new GUI_TrayIcon(m->playManager, this);

	connect(trayIcon, &GUI_TrayIcon::sigCloseClicked, this, &GUI_Player::reallyClose);
	connect(trayIcon, &GUI_TrayIcon::sigShowClicked, this, &GUI_Player::raise);
	connect(trayIcon, &GUI_TrayIcon::sigWheelChanged, m->controls, &GUI_ControlsBase::changeVolumeByDelta);
	connect(trayIcon, &GUI_TrayIcon::activated, this, &GUI_Player::trayIconActivated);

	if(GetSetting(Set::Player_ShowTrayIcon))
	{
		trayIcon->show();
	}

	m->trayIcon = trayIcon;
}

void GUI_Player::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if(reason != QSystemTrayIcon::Trigger)
	{
		return;
	}

	if(this->isMinimized() || !isVisible() || !isActiveWindow())
	{
		raise();
	}

	else
	{
		minimize();
	}
}

void GUI_Player::playError(const QString& message)
{
	const auto& md = m->playManager->currentTrack();
	Message::warning
		(
			message + "\n\n" + md.filepath(),
			Lang::get(Lang::Play)
		);
}

void GUI_Player::pluginAdded(PlayerPlugin::Base* plugin)
{
	auto* pph = PlayerPlugin::Handler::instance();
	if(plugin == pph->currentPlugin())
	{
		ui->pluginWidget->showCurrentPlugin();
	}
}

void GUI_Player::pluginActionTriggered(bool b)
{
	if(b)
	{
		ui->pluginWidget->showCurrentPlugin();
	}

	else
	{
		ui->pluginWidget->close();
	}

	checkControlSplitter();
}

void GUI_Player::initControls()
{
	m->controls = (GetSetting(Set::Player_ControlStyle) == 0)
	              ? static_cast<GUI_ControlsBase*>(new GUI_Controls(m->playManager, m->coverProvider, this))
	              : static_cast<GUI_ControlsBase*>(new GUI_ControlsNew(m->playManager, m->coverProvider, this));

	m->controls->init();
	ui->controls->layout()->addWidget(m->controls);
	ui->splitterControls->setHandleEnabled(m->controls->isExternResizeAllowed());
}

void GUI_Player::controlstyleChanged()
{
	ui->controls->layout()->removeWidget(m->controls);
	m->controls->deleteLater();

	initControls();

	if(!m->controls->isExternResizeAllowed())
	{
		ui->splitterControls->setSizes({0, this->height()});
	}

	else
	{
		ui->splitterControls->setSizes({350, this->height() - 350});
	}
}

void GUI_Player::currentLibraryChanged()
{
	showLibraryChanged();
}

void GUI_Player::initLibrary()
{
	bool isVisible = GetSetting(Set::Lib_Show);
	ui->libraryWidget->setVisible(isVisible);

	m->menubar->showLibraryMenu(isVisible);

	if(isVisible)
	{
		addCurrentLibrary();
		QWidget* w = Library::PluginHandler::instance()->currentLibraryWidget();
		if(w)
		{
			w->show();
			w->setFocus();
		}
	}

	else
	{
		ui->libraryWidget->setVisible(false);
		removeCurrentLibrary();
	}
}

void GUI_Player::showLibraryChanged()
{
	// we have to do this here because init_library will show/hide ui->library_widget
	bool wasVisible = ui->libraryWidget->isVisible();
	int oldLibraryWidth = ui->libraryWidget->width();

	initLibrary();

	QSize playerSize = this->size();
	QList<int> sizes = ui->splitter->sizes();

	if(GetSetting(Set::Lib_Show))
	{
		if(!wasVisible)
		{
			// only change sizes if library wasn't visible until now,
			// otherwise player becomes wider
			sizes[1] = GetSetting(Set::Lib_OldWidth);
			playerSize.setWidth(playerSize.width() + GetSetting(Set::Lib_OldWidth));
		}

		ui->splitter->setHandleWidth(4);
	}

	else
	{
		sizes[1] = 0;
		playerSize.setWidth(playerSize.width() - ui->libraryWidget->width());

		if(wasVisible)
		{
			SetSetting(Set::Lib_OldWidth, oldLibraryWidth);
		}

		ui->splitter->setHandleWidth(0);
	}

	if((playerSize != this->size()))
	{
		QApplication::processEvents();

		if(this->minimumWidth() > playerSize.width())
		{
			this->setMinimumWidth(playerSize.width());
		}

		this->resize(playerSize);
		ui->splitter->setSizes(sizes);
	}
}

void GUI_Player::addCurrentLibrary()
{
	auto* layout = ui->libraryWidget->layout();
	if(!layout)
	{
		layout = new QVBoxLayout();
		ui->libraryWidget->setLayout(layout);
	}

	removeCurrentLibrary();

	auto* libraryWidget = Library::PluginHandler::instance()->currentLibraryWidget();
	if(libraryWidget)
	{
		layout->addWidget(libraryWidget);
	}
}

void GUI_Player::removeCurrentLibrary()
{
	auto* layout = ui->libraryWidget->layout();
	while(layout->count() > 0)
	{
		auto* layoutItem = layout->takeAt(0);
		if(layoutItem && layoutItem->widget())
		{
			layoutItem->widget()->hide();
		}
	}
}

void GUI_Player::splitterMainMoved(int /*pos*/, int /*idx*/)
{
	checkControlSplitter();
}

void GUI_Player::splitterControlsMoved(int /*pos*/, int /*idx*/)
{
	checkControlSplitter();
}

void GUI_Player::checkControlSplitter()
{
	if(m->controls && m->controls->isExternResizeAllowed())
	{
		const auto sizes = ui->splitterControls->sizes();

		// remove empty space on top/bottom of cover
		const auto difference = m->controls->btnCover()->verticalPadding();
		if(difference > 0)
		{
			const auto newSize = sizes[0] - difference;
			ui->splitterControls->widget(0)->setMaximumHeight(newSize);
		}

		const auto minimumHeight = ui->pluginWidget->isVisible()
		                           ? 350
		                           : 200;

		ui->splitterControls->widget(1)->setMinimumHeight(minimumHeight);
	}
}

void GUI_Player::languageChanged()
{
	const auto language = GetSetting(Set::Player_Language);
	Translator::instance()->changeLanguage(this, language);

	if(ui)
	{
		ui->retranslateUi(this);
	}
}

void GUI_Player::minimize()
{
	if(GetSetting(Set::Player_Min2Tray))
	{
		hide();
	}

	else
	{
		showMinimized();
	}
}

void GUI_Player::fullscreenChanged()
{
	if(GetSetting(Set::Player_Fullscreen))
	{
		showFullScreen();
	}

	else
	{
		showNormal();
	}
}

void GUI_Player::reallyClose()
{
	spLog(Log::Info, this) << "closing player...";
	Gui::MainWindow::close();
}

void GUI_Player::requestShutdown()
{
	m->shutdownRequested = true;
}

void GUI_Player::resizeEvent(QResizeEvent* e)
{
	Gui::MainWindow::resizeEvent(e);
	checkControlSplitter();
}

void GUI_Player::closeEvent(QCloseEvent* e)
{
	SetSetting(Set::Player_Geometry, this->saveGeometry());
	SetSetting(Set::Player_Maximized, this->isMaximized());
	SetSetting(Set::Player_Fullscreen, this->isFullScreen());
	SetSetting(Set::Player_SplitterState, ui->splitter->saveState());
	SetSetting(Set::Player_SplitterControls, ui->splitterControls->saveState());

	if(!m->shutdownRequested && GetSetting(Set::Player_Min2Tray) && !GetSetting(SetNoDB::Player_Quit))
	{
		if(GetSetting(Set::Player_514Fix))
		{
			e->ignore();
			QApplication::processEvents();
		}

		this->hide();
	}

	else
	{
		m->trayIcon->hide();
		Gui::MainWindow::closeEvent(e);
		emit sigClosed();
	}
}

bool GUI_Player::event(QEvent* e)
{
	const auto b = Gui::MainWindow::event(e);
	if(e->type() == QEvent::WindowStateChange)
	{
		m->menubar->setShowLibraryActionEnabled(!(isMaximized() || isFullScreen()));
	}

	return b;
}
