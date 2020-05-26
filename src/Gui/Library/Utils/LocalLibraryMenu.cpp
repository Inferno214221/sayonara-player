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
#include "GUI_EditLibrary.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

using Library::LocalLibraryMenu;

struct LocalLibraryMenu::Private
{
	QString name, path;

	QAction* reloadLibraryAction = nullptr;
	QAction* importFileAction = nullptr;
	QAction* importFolderAction = nullptr;
	QAction* infoAction = nullptr;
	QAction* editAction = nullptr;

	bool hasPreferenceAction;
	bool isInitialized;
	bool isLibraryEmpty;

	Private(const QString& name, const QString& path) :
		name(name),
		path(path),
		hasPreferenceAction(false),
		isInitialized(false),
		isLibraryEmpty(false) {}
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
	if(!m->isInitialized)
	{
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

	if(!m->hasPreferenceAction)
	{
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

	connect(m->reloadLibraryAction, &QAction::triggered, this, &LocalLibraryMenu::sigReloadLibrary);
	connect(m->importFileAction, &QAction::triggered, this, &LocalLibraryMenu::sigImportFile);
	connect(m->importFolderAction, &QAction::triggered, this, &LocalLibraryMenu::sigImportFolder);
	connect(m->infoAction, &QAction::triggered, this, &LocalLibraryMenu::sigInfo);
	connect(m->editAction, &QAction::triggered, this, &LocalLibraryMenu::editClicked);

	QAction* sep = this->addSeparator();
	QList<QAction*> actions
		{
			m->infoAction,
			m->editAction,
			sep,
			m->importFileAction,
			m->importFolderAction,
			m->reloadLibraryAction,
		};

	this->addActions(actions);
	this->addPreferenceAction(new Gui::LibraryPreferenceAction(this));

	m->isInitialized = true;

	shortcutChanged(ShortcutIdentifier::Invalid);
	languageChanged();
	skinChanged();
}

void LocalLibraryMenu::languageChanged()
{
	if(!m->isInitialized)
	{
		return;
	}

	m->infoAction->setText(tr("Statistics"));
	m->editAction->setText(tr("Edit library"));

	m->importFileAction->setText(Lang::get(Lang::ImportFiles));
	m->importFolderAction->setText(Lang::get(Lang::ImportDir));

	if(m->isLibraryEmpty)
	{
		m->reloadLibraryAction->setText(Lang::get(Lang::ScanForFiles));
	}

	else
	{
		m->reloadLibraryAction->setText(Lang::get(Lang::ReloadLibrary));
	}
}

void LocalLibraryMenu::skinChanged()
{
	if(!m->isInitialized)
	{
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

	if(!m->isInitialized)
	{
		return;
	}

	auto* sch = ShortcutHandler::instance();
	m->reloadLibraryAction->setShortcutContext(Qt::WidgetShortcut);
	m->reloadLibraryAction->setShortcut(sch->shortcut(ShortcutIdentifier::ReloadLibrary).sequence());
}

void LocalLibraryMenu::editClicked()
{
	if(!m->isInitialized)
	{
		return;
	}

	auto* editDialog = new GUI_EditLibrary(m->name, m->path, this);
	connect(editDialog, &GUI_EditLibrary::sigAccepted, this, &LocalLibraryMenu::editAccepted);

	editDialog->show();
}

void LocalLibraryMenu::editAccepted()
{
	auto* editDialog = dynamic_cast<GUI_EditLibrary*>(sender());

	const QString name = editDialog->name();
	const QString path = editDialog->path();
	if(name.isEmpty() || path.isEmpty())
	{
		return;
	}

	if(editDialog->hasNameChanged())
	{
		emit sigNameChanged(name);
	}

	if(editDialog->hasPathChanged())
	{
		emit sigPathChanged(path);
	}
}
