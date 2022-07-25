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

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/ExtensionSet.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"

#include <QActionGroup>
#include <QMap>
#include <QTimer>
#include <optional>

using Library::ContextMenu;

namespace
{
	void setNewShortcut(QAction* action, const ShortcutIdentifier identifier)
	{
		auto* shortcutHandler = ShortcutHandler::instance();
		action->setShortcut(shortcutHandler->shortcut(identifier).sequence());
	}

	struct ActionInfo
	{
		QAction* action;
		ContextMenu::Entry entry;
		Lang::Term term;
		std::optional<Gui::Icons::IconName> icon;
	};

	void createActionInfo(QMap<ContextMenu::Entry, ActionInfo>& actions, const ContextMenu::Entry entry,
	                      const Lang::Term term, const std::optional<Gui::Icons::IconName> icon, ContextMenu* parent)
	{
		actions.insert(entry, {new QAction(parent), entry, term, icon});
	}

	void initViewAction(QAction* action, QMenu* parentMenu, QActionGroup* actionGroup)
	{
		action->setCheckable(true);
		parentMenu->addAction(action);
		actionGroup->addAction(action);
	}
}

struct ContextMenu::Private
{
	QMap<ContextMenu::Entry, ActionInfo> actions;

	QMenu* filetypeMenu;
	QAction* filetypeMenuAction;
	QAction* showFiletypeBarAction;

	QMenu* viewTypeMenu;
	QAction* viewTypeMenuAction;
	QAction* standardViewAction;
	QAction* coverViewAction;
	QAction* directoryViewAction;

	QAction* preferenceSeparator {nullptr};

	explicit Private(ContextMenu* parent) :
		filetypeMenu {new QMenu(parent)},
		showFiletypeBarAction {new QAction(parent)},
		viewTypeMenu {new QMenu(parent)},
		standardViewAction {new QAction(parent)},
		coverViewAction {new QAction(parent)},
		directoryViewAction {new QAction(parent)}
	{
		using namespace Gui;
		createActionInfo(actions, EntryInfo, Lang::Info, Icons::Info, parent);
		createActionInfo(actions, EntryLyrics, Lang::Lyrics, Icons::Lyrics, parent);
		createActionInfo(actions, EntryEdit, Lang::Edit, Icons::Edit, parent);
		createActionInfo(actions, EntryRemove, Lang::Remove, Icons::Remove, parent);
		createActionInfo(actions, EntryDelete, Lang::Delete, Icons::Delete, parent);
		createActionInfo(actions, EntryPlay, Lang::Play, Icons::PlaySmall, parent);
		createActionInfo(actions, EntryPlayNewTab, Lang::PlayInNewTab, Icons::PlaySmall, parent);
		createActionInfo(actions, EntryPlayNext, Lang::PlayNext, Icons::PlaySmall, parent);
		createActionInfo(actions, EntryAppend, Lang::Append, Icons::Append, parent);
		createActionInfo(actions, EntryRefresh, Lang::Refresh, Icons::Undo, parent);
		createActionInfo(actions, EntryReload, Lang::ReloadLibrary, Icons::Refresh, parent);
		createActionInfo(actions, EntryClear, Lang::Clear, Icons::Clear, parent);

		auto* actionGroup = new QActionGroup(parent);
		initViewAction(standardViewAction, viewTypeMenu, actionGroup);
		initViewAction(directoryViewAction, viewTypeMenu, actionGroup);
		initViewAction(coverViewAction, viewTypeMenu, actionGroup);

		viewTypeMenuAction = parent->addMenu(viewTypeMenu); // NOLINT(cppcoreguidelines-prefer-member-initializer)
		actions.insert(EntryViewType,
		               {viewTypeMenuAction, EntryViewType, Lang::LibraryView, std::nullopt});

		showFiletypeBarAction->setCheckable(true);
		filetypeMenuAction = parent->addMenu(filetypeMenu); // NOLINT(cppcoreguidelines-prefer-member-initializer)
		actions.insert(EntryFilterExtension,
		               {filetypeMenuAction, EntryFilterExtension, Lang::Filetype, std::nullopt});
	}
};

ContextMenu::ContextMenu(QWidget* parent) :
	WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(this);

	ListenSetting(Set::Lib_ViewType, ContextMenu::libraryViewTypeChanged);
	ListenSetting(Set::Lib_ShowFilterExtBar, ContextMenu::showFilterExtensionBarChanged);

	auto* shortcutHandler = ShortcutHandler::instance();
	connect(shortcutHandler, &ShortcutHandler::sigShortcutChanged, this, &ContextMenu::shortcutChanged);

	const auto actions = QList<QAction*> {
		m->actions[EntryPlay].action,
		m->actions[EntryPlayNewTab].action,
		m->actions[EntryPlayNext].action,
		m->actions[EntryAppend].action,
		addSeparator(),
		m->actions[EntryInfo].action,
		m->actions[EntryLyrics].action,
		m->actions[EntryEdit].action,
		m->actions[EntryFilterExtension].action,
		addSeparator(),
		m->actions[EntryReload].action,
		m->actions[EntryRefresh].action,
		m->actions[EntryRemove].action,
		m->actions[EntryClear].action,
		m->actions[EntryDelete].action,
		addSeparator(),
		m->actions[EntryViewType].action,
		addSeparator()
	};

	m->preferenceSeparator = actions.last();

	addActions(actions);
	parentWidget()->addActions(actions);

	for(auto* action: actions)
	{
		action->setVisible(action->isSeparator());
	}

	m->preferenceSeparator->setVisible(false);

	connect(m->standardViewAction, &QAction::triggered, this, &ContextMenu::libraryViewTypeTriggered);
	connect(m->coverViewAction, &QAction::triggered, this, &ContextMenu::libraryViewTypeTriggered);
	connect(m->directoryViewAction, &QAction::triggered, this, &ContextMenu::libraryViewTypeTriggered);
	connect(m->showFiletypeBarAction, &QAction::triggered, this, &ContextMenu::showFilterExtensionBarTriggered);
}

ContextMenu::~ContextMenu() = default;

void ContextMenu::languageChanged()
{
	for(auto& actionInfo: m->actions)
	{
		actionInfo.action->setText(Lang::get(actionInfo.term));
	}

	m->standardViewAction->setText(tr("Standard view"));
	m->coverViewAction->setText(tr("Cover view"));
	m->directoryViewAction->setText(tr("Directory view"));
	m->showFiletypeBarAction->setText(Lang::get(Lang::Show) + ": " + tr("Toolbar"));

	m->actions[EntryPlay].action->setShortcut(QKeySequence(Qt::Key_Enter));
	m->actions[EntryDelete].action->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Delete));
	m->actions[EntryRemove].action->setShortcut(QKeySequence(QKeySequence::Delete));
	m->actions[EntryClear].action->setShortcut(QKeySequence(Qt::Key_Backspace));

	shortcutChanged(ShortcutIdentifier::Invalid);
}

void ContextMenu::shortcutChanged(const ShortcutIdentifier /*identifier*/)
{
	setNewShortcut(m->actions[EntryPlayNewTab].action, ShortcutIdentifier::PlayNewTab);
	setNewShortcut(m->actions[EntryPlayNext].action, ShortcutIdentifier::PlayNext);
	setNewShortcut(m->actions[EntryAppend].action, ShortcutIdentifier::Append);
	setNewShortcut(m->actions[EntryReload].action, ShortcutIdentifier::ReloadLibrary);
}

void ContextMenu::skinChanged()
{
	QTimer::singleShot(100, this, [&]() { // NOLINT(readability-magic-numbers)
		for(auto& action: m->actions)
		{
			if(action.icon.has_value())
			{
				action.action->setIcon(Gui::Icons::icon(action.icon.value()));
			}
		}
	});
}

ContextMenu::Entries ContextMenu::entries() const
{
	auto entries = ContextMenu::Entries {EntryNone};
	for(const auto& entry: m->actions)
	{
		if(entry.action->isVisible())
		{
			entries |= entry.entry;
		}
	}

	return entries;
}

void ContextMenu::showActions(const ContextMenu::Entries entries)
{
	for(const auto& entry: m->actions)
	{
		const auto isVisible = (entries & entry.entry);
		entry.action->setVisible(isVisible);
	}
}

void ContextMenu::showAction(const ContextMenu::Entry entry, const bool visible)
{
	auto entries = this->entries();
	if(visible)
	{
		entries |= entry;
	}

	else
	{
		entries &= ~(entry);
	}

	showActions(entries);
}

void ContextMenu::showAll()
{
	const auto actions = this->actions();
	for(auto* action: actions)
	{
		action->setVisible(true);
	}
}

QAction* ContextMenu::action(const ContextMenu::Entry entry) const
{
	return m->actions[entry].action;
}

QAction* ContextMenu::actionAfter(const ContextMenu::Entry entry) const
{
	auto* a = action(entry);
	if(!a)
	{
		return nullptr;
	}

	const auto actions = this->actions();
	const auto index = actions.indexOf(a);
	return (index + 1 < actions.count())
	       ? actions[index + 1]
	       : nullptr;
}

QAction* ContextMenu::addPreferenceAction(Gui::PreferenceAction* action)
{
	m->preferenceSeparator->setVisible(true);
	addActions({action});

	return action;
}

QAction* ContextMenu::beforePreferenceAction() const
{
	return m->preferenceSeparator;
}

void ContextMenu::setExtensions(const Gui::ExtensionSet& extensions)
{
	auto* filetypeMenu = m->filetypeMenu;
	if(filetypeMenu->isEmpty())
	{
		filetypeMenu->addActions({filetypeMenu->addSeparator(), m->showFiletypeBarAction});
	}

	while(filetypeMenu->actions().count() > 2)
	{
		filetypeMenu->removeAction(filetypeMenu->actions().at(0));
	}

	auto* sep = filetypeMenu->actions().at(filetypeMenu->actions().count() - 2);

	const auto extensionList = extensions.extensions();

	for(const auto& extension: extensionList)
	{
		auto* extensionAction = new QAction(extension, filetypeMenu);
		extensionAction->setCheckable(true);
		extensionAction->setChecked(extensions.isEnabled(extension));
		extensionAction->setEnabled(extensionList.count() > 1);

		connect(extensionAction, &QAction::triggered, this, [=](bool b) {
			emit sigFilterTriggered(extensionAction->text(), b);
		});

		filetypeMenu->insertAction(sep, extensionAction);
	}
}

void ContextMenu::setSelectionCount(const int selectionCount)
{
	for(auto& entry: m->actions)
	{
		entry.action->setEnabled(selectionCount > 0);
	}

	m->actions[EntryReload].action->setEnabled(true);
}

QKeySequence ContextMenu::shortcut(ContextMenu::Entry entry) const
{
	auto* a = action(entry);
	return (a != nullptr)
	       ? a->shortcut()
	       : QKeySequence {};
}

void ContextMenu::libraryViewTypeChanged()
{
	const auto viewType = GetSetting(Set::Lib_ViewType);

	m->standardViewAction->setChecked(viewType == Library::ViewType::Standard);
	m->coverViewAction->setChecked(viewType == Library::ViewType::CoverView);
	m->directoryViewAction->setChecked(viewType == Library::ViewType::FileView);
}

void ContextMenu::libraryViewTypeTriggered(const bool /*b*/)
{
	auto viewType = Library::ViewType::Standard;
	if(m->coverViewAction->isChecked())
	{
		viewType = Library::ViewType::CoverView;
	}

	else if(m->directoryViewAction->isChecked())
	{
		viewType = Library::ViewType::FileView;
	}

	SetSetting(Set::Lib_ViewType, viewType);
}

void ContextMenu::showFilterExtensionBarChanged()
{
	m->showFiletypeBarAction->setChecked(GetSetting(Set::Lib_ShowFilterExtBar));
}

void
ContextMenu::showFilterExtensionBarTriggered(const bool b) // NOLINT(readability-convert-member-functions-to-static)
{
	SetSetting(Set::Lib_ShowFilterExtBar, b);

	if(b)
	{
		Message::info(
			tr("The toolbar is visible when there are tracks with differing file types listed in the track view"),
			Lang::get(Lang::Filetype));
	}
}
