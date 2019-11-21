/* Sorting.cpp */

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

#include "Sorting.h"

#include "Utils/Algorithm.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"

#include <functional>
#include <QMap>

namespace Algorithm=Util::Algorithm;

namespace Compare
{
	bool artistNameAsc(const Artist& artist1, const Artist& artist2) {
		return (artist1.name() < artist2.name());
	}

	bool artistNameDesc(const Artist& artist1, const Artist& artist2) {
		return artistNameAsc(artist2, artist1);
	}

	bool artistTrackcountAsc(const Artist& artist1, const Artist& artist2) {
		return (artist1.songcount() < artist2.songcount());
	}

	bool artistTrackcountDesc(const Artist& artist1, const Artist& artist2) {
		return artistTrackcountAsc(artist2, artist1);
	}

	bool albumNameAsc(const Album& album1, const Album& album2) {
		return (album1.name() < album2.name());
	}

	bool albumNameDesc(const Album& album1, const Album& album2) {
		return albumNameAsc(album2, album1);
	}

	bool albumYearAsc(const Album& album1, const Album& album2) {
		return (album1.year() < album2.year());
	}

	bool albumYearDesc(const Album& album1, const Album& album2) {
		return albumYearAsc(album2, album1);
	}

	bool albumRatingAsc(const Album& album1, const Album& album2) {
		return (album1.rating() < album2.rating());
	}

	bool albumRatingDesc(const Album& album1, const Album& album2) {
		return albumRatingAsc(album2, album1);
	}

	bool albumDurationAsc(const Album& album1, const Album& album2) {
		return (album1.duration_sec() < album2.duration_sec());
	}

	bool albumDurationDesc(const Album& album1, const Album& album2) {
		return albumDurationAsc(album2, album1);
	}

	bool trackTitleAsc(const MetaData& md1, const MetaData& md2){
		return (md1.title() < md2.title());
	}

	bool trackTitleDesc(const MetaData& md1, const MetaData& md2){
		return trackTitleAsc(md2, md1);
	}

	bool trackNumAsc(const MetaData& md1, const MetaData& md2){
		return (md1.track_number() < md2.track_number());
	}

	bool trackNumDesc(const MetaData& md1, const MetaData& md2){
		return trackNumAsc(md2, md1);
	}

	bool trackAlbumAsc(const MetaData& md1, const MetaData& md2){
		if(md1.album() == md2.album()){
			return trackNumAsc(md1, md2);
		}

		return (md1.album() < md2.album());
	}

	bool trackAlbumDesc(const MetaData& md1, const MetaData& md2){
		return trackAlbumAsc(md2, md1);
	}

	bool trackArtistAsc(const MetaData& md1, const MetaData& md2){
		if(md1.artist() == md2.artist()){
			return trackAlbumAsc(md1, md2);
		}

		return (md1.artist() < md2.artist());
	}

	bool trackArtistDesc(const MetaData& md1, const MetaData& md2){
		return trackArtistAsc(md2, md1);
	}

	bool trackYearAsc(const MetaData& md1, const MetaData& md2){
		if(md1.year() == md2.year()){
			return trackArtistAsc(md1, md2);
		}

		return (md1.year() < md2.year());
	}

	bool trackYearDesc(const MetaData& md1, const MetaData& md2){
		return trackYearAsc(md2, md1);
	}

	bool trackLengthAsc(const MetaData& md1, const MetaData& md2){
		return (md1.duration_ms() < md2.duration_ms());
	}

	bool trackLengthDesc(const MetaData& md1, const MetaData& md2){
		return trackLengthAsc(md2, md1);
	}

	bool trackBitrateAsc(const MetaData& md1, const MetaData& md2){
		if(md1.bitrate() == md2.bitrate()){
			return trackArtistAsc(md1, md2);
		}

		return (md1.bitrate() < md2.bitrate());
	}

	bool trackBitrateDesc(const MetaData& md1, const MetaData& md2){
		return trackBitrateAsc(md2, md1);
	}
}

void SC::Sorting::sort_artists(ArtistList& artists, Library::SortOrder so)
{
	using namespace Library;
	using SortFn=std::function<bool (const Artist&, const Artist&)>;
	QMap<Library::SortOrder, SortFn> functions
	{
		{SortOrder::ArtistNameAsc,			Compare::artistNameAsc},
		{SortOrder::ArtistNameDesc,			Compare::artistNameDesc},
		{SortOrder::ArtistTrackcountAsc,	Compare::artistTrackcountAsc},
		{SortOrder::ArtistTrackcountDesc,	Compare::artistTrackcountDesc}
	};

	if(functions.contains(so)){
		Algorithm::sort(artists, functions[so]);
	}
}

void SC::Sorting::sort_albums(AlbumList& albums, Library::SortOrder so)
{
	using namespace Library;
	using SortFn=std::function<bool (const Album&, const Album&)>;
	QMap<Library::SortOrder, SortFn> functions
	{
		{SortOrder::AlbumNameAsc,		&Compare::albumNameAsc},
		{SortOrder::AlbumNameDesc,		&Compare::albumNameDesc},
		{SortOrder::AlbumYearAsc,		&Compare::albumYearAsc},
		{SortOrder::AlbumYearDesc,		&Compare::albumYearDesc},
		{SortOrder::AlbumDurationAsc,	&Compare::albumDurationAsc},
		{SortOrder::AlbumDurationDesc,	&Compare::albumDurationDesc}
	};

	if(functions.contains(so)){
		Algorithm::sort(albums, functions[so]);
	}
}


void SC::Sorting::sort_tracks(MetaDataList& tracks, Library::SortOrder so)
{
	using namespace Library;
	using SortFn=std::function<bool (const MetaData&, const MetaData&)>;

	QMap<Library::SortOrder, SortFn> functions
	{
		{SortOrder::TrackNumAsc,		&Compare::trackNumAsc},
		{SortOrder::TrackNumDesc,		&Compare::trackNumDesc},
		{SortOrder::TrackTitleAsc,		&Compare::trackTitleAsc},
		{SortOrder::TrackTitleDesc,		&Compare::trackTitleDesc},
		{SortOrder::TrackAlbumAsc,		&Compare::trackAlbumAsc},
		{SortOrder::TrackAlbumDesc,		&Compare::trackAlbumDesc},
		{SortOrder::TrackArtistAsc,		&Compare::trackArtistAsc},
		{SortOrder::TrackArtistDesc,	&Compare::trackArtistDesc},
		{SortOrder::TrackYearAsc,		&Compare::trackYearAsc},
		{SortOrder::TrackYearDesc,		&Compare::trackYearDesc},
		{SortOrder::TrackLenghtAsc,		&Compare::trackLengthAsc},
		{SortOrder::TrackLengthDesc,	&Compare::trackLengthDesc},
		{SortOrder::TrackBitrateAsc,	&Compare::trackBitrateAsc},
		{SortOrder::TrackBitrateDesc,	&Compare::trackBitrateDesc}
	};

	if(functions.contains(so)){
		Algorithm::sort(tracks, functions[so]);
	}
}
