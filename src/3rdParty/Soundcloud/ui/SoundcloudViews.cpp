/* SoundcloudViews.cpp */

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



#include "SoundcloudViews.h"

static LibraryContextMenu::Entries entry_mask()
{
	LibraryContextMenu::Entries entry_mask =
			(LibraryContextMenu::EntryPlayNext |
			 LibraryContextMenu::EntryInfo |
			 LibraryContextMenu::EntryDelete |
			 LibraryContextMenu::EntryAppend |
			 LibraryContextMenu::EntryRefresh);

	return entry_mask;
}


LibraryContextMenu::Entries SC::TrackView::context_menu_entries() const
{
	return entry_mask();
}

LibraryContextMenu::Entries SC::AlbumView::context_menu_entries() const
{
	return entry_mask();
}

LibraryContextMenu::Entries SC::ArtistView::context_menu_entries() const
{
	return entry_mask();
}
