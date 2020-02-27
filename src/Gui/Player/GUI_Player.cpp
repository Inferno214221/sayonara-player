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

#include "Components/PlayManager/PlayManager.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/LibraryManagement/AbstractLibraryContainer.h"

#include "Gui/Covers/CoverButton.h"
#include "Gui/Player/ui_GUI_Player.h"
#include "Gui/Plugins/PlayerPluginBase.h"
#include "Gui/Plugins/PlayerPluginHandler.h"

#include "Utils/Utils.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaData.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/EventFilter.h"

#include <QAction>
#include <QDataStream>
#include <QKeySequence>
#include <QTimer>

struct GUI_Player::Private
{
	Menubar*					menubar=nullptr;
	std::shared_ptr<GUI_Logger>	logger=nullptr;
	GUI_TrayIcon*				trayIcon=nullptr;
	GUI_ControlsBase*			controls=nullptr;
	bool						shutdownRequested;

	Private(GUI_Player* parent) :
		shutdownRequested(false)
	{
		logger = std::make_shared<GUI_Logger>();
		menubar = new Menubar(parent);
	}
};


GUI_Player::GUI_Player(QWidget* parent) :
	Gui::MainWindow(parent),
	MessageReceiverInterface("Player Main Window")
{
	m = Pimpl::make<Private>(this);

	languageChanged();

	ui = new Ui::GUI_Player();
	ui->setupUi(this);
	ui->retranslateUi(this);

	ui->plugin_widget->setVisible(false);

	Message::registerReceiver(this);

	this->setMenuBar(m->menubar);
	this->setWindowIcon(Gui::Util::icon("logo.png", Gui::Util::NoTheme));
	this->setAttribute(Qt::WA_DeleteOnClose, false);

	initControls();
	initLibrary();
	initFontChangeFix();
	initGeometry();		// init geometry before init_connections
	initConnections();
	initTrayActions();

	currentTrackChanged(PlayManager::instance()->currentTrack());

	if(GetSetting(Set::Player_NotifyNewVersion))
	{
		auto* vc = new VersionChecker(this);
		connect(vc, &VersionChecker::sigFinished, vc, &QObject::deleteLater);
	}

	ListenSettingNoCall(Set::Player_Fullscreen, GUI_Player::fullscreenChanged);
	ListenSettingNoCall(Set::Lib_Show, GUI_Player::showLibraryChanged);
	ListenSettingNoCall(SetNoDB::Player_Quit, GUI_Player::reallyClose);
	ListenSettingNoCall(Set::Player_ControlStyle, GUI_Player::controlstyleChanged);
}

GUI_Player::~GUI_Player()
{
	spLog(Log::Debug, this) << "Player closed.";
	delete ui; ui=nullptr;
}

static int16_t getGeometryVersion(const QByteArray& geometry)
{
	QDataStream str(geometry);
	int32_t magic;
	int16_t ret;

	str >> magic >> ret;
	return ret;
}

void GUI_Player::initGeometry()
{
	QByteArray geometry = GetSetting(Set::Player_Geometry);
	if(!geometry.isEmpty())
	{
		// newer version of qt store more values than older versions
		// older version have trouble using the new representation,
		// so we have to trim it
		int16_t our_geometry_version = getGeometryVersion(this->saveGeometry());
		int16_t db_geometry_version = getGeometryVersion(geometry);

		if(our_geometry_version < db_geometry_version) {
			Gui::Util::placeInScreenCenter(this, 0.8f, 0.8f);
		}

		else {
			this->restoreGeometry(geometry);
		}
	}

	else {
		Gui::Util::placeInScreenCenter(this, 0.8f, 0.8f);
	}

	if(GetSetting(Set::Player_StartInTray)) {
		this->setHidden(true);
	}

	else if(GetSetting(Set::Player_Fullscreen))	{
		this->showFullScreen();
	}

	else if(GetSetting(Set::Player_Maximized)) {
		this->showMaximized();
	}

	else {
		this->showNormal();	
	}

	this->initMainSplitter();
	this->initControlSplitter();
}


void GUI_Player::initMainSplitter()
{
	ui->library_widget->setVisible(GetSetting(Set::Lib_Show));

	QByteArray splitter_state = GetSetting(Set::Player_SplitterState);
	if(!splitter_state.isEmpty())
	{
		ui->splitter->restoreState(splitter_state);
	}

	else
	{
		int w1 = width() / 3;
		int w2 = width() - w1;
		ui->splitter->setSizes({w1, w2});
	}
}

void GUI_Player::initControlSplitter()
{
	QByteArray splitter_state = GetSetting(Set::Player_SplitterControls);
	if(!splitter_state.isEmpty())
	{
		ui->splitterControls->restoreState(splitter_state);
	}

	else
	{
		this->checkControlSplitter(true);
	}
}

void GUI_Player::initFontChangeFix()
{
	auto* filter = new Gui::GenericFilter
	(
		QList<QEvent::Type>{QEvent::Paint},
		this
	);

	connect(filter, &Gui::GenericFilter::sigEvent, this, [=](QEvent::Type t)
	{
		Q_UNUSED(t)

		this->removeEventFilter(filter);
		this->skinChanged();
		this->update();
	});

	installEventFilter(filter);
}

void GUI_Player::initConnections()
{
	auto* lph = Library::PluginHandler::instance();
	connect(lph, &Library::PluginHandler::sigCurrentLibraryChanged,
			this, &GUI_Player::currentLibraryChanged);

	auto* playManager = PlayManager::instance();
	connect(playManager, &PlayManager::sigCurrentTrackChanged, this, &GUI_Player::currentTrackChanged);
	connect(playManager, &PlayManager::sigPlaystateChanged, this, &GUI_Player::playstateChanged);
	connect(playManager, &PlayManager::sigError, this, &GUI_Player::playError);

	connect(ui->splitter, &QSplitter::splitterMoved, this, &GUI_Player::splitterMainMoved);
	connect(ui->splitterControls, &QSplitter::splitterMoved, this, &GUI_Player::splitterControlsMoved);

	connect(m->menubar, &Menubar::sigCloseClicked, this, &GUI_Player::reallyClose);
	connect(m->menubar, &Menubar::sigLoggerClicked, m->logger.get(), &GUI_Logger::show);
	connect(m->menubar, &Menubar::sigMinimizeClicked, this, &GUI_Player::minimize);

	auto* pph = PlayerPlugin::Handler::instance();
	connect(pph, &PlayerPlugin::Handler::sigPluginAdded, this, &GUI_Player::pluginAdded);
	connect(pph, &PlayerPlugin::Handler::sigPluginActionTriggered, this, &GUI_Player::pluginActionTriggered);

	auto* dbl_click_filter = new Gui::GenericFilter(QEvent::MouseButtonDblClick, ui->splitterControls);
	connect(dbl_click_filter, &Gui::GenericFilter::sigEvent, this, [=](QEvent::Type)
	{
		this->checkControlSplitter(true);
	});

	ui->splitterControls->handle(1)->installEventFilter(dbl_click_filter);
}

void GUI_Player::registerPreferenceDialog(QAction* dialog_action)
{
	m->menubar->insertPreferenceAction(dialog_action);
}

void GUI_Player::initTrayActions()
{
	auto* tray_icon = new GUI_TrayIcon(this);

	connect(tray_icon, &GUI_TrayIcon::sigCloseClicked, this, &GUI_Player::reallyClose);
	connect(tray_icon, &GUI_TrayIcon::sigShowClicked, this, &GUI_Player::raise);
	connect(tray_icon, &GUI_TrayIcon::sigWheelChanged, m->controls, &GUI_ControlsBase::changeVolumeByDelta);
	connect(tray_icon, &GUI_TrayIcon::activated, this, &GUI_Player::trayIconActivated);

	if(GetSetting(Set::Player_ShowTrayIcon)) {
		tray_icon->show();
	}

	m->trayIcon = tray_icon;
}

void GUI_Player::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{	
	if(reason != QSystemTrayIcon::Trigger) {
		return;
	}

	if(this->isMinimized() || !isVisible() || !isActiveWindow()) {
		raise();
	}

	else {
		minimize();
	}
}

void GUI_Player::currentTrackChanged(const MetaData& md)
{
	if(md.title().trimmed().isEmpty()) {
		this->setWindowTitle("Sayonara " + GetSetting(Set::Player_Version));
	}

	else if(md.artist().trimmed().isEmpty()) {
		this->setWindowTitle(md.title());
	}

	else {
		this->setWindowTitle(md.artist() + " - " + md.title());
	}
}


void GUI_Player::playstateChanged(PlayState state)
{
	if(state == PlayState::Stopped) {
		setWindowTitle("Sayonara Player");
	}
}

void GUI_Player::playError(const QString& message)
{
	MetaData md = PlayManager::instance()->currentTrack();
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
		ui->plugin_widget->showCurrentPlugin();
	}
}

void GUI_Player::pluginActionTriggered(bool b)
{
	if(b) {
		ui->plugin_widget->showCurrentPlugin();
	}

	else {
		ui->plugin_widget->close();
	}
}


void GUI_Player::initControls()
{
	if(GetSetting(Set::Player_ControlStyle) == 0) {
		m->controls = new GUI_Controls();
	}

	else {
		m->controls = new GUI_ControlsNew();
	}

	m->controls->init();
	ui->controls->layout()->addWidget(m->controls);
	ui->splitterControls->set_handle_enabled(m->controls->isExternResizeAllowed());
}


void GUI_Player::controlstyleChanged()
{
	ui->controls->layout()->removeWidget(m->controls);
	m->controls->deleteLater();

	initControls();

	if(!m->controls->isExternResizeAllowed()) {
		ui->splitterControls->setSizes({0, this->height()});
	}

	else {
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
	ui->library_widget->setVisible(isVisible);

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
		ui->library_widget->setVisible(false);
		removeCurrentLibrary();
	}
}


void GUI_Player::showLibraryChanged()
{
	// we have to do this here because init_library will show/hide ui->library_widget
	bool was_visible = ui->library_widget->isVisible();
	int old_lib_width = ui->library_widget->width();

	initLibrary();

	QSize player_size = this->size();
	QList<int> sizes = ui->splitter->sizes();
	QByteArray splitter_controls_state = ui->splitterControls->saveState();

	if(GetSetting(Set::Lib_Show))
	{
		if(!was_visible)
		{
			// only change sizes if library wasn't visible until now,
			// otherwise player becomes wider
			sizes[1] = GetSetting(Set::Lib_OldWidth);
			player_size.setWidth(player_size.width() + GetSetting(Set::Lib_OldWidth));
		}
	}

	else
	{
		sizes[1] = 0;
		player_size.setWidth(player_size.width() - ui->library_widget->width());

		if(was_visible)
		{
			SetSetting(Set::Lib_OldWidth, old_lib_width);
		}
	}

	if((player_size != this->size()))
	{
		QTimer::singleShot(100, this, [=]()
		{
			resize(player_size);
			ui->splitter->setSizes(sizes);
			ui->splitterControls->restoreState(splitter_controls_state);
		});
	}
}

void GUI_Player::addCurrentLibrary()
{
	QLayout* layout = ui->library_widget->layout();
	if(!layout)
	{
		layout = new QVBoxLayout();
		ui->library_widget->setLayout(layout);
	}

	removeCurrentLibrary();

	QWidget* w = Library::PluginHandler::instance()->currentLibraryWidget();
	if(w)
	{
		layout->addWidget(w);
	}
}

void GUI_Player::removeCurrentLibrary()
{
	QLayout* layout = ui->library_widget->layout();
	while(layout->count() > 0)
	{
		QLayoutItem* item = layout->takeAt(0);
		if(item && item->widget()){
			item->widget()->hide();
		}
	}
}

void GUI_Player::splitterMainMoved(int /*pos*/, int /*idx*/)
{
	checkControlSplitter((QApplication::keyboardModifiers() & Qt::ControlModifier));

	// we have to reset the minimum size otherwise the old minimum size stays active
	// and we cannot enlarge that cover after the cover geometry has changed
	ui->splitterControls->widget(1)->setMinimumHeight(200);
}

void GUI_Player::splitterControlsMoved(int /*pos*/, int /*idx*/)
{
	checkControlSplitter(false);
}

void GUI_Player::checkControlSplitter(bool force)
{
	if(m->controls && m->controls->isExternResizeAllowed())
	{
		// remove empty space on top/bottom of cover
		int difference = m->controls->btnCover()->verticalPadding();
		if(difference > 0 || force)
		{
			auto sizes = ui->splitterControls->sizes();
				sizes[0] -= difference;
				sizes[1] += difference;

			if(sizes[1] >= 200)
			{
				{ // avoid flickering
					ui->splitterControls->widget(1)->setMinimumHeight(sizes[1]);
				}

				ui->splitterControls->setSizes(sizes);
			}
		}
	}
}

void GUI_Player::languageChanged()
{
	QString language = GetSetting(Set::Player_Language);
	Translator::instance()->changeLanguage(this, language);

	if(ui) {
		ui->retranslateUi(this);
	}
}

void GUI_Player::minimize()
{
	if(GetSetting(Set::Player_Min2Tray)) {
		hide();
	}

	else {
		showMinimized();
	}
}

void GUI_Player::fullscreenChanged()
{
	if(GetSetting(Set::Player_Fullscreen)) {
		showFullScreen();
	}

	else {
		showNormal();
	}
}

void GUI_Player::reallyClose()
{
	spLog(Log::Info, this) << "closing player...";

	Gui::MainWindow::close();

	emit sig_player_closed();
}

void GUI_Player::requestShutdown()
{
	m->shutdownRequested = true;
}

void GUI_Player::resizeEvent(QResizeEvent* e)
{
	Gui::MainWindow::resizeEvent(e);
	checkControlSplitter(true);
}

void GUI_Player::closeEvent(QCloseEvent* e)
{
	bool min_to_tray = GetSetting(Set::Player_Min2Tray);

	SetSetting(Set::Player_Geometry, this->saveGeometry());
	SetSetting(Set::Player_Maximized, this->isMaximized());
	SetSetting(Set::Player_Fullscreen, this->isFullScreen());
	SetSetting(Set::Player_SplitterState, ui->splitter->saveState());
	SetSetting(Set::Player_SplitterControls, ui->splitterControls->saveState());

	if(!m->shutdownRequested && min_to_tray && !GetSetting(SetNoDB::Player_Quit))
	{
		if(GetSetting(Set::Player_514Fix)) {
			e->ignore();
			QTimer::singleShot(10, this, &GUI_Player::hide);
		}

		else {
			hide();
		}
	}

	else
	{
		m->trayIcon->hide();
		Gui::MainWindow::closeEvent(e);
		emit sig_player_closed();
	}
}

bool GUI_Player::event(QEvent* e)
{
	bool b = Gui::MainWindow::event(e);

	if(e->type() == QEvent::WindowStateChange)
	{
		m->menubar->setShowLibraryActionEnabled
		(
			!(isMaximized() || isFullScreen())
		);
	}

	return b;
}
