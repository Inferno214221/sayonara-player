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

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Settings/Settings.h"

#include "Components/Covers/CoverLocation.h"

#include <limits>
#include <QStringList>
#include <QDateTime>
#include <QLocale>

namespace Algorithm = Util::Algorithm;

struct MetaDataInfo::Private
{
	Util::Set<QString> albums;
	Util::Set<QString> artists;
	Util::Set<QString> album_artists;

	Util::Set<AlbumId> albumIds;
	Util::Set<ArtistId> artistIds;
	Util::Set<ArtistId> album_artistIds;

	Util::Set<QString> paths;

	Cover::Location coverLocation;
};

MetaDataInfo::MetaDataInfo(const MetaDataList& tracks) :
	QObject(nullptr)
{
	m = Pimpl::make<Private>();

	if(tracks.isEmpty())
	{
		return;
	}

	MilliSeconds length = 0;
	Filesize filesize = 0;
	Year minYear = std::numeric_limits<uint16_t>::max();
	Year maxYear = 0;
	Bitrate minBitrate = std::numeric_limits<Bitrate>::max();
	Bitrate maxBitrate = 0;
	TrackNum tracknum = 0;
	uint64_t minCreateDate = std::numeric_limits<uint64_t>::max();
	uint64_t maxCreateDate = 0;
	uint64_t minModifyDate = minCreateDate;
	uint64_t maxModifyDate = 0;

	Util::Set<QString> filetypes;
	Util::Set<Genre> genres;
	Util::Set<QString> comments;

	for(const auto& track : tracks)
	{
		m->artists.insert(track.artist());
		m->albums.insert(track.album());
		m->album_artists.insert(track.albumArtist());

		m->albumIds.insert(track.albumId());
		m->artistIds.insert(track.artistId());
		m->album_artistIds.insert(track.albumArtistId());

		length += track.durationMs();
		filesize += track.filesize();

		if(tracks.size() == 1)
		{
			tracknum = track.trackNumber();
		}

		// bitrate
		if(track.bitrate() != 0)
		{
			minBitrate = std::min(track.bitrate(), minBitrate);
			maxBitrate = std::max(track.bitrate(), maxBitrate);
		}

		// year
		if(track.year() != 0)
		{
			minYear = std::min(minYear, track.year());
			maxYear = std::max(maxYear, track.year());
		}

		if(track.createdDate() != 0)
		{
			minCreateDate = std::min(minCreateDate, track.createdDate());
			maxCreateDate = std::max(maxCreateDate, track.createdDate());
		}

		if(track.modifiedDate() != 0)
		{
			minModifyDate = std::min(minModifyDate, track.modifiedDate());
			maxModifyDate = std::max(maxModifyDate, track.modifiedDate());
		}

		if(!track.comment().isEmpty())
		{
			comments << track.comment();
		}

		// custom fields
		const auto& customFields = track.customFields();

		for(const auto& field : customFields)
		{
			const auto name = field.displayName();
			const auto value = field.value();
			if(!value.isEmpty())
			{
				mAdditionalInfo << StringPair(name, value);
			}
		}

		// genre
		genres << track.genres();

		const auto path = Util::File::isWWW(track.filepath())
			? track.filepath()
			: Util::File::getParentDirectory(track.filepath());

		m->paths << path;

		filetypes << Util::File::getFileExtension(track.filepath());
	}

	if(maxBitrate > 0)
	{
		insertIntervalInfoField(InfoStrings::Bitrate, minBitrate / 1000, maxBitrate / 1000);
	}

	if(maxYear > 0)
	{
		insertIntervalInfoField(InfoStrings::Year, minYear, maxYear);
	}

	insertNumericInfoField(InfoStrings::nTracks, tracks.count());
	insertFilesize(filesize);
	insertFiletype(filetypes);
	insertPlayingTime(length);
	insertGenre(genres);
	insertComment(comments);
	insertCreatedates(minCreateDate, maxCreateDate);
	insertModifydates(minModifyDate, maxModifyDate);

	calcHeader(tracks);
	calcSubheader(tracknum);
	calcCoverLocation(tracks);
}

MetaDataInfo::~MetaDataInfo() {}

void MetaDataInfo::calcHeader() {}

void MetaDataInfo::calcHeader(const MetaDataList& tracks)
{
	mHeader = (tracks.size() == 1)
		? tracks.first().title()
		: Lang::get(Lang::VariousTracks);
}

void MetaDataInfo::calcSubheader() {}

void MetaDataInfo::calcSubheader(uint16_t tracknum)
{
	mSubheader = calcArtistString();

	if(tracknum)
	{
		mSubheader += CAR_RET + calcTracknumString(tracknum) + " " +
		              Lang::get(Lang::TrackOn) + " ";
	}

	else
	{
		mSubheader += CAR_RET + Lang::get(Lang::On) + " ";
	}

	mSubheader += calcAlbumString();
}

void MetaDataInfo::calcCoverLocation() {}

void MetaDataInfo::calcCoverLocation(const MetaDataList& tracks)
{
	if(tracks.isEmpty())
	{
		m->coverLocation = Cover::Location::invalidLocation();
		return;
	}

	if(tracks.size() == 1)
	{
		m->coverLocation = Cover::Location::coverLocation(tracks[0]);
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

		album.setDatabaseId(tracks[0].databaseId());

		QStringList pathHints;
		Util::Algorithm::transform(tracks, pathHints, [](const auto& track){
			return track.filepath();
		});

		album.setPathHint(pathHints);

		m->coverLocation = Cover::Location::xcoverLocation(album);
	}

	else if(m->albums.size() == 1 && m->artists.size() == 1)
	{
		const auto album = m->albums.first();
		const auto artist = m->artists.first();

		m->coverLocation = Cover::Location::coverLocation(album, artist);
	}

	else if(m->albums.size() == 1 && m->album_artists.size() == 1)
	{
		const auto album = m->albums.first();
		const auto artist = m->album_artists.first();

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
	if(m->album_artists.size() == 1)
	{
		return m->album_artists.first();
	}

	if(m->artists.size() == 1)
	{
		return m->artists.first();
	}

	return QString::number(m->artists.size()) + " " + Lang::get(Lang::VariousArtists);
}

QString MetaDataInfo::calcAlbumString()
{
	return (m->albums.size() == 1)
	       ? m->albums.first()
	       : QString::number(m->artists.size()) + " " + Lang::get(Lang::VariousAlbums);
}

QString MetaDataInfo::calcTracknumString(uint16_t tracknum)
{
	QString str;
	switch(tracknum)
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
	const auto timeString = Util::msToString(ms, "$De $He $M:$S");
	mInfo.insert(InfoStrings::PlayingTime, timeString);
}

void MetaDataInfo::insertGenre(const Util::Set<Genre>& genres)
{
	if(genres.isEmpty())
	{
		return;
	}

	QStringList genreList;
	Util::Algorithm::transform(genres, genreList, [](const auto& genre) {
		return genre.name();
	});

	const auto genreString = genreList.join(", ");
	auto oldGenre = mInfo[InfoStrings::Genre];
	if(!oldGenre.isEmpty())
	{
		oldGenre += ", ";
	}

	mInfo[InfoStrings::Genre] = oldGenre + genreString;
}

void MetaDataInfo::insertFilesize(uint64_t filesize)
{
	const auto filesizeString = Util::File::getFilesizeString(filesize);
	mInfo.insert(InfoStrings::Filesize, filesizeString);
}

void MetaDataInfo::insertComment(const Util::Set<QString>& comments)
{
	auto commentsString = QStringList(comments.toList()).join(", ");
	if(commentsString.size() > 50)
	{
		commentsString = commentsString.left(50) + "...";
	}

	mInfo.insert(InfoStrings::Comment, commentsString);
}

static QString get_date_text(uint64_t minDate, uint64_t maxDate)
{
	const auto minDateTime = Util::intToDate(minDate);
	const auto maxDateTime = Util::intToDate(maxDate);

	const auto locale = Util::Language::getCurrentLocale();

	auto text = minDateTime.toString(locale.dateTimeFormat(QLocale::ShortFormat));
	if(minDate != maxDate)
	{
		text += " -\n" + maxDateTime.toString(locale.dateTimeFormat(QLocale::ShortFormat));
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

		default:
			break;
	}

	return "";
}

QString MetaDataInfo::infostring() const
{
	QString str;

	for(auto it = mInfo.cbegin(); it != mInfo.cend(); it++)
	{
		str += BOLD(getInfoString(it.key())) + it.value() + CAR_RET;
	}

	return str;
}

QList<StringPair> MetaDataInfo::infostringMap() const
{
	QList<StringPair> ret;

	for(auto it = mInfo.cbegin(); it != mInfo.cend(); it++)
	{
		const auto value = (it.value().isEmpty()) ? "-" : it.value();
		ret << StringPair(getInfoString(it.key()), value);
	}

	if(!mAdditionalInfo.isEmpty())
	{
		ret << StringPair(QString(), QString());
		ret.append(mAdditionalInfo);
	}

	return ret;
}

QStringList MetaDataInfo::paths() const
{
	QStringList links;

	const auto dark = (GetSetting(Set::Player_Style) == 1);
	Util::Algorithm::transform(m->paths, links, [&](const auto& path){
		return Util::createLink(path, dark, false, path);
	});

	return links;
}

Cover::Location MetaDataInfo::coverLocation() const
{
	return m->coverLocation;
}

const Util::Set<QString>& MetaDataInfo::albums() const
{
	return m->albums;
}

const Util::Set<QString>& MetaDataInfo::artists() const
{
	return m->artists;
}

const Util::Set<QString>& MetaDataInfo::albumArtists() const
{
	return m->album_artists;
}

const Util::Set<AlbumId>& MetaDataInfo::albumIds() const
{
	return m->albumIds;
}

const Util::Set<ArtistId>& MetaDataInfo::artistIds() const
{
	return m->artistIds;
}

const Util::Set<ArtistId>& MetaDataInfo::albumArtistIds() const
{
	return m->album_artistIds;
}

void MetaDataInfo::insertIntervalInfoField(InfoStrings key, int min, int max)
{
	QString str;

	if(min == max)
	{
		str = QString::number(min);
	}

	else
	{
		str = QString::number(min) + " - " + QString::number(max);
	}

	if(key == InfoStrings::Bitrate)
	{
		str += " kBit/s";
	}

	mInfo.insert(key, str);
}

void MetaDataInfo::insertNumericInfoField(InfoStrings key, int number)
{
	QString str = QString::number(number);

	mInfo.insert(key, str);
}
