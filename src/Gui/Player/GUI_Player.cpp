/* GUI_Player.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
#include "Interfaces/Library/LibraryContainer.h"

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
	GUI_TrayIcon*				tray_icon=nullptr;
	GUI_ControlsBase*			controls=nullptr;
	bool						shutdown_requested;

	Private(GUI_Player* parent) :
		shutdown_requested(false)
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

	language_changed();

	ui = new Ui::GUI_Player();
	ui->setupUi(this);
	ui->retranslateUi(this);

	ui->plugin_widget->setVisible(false);

	Message::register_receiver(this);

	this->setMenuBar(m->menubar);
	this->setWindowIcon(Gui::Util::icon("logo.png", Gui::Util::NoTheme));
	this->setAttribute(Qt::WA_DeleteOnClose, false);

	init_controls();
	init_library();
	init_font_change_fix();
	init_geometry();		// init geometry before init_connections
	init_connections();
	init_tray_actions();

	current_track_changed(PlayManager::instance()->current_track());

	if(GetSetting(Set::Player_NotifyNewVersion))
	{
		auto* vc = new VersionChecker(this);
		connect(vc, &VersionChecker::sig_finished, vc, &QObject::deleteLater);
	}

	ListenSettingNoCall(Set::Player_Fullscreen, GUI_Player::fullscreen_changed);
	ListenSettingNoCall(Set::Lib_Show, GUI_Player::show_library_changed);
	ListenSettingNoCall(SetNoDB::Player_Quit, GUI_Player::really_close);
	ListenSettingNoCall(Set::Player_ControlStyle, GUI_Player::controlstyle_changed);
}

GUI_Player::~GUI_Player()
{
	sp_log(Log::Debug, this) << "Player closed.";
	delete ui; ui=nullptr;
}

static int16_t get_geometry_version(const QByteArray& geometry)
{
	QDataStream str(geometry);
	int32_t magic;
	int16_t ret;

	str >> magic >> ret;
	return ret;
}

void GUI_Player::init_geometry()
{
	QByteArray geometry = GetSetting(Set::Player_Geometry);
	if(!geometry.isEmpty())
	{
		// newer version of qt store more values than older versions
		// older version have trouble using the new representation,
		// so we have to trim it
		int16_t our_geometry_version = get_geometry_version(this->saveGeometry());
		int16_t db_geometry_version = get_geometry_version(geometry);

		if(our_geometry_version < db_geometry_version) {
			Gui::Util::place_in_screen_center(this, 0.8f, 0.8f);
		}

		else {
			this->restoreGeometry(geometry);
		}
	}

	else {
		Gui::Util::place_in_screen_center(this, 0.8f, 0.8f);
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

	this->init_main_splitter();
	this->init_control_splitter();
}


void GUI_Player::init_main_splitter()
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

void GUI_Player::init_control_splitter()
{
	QByteArray splitter_state = GetSetting(Set::Player_SplitterControls);
	if(!splitter_state.isEmpty())
	{
		ui->splitterControls->restoreState(splitter_state);
	}

	else
	{
		this->check_control_splitter(true);
	}
}

void GUI_Player::init_font_change_fix()
{
	auto* filter = new Gui::GenericFilter
	(
		QList<QEvent::Type>{QEvent::Paint},
		this
	);

	connect(filter, &Gui::GenericFilter::sig_event, this, [=](QEvent::Type t)
	{
		Q_UNUSED(t)

		this->removeEventFilter(filter);
		this->skin_changed();
		this->update();
	});

	installEventFilter(filter);
}

void GUI_Player::init_connections()
{
	auto* lph = Library::PluginHandler::instance();
	connect(lph, &Library::PluginHandler::sig_current_library_changed,
			this, &GUI_Player::current_library_changed);

	auto* play_manager = PlayManager::instance();
	connect(play_manager, &PlayManager::sig_track_changed, this, &GUI_Player::current_track_changed);
	connect(play_manager, &PlayManager::sig_playstate_changed, this, &GUI_Player::playstate_changed);
	connect(play_manager, &PlayManager::sig_error, this, &GUI_Player::play_error);

	connect(ui->splitter, &QSplitter::splitterMoved, this, &GUI_Player::splitter_main_moved);
	connect(ui->splitterControls, &QSplitter::splitterMoved, this, &GUI_Player::splitter_controls_moved);

	connect(m->menubar, &Menubar::sig_close_clicked, this, &GUI_Player::really_close);
	connect(m->menubar, &Menubar::sig_logger_clicked, m->logger.get(), &GUI_Logger::show);
	connect(m->menubar, &Menubar::sig_minimize_clicked, this, &GUI_Player::minimize);

	auto* pph = PlayerPlugin::Handler::instance();
	connect(pph, &PlayerPlugin::Handler::sig_plugin_added, this, &GUI_Player::plugin_added);
	connect(pph, &PlayerPlugin::Handler::sig_plugin_action_triggered, this, &GUI_Player::plugin_action_triggered);

	auto* dbl_click_filter = new Gui::GenericFilter(QEvent::MouseButtonDblClick, ui->splitterControls);
	connect(dbl_click_filter, &Gui::GenericFilter::sig_event, this, [=](QEvent::Type)
	{
		this->check_control_splitter(true);
	});

	ui->splitterControls->handle(1)->installEventFilter(dbl_click_filter);
}

void GUI_Player::register_preference_dialog(QAction* dialog_action)
{
	m->menubar->insert_preference_action(dialog_action);
}

void GUI_Player::init_tray_actions()
{
	auto* tray_icon = new GUI_TrayIcon(this);

	connect(tray_icon, &GUI_TrayIcon::sig_close_clicked, this, &GUI_Player::really_close);
	connect(tray_icon, &GUI_TrayIcon::sig_show_clicked, this, &GUI_Player::raise);
	connect(tray_icon, &GUI_TrayIcon::sig_wheel_changed, m->controls, &GUI_ControlsBase::change_volume_by_tick);
	connect(tray_icon, &GUI_TrayIcon::activated, this, &GUI_Player::tray_icon_activated);

	if(GetSetting(Set::Player_ShowTrayIcon)) {
		tray_icon->show();
	}

	m->tray_icon = tray_icon;
}

void GUI_Player::tray_icon_activated(QSystemTrayIcon::ActivationReason reason)
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

void GUI_Player::current_track_changed(const MetaData& md)
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


void GUI_Player::playstate_changed(PlayState state)
{
	if(state == PlayState::Stopped) {
		setWindowTitle("Sayonara Player");
	}
}

void GUI_Player::play_error(const QString& message)
{
	MetaData md = PlayManager::instance()->current_track();
	Message::warning
	(
		message + "\n\n" + md.filepath(),
		Lang::get(Lang::Play)
	);
}

void GUI_Player::plugin_added(PlayerPlugin::Base* plugin)
{
	auto* pph = PlayerPlugin::Handler::instance();
	if(plugin == pph->current_plugin())
	{
		ui->plugin_widget->show_current_plugin();
	}
}

void GUI_Player::plugin_action_triggered(bool b)
{
	if(b) {
		ui->plugin_widget->show_current_plugin();
	}

	else {
		ui->plugin_widget->close();
	}
}


void GUI_Player::init_controls()
{
	if(GetSetting(Set::Player_ControlStyle) == 0) {
		m->controls = new GUI_Controls();
	}

	else {
		m->controls = new GUI_ControlsNew();
	}

	m->controls->init();
	ui->controls->layout()->addWidget(m->controls);
	ui->splitterControls->set_handle_enabled(m->controls->is_extern_resize_allowed());
}


void GUI_Player::controlstyle_changed()
{
	ui->controls->layout()->removeWidget(m->controls);
	m->controls->deleteLater();

	init_controls();

	if(!m->controls->is_extern_resize_allowed()) {
		ui->splitterControls->setSizes({0, this->height()});
	}

	else {
		ui->splitterControls->setSizes({350, this->height() - 350});
	}
}


void GUI_Player::current_library_changed()
{
	show_library_changed();
}

void GUI_Player::init_library()
{
	bool is_visible = GetSetting(Set::Lib_Show);
	ui->library_widget->setVisible(is_visible);

	m->menubar->show_library_menu(is_visible);

	if(is_visible)
	{
		add_current_library();
		QWidget* w = Library::PluginHandler::instance()->current_library_widget();
		if(w)
		{
			w->show();
			w->setFocus();
		}
	}

	else
	{
		ui->library_widget->setVisible(false);
		remove_current_library();
	}
}


void GUI_Player::show_library_changed()
{
	// we have to do this here because init_library will show/hide ui->library_widget
	bool was_visible = ui->library_widget->isVisible();
	int old_lib_width = ui->library_widget->width();

	init_library();

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

void GUI_Player::add_current_library()
{
	QLayout* layout = ui->library_widget->layout();
	if(!layout)
	{
		layout = new QVBoxLayout();
		ui->library_widget->setLayout(layout);
	}

	remove_current_library();

	auto* lph = Library::PluginHandler::instance();
	QWidget* w = lph->current_library_widget();
	if(w) {
		layout->addWidget(w);
	}
}

void GUI_Player::remove_current_library()
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

void GUI_Player::splitter_main_moved(int /*pos*/, int /*idx*/)
{
	check_control_splitter((QApplication::keyboardModifiers() & Qt::ControlModifier));

	// we have to reset the minimum size otherwise the old minimum size stays active
	// and we cannot enlarge that cover after the cover geometry has changed
	ui->splitterControls->widget(1)->setMinimumHeight(200);
}

void GUI_Player::splitter_controls_moved(int /*pos*/, int /*idx*/)
{
	check_control_splitter(false);
}

void GUI_Player::check_control_splitter(bool force)
{
	if(m->controls && m->controls->is_extern_resize_allowed())
	{
		// remove empty space on top/bottom of cover
		int difference = m->controls->btn_cover()->vertical_padding();
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

void GUI_Player::language_changed()
{
	QString language = GetSetting(Set::Player_Language);
	Translator::instance()->change_language(this, language);

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

void GUI_Player::fullscreen_changed()
{
	if(GetSetting(Set::Player_Fullscreen)) {
		showFullScreen();
	}

	else {
		showNormal();
	}
}

void GUI_Player::really_close()
{
	sp_log(Log::Info, this) << "closing player...";

	Gui::MainWindow::close();

	emit sig_player_closed();
}

void GUI_Player::request_shutdown()
{
	m->shutdown_requested = true;
}

void GUI_Player::resizeEvent(QResizeEvent* e)
{
	Gui::MainWindow::resizeEvent(e);
	check_control_splitter(true);
}

void GUI_Player::closeEvent(QCloseEvent* e)
{
	bool min_to_tray = GetSetting(Set::Player_Min2Tray);

	SetSetting(Set::Player_Geometry, this->saveGeometry());
	SetSetting(Set::Player_Maximized, this->isMaximized());
	SetSetting(Set::Player_Fullscreen, this->isFullScreen());
	SetSetting(Set::Player_SplitterState, ui->splitter->saveState());
	SetSetting(Set::Player_SplitterControls, ui->splitterControls->saveState());

	if(!m->shutdown_requested && min_to_tray && !GetSetting(SetNoDB::Player_Quit))
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
		m->tray_icon->hide();
		Gui::MainWindow::closeEvent(e);
		emit sig_player_closed();
	}
}

bool GUI_Player::event(QEvent* e)
{
	bool b = Gui::MainWindow::event(e);

	if(e->type() == QEvent::WindowStateChange)
	{
		m->menubar->set_show_library_action_enabled
		(
			!(isMaximized() || isFullScreen())
		);
	}

	return b;
}
