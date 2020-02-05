/* PlaylistContextMenu.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
#include "PlaylistBookmarksMenu.h"

#include "Gui/Playlist/PlaylistActionMenu.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"

using Playlist::ContextMenu;
using Playlist::BookmarksMenu;
using Playlist::ActionMenu;

struct ContextMenu::Private
{
	QMap<ContextMenu::Entry, QAction*> entry_map;

	BookmarksMenu*	bookmarks_menu=nullptr;
	QMenu*			rating_menu=nullptr;

	QAction*		playlist_mode_action=nullptr;
	QMenu*			playlist_mode_menu=nullptr;

	Private(ContextMenu* parent)
	{
		rating_menu = new QMenu(parent);

		bookmarks_menu = new BookmarksMenu(parent);

		playlist_mode_menu = new ActionMenu(parent);
		playlist_mode_action = parent->addMenu(playlist_mode_menu);

		entry_map[EntryRating] = parent->addMenu(rating_menu);
		entry_map[EntryBookmarks] = parent->addMenu(bookmarks_menu);
		entry_map[EntryCurrentTrack] = new QAction(parent);
		entry_map[EntryFindInLibrary] = new QAction(parent);
		entry_map[EntryReverse] = new QAction(parent);

		parent->addActions
		({
			entry_map[EntryReverse],
			entry_map[EntryCurrentTrack],
			entry_map[EntryFindInLibrary],
		});
	}
};

ContextMenu::ContextMenu(QWidget *parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>(this);

	QList<QAction*> rating_actions;
	for(int i=int(Rating::Zero); i != int(Rating::Last); i++)
	{
		rating_actions << init_rating_action(Rating(i), m->rating_menu);
	}

	m->rating_menu->addActions(rating_actions);

	connect(m->entry_map[EntryCurrentTrack],	&QAction::triggered, this, &ContextMenu::sig_jump_to_current_track);
	connect(m->entry_map[EntryFindInLibrary],	&QAction::triggered, this, &ContextMenu::sig_find_track_triggered);
	connect(m->entry_map[EntryReverse],			&QAction::triggered, this, &ContextMenu::sig_reverse_triggered);

	connect(m->bookmarks_menu, &BookmarksMenu::sig_bookmark_pressed, this, &ContextMenu::bookmark_pressed);

	skin_changed();
}

ContextMenu::~ContextMenu() = default;

ContextMenu::Entries ContextMenu::get_entries() const
{
	ContextMenu::Entries entries = Library::ContextMenu::get_entries();

	for(auto it=m->entry_map.begin(); it != m->entry_map.end(); it++)
	{
		if(it.value()->isVisible()) {
			entries |= it.key();
		}
	}

	return entries;
}

void ContextMenu::show_actions(ContextMenu::Entries entries)
{
	Library::ContextMenu::show_actions(entries);

	for(auto it=m->entry_map.begin(); it != m->entry_map.end(); it++)
	{
		it.value()->setVisible(entries & it.key());
	}
}

void ContextMenu::set_rating(Rating rating)
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

		m->entry_map[EntryRating]->setText(text);
	}

	else {
		m->entry_map[EntryRating]->setText(rating_text);
	}
}

void ContextMenu::set_metadata(const MetaData& md)
{
	m->bookmarks_menu->set_metadata(md);
}

QAction* ContextMenu::init_rating_action(Rating rating, QObject* parent)
{
	auto* action = new QAction
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

void ContextMenu::language_changed()
{
	Library::ContextMenu::language_changed();

	m->entry_map[EntryRating]-> setText(Lang::get(Lang::Rating));
	m->entry_map[EntryBookmarks]-> setText(Lang::get(Lang::Bookmarks));
	m->entry_map[EntryCurrentTrack]->setText(tr("Jump to current track"));
	m->entry_map[EntryFindInLibrary]->setText(tr("Show track in library"));
	m->entry_map[EntryReverse]->setText(tr("Reverse"));

	m->playlist_mode_action->setText(tr("Playlist mode"));
}

void ContextMenu::skin_changed()
{
	Library::ContextMenu::skin_changed();

	using namespace Gui;
	m->entry_map[EntryRating]->setIcon(Icons::icon(Icons::Star));
	m->entry_map[EntryFindInLibrary]->setIcon(Icons::icon(Icons::Search));
}

void ContextMenu::bookmark_pressed(Seconds timestamp)
{
   emit sig_bookmark_pressed(timestamp);
}
