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
