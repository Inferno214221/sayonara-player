/* MetaDataSorting.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#ifndef METADATASORTING_H
#define METADATASORTING_H

#include "Utils/Library/Sortorder.h"

class MetaData;
class Album;
class Artist;

class MetaDataList;
class AlbumList;
class ArtistList;

namespace MetaDataSorting
{
	bool TracksByTrackNumAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByTrackNumDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByDiscnumberAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByDiscnumberDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByTitleAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByTitleDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByAlbumAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByAlbumDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByArtistAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByArtistDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByAlbumArtistAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByAlbumArtistDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByYearAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByYearDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByLengthAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByLengthDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByBitrateAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByBitrateDesc(const MetaData& md1, const MetaData& md2);

	bool TracksByFilesizeAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByFilesizeDesc(const MetaData& md1, const MetaData& md2);


	bool TracksByRatingAsc(const MetaData& md1, const MetaData& md2);
	bool TracksByRatingDesc(const MetaData& md1, const MetaData& md2);

	bool ArtistByNameAsc(const Artist& artist1, const Artist& artist2);
	bool ArtistByNameDesc(const Artist& artist1, const Artist& artist2);
	bool ArtistByTrackCountAsc(const Artist& artist1, const Artist& artist2);
	bool ArtistByTrackCountDesc(const Artist& artist1, const Artist& artist2);

	bool AlbumByArtistNameAsc(const Album& album1, const Album& album2);
	bool AlbumByArtistNameDesc(const Album& album1, const Album& album2);
	bool AlbumByNameAsc(const Album& album1, const Album& album2);
	bool AlbumByNameDesc(const Album& album1, const Album& album2);
	bool AlbumByYearAsc(const Album& album1, const Album& album2);
	bool AlbumByYearDesc(const Album& album1, const Album& album2);
	bool AlbumByDurationAsc(const Album& album1, const Album& album2);
	bool AlbumByDurationDesc(const Album& album1, const Album& album2);
	bool AlbumByTracksAsc(const Album& album1, const Album& album2);
	bool AlbumByTracksDesc(const Album& album1, const Album& album2);
	bool AlbumByRatingAsc(const Album& album1, const Album& album2);
	bool AlbumByRatingDesc(const Album& album1, const Album& album2);

	void sort_metadata(MetaDataList& v_md, Library::SortOrder);
	void sort_albums(AlbumList& v_md, Library::SortOrder);
	void sort_artists(ArtistList& v_md, Library::SortOrder);
}

#endif // METADATASORTING_H
