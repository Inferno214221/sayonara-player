/* LocalLibraryMenu.cpp */

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

#include "LocalLibraryMenu.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Library/GUI_EditLibrary.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

using namespace Library;

struct LocalLibraryMenu::Private
{
	QString name;
	QString path;
	bool	initialized;

	QAction* reload_library_action=nullptr;
	QAction* import_file_action=nullptr;
	QAction* import_folder_action=nullptr;
	QAction* info_action=nullptr;
	QAction* edit_action=nullptr;
	QAction* livesearch_action=nullptr;
	QAction* show_album_artists_action=nullptr;
	QAction* show_album_cover_view=nullptr;

	bool has_preference_action;

	Private(const QString& name, const QString& path) :
		name(name),
		path(path),
		initialized(false),
		has_preference_action(false)
	{}
};

LocalLibraryMenu::LocalLibraryMenu(const QString& name, const QString& path, QWidget* parent) :
	WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(name, path);

	init_menu();
}

LocalLibraryMenu::~LocalLibraryMenu() {}

void LocalLibraryMenu::refresh_name(const QString& name)
{
	m->name = name;
}

void LocalLibraryMenu::refresh_path(const QString& path)
{
	m->path = path;
}

void LocalLibraryMenu::set_show_album_covers_checked(bool checked)
{
	if(!m->initialized){
		return;
	}

	m->show_album_cover_view->setChecked(checked);
}

void LocalLibraryMenu::set_library_busy(bool b)
{
	if(!m->initialized){
		return;
	}

	m->reload_library_action->setEnabled(!b);
	m->edit_action->setEnabled(!b);
	m->import_file_action->setEnabled(!b);
	m->import_folder_action->setEnabled(!b);
}

void LocalLibraryMenu::add_preference_action(PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->has_preference_action){
		actions << this->addSeparator();
	}

	actions << action;

	this->addActions(actions);
	m->has_preference_action = true;
}


void LocalLibraryMenu::init_menu()
{
	if(m->initialized)
	{
		return;
	}

	m->reload_library_action = new QAction(this);
	m->import_file_action = new QAction(this);
	m->import_folder_action = new QAction(this);
	m->info_action = new QAction(this);
	m->edit_action = new QAction(this);

	m->livesearch_action = new QAction(this);
	m->livesearch_action->setCheckable(true);
	m->livesearch_action->setChecked(GetSetting(Set::Lib_LiveSearch));

	m->show_album_artists_action = new QAction(this);
	m->show_album_artists_action->setCheckable(true);
	m->show_album_artists_action->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));

	m->show_album_cover_view = new QAction(this);
	m->show_album_cover_view->setCheckable(true);
	m->show_album_cover_view->setChecked(GetSetting(Set::Lib_ShowAlbumCovers));

	connect(m->reload_library_action, &QAction::triggered, this, &LocalLibraryMenu::sig_reload_library);
	connect(m->import_file_action, &QAction::triggered, this, &LocalLibraryMenu::sig_import_file);
	connect(m->import_folder_action, &QAction::triggered, this, &LocalLibraryMenu::sig_import_folder);
	connect(m->info_action, &QAction::triggered, this, &LocalLibraryMenu::sig_info);
	connect(m->edit_action, &QAction::triggered, this, &LocalLibraryMenu::edit_clicked);
	connect(m->livesearch_action, &QAction::triggered, this, &LocalLibraryMenu::realtime_search_triggered);
	connect(m->show_album_artists_action, &QAction::triggered, this, &LocalLibraryMenu::show_album_artists_triggered);
	connect(m->show_album_cover_view, &QAction::triggered, this, &LocalLibraryMenu::show_album_covers_triggered);

	QList<QAction*> actions;
	actions <<
		m->info_action <<
		m->edit_action <<
		this->addSeparator() <<
		m->import_file_action <<
		m->import_folder_action <<
		m->reload_library_action <<
		this->addSeparator() <<
		m->livesearch_action <<
		m->show_album_cover_view <<
		m->show_album_artists_action;

	this->addActions(actions);
	this->add_preference_action(new LibraryPreferenceAction(this));

	m->initialized = true;

	ListenSetting(Set::Lib_ShowAlbumCovers, LocalLibraryMenu::show_album_covers_changed);
	ListenSetting(Set::Lib_ShowAlbumArtists, LocalLibraryMenu::show_album_artists_changed);
	ListenSetting(Set::Lib_LiveSearch, LocalLibraryMenu::realtime_search_changed);

	shortcut_changed(ShortcutIdentifier::Invalid);
	language_changed();
	skin_changed();
}

void LocalLibraryMenu::language_changed()
{
	if(!m->initialized){
		return;
	}

	m->info_action->setText(Lang::get(Lang::Library) + ": " + Lang::get(Lang::Info));
	m->edit_action->setText(Lang::get(Lang::Library) + ": " + Lang::get(Lang::Edit));

	m->import_file_action->setText(Lang::get(Lang::ImportFiles));
	m->import_folder_action->setText(Lang::get(Lang::ImportDir));
	m->reload_library_action->setText(Lang::get(Lang::ReloadLibrary));

	m->livesearch_action->setText(Lang::get(Lang::LiveSearch));
	m->show_album_artists_action->setText(Lang::get(Lang::ShowAlbumArtists));
	m->show_album_cover_view->setText(Lang::get(Lang::ShowCovers));
}

void LocalLibraryMenu::skin_changed()
{
	if(!m->initialized){
		return;
	}

	using namespace Gui;
	m->reload_library_action->setIcon(Icons::icon(Icons::Refresh));
	m->import_file_action->setIcon(Icons::icon(Icons::Open));
	m->import_folder_action->setIcon(Icons::icon(Icons::Open));
	m->info_action->setIcon(Icons::icon(Icons::Info));
	m->edit_action->setIcon(Icons::icon(Icons::Edit));
}

void LocalLibraryMenu::shortcut_changed(ShortcutIdentifier identifier)
{
	Q_UNUSED(identifier)

	if(!m->initialized){
		return;
	}

	ShortcutHandler* sch = ShortcutHandler::instance();

	m->show_album_artists_action->setShortcutContext(Qt::WidgetShortcut);
	m->show_album_artists_action->setShortcut(sch->shortcut(ShortcutIdentifier::AlbumArtists).sequence());
	m->show_album_cover_view->setShortcutContext(Qt::WidgetShortcut);
	m->show_album_cover_view->setShortcut(sch->shortcut(ShortcutIdentifier::CoverView).sequence());
	m->reload_library_action->setShortcutContext(Qt::WidgetShortcut);
	m->reload_library_action->setShortcut(sch->shortcut(ShortcutIdentifier::ReloadLibrary).sequence());
}

void LocalLibraryMenu::realtime_search_triggered(bool b)
{
	SetSetting(Set::Lib_LiveSearch, b);
}


void LocalLibraryMenu::realtime_search_changed()
{
	if(!m->initialized){
		return;
	}

	m->livesearch_action->setChecked(GetSetting(Set::Lib_LiveSearch) );
}

void LocalLibraryMenu::edit_clicked()
{
	if(!m->initialized){
		return;
	}

	GUI_EditLibrary* edit_dialog = new GUI_EditLibrary(m->name, m->path, this);

	connect(edit_dialog, &GUI_EditLibrary::sig_accepted, this, &LocalLibraryMenu::edit_accepted);

	edit_dialog->show();
}

void LocalLibraryMenu::edit_accepted()
{
	GUI_EditLibrary* edit_dialog = static_cast<GUI_EditLibrary*>(sender());
	QString name = edit_dialog->name();
	QString path = edit_dialog->path();

	if(name.isEmpty() || path.isEmpty())
	{
		return;
	}

	if(edit_dialog->has_name_changed()){
		emit sig_name_changed(name);
	}

	if(edit_dialog->has_path_changed()){
		emit sig_path_changed(path);
	}
}

void LocalLibraryMenu::show_album_covers_triggered(bool b)
{
	SetSetting(Set::Lib_ShowAlbumCovers, b);
}

void LocalLibraryMenu::show_album_covers_changed()
{
	m->show_album_cover_view->setChecked(GetSetting(Set::Lib_ShowAlbumCovers));
}

void LocalLibraryMenu::show_album_artists_triggered(bool b)
{
	SetSetting(Set::Lib_ShowAlbumArtists, b);
}

void LocalLibraryMenu::show_album_artists_changed()
{
	m->show_album_artists_action->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));
}

