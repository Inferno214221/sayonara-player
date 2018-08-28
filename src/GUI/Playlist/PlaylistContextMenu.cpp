/* PlaylistContextMenu.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#include "PlaylistContextMenu.h"
#include "BookmarksMenu.h"
#include "GUI/Utils/RatingLabel.h"
#include "GUI/Utils/Icons.h"

#include "Utils/Language.h"
#include "Utils/MetaData/MetaData.h"


struct PlaylistContextMenu::Private
{
	BookmarksMenu*	bookmarks_menu=nullptr;
	QAction*		bookmarks_action=nullptr;
	QAction*		rating_action=nullptr;
	QMenu*			rating_menu=nullptr;

	Private(PlaylistContextMenu* parent)
	{
		rating_menu = new QMenu(parent);
		rating_action = parent->addMenu(rating_menu);

		bookmarks_menu = new BookmarksMenu(parent);
		bookmarks_action = parent->addMenu(bookmarks_menu);
	}
};

PlaylistContextMenu::PlaylistContextMenu(QWidget *parent) :
	LibraryContextMenu(parent)
{
	m = Pimpl::make<Private>(this);

	QList<QAction*> rating_actions;
	for(Rating i=0; i<=5; i++)
	{
		rating_actions << init_rating_action(i, m->rating_menu);
	}

	m->rating_menu->addActions(rating_actions);

	connect(m->bookmarks_menu, &BookmarksMenu::sig_bookmark_pressed, this, &PlaylistContextMenu::bookmark_pressed);

	skin_changed();
}

PlaylistContextMenu::~PlaylistContextMenu() {}

PlaylistContextMenu::Entries PlaylistContextMenu::get_entries() const
{
	PlaylistContextMenu::Entries entries = LibraryContextMenu::get_entries();
	if(m->bookmarks_action->isVisible()){
		entries |= PlaylistContextMenu::EntryBookmarks;
	}

	if(m->rating_action->isVisible()){
		entries |= PlaylistContextMenu::EntryRating;
	}

	return entries;
}

void PlaylistContextMenu::show_actions(PlaylistContextMenu::Entries entries)
{
	LibraryContextMenu::show_actions(entries);

	m->rating_action->setVisible(entries & PlaylistContextMenu::EntryRating);
	m->bookmarks_action->setVisible((entries & PlaylistContextMenu::EntryBookmarks) && m->bookmarks_menu->has_bookmarks());
}

void PlaylistContextMenu::set_rating(Rating rating)
{
	QList<QAction*> actions = m->rating_menu->actions();
	for(QAction* action : actions){
		int data = action->data().toInt();
		action->setChecked(data == rating);
	}

	QString rating_text = Lang::get(Lang::Rating);
	if(rating > 0){
		m->rating_action->setText(rating_text + " (" + QString::number(rating) + ")");
	}

	else{
		m->rating_action->setText(rating_text);
	}
}

void PlaylistContextMenu::set_metadata(const MetaData& md)
{
	m->bookmarks_menu->set_metadata(md);
}

QAction* PlaylistContextMenu::init_rating_action(Rating rating, QObject* parent)
{
	QAction* action = new QAction(QString::number(rating), parent);
	action->setData(rating);
	action->setCheckable(true);

	connect(action, &QAction::triggered, this, [=](bool b)
	{
		Q_UNUSED(b)
		emit sig_rating_changed(rating);
	});

	return action;
}

void PlaylistContextMenu::language_changed()
{
	LibraryContextMenu::language_changed();
	m->rating_action->setText(Lang::get(Lang::Rating));
}

void PlaylistContextMenu::skin_changed()
{
	LibraryContextMenu::skin_changed();

	using namespace Gui;
	m->rating_action->setIcon(Icons::icon(Icons::Star));
}

void PlaylistContextMenu::bookmark_pressed(Seconds timestamp)
{
   emit sig_bookmark_pressed(timestamp);
}

