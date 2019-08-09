/* PlaylistContextMenu.cpp */

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

#include "PlaylistContextMenu.h"
#include "BookmarksMenu.h"
#include "Gui/Utils/RatingLabel.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Playlist/PlaylistActionMenu.h"

#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"

struct PlaylistContextMenu::Private
{
	QAction*		current_track_action=nullptr;
	BookmarksMenu*	bookmarks_menu=nullptr;
	QAction*		bookmarks_action=nullptr;
	QAction*		rating_action=nullptr;
	QAction*		find_track_action=nullptr;
	QMenu*			rating_menu=nullptr;

	QAction*		playlist_mode_action=nullptr;
	QMenu*			playlist_mode_menu=nullptr;

	Private(PlaylistContextMenu* parent)
	{
		current_track_action = new QAction(parent);
		find_track_action = new QAction(parent);

		parent->addActions({current_track_action, find_track_action});

		rating_menu = new QMenu(parent);
		rating_action = parent->addMenu(rating_menu);

		bookmarks_menu = new BookmarksMenu(parent);
		bookmarks_action = parent->addMenu(bookmarks_menu);

		playlist_mode_menu = new PlaylistActionMenu(parent);
		playlist_mode_action = parent->addMenu(playlist_mode_menu);
	}
};

PlaylistContextMenu::PlaylistContextMenu(QWidget *parent) :
	Gui::LibraryContextMenu(parent)
{
	m = Pimpl::make<Private>(this);


	QList<QAction*> rating_actions;

	for(int i=int(Rating::Zero); i != int(Rating::Last); i++)
	{
		rating_actions << init_rating_action(Rating(i), m->rating_menu);
	}

	m->rating_menu->addActions(rating_actions);

	connect(m->bookmarks_menu, &BookmarksMenu::sig_bookmark_pressed, this, &PlaylistContextMenu::bookmark_pressed);
	connect(m->current_track_action, &QAction::triggered, this, &PlaylistContextMenu::sig_jump_to_current_track);
	connect(m->find_track_action, &QAction::triggered, this, &PlaylistContextMenu::sig_find_track_triggered);

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

	if(m->current_track_action->isVisible()){
		entries |= PlaylistContextMenu::EntryCurrentTrack;
	}

	if(m->find_track_action->isVisible()){
		entries |= PlaylistContextMenu::EntryFindInLibrary;
	}

	return entries;
}

void PlaylistContextMenu::show_actions(PlaylistContextMenu::Entries entries)
{
	LibraryContextMenu::show_actions(entries);

	m->rating_action->setVisible(entries & PlaylistContextMenu::EntryRating);
	m->bookmarks_action->setVisible((entries & PlaylistContextMenu::EntryBookmarks) && m->bookmarks_menu->has_bookmarks());
	m->current_track_action->setVisible(entries & PlaylistContextMenu::EntryCurrentTrack);
	m->find_track_action->setVisible(entries & PlaylistContextMenu::EntryFindInLibrary);
}

void PlaylistContextMenu::set_rating(Rating rating)
{
	QList<QAction*> actions = m->rating_menu->actions();
	for(QAction* action : actions)
	{
		auto data = action->data().value<Rating>();
		action->setChecked(data == rating);
	}

	QString rating_text = Lang::get(Lang::Rating);
	if(rating != Rating::Zero && rating != Rating::Last)
	{
		QString text = QString("%1 (%2)")
							.arg(rating_text)
							.arg(int(rating));

		m->rating_action->setText(text);
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
	QAction* action = new QAction
	(
		QString::number(int(rating)),
		parent
	);

	action->setData(QVariant::fromValue(rating));
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
	m->current_track_action->setText(tr("Jump to current track"));
	m->find_track_action->setText(tr("Show track in library"));
	m->playlist_mode_action->setText(tr("Playlist mode"));
}

void PlaylistContextMenu::skin_changed()
{
	LibraryContextMenu::skin_changed();

	using namespace Gui;
	m->rating_action->setIcon(Icons::icon(Icons::Star));
	m->find_track_action->setIcon(Icons::icon(Icons::Search));
}

void PlaylistContextMenu::bookmark_pressed(Seconds timestamp)
{
   emit sig_bookmark_pressed(timestamp);
}

