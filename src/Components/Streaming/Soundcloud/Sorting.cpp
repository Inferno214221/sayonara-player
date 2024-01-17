/* Sorting.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Sorting.h"

#include "Utils/Algorithm.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"

#include <functional>
#include <QMap>

namespace Algorithm = Util::Algorithm;

namespace Compare
{
	bool artistNameAsc(const Artist& artist1, const Artist& artist2)
	{
		return (artist1.name() < artist2.name());
	}

	bool artistNameDesc(const Artist& artist1, const Artist& artist2)
	{
		return artistNameAsc(artist2, artist1);
	}

	bool artistTrackcountAsc(const Artist& artist1, const Artist& artist2)
	{
		return (artist1.songcount() < artist2.songcount());
	}

	bool artistTrackcountDesc(const Artist& artist1, const Artist& artist2)
	{
		return artistTrackcountAsc(artist2, artist1);
	}

	bool albumNameAsc(const Album& album1, const Album& album2)
	{
		return (album1.name() < album2.name());
	}

	bool albumNameDesc(const Album& album1, const Album& album2)
	{
		return albumNameAsc(album2, album1);
	}

	bool albumYearAsc(const Album& album1, const Album& album2)
	{
		return (album1.year() < album2.year());
	}

	bool albumYearDesc(const Album& album1, const Album& album2)
	{
		return albumYearAsc(album2, album1);
	}

	bool albumRatingAsc(const Album& album1, const Album& album2)
	{
		return (album1.rating() < album2.rating());
	}

	bool albumRatingDesc(const Album& album1, const Album& album2)
	{
		return albumRatingAsc(album2, album1);
	}

	bool albumDurationAsc(const Album& album1, const Album& album2)
	{
		return (album1.durationSec() < album2.durationSec());
	}

	bool albumDurationDesc(const Album& album1, const Album& album2)
	{
		return albumDurationAsc(album2, album1);
	}

	bool trackTitleAsc(const MetaData& md1, const MetaData& md2)
	{
		return (md1.title() < md2.title());
	}

	bool trackTitleDesc(const MetaData& md1, const MetaData& md2)
	{
		return trackTitleAsc(md2, md1);
	}

	bool trackNumAsc(const MetaData& md1, const MetaData& md2)
	{
		return (md1.trackNumber() < md2.trackNumber());
	}

	bool trackNumDesc(const MetaData& md1, const MetaData& md2)
	{
		return trackNumAsc(md2, md1);
	}

	bool trackAlbumAsc(const MetaData& md1, const MetaData& md2)
	{
		if(md1.album() == md2.album())
		{
			return trackNumAsc(md1, md2);
		}

		return (md1.album() < md2.album());
	}

	bool trackAlbumDesc(const MetaData& md1, const MetaData& md2)
	{
		return trackAlbumAsc(md2, md1);
	}

	bool trackArtistAsc(const MetaData& md1, const MetaData& md2)
	{
		if(md1.artist() == md2.artist())
		{
			return trackAlbumAsc(md1, md2);
		}

		return (md1.artist() < md2.artist());
	}

	bool trackArtistDesc(const MetaData& md1, const MetaData& md2)
	{
		return trackArtistAsc(md2, md1);
	}

	bool trackYearAsc(const MetaData& md1, const MetaData& md2)
	{
		if(md1.year() == md2.year())
		{
			return trackArtistAsc(md1, md2);
		}

		return (md1.year() < md2.year());
	}

	bool trackYearDesc(const MetaData& md1, const MetaData& md2)
	{
		return trackYearAsc(md2, md1);
	}

	bool trackLengthAsc(const MetaData& md1, const MetaData& md2)
	{
		return (md1.durationMs() < md2.durationMs());
	}

	bool trackLengthDesc(const MetaData& md1, const MetaData& md2)
	{
		return trackLengthAsc(md2, md1);
	}

	bool trackBitrateAsc(const MetaData& md1, const MetaData& md2)
	{
		if(md1.bitrate() == md2.bitrate())
		{
			return trackArtistAsc(md1, md2);
		}

		return (md1.bitrate() < md2.bitrate());
	}

	bool trackBitrateDesc(const MetaData& md1, const MetaData& md2)
	{
		return trackBitrateAsc(md2, md1);
	}
}

void SC::Sorting::sortArtists(ArtistList& artists, const Library::ArtistSortorder so)
{
	using namespace Library;
	using SortFn = std::function<bool(const Artist&, const Artist&)>;
	QMap<Library::ArtistSortorder, SortFn> functions
		{
			{ArtistSortorder::NameAsc,        Compare::artistNameAsc},
			{ArtistSortorder::NameDesc,       Compare::artistNameDesc},
			{ArtistSortorder::TrackcountAsc,  Compare::artistTrackcountAsc},
			{ArtistSortorder::TrackcountDesc, Compare::artistTrackcountDesc}
		};

	if(functions.contains(so))
	{
		Algorithm::sort(artists, functions[so]);
	}
}

void SC::Sorting::sortAlbums(AlbumList& albums, const Library::AlbumSortorder so)
{
	using namespace Library;
	using SortFn = std::function<bool(const Album&, const Album&)>;
	QMap<Library::AlbumSortorder, SortFn> functions
		{
			{AlbumSortorder::NameAsc,      &Compare::albumNameAsc},
			{AlbumSortorder::NameDesc,     &Compare::albumNameDesc},
			{AlbumSortorder::YearAsc,      &Compare::albumYearAsc},
			{AlbumSortorder::YearDesc,     &Compare::albumYearDesc},
			{AlbumSortorder::DurationAsc,  &Compare::albumDurationAsc},
			{AlbumSortorder::DurationDesc, &Compare::albumDurationDesc}
		};

	if(functions.contains(so))
	{
		Algorithm::sort(albums, functions[so]);
	}
}

void SC::Sorting::sortTracks(MetaDataList& tracks, const Library::TrackSortorder so)
{
	using namespace Library;
	using SortFn = std::function<bool(const MetaData&, const MetaData&)>;

	QMap<Library::TrackSortorder, SortFn> functions
		{
			{TrackSortorder::TrackNumberAsc,  &Compare::trackNumAsc},
			{TrackSortorder::TrackNumberDesc, &Compare::trackNumDesc},
			{TrackSortorder::TitleAsc,        &Compare::trackTitleAsc},
			{TrackSortorder::TitleDesc,       &Compare::trackTitleDesc},
			{TrackSortorder::AlbumAsc,        &Compare::trackAlbumAsc},
			{TrackSortorder::AlbumDesc,       &Compare::trackAlbumDesc},
			{TrackSortorder::ArtistAsc,       &Compare::trackArtistAsc},
			{TrackSortorder::ArtistDesc,      &Compare::trackArtistDesc},
			{TrackSortorder::YearAsc,         &Compare::trackYearAsc},
			{TrackSortorder::YearDesc,        &Compare::trackYearDesc},
			{TrackSortorder::LengthAsc,       &Compare::trackLengthAsc},
			{TrackSortorder::LengthDesc,      &Compare::trackLengthDesc},
			{TrackSortorder::BitrateAsc,      &Compare::trackBitrateAsc},
			{TrackSortorder::BitrateDesc,     &Compare::trackBitrateDesc}
		};

	if(functions.contains(so))
	{
		Algorithm::sort(tracks, functions[so]);
	}
}
