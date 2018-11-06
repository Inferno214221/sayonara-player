/* MetaDataInfo.cpp */

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

#include "MetaDataInfo.h"
#include "Components/Library/LibraryManager.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/globals.h"
#include "Utils/FileUtils.h"
#include "Utils/Language.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Library/LibraryInfo.h"

#include "Components/Covers/CoverLocation.h"

#include <limits>
#include <QStringList>

struct MetaDataInfo::Private
{
	Util::Set<QString> albums;
	Util::Set<QString> artists;
	Util::Set<QString> album_artists;

	Util::Set<AlbumId> album_ids;
	Util::Set<ArtistId> artist_ids;
	Util::Set<ArtistId> album_artist_ids;

	QStringList paths;

	Cover::Location	cover_location;
};

MetaDataInfo::MetaDataInfo(const MetaDataList& v_md) :
	QObject(nullptr),
	SayonaraClass()
{
	m = Pimpl::make<Private>();

	if(v_md.isEmpty()) {
		return;
	}

	MilliSeconds length = 0;
	uint64_t filesize = 0;
	uint16_t year_min = std::numeric_limits<uint16_t>::max();
	uint16_t year_max = 0;
	Bitrate bitrate_min = std::numeric_limits<Bitrate>::max();
	Bitrate bitrate_max = 0;
	uint16_t tracknum = 0;
	bool calc_track_num = (v_md.size() == 1);

	QStringList genres;

	for(const MetaData& md : v_md )
	{
		m->artists.insert(md.artist());
		m->albums.insert(md.album());
		m->album_artists.insert(md.album_artist());

		m->album_ids.insert(md.album_id);
		m->artist_ids.insert(md.artist_id);
		m->album_artist_ids.insert(md.album_artist_id());

		length += md.length_ms;
		filesize += md.filesize;

		if(calc_track_num){
			tracknum = md.track_num;
		}

		// bitrate
		if(md.bitrate != 0){
			bitrate_min = std::min(md.bitrate, bitrate_min);
		}

		bitrate_max = std::max(md.bitrate, bitrate_max);

		// year
		if(md.year != 0) {
			year_min = std::min(year_min, md.year);
		}

		year_max = std::max(year_max, md.year);

		// custom fields
		const CustomFieldList& custom_fields = md.get_custom_fields();

		for(const CustomField& field : custom_fields)
		{
			QString name = field.get_display_name();
			QString value = field.get_value();
			if(!value.isEmpty()){
				_additional_info << StringPair(name, value);
			}
		}

		// genre
		genres << md.genres_to_list();

		// paths
		if(!Util::File::is_www(md.filepath()))
		{
			QString filename, dir;
			Util::File::split_filename(md.filepath(), dir, filename);
			if(!m->paths.contains(dir))
			{
				m->paths << dir;
			}
		}

		else
		{
			m->paths << md.filepath();
		}
	}

	if(bitrate_max > 0){
		insert_interval_info_field(InfoStrings::Bitrate, bitrate_min / 1000, bitrate_max / 1000);
	}

	if(year_max > 0){
		insert_interval_info_field(InfoStrings::Year, year_min, year_max);
	}

	insert_numeric_info_field(InfoStrings::nTracks, v_md.count());
	insert_filesize(filesize);
	insert_playing_time(length);

	genres.removeDuplicates();
	insert_genre(genres);

	calc_header(v_md);
	calc_subheader(tracknum);
	calc_cover_location(v_md);
}

MetaDataInfo::~MetaDataInfo() {}

void MetaDataInfo::calc_header() {}
void MetaDataInfo::calc_header(const MetaDataList& lst)
{
	if(lst.size() == 1){
		const MetaData& md = lst[0];
		_header = md.title();
	}

	else{
		_header = Lang::get(Lang::VariousTracks);
	}
}


void MetaDataInfo::calc_subheader() {}
void MetaDataInfo::calc_subheader(uint16_t tracknum)
{
	_subheader = calc_artist_str();

	if(tracknum){
		_subheader += CAR_RET + calc_tracknum_str(tracknum) + " " +
				Lang::get(Lang::TrackOn) + " ";
	}

	else{
		_subheader += CAR_RET + Lang::get(Lang::On) + " ";
	}

	_subheader += calc_album_str();
}

void MetaDataInfo::calc_cover_location() {}
void MetaDataInfo::calc_cover_location(const MetaDataList& lst)
{
	if(lst.size() == 1)
	{
		const MetaData& md = lst[0];
		m->cover_location = Cover::Location::cover_location(md);
	}

	else if(album_ids().size() == 1)
	{
		Album album;

		album.id = album_ids().first();
		album.set_name(m->albums.first());
		album.set_artists(m->artists.toList());
		album.set_album_artists(m->album_artists.toList());
		album.set_db_id(lst[0].db_id());

		m->cover_location = Cover::Location::cover_location(album);
	}

	else if(m->albums.size() == 1 && m->artists.size() == 1)
	{
		QString album = m->albums.first();
		QString artist = m->artists.first();

		m->cover_location = Cover::Location::cover_location(album, artist);
	}

	else if(m->albums.size() == 1 && m->album_artists.size() == 1)
	{
		QString album = m->albums.first();
		QString artist = m->album_artists.first();

		m->cover_location = Cover::Location::cover_location(album, artist);
	}

	else if(m->albums.size() == 1)
	{
		QString album = m->albums.first();
		m->cover_location = Cover::Location::cover_location(album, m->artists.toList());
	}

	else
	{
		m->cover_location = Cover::Location::invalid_location();
	}
}


QString MetaDataInfo::calc_artist_str() const
{
	QString str;

	if( m->album_artists.size() == 1){
		str = m->album_artists.first();
	}

	else if( m->artists.size() == 1 ){
		str = m->artists.first();
	}

	else{
		QString::number(m->artists.size()) + " " + Lang::get(Lang::VariousArtists);
	}

	return str;
}


QString MetaDataInfo::calc_album_str()
{
	QString str;

	if( m->albums.size() == 1){
		str = m->albums.first();
	}

	else{
		QString::number(m->artists.size()) + " " + Lang::get(Lang::VariousAlbums) ;
	}

	return str;
}

QString MetaDataInfo::calc_tracknum_str( uint16_t tracknum )
{
	QString str;
	switch (tracknum)
	{
		case 1:
			str = Lang::get(Lang::First);
			break;
		case 2:
			str = Lang::get(Lang::Second);
			break;
		case 3:
			str = Lang::get(Lang::Third);
			break;
		default:
			str = QString::number(tracknum) + Lang::get(Lang::Th);
		break;
	}

	return str;
}


void MetaDataInfo::insert_playing_time(MilliSeconds ms)
{
	QString str = Util::cvt_ms_to_string(ms);
	_info.insert(InfoStrings::PlayingTime, str);
}

void MetaDataInfo::insert_genre(const QStringList& lst)
{
	if(lst.isEmpty()){
		return;
	}

	QString str = lst.join(", ");
	QString old_genre = _info[InfoStrings::Genre];
	if(!old_genre.isEmpty()){
		old_genre += ", ";
	}

	_info[InfoStrings::Genre] = old_genre + str;
}

void MetaDataInfo::insert_filesize(uint64_t filesize)
{
	QString str = Util::File::calc_filesize_str(filesize);
	_info.insert(InfoStrings::Filesize, str);
}

QString MetaDataInfo::header() const
{
	return _header;
}

QString MetaDataInfo::subheader() const
{
	return _subheader;
}


QString MetaDataInfo::get_info_string(InfoStrings idx) const
{
	switch(idx)
	{
		case InfoStrings::nTracks:
			return QString("#") + Lang::get(Lang::Tracks);
		case InfoStrings::nAlbums:
			return QString("#") + Lang::get(Lang::Albums);
		case InfoStrings::nArtists:
			return QString("#") + Lang::get(Lang::Artists);
		case InfoStrings::Filesize:
			return Lang::get(Lang::Filesize);
		case InfoStrings::PlayingTime:
			return Lang::get(Lang::PlayingTime);
		case InfoStrings::Year:
			return Lang::get(Lang::Year);
		case InfoStrings::Sampler:
			return Lang::get(Lang::Sampler);
		case InfoStrings::Bitrate:
			return Lang::get(Lang::Bitrate);
		case InfoStrings::Genre:
			return Lang::get(Lang::Genre);
		default: break;
	}

	return "";
}

QString MetaDataInfo::infostring() const
{
	QString str;

	for(auto it=_info.cbegin(); it != _info.cend(); it++)
	{
		str += BOLD(get_info_string(it.key())) + it.value() + CAR_RET;
	}

	return str;
}

QList<StringPair> MetaDataInfo::infostring_map() const
{
	QList<StringPair> ret;

	for(auto it=_info.cbegin(); it != _info.cend(); it++)
	{
		QString value = it.value();
		if(value.isEmpty()){
			value = Lang::get(Lang::None);

		}

		ret << StringPair(get_info_string(it.key()), value);
	}

	if(!_additional_info.isEmpty())
	{
		ret << StringPair(QString(), QString());
		ret.append(_additional_info);
	}

	return ret;
}

// todo: delete me
QString MetaDataInfo::additional_infostring() const
{

	return QString();
}

// todo: delete me
QString MetaDataInfo::pathsstring() const
{
	return this->paths().join(CAR_RET);
}


QStringList MetaDataInfo::paths() const
{
	bool dark = (Settings::instance()->get<Set::Player_Style>() == 1);
	QStringList ret;
	QList<Library::Info> lib_infos = Library::Manager::instance()->all_libraries();
	QStringList lib_paths;

	for(const Library::Info& li : lib_infos)
	{
		lib_paths << li.path();
	}

	Util::sort(lib_paths, [](const QString& lp1, const QString& lp2){
		return (lp1.length() > lp2.length());
	});

	for(const QString& path : ::Util::AsConst(m->paths))
	{
		QString name = path;

		for(const QString& lp : ::Util::AsConst(lib_paths))
		{
			if(name.contains(lp))
			{
				name.replace(lp, "...");
				break;
			}
		}

		QString link = Util::create_link(name, dark, path, false);
		ret << link;
	}

	return ret;
}



Cover::Location MetaDataInfo::cover_location() const
{
	return m->cover_location;
}

const Util::Set<QString>& MetaDataInfo::albums() const
{
	return m->albums;
}

const Util::Set<QString> &MetaDataInfo::artists() const
{
	return m->artists;
}

const Util::Set<QString> &MetaDataInfo::album_artists() const
{
	return m->album_artists;
}

const Util::Set<AlbumId> &MetaDataInfo::album_ids() const
{
	return m->album_ids;
}

const Util::Set<ArtistId> &MetaDataInfo::artist_ids() const
{
	return m->artist_ids;
}

const Util::Set<ArtistId> &MetaDataInfo::album_artist_ids() const
{
	return m->album_artist_ids;
}

void MetaDataInfo::insert_interval_info_field(InfoStrings key, int min, int max)
{
	QString str;

	if(min == max){
		str = QString::number(min);
	}

	else {
		str = QString::number(min) + " - " + QString::number(max);
	}

	if(key == InfoStrings::Bitrate){
		str += " kBit/s";
	}

	_info.insert(key, str);
}


void MetaDataInfo::insert_numeric_info_field(InfoStrings key, int number)
{
	QString str = QString::number(number);

	_info.insert(key, str);
}
