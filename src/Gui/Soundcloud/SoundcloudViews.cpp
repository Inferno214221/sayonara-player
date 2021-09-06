/* SoundcloudViews.cpp */

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

#include "Gui/Soundcloud/SoundcloudViews.h"
#include "Gui/Soundcloud/ContextMenu.h"

static Library::ContextMenu::Entries entryMask()
{
	Library::ContextMenu::Entries entryMask =
			(Library::ContextMenu::EntryPlayNext |
			 Library::ContextMenu::EntryInfo |
			 Library::ContextMenu::EntryDelete |
			 Library::ContextMenu::EntryAppend |
			 Library::ContextMenu::EntryRefresh);

	return entryMask;
}

Library::ContextMenu::Entries SC::TrackView::contextMenuEntries() const
{
	return entryMask();
}

bool SC::TrackView::isMergeable() const
{
	return false;
}

Library::ContextMenu::Entries SC::AlbumView::contextMenuEntries() const
{
	return entryMask();
}

bool SC::AlbumView::isMergeable() const
{
	return false;
}

Library::ContextMenu::Entries SC::ArtistView::contextMenuEntries() const
{
	return entryMask();
}

void SC::ArtistView::initContextMenu()
{
	if(contextMenu()){
		return;
	}

	auto* cm = new SC::ContextMenu(this);
	ItemView::initCustomContextMenu(cm);

	connect(cm, &SC::ContextMenu::sigAddArtistTriggered, this, &SC::ArtistView::sigAddArtistTriggered);
}

bool SC::ArtistView::isMergeable() const
{
	return false;
}
