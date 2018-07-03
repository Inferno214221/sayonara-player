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

#include "Components/PlayManager/PlayManager.h"

#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Utils.h"
#include "Utils/globals.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaData.h"

#include "GUI/Utils/Icons.h"
#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Style.h"

#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"
#include "Interfaces/LibraryInterface/LibraryContainer/LibraryContainer.h"
#include "Interfaces/PlayerPlugin/PlayerPluginHandler.h"
#include "Interfaces/PlayerPlugin/PlayerPlugin.h"
#include "Interfaces/PreferenceDialog/PreferenceDialog.h"

#include <QTranslator>
#include <QAction>
#include <QHBoxLayout>
#include <QShowEvent>
#include <QCloseEvent>
#include <QShortcut>
#include <QKeySequence>

struct GUI_Player::Private
{
	PlayerPlugin::Handler*		pph=nullptr;
	Menubar*					menubar=nullptr;
	QTranslator*				translator=nullptr;
	GUI_Logger*					logger=nullptr;
	GUI_TrayIcon*				tray_icon=nullptr;
	GUI_ControlsBase*			controls=nullptr;
	QPoint						initial_pos;
	QSize						initial_sz;
	int							style;
	bool						shutdown_requested;

	Private(QTranslator* translator) :
		translator(translator),
		shutdown_requested(false)
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

#include <QSplitterHandle>
GUI_Player::GUI_Player(QTranslator* translator, QWidget* parent) :
	Gui::MainWindow(parent),
	MessageReceiverInterface("Player Main Window")
{
	m = Pimpl::make<Private>(translator);
	ui = new Ui::GUI_Player();
	ui->setupUi(this);

	Message::register_receiver(this);

	m->controls = new GUI_ControlsNew(ui->controls);
	ui->controls->layout()->addWidget(m->controls);
	m->controls->init();

	m->menubar = new Menubar(this);
	this->setMenuBar(m->menubar);

	Library::PluginHandler* lph = Library::PluginHandler::instance();
	lph->set_library_parent(ui->library_widget);

	QString version = _settings->get<Set::Player_Version>();
	setWindowTitle(QString("Sayonara %1").arg(version));
	setWindowIcon(Gui::Util::icon("logo.png"));
	setAttribute(Qt::WA_DeleteOnClose, false);

	init_tray_actions();
	init_connections();
	init_sizes();
	init_splitter();

	if(_settings->get<Set::Player_NotifyNewVersion>())
	{
		AsyncWebAccess* awa = new AsyncWebAccess(this);
		awa->run("http://sayonara-player.com/current_version");
		connect(awa, &AsyncWebAccess::sig_finished, this, &GUI_Player::awa_version_finished);
	}

	Set::listen<Set::Player_Fullscreen>(this, &GUI_Player::fullscreen_changed, false);
	Set::listen<SetNoDB::Player_Quit>(this, &GUI_Player::really_close, false);
	Set::listen<Set::Lib_Show>(this, &GUI_Player::show_library_changed, false);
}

GUI_Player::~GUI_Player()
{
	sp_log(Log::Debug, this) << "Player closed.";
}

void GUI_Player::init_sizes()
{
	if(_settings->get<Set::Player_StartInTray>()){
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

void GUI_Player::init_splitter()
{
	if(_settings->get<Set::Lib_Show>()){
		ui->splitter->widget(1)->show();
	}
	else{
		ui->splitter->widget(1)->hide();
	}

	QByteArray splitter_state = _settings->get<Set::Player_SplitterState>();
	if(splitter_state.size() <= 1)
	{
		int w1 = width() / 3;
		int w2 = width() - w1;
		ui->splitter->setSizes({w1, w2});

		_settings->set<Set::Player_SplitterState>(splitter->saveState());
	}

	else {
		ui->splitter->restoreState(splitter_state);
	}

	if(_settings->get<Set::Lib_Show>())
	{
		library_widget->resize(ui->splitter->widget(1)->size());
	}
}


void GUI_Player::init_connections()
{
	PlayManagerPtr play_manager = PlayManager::instance();
	Library::PluginHandler* lph = Library::PluginHandler::instance();

	connect(lph, &Library::PluginHandler::sig_current_library_changed,
			this, &GUI_Player::current_library_changed);

	connect(lph, &Library::PluginHandler::sig_initialized,
			this, &GUI_Player::check_library_menu_action);

	connect(lph, &Library::PluginHandler::sig_libraries_changed,
			this, &GUI_Player::check_library_menu_action);

	connect(play_manager, &PlayManager::sig_playstate_changed, this, &GUI_Player::playstate_changed);
	connect(play_manager, &PlayManager::sig_error, this, &GUI_Player::play_error);

	connect(ui->splitter, &QSplitter::splitterMoved, this, &GUI_Player::main_splitter_moved);

	connect(m->menubar, &Menubar::sig_close_clicked, this, &GUI_Player::really_close);
	connect(m->menubar, &Menubar::sig_logger_clicked, m->logger, &GUI_Logger::show);
	connect(m->menubar, &Menubar::sig_minimize_clicked, this, &GUI_Player::minimize);
}


/** TRAY ICON **/
void GUI_Player::init_tray_actions()
{
	GUI_TrayIcon* tray_icon = new GUI_TrayIcon(this);
	tray_icon->installEventFilter(this);

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
	switch (reason)
	{
		case QSystemTrayIcon::Trigger:
			if( this->isMinimized() ||
				!this->isVisible() ||
				!this->isActiveWindow())
			{
				raise();
			}

			else
			{
				if(min_to_tray) {
					minimize_to_tray();
				}

				else{
					this->showMinimized();
				}
			}

			break;

		default:
			break;
	}
}


void GUI_Player::current_library_changed(const QString& name)
{
	Q_UNUSED(name)

	check_library_menu_action();
}

void GUI_Player::check_library_menu_action()
{
	QList<Library::Container*> libraries;

	Library::PluginHandler* lph = Library::PluginHandler::instance();
	libraries = lph->get_libraries();
	if(!lph->current_library()){
		m->menubar->show_library_action(false);
	}

	else {
		m->menubar->update_library_action(lph->current_library_menu(), lph->current_library()->display_name());
	}
}


void GUI_Player::register_preference_dialog(PreferenceDialog* dialog)
{
	m->menubar->insert_preference_action(dialog->action());
}


void GUI_Player::playstate_changed(PlayState state)
{
	switch(state)
	{
		case PlayState::Stopped:
			setWindowTitle("Sayonara Player");
			break;
		default:
			break;
	}
}


void GUI_Player::play_error(const QString& message)
{
	const MetaData& md = PlayManager::instance()->current_track();
	QString err = message + "\n\n" + md.filepath();
	Message::warning(err, Lang::get(Lang::Play));
}


void GUI_Player::register_player_plugin_handler(PlayerPlugin::Handler* pph)
{
	m->pph = pph;
	if(!pph){
		return;
	}

	QList<PlayerPlugin::Base*> lst = m->pph->get_all_plugins();

	int i=1;
	for(PlayerPlugin::Base* p : lst)
	{
		QAction* action = p->get_action();
		QKeySequence ks("Ctrl+F" + QString::number(i));
		action->setShortcut(ks);
		action->setData(p->get_name());
		m->menubar->insert_player_plugin_action(action);

		i++;
	}

	connect(m->pph, &PlayerPlugin::Handler::sig_plugin_closed, this, &GUI_Player::plugin_closed);
	connect(m->pph, &PlayerPlugin::Handler::sig_plugin_action_triggered, this, &GUI_Player::plugin_action_triggered);

	plugin_opened();
}


void GUI_Player::plugin_action_triggered(bool b)
{
	if(m->pph && b)
	{
		plugin_opened();
	}

	else{
		plugin_closed();
	}
}

void GUI_Player::plugin_opened()
{
	PlayerPlugin::Base* current_plugin=nullptr;

	if(m->pph)
	{
		current_plugin = m->pph->current_plugin();
	}

	if(current_plugin)
	{
		ui->plugin_widget->show(current_plugin);
	}

	else {
		ui->plugin_widget->hide();
	}
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

	QString new_version(awa->data());
	QString cur_version = _settings->get<Set::Player_Version>();
	bool notify_new_version = _settings->get<Set::Player_NotifyNewVersion>();
	bool dark = Style::is_dark();

	new_version = new_version.trimmed();

	sp_log(Log::Info, this) << "Newest Version: " << new_version;
	sp_log(Log::Info, this) << "This Version:   " << cur_version;

	QString link;
	LINK("http://sayonara-player.com", "http://sayonara-player.com", dark, link);

	if(new_version > cur_version && notify_new_version) {
		Message::info(tr("A new version is available!") + "<br />" +  link);
	}

	awa->deleteLater();
}


void GUI_Player::language_changed()
{
	QString language = _settings->get<Set::Player_Language>();
	m->translator->load(language, Util::share_path("translations/"));

	ui->retranslateUi(this);
}

void GUI_Player::fullscreen_changed()
{
	bool b = _settings->get<Set::Player_Fullscreen>();

	if(b){
		showFullScreen();
	}

	else {
		showNormal();
	}
}

void GUI_Player::show_library_changed()
{
	show_library(_settings->get<Set::Lib_Show>(), ui->splitter->widget(1)->isVisible());
}

void GUI_Player::show_library(bool is_library_visible, bool was_library_visible)
{
	QSize player_size = this->size();

	int library_width = ui->library_widget->width();

	if(is_library_visible)
	{
		ui->splitter->widget(1)->show();
		library_widget->show();

		library_width = _settings->get<Set::Lib_OldWidth>();
		_settings->set<Set::Lib_OldWidth>(0);

		if(library_width < 100){
			library_width = 400;
		}

		if(!was_library_visible)
		{
			player_size.setWidth( player_size.width() + library_width);
		}

		Library::PluginHandler* lph = Library::PluginHandler::instance();
		if(is_library_visible && lph)
		{
			Library::Container* container;
			container = lph->current_library();
			if(container && container->is_initialized()){
				container->widget()->resize(library_widget->size());
			}
		}
	}

	else
	{
		ui->splitter->widget(1)->hide();
		if(_settings->get<Set::Lib_OldWidth>() == 0){
			_settings->set<Set::Lib_OldWidth>(library_width);
		}

		if(was_library_visible){
			player_size.setWidth( player_size.width() - library_width);
		}
	}

	check_library_menu_action();

	if(!this->isMaximized() && !this->isFullScreen()){
		this->resize(player_size);
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


void GUI_Player::really_close()
{
	sp_log(Log::Info, this) << "closing player...";

	Gui::MainWindow::close();

	emit sig_player_closed();
}

void GUI_Player::moveEvent(QMoveEvent *e)
{
	Gui::MainWindow::moveEvent(e);

	QPoint p= this->pos();
	_settings->set<Set::Player_Pos>(p);
}


void GUI_Player::resizeEvent(QResizeEvent* e)
{
	Gui::MainWindow::resizeEvent(e);

	bool is_maximized = _settings->get<Set::Player_Maximized>();
	bool is_fullscreen = _settings->get<Set::Player_Fullscreen>();
	bool is_library_visible = _settings->get<Set::Lib_Show>();

	if(is_maximized) {
		_settings->set<Set::Player_Fullscreen>(false);
	}

	show_library(is_library_visible, is_library_visible);

	if( !is_maximized &&
		!this->isMaximized() &&
		!is_fullscreen &&
		!this->isFullScreen())
	{
		_settings->set<Set::Player_Size>(this->size());
	}

	update();
}


void GUI_Player::main_splitter_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	Library::PluginHandler* lph = Library::PluginHandler::instance();
	if(lph)
	{
		Library::Container* container;
		container = lph->current_library();
		if(container){
			container->widget()->resize(ui->library_widget->size());
		}
	}

	QByteArray splitter_state = ui->splitter->saveState();
	_settings->set<Set::Player_SplitterState>(splitter_state);
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

	else {
		m->tray_icon->hide();
		Gui::MainWindow::closeEvent(e);
		emit sig_player_closed();
	}
}

void GUI_Player::request_shutdown()
{
	m->shutdown_requested = true;
}

void GUI_Player::raise()
{
	Gui::MainWindow::raise();

	QPoint p = _settings->get<Set::Player_Pos>();
	QSize sz = _settings->get<Set::Player_Size>();

	if(!m->initial_pos.isNull())
	{
		p = m->initial_pos;
		sz = m->initial_sz;

		m->initial_pos = QPoint();
		m->initial_sz = QSize();
	}

	this->setGeometry(p.x(), p.y(), sz.width(), sz.height());
	this->menuBar()->show();

	show_library_changed();
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
