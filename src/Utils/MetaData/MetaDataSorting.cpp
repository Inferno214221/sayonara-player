/* MetaDataSorting.cpp */

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

#include "MetaDataSorting.h"
#include "MetaData.h"
#include "MetaDataList.h"
#include "Artist.h"
#include "Album.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"

#include <QDateTime>

namespace
{
	bool ignoreArticle = false;
}

namespace
{
	template<typename Item, typename Fallback, typename Extractor>
	bool compare(const Item& item1, const Item& item2, Fallback&& fallback, Extractor&& extractor)
	{
		const auto val1 = extractor(item1);
		const auto val2 = extractor(item2);
		if(val1 < val2)
		{
			return true;
		}

		else if(val1 > val2)
		{
			return false;
		}

		return fallback(item1, item2);
	}

	template<typename Item, typename Fallback, typename Extractor>
	bool compareRev(const Item& item1, const Item& item2, Fallback&& fallback, Extractor&& extractor)
	{
		const auto val1 = extractor(item1);
		const auto val2 = extractor(item2);
		if(val1 < val2)
		{
			return true;
		}

		else if(val1 > val2)
		{
			return false;
		}

		// pass the items in reverse order to the fallback
		return fallback(item2, item1);
	}

	QString checkArticleInTitle(const QString& title)
	{
		return (ignoreArticle && title.startsWith("the ", Qt::CaseInsensitive))
		       ? title.right(title.size() - 4)
		       : title;
	}

	bool TracksByTitleAsc(const MetaData& track1, const MetaData& track2)
	{
		auto fallback = [](const auto& t1, const auto& t2) { return (t1.filepath() < t2.filepath()); };
		return compare(track1, track2, std::move(fallback), [](const auto& track) { return track.title(); });
	}

	bool TracksByTitleDesc(const MetaData& track1, const MetaData& track2)
	{
		auto fallback = [](const auto& t1, const auto& t2) { return (t1.filepath() < t2.filepath()); };
		return compareRev(track2, track1, std::move(fallback), [](const auto& track) { return track.title(); });
	}

	bool TracksByTrackNumAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByTitleAsc, [](const auto& track) { return track.trackNumber(); });
	}

	bool TracksByTrackNumDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByTitleAsc, [](const auto& track) { return track.trackNumber(); });
	}

	bool TracksByDiscnumberAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByTrackNumAsc, [](const auto& track) { return track.discnumber(); });
	}

	bool TracksByDiscnumberDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByTrackNumAsc, [](const auto& track) { return track.discnumber(); });
	}

	bool TracksByAlbumAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByDiscnumberAsc, [](const auto& track) { return track.album(); });
	}

	bool TracksByAlbumDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByAlbumAsc, [](const auto& track) { return track.album(); });
	}

	bool TracksByAlbumArtistAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByAlbumAsc, [](const auto& track) {
			return checkArticleInTitle(track.albumArtist());
		});
	}

	bool TracksByAlbumArtistDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByAlbumAsc, [](const auto& track) {
			return checkArticleInTitle(track.albumArtist());
		});
	}

	bool TracksByArtistAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByAlbumAsc, [](const auto& track) {
			return checkArticleInTitle(track.artist());
		});
	}

	bool TracksByArtistDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByAlbumAsc, [](const auto& track) {
			return checkArticleInTitle(track.artist());
		});
	}

	bool TracksByYearAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByArtistAsc, [](const auto& track) { return track.year(); });
	}

	bool TracksByYearDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByArtistAsc, [](const auto& track) { return track.year(); });
	}

	bool TracksByLengthAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByArtistAsc, [](const auto& track) { return track.durationMs(); });
	}

	bool TracksByLengthDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByArtistAsc, [](const auto& track) { return track.durationMs(); });
	}

	bool TracksByBitrateAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByArtistAsc, [](const auto& track) { return track.bitrate(); });
	}

	bool TracksByBitrateDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByArtistAsc, [](const auto& track) { return track.bitrate(); });
	}

	bool TracksByFilesizeAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByArtistAsc, [](const auto& track) { return track.filesize(); });
	}

	bool TracksByFilesizeDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByArtistAsc, [](const auto& track) { return track.filesize(); });
	}

	bool TracksByFiletypeAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1,
		               track2,
		               TracksByArtistAsc,
		               [](const auto& track) { return Util::File::getFileExtension(track.filepath()); });
	}

	bool TracksByFiletypeDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2,
		                  track1,
		                  TracksByArtistAsc,
		                  [](const auto& track) { return Util::File::getFileExtension(track.filepath()); });
	}

	bool TracksByRatingAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByArtistAsc, [](const auto& track) { return track.rating(); });
	}

	bool TracksByRatingDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByArtistAsc, [](const auto& track) { return track.rating(); });
	}

	bool TracksByAddedDateAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByArtistAsc, [](const auto& track) { return track.createdDate(); });
	}

	bool TracksByAddedDateDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2, track1, TracksByArtistAsc, [](const auto& track) { return track.createdDate(); });
	}

	bool TracksByModifiedDateAsc(const MetaData& track1, const MetaData& track2)
	{
		return compare(track1, track2, TracksByAddedDateAsc, [](const auto& track) { return track.modifiedDate(); });
	}

	bool TracksByModifiedDateDesc(const MetaData& track1, const MetaData& track2)
	{
		return compareRev(track2,
		                  track1,
		                  TracksByAddedDateDesc,
		                  [](const auto& track) { return track.modifiedDate(); });
	}

	/*** Albums ***/
	bool AlbumByNameAsc(const Album& album1, const Album& album2)
	{
		auto fallback = [](const auto& a1, const auto& a2) { return a1.id() < a2.id(); };
		return compare(album1, album2, fallback, [](const auto& album) { return album.name(); });
	}

	bool AlbumByNameDesc(const Album& album1, const Album& album2)
	{
		auto fallback = [](const auto& a1, const auto& a2) { return a1.id() < a2.id(); };
		return compareRev(album2, album1, fallback, [](const auto& album) { return album.name(); });
	}

	bool AlbumByYearAsc(const Album& album1, const Album& album2)
	{
		return compare(album1, album2, AlbumByNameAsc, [](const auto& album) { return album.year(); });
	}

	bool AlbumByYearDesc(const Album& album1, const Album& album2)
	{
		return compareRev(album2, album1, AlbumByNameAsc, [](const auto& album) { return album.year(); });
	}

	bool AlbumByAlbumArtistAsc(const Album& album1, const Album& album2)
	{
		return compare(album1, album2, AlbumByNameAsc, [](const auto& album) {
			return checkArticleInTitle(album.albumArtist());
		});
	}

	bool AlbumByAlbumArtistDesc(const Album& album1, const Album& album2)
	{
		return compareRev(album2, album1, AlbumByNameAsc, [](const auto& album) {
			return checkArticleInTitle(album.albumArtist());
		});
	}

	bool AlbumByDurationAsc(const Album& album1, const Album& album2)
	{
		return compare(album1, album2, AlbumByNameAsc, [](const auto& album) { return album.durationSec(); });
	}

	bool AlbumByDurationDesc(const Album& album1, const Album& album2)
	{
		return compareRev(album2, album1, AlbumByNameAsc, [](const auto& album) { return album.durationSec(); });
	}

	bool AlbumBySongcountAsc(const Album& album1, const Album& album2)
	{
		return compare(album1, album2, AlbumByNameAsc, [](const auto& album) { return album.songcount(); });
	}

	bool AlbumBySongcountDesc(const Album& album1, const Album& album2)
	{
		return compareRev(album2, album1, AlbumByNameAsc, [](const auto& album) { return album.songcount(); });
	}

	bool AlbumByRatingAsc(const Album& album1, const Album& album2)
	{
		return compare(album1, album2, AlbumByNameAsc, [](const auto& album) { return album.rating(); });
	}

	bool AlbumByRatingDesc(const Album& album1, const Album& album2)
	{
		return compareRev(album2, album1, AlbumByNameAsc, [](const auto& album) { return album.rating(); });
	}

	/*** Artists ***/
	bool ArtistByNameAsc(const Artist& artist1, const Artist& artist2)
	{
		auto fallback = [](const auto& a1, const auto a2) { return (a1.id() < a2.id()); };
		return compare(artist1, artist2, fallback, [](const auto& artist) {
			return checkArticleInTitle(artist.name());
		});
	}

	bool ArtistByNameDesc(const Artist& artist1, const Artist& artist2)
	{
		return ArtistByNameAsc(artist2, artist1);
	}

	bool ArtistByTrackCountAsc(const Artist& artist1, const Artist& artist2)
	{
		return compare(artist1, artist2, ArtistByNameAsc, [](const auto& artist) { return artist.songcount(); });
	}

	bool ArtistByTrackCountDesc(const Artist& artist1, const Artist& artist2)
	{
		return compareRev(artist2, artist1, ArtistByNameDesc, [](const auto& artist) { return artist.songcount(); });
	}
}

void MetaDataSorting::sortMetadata(MetaDataList& tracks, Library::SortOrder so)
{
	namespace Algorithm = Util::Algorithm;
	using So = Library::SortOrder;
	switch(so)
	{
		case So::TrackNumAsc:
			Algorithm::sort(tracks, TracksByTrackNumAsc);
			break;
		case So::TrackNumDesc:
			Algorithm::sort(tracks, TracksByTrackNumDesc);
			break;
		case So::TrackTitleAsc:
			Algorithm::sort(tracks, TracksByTitleAsc);
			break;
		case So::TrackTitleDesc:
			Algorithm::sort(tracks, TracksByTitleDesc);
			break;
		case So::TrackAlbumAsc:
			Algorithm::sort(tracks, TracksByAlbumAsc);
			break;
		case So::TrackAlbumDesc:
			Algorithm::sort(tracks, TracksByAlbumDesc);
			break;
		case So::TrackArtistAsc:
			Algorithm::sort(tracks, TracksByArtistAsc);
			break;
		case So::TrackArtistDesc:
			Algorithm::sort(tracks, TracksByArtistDesc);
			break;
		case So::TrackAlbumArtistAsc:
			Algorithm::sort(tracks, TracksByAlbumArtistAsc);
			break;
		case So::TrackAlbumArtistDesc:
			Algorithm::sort(tracks, TracksByAlbumArtistDesc);
			break;
		case So::TrackYearAsc:
			Algorithm::sort(tracks, TracksByYearAsc);
			break;
		case So::TrackYearDesc:
			Algorithm::sort(tracks, TracksByYearDesc);
			break;
		case So::TrackLenghtAsc:
			Algorithm::sort(tracks, TracksByLengthAsc);
			break;
		case So::TrackLengthDesc:
			Algorithm::sort(tracks, TracksByLengthDesc);
			break;
		case So::TrackBitrateAsc:
			Algorithm::sort(tracks, TracksByBitrateAsc);
			break;
		case So::TrackBitrateDesc:
			Algorithm::sort(tracks, TracksByBitrateDesc);
			break;
		case So::TrackSizeAsc:
			Algorithm::sort(tracks, TracksByFilesizeAsc);
			break;
		case So::TrackSizeDesc:
			Algorithm::sort(tracks, TracksByFilesizeDesc);
			break;
		case So::TrackDiscnumberAsc:
			Algorithm::sort(tracks, TracksByDiscnumberAsc);
			break;
		case So::TrackDiscnumberDesc:
			Algorithm::sort(tracks, TracksByDiscnumberDesc);
			break;
		case So::TrackFiletypeAsc:
			Algorithm::sort(tracks, TracksByFiletypeAsc);
			break;
		case So::TrackFiletypeDesc:
			Algorithm::sort(tracks, TracksByFiletypeDesc);
			break;
		case So::TrackRatingAsc:
			Algorithm::sort(tracks, TracksByRatingAsc);
			break;
		case So::TrackRatingDesc:
			Algorithm::sort(tracks, TracksByRatingDesc);
			break;
		case So::TrackDateAddedAsc:
			Algorithm::sort(tracks, TracksByAddedDateAsc);
			break;
		case So::TrackDateAddedDesc:
			Algorithm::sort(tracks, TracksByAddedDateDesc);
			break;
		case So::TrackDateModifiedAsc:
			Algorithm::sort(tracks, TracksByModifiedDateAsc);
			break;
		case So::TrackDateModifiedDesc:
			Algorithm::sort(tracks, TracksByModifiedDateDesc);
			break;
		default:
			break;
	}
}

void MetaDataSorting::sortAlbums(AlbumList& albums, Library::SortOrder so)
{
	namespace Algorithm = Util::Algorithm;
	using So = Library::SortOrder;
	switch(so)
	{
		case So::ArtistNameAsc:
			Algorithm::sort(albums, AlbumByAlbumArtistAsc);
			break;
		case So::ArtistNameDesc:
			Algorithm::sort(albums, AlbumByAlbumArtistDesc);
			break;
		case So::AlbumNameAsc:
			Algorithm::sort(albums, AlbumByNameAsc);
			break;
		case So::AlbumNameDesc:
			Algorithm::sort(albums, AlbumByNameDesc);
			break;
		case So::AlbumDurationAsc:
			Algorithm::sort(albums, AlbumByDurationAsc);
			break;
		case So::AlbumDurationDesc:
			Algorithm::sort(albums, AlbumByDurationDesc);
			break;
		case So::AlbumRatingAsc:
			Algorithm::sort(albums, AlbumByRatingAsc);
			break;
		case So::AlbumRatingDesc:
			Algorithm::sort(albums, AlbumByRatingDesc);
			break;
		case So::AlbumTracksAsc:
			Algorithm::sort(albums, AlbumBySongcountAsc);
			break;
		case So::AlbumTracksDesc:
			Algorithm::sort(albums, AlbumBySongcountDesc);
			break;
		case So::AlbumYearAsc:
			Algorithm::sort(albums, AlbumByYearAsc);
			break;
		case So::AlbumYearDesc:
			Algorithm::sort(albums, AlbumByYearDesc);
			break;
		default:
			break;
	}
}

void MetaDataSorting::sortArtists(ArtistList& artists, Library::SortOrder so)
{
	namespace Algorithm = Util::Algorithm;
	using So = Library::SortOrder;
	switch(so)
	{
		case So::ArtistNameAsc:
			Algorithm::sort(artists, ArtistByNameAsc);
			break;
		case So::ArtistNameDesc:
			Algorithm::sort(artists, ArtistByNameDesc);
			break;
		case So::ArtistTrackcountAsc:
			Algorithm::sort(artists, ArtistByTrackCountAsc);
			break;
		case So::ArtistTrackcountDesc:
			Algorithm::sort(artists, ArtistByTrackCountDesc);
			break;
		default:
			break;
	}
}

void MetaDataSorting::setIgnoreArticle(bool b)
{
	ignoreArticle = b;
}
