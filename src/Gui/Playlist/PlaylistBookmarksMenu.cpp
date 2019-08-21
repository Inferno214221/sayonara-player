/* BookmarksMenu.cpp */

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
	BookmarksBase*	bookmarks=nullptr;

	Private(BookmarksMenu* parent)
	{
		bookmarks = new BookmarksBase(parent);
	}
};

BookmarksMenu::BookmarksMenu(QWidget* parent) :
	QMenu(parent)
{
	m = Pimpl::make<Private>(this);

	this->setTitle( Lang::get(Lang::Bookmarks));
}

BookmarksMenu::~BookmarksMenu() {}

bool BookmarksMenu::has_bookmarks() const
{
	return (this->actions().size() > 0);
}

void BookmarksMenu::set_metadata(const MetaData& md)
{
	m->bookmarks->set_metadata(md);

	bookmarks_changed();
}

MetaData BookmarksMenu::metadata() const
{
	return m->bookmarks->metadata();
}

void BookmarksMenu::bookmarks_changed()
{
	for(QAction* a : this->actions()){
		a->deleteLater();
	}

	this->clear();

	const QList<Bookmark> bookmarks = m->bookmarks->bookmarks();
	for(const Bookmark& bookmark : bookmarks)
	{
		QString name = bookmark.name();
		if(name.isEmpty()){
			continue;
		}

		QAction* action = this->addAction(name);
		action->setData(bookmark.timestamp());
		connect(action, &QAction::triggered, this, &BookmarksMenu::action_pressed);
	}

	this->addSeparator();
	QAction* edit_action = new QAction(Gui::Icons::icon(Gui::Icons::Edit), Lang::get(Lang::Edit), this);
	this->addAction(edit_action);

	connect(edit_action, &QAction::triggered, [](){
		PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();
		pph->show_plugin("Bookmarks");
	});

	PlayManager* pm = PlayManager::instance();
	edit_action->setEnabled(
		metadata().id == pm->current_track().id
	);
}

void BookmarksMenu::action_pressed()
{
	QAction* action = dynamic_cast<QAction*>(sender());
	Seconds time = Seconds(action->data().toInt());

	emit sig_bookmark_pressed(time);
}


