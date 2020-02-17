/* LibraryContextMenu.cpp */

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

#include "LibraryContextMenu.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/ExtensionSet.h"
#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryNamespaces.h"

#include <QMap>
#include <QTimer>

using Library::ContextMenu;
namespace Algorithm=Util::Algorithm;

struct ContextMenu::Private
{
	QMap<ContextMenu::Entry, QAction*> entryActionMap;

	QMenu*		filetype_menu=nullptr;

	QAction*	infoAction=nullptr;
	QAction*	lyricsAction=nullptr;
	QAction*	editAction=nullptr;
	QAction*	removeAction=nullptr;
	QAction*	deleteAction=nullptr;
	QAction*	playAction=nullptr;
	QAction*	playNewTabAction=nullptr;
	QAction*	playNextAction=nullptr;
	QAction*	appendAction=nullptr;
	QAction*	refreshAction=nullptr;
	QAction*	reloadLibraryAction=nullptr;
	QAction*	clearAction=nullptr;
	QAction*	standardViewAction=nullptr;
	QAction*	coverViewAction=nullptr;
	QAction*	directoryViewAction=nullptr;
	QAction*	filetypeAction=nullptr;
	QAction*	showFiletypeBarAction=nullptr;
	QAction*	preferenceSeparator=nullptr;

	bool has_preferenceActions;

	Private() :
		has_preferenceActions(false)
	{}
};


#include <QActionGroup>

ContextMenu::ContextMenu(QWidget* parent) :
	WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>();

	m->infoAction		= new QAction(this);
	m->lyricsAction		= new QAction(this);
	m->editAction		= new QAction(this);
	m->removeAction		= new QAction(this);
	m->deleteAction		= new QAction(this);
	m->playAction		= new QAction(this);
	m->playNewTabAction = new QAction(this);
	m->playNextAction	= new QAction(this);
	m->appendAction		= new QAction(this);
	m->refreshAction	= new QAction(this);
	m->reloadLibraryAction = new QAction(this);
	m->clearAction		= new QAction(this);

	{
		m->standardViewAction = new QAction(this);
		m->standardViewAction->setCheckable(true);

		m->coverViewAction = new QAction(this);
		m->coverViewAction->setCheckable(true);

		m->directoryViewAction = new QAction(this);
		m->directoryViewAction->setCheckable(true);

		auto* actionGroup = new QActionGroup(this);
		actionGroup->addAction(m->standardViewAction);
		actionGroup->addAction(m->coverViewAction);
		actionGroup->addAction(m->directoryViewAction);
	}

	m->filetype_menu = new QMenu(this);
	m->filetypeAction = this->addMenu(m->filetype_menu);
	m->showFiletypeBarAction = new QAction(this);
	m->showFiletypeBarAction->setCheckable(true);

	ListenSetting(Set::Lib_ViewType, ContextMenu::libraryViewTypeChanged);
	ListenSetting(Set::Lib_ShowFilterExtBar, ContextMenu::showFilterExtensionBarChanged);

	ShortcutHandler* sch = ShortcutHandler::instance();
	connect(sch, &ShortcutHandler::sigShortcutChanged, this, &ContextMenu::shortcutChanged);

	QList<QAction*> actions;
	actions << m->playAction
			<< m->playNewTabAction
			<< m->playNextAction
			<< m->appendAction
			<< addSeparator()

			<< m->infoAction
			<< m->lyricsAction
			<< m->editAction
			<< m->filetypeAction
			<< addSeparator()

			<< m->reloadLibraryAction
			<< m->refreshAction
			<< m->removeAction
			<< m->clearAction
			<< m->deleteAction
			<< addSeparator()

			<< m->standardViewAction
			<< m->coverViewAction
			<< m->directoryViewAction
	;

	this->addActions(actions);

	m->entryActionMap[EntryInfo]		= m->infoAction;
	m->entryActionMap[EntryEdit]		= m->editAction;
	m->entryActionMap[EntryLyrics]		= m->lyricsAction;
	m->entryActionMap[EntryRemove]		= m->removeAction;
	m->entryActionMap[EntryDelete]		= m->deleteAction;
	m->entryActionMap[EntryPlay]		= m->playAction;
	m->entryActionMap[EntryPlayNewTab]	= m->playNewTabAction;
	m->entryActionMap[EntryPlayNext]	= m->playNextAction;
	m->entryActionMap[EntryAppend]		= m->appendAction;
	m->entryActionMap[EntryRefresh]		= m->refreshAction;
	m->entryActionMap[EntryReload]		= m->reloadLibraryAction;
	m->entryActionMap[EntryClear]		= m->clearAction;
	m->entryActionMap[EntryStandardView] = m->standardViewAction;
	m->entryActionMap[EntryCoverView]	= m->coverViewAction;
	m->entryActionMap[EntryDirectoryView] = m->directoryViewAction;
	m->entryActionMap[EntryFilterExtension] = m->filetypeAction;

	for(QAction* action : Algorithm::AsConst(actions))
	{
		action->setVisible(action->isSeparator());
	}

	connect(m->infoAction, &QAction::triggered, this, &ContextMenu::sigInfoClicked);
	connect(m->lyricsAction, &QAction::triggered, this, &ContextMenu::sigLyricsClicked);
	connect(m->editAction, &QAction::triggered, this, &ContextMenu::sigEditClicked);
	connect(m->removeAction, &QAction::triggered, this, &ContextMenu::sigRemoveClicked);
	connect(m->deleteAction, &QAction::triggered, this, &ContextMenu::sigDeleteClicked);
	connect(m->playAction, &QAction::triggered, this, &ContextMenu::sigPlayClicked);
	connect(m->playNewTabAction, &QAction::triggered, this, &ContextMenu::sigPlayNewTabClicked);
	connect(m->playNextAction, &QAction::triggered, this, &ContextMenu::sigPlayNextClicked);
	connect(m->appendAction, &QAction::triggered, this, &ContextMenu::sigAppendClicked);
	connect(m->refreshAction, &QAction::triggered, this, &ContextMenu::sigRefreshClicked);
	connect(m->reloadLibraryAction, &QAction::triggered, this, &ContextMenu::sigReloadClicked);
	connect(m->clearAction, &QAction::triggered, this, &ContextMenu::sigClearClicked);
	connect(m->standardViewAction, &QAction::triggered, this, &ContextMenu::libraryViewTypeTriggered);
	connect(m->coverViewAction, &QAction::triggered, this, &ContextMenu::libraryViewTypeTriggered);
	connect(m->directoryViewAction, &QAction::triggered, this, &ContextMenu::libraryViewTypeTriggered);
	connect(m->showFiletypeBarAction, &QAction::triggered, this, &ContextMenu::showFilterExtensionBarTriggered);
}

ContextMenu::~ContextMenu() = default;

void ContextMenu::languageChanged()
{
	m->infoAction->setText(Lang::get(Lang::Info));
	m->lyricsAction->setText(Lang::get(Lang::Lyrics));
	m->editAction->setText(Lang::get(Lang::Edit));
	m->removeAction->setText(Lang::get(Lang::Remove));
	m->deleteAction->setText(Lang::get(Lang::Delete));
	m->playAction->setText(Lang::get(Lang::Play));
	m->playNewTabAction->setText(tr("Play in new tab"));
	m->playNextAction->setText(Lang::get(Lang::PlayNext));
	m->appendAction->setText(Lang::get(Lang::Append));
	m->refreshAction->setText(Lang::get(Lang::Refresh));
	m->reloadLibraryAction->setText(Lang::get(Lang::ReloadLibrary));
	m->clearAction->setText(Lang::get(Lang::Clear));
	m->standardViewAction->setText(tr("Standard view"));
	m->coverViewAction->setText(tr("Cover view"));
	m->directoryViewAction->setText(tr("Directory view"));
	m->filetypeAction->setText(Lang::get(Lang::Filetype));
	m->showFiletypeBarAction->setText(Lang::get(Lang::Show) + ": " + tr("Toolbar"));

	m->playAction->setShortcut(QKeySequence(Qt::Key_Enter));
	m->deleteAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Delete));
	m->removeAction->setShortcut(QKeySequence(QKeySequence::Delete));
	m->clearAction->setShortcut(QKeySequence(Qt::Key_Backspace));

	shortcutChanged(ShortcutIdentifier::Invalid);
}


void ContextMenu::shortcutChanged(ShortcutIdentifier identifier)
{
	Q_UNUSED(identifier)
	ShortcutHandler* sch = ShortcutHandler::instance();

	m->playNewTabAction->setShortcut(sch->shortcut(ShortcutIdentifier::PlayNewTab).sequence());
	m->playNextAction->setShortcut(sch->shortcut(ShortcutIdentifier::PlayNext).sequence());
	m->appendAction->setShortcut(sch->shortcut(ShortcutIdentifier::Append).sequence());
	m->coverViewAction->setShortcut(sch->shortcut(ShortcutIdentifier::CoverView).sequence());
	m->reloadLibraryAction->setShortcut(sch->shortcut(ShortcutIdentifier::ReloadLibrary).sequence());
}

void ContextMenu::skinChanged()
{
	using namespace Gui;

	QTimer::singleShot(100, this, &ContextMenu::skinTimerTimeout);
}

void ContextMenu::skinTimerTimeout()
{
	namespace Icons=Gui::Icons;

	m->infoAction->setIcon(Icons::icon(Icons::Info));
	m->lyricsAction->setIcon(Icons::icon(Icons::Lyrics));
	m->editAction->setIcon(Icons::icon(Icons::Edit));
	m->removeAction->setIcon(Icons::icon(Icons::Remove));
	m->deleteAction->setIcon(Icons::icon(Icons::Delete));
	m->playAction->setIcon(Icons::icon(Icons::PlaySmall));
	m->playNewTabAction->setIcon(Icons::icon(Icons::PlaySmall));
	m->playNextAction->setIcon(Icons::icon(Icons::PlaySmall));
	m->appendAction->setIcon(Icons::icon(Icons::Append));
	m->refreshAction->setIcon(Icons::icon(Icons::Undo));
	m->reloadLibraryAction->setIcon(Icons::icon(Icons::Refresh));
	m->clearAction->setIcon(Icons::icon(Icons::Clear));
}

ContextMenu::Entries ContextMenu::entries() const
{
	ContextMenu::Entries entries = EntryNone;

	for(auto it=m->entryActionMap.cbegin(); it != m->entryActionMap.cend(); it++)
	{
		QAction* action = it.value();
		if(action->isVisible())
		{
			ContextMenu::Entry entry = m->entryActionMap.key(action);
			entries |= entry;
		}
	}

	return entries;
}


void ContextMenu::showActions(ContextMenu::Entries entries)
{
	for(auto it=m->entryActionMap.cbegin(); it != m->entryActionMap.cend(); it++)
	{
		QAction* action = it.value();
		Entry entry = m->entryActionMap.key(action);

		bool isVisible = (entries & entry);
		action->setVisible(isVisible);
	}
}

void ContextMenu::showAction(ContextMenu::Entry entry, bool visible)
{
	ContextMenu::Entries entries = this->entries();
	if(visible){
		entries |= entry;
	}

	else{
		entries &= ~(entry);
	}

	showActions(entries);
}

void ContextMenu::showAll()
{
	const QList<QAction*> actions = this->actions();
	for(QAction* action : actions)
	{
		action->setVisible(true);
	}
}

QAction* ContextMenu::action(ContextMenu::Entry entry) const
{
	return m->entryActionMap[entry];
}

QAction* ContextMenu::actionAfter(ContextMenu::Entry entry) const
{
	QAction* a = action(entry);
	if(!a){
		return nullptr;
	}

	QList<QAction*> actions = this->actions();
	auto it = std::find(actions.begin(), actions.end(), a);

	if(it == actions.end()){
		return nullptr;
	}

	it++;
	if(it == actions.end()) {
		return nullptr;
	}

	return *it;
}

QAction* ContextMenu::addPreferenceAction(Gui::PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->has_preferenceActions){
		m->preferenceSeparator = this->addSeparator();
		actions << m->preferenceSeparator;
	}

	actions << action;

	this->addActions(actions);
	m->has_preferenceActions = true;

	return action;
}

QAction* ContextMenu::beforePreferenceAction() const
{
	return m->preferenceSeparator;
}

void ContextMenu::setActionShortcut(ContextMenu::Entry entry, const QString& shortcut)
{
	QAction* action = this->action(entry);
	if(action)
	{
		action->setShortcut(QKeySequence(shortcut));
	}
}

void ContextMenu::setExtensions(const Gui::ExtensionSet& extensions)
{
	QMenu* fem = m->filetype_menu;
	if(fem->isEmpty())
	{
		fem->addActions({fem->addSeparator(), m->showFiletypeBarAction});
	}

	while(fem->actions().count() > 2)
	{
		fem->removeAction(fem->actions().at(0));
	}

	QAction* sep = fem->actions().at(fem->actions().count() - 2);

	const QStringList extensionList = extensions.extensions();

	for(const QString& ext : extensionList)
	{
		QAction* a = new QAction(ext, fem);
		a->setCheckable(true);
		a->setChecked(extensions.isEnabled(ext));
		a->setEnabled(extensionList.count() > 1);

		connect(a, &QAction::triggered, this, [=](bool b){
			emit sigFilterTriggered(a->text(), b);
		});

		fem->insertAction(sep, a);
	}
}

void ContextMenu::setSelectionCount(int selectionCount)
{
	bool hasSelections = (selectionCount > 0);
	for(auto it : m->entryActionMap)
	{
		it->setEnabled(hasSelections);
	}

	m->entryActionMap[EntryCoverView]->setEnabled(true);
	m->entryActionMap[EntryReload]->setEnabled(true);
}

QKeySequence ContextMenu::shortcut(ContextMenu::Entry entry) const
{
	QAction* a = action(entry);
	if(!a){
		return QKeySequence();
	}

	return a->shortcut();
}

void ContextMenu::libraryViewTypeChanged()
{
	Library::ViewType viewType = GetSetting(Set::Lib_ViewType);

	m->standardViewAction->setChecked(viewType == Library::ViewType::Standard);
	m->coverViewAction->setChecked(viewType == Library::ViewType::CoverView);
	m->directoryViewAction->setChecked(viewType == Library::ViewType::FileView);
}

void ContextMenu::libraryViewTypeTriggered(bool b)
{
	Q_UNUSED(b)

	Library::ViewType viewType = Library::ViewType::Standard;
	if(m->coverViewAction->isChecked()) {
		viewType = Library::ViewType::CoverView;
	}

	else if(m->directoryViewAction->isChecked()) {
		viewType = Library::ViewType::FileView;
	}

	SetSetting(Set::Lib_ViewType, viewType);
}

void ContextMenu::showFilterExtensionBarChanged()
{
	m->showFiletypeBarAction->setChecked(GetSetting(Set::Lib_ShowFilterExtBar));
}

void ContextMenu::showFilterExtensionBarTriggered(bool b)
{
	SetSetting(Set::Lib_ShowFilterExtBar, b);

	if(b)
	{
		Message::info
		(
			tr("The toolbar is visible when there are tracks with differing file types listed in the track view"),
			Lang::get(Lang::Filetype)
		);
	}
}

