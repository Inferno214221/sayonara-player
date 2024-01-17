/* MetaDataSorting.cpp */

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

#include "MetaDataSorting.h"
#include "MetaData.h"
#include "MetaDataList.h"
#include "Artist.h"
#include "Album.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/SearchMode.h"

#include <QDateTime>

using Library::SortOrder;

namespace MetaDataSorting
{
	namespace
	{
		template<typename Item, typename Fallback, typename Extractor>
		bool compare(const Item& item1, const Item& item2, const SortModeMask sortMode, Fallback&& fallback,
		             Extractor&& extractor)
		{
			const auto val1 = extractor(item1);
			const auto val2 = extractor(item2);
			if(val1 < val2)
			{
				return true;
			}

			if(val1 > val2)
			{
				return false;
			}

			return fallback(item1, item2, sortMode);
		}

		template<typename Item, typename Fallback, typename Extractor>
		bool compareRev(const Item& item1, const Item& item2, const SortModeMask sortMode, Fallback&& fallback,
		                Extractor&& extractor)
		{
			const auto val1 = extractor(item1);
			const auto val2 = extractor(item2);
			if(val1 < val2)
			{
				return true;
			}

			if(val1 > val2)
			{
				return false;
			}

			return fallback(item2, item1, sortMode);
		}

		Library::SearchModeMask sortModeToSearchMode(const SortModeMask sortModeMask)
		{
			auto searchModeMask = +Library::SearchMode::None;
			if(sortModeMask & +SortMode::CaseInsensitive)
			{
				searchModeMask |= +Library::SearchMode::CaseInsensitve;
			}

			if(sortModeMask & +SortMode::IgnoreDiacryticChars)
			{
				searchModeMask |= +Library::SearchMode::NoDiacriticChars;
			}

			if(sortModeMask & +SortMode::IgnoreSpecialChars)
			{
				searchModeMask |= +Library::SearchMode::NoSpecialChars;
			}

			return searchModeMask;
		}

		QString convertString(const QString& str, const SortModeMask sortMode)
		{
			return Library::convertSearchstring(str, sortModeToSearchMode(sortMode));
		}

		QString convertString(QString str, const SortModeMask sortMode, const bool mayIgnoreArticle)
		{
			const auto searchModeMask = sortModeToSearchMode(sortMode);
			if(mayIgnoreArticle && (sortMode & +SortMode::IgnoreArticle) && str.toLower().startsWith("the "))
			{
				str = str.right(str.size() - 4);
			}

			return Library::convertSearchstring(str, searchModeMask);
		}

		bool TracksByFilepathAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return convertString(track1.filepath(), sortMode) < convertString(track2.filepath(), sortMode);
		}

		bool TracksByFilepathDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return convertString(track2.filepath(), sortMode) < convertString(track1.filepath(), sortMode);
		}

		bool TracksByTitleAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByFilepathAsc, [&](const auto& track) {
				return convertString(track.title(), sortMode);
			});
		}

		bool TracksByTitleDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByFilepathAsc, [&](const auto& track) {
				return convertString(track.title(), sortMode);
			});
		}

		bool TracksByTrackNumAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByTitleAsc, [](const auto& track) {
				return track.trackNumber();
			});
		}

		bool TracksByTrackNumDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByTitleAsc, [](const auto& track) {
				return track.trackNumber();
			});
		}

		bool TracksByDiscnumberAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByTrackNumAsc, [](const auto& track) {
				return track.discnumber();
			});
		}

		bool TracksByDiscnumberDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByTrackNumAsc, [](const auto& track) {
				return track.discnumber();
			});
		}

		bool TracksByAlbumAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByDiscnumberAsc, [&](const auto& track) {
				return convertString(track.album(), sortMode);
			});
		}

		bool TracksByAlbumDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByAlbumAsc, [&](const auto& track) {
				return convertString(track.album(), sortMode);
			});
		}

		bool TracksByAlbumArtistAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByAlbumAsc, [&](const auto& track) {
				return convertString(track.albumArtist(), sortMode, true);
			});
		}

		bool TracksByAlbumArtistDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByAlbumAsc, [&](const auto& track) {
				return convertString(track.albumArtist(), sortMode, true);
			});
		}

		bool TracksByArtistAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByAlbumAsc, [&](const auto& track) {
				return convertString(track.artist(), sortMode, true);
			});
		}

		bool TracksByArtistDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByAlbumAsc, [&](const auto& track) {
				return convertString(track.artist(), sortMode, true);
			});
		}

		bool TracksByYearAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.year();
			});
		}

		bool TracksByYearDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.year();
			});
		}

		bool TracksByLengthAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.durationMs();
			});
		}

		bool TracksByLengthDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.durationMs();
			});
		}

		bool TracksByBitrateAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.bitrate();
			});
		}

		bool TracksByBitrateDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.bitrate();
			});
		}

		bool TracksByFilesizeAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.filesize();
			});
		}

		bool TracksByFilesizeDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.filesize();
			});
		}

		bool TracksByFiletypeAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByArtistAsc, [](const auto& track) {
				return convertString(Util::File::getFileExtension(track.filepath()), +SortMode::CaseInsensitive);
			});
		}

		bool TracksByFiletypeDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByArtistAsc, [](const auto& track) {
				return convertString(Util::File::getFileExtension(track.filepath()), +SortMode::CaseInsensitive);
			});
		}

		bool TracksByRatingAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.rating();
			});
		}

		bool TracksByRatingDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.rating();
			});
		}

		bool TracksByAddedDateAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.createdDate();
			});
		}

		bool TracksByAddedDateDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByArtistAsc, [](const auto& track) {
				return track.createdDate();
			});
		}

		bool TracksByModifiedDateAsc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compare(track1, track2, sortMode, TracksByAddedDateAsc, [](const auto& track) {
				return track.modifiedDate();
			});
		}

		bool TracksByModifiedDateDesc(const MetaData& track1, const MetaData& track2, const SortModeMask sortMode)
		{
			return compareRev(track2, track1, sortMode, TracksByAddedDateDesc, [](const auto& track) {
				return track.modifiedDate();
			});
		}

		/*** Albums ***/
		bool AlbumByNameAsc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			auto fallback = [](const auto& a1, const auto& a2, const auto /*sm*/) {
				return a1.id() < a2.id();
			};

			return compare(album1, album2, sortMode, fallback, [&](const auto& album) {
				return convertString(album.name(), sortMode);
			});
		}

		bool AlbumByNameDesc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			auto fallback = [](const auto& a1, const auto& a2, const auto /*sm*/) {
				return a1.id() < a2.id();
			};
			return compareRev(album2, album1, sortMode, fallback, [&](const auto& album) {
				return convertString(album.name(), sortMode);
			});
		}

		bool AlbumByYearAsc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compare(album1, album2, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.year();
			});
		}

		bool AlbumByYearDesc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compareRev(album2, album1, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.year();
			});
		}

		bool AlbumByAlbumArtistAsc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compare(album1, album2, sortMode, AlbumByNameAsc, [&](const auto& album) {
				return convertString(album.albumArtist(), sortMode, true);
			});
		}

		bool AlbumByAlbumArtistDesc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compareRev(album2, album1, sortMode, AlbumByNameAsc, [&](const auto& album) {
				return convertString(album.albumArtist(), sortMode, true);
			});
		}

		bool AlbumByDurationAsc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compare(album1, album2, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.durationSec();
			});
		}

		bool AlbumByDurationDesc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compareRev(album2, album1, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.durationSec();
			});
		}

		bool AlbumBySongcountAsc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compare(album1, album2, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.songcount();
			});
		}

		bool AlbumBySongcountDesc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compareRev(album2, album1, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.songcount();
			});
		}

		bool AlbumByRatingAsc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compare(album1, album2, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.rating();
			});
		}

		bool AlbumByRatingDesc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compareRev(album2, album1, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.rating();
			});
		}

		bool AlbumByCreationDateAsc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compare(album1, album2, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.creationDate();
			});
		}

		bool AlbumByCreationDateDesc(const Album& album1, const Album& album2, const SortModeMask sortMode)
		{
			return compareRev(album2, album1, sortMode, AlbumByNameAsc, [](const auto& album) {
				return album.creationDate();
			});
		}

		/*** Artists ***/
		bool ArtistByNameAsc(const Artist& artist1, const Artist& artist2, const SortModeMask sortMode)
		{
			auto fallback = [](const auto& a1, const auto a2, const auto /*sm*/) {
				return (a1.id() < a2.id());
			};

			return compare(artist1, artist2, sortMode, fallback, [&](const auto& artist) {
				return convertString(artist.name(), sortMode, true);
			});
		}

		bool ArtistByNameDesc(const Artist& artist1, const Artist& artist2, const SortModeMask sortMode)
		{
			return ArtistByNameAsc(artist2, artist1, sortMode); // NOLINT(readability-suspicious-call-argument)
		}

		bool ArtistByTrackCountAsc(const Artist& artist1, const Artist& artist2, const SortModeMask sortMode)
		{
			return compare(artist1, artist2, sortMode, ArtistByNameAsc, [](const auto& artist) {
				return artist.songcount();
			});
		}

		bool ArtistByTrackCountDesc(const Artist& artist1, const Artist& artist2, const SortModeMask sortMode)
		{
			return compareRev(artist2, artist1, sortMode, ArtistByNameDesc, [](const auto& artist) {
				return artist.songcount();
			});
		}

		template<typename Container, typename FN>
		void sort(Container& container, const SortModeMask sortMode, FN& fn)
		{
			Util::Algorithm::sort(container, [&](const auto& c1, const auto& c2) {
				return fn(c1, c2, sortMode);
			});
		}
	}

	void sortMetadata(MetaDataList& tracks, const Library::SortOrder sortOrder, const SortModeMask sortMode)
	{
		switch(sortOrder)
		{
			case SortOrder::TrackNumAsc:
				sort(tracks, sortMode, TracksByTrackNumAsc);
				break;
			case SortOrder::TrackNumDesc:
				sort(tracks, sortMode, TracksByTrackNumDesc);
				break;
			case SortOrder::TrackTitleAsc:
				sort(tracks, sortMode, TracksByTitleAsc);
				break;
			case SortOrder::TrackTitleDesc:
				sort(tracks, sortMode, TracksByTitleDesc);
				break;
			case SortOrder::TrackAlbumAsc:
				sort(tracks, sortMode, TracksByAlbumAsc);
				break;
			case SortOrder::TrackAlbumDesc:
				sort(tracks, sortMode, TracksByAlbumDesc);
				break;
			case SortOrder::TrackArtistAsc:
				sort(tracks, sortMode, TracksByArtistAsc);
				break;
			case SortOrder::TrackArtistDesc:
				sort(tracks, sortMode, TracksByArtistDesc);
				break;
			case SortOrder::TrackAlbumArtistAsc:
				sort(tracks, sortMode, TracksByAlbumArtistAsc);
				break;
			case SortOrder::TrackAlbumArtistDesc:
				sort(tracks, sortMode, TracksByAlbumArtistDesc);
				break;
			case SortOrder::TrackYearAsc:
				sort(tracks, sortMode, TracksByYearAsc);
				break;
			case SortOrder::TrackYearDesc:
				sort(tracks, sortMode, TracksByYearDesc);
				break;
			case SortOrder::TrackLenghtAsc:
				sort(tracks, sortMode, TracksByLengthAsc);
				break;
			case SortOrder::TrackLengthDesc:
				sort(tracks, sortMode, TracksByLengthDesc);
				break;
			case SortOrder::TrackBitrateAsc:
				sort(tracks, sortMode, TracksByBitrateAsc);
				break;
			case SortOrder::TrackBitrateDesc:
				sort(tracks, sortMode, TracksByBitrateDesc);
				break;
			case SortOrder::TrackSizeAsc:
				sort(tracks, sortMode, TracksByFilesizeAsc);
				break;
			case SortOrder::TrackSizeDesc:
				sort(tracks, sortMode, TracksByFilesizeDesc);
				break;
			case SortOrder::TrackDiscnumberAsc:
				sort(tracks, sortMode, TracksByDiscnumberAsc);
				break;
			case SortOrder::TrackDiscnumberDesc:
				sort(tracks, sortMode, TracksByDiscnumberDesc);
				break;
			case SortOrder::TrackFilenameAsc:
				sort(tracks, sortMode, TracksByFilepathAsc);
				break;
			case SortOrder::TrackFilenameDesc:
				sort(tracks, sortMode, TracksByFilepathDesc);
				break;
			case SortOrder::TrackFiletypeAsc:
				sort(tracks, sortMode, TracksByFiletypeAsc);
				break;
			case SortOrder::TrackFiletypeDesc:
				sort(tracks, sortMode, TracksByFiletypeDesc);
				break;
			case SortOrder::TrackRatingAsc:
				sort(tracks, sortMode, TracksByRatingAsc);
				break;
			case SortOrder::TrackRatingDesc:
				sort(tracks, sortMode, TracksByRatingDesc);
				break;
			case SortOrder::TrackDateAddedAsc:
				sort(tracks, sortMode, TracksByAddedDateAsc);
				break;
			case SortOrder::TrackDateAddedDesc:
				sort(tracks, sortMode, TracksByAddedDateDesc);
				break;
			case SortOrder::TrackDateModifiedAsc:
				sort(tracks, sortMode, TracksByModifiedDateAsc);
				break;
			case SortOrder::TrackDateModifiedDesc:
				sort(tracks, sortMode, TracksByModifiedDateDesc);
				break;
			default:
				break;
		}
	}

	void sortAlbums(AlbumList& albums, Library::SortOrder so, const SortModeMask sortMode)
	{
		switch(so)
		{
			case SortOrder::ArtistNameAsc:
				sort(albums, sortMode, AlbumByAlbumArtistAsc);
				break;
			case SortOrder::ArtistNameDesc:
				sort(albums, sortMode, AlbumByAlbumArtistDesc);
				break;
			case SortOrder::AlbumNameAsc:
				sort(albums, sortMode, AlbumByNameAsc);
				break;
			case SortOrder::AlbumNameDesc:
				sort(albums, sortMode, AlbumByNameDesc);
				break;
			case SortOrder::AlbumDurationAsc:
				sort(albums, sortMode, AlbumByDurationAsc);
				break;
			case SortOrder::AlbumDurationDesc:
				sort(albums, sortMode, AlbumByDurationDesc);
				break;
			case SortOrder::AlbumRatingAsc:
				sort(albums, sortMode, AlbumByRatingAsc);
				break;
			case SortOrder::AlbumRatingDesc:
				sort(albums, sortMode, AlbumByRatingDesc);
				break;
			case SortOrder::AlbumTracksAsc:
				sort(albums, sortMode, AlbumBySongcountAsc);
				break;
			case SortOrder::AlbumTracksDesc:
				sort(albums, sortMode, AlbumBySongcountDesc);
				break;
			case SortOrder::AlbumYearAsc:
				sort(albums, sortMode, AlbumByYearAsc);
				break;
			case SortOrder::AlbumYearDesc:
				sort(albums, sortMode, AlbumByYearDesc);
				break;
			case SortOrder::AlbumCreationDateAsc:
				sort(albums, sortMode, AlbumByCreationDateAsc);
				break;
			case SortOrder::AlbumCreationDateDesc:
				sort(albums, sortMode, AlbumByCreationDateDesc);
				break;
			default:
				break;
		}
	}

	void sortArtists(ArtistList& artists, Library::SortOrder so, const SortModeMask sortMode)
	{
		switch(so)
		{
			case SortOrder::ArtistNameAsc:
				sort(artists, sortMode, ArtistByNameAsc);
				break;
			case SortOrder::ArtistNameDesc:
				sort(artists, sortMode, ArtistByNameDesc);
				break;
			case SortOrder::ArtistTrackcountAsc:
				sort(artists, sortMode, ArtistByTrackCountAsc);
				break;
			case SortOrder::ArtistTrackcountDesc:
				sort(artists, sortMode, ArtistByTrackCountDesc);
				break;
			default:
				break;
		}
	}
}