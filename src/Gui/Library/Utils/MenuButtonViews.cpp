/* MenuButtonViews.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "MenuButtonViews.h"

#include "Gui/Utils/Icons.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Gui/Utils/PreferenceAction.h"

#include <QMenu>
#include <QAction>

using Library::MenuButtonViews;

struct MenuButtonViews::Private
{
	QMenu* menu = nullptr;
	QAction* tableViewAction = nullptr;
	QAction* coverViewAction = nullptr;
	QAction* directoryViewAction = nullptr;
	QAction* shortcutSection = nullptr;

	Gui::PreferenceAction* preferenceAction = nullptr;

	Private(QWidget* parent)
	{
		menu = new QMenu(parent);

		tableViewAction = new QAction();
		tableViewAction->setCheckable(true);

		coverViewAction = new QAction();
		coverViewAction->setCheckable(true);

		directoryViewAction = new QAction();
		directoryViewAction->setCheckable(true);

		auto* shortcutHandler = ShortcutHandler::instance();
		QString shortcutString = shortcutHandler->shortcut(ShortcutIdentifier::CoverView).sequence().toString();
		shortcutSection = new QAction(shortcutString);
		shortcutSection->setEnabled(false);

		preferenceAction = new Gui::ShortcutPreferenceAction(menu);

		auto* actionGroup = new QActionGroup(parent);
		actionGroup->addAction(tableViewAction);
		actionGroup->addAction(coverViewAction);
		actionGroup->addAction(directoryViewAction);
	}
};

MenuButtonViews::MenuButtonViews(QWidget* parent) :
	Gui::MenuToolButton(parent)
{
	m = Pimpl::make<Private>(this);

	this->registerAction(m->tableViewAction);
	this->registerAction(m->coverViewAction);
	this->registerAction(m->directoryViewAction);
	this->registerAction(m->menu->addSeparator());
	this->registerAction(m->shortcutSection);
	this->registerPreferenceAction(m->preferenceAction);

	viewTypeChanged();

	auto* shortcutHandler = ShortcutHandler::instance();
	connect(shortcutHandler, &ShortcutHandler::sigShortcutChanged, this, &MenuButtonViews::shortcutChanged);

	connect(m->tableViewAction, &QAction::triggered, this, &MenuButtonViews::actionTriggered);
	connect(m->coverViewAction, &QAction::triggered, this, &MenuButtonViews::actionTriggered);
	connect(m->directoryViewAction, &QAction::triggered, this, &MenuButtonViews::actionTriggered);

	ListenSetting(Set::Lib_ViewType, MenuButtonViews::viewTypeChanged);
}

void MenuButtonViews::actionTriggered(bool b)
{
	Q_UNUSED(b)

	if(m->tableViewAction->isChecked())
	{
		SetSetting(Set::Lib_ViewType, Library::ViewType::Standard);
	}

	else if(m->coverViewAction->isChecked())
	{
		SetSetting(Set::Lib_ViewType, Library::ViewType::CoverView);
	}

	else if(m->directoryViewAction->isChecked())
	{
		SetSetting(Set::Lib_ViewType, Library::ViewType::FileView);
	}
}

void MenuButtonViews::viewTypeChanged()
{
	Library::ViewType viewType = GetSetting(Set::Lib_ViewType);

	if(viewType == Library::ViewType::Standard)
	{
		m->tableViewAction->setChecked(true);
	}

	else if(viewType == Library::ViewType::CoverView)
	{
		m->coverViewAction->setChecked(true);
	}

	else if(viewType == Library::ViewType::FileView)
	{
		m->directoryViewAction->setChecked(true);
	}
}

void MenuButtonViews::shortcutChanged(ShortcutIdentifier identifier)
{
	if(identifier == ShortcutIdentifier::CoverView)
	{
		auto* shortcutHandler = ShortcutHandler::instance();
		QString shortcutString = shortcutHandler->shortcut(ShortcutIdentifier::CoverView).sequence().toString();
		m->shortcutSection->setText(shortcutString);
	}
}

MenuButtonViews::~MenuButtonViews() = default;

static void checkIcon(QPushButton* btn)
{
	QIcon icon = Gui::Icons::icon(Gui::Icons::Grid);
	if(icon.isNull())
	{
		btn->setText(QString::fromUtf8("≡"));
		btn->setIcon(QIcon());
	}

	else
	{
		btn->setText("");
		btn->setIcon(icon);
	}
}

void MenuButtonViews::languageChanged()
{
	Gui::MenuToolButton::languageChanged();

	m->tableViewAction->setText(Lang::get(Lang::Default));
	m->coverViewAction->setText(Lang::get(Lang::Covers));
	m->directoryViewAction->setText(Lang::get(Lang::Directories));

	checkIcon(this);
}

void MenuButtonViews::skinChanged()
{
	Gui::MenuToolButton::skinChanged();

	checkIcon(this);
}

