#ifndef SOUNDCLOUDVIEWS_H
#define SOUNDCLOUDVIEWS_H

#include "GUI/Library/TrackView.h"
#include "GUI/Library/AlbumView.h"
#include "GUI/Library/ArtistView.h"

namespace SC
{
	class TrackView : public ::Library::TrackView
	{
		Q_OBJECT
		public:
			using ::Library::TrackView::TrackView;
			LibraryContextMenu::Entries context_menu_entries() const override;
	};

	class AlbumView : public ::Library::AlbumView
	{
		Q_OBJECT
		public:
			using ::Library::AlbumView::AlbumView;
			LibraryContextMenu::Entries context_menu_entries() const override;
	};

	class ArtistView : public ::Library::ArtistView
	{
		Q_OBJECT
		public:
			using ::Library::ArtistView::ArtistView;
			LibraryContextMenu::Entries context_menu_entries() const override;
	};

}

#endif // SOUNDCLOUDVIEWS_H
