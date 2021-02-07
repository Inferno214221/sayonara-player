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

#include "Gui/Plugins/PlayerPluginHandler.h"
#include "Gui/Utils/Icons.h"

#include "Components/Bookmarks/Bookmarks.h"
#include "Components/Bookmarks/Bookmark.h"
#include "Components/PlayManager/PlayManager.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Language/Language.h"

using Playlist::BookmarksMenu;

struct BookmarksMenu::Private
{
	PlayManager* playManager;
	PlayerPlugin::Handler* playerPluginHandler;
	BookmarkStorage bookmarks;

	Private() :
		playManager {PlayManagerProvider::instance()->playManager()},
		playerPluginHandler {PlayerPlugin::Handler::instance()}
	{}
};

BookmarksMenu::BookmarksMenu(QWidget* parent) :
	QMenu(parent)
{
	m = Pimpl::make<Private>();

	this->setTitle(Lang::get(Lang::Bookmarks));
}

BookmarksMenu::~BookmarksMenu() = default;

bool BookmarksMenu::hasBookmarks() const
{
	return (!this->actions().isEmpty());
}

void BookmarksMenu::setMetadata(const MetaData& track)
{
	m->bookmarks.setTrack(track);
	bookmarksChanged();
}

MetaData BookmarksMenu::metadata() const
{
	return m->bookmarks.track();
}

void BookmarksMenu::bookmarksChanged()
{
	for(auto* a : this->actions())
	{
		a->deleteLater();
	}

	this->clear();

	const auto& bookmarks = m->bookmarks.bookmarks();
	for(const auto& bookmark : bookmarks)
	{
		const auto name = bookmark.name();
		if(!name.isEmpty())
		{
			auto* action = this->addAction(name);
			action->setData(bookmark.timestamp());
			connect(action, &QAction::triggered, this, &BookmarksMenu::actionPressed);
		}
	}

	this->addSeparator();

	auto* editAction = new QAction(Gui::Icons::icon(Gui::Icons::Edit), Lang::get(Lang::Edit), this);
	this->addAction(editAction);

	connect(editAction, &QAction::triggered, [&]() {
		m->playerPluginHandler->showPlugin("Bookmarks");
	});

	editAction->setEnabled
		(
			(metadata().id() == m->playManager->currentTrack().id())
		);
}

void BookmarksMenu::actionPressed()
{
	auto* action = dynamic_cast<QAction*>(sender());
	const auto time = static_cast<Seconds>(action->data().toInt());

	emit sigBookmarkPressed(time);
}
