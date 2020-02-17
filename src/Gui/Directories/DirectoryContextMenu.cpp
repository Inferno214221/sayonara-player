/* DirectoryContextMenu.cpp */

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

#include "DirectoryContextMenu.h"

#include "Components/LibraryManagement/LibraryManager.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Language/Language.h"
#include "Utils/Library/LibraryInfo.h"

#include <QAction>

struct DirectoryContextMenu::Private
{
	QAction*	actionCreateDirectory=nullptr;
	QAction*	actionRename=nullptr;
	QAction*	actionRenameByTag=nullptr;
	QAction*	actionCollapseAll=nullptr;

	QMap<DirectoryContextMenu::Entry, QAction*> entryActionMap;

	QMenu*		menuMoveToLibrary=nullptr;
	QAction*	actionMoveToLibrary=nullptr;

	QMenu*		menuCopyToLibrary=nullptr;
	QAction*	actionCopyToLibrary=nullptr;

	QList<QAction*> libraryMoveActions, libraryCopyActions;

	DirectoryContextMenu::Mode mode;

	Private(DirectoryContextMenu::Mode mode, DirectoryContextMenu* parent) :
		mode(mode)
	{
		actionCreateDirectory = new QAction(parent);
		actionRename = new QAction(parent);
		actionRenameByTag =	new QAction(parent);
		actionCollapseAll =	new QAction(parent);

		{ // init copy/move to library menus
			auto* lm = Library::Manager::instance();
			const QList<Library::Info> libraries = lm->allLibraries();

			menuCopyToLibrary = new QMenu(parent);
			menuMoveToLibrary = new QMenu(parent);

			for(const Library::Info& info : libraries)
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
			{EntryCreateDir, actionCreateDirectory},
			{EntryRename, actionRename},
			{EntryRenameByTag, actionRenameByTag},
			{EntryCollapseAll, actionCollapseAll},
			{EntryMoveToLib, actionMoveToLibrary},
			{EntryCopyToLib, actionCopyToLibrary}
		};
	}
};

DirectoryContextMenu::DirectoryContextMenu(DirectoryContextMenu::Mode mode, QWidget* parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>(mode, this);

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

	QAction* action	= this->action(Library::ContextMenu::EntryDelete);
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

	connect(m->actionCreateDirectory, &QAction::triggered, this, &DirectoryContextMenu::sigCreateDirectoryClicked);
	connect(m->actionRename, &QAction::triggered, this, &DirectoryContextMenu::sigRenameClicked);
	connect(m->actionRenameByTag, &QAction::triggered, this, &DirectoryContextMenu::sigRenameByTagClicked);
	connect(m->actionCollapseAll, &QAction::triggered, this, &DirectoryContextMenu::sigCollapseAllClicked);

	for(QAction* action : m->libraryMoveActions)
	{
		connect(action, &QAction::triggered, this, &DirectoryContextMenu::libraryMoveActionTriggered);
	}

	for(QAction* action : m->libraryCopyActions)
	{
		connect(action, &QAction::triggered, this, &DirectoryContextMenu::libraryCopyActionTriggered);
	}

	action = this->addPreferenceAction(new Gui::LibraryPreferenceAction(this));

	this->insertActions
	(
		action,
		{
			m->actionCollapseAll,
			this->addSeparator(),
			m->actionMoveToLibrary,
			m->actionCopyToLibrary,
			this->addSeparator()
		}
	);

	switch(mode)
	{
		case DirectoryContextMenu::Mode::Dir:
			this->showAction(Library::ContextMenu::EntryLyrics, false);
			break;
		case DirectoryContextMenu::Mode::File:
			m->actionCreateDirectory->setVisible(false);
			m->actionCollapseAll->setVisible(false);
			break;
		default:
			break;
	}

	skinChanged();
	languageChanged();
}

DirectoryContextMenu::~DirectoryContextMenu() = default;

void DirectoryContextMenu::refresh(int count)
{
	if((count == 0) && (m->mode == DirectoryContextMenu::Mode::File))
	{
		this->showActions(Library::ContextMenu::EntryDelete);

		m->actionCreateDirectory->setVisible(false);
		m->actionCollapseAll->setVisible(false);
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
//			Library::ContextMenu::EntryStandardView |
//			Library::ContextMenu::EntryCoverView |
//			Library::ContextMenu::EntryDirectoryView
		);

		switch(m->mode)
		{
			case DirectoryContextMenu::Mode::Dir:
				this->showAction(Library::ContextMenu::EntryLyrics, false);
				m->actionCreateDirectory->setVisible(count == 1);
				m->actionCollapseAll->setVisible(true);
				m->actionRenameByTag->setVisible(false);

				this->showAction(Library::ContextMenu::EntryLyrics, false);

				break;
			case DirectoryContextMenu::Mode::File:
				m->actionCreateDirectory->setVisible(false);
				m->actionCollapseAll->setVisible(false);
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

Library::ContextMenu::Entries DirectoryContextMenu::entries() const
{
	auto entries = Library::ContextMenu::entries();
	for(auto it=m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		if(it.value()->isVisible()){
			entries |= it.key();
		}
	}

	return entries;
}

void DirectoryContextMenu::showActions(Library::ContextMenu::Entries entries)
{
	Library::ContextMenu::showActions(entries);

	for(auto it=m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		DirectoryContextMenu::Entry entry = it.key();
		QAction* action = it.value();
		action->setVisible(entries & entry);
	}
}

void DirectoryContextMenu::showDirectoryAction(DirectoryContextMenu::Entry entry, bool b)
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

void DirectoryContextMenu::libraryMoveActionTriggered()
{
	auto* action = static_cast<QAction*>(sender());

	LibraryId id = action->data().value<LibraryId>();
	emit sigMoveToLibrary(id);
}

void DirectoryContextMenu::libraryCopyActionTriggered()
{
	auto* action = static_cast<QAction*>(sender());

	LibraryId id = action->data().value<LibraryId>();
	emit sigCopyToLibrary(id);
}

void DirectoryContextMenu::languageChanged()
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
	}
}

void DirectoryContextMenu::skinChanged()
{
	Library::ContextMenu::skinChanged();

	using namespace Gui;
	if(m && m->actionRename)
	{
		m->actionRename->setIcon(Icons::icon(Icons::Rename));
		m->actionRenameByTag->setIcon(Icons::icon(Icons::Rename));
		m->actionCreateDirectory->setIcon(Icons::icon(Icons::New));
	}
}

