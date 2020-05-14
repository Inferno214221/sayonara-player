/* MetaDataInfo.cpp */

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

#include "MetaDataInfo.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Settings/Settings.h"

#include "Components/Covers/CoverLocation.h"

#include <limits>
#include <QStringList>
#include <QDateTime>
#include <QLocale>

namespace Algorithm=Util::Algorithm;

struct MetaDataInfo::Private
{
	Util::Set<QString> albums;
	Util::Set<QString> artists;
	Util::Set<QString> album_artists;

	Util::Set<AlbumId> albumIds;
	Util::Set<ArtistId> artistIds;
	Util::Set<ArtistId> album_artistIds;

	Util::Set<QString> paths;

	Cover::Location	coverLocation;
};

MetaDataInfo::MetaDataInfo(const MetaDataList& v_md) :
	QObject(nullptr)
{
	m = Pimpl::make<Private>();

	if(v_md.isEmpty()) {
		return;
	}

	MilliSeconds length = 0;
	Filesize filesize = 0;
	Year year_min = std::numeric_limits<uint16_t>::max();
	Year year_max = 0;
	Bitrate bitrate_min = std::numeric_limits<Bitrate>::max();
	Bitrate bitrate_max = 0;
	TrackNum tracknum = 0;
	uint64_t createdate_min = std::numeric_limits<uint64_t>::max();
	uint64_t createdate_max = 0;
	uint64_t modifydate_min = createdate_min;
	uint64_t modifydate_max = 0;

	bool calc_track_num = (v_md.size() == 1);

	Util::Set<QString> filetypes;
	Util::Set<Genre> genres;
	Util::Set<QString> comments;

	for(const MetaData& md : v_md )
	{
		m->artists.insert(md.artist());
		m->albums.insert(md.album());
		m->album_artists.insert(md.albumArtist());

		m->albumIds.insert(md.albumId());
		m->artistIds.insert(md.artistId());
		m->album_artistIds.insert(md.albumArtistId());

		length += md.durationMs();
		filesize += md.filesize();

		if(calc_track_num){
			tracknum = md.trackNumber();
		}

		// bitrate
		if(md.bitrate() != 0){
			bitrate_min = std::min(md.bitrate(), bitrate_min);
			bitrate_max = std::max(md.bitrate(), bitrate_max);
		}

		// year
		if(md.year() != 0) {
			year_min = std::min(year_min, md.year());
			year_max = std::max(year_max, md.year());
		}

		if(md.createdDate() != 0){
			createdate_min = std::min(createdate_min, md.createdDate());
			createdate_max = std::max(createdate_max, md.createdDate());
		}

		if(md.modifiedDate() != 0){
			modifydate_min = std::min(modifydate_min, md.modifiedDate());
			modifydate_max = std::max(modifydate_max, md.modifiedDate());
		}

		if(!md.comment().isEmpty()){
			comments << md.comment();
		}

		// custom fields
		const CustomFieldList& custom_fields = md.customFields();

		for(const CustomField& field : custom_fields)
		{
			QString name = field.displayName();
			QString value = field.value();
			if(!value.isEmpty()){
				mAdditionalInfo << StringPair(name, value);
			}
		}

		// genre
		genres << md.genres();

		// paths
		if(!Util::File::isWWW(md.filepath()))
		{
			QString filename, dir;
			Util::File::splitFilename(md.filepath(), dir, filename);
			m->paths << dir;
		}

		else
		{
			m->paths << md.filepath();
		}

		filetypes << Util::File::getFileExtension(md.filepath());
	}

	if(bitrate_max > 0){
		insertIntervalInfoField(InfoStrings::Bitrate, bitrate_min / 1000, bitrate_max / 1000);
	}

	if(year_max > 0){
		insertIntervalInfoField(InfoStrings::Year, year_min, year_max);
	}

	insertNumericInfoField(InfoStrings::nTracks, v_md.count());
	insertFilesize(filesize);
	insertFiletype(filetypes);
	insertPlayingTime(length);
	insertGenre(genres);
	insertComment(comments);
	insertCreatedates(createdate_min, createdate_max);
	insertModifydates(modifydate_min, modifydate_max);

	calcHeader(v_md);
	calcSubheader(tracknum);
	calcCoverLocation(v_md);
}

MetaDataInfo::~MetaDataInfo() {}

void MetaDataInfo::calcHeader() {}
void MetaDataInfo::calcHeader(const MetaDataList& lst)
{
	if(lst.size() == 1){
		const MetaData& md = lst[0];
		mHeader = md.title();
	}

	else{
		mHeader = Lang::get(Lang::VariousTracks);
	}
}


void MetaDataInfo::calcSubheader() {}
void MetaDataInfo::calcSubheader(uint16_t tracknum)
{
	mSubheader = calcArtistString();

	if(tracknum){
		mSubheader += CAR_RET + calcTracknumString(tracknum) + " " +
				Lang::get(Lang::TrackOn) + " ";
	}

	else{
		mSubheader += CAR_RET + Lang::get(Lang::On) + " ";
	}

	mSubheader += calcAlbumString();
}

void MetaDataInfo::calcCoverLocation() {}
void MetaDataInfo::calcCoverLocation(const MetaDataList& lst)
{
	if(lst.isEmpty()){
		m->coverLocation = Cover::Location::invalidLocation();
		return;
	}

	if(lst.size() == 1)
	{
		const MetaData& md = lst[0];
		m->coverLocation = Cover::Location::coverLocation(md);
	}

	else if(albumIds().size() == 1)
	{
		Album album;

		album.setId(albumIds().first());
		album.setName(m->albums.first());
		album.setArtists(m->artists.toList());

		if(m->album_artists.size() > 0)
		{
			album.setAlbumArtist(m->album_artists.first());
		}

		album.setDatabaseId(lst[0].databaseId());

		QStringList path_hint;
		for(const MetaData& md : lst){
			path_hint << md.filepath();
		}
		album.setPathHint(path_hint);

		m->coverLocation = Cover::Location::xcoverLocation(album);
	}

	else if(m->albums.size() == 1 && m->artists.size() == 1)
	{
		QString album = m->albums.first();
		QString artist = m->artists.first();

		m->coverLocation = Cover::Location::coverLocation(album, artist);
	}

	else if(m->albums.size() == 1 && m->album_artists.size() == 1)
	{
		QString album = m->albums.first();
		QString artist = m->album_artists.first();

		m->coverLocation = Cover::Location::coverLocation(album, artist);
	}

	else if(m->albums.size() == 1)
	{
		QString album = m->albums.first();
		m->coverLocation = Cover::Location::coverLocation(album, m->artists.toList());
	}

	else
	{
		m->coverLocation = Cover::Location::invalidLocation();
	}
}


QString MetaDataInfo::calcArtistString() const
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


QString MetaDataInfo::calcAlbumString()
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

QString MetaDataInfo::calcTracknumString( uint16_t tracknum )
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


void MetaDataInfo::insertPlayingTime(MilliSeconds ms)
{
	QString str = Util::msToString(ms, "$De $He $M:$S");
	mInfo.insert(InfoStrings::PlayingTime, str);
}

void MetaDataInfo::insertGenre(const Util::Set<Genre>& genres)
{
	if(genres.isEmpty()){
		return;
	}

	QStringList genres_list;
	for(auto g : genres){
		genres_list << g.name();
	}

	QString str = genres_list.join(", ");
	QString old_genre = mInfo[InfoStrings::Genre];
	if(!old_genre.isEmpty()){
		old_genre += ", ";
	}

	mInfo[InfoStrings::Genre] = old_genre + str;
}

void MetaDataInfo::insertFilesize(uint64_t filesize)
{
	QString str = Util::File::getFilesizeString(filesize);
	mInfo.insert(InfoStrings::Filesize, str);
}

void MetaDataInfo::insertComment(const Util::Set<QString>& comments)
{
	QString comments_str(QStringList(comments.toList()).join(", "));
	if(comments_str.size() > 50){
		comments_str = comments_str.left(50) + "...";
	}

	mInfo.insert(InfoStrings::Comment, comments_str);
}

static QString get_date_text(uint64_t min_date, uint64_t max_date)
{
	QDateTime dd_min_date = Util::intToDate(min_date);
	QDateTime dd_max_date = Util::intToDate(max_date);

	QLocale locale = Util::Language::getCurrentLocale();
	QString text = dd_min_date.toString(locale.dateTimeFormat(QLocale::ShortFormat));
	if(min_date != max_date)
	{
		text += " -\n" + dd_max_date.toString(locale.dateTimeFormat(QLocale::ShortFormat));
	}

	return text;
}

void MetaDataInfo::insertCreatedates(uint64_t min_date, uint64_t max_date)
{
	mInfo.insert(InfoStrings::CreateDate, get_date_text(min_date, max_date));
}

void MetaDataInfo::insertModifydates(uint64_t min_date, uint64_t max_date)
{
	mInfo.insert(InfoStrings::ModifyDate, get_date_text(min_date, max_date));
}

void MetaDataInfo::insertFiletype(const Util::Set<QString>& filetypes)
{
	QStringList filetypes_str(filetypes.toList());
	mInfo.insert(InfoStrings::Filetype, filetypes_str.join(", "));
}

QString MetaDataInfo::header() const
{
	return mHeader;
}

QString MetaDataInfo::subheader() const
{
	return mSubheader;
}


QString MetaDataInfo::getInfoString(InfoStrings idx) const
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
		case InfoStrings::Filetype:
			return Lang::get(Lang::Filetype);
		case InfoStrings::Comment:
			return Lang::get(Lang::Comment);
		case InfoStrings::CreateDate:
			return Lang::get(Lang::Created);
		case InfoStrings::ModifyDate:
			return Lang::get(Lang::Modified);

		default: break;
	}

	return "";
}

QString MetaDataInfo::infostring() const
{
	QString str;

	for(auto it=mInfo.cbegin(); it != mInfo.cend(); it++)
	{
		str += BOLD(getInfoString(it.key())) + it.value() + CAR_RET;
	}

	return str;
}

QList<StringPair> MetaDataInfo::infostringMap() const
{
	QList<StringPair> ret;

	for(auto it=mInfo.cbegin(); it != mInfo.cend(); it++)
	{
		QString value = it.value();
		if(value.isEmpty())
		{
			/// todo: none
			value = "-";
		}

		ret << StringPair(getInfoString(it.key()), value);
	}

	if(!mAdditionalInfo.isEmpty())
	{
		ret << StringPair(QString(), QString());
		ret.append(mAdditionalInfo);
	}

	return ret;
}

// todo: delete me
QString MetaDataInfo::additionalInfostring() const
{

	return QString();
}

// todo: delete me
QString MetaDataInfo::pathsString() const
{
	return this->paths().join(CAR_RET);
}


QStringList MetaDataInfo::paths() const
{
	bool dark = (GetSetting(Set::Player_Style) == 1);
	QStringList ret;
	QList<Library::Info> lib_infos = Library::Manager::instance()->allLibraries();
	QStringList lib_paths;

	for(const Library::Info& li : lib_infos)
	{
		lib_paths << li.path();
	}

	Algorithm::sort(lib_paths, [](const QString& lp1, const QString& lp2){
		return (lp1.length() > lp2.length());
	});

	for(const QString& path : Algorithm::AsConst(m->paths))
	{
		QString name = path;

		for(const QString& lp : Algorithm::AsConst(lib_paths))
		{
			if(name.contains(lp))
			{
				name.replace(lp, "...");
				break;
			}
		}

		QString link = Util::createLink(name, dark, false, path);
		ret << link;
	}

	return ret;
}



Cover::Location MetaDataInfo::coverLocation() const
{
	return m->coverLocation;
}

const Util::Set<QString>& MetaDataInfo::albums() const
{
	return m->albums;
}

const Util::Set<QString> &MetaDataInfo::artists() const
{
	return m->artists;
}

const Util::Set<QString> &MetaDataInfo::albumArtists() const
{
	return m->album_artists;
}

const Util::Set<AlbumId> &MetaDataInfo::albumIds() const
{
	return m->albumIds;
}

const Util::Set<ArtistId> &MetaDataInfo::artistIds() const
{
	return m->artistIds;
}

const Util::Set<ArtistId> &MetaDataInfo::albumArtistIds() const
{
	return m->album_artistIds;
}

void MetaDataInfo::insertIntervalInfoField(InfoStrings key, int min, int max)
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

	mInfo.insert(key, str);
}


void MetaDataInfo::insertNumericInfoField(InfoStrings key, int number)
{
	QString str = QString::number(number);

	mInfo.insert(key, str);
}
