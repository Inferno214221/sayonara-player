/* PlaylistContextMenu.cpp */

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

#include "PlaylistContextMenu.h"
#include "PlaylistBookmarksMenu.h"

#include "Gui/Playlist/PlaylistActionMenu.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Widgets/RatingLabel.h"

#include "Interfaces/DynamicPlayback.h"

#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"

using Playlist::ContextMenu;
using Playlist::BookmarksMenu;
using Playlist::ActionMenu;

namespace
{
	ContextMenu::Entries analyzeTrack(const MetaData& track)
	{
		const auto isLibraryTrack = (track.id() >= 0);
		const auto isLocalTrack = (track.radioMode() == RadioMode::Off);

		return ((isLibraryTrack) ? ContextMenu::EntryBookmarks : 0) |
		       ((isLibraryTrack) ? ContextMenu::EntryFindInLibrary : 0) |
		       ((isLocalTrack) ? ContextMenu::EntryRating : 0) |
		       ContextMenu::EntryLyrics;
	}
}

struct ContextMenu::Private
{
	QMap<ContextMenu::Entry, QAction*> entryActionMap;

	QMenu* ratingMenu;
	BookmarksMenu* bookmarksMenu;
	QMenu* playlistModeMenu;
	QAction* playlistModeAction;

	Private(DynamicPlaybackChecker* dynamicPlaybackChecker, ContextMenu* parent) :
		ratingMenu {new QMenu(parent)},
		bookmarksMenu {new BookmarksMenu(parent)},
		playlistModeMenu {new ActionMenu(dynamicPlaybackChecker, parent)},
		playlistModeAction {parent->addMenu(playlistModeMenu)}
	{
		entryActionMap[EntryRating] = parent->addMenu(ratingMenu);
		entryActionMap[EntryBookmarks] = parent->addMenu(bookmarksMenu);
		entryActionMap[EntryCurrentTrack] = new QAction(parent);
		entryActionMap[EntryFindInLibrary] = new QAction(parent);
		entryActionMap[EntryReverse] = new QAction(parent);

		parent->addActions
			({
				 entryActionMap[EntryReverse],
				 entryActionMap[EntryCurrentTrack],
				 entryActionMap[EntryFindInLibrary],
			 });
	}
};

ContextMenu::ContextMenu(DynamicPlaybackChecker* dynamicPlaybackChecker, QWidget* parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>(dynamicPlaybackChecker, this);

	QList<QAction*> ratingActions;
	for(auto i = +Rating::Zero; i != +Rating::Last; i++)
	{
		ratingActions << initRatingAction(static_cast<Rating>(i), m->ratingMenu);
	}

	m->ratingMenu->addActions(ratingActions);

	connect(m->entryActionMap[EntryCurrentTrack], &QAction::triggered, this, &ContextMenu::sigJumpToCurrentTrack);
	connect(m->entryActionMap[EntryFindInLibrary], &QAction::triggered, this, &ContextMenu::sigFindTrackTriggered);
	connect(m->entryActionMap[EntryReverse], &QAction::triggered, this, &ContextMenu::sigReverseTriggered);

	connect(m->bookmarksMenu, &BookmarksMenu::sigBookmarkPressed, this, &ContextMenu::bookmarkPressed);

	skinChanged();
}

ContextMenu::~ContextMenu() = default;

ContextMenu::Entries ContextMenu::entries() const
{
	auto entries = Library::ContextMenu::entries();

	for(auto it = m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		if(it.value()->isVisible())
		{
			entries |= it.key();
		}
	}

	return entries;
}

void ContextMenu::showActions(ContextMenu::Entries entries)
{
	Library::ContextMenu::showActions(entries);

	for(auto it = m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		const auto isVisible = (entries & it.key());
		it.value()->setVisible(isVisible);
	}
}

void ContextMenu::setRating(Rating rating)
{
	const auto actions = m->ratingMenu->actions();
	for(auto* action : actions)
	{
		const auto data = action->data().value<Rating>();
		action->setChecked(data == rating);
	}

	const auto ratingText = Lang::get(Lang::Rating);

	const auto text = (rating != Rating::Zero && rating != Rating::Last)
	                  ? QString("%1 (%2)").arg(ratingText).arg(+rating)
	                  : static_cast<QString>(ratingText);

	m->entryActionMap[EntryRating]->setText(text);
}

ContextMenu::Entries ContextMenu::setTrack(const MetaData& track, bool isCurrentTrack)
{
	const auto isLibraryTrack = (track.id() >= 0);

	m->bookmarksMenu->setTrack(track, (isCurrentTrack && isLibraryTrack));
	setRating(track.rating());

	return analyzeTrack(track);
}

void ContextMenu::clearTrack()
{
	m->bookmarksMenu->setTrack(MetaData(), false);
	setRating(Rating::Last);
}

QAction* ContextMenu::initRatingAction(Rating rating, QObject* parent)
{
	auto* action = new QAction(QString::number(+rating), parent);

	action->setData(QVariant::fromValue(rating));
	action->setCheckable(true);

	connect(action, &QAction::triggered, this, [&]([[maybe_unused]] const auto b) {
		emit sigRatingChanged(rating);
	});

	return action;
}

void ContextMenu::languageChanged()
{
	Library::ContextMenu::languageChanged();

	m->entryActionMap[EntryRating]->setText(Lang::get(Lang::Rating));
	m->entryActionMap[EntryBookmarks]->setText(Lang::get(Lang::Bookmarks));
	m->entryActionMap[EntryCurrentTrack]->setText(tr("Jump to current track") + QString("    "));
	m->entryActionMap[EntryFindInLibrary]->setText(tr("Show track in library") + QString("    "));
	m->entryActionMap[EntryReverse]->setText(Lang::get(Lang::ReverseOrder));
	m->playlistModeAction->setText(tr("Playlist mode"));

	m->entryActionMap[EntryCurrentTrack]->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_J));
	m->entryActionMap[EntryFindInLibrary]->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_G));
}

void ContextMenu::skinChanged()
{
	Library::ContextMenu::skinChanged();

	using namespace Gui;
	m->entryActionMap[EntryRating]->setIcon(Icons::icon(Icons::Star));
	m->entryActionMap[EntryFindInLibrary]->setIcon(Icons::icon(Icons::Search));
}

void ContextMenu::bookmarkPressed(Seconds timestamp)
{
	emit sigBookmarkPressed(timestamp);
}
