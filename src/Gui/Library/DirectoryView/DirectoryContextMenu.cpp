/* DirectoryContextMenu.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "DirectoryContextMenu.h"

#include "Components/LibraryManagement/LibraryManager.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/LibraryInfo.h"

#include <QAction>

using Directory::ContextMenu;

struct ContextMenu::Private
{
	QAction* actionCreateDirectory = nullptr;
	QAction* actionRename = nullptr;
	QAction* actionRenameByTag = nullptr;
	QAction* actionCollapseAll = nullptr;
	QAction* actionViewInFileManager = nullptr;

	QMap<ContextMenu::Entry, QAction*> entryActionMap;

	QMenu* menuMoveToLibrary = nullptr;
	QAction* actionMoveToLibrary = nullptr;

	QMenu* menuCopyToLibrary = nullptr;
	QAction* actionCopyToLibrary = nullptr;

	QList<QAction*> libraryMoveActions, libraryCopyActions;

	ContextMenu::Mode mode;

	Private(ContextMenu::Mode mode, Library::InfoAccessor* libraryInfoAccessor, ContextMenu* parent) :
		mode(mode)
	{
		actionCreateDirectory = new QAction(parent);
		actionRename = new QAction(parent);
		actionRenameByTag = new QAction(parent);
		actionCollapseAll = new QAction(parent);
		actionViewInFileManager = new QAction(parent);

		{ // init copy/move to library menus
			const QList<Library::Info> libraries = libraryInfoAccessor->allLibraries();

			menuCopyToLibrary = new QMenu(parent);
			menuMoveToLibrary = new QMenu(parent);

			for(const Library::Info& info: libraries)
			{
				QAction* action1 = menuCopyToLibrary->addAction(info.name());
				QAction* action2 = menuMoveToLibrary->addAction(info.name());

				action1->setData(info.id());
				action2->setData(info.id());

				libraryCopyActions << action1;
				libraryMoveActions << action2;
			}

			actionCopyToLibrary = parent->addMenu(menuCopyToLibrary);
			actionMoveToLibrary = parent->addMenu(menuMoveToLibrary);
		}

		entryActionMap =
			{
				{EntryCreateDir,   actionCreateDirectory},
				{EntryRename,      actionRename},
				{EntryRenameByTag, actionRenameByTag},
				{EntryCollapseAll, actionCollapseAll},
				{EntryMoveToLib,   actionMoveToLibrary},
				{EntryCopyToLib,   actionCopyToLibrary},
				{EntryViewInFM,    actionViewInFileManager}
			};
	}
};

ContextMenu::ContextMenu(ContextMenu::Mode mode, Library::InfoAccessor* libraryInfoAccessor, QWidget* parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>(mode, libraryInfoAccessor, this);

	this->showActions
		(
			(Library::ContextMenu::EntryPlay |
			 Library::ContextMenu::EntryPlayNewTab |
			 Library::ContextMenu::EntryDelete |
			 Library::ContextMenu::EntryInfo |
			 Library::ContextMenu::EntryEdit |
			 Library::ContextMenu::EntryLyrics |
			 Library::ContextMenu::EntryAppend |
			 Library::ContextMenu::EntryPlayNext)
		);

	QAction* action = this->action(Library::ContextMenu::EntryDelete);
	if(action)
	{
		this->insertActions
			(
				action,
				{
					this->addSeparator(),
					m->actionCreateDirectory, m->actionRename, m->actionRenameByTag,
				}
			);
	}

	connect(m->actionCreateDirectory, &QAction::triggered, this, &ContextMenu::sigCreateDirectoryClicked);
	connect(m->actionRename, &QAction::triggered, this, &ContextMenu::sigRenameClicked);
	connect(m->actionRenameByTag, &QAction::triggered, this, &ContextMenu::sigRenameByTagClicked);
	connect(m->actionCollapseAll, &QAction::triggered, this, &ContextMenu::sigCollapseAllClicked);
	connect(m->actionViewInFileManager, &QAction::triggered, this, &ContextMenu::sigViewInFileManagerClicked);

	for(QAction* action: m->libraryMoveActions)
	{
		connect(action, &QAction::triggered, this, &ContextMenu::libraryMoveActionTriggered);
	}

	for(QAction* action: m->libraryCopyActions)
	{
		connect(action, &QAction::triggered, this, &ContextMenu::libraryCopyActionTriggered);
	}

	action = this->addPreferenceAction(new Gui::LibraryPreferenceAction(this));

	this->insertActions
		(
			action,
			{
				m->actionViewInFileManager,
				m->actionCollapseAll,
				this->addSeparator(),
				m->actionMoveToLibrary,
				m->actionCopyToLibrary,
				this->addSeparator()
			}
		);

	switch(mode)
	{
		case ContextMenu::Mode::Dir:
			this->showAction(Library::ContextMenu::EntryLyrics, false);
			break;
		case ContextMenu::Mode::File:
			m->actionCreateDirectory->setVisible(false);
			m->actionCollapseAll->setVisible(false);
			break;
		default:
			break;
	}

	skinChanged();
	languageChanged();
}

ContextMenu::~ContextMenu() = default;

void ContextMenu::refresh(int count)
{
	if((count == 0) && (m->mode == ContextMenu::Mode::File))
	{
		this->showActions(Library::ContextMenu::EntryDelete);

		m->actionCreateDirectory->setVisible(false);
		m->actionCollapseAll->setVisible(false);
		m->actionViewInFileManager->setVisible(false);
		m->actionMoveToLibrary->setVisible(false);
		m->actionCopyToLibrary->setVisible(false);
		m->actionRenameByTag->setVisible(false);
		m->actionRename->setVisible(false);
	}

	else
	{
		this->showActions
			(
				Library::ContextMenu::EntryPlay |
				Library::ContextMenu::EntryPlayNewTab |
				Library::ContextMenu::EntryDelete |
				Library::ContextMenu::EntryInfo |
				Library::ContextMenu::EntryEdit |
				Library::ContextMenu::EntryLyrics |
				Library::ContextMenu::EntryAppend |
				Library::ContextMenu::EntryPlayNext
			);

		switch(m->mode)
		{
			case ContextMenu::Mode::Dir:
				this->showAction(Library::ContextMenu::EntryLyrics, false);
				m->actionCreateDirectory->setVisible(count == 1);
				m->actionViewInFileManager->setVisible(count > 0);
				m->actionCollapseAll->setVisible(true);
				m->actionRenameByTag->setVisible(false);

				this->showAction(Library::ContextMenu::EntryLyrics, false);

				break;
			case ContextMenu::Mode::File:
				m->actionCreateDirectory->setVisible(false);
				m->actionCollapseAll->setVisible(false);
				m->actionViewInFileManager->setVisible(false);
				m->actionRenameByTag->setVisible(true);

				this->showAction(Library::ContextMenu::EntryLyrics, (count == 1));
				break;
			default:
				break;
		}

		m->actionRename->setVisible(count == 1);
		m->actionMoveToLibrary->setVisible(true);
		m->actionCopyToLibrary->setVisible(true);
	}
}

Library::ContextMenu::Entries ContextMenu::entries() const
{
	auto entries = Library::ContextMenu::entries();
	for(auto it = m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		if(it.value()->isVisible())
		{
			entries |= it.key();
		}
	}

	return entries;
}

void ContextMenu::showActions(Library::ContextMenu::Entries entries)
{
	Library::ContextMenu::showActions(entries);

	for(auto it = m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		ContextMenu::Entry entry = it.key();
		QAction* action = it.value();
		action->setVisible(entries & entry);
	}
}

void ContextMenu::showDirectoryAction(ContextMenu::Entry entry, bool b)
{
	Library::ContextMenu::Entries entries = this->entries();
	if(b)
	{
		entries |= entry;
	}

	else
	{
		entries &= ~entry;
	}

	showActions(entries);
}

void ContextMenu::libraryMoveActionTriggered()
{
	auto* action = static_cast<QAction*>(sender());

	LibraryId id = action->data().value<LibraryId>();
	emit sigMoveToLibrary(id);
}

void ContextMenu::libraryCopyActionTriggered()
{
	auto* action = static_cast<QAction*>(sender());

	LibraryId id = action->data().value<LibraryId>();
	emit sigCopyToLibrary(id);
}

void ContextMenu::languageChanged()
{
	Library::ContextMenu::languageChanged();

	if(m && m->actionRename)
	{
		m->actionRename->setText(Lang::get(Lang::Rename));
		m->actionRenameByTag->setText(tr("Rename by metadata"));
		m->actionCreateDirectory->setText(Lang::get(Lang::CreateDirectory));
		m->actionCollapseAll->setText(tr("Collapse all"));
		m->actionMoveToLibrary->setText(tr("Move to another library"));
		m->actionCopyToLibrary->setText(tr("Copy to another library"));
		m->actionViewInFileManager->setText(tr("View in file manager"));
	}
}

void ContextMenu::skinChanged()
{
	Library::ContextMenu::skinChanged();

	using namespace Gui;
	if(m && m->actionRename)
	{
		m->actionViewInFileManager->setIcon(Icons::icon(Icons::FolderOpen));
		m->actionRename->setIcon(Icons::icon(Icons::Rename));
		m->actionRenameByTag->setIcon(Icons::icon(Icons::Rename));
		m->actionCreateDirectory->setIcon(Icons::icon(Icons::New));
	}
}
