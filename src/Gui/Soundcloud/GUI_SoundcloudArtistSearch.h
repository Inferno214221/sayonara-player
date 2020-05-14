/* GUI_SoundcloudArtistSearch.h */

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

#ifndef GUI_SOUNDCLOUDARTISTSEARCH_H
#define GUI_SOUNDCLOUDARTISTSEARCH_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/Dialog.h"

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
			explicit GUI_ArtistSearch(SC::Library* library, QWidget* parent=nullptr);
			~GUI_ArtistSearch() override;

		private slots:
			void searchClicked();
			void clearClicked();
			void addClicked();
			void closeClicked();

			void artistsFetched(const ArtistList& artists);
			void artistsExtFetched(const ArtistList& artists);
			void albumsFetched(const AlbumList& albums);
			void tracksFetched(const MetaDataList& tracks);

			void artistSelected(int idx);

		private:
			void setTrackCountLabel(int trackCount);
			void setPlaylistCountLabel(int playlistCount);

		protected:
			void languageChanged() override;
			void skinChanged() override;
	};
}
#endif // GUI_SOUNDCLOUDARTISTSEARCH_H
