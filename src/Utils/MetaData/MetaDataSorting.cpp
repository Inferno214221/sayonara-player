/* MetaDataSorting.cpp */

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

#include "MetaDataSorting.h"
#include "MetaData.h"
#include "MetaDataList.h"
#include "Artist.h"
#include "Album.h"

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
	if(md1.track_number() < md2.track_number()){
		return true;
	}

	if(md1.track_number() == md2.track_number()){
		return TracksByTitleAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByTrackNumDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.track_number() < md1.track_number()){
		return true;
	}

	if(md1.track_number() == md2.track_number()){
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
	switch(compare_string(md1.album_artist(), md2.album_artist())){
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
	switch(compare_string(md2.album_artist(), md1.album_artist())){
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
	if(md1.duration_ms() < md2.duration_ms()){
		return true;
	}

	if(md1.duration_ms() == md2.duration_ms()){
		return TracksByArtistAsc(md1, md2);
	}

	return false;
}

bool MetaDataSorting::TracksByLengthDesc(const MetaData& md1, const MetaData& md2)
{
	if(md2.duration_ms() < md1.duration_ms()){
		return true;
	}

	if(md1.duration_ms() == md2.duration_ms()){
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
	switch(compare_string(Util::File::get_file_extension(md1.filepath()), Util::File::get_file_extension(md2.filepath()) ))
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
	switch(compare_string(Util::File::get_file_extension(md2.filepath()), Util::File::get_file_extension(md1.filepath())))
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
	Relation rel = compare_string(album1.album_artists().join(","), album2.album_artists().join(","));
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
			return (album1.id < album2.id);
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
			return (album1.id < album2.id);
		case Greater:
			return false;
		case Lesser:
		default:
			return true;
	}
}

bool MetaDataSorting::AlbumByYearAsc(const Album& album1, const Album& album2)
{
	if(album1.year < album2.year){
		return true;
	}

	if(album1.year == album2.year){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByYearDesc(const Album& album1, const Album& album2)
{
	if(album2.year < album1.year){
		return true;
	}

	if(album1.year == album2.year){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByDurationAsc(const Album& album1, const Album& album2)
{
	if(album1.length_sec < album2.length_sec){
		return true;
	}

	if(album1.length_sec == album2.length_sec){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByDurationDesc(const Album& album1, const Album& album2)
{
	if(album2.length_sec < album1.length_sec){
		return true;
	}

	if(album1.length_sec == album2.length_sec){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByTracksAsc(const Album& album1, const Album& album2)
{
	if(album1.num_songs < album2.num_songs){
		return true;
	}

	if(album1.num_songs == album2.num_songs){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByTracksDesc(const Album& album1, const Album& album2)
{
	if(album2.num_songs < album1.num_songs){
		return true;
	}

	if(album1.num_songs == album2.num_songs){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByRatingAsc(const Album& album1, const Album& album2)
{
	if(album1.rating < album2.rating){
		return true;
	}

	if(album1.rating == album2.rating){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

bool MetaDataSorting::AlbumByRatingDesc(const Album& album1, const Album& album2)
{
	if(album2.rating < album1.rating){
		return true;
	}

	if(album1.rating == album2.rating){
		return AlbumByNameAsc(album1, album2);
	}

	return false;
}

void MetaDataSorting::sort_metadata(MetaDataList& v_md, Library::SortOrder so)
{
	using So=Library::SortOrder;
	switch(so)
	{
		case So::TrackNumAsc:
			Algorithm::sort(v_md, TracksByTrackNumAsc);
			break;
		case So::TrackNumDesc:
			Algorithm::sort(v_md, TracksByTrackNumDesc);
			break;
		case So::TrackTitleAsc:
			Algorithm::sort(v_md, TracksByTitleAsc);
			break;
		case So::TrackTitleDesc:
			Algorithm::sort(v_md, TracksByTitleDesc);
			break;
		case So::TrackAlbumAsc:
			Algorithm::sort(v_md, TracksByAlbumAsc);
			break;
		case So::TrackAlbumDesc:
			Algorithm::sort(v_md, TracksByAlbumDesc);
			break;
		case So::TrackArtistAsc:
			Algorithm::sort(v_md, TracksByArtistAsc);
			break;
		case So::TrackArtistDesc:
			Algorithm::sort(v_md, TracksByArtistDesc);
			break;
		case So::TrackAlbumArtistAsc:
			Algorithm::sort(v_md, TracksByAlbumArtistAsc);
			break;
		case So::TrackAlbumArtistDesc:
			Algorithm::sort(v_md, TracksByAlbumArtistDesc);
			break;
		case So::TrackYearAsc:
			Algorithm::sort(v_md, TracksByYearAsc);
			break;
		case So::TrackYearDesc:
			Algorithm::sort(v_md, TracksByYearDesc);
			break;
		case So::TrackLenghtAsc:
			Algorithm::sort(v_md, TracksByLengthAsc);
			break;
		case So::TrackLengthDesc:
			Algorithm::sort(v_md, TracksByLengthDesc);
			break;
		case So::TrackBitrateAsc:
			Algorithm::sort(v_md, TracksByBitrateAsc);
			break;
		case So::TrackBitrateDesc:
			Algorithm::sort(v_md, TracksByBitrateDesc);
			break;
		case So::TrackSizeAsc:
			Algorithm::sort(v_md, TracksByFilesizeAsc);
			break;
		case So::TrackSizeDesc:
			Algorithm::sort(v_md, TracksByFilesizeDesc);
			break;
		case So::TrackDiscnumberAsc:
			Algorithm::sort(v_md, TracksByDiscnumberAsc);
			break;
		case So::TrackDiscnumberDesc:
			Algorithm::sort(v_md, TracksByDiscnumberDesc);
			break;
		case So::TrackFiletypeAsc:
			Algorithm::sort(v_md, TracksByFiletypeAsc);
			break;
		case So::TrackFiletypeDesc:
			Algorithm::sort(v_md, TracksByFiletypeDesc);
			break;
		case So::TrackRatingAsc:
			Algorithm::sort(v_md, TracksByRatingAsc);
			break;
		case So::TrackRatingDesc:
			Algorithm::sort(v_md, TracksByRatingDesc);
			break;
		default:
			break;
	}
}


void MetaDataSorting::sort_albums(AlbumList& albums, Library::SortOrder so)
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

void MetaDataSorting::sort_artists(ArtistList& artists, Library::SortOrder so)
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

void MetaDataSorting::set_ignore_article(bool b)
{
	ignore_article = b;
}
