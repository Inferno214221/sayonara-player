/* CoverViewContextMenu.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "CoverViewContextMenu.h"
#include "CoverView.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Library/Header/ActionPair.h"

#include <QStringList>

using Library::CoverViewContextMenu;
using Library::ActionPair;
using ActionPairList=QList<ActionPair>;

struct CoverViewContextMenu::Private
{
	QMenu*		menu_sorting=nullptr;
	QAction*	action_sorting=nullptr;

	QMenu*		menu_zoom=nullptr;
	QAction*	action_zoom=nullptr;

	QAction*	action_show_utils=nullptr;
	QAction*	action_show_artist=nullptr;

	QStringList		zoom_actions;
	ActionPairList	sorting_actions;

	Private() :
		zoom_actions(Library::CoverView::zoom_actions()),
		sorting_actions(Library::CoverView::sorting_actions())
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
	this->add_preference_action(new Gui::CoverPreferenceAction(this));
	this->addSeparator();

	// insert everything before the preferences
	QAction* sep_before_prefs = this->before_preference_action();
	this->insertSeparator(sep_before_prefs);

	m->action_show_artist = new QAction(this);
	m->action_show_artist->setCheckable(true);
	m->action_show_artist->setChecked(GetSetting(Set::Lib_CoverShowArtist));
	this->insertAction(sep_before_prefs, m->action_show_artist);

	connect(m->action_show_artist, &QAction::triggered, this, [=](){
		SetSetting(Set::Lib_CoverShowArtist, m->action_show_artist->isChecked());
	});

	m->action_show_utils = new QAction(this);
	m->action_show_utils->setCheckable(true);
	m->action_show_utils->setChecked(GetSetting(Set::Lib_CoverShowUtils));
	this->insertAction(sep_before_prefs, m->action_show_utils);

	connect(m->action_show_utils, &QAction::triggered, this, [=](){
		SetSetting(Set::Lib_CoverShowUtils, m->action_show_utils->isChecked());
	});

	m->menu_sorting = new QMenu(this);
	m->action_sorting = this->insertMenu(sep_before_prefs, m->menu_sorting);
	init_sorting_actions();

	m->menu_zoom  = new QMenu(this);
	m->action_zoom = this->insertMenu(sep_before_prefs, m->menu_zoom);
	init_zoom_actions();
}

void CoverViewContextMenu::init_sorting_actions()
{
	m->menu_sorting->clear();
	m->action_sorting->setText(Lang::get(Lang::SortBy));

	for(const ActionPair& ap : m->sorting_actions)
	{
		QAction* a = m->menu_sorting->addAction(ap.name());

		a->setCheckable(true);
		a->setData((int) ap.sortorder());

		connect(a, &QAction::triggered, this, &CoverViewContextMenu::action_sorting_triggered);
	}
}

void CoverViewContextMenu::init_zoom_actions()
{
	m->menu_zoom->clear();

	for(const QString& z : m->zoom_actions)
	{
		QAction* action = m->menu_zoom->addAction(z);
		action->setData(z.toInt());
		action->setCheckable(true);

		connect(action, &QAction::triggered, this, &CoverViewContextMenu::action_zoom_triggered);
	}
}


void CoverViewContextMenu::action_zoom_triggered(bool b)
{
	Q_UNUSED(b)
	auto* action = static_cast<QAction*>(sender());

	int zoom = action->data().toInt();
	emit sig_zoom_changed(zoom);
}

void CoverViewContextMenu::action_sorting_triggered(bool b)
{
	Q_UNUSED(b)
	auto* action = static_cast<QAction*>(sender());

	Library::SortOrder so = Library::SortOrder(action->data().toInt());
	emit sig_sorting_changed(so);
}


CoverViewContextMenu::Entries CoverViewContextMenu::get_entries() const
{
	CoverViewContextMenu::Entries entries = Library::ContextMenu::get_entries();
	entries |= CoverViewContextMenu::EntryShowUtils;
	entries |= CoverViewContextMenu::EntrySorting;
	entries |= CoverViewContextMenu::EntryZoom;
	entries |= CoverViewContextMenu::EntryShowArtist;

	return entries;
}

void CoverViewContextMenu::show_actions(CoverViewContextMenu::Entries entries)
{
	Library::ContextMenu::show_actions(entries);

	m->action_show_utils->setVisible(entries & CoverViewContextMenu::EntryShowUtils);
	m->action_sorting->setVisible(entries & CoverViewContextMenu::EntrySorting);
	m->action_zoom->setVisible(entries & CoverViewContextMenu::EntryZoom);
	m->action_show_artist->setVisible(entries & CoverViewContextMenu::EntryShowArtist);
}

void CoverViewContextMenu::set_zoom(int zoom)
{
	bool found=false;

	const QList<QAction*> actions = m->menu_zoom->actions();
	for(QAction* a : actions)
	{
		a->setChecked( (a->text().toInt() >= zoom) && !found );
		if(a->text().toInt() >= zoom)
		{
			found = true;
		}
	}
}

void CoverViewContextMenu::set_sorting(Library::SortOrder so)
{
	const QList<QAction*> actions = m->menu_sorting->actions();
	for(QAction* a : actions)
	{
		a->setChecked(a->data().toInt() == static_cast<int>(so));
	}
}

void CoverViewContextMenu::showEvent(QShowEvent* e)
{
	Library::ContextMenu::showEvent(e);

	set_sorting(GetSetting(Set::Lib_Sorting).so_albums);
	set_zoom(GetSetting(Set::Lib_CoverZoom));
	m->action_show_utils->setChecked(GetSetting(Set::Lib_CoverShowUtils));
	m->action_show_artist->setChecked(GetSetting(Set::Lib_CoverShowArtist));
}


void CoverViewContextMenu::language_changed()
{
	Library::ContextMenu::language_changed();

	init_sorting_actions();

	m->action_zoom->setText(Lang::get(Lang::Zoom));
	m->action_show_utils->setText(Lang::get(Lang::Show) + ": " + tr("Toolbar"));
	m->action_show_artist->setText(Lang::get(Lang::Show) + ": " + Lang::get(Lang::Artist));

	m->menu_sorting->clear();
	init_sorting_actions();
}
