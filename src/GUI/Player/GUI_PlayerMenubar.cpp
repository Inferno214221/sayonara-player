/*
 * GUI_PlayerMenubar.cpp
 *
 *  Created on: 10.10.2012
 *      Author: Lucio Carreras
 */

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

#include "GUI_PlayerMenubar.h"

#include "GUI/ShutdownDialog/GUI_Shutdown.h"
#include "Components/Playlist/PlaylistHandler.h"

#include "GUI/Utils/Shortcuts/ShortcutHandler.h"
#include "GUI/Utils/Shortcuts/Shortcut.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Style.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Language.h"
#include "Utils/Message/Message.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QDateTime>

struct Menubar::Private
{
	QMenu*			menu_file=nullptr;
	QMenu*			menu_view=nullptr;
	QMenu*			menu_help=nullptr;

	QAction*		menu_file_action=nullptr;
	QAction*		menu_view_action=nullptr;
	QAction*		menu_help_action=nullptr;

	//file
	QAction*		action_open_file=nullptr;
	QAction*		action_open_dir=nullptr;
	QAction*		sep_after_open=nullptr; // after open file and open dir
	QAction*		action_preferences=nullptr;
	QAction*		sep_after_preferences=nullptr;
	QAction*		action_shutdown=nullptr;
	QAction*		action_close=nullptr;

	// view
	QAction*		action_view_library=nullptr;
	QAction*		sep_after_view_library=nullptr;
	QList<QAction*> actions_plugins;
	QAction*		sep_after_plugins=nullptr;
	QAction*		action_logger=nullptr;
	QAction*		action_dark=nullptr;
	QAction*		action_big_cover=nullptr;
	QAction*		action_fullscreen=nullptr;

	// help
	QAction*		action_help=nullptr;
	QAction*		action_about=nullptr;

	QMenu*			current_library_menu=nullptr;
	QAction*		current_library_menu_action=nullptr;

	QMessageBox*	about_box=nullptr;

	const QString SC_ID_VIEW_LIBRARY=QString("view_library");

	Private(Menubar* menubar)
	{
		menu_file = new QMenu(menubar);
		menu_view = new QMenu(menubar);
		menu_help = new QMenu(menubar);

		menu_file_action = menubar->insertMenu(nullptr, menu_file);
		menu_view_action = menubar->insertMenu(nullptr, menu_view);
		menu_help_action = menubar->insertMenu(nullptr, menu_help);

		// file
		action_open_dir = new QAction(menu_file);
		action_open_file = new QAction(menu_file);
		sep_after_open = menu_file->addSeparator();
		sep_after_preferences = menu_file->addSeparator();
		action_shutdown = new QAction(menu_file);
		action_close = new QAction(menu_file);

		menu_file->insertActions(nullptr,
		{
			action_open_file, action_open_dir, sep_after_open, sep_after_preferences, action_shutdown, action_close
		});

		// view
		action_view_library = new QAction(menu_view);
		action_view_library->setCheckable(true);
		sep_after_view_library = menu_view->addSeparator();
		sep_after_plugins = menu_view->addSeparator();
		action_logger = new QAction(menu_view);
		action_dark = new QAction(menu_view);
		action_dark->setCheckable(true);
		action_big_cover = new QAction(menu_view);
		action_big_cover->setCheckable(true);
		action_fullscreen = new QAction(menu_view);
		action_fullscreen->setCheckable(true);

		menu_view->insertActions(nullptr,
		{
			action_view_library,
			sep_after_view_library,
			sep_after_plugins,
			action_logger,
			action_big_cover,
			action_dark,
			action_fullscreen
		});

		//help
		action_help = new QAction(menu_help);
		action_about = new QAction(menu_help);

		menu_help->insertActions(nullptr,
		{
			action_help, action_about
		});
	}
};

Menubar::Menubar(QWidget* parent) :
	Gui::WidgetTemplate<QMenuBar>(parent),
	ShortcutWidget()
{
	m = Pimpl::make<Private>(this);

	m->action_view_library->setChecked(GetSetting(Set::Lib_Show));
	m->action_view_library->setText(Lang::get(Lang::Library));
	m->action_view_library->setShortcut(QKeySequence("Ctrl+L"));

	m->action_big_cover->setShortcut(QKeySequence("F9"));
	ListenSetting(Set::Player_ControlStyle, Menubar::style_changed);

	m->action_dark->setShortcut(QKeySequence("F10"));
	ListenSetting(Set::Player_ControlStyle, Menubar::style_changed);

	m->action_fullscreen->setShortcut(QKeySequence("F11"));
	m->action_fullscreen->setChecked(GetSetting(Set::Player_Fullscreen));

#ifdef WITH_SHUTDOWN
	m->action_shutdown->setVisible(true);
#else
	m->action_shutdown->setVisible(false);
#endif

	init_connections();
	language_changed();
	skin_changed();
	style_changed();
}

Menubar::~Menubar() {}


void Menubar::insert_player_plugin_action(QAction* action)
{
	m->menu_view->insertAction(m->sep_after_plugins, action);
}

void Menubar::insert_preference_action(QAction* action)
{
	m->menu_file->insertAction(m->sep_after_preferences, action);
}

QAction* Menubar::update_library_action(QMenu* new_library_menu, const QString& name)
{
	if(m->current_library_menu_action){
		this->removeAction(m->current_library_menu_action);
	}

	if(!new_library_menu)
	{
		m->current_library_menu = nullptr;
		m->current_library_menu_action = nullptr;

		return nullptr;
	}

	m->current_library_menu = new_library_menu;

	m->current_library_menu_action = this->insertMenu(m->menu_help_action, new_library_menu);
	m->current_library_menu_action->setText(name);

	bool library_visible = GetSetting(Set::Lib_Show);
	m->current_library_menu_action->setVisible(library_visible);

	return m->current_library_menu_action;
}

void Menubar::show_library_action(bool visible)
{
	if(m->current_library_menu_action){
		m->current_library_menu_action->setVisible(visible);
	}
}

void Menubar::set_show_library_action_enabled(bool b)
{
	m->action_view_library->setEnabled(b);
}

void Menubar::init_connections()
{
	// file
	connect(m->action_open_file, &QAction::triggered, this, &Menubar::open_files_clicked);
	connect(m->action_open_dir, &QAction::triggered, this, &Menubar::open_dir_clicked);
	connect(m->action_close, &QAction::triggered, this, &Menubar::sig_close_clicked);
	connect(m->action_shutdown, &QAction::triggered, this, &Menubar::shutdown_clicked);

	// view
	connect(m->action_view_library, &QAction::toggled, this, &Menubar::show_library_toggled);
	connect(m->action_dark, &QAction::toggled, this, &Menubar::skin_toggled);
	connect(m->action_big_cover, &QAction::toggled, this, &Menubar::big_cover_toggled);
	connect(m->action_fullscreen, &QAction::toggled, this, &Menubar::show_fullscreen_toggled);
	connect(m->action_logger, &QAction::triggered, this, &Menubar::sig_logger_clicked);

	// about
	connect(m->action_about, &QAction::triggered, this, &Menubar::about_clicked);
	connect(m->action_help, &QAction::triggered, this, &Menubar::help_clicked);

	// shortcuts
	ShortcutHandler* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::Quit).connect(this, this, SLOT(close_clicked()));
	sch->shortcut(ShortcutIdentifier::Minimize).connect(this, this, SLOT(minimize_clicked()));

	shortcut_changed(ShortcutIdentifier::Invalid);

	connect(sch, &ShortcutHandler::sig_shortcut_changed, this, &Menubar::shortcut_changed);
}

void Menubar::language_changed()
{
	m->menu_file->setTitle(Lang::get(Lang::File));
	m->menu_view->setTitle(tr("View"));
	m->menu_help->setTitle(tr("Help"));

	m->action_open_file->setText(Lang::get(Lang::OpenFile).triplePt());
	m->action_open_dir->setText(Lang::get(Lang::OpenDir).triplePt());
	m->action_shutdown->setText(Lang::get(Lang::Shutdown).triplePt());

	m->action_close->setText(Lang::get(Lang::Quit));

	m->action_view_library->setText(Lang::get(Lang::ShowLibrary));
	m->action_logger->setText(Lang::get(Lang::Logger));
	m->action_dark->setText(Lang::get(Lang::DarkMode));
	m->action_big_cover->setText(tr("Show large cover"));
	m->action_fullscreen->setText(tr("Fullscreen"));

	m->action_help->setText(tr("Help"));
	m->action_about->setText(Lang::get(Lang::About).triplePt());
}

void Menubar::skin_changed()
{
	QString stylesheet = Style::current_style();
	this->setStyleSheet(stylesheet);

	using namespace Gui;
	m->action_open_file->setIcon(Icons::icon(Icons::Open));
	m->action_open_dir->setIcon(Icons::icon(Icons::Open));
	m->action_close->setIcon(Icons::icon(Icons::Exit));
}

void Menubar::open_dir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this,
			Lang::get(Lang::OpenDir),
			QDir::homePath(),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (dir.isEmpty()){
		return;
	}

	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->create_playlist(dir);
}


void Menubar::open_files_clicked()
{
	QStringList filetypes;

	filetypes << Util::soundfile_extensions();
	filetypes << Util::playlist_extensions();

	QString filetypes_str = tr("Media files") + " (" + filetypes.join(" ") + ")";

	QStringList list =
			QFileDialog::getOpenFileNames(
					this,
					tr("Open Media files"),
					QDir::homePath(),
					filetypes_str);

	if(list.isEmpty()){
		return;
	}

	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->create_playlist(list);
}

void Menubar::shutdown_clicked()
{
	GUI_Shutdown* gui = new GUI_Shutdown(this);
	gui->exec();
}

void Menubar::close_clicked()
{
	emit sig_close_clicked();
}

void Menubar::minimize_clicked()
{
	emit sig_minimize_clicked();
}

void Menubar::style_changed()
{
	m->action_big_cover->setChecked(GetSetting(Set::Player_ControlStyle) == 1);
	m->action_dark->setChecked(Style::is_dark());
}

void Menubar::skin_toggled(bool b)
{
	Style::set_dark(b);
}

void Menubar::big_cover_toggled(bool b)
{
	SetSetting(Set::Player_ControlStyle, (b==true) ? 1 : 0);
}


void Menubar::show_library_toggled(bool b)
{
	m->action_view_library->setChecked(b);
	SetSetting(Set::Lib_Show, b);
}


void Menubar::show_fullscreen_toggled(bool b)
{
	// may happened because of F11 too
	m->action_fullscreen->setChecked(b);
	SetSetting(Set::Player_Fullscreen, b);
}


void Menubar::help_clicked()
{
	QStringList text
	{
		tr("Please visit the forum at"),
		Util::create_link("https://sayonara-player.com/forum", Style::is_dark()),
		"",
		tr("FAQ") + ": ",
		Util::create_link("http://sayonara-player.com/faq.php", Style::is_dark()),
	};

	Message::info(text.join("<br/>"));
}

// private slot
void Menubar::about_clicked()
{
	QString version = GetSetting(Set::Player_Version);

	if(!m->about_box)
	{
		m->about_box = new QMessageBox(this);
		m->about_box->setParent(this);
		m->about_box->setIconPixmap(Gui::Util::pixmap("logo.png", QSize(150, 150), true));
		m->about_box->setWindowFlags(Qt::Dialog);
		m->about_box->setModal(true);
		m->about_box->setStandardButtons(QMessageBox::Ok);
		m->about_box->setWindowTitle(tr("About Sayonara"));

		m->about_box->setText(QStringList
		({
			"<b><font size=\"+2\">",
			"Sayonara Player " + version,
			"</font></b>"
		}).join(""));

		m->about_box->setInformativeText( QStringList
		({
			tr("Written by Lucio Carreras"),
			"",
			tr("License") + ": GPLv3",
			"Copyright 2011-" + QString::number(QDateTime::currentDateTime().date().year()),
			Util::create_link("http://sayonara-player.com", Style::is_dark()),
			"",
			"<b>" + tr("Donate") + "</b>",
			Util::create_link("http://sayonara-player.com/donations.php", Style::is_dark()),
			"",
			tr("Special tanks to all the brave translators") + "!"
		}).join("<br/>"));
	}

	m->about_box->exec();
}

void Menubar::shortcut_changed(ShortcutIdentifier identifier)
{
	Q_UNUSED(identifier)

	ShortcutHandler* sch = ShortcutHandler::instance();
	Shortcut sc = sch->shortcut(ShortcutIdentifier::ViewLibrary);
	m->action_view_library->setShortcut(sc.sequence());
}
