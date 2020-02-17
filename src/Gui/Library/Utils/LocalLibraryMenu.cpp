/* LocalLibraryMenu.cpp */

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

#include "LocalLibraryMenu.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Library/GUI_EditLibrary.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

using Library::LocalLibraryMenu;

struct LocalLibraryMenu::Private
{
	QString name, path;

	QAction* reloadLibraryAction=nullptr;
	QAction* importFileAction=nullptr;
	QAction* importFolderAction=nullptr;
	QAction* infoAction=nullptr;
	QAction* editAction=nullptr;
	QAction* livesearchAction=nullptr;
	QAction* showAlbumArtistsAction=nullptr;

	bool hasPreferenceAction;
	bool isInitialized;
	bool isLibraryEmpty;

	Private(const QString& name, const QString& path) :
		name(name),
		path(path),
		hasPreferenceAction(false),
		isInitialized(false),
		isLibraryEmpty(false)
	{}
};

LocalLibraryMenu::LocalLibraryMenu(const QString& name, const QString& path, QWidget* parent) :
	WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(name, path);

	initMenu();
}

LocalLibraryMenu::~LocalLibraryMenu() = default;

void LocalLibraryMenu::refreshName(const QString& name)
{
	m->name = name;
}

void LocalLibraryMenu::refreshPath(const QString& path)
{
	m->path = path;
}

void LocalLibraryMenu::setLibraryBusy(bool b)
{
	if(!m->isInitialized){
		return;
	}

	m->reloadLibraryAction->setEnabled(!b);
	m->editAction->setEnabled(!b);
	m->importFileAction->setEnabled(!b);
	m->importFolderAction->setEnabled(!b);
}

void LocalLibraryMenu::setLibraryEmpty(bool b)
{
	m->isLibraryEmpty = b;
	languageChanged();
}

void LocalLibraryMenu::addPreferenceAction(Gui::PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->hasPreferenceAction){
		actions << this->addSeparator();
	}

	actions << action;

	this->addActions(actions);
	m->hasPreferenceAction = true;
}


void LocalLibraryMenu::initMenu()
{
	if(m->isInitialized)
	{
		return;
	}

	m->reloadLibraryAction = new QAction(this);
	m->importFileAction = new QAction(this);
	m->importFolderAction = new QAction(this);
	m->infoAction = new QAction(this);
	m->editAction = new QAction(this);

	m->livesearchAction = new QAction(this);
	m->livesearchAction->setCheckable(true);
	m->livesearchAction->setChecked(GetSetting(Set::Lib_LiveSearch));

	m->showAlbumArtistsAction = new QAction(this);
	m->showAlbumArtistsAction->setCheckable(true);
	m->showAlbumArtistsAction->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));

	connect(m->reloadLibraryAction, &QAction::triggered, this, &LocalLibraryMenu::sigReloadLibrary);
	connect(m->importFileAction, &QAction::triggered, this, &LocalLibraryMenu::sigImportFile);
	connect(m->importFolderAction, &QAction::triggered, this, &LocalLibraryMenu::sigImportFolder);
	connect(m->infoAction, &QAction::triggered, this, &LocalLibraryMenu::sigInfo);
	connect(m->editAction, &QAction::triggered, this, &LocalLibraryMenu::editClicked);
	connect(m->livesearchAction, &QAction::triggered, this, &LocalLibraryMenu::setLiveSearchEnabled);
	connect(m->showAlbumArtistsAction, &QAction::triggered, this, &LocalLibraryMenu::showAlbumArtistsTriggered);

	QList<QAction*> actions;
	actions <<
		m->infoAction <<
		m->editAction <<
		this->addSeparator() <<
		m->importFileAction <<
		m->importFolderAction <<
		m->reloadLibraryAction <<
		this->addSeparator() <<
		m->livesearchAction <<
		m->showAlbumArtistsAction;

	this->addActions(actions);
	this->addPreferenceAction(new Gui::LibraryPreferenceAction(this));

	m->isInitialized = true;

	ListenSetting(Set::Lib_ShowAlbumArtists, LocalLibraryMenu::showAlbumArtistsChanged);
	ListenSetting(Set::Lib_LiveSearch, LocalLibraryMenu::livesearchTriggered);

	shortcutChanged(ShortcutIdentifier::Invalid);
	languageChanged();
	skinChanged();
}

void LocalLibraryMenu::languageChanged()
{
	if(!m->isInitialized){
		return;
	}

	m->infoAction->setText(tr("Statistics"));
	m->editAction->setText(tr("Edit library"));

	m->importFileAction->setText(Lang::get(Lang::ImportFiles));
	m->importFolderAction->setText(Lang::get(Lang::ImportDir));

	m->livesearchAction->setText(Lang::get(Lang::LiveSearch));
	m->showAlbumArtistsAction->setText(Lang::get(Lang::ShowAlbumArtists));

	if(m->isLibraryEmpty) {
		m->reloadLibraryAction->setText(Lang::get(Lang::ScanForFiles));
	}

	else {
		m->reloadLibraryAction->setText(Lang::get(Lang::ReloadLibrary));
	}
}

void LocalLibraryMenu::skinChanged()
{
	if(!m->isInitialized){
		return;
	}

	using namespace Gui;
	m->reloadLibraryAction->setIcon(Icons::icon(Icons::Refresh));
	m->importFileAction->setIcon(Icons::icon(Icons::Open));
	m->importFolderAction->setIcon(Icons::icon(Icons::Open));
	m->infoAction->setIcon(Icons::icon(Icons::Info));
	m->editAction->setIcon(Icons::icon(Icons::Edit));
}

void LocalLibraryMenu::shortcutChanged(ShortcutIdentifier identifier)
{
	Q_UNUSED(identifier)

	if(!m->isInitialized){
		return;
	}

	ShortcutHandler* sch = ShortcutHandler::instance();

	m->showAlbumArtistsAction->setShortcutContext(Qt::WidgetShortcut);
	m->showAlbumArtistsAction->setShortcut(sch->shortcut(ShortcutIdentifier::AlbumArtists).sequence());
	m->reloadLibraryAction->setShortcutContext(Qt::WidgetShortcut);
	m->reloadLibraryAction->setShortcut(sch->shortcut(ShortcutIdentifier::ReloadLibrary).sequence());
}

void LocalLibraryMenu::setLiveSearchEnabled(bool b)
{
	SetSetting(Set::Lib_LiveSearch, b);
}


void LocalLibraryMenu::livesearchTriggered()
{
	if(!m->isInitialized){
		return;
	}

	m->livesearchAction->setChecked(GetSetting(Set::Lib_LiveSearch) );
}

void LocalLibraryMenu::editClicked()
{
	if(!m->isInitialized){
		return;
	}

	GUI_EditLibrary* edit_dialog = new GUI_EditLibrary(m->name, m->path, this);

	connect(edit_dialog, &GUI_EditLibrary::sigAccepted, this, &LocalLibraryMenu::editAccepted);

	edit_dialog->show();
}

void LocalLibraryMenu::editAccepted()
{
	auto* edit_dialog = static_cast<GUI_EditLibrary*>(sender());
	QString name = edit_dialog->name();
	QString path = edit_dialog->path();

	if(name.isEmpty() || path.isEmpty())
	{
		return;
	}

	if(edit_dialog->hasNameChanged()){
		emit sigNameChanged(name);
	}

	if(edit_dialog->hasPathChanged()){
		emit sigPathChanged(path);
	}
}

void LocalLibraryMenu::showAlbumArtistsTriggered(bool b)
{
	SetSetting(Set::Lib_ShowAlbumArtists, b);
}

void LocalLibraryMenu::showAlbumArtistsChanged()
{
	m->showAlbumArtistsAction->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));
}

