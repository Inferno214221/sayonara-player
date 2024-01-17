/* CoverViewContextMenu.cpp */

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

#include "CoverView.h"
#include "CoverViewContextMenu.h"
#include "CoverViewSortorderInfo.h"

#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>

using Library::CoverViewContextMenu;

struct CoverViewContextMenu::Private
{
	QAction* actionShowArtist;
	QAction* actionShowUtils;

	QMenu* menuSorting;
	QAction* actionSorting;

	QMenu* menuZoom;
	QAction* actionZoom;

	explicit Private(CoverViewContextMenu* menu) :
		actionShowArtist(new QAction(menu)),
		actionShowUtils(new QAction(menu)),
		menuSorting(new QMenu(menu)),
		menuZoom(new QMenu(menu))
	{
		actionShowArtist->setCheckable(true);
		actionShowArtist->setChecked(GetSetting(Set::Lib_CoverShowArtist));
		actionShowUtils->setCheckable(true);
		actionShowUtils->setChecked(GetSetting(Set::Lib_CoverShowUtils));

		menu->addPreferenceAction(new Gui::CoverPreferenceAction(menu));
		menu->addSeparator();

		// insert everything before the preferences
		auto* sepBeforePrefs = menu->beforePreferenceAction();
		menu->insertSeparator(sepBeforePrefs);
		menu->insertAction(sepBeforePrefs, actionShowArtist);
		menu->insertAction(sepBeforePrefs, actionShowUtils);

		this->actionSorting = menu->insertMenu(sepBeforePrefs, menuSorting);
		this->actionZoom = menu->insertMenu(sepBeforePrefs, menuZoom);
	}
};

CoverViewContextMenu::CoverViewContextMenu(QWidget* parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>(this);

	init();
}

CoverViewContextMenu::~CoverViewContextMenu() = default;

void CoverViewContextMenu::init()
{
	initSortingActions();
	initZoomActions();

	connect(m->actionShowArtist, &QAction::triggered, this, [=]() {
		SetSetting(Set::Lib_CoverShowArtist, m->actionShowArtist->isChecked());
	});

	connect(m->actionShowUtils, &QAction::triggered, this, [=]() {
		SetSetting(Set::Lib_CoverShowUtils, m->actionShowUtils->isChecked());
	});
}

void CoverViewContextMenu::initSortingActions()
{
	m->menuSorting->clear();
	m->actionSorting->setText(Lang::get(Lang::SortBy));

	static const auto sortingActions = CoverView::sortingActions();
	for(const auto& sortingAction: sortingActions)
	{
		auto* action = m->menuSorting->addAction(sortingAction.name());
		action->setCheckable(true);
		action->setData(static_cast<int>(sortingAction.sortorder));
		connect(action, &QAction::triggered, this, &CoverViewContextMenu::actionSortingTriggered);
	}
}

void CoverViewContextMenu::initZoomActions()
{
	m->menuZoom->clear();

	const static auto zoomFactors = CoverView::zoomFactors();
	for(const auto& zoomFactor: zoomFactors)
	{
		auto* action = m->menuZoom->addAction(QString::number(zoomFactor));
		action->setData(zoomFactor);
		action->setCheckable(true);
		connect(action, &QAction::triggered, this, &CoverViewContextMenu::actionZoomTriggered);
	}
}

void CoverViewContextMenu::actionZoomTriggered([[maybe_unused]] bool b)
{
	auto* action = dynamic_cast<QAction*>(sender());

	const auto zoom = action->data().toInt();
	emit sigZoomChanged(zoom);
}

void CoverViewContextMenu::actionSortingTriggered([[maybe_unused]] bool b)
{
	auto* action = dynamic_cast<QAction*>(sender());

	const auto sortorder = static_cast<Library::AlbumSortorder>(action->data().toInt());
	emit sigSortingChanged(sortorder);
}

CoverViewContextMenu::Entries CoverViewContextMenu::entries() const
{
	auto entries = Library::ContextMenu::entries();

	entries |= CoverViewContextMenu::EntryShowUtils;
	entries |= CoverViewContextMenu::EntrySorting;
	entries |= CoverViewContextMenu::EntryZoom;
	entries |= CoverViewContextMenu::EntryShowArtist;

	return entries;
}

void CoverViewContextMenu::showActions(CoverViewContextMenu::Entries entries)
{
	Library::ContextMenu::showActions(entries);

	m->actionShowUtils->setVisible(entries & CoverViewContextMenu::EntryShowUtils);
	m->actionSorting->setVisible(entries & CoverViewContextMenu::EntrySorting);
	m->actionZoom->setVisible(entries & CoverViewContextMenu::EntryZoom);
	m->actionShowArtist->setVisible(entries & CoverViewContextMenu::EntryShowArtist);
}

void CoverViewContextMenu::setZoom(int zoom)
{
	auto found = false;

	const auto actions = m->menuZoom->actions();
	for(auto* action: actions)
	{
		action->setChecked((action->text().toInt() >= zoom) && !found);
		found |= (action->text().toInt() >= zoom);
	}
}

void CoverViewContextMenu::setSorting(const Library::AlbumSortorder sortOrder)
{
	const auto actions = m->menuSorting->actions();
	for(auto* action: actions)
	{
		action->setChecked(action->data().toInt() == static_cast<int>(sortOrder));
	}
}

void CoverViewContextMenu::showEvent(QShowEvent* e)
{
	Library::ContextMenu::showEvent(e);

	setSorting(GetSetting(Set::Lib_Sorting).album);
	setZoom(GetSetting(Set::Lib_CoverZoom));
	m->actionShowUtils->setChecked(GetSetting(Set::Lib_CoverShowUtils));
	m->actionShowArtist->setChecked(GetSetting(Set::Lib_CoverShowArtist));
}

void CoverViewContextMenu::languageChanged()
{
	Library::ContextMenu::languageChanged();

	initSortingActions();

	m->actionZoom->setText(Lang::get(Lang::Zoom));
	m->actionShowUtils->setText(Lang::get(Lang::Show) + ": " + tr("Toolbar"));
	m->actionShowArtist->setText(Lang::get(Lang::Show) + ": " + Lang::get(Lang::AlbumArtist));

	m->menuSorting->clear();
	initSortingActions();
}
