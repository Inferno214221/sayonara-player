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
#include <QDateTime>

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"

namespace Algorithm=Util::Algorithm;

static bool ignore_article=false;

enum Relation
{
	Lesser,
	Greater,
	Equal
};

static Relation compare_string(const QString& s1, const QString& s2)
{
	if(s1 < s2){
		return Lesser;
	}

	if(s1 == s2){
		return Equal;
	}

	return Greater;
}

bool MetaDataSorting::TracksByTitleAsc(const MetaData& md1, const MetaData& md2)
{
	switch(compare_string(md1.title(), md2.title())){
		case Equal:
			return (md1.filepath() < md2.filepath());
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByTitleDesc(const MetaData& md1, const MetaData& md2)
{
	return TracksByTitleAsc(md2, md1);
}

bool MetaDataSorting::TracksByTrackNumAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.trackNumber() < md2.trackNumber()){
		return true;
	}

	if(md1.trackNumber() == md2.trackNumber()){
		return TracksByTitleAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByTrackNumDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.trackNumber() < md1.trackNumber()){
		return true;
	}

	if(md1.trackNumber() == md2.trackNumber()){
		return TracksByTitleDesc(md1, md2);
	}

	return false;
}


bool MetaDataSorting::TracksByDiscnumberAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.discnumber() < md2.discnumber()){
		return true;
	}

	if(md1.discnumber() == md2.discnumber()){
		return TracksByTrackNumAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByDiscnumberDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.discnumber() < md1.discnumber()){
		return true;
	}

	if(md1.discnumber() == md2.discnumber()){
		return TracksByTrackNumDesc(md1, md2);
	}

	return false;
}


bool MetaDataSorting::TracksByAlbumAsc(const MetaData& md1, const MetaData& md2)
{
	switch(compare_string(md1.album(), md2.album())){
		case Equal:
			return TracksByDiscnumberAsc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByAlbumDesc(const MetaData& md1, const MetaData& md2)
{
	switch(compare_string(md2.album(), md1.album())){
		case Equal:
			return TracksByDiscnumberDesc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByArtistAsc(const MetaData& md1, const MetaData& md2)
{
	QString n1 = md1.artist();
	QString n2 = md2.artist();

	if(ignore_article)
	{
		if(n1.startsWith("the ", Qt::CaseInsensitive)){
			n1.remove(0, 4);
		}

		if(n2.startsWith("the ", Qt::CaseInsensitive)){
			n2.remove(0, 4);
		}
	}

	switch(compare_string(n1, n2)){
		case Equal:
			return TracksByAlbumAsc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByArtistDesc(const MetaData& md1, const MetaData& md2)
{
	QString n1 = md1.artist();
	QString n2 = md2.artist();

	if(ignore_article)
	{
		if(n1.startsWith("the ", Qt::CaseInsensitive)){
			n1.remove(0, 4);
		}

		if(n2.startsWith("the ", Qt::CaseInsensitive)){
			n2.remove(0, 4);
		}
	}

	switch(compare_string(n2, n1)){
		case Equal:
			return TracksByAlbumAsc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByAlbumArtistAsc(const MetaData& md1, const MetaData& md2)
{
	switch(compare_string(md1.albumArtist(), md2.albumArtist())){
		case Equal:
			return TracksByAlbumAsc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByAlbumArtistDesc(const MetaData& md1, const MetaData& md2)
{
	switch(compare_string(md2.albumArtist(), md1.albumArtist())){
		case Equal:
			return TracksByAlbumDesc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByYearAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.year() < md2.year()){
		return true;
	}

	if(md1.year() == md2.year()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByYearDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.year() < md1.year()){
		return true;
	}

	if(md1.year() == md2.year()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByLengthAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.durationMs() < md2.durationMs()){
		return true;
	}

	if(md1.durationMs() == md2.durationMs()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByLengthDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.durationMs() < md1.durationMs()){
		return true;
	}

	if(md1.durationMs() == md2.durationMs()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByBitrateAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.bitrate() < md2.bitrate()){
		return true;
	}

	if(md1.bitrate() == md2.bitrate()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByBitrateDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.bitrate() < md1.bitrate()){
		return true;
	}

	if(md1.bitrate() == md2.bitrate()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByFilesizeAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.filesize() < md2.filesize()){
		return true;
	}

	if(md1.filesize() == md2.filesize()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByFilesizeDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.filesize() < md1.filesize()){
		return true;
	}

	if(md1.filesize() == md2.filesize()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByFiletypeAsc(const MetaData& md1, const MetaData& md2)
{
	switch(compare_string(Util::File::getFileExtension(md1.filepath()), Util::File::getFileExtension(md2.filepath()) ))
	{
		case Equal:
			return TracksByArtistAsc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByFiletypeDesc(const MetaData& md1, const MetaData& md2)
{
	switch(compare_string(Util::File::getFileExtension(md2.filepath()), Util::File::getFileExtension(md1.filepath())))
	{
		case Equal:
			return TracksByArtistDesc(md1, md2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::TracksByRatingAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.rating() < md2.rating()){
		return true;
	}

	if(md1.rating() == md2.rating()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByRatingDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.rating() < md1.rating()){
		return true;
	}

	if(md1.rating() == md2.rating()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByAddedDateAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.createdDate() < md2.createdDate()){
		return true;
	}

	if(md1.createdDate() == md2.createdDate()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByAddedDateDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.createdDate() < md1.createdDate()){
		return true;
	}

	if(md1.createdDate() == md2.createdDate()){
		return TracksByArtistDesc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByModifiedDateAsc(const MetaData& md1, const MetaData& md2)
{
	if(md1.modifiedDate() < md2.modifiedDate()){
		return true;
	}

	if(md1.modifiedDate() == md2.modifiedDate()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByModifiedDateDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.modifiedDate() < md1.modifiedDate()){
		return true;
	}

	if(md1.modifiedDate() == md2.modifiedDate()){
		return TracksByArtistDesc(md1, md2);
	}

	return false;
}



bool MetaDataSorting::ArtistByNameAsc(const Artist& artist1, const Artist& artist2)
{
	QString n1 = artist1.name();
	QString n2 = artist2.name();

	if(ignore_article)
	{
		if(n1.startsWith("the ", Qt::CaseInsensitive)){
			n1.remove(0, 4);
		}

		if(n2.startsWith("the ", Qt::CaseInsensitive)){
			n2.remove(0, 4);
		}
	}

	switch(compare_string(n1, n2)){
		case Equal:
			return (artist1.id() < artist2.id());
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::ArtistByNameDesc(const Artist& artist1, const Artist& artist2)
{
	QString n1 = artist1.name();
	QString n2 = artist2.name();

	if(ignore_article)
	{
		if(n1.startsWith("the ", Qt::CaseInsensitive)){
			n1.remove(0, 4);
		}

		if(n2.startsWith("the ", Qt::CaseInsensitive)){
			n2.remove(0, 4);
		}
	}

	switch(compare_string(n2, n1)){
		case Equal:
			return (artist1.id() < artist2.id());
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::ArtistByTrackCountAsc(const Artist& artist1, const Artist& artist2)
{
	if(artist1.songcount() < artist2.songcount()){
		return true;
	}

	if(artist1.songcount() == artist2.songcount()){
		return ArtistByNameAsc(artist1, artist2);
	}

	return false;
}

bool MetaDataSorting::ArtistByTrackCountDesc(const Artist& artist1, const Artist& artist2)
{
	if(artist2.songcount() < artist1.songcount()){
		return true;
	}

	if(artist1.songcount() == artist2.songcount()){
		return ArtistByNameAsc(artist1, artist2);
	}

	return false;
}


bool MetaDataSorting::AlbumByArtistNameAsc(const Album& album1, const Album& album2)
{
	Relation rel = compare_string(album1.albumArtist(), album2.albumArtist());
	if(rel == Equal)
	{
		rel = compare_string(album1.artists().join(","), album2.artists().join(","));
	}

	switch(rel){
		case Equal:
			return AlbumByYearAsc(album1, album2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::AlbumByArtistNameDesc(const Album& album1, const Album& album2)
{
	switch(compare_string(album2.artists().join(","), album1.artists().join(","))){
		case Equal:
			return AlbumByYearDesc(album1, album2);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}


bool MetaDataSorting::AlbumByNameAsc(const Album& album1, const Album& album2)
{
	switch(compare_string(album1.name(), album2.name())){
		case Equal:
			return (album1.id() < album2.id());
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::AlbumByNameDesc(const Album& album1, const Album& album2)
{
	switch(compare_string(album2.name(), album1.name())){
		case Equal:
			return (album1.id() < album2.id());
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::AlbumByYearAsc(const Album& album1, const Album& album2)
{
	if(album1.year() < album2.year()){
		return true;
	}

	if(album1.year() == album2.year()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByYearDesc(const Album& album1, const Album& album2)
{
	if(album2.year() < album1.year()){
		return true;
	}

	if(album1.year() == album2.year()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByDurationAsc(const Album& album1, const Album& album2)
{
	if(album1.durationSec() < album2.durationSec()){
		return true;
	}

	if(album1.durationSec() == album2.durationSec()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByDurationDesc(const Album& album1, const Album& album2)
{
	if(album2.durationSec() < album1.durationSec()){
		return true;
	}

	if(album1.durationSec() == album2.durationSec()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByTracksAsc(const Album& album1, const Album& album2)
{
	if(album1.songcount() < album2.songcount()){
		return true;
	}

	if(album1.songcount() == album2.songcount()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByTracksDesc(const Album& album1, const Album& album2)
{
	if(album2.songcount() < album1.songcount()){
		return true;
	}

	if(album1.songcount() == album2.songcount()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByRatingAsc(const Album& album1, const Album& album2)
{
	if(album1.rating() < album2.rating()){
		return true;
	}

	if(album1.rating() == album2.rating()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByRatingDesc(const Album& album1, const Album& album2)
{
	if(album2.rating() < album1.rating()){
		return true;
	}

	if(album1.rating() == album2.rating()){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

void MetaDataSorting::sortMetadata(MetaDataList& tracks, Library::SortOrder so)
{
	using So=Library::SortOrder;
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
	using So=Library::SortOrder;
	switch(so)
	{
		case So::ArtistNameAsc:
			Algorithm::sort(albums, AlbumByArtistNameAsc);
			break;
		case So::ArtistNameDesc:
			Algorithm::sort(albums, AlbumByArtistNameDesc);
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
			Algorithm::sort(albums, AlbumByTracksAsc);
			break;
		case So::AlbumTracksDesc:
			Algorithm::sort(albums, AlbumByTracksDesc);
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
	using So=Library::SortOrder;
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
	ignore_article = b;
}
