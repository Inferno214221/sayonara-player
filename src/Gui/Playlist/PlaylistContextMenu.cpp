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

#include "Components/DynamicPlayback/DynamicPlaybackChecker.h"
#include "Gui/Playlist/PlaylistActionMenu.h"
#include "Gui/Playlist/SortingMenu.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/globals.h"

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

	QAction* initRatingAction(const Rating rating, QObject* parent)
	{
		auto* action = new QAction(QString::number(static_cast<int>(rating)), parent);

		action->setData(QVariant::fromValue(rating));
		action->setCheckable(true);
		action->setProperty("rating", QVariant::fromValue(rating));

		return action;
	}

	void setRating(const Rating rating, QMenu* ratingMenu, QAction* ratingAction)
	{
		const auto actions = ratingMenu->actions();
		for(auto* action: actions)
		{
			const auto data = action->property("rating").value<Rating>();
			action->setChecked(data == rating);
		}

		const auto ratingText = Lang::get(Lang::Rating);

		const auto text = (rating != Rating::Zero && rating != Rating::Last)
		                  ? QString("%1 (%2)").arg(ratingText).arg(+rating)
		                  : static_cast<QString>(ratingText);

		ratingAction->setText(text);
		ratingAction->setProperty("rating", QVariant::fromValue(rating));
	}

	template<typename ContextMenuType, typename EntryType>
	void
	initShortcut(ContextMenuType* menu, QWidget* parentWidget, const EntryType entry, const QKeySequence& keySequence)
	{
		auto* action = menu->action(entry);
		action->setShortcut(keySequence);
		action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
		parentWidget->addAction(action);
	}

	void configureShortcuts(ContextMenu* menu, QWidget* parent)
	{
		initShortcut(menu, parent, Library::ContextMenu::EntryPlay, {Qt::Key_Return});
		initShortcut(menu, parent, Library::ContextMenu::EntryClear, {Qt::Key_Backspace});
		initShortcut(menu, parent, Library::ContextMenu::EntryRemove, QKeySequence::Delete);
		initShortcut(menu, parent, Library::ContextMenu::EntryDelete, {Qt::ShiftModifier + Qt::Key_Delete});

		initShortcut(menu, parent, ContextMenu::EntryCurrentTrack, {Qt::ControlModifier | Qt::Key_J});
		initShortcut(menu, parent, ContextMenu::EntryFindInLibrary, {Qt::ControlModifier | Qt::Key_G});
		initShortcut(menu, parent, ContextMenu::EntryRandomize, {Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_R});
		initShortcut(menu, parent, ContextMenu::EntryReverse, {Qt::ControlModifier | Qt::Key_R});
		initShortcut(menu, parent, ContextMenu::EntryJumpToNextAlbum, {Qt::ControlModifier | Qt::Key_N});
	}
}// namespace

struct ContextMenu::Private
{
	QMap<ContextMenu::Entry, QAction*> entryActionMap;

	QMenu* ratingMenu;
	BookmarksMenu* bookmarksMenu;
	QMenu* playlistModeMenu;
	QAction* playlistModeAction;
	SortingMenu* sortingMenu;

	Private(DynamicPlaybackChecker* dynamicPlaybackChecker, ContextMenu* contextMenu) :
		ratingMenu {new QMenu(contextMenu)},
		bookmarksMenu {new BookmarksMenu(contextMenu)},
		playlistModeMenu {new ActionMenu(dynamicPlaybackChecker, contextMenu)},
		sortingMenu {new SortingMenu(contextMenu)}
	{
		entryActionMap[EntrySort] = contextMenu->addMenu(sortingMenu);
		entryActionMap[EntryReverse] = contextMenu->addAction(QString());
		entryActionMap[EntryRandomize] = contextMenu->addAction(QString());
		contextMenu->addSeparator();
		entryActionMap[EntryCurrentTrack] = contextMenu->addAction(QString());
		entryActionMap[EntryJumpToNextAlbum] = contextMenu->addAction(QString());
		entryActionMap[EntryFindInLibrary] = contextMenu->addAction(QString());
		contextMenu->addSeparator();
		entryActionMap[EntryRating] = contextMenu->addMenu(ratingMenu);
		entryActionMap[EntryBookmarks] = contextMenu->addMenu(bookmarksMenu);
		playlistModeAction = contextMenu->addMenu(playlistModeMenu); // NOLINT(cppcoreguidelines-prefer-member-initializer)
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

	connect(m->bookmarksMenu, &BookmarksMenu::sigBookmarkPressed, this, &ContextMenu::sigBookmarkTriggered);
	connect(m->sortingMenu, &SortingMenu::sigSortingTriggered, this, &ContextMenu::sigSortingTriggered);

	configureShortcuts(this, parent);
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

ContextMenu::Entries ContextMenu::setTrack(const MetaData& track, const bool isCurrentTrack)
{
	const auto isLibraryTrack = (track.id() >= 0);

	m->bookmarksMenu->setTrack(track, (isCurrentTrack && isLibraryTrack));
	setRating(track.rating(), m->ratingMenu, m->entryActionMap[EntryRating]);

	return analyzeTrack(track);
}

void ContextMenu::clearTrack()
{
	m->bookmarksMenu->setTrack(MetaData(), false);
	setRating(Rating::Last, m->ratingMenu, m->entryActionMap[EntryRating]);
}

void ContextMenu::languageChanged()
{
	Library::ContextMenu::languageChanged();

	m->entryActionMap[EntryRating]->setText(Lang::get(Lang::Rating));
	m->entryActionMap[EntryBookmarks]->setText(Lang::get(Lang::Bookmarks));
	m->entryActionMap[EntryCurrentTrack]->setText(tr("Jump to current track"));
	m->entryActionMap[EntryFindInLibrary]->setText(tr("Show track in library"));
	m->entryActionMap[EntryReverse]->setText(Lang::get(Lang::ReverseOrder));
	m->entryActionMap[EntryRandomize]->setText(Lang::get(Lang::ShufflePlaylist));
	m->entryActionMap[EntryJumpToNextAlbum]->setText(tr("Jump to next album"));
	m->entryActionMap[EntrySort]->setText(Lang::get(Lang::SortBy));
	m->playlistModeAction->setText(tr("Playlist mode"));

	configureShortcuts(this, parentWidget());
}

void ContextMenu::skinChanged()
{
	Library::ContextMenu::skinChanged();

	using namespace Gui;
	m->entryActionMap[EntryRating]->setIcon(Icons::icon(Icons::Star));
	m->entryActionMap[EntryFindInLibrary]->setIcon(Icons::icon(Icons::Search));
}

QAction* Playlist::ContextMenu::action(const ContextMenu::Entry entry) const
{
	return m->entryActionMap.contains(entry)
	       ? m->entryActionMap[entry]
	       : nullptr;
}
