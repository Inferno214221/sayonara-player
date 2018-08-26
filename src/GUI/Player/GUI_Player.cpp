/* GUI_Player.cpp */

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

#include "GUI_Player.h"
#include "GUI_Logger.h"
#include "GUI_TrayIcon.h"
#include "GUI_PlayerMenubar.h"
#include "GUI_Controls.h"
#include "GUI_ControlsNew.h"
#include "GUI/Player/ui_GUI_Player.h"

#include "Components/PlayManager/PlayManager.h"

#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Utils.h"
#include "Utils/globals.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaData.h"

#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Style.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/EventFilter.h"

#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"
#include "Interfaces/LibraryInterface/LibraryContainer/LibraryContainer.h"
#include "Interfaces/PlayerPlugin/PlayerPluginBase.h"
#include "Interfaces/PlayerPlugin/PlayerPluginHandler.h"

#include <QTranslator>
#include <QAction>
#include <QKeySequence>
#include <QCloseEvent>
#include <QResizeEvent>

struct GUI_Player::Private
{
	Menubar*					menubar=nullptr;
	QList<QTranslator*>			translators;
	GUI_Logger*					logger=nullptr;
	GUI_TrayIcon*				tray_icon=nullptr;
	GUI_ControlsBase*			controls=nullptr;

	// hack for resizing the player after library show/hide:
	// star resizing after splitter has finished painting
	GenericFilter*				splitter_paint_event_filter=nullptr;
	GenericFilter*				style_changed_event_filter=nullptr;

	QPoint						initial_pos;
	QSize						initial_sz;
	QSize						new_size;
	QString						current_language;

	int							style;
	bool						shutdown_requested;
	bool						is_maximizable;
	bool						is_fullscreenable;

	Private() :
		shutdown_requested(false),
		is_maximizable(true),
		is_fullscreenable(true)
	{
		Settings* s = Settings::instance();
		logger = new GUI_Logger();

		initial_pos = s->get<Set::Player_Pos>();
		initial_sz = s->get<Set::Player_Size>();
		style = s->get<Set::Player_Style>();
	}

	~Private()
	{
		delete logger; logger = nullptr;
	}
};


GUI_Player::GUI_Player(QWidget* parent) :
	Gui::MainWindow(parent),
	MessageReceiverInterface("Player Main Window")
{
	m = Pimpl::make<Private>();
	m->style_changed_event_filter = new GenericFilter(
		QList<QEvent::Type>{QEvent::Paint},
		this
	);

	language_changed();

	ui = new Ui::GUI_Player();
	ui->setupUi(this);
	ui->retranslateUi(this);

	ui->plugin_widget->setVisible(false);

	Message::register_receiver(this);

	m->menubar = new Menubar(this);
	setMenuBar(m->menubar);

	QString version = _settings->get<Set::Player_Version>();
	setWindowTitle(QString("Sayonara %1").arg(version));
	setWindowIcon(Gui::Util::icon("logo.png"));
	setAttribute(Qt::WA_DeleteOnClose, false);

	init_font_change_fix();
	init_sizes();
	init_main_splitter();
	init_controlstyle();
	init_connections();
	init_tray_actions();

	current_track_changed(PlayManager::instance()->current_track());

	if(_settings->get<Set::Player_NotifyNewVersion>())
	{
		AsyncWebAccess* awa = new AsyncWebAccess(this);
		awa->run("http://sayonara-player.com/current_version");
		connect(awa, &AsyncWebAccess::sig_finished, this, &GUI_Player::awa_version_finished);
	}

	Set::listen<Set::Player_Fullscreen>(this, &GUI_Player::fullscreen_changed, false);
	Set::listen<Set::Lib_Show>(this, &GUI_Player::show_library_changed, false);
	Set::listen<SetNoDB::Player_Quit>(this, &GUI_Player::really_close, false);
	Set::listen<Set::Player_ControlStyle>(this, &GUI_Player::controlstyle_changed, false);
}

GUI_Player::~GUI_Player()
{
	sp_log(Log::Debug, this) << "Player closed.";

	delete ui; ui=nullptr;
}


void GUI_Player::init_sizes()
{
	if(_settings->get<Set::Player_StartInTray>())
	{
		this->setHidden(true);
	}

	else if(_settings->get<Set::Player_Fullscreen>())
	{
		this->showFullScreen();
	}

	else if(_settings->get<Set::Player_Maximized>())
	{
		this->showMaximized();
	}

	else
	{
		QPoint pos = m->initial_pos;
		QSize sz = m->initial_sz;

		this->showNormal();
		this->setGeometry(pos.x(), pos.y(), sz.width(), sz.height());

		m->initial_sz = QSize();
		m->initial_pos = QPoint();
	}
}


void GUI_Player::init_main_splitter()
{
	ui->library_widget->setVisible(_settings->get<Set::Lib_Show>());

	QByteArray splitter_state_main = _settings->get<Set::Player_SplitterState>();
	if(splitter_state_main.size() <= 1)
	{
		int w1 = width() / 3;
		int w2 = width() - w1;
		ui->splitter->setSizes({w1, w2});

		_settings->set<Set::Player_SplitterState>(ui->splitter->saveState());
	}

	else
	{
		ui->splitter->restoreState(splitter_state_main);
	}

	m->splitter_paint_event_filter = new GenericFilter(QEvent::Paint, ui->splitter);
	connect(m->splitter_paint_event_filter, &GenericFilter::sig_event, this, [=](QEvent::Type t)
	{
		Q_UNUSED(t)
		this->splitter_painted();
	});
}

void GUI_Player::init_font_change_fix()
{
	installEventFilter(m->style_changed_event_filter);

	connect(m->style_changed_event_filter, &GenericFilter::sig_event, this, [=](QEvent::Type t){
		Q_UNUSED(t)

		this->removeEventFilter(m->style_changed_event_filter);
		this->skin_changed();
		this->update();
	});
}


void GUI_Player::init_connections()
{
	PlayManager* play_manager = PlayManager::instance();
	Library::PluginHandler* lph = Library::PluginHandler::instance();
	PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();

	connect(lph, &Library::PluginHandler::sig_current_library_changed,
			this, &GUI_Player::current_library_changed);

	connect(lph, &Library::PluginHandler::sig_libraries_changed,
			this, &GUI_Player::check_library_menu_action);

	connect(play_manager, &PlayManager::sig_track_changed, this, &GUI_Player::current_track_changed);
	connect(play_manager, &PlayManager::sig_playstate_changed, this, &GUI_Player::playstate_changed);
	connect(play_manager, &PlayManager::sig_error, this, &GUI_Player::play_error);

	connect(ui->splitter, &QSplitter::splitterMoved, this, &GUI_Player::splitter_main_moved);
	connect(ui->splitterControls, &QSplitter::splitterMoved, this, &GUI_Player::splitter_controls_moved);

	connect(m->menubar, &Menubar::sig_close_clicked, this, &GUI_Player::really_close);
	connect(m->menubar, &Menubar::sig_logger_clicked, m->logger, &GUI_Logger::show);
	connect(m->menubar, &Menubar::sig_minimize_clicked, this, &GUI_Player::minimize);

	connect(pph, &PlayerPlugin::Handler::sig_plugin_added, this, &GUI_Player::plugin_added);
	connect(pph, &PlayerPlugin::Handler::sig_plugin_closed, this, &GUI_Player::plugin_closed);
	connect(pph, &PlayerPlugin::Handler::sig_plugin_action_triggered, this, &GUI_Player::plugin_action_triggered);
}


void GUI_Player::init_tray_actions()
{
	GUI_TrayIcon* tray_icon = new GUI_TrayIcon(this);

	connect(tray_icon, &GUI_TrayIcon::sig_close_clicked, this, &GUI_Player::really_close);
	connect(tray_icon, &GUI_TrayIcon::sig_show_clicked, this, &GUI_Player::raise);
	connect(tray_icon, &GUI_TrayIcon::sig_wheel_changed, m->controls, &GUI_ControlsBase::change_volume_by_tick);
	connect(tray_icon, &GUI_TrayIcon::activated, this, &GUI_Player::tray_icon_activated);

	if(_settings->get<Set::Player_ShowTrayIcon>()){
		tray_icon->show();
	}

	m->tray_icon = tray_icon;
}


void GUI_Player::tray_icon_activated(QSystemTrayIcon::ActivationReason reason)
{
	bool min_to_tray = _settings->get<Set::Player_Min2Tray>();
	if(reason != QSystemTrayIcon::Trigger){
		return;
	}

	bool invisible = (this->isMinimized() || !isVisible() || !isActiveWindow());

	if(invisible) {
		raise();
	}

	else if(min_to_tray) {
		minimize_to_tray();
	}

	else {
		showMinimized();
	}
}

void GUI_Player::current_track_changed(const MetaData& md)
{
	bool title_empty = md.title().trimmed().isEmpty();
	bool artist_empty = md.artist().trimmed().isEmpty();

	if(title_empty)
	{
		this->setWindowTitle("Sayonara " + _settings->get<Set::Player_Version>());
	}

	else if(artist_empty)
	{
		this->setWindowTitle(md.title());
	}

	else {
		this->setWindowTitle(md.artist() + " - " + md.title());
	}
}


void GUI_Player::register_preference_dialog(QAction* dialog_action)
{
	m->menubar->insert_preference_action(dialog_action);
}


void GUI_Player::playstate_changed(PlayState state)
{
	if(state == PlayState::Stopped) {
		setWindowTitle("Sayonara Player");
	}
}


void GUI_Player::play_error(const QString& message)
{
	const MetaData& md = PlayManager::instance()->current_track();
	QString err = message + "\n\n" + md.filepath();
	Message::warning(err, Lang::get(Lang::Play));
}


void GUI_Player::plugin_added(PlayerPlugin::Base* plugin)
{
	PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();
	QList<PlayerPlugin::Base*> lst = pph->all_plugins();

	QAction* action = plugin->get_action();
	QKeySequence ks("Ctrl+F" + QString::number(lst.size()));
	action->setShortcut(ks);
	action->setData(plugin->get_name());

	m->menubar->insert_player_plugin_action(action);

	if(plugin == pph->current_plugin())
	{
		plugin_opened();
	}
}


void GUI_Player::plugin_action_triggered(bool b)
{
	if(b) {
		plugin_opened();
	}

	else {
		plugin_closed();
	}
}


void GUI_Player::plugin_opened()
{
	PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();
	PlayerPlugin::Base* current_plugin = pph->current_plugin();
	ui->plugin_widget->show(current_plugin);
}


void GUI_Player::plugin_closed()
{
	ui->plugin_widget->close();
}


void GUI_Player::awa_version_finished()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	if(!awa){
		return;
	}

	if(awa->status() != AsyncWebAccess::Status::GotData) {
		awa->deleteLater();
		return;
	}

	QString new_version = QString(awa->data()).trimmed();
	QString cur_version = _settings->get<Set::Player_Version>();

	bool notify_new_version = _settings->get<Set::Player_NotifyNewVersion>();
	bool dark = Style::is_dark();

	sp_log(Log::Info, this) << "Newest Version: " << new_version;
	sp_log(Log::Info, this) << "This Version:   " << cur_version;

	QString link = Util::create_link("http://sayonara-player.com", dark);

	if(new_version > cur_version && notify_new_version) {
		Message::info(tr("A new version is available!") + "<br />" +  link);
	}

	awa->deleteLater();
}


void GUI_Player::init_controlstyle()
{
	QByteArray splitter_state = _settings->get<Set::Player_SplitterControls>();

	controlstyle_changed();

	ui->splitterControls->restoreState(splitter_state);
	splitter_controls_moved(0, 0);
}


void GUI_Player::controlstyle_changed()
{
	if(m->controls)
	{
		ui->controls->layout()->removeWidget(m->controls);
		m->controls->deleteLater();
	}

	int h = 0;
	if(_settings->get<Set::Player_ControlStyle>() == 0)
	{
		m->controls = new GUI_Controls();
	}

	else
	{
		m->controls = new GUI_ControlsNew();
		h = 350;
	}

	m->controls->init();

	ui->controls->layout()->addWidget(m->controls);
	ui->splitterControls->setSizes({h, this->height() - h});

	splitter_controls_moved(0, 0);
}


void GUI_Player::current_library_changed(const QString& name)
{
	Q_UNUSED(name)

	show_library_changed();
	check_library_menu_action();
}


void GUI_Player::show_library_changed()
{
	Library::PluginHandler* lph = Library::PluginHandler::instance();
	Library::Container* cur_lib = lph->current_library();
	if(cur_lib && cur_lib->is_initialized())
	{
		ui->library_widget->layout()->addWidget(cur_lib->widget());
		if(ui->library_widget->isVisible()){
			cur_lib->widget()->setVisible(true);
		}
	}

	show_library(_settings->get<Set::Lib_Show>(), ui->library_widget->isVisible());
}


void GUI_Player::show_library(bool is_library_visible, bool was_library_visible)
{
	if(this->isMaximized() || this->isFullScreen())
	{
		return;
	}

	if(is_library_visible == was_library_visible)
	{
		return;
	}

	ui->splitter->installEventFilter(m->splitter_paint_event_filter);

	if(is_library_visible)
	{
		int old_lib_width = _settings->get<Set::Lib_OldWidth>();
		m->new_size = QSize(this->width() + old_lib_width, this->height());

		m->is_maximizable = true;
	}

	else
	{
		int lib_width = ui->library_widget->width();
		_settings->set<Set::Lib_OldWidth>(lib_width);
		m->new_size = QSize(this->width() - lib_width, this->height());
	}

	ui->library_widget->setVisible(is_library_visible);

	check_library_menu_action();
}

void GUI_Player::splitter_painted()
{
	if(m->new_size != this->size())
	{
		this->resize(m->new_size);
	}

	else {
		ui->splitter->removeEventFilter(m->splitter_paint_event_filter);
		_settings->set<Set::Player_Size>(this->size());
	}
}

void GUI_Player::check_library_menu_action()
{
	Library::PluginHandler* lph = Library::PluginHandler::instance();

	if(!lph->current_library()) {
		m->menubar->show_library_action(false);
	}

	else {
		m->menubar->update_library_action(lph->current_library_menu(), lph->current_library()->display_name());
	}
}

void GUI_Player::splitter_main_moved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray splitter_state = ui->splitter->saveState();
	_settings->set<Set::Player_SplitterState>(splitter_state);
}

void GUI_Player::splitter_controls_moved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray splitter_state = ui->splitterControls->saveState();
	_settings->set<Set::Player_SplitterControls>(splitter_state);
}

bool GUI_Player::init_translator(const QString& four_letter, const QString& dir)
{
	QString filename = QString("sayonara_lang_%1.qm").arg(four_letter);

	QTranslator* t = new QTranslator(this);
	bool loaded = t->load(filename, dir);
	if(!loaded){
		sp_log(Log::Warning, this) << "Translator " << dir << "/" << filename << " could not be loaded";
		return false;
	}

	bool installed = QApplication::installTranslator(t);
	if(!installed){
		sp_log(Log::Warning, this) << "Translator " << dir << "/" << filename << " could not be installed";
		return false;
	}

	m->translators << t;
	return true;
}

void GUI_Player::language_changed()
{
	QString language = _settings->get<Set::Player_Language>();
	if(language == m->current_language)
	{
		return;
	}

	for(QTranslator* t : m->translators)
	{
		QApplication::removeTranslator(t);
	}

	m->translators.clear();
	m->current_language = language;

	init_translator(m->current_language, Util::share_path("translations/"));

	if(ui) {
		ui->retranslateUi(this);
	}
}


void GUI_Player::skin_changed()
{
	QString stylesheet = Style::current_style();
	this->setStyleSheet(stylesheet);
	int style = _settings->get<Set::Player_Style>();
	if(style != m->style)
	{
		m->style = style;
		Set::shout<Set::Player_Style>();
	}
}


void GUI_Player::minimize()
{
	tray_icon_activated(QSystemTrayIcon::Trigger);
}


void GUI_Player::minimize_to_tray()
{
	if(this->isHidden()){
		return;
	}

	QPoint p = this->pos();
	QSize sz = this->size();

	this->hide();

	_settings->set<Set::Player_Pos>(p);
	_settings->set<Set::Player_Size>(sz);
}


void GUI_Player::fullscreen_changed()
{
	if(_settings->get<Set::Player_Fullscreen>()) {
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

void GUI_Player::moveEvent(QMoveEvent* e)
{
	Gui::MainWindow::moveEvent(e);

	_settings->set<Set::Player_Pos>(pos());
}

void GUI_Player::resizeEvent(QResizeEvent* e)
{
	Gui::MainWindow::resizeEvent(e);

	bool is_maximized = _settings->get<Set::Player_Maximized>();
	bool is_fullscreen = _settings->get<Set::Player_Fullscreen>();

	if(is_maximized) {
		_settings->set<Set::Player_Fullscreen>(false);
	}

	if( !is_maximized && !this->isMaximized() &&
		!is_fullscreen && !this->isFullScreen())
	{
		_settings->set<Set::Player_Size>(this->size());
	}

	m->menubar->set_show_library_action_enabled(
		!(this->isMaximized() || this->isFullScreen())
	);
}


void GUI_Player::closeEvent(QCloseEvent* e)
{
	bool min_to_tray = _settings->get<Set::Player_Min2Tray>();

	_settings->set<Set::Player_Maximized>(this->isMaximized());
	_settings->set<Set::Player_Fullscreen>(this->isFullScreen());
	_settings->set<Set::Player_Pos>(this->pos());

	if(!m->shutdown_requested && min_to_tray)
	{
		minimize_to_tray();
	}

	else
	{
		m->tray_icon->hide();
		Gui::MainWindow::closeEvent(e);
		emit sig_player_closed();
	}
}
