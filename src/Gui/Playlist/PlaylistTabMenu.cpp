
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

#include "PlaylistTabMenu.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Language/Language.h"

using namespace Gui;
using Playlist::MenuEntry;
using Playlist::TabMenu;

struct TabMenu::Private
{
	QAction* actionOpenFile;
	QAction* actionOpenDir;
	QAction* actionDelete;
	QAction* actionSave;
	QAction* actionSaveAs;
	QAction* actionSaveToFile;
	QAction* actionReset;
	QAction* actionClose;
	QAction* actionCloseOthers;
	QAction* actionRename;
	QAction* actionClear;
	QAction* actionLock;
	QAction* actionUnlock;

	bool hasPreferenceAction {false};

	explicit Private(QWidget* parent) :
		actionOpenFile {new QAction(parent)},
		actionOpenDir {new QAction(parent)},
		actionDelete {new QAction(parent)},
		actionSave {new QAction(parent)},
		actionSaveAs {new QAction(parent)},
		actionSaveToFile {new QAction(parent)},
		actionReset {new QAction(parent)},
		actionClose {new QAction(parent)},
		actionCloseOthers {new QAction(parent)},
		actionRename {new QAction(parent)},
		actionClear {new QAction(parent)},
		actionLock {new QAction(parent)},
		actionUnlock {new QAction(parent)} {}
};

TabMenu::TabMenu(QWidget* parent) :
	WidgetTemplate<QMenu>(parent),
	m {Pimpl::make<Private>(this)}
{
	const auto actions = QList<QAction*> {
		m->actionOpenFile,
		m->actionOpenDir,
		this->addSeparator(),
		m->actionReset,
		m->actionLock,
		m->actionUnlock,
		this->addSeparator(),
		m->actionRename,
		m->actionSave,
		m->actionSaveAs,
		m->actionSaveToFile,
		m->actionDelete,
		this->addSeparator(),
		m->actionClear,
		this->addSeparator(),
		m->actionCloseOthers,
		m->actionClose
	};

	addActions(actions);

	connect(m->actionOpenFile, &QAction::triggered, this, &TabMenu::sigOpenFileClicked);
	connect(m->actionOpenDir, &QAction::triggered, this, &TabMenu::sigOpenDirClicked);
	connect(m->actionReset, &QAction::triggered, this, &TabMenu::sigResetClicked);
	connect(m->actionRename, &QAction::triggered, this, &TabMenu::sigRenameClicked);
	connect(m->actionDelete, &QAction::triggered, this, &TabMenu::sigDeleteClicked);
	connect(m->actionSave, &QAction::triggered, this, &TabMenu::sigSaveClicked);
	connect(m->actionSaveAs, &QAction::triggered, this, &TabMenu::sigSaveAsClicked);
	connect(m->actionSaveToFile, &QAction::triggered, this, &TabMenu::sigSaveToFileClicked);
	connect(m->actionClear, &QAction::triggered, this, &TabMenu::sigClearClicked);
	connect(m->actionClose, &QAction::triggered, this, &TabMenu::sigCloseClicked);
	connect(m->actionCloseOthers, &QAction::triggered, this, &TabMenu::sigCloseOthersClicked);
	connect(m->actionLock, &QAction::triggered, this, [this](const auto /*b*/) { emit sigLockTriggered(true); });
	connect(m->actionUnlock, &QAction::triggered, this, [this](const auto /*b*/) { emit sigLockTriggered(false); });

	addPreferenceAction(new PlaylistPreferenceAction(this));
}

TabMenu::~TabMenu()
{
	clear();
}

void TabMenu::languageChanged()
{
	m->actionOpenFile->setText(Lang::get(Lang::OpenFile).triplePt());
	m->actionOpenDir->setText(Lang::get(Lang::OpenDir).triplePt());
	m->actionReset->setText(Lang::get(Lang::Reset));
	m->actionRename->setText(Lang::get(Lang::Rename).triplePt());
	m->actionDelete->setText(Lang::get(Lang::Delete));
	m->actionSave->setText(Lang::get(Lang::Save));
	m->actionSaveAs->setText(Lang::get(Lang::SaveAs).triplePt());
	m->actionSaveToFile->setText(Lang::get(Lang::SaveToFile));
	m->actionClear->setText(Lang::get(Lang::Clear));
	m->actionClose->setText(Lang::get(Lang::Close));
	m->actionCloseOthers->setText(Lang::get(Lang::CloseOthers));
	m->actionLock->setText(Lang::get(Lang::LockPlaylist));
	m->actionUnlock->setText(Lang::get(Lang::UnlockPlaylist));

	m->actionRename->setShortcut(QKeySequence("F2"));
	m->actionSave->setShortcut(QKeySequence::Save);
	m->actionSaveAs->setShortcut(QKeySequence::SaveAs);
	m->actionOpenFile->setShortcut(QKeySequence::Open);

	QKeySequence ks(QKeySequence::Open);
	m->actionOpenDir->setShortcut(QKeySequence("Shift+" + ks.toString()));
}

void TabMenu::skinChanged()
{
	m->actionOpenFile->setIcon(Icons::icon(Icons::Open));
	m->actionOpenDir->setIcon(Icons::icon(Icons::Open));

	m->actionReset->setIcon(Icons::icon(Icons::Undo));
	m->actionRename->setIcon(Icons::icon(Icons::Rename));
	m->actionDelete->setIcon(Icons::icon(Icons::Delete));
	m->actionSave->setIcon(Icons::icon(Icons::Save));
	m->actionSaveAs->setIcon(Icons::icon(Icons::SaveAs));
	m->actionSaveToFile->setIcon(Icons::icon(Icons::SaveAs));
	m->actionClear->setIcon(Icons::icon(Icons::Clear));
	m->actionClose->setIcon(Icons::icon(Icons::Close));
	m->actionCloseOthers->setIcon(Icons::icon(Icons::Close));
	m->actionLock->setIcon(Icons::icon(Icons::Lock));
	m->actionUnlock->setIcon(Icons::icon(Icons::Unlock));
}

void TabMenu::showMenuItems(Playlist::MenuEntries entries)
{
	m->actionOpenFile->setVisible(entries & MenuEntry::OpenFile);
	m->actionOpenDir->setVisible(entries & MenuEntry::OpenDir);
	m->actionReset->setVisible(entries & MenuEntry::Reset);
	m->actionRename->setVisible(entries & MenuEntry::Rename);
	m->actionDelete->setVisible(entries & MenuEntry::Delete);
	m->actionSave->setVisible(entries & MenuEntry::Save);
	m->actionSaveAs->setVisible(entries & MenuEntry::SaveAs);
	m->actionSaveToFile->setVisible(entries & MenuEntry::SaveToFile);
	m->actionClear->setVisible(entries & MenuEntry::Clear);
	m->actionClose->setVisible(entries & MenuEntry::Close);
	m->actionCloseOthers->setVisible(entries & MenuEntry::CloseOthers);
	m->actionLock->setVisible(entries & MenuEntry::Lock);
	m->actionUnlock->setVisible(entries & MenuEntry::Unlock);
}

void TabMenu::showClose(bool b)
{
	m->actionClose->setVisible(b);
	m->actionCloseOthers->setVisible(b);
}

void TabMenu::addPreferenceAction(Gui::PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->hasPreferenceAction)
	{
		actions << addSeparator();
	}

	actions << action;

	addActions(actions);
	m->hasPreferenceAction = true;
}
