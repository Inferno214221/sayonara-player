/* GUI_SoundcloudArtistSearch.h */

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

#ifndef GUI_SOUNDCLOUDARTISTSEARCH_H
#define GUI_SOUNDCLOUDARTISTSEARCH_H

#include "Utils/Pimpl.h"
#include "GUI/Utils/Widgets/Dialog.h"

class ArtistList;
class MetaDataList;
class AlbumList;

UI_FWD(GUI_SoundcloudArtistSearch)

namespace SC
{
	class Library;

	class GUI_ArtistSearch :
			public Gui::Dialog
	{
		Q_OBJECT
		UI_CLASS(GUI_SoundcloudArtistSearch)
		PIMPL(GUI_ArtistSearch)

	public:
		explicit GUI_ArtistSearch(SC::Library* library, QWidget *parent=nullptr);
		~GUI_ArtistSearch();

	private slots:
		void search_clicked();
		void clear_clicked();
		void add_clicked();
		void close_clicked();

		void artists_fetched(const ArtistList& artists);
		void artists_ext_fetched(const ArtistList& artists);
		void albums_fetched(const AlbumList& albums);
		void tracks_fetched(const MetaDataList& tracks);

		void artist_selected(int idx);

	private:
		void set_tracks_label(int n_tracks);
		void set_playlist_label(int n_playlists);
        void language_changed() override;
	};
}
#endif // GUI_SOUNDCLOUDARTISTSEARCH_H
