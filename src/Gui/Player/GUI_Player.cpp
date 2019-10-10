/* GUI_Player.cpp */

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
#include "Utils/globals.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaData.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/EventFilter.h"

#include <QAction>
#include <QKeySequence>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QTimer>

struct GUI_Player::Private
{
	Menubar*					menubar=nullptr;
	GUI_Logger*					logger=nullptr;
	GUI_TrayIcon*				tray_icon=nullptr;
	GUI_ControlsBase*			controls=nullptr;

	// hack for resizing the player after library show/hide:
	// star resizing after splitter has finished painting
	Gui::GenericFilter*			splitter_paint_event_filter=nullptr;

	QPoint						initial_pos;
	QSize						initial_sz;
	QSize						new_size;

	int							style;
	bool						shutdown_requested;
	bool						ctrl_pressed;

	Private() :
		shutdown_requested(false),
		ctrl_pressed(false)
	{
		logger = new GUI_Logger();

		initial_pos = GetSetting(Set::Player_Pos);
		initial_sz = GetSetting(Set::Player_Size);
		style = GetSetting(Set::Player_Style);
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

	language_changed();

	ui = new Ui::GUI_Player();
	ui->setupUi(this);
	ui->retranslateUi(this);

	ui->plugin_widget->setVisible(false);

	Message::register_receiver(this);

	m->menubar = new Menubar(this);
	setMenuBar(m->menubar);

	QString version = GetSetting(Set::Player_Version);
	setWindowTitle(QString("Sayonara %1").arg(version));
	setWindowIcon(Gui::Util::icon("logo.png", Gui::Util::NoTheme));
	setAttribute(Qt::WA_DeleteOnClose, false);

	init_font_change_fix();
	init_sizes();
	init_main_splitter();
	init_controlstyle();
	init_connections();
	init_tray_actions();

	current_track_changed(PlayManager::instance()->current_track());

	if(GetSetting(Set::Player_NotifyNewVersion))
	{
		VersionChecker* vc = new VersionChecker(this);
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


void GUI_Player::init_sizes()
{
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
	ui->library_widget->setVisible(GetSetting(Set::Lib_Show));

	QByteArray splitter_state_main = GetSetting(Set::Player_SplitterState);
	if(splitter_state_main.size() <= 1)
	{
		int w1 = width() / 3;
		int w2 = width() - w1;
		ui->splitter->setSizes({w1, w2});

		SetSetting(Set::Player_SplitterState, ui->splitter->saveState());
	}

	else
	{
		ui->splitter->restoreState(splitter_state_main);
	}

	m->splitter_paint_event_filter = new Gui::GenericFilter(QEvent::Paint, ui->splitter);
	connect(m->splitter_paint_event_filter, &Gui::GenericFilter::sig_event, this, [=](QEvent::Type t)
	{
		Q_UNUSED(t)
		this->splitter_painted();
	});
}

void GUI_Player::init_font_change_fix()
{
	auto* filter = new Gui::GenericFilter(
		QList<QEvent::Type>{QEvent::Paint},
		this
	);

	installEventFilter(filter);

	connect(filter, &Gui::GenericFilter::sig_event, this, [=](QEvent::Type t){
		Q_UNUSED(t)

		this->removeEventFilter(filter);
		this->skin_changed();
		this->update();
	});
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
	connect(m->menubar, &Menubar::sig_logger_clicked, m->logger, &GUI_Logger::show);
	connect(m->menubar, &Menubar::sig_minimize_clicked, this, &GUI_Player::minimize);

	auto* pph = PlayerPlugin::Handler::instance();
	connect(pph, &PlayerPlugin::Handler::sig_plugin_added, this, &GUI_Player::plugin_added);
	connect(pph, &PlayerPlugin::Handler::sig_plugin_closed, ui->plugin_widget, &QWidget::close);
	connect(pph, &PlayerPlugin::Handler::sig_plugin_action_triggered, this, &GUI_Player::plugin_action_triggered);

	auto* dbl_click_filter = new Gui::GenericFilter(QEvent::MouseButtonDblClick, ui->splitterControls);

	ui->splitterControls->handle(1)->installEventFilter(dbl_click_filter);

	connect(dbl_click_filter, &Gui::GenericFilter::sig_event, this, [=](QEvent::Type)
	{
		this->check_control_splitter(true);
	});
}


void GUI_Player::init_tray_actions()
{
	GUI_TrayIcon* tray_icon = new GUI_TrayIcon(this);

	connect(tray_icon, &GUI_TrayIcon::sig_close_clicked, this, &GUI_Player::really_close);
	connect(tray_icon, &GUI_TrayIcon::sig_show_clicked, this, &GUI_Player::raise);
	connect(tray_icon, &GUI_TrayIcon::sig_wheel_changed, m->controls, &GUI_ControlsBase::change_volume_by_tick);
	connect(tray_icon, &GUI_TrayIcon::activated, this, &GUI_Player::tray_icon_activated);

	if(GetSetting(Set::Player_ShowTrayIcon)){
		tray_icon->show();
	}

	m->tray_icon = tray_icon;
}


void GUI_Player::tray_icon_activated(QSystemTrayIcon::ActivationReason reason)
{
	bool min_to_tray = GetSetting(Set::Player_Min2Tray);
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
		this->setWindowTitle("Sayonara " + GetSetting(Set::Player_Version));
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
	QKeySequence ks("Shift+F" + QString::number(lst.size()));
	action->setShortcut(ks);
	action->setData(plugin->get_name());

	m->menubar->insert_player_plugin_action(action);

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


void GUI_Player::init_controlstyle()
{
	QByteArray splitter_state = GetSetting(Set::Player_SplitterControls);

	controlstyle_changed();

	ui->splitterControls->restoreState(splitter_state);
	ui->controls->setFocus();
}


void GUI_Player::controlstyle_changed()
{
	if(m->controls)
	{
		ui->controls->layout()->removeWidget(m->controls);
		m->controls->deleteLater();
	}

	int h = 0;
	if(GetSetting(Set::Player_ControlStyle) == 0)
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

	QByteArray splitter_state = ui->splitterControls->saveState();
	SetSetting(Set::Player_SplitterControls, splitter_state);

	splitter_main_moved(0, 0);
	splitter_controls_moved(0, 0);
}


void GUI_Player::current_library_changed()
{
	show_library_changed();
}


void GUI_Player::show_library_changed()
{
	QLayout* layout = ui->library_widget->layout();
	if(layout)
	{
		for(int i=0; i<layout->count(); i++)
		{
			QLayoutItem* item = layout->itemAt(i);
			item->widget()->hide();
			layout->removeItem(item);
		}
	}

	auto* lph = Library::PluginHandler::instance();
	Library::Container* current_library = lph->current_library();
	if(current_library)
	{
		QWidget* cur_library_widget = current_library->widget();
		if(cur_library_widget)
		{
			ui->library_widget->layout()->addWidget(cur_library_widget);
			if(ui->library_widget->isVisible())
			{
				cur_library_widget->setVisible(true);
			}
		}

		show_library(GetSetting(Set::Lib_Show), ui->library_widget->isVisible());
	}
}


void GUI_Player::show_library(bool is_library_visible, bool was_library_visible)
{
	m->menubar->show_library_menu(is_library_visible);

	if(isMaximized() || isFullScreen() || (is_library_visible == was_library_visible))
	{
		return;
	}

	ui->splitter->installEventFilter(m->splitter_paint_event_filter);

	if(is_library_visible)
	{
		int old_lib_width = GetSetting(Set::Lib_OldWidth);
		m->new_size = QSize(this->width() + old_lib_width, this->height());
	}

	else
	{
		int lib_width = ui->library_widget->width();
		SetSetting(Set::Lib_OldWidth, lib_width);
		m->new_size = QSize(this->width() - lib_width, this->height());
	}

	ui->library_widget->setVisible(is_library_visible);
}

void GUI_Player::splitter_painted()
{
	if(m->new_size != this->size())
	{
		this->resize(m->new_size);
	}

	else {
		ui->splitter->removeEventFilter(m->splitter_paint_event_filter);
		SetSetting(Set::Player_Size, this->size());
	}
}

void GUI_Player::check_control_splitter(bool force)
{
	if(m->controls->is_extern_resize_allowed())
	{
		int difference = m->controls->btn_cover()->vertical_padding();
		if(difference > 0 || force)
		{
			auto sizes = ui->splitterControls->sizes();
				sizes[0] -= difference;
				sizes[1] += difference;

			ui->splitterControls->setSizes(sizes);
		}
	}
}

void GUI_Player::splitter_main_moved(int pos, int idx)
{
	Q_UNUSED(pos); Q_UNUSED(idx);

	check_control_splitter(m->ctrl_pressed);

	QByteArray splitter_state = ui->splitter->saveState();
	SetSetting(Set::Player_SplitterState, splitter_state);
}

void GUI_Player::splitter_controls_moved(int pos, int idx)
{
	Q_UNUSED(pos); Q_UNUSED(idx);

	if(m->controls->is_extern_resize_allowed() == false)
	{
		QByteArray state = GetSetting(Set::Player_SplitterControls);
		ui->splitterControls->restoreState(state);

		return;
	}

	check_control_splitter(false);

	QByteArray splitter_state = ui->splitterControls->saveState();
	SetSetting(Set::Player_SplitterControls, splitter_state);
}

void GUI_Player::language_changed()
{
	QString language = GetSetting(Set::Player_Language);
	Translator::instance()->change_language(this, language);

	if(ui) {
		ui->retranslateUi(this);
	}
}

void GUI_Player::skin_changed()
{
	int style = GetSetting(Set::Player_Style);
	if(style != m->style)
	{
		m->style = style;
		Set::shout<SetNoDB::Player_MetaStyle>();
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

	SetSetting(Set::Player_Pos, p);
	SetSetting(Set::Player_Size, sz);
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

void GUI_Player::moveEvent(QMoveEvent* e)
{
	Gui::MainWindow::moveEvent(e);
	SetSetting(Set::Player_Pos, pos());
}

void GUI_Player::keyPressEvent(QKeyEvent* e)
{
	m->ctrl_pressed = ((e->modifiers() & Qt::ControlModifier) != 0);
	Gui::MainWindow::keyPressEvent(e);
}

void GUI_Player::keyReleaseEvent(QKeyEvent* e)
{
	m->ctrl_pressed = ((e->modifiers() & Qt::ControlModifier) != 0);
	Gui::MainWindow::keyReleaseEvent(e);
}


void GUI_Player::resizeEvent(QResizeEvent* e)
{
	Gui::MainWindow::resizeEvent(e);

	bool is_maximized = GetSetting(Set::Player_Maximized);
	bool is_fullscreen = GetSetting(Set::Player_Fullscreen);

	if(is_maximized) {
		SetSetting(Set::Player_Fullscreen, false);
	}

	if( !is_maximized && !this->isMaximized() &&
		!is_fullscreen && !this->isFullScreen())
	{
		SetSetting(Set::Player_Size, this->size());
	}

	if(m->controls)
	{
		bool b = m->ctrl_pressed;
		m->ctrl_pressed = true;
		this->splitter_main_moved(0, 0);
		m->ctrl_pressed = b;
	}
}


void GUI_Player::closeEvent(QCloseEvent* e)
{
	bool min_to_tray = GetSetting(Set::Player_Min2Tray);

	SetSetting(Set::Player_Maximized, this->isMaximized());
	SetSetting(Set::Player_Fullscreen, this->isFullScreen());
	SetSetting(Set::Player_Pos, this->pos());

	if(!m->shutdown_requested && min_to_tray && !GetSetting(SetNoDB::Player_Quit))
	{
		if(GetSetting(Set::Player_514Fix))
		{
			e->ignore();
			QTimer::singleShot(10, this, &GUI_Player::minimize_to_tray);
		}

		else
		{
			minimize_to_tray();
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
