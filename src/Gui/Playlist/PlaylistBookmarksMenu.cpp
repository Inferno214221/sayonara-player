/* BookmarksMenu.cpp */

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

#include "PlaylistBookmarksMenu.h"

#include "Components/Bookmarks/Bookmarks.h"
#include "Components/Bookmarks/Bookmark.h"

#include "Gui/Plugins/PlayerPluginHandler.h"
#include "Gui/Utils/Icons.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Language/Language.h"

namespace Playlist
{
	struct BookmarksMenu::Private
	{
		PlayerPlugin::Handler* playerPluginHandler {PlayerPlugin::Handler::instance()};
		BookmarkStoragePtr bookmarkStorage {BookmarkStorage::create()};
	};

	BookmarksMenu::BookmarksMenu(QWidget* parent) :
		QMenu(parent),
		m {Pimpl::make<Private>()}
	{
		setTitle(Lang::get(Lang::Bookmarks));
	}

	BookmarksMenu::~BookmarksMenu() = default;

	bool BookmarksMenu::hasBookmarks() const { return !actions().isEmpty(); }

	void BookmarksMenu::setTrack(const MetaData& track, bool editAllowed)
	{
		m->bookmarkStorage->setTrack(track);
		bookmarksChanged(editAllowed);
	}

	MetaData BookmarksMenu::track() const { return m->bookmarkStorage->track(); }

	void BookmarksMenu::bookmarksChanged(const bool editAllowed)
	{
		for(auto* a: actions())
		{
			a->deleteLater();
		}

		clear();

		const auto& bookmarks = m->bookmarkStorage->bookmarks();
		for(const auto& bookmark: bookmarks)
		{
			const auto name = bookmark.name();
			if(!name.isEmpty())
			{
				auto* action = addAction(name);
				action->setData(bookmark.timestamp());
				connect(action, &QAction::triggered, this, &BookmarksMenu::actionPressed);
			}
		}

		addSeparator();

		auto* editAction = new QAction(Gui::Icons::icon(Gui::Icons::Edit), Lang::get(Lang::Edit), this);
		addAction(editAction);

		connect(editAction, &QAction::triggered, [this]() {
			m->playerPluginHandler->showPlugin("Bookmarks");
		});

		editAction->setEnabled(editAllowed);
	}

	void BookmarksMenu::actionPressed()
	{
		auto* action = dynamic_cast<QAction*>(sender());
		const auto time = static_cast<Seconds>(action->data().toInt());

		emit sigBookmarkPressed(time);
	}
}