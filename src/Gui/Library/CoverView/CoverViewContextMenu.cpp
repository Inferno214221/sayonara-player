/* CoverViewContextMenu.cpp */

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

#include "CoverView.h"
#include "CoverViewContextMenu.h"
#include "Gui/Library/Header/ActionPair.h"

#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>

using Library::CoverViewContextMenu;
using Library::ActionPair;
using ActionPairList=QList<ActionPair>;

struct CoverViewContextMenu::Private
{
	QMenu*		menuSorting=nullptr;
	QAction*	actionSorting=nullptr;

	QMenu*		menuZoom=nullptr;
	QAction*	actionZoom=nullptr;

	QAction*	actionShowUtils=nullptr;
	QAction*	actionShowArtist=nullptr;

	QStringList		zoomActions;
	ActionPairList	sortingActions;

	Private() :
		zoomActions(Library::CoverView::zoomActions()),
		sortingActions(Library::CoverView::sortingActions())
	{}
};

CoverViewContextMenu::CoverViewContextMenu(QWidget* parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>();

	init();
}

CoverViewContextMenu::~CoverViewContextMenu() = default;


void CoverViewContextMenu::init()
{
	this->addPreferenceAction(new Gui::CoverPreferenceAction(this));
	this->addSeparator();

	// insert everything before the preferences
	QAction* sepBeforePrefs = this->beforePreferenceAction();
	this->insertSeparator(sepBeforePrefs);

	m->actionShowArtist = new QAction(this);
	m->actionShowArtist->setCheckable(true);
	m->actionShowArtist->setChecked(GetSetting(Set::Lib_CoverShowArtist));
	this->insertAction(sepBeforePrefs, m->actionShowArtist);

	connect(m->actionShowArtist, &QAction::triggered, this, [=](){
		SetSetting(Set::Lib_CoverShowArtist, m->actionShowArtist->isChecked());
	});

	m->actionShowUtils = new QAction(this);
	m->actionShowUtils->setCheckable(true);
	m->actionShowUtils->setChecked(GetSetting(Set::Lib_CoverShowUtils));
	this->insertAction(sepBeforePrefs, m->actionShowUtils);

	connect(m->actionShowUtils, &QAction::triggered, this, [=](){
		SetSetting(Set::Lib_CoverShowUtils, m->actionShowUtils->isChecked());
	});

	m->menuSorting = new QMenu(this);
	m->actionSorting = this->insertMenu(sepBeforePrefs, m->menuSorting);
	initSortingActions();

	m->menuZoom  = new QMenu(this);
	m->actionZoom = this->insertMenu(sepBeforePrefs, m->menuZoom);
	initZoomActions();
}

void CoverViewContextMenu::initSortingActions()
{
	m->menuSorting->clear();
	m->actionSorting->setText(Lang::get(Lang::SortBy));

	for(const ActionPair& ap : m->sortingActions)
	{
		QAction* a = m->menuSorting->addAction(ap.name());

		a->setCheckable(true);
		a->setData((int) ap.sortorder());

		connect(a, &QAction::triggered, this, &CoverViewContextMenu::actionSortingTriggered);
	}
}

void CoverViewContextMenu::initZoomActions()
{
	m->menuZoom->clear();

	for(const QString& z : m->zoomActions)
	{
		QAction* action = m->menuZoom->addAction(z);
		action->setData(z.toInt());
		action->setCheckable(true);

		connect(action, &QAction::triggered, this, &CoverViewContextMenu::actionZoomTriggered);
	}
}


void CoverViewContextMenu::actionZoomTriggered(bool b)
{
	Q_UNUSED(b)
	auto* action = static_cast<QAction*>(sender());

	int zoom = action->data().toInt();
	emit sigZoomChanged(zoom);
}

void CoverViewContextMenu::actionSortingTriggered(bool b)
{
	Q_UNUSED(b)
	auto* action = static_cast<QAction*>(sender());

	Library::SortOrder so = Library::SortOrder(action->data().toInt());
	emit sigSortingChanged(so);
}


CoverViewContextMenu::Entries CoverViewContextMenu::entries() const
{
	CoverViewContextMenu::Entries entries = Library::ContextMenu::entries();
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
	bool found=false;

	const QList<QAction*> actions = m->menuZoom->actions();
	for(QAction* a : actions)
	{
		a->setChecked( (a->text().toInt() >= zoom) && !found );
		if(a->text().toInt() >= zoom)
		{
			found = true;
		}
	}
}

void CoverViewContextMenu::setSorting(Library::SortOrder so)
{
	const QList<QAction*> actions = m->menuSorting->actions();
	for(QAction* a : actions)
	{
		a->setChecked(a->data().toInt() == static_cast<int>(so));
	}
}

void CoverViewContextMenu::showEvent(QShowEvent* e)
{
	Library::ContextMenu::showEvent(e);

	setSorting(GetSetting(Set::Lib_Sorting).so_albums);
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
