/* LibraryItemInfo.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#include "LibraryItemInfo.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"

#include <QDateTime>

namespace
{
	using InfoFieldMap = QMap<InfoStrings, QString>;
	using AdditionalInfo = LibraryItemInfo::AdditionalInfo;

	struct TracksInfo
	{
		int tracksCount {0};
		MilliSeconds length {0};
		Filesize filesize {0};
		Year minYear {std::numeric_limits<Year>::max()};
		Year maxYear {0};
		Bitrate minBitrate {std::numeric_limits<Bitrate>::max()};
		Bitrate maxBitrate {0};
		TrackNum tracknum {0};
		uint64_t minCreateDate {std::numeric_limits<uint64_t>::max()};
		uint64_t maxCreateDate {0};
		uint64_t minModifyDate {0};
		uint64_t maxModifyDate {0};

		Util::Set<QString> filetypes;
		Util::Set<Genre> genres;
		Util::Set<QString> comments;
	};

	TracksInfo updateTracksInfo(const MetaData& track, TracksInfo tracksInfo)
	{
		tracksInfo.length += track.durationMs();
		tracksInfo.filesize += track.filesize();

		if(track.bitrate() != 0)
		{
			tracksInfo.minBitrate = std::min(track.bitrate(), tracksInfo.minBitrate);
			tracksInfo.maxBitrate = std::max(track.bitrate(), tracksInfo.maxBitrate);
		}

		if(track.year() != 0)
		{
			tracksInfo.minYear = std::min(tracksInfo.minYear, track.year());
			tracksInfo.maxYear = std::max(tracksInfo.maxYear, track.year());
		}

		if(track.createdDate() != 0)
		{
			tracksInfo.minCreateDate = std::min(tracksInfo.minCreateDate, track.createdDate());
			tracksInfo.maxCreateDate = std::max(tracksInfo.maxCreateDate, track.createdDate());
		}

		if(track.modifiedDate() != 0)
		{
			tracksInfo.minModifyDate = std::min(tracksInfo.minModifyDate, track.modifiedDate());
			tracksInfo.maxModifyDate = std::max(tracksInfo.maxModifyDate, track.modifiedDate());
		}

		if(!track.comment().isEmpty())
		{
			tracksInfo.comments << track.comment();
		}

		tracksInfo.genres << track.genres();
		tracksInfo.filetypes << Util::File::getFileExtension(track.filepath());

		return tracksInfo;
	}

	void insertIntervalIntoInfoField(const InfoStrings key, int min, int max, InfoFieldMap& infoField)
	{
		auto str = (min == max)
		           ? QString::number(min)
		           : QString("%1 - %2").arg(min).arg(max);

		if(key == InfoStrings::Bitrate)
		{
			str += " kBit/s";
		}

		infoField.insert(key, str);
	}

	void insertPlayingTimeIntoInfoField(const MilliSeconds ms, InfoFieldMap& infoField)
	{
		const auto timeString = Util::msToString(ms, "$De $He $M:$S");
		infoField.insert(InfoStrings::PlayingTime, timeString);
	}

	void insertGenreIntoInfoField(const Util::Set<Genre>& genres, InfoFieldMap& infoField)
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
		auto oldGenre = infoField[InfoStrings::Genre];
		if(!oldGenre.isEmpty())
		{
			oldGenre += ", ";
		}

		infoField[InfoStrings::Genre] = oldGenre + genreString;
	}

	void insertFilesizeIntoInfoField(const Filesize filesize, InfoFieldMap& infoField)
	{
		const auto filesizeString = Util::File::getFilesizeString(filesize);
		infoField.insert(InfoStrings::Filesize, filesizeString);
	}

	void insertCommentIntoInfoField(const Util::Set<QString>& comments, InfoFieldMap& infoField)
	{
		constexpr const auto maxCommentLength = 50;
		auto commentsString = QStringList(comments.toList()).join(", ");
		if(commentsString.size() > maxCommentLength)
		{
			commentsString = commentsString.left(maxCommentLength) + "...";
		}

		infoField.insert(InfoStrings::Comment, commentsString);
	}

	QString convertDateToText(const uint64_t minDate, const uint64_t maxDate)
	{
		const auto minDateTime = Util::intToDate(minDate);
		const auto maxDateTime = Util::intToDate(maxDate);

		const auto locale = Util::Language::getCurrentLocale();

		return (minDate == maxDate)
		       ? minDateTime.toString(locale.dateTimeFormat(QLocale::ShortFormat))
		       : QString("%1 - %2")
			       .arg(minDateTime.toString(locale.dateTimeFormat(QLocale::ShortFormat))) // clazy:exclude=qstring-arg
			       .arg(maxDateTime.toString(locale.dateTimeFormat(QLocale::ShortFormat)));
	}

	void insertCreatedatesIntoInfoField(const uint64_t minDate, const uint64_t maxDate, InfoFieldMap& infoField)
	{
		infoField.insert(InfoStrings::CreateDate, convertDateToText(minDate, maxDate));
	}

	void insertModifydatesIntoInfoField(const uint64_t minDate, const uint64_t maxDate, InfoFieldMap& infoField)
	{
		infoField.insert(InfoStrings::ModifyDate, convertDateToText(minDate, maxDate));
	}

	void insertFiletypeIntoInfoField(const Util::Set<QString>& filetypes, InfoFieldMap& infoField)
	{
		const auto stringList = filetypes.toList();
		infoField.insert(InfoStrings::Filetype, stringList.join(", "));
	}

	InfoFieldMap convertTracksInfoToInfoFieldMap(const TracksInfo& tracksInfo)
	{
		auto result = InfoFieldMap {};
		result.insert(InfoStrings::TrackCount, QString::number(tracksInfo.tracksCount));

		if(tracksInfo.maxBitrate > 0)
		{
			constexpr const auto kilobitFactor = 1000;
			insertIntervalIntoInfoField(InfoStrings::Bitrate,
			                            static_cast<int>(tracksInfo.minBitrate / kilobitFactor),
			                            static_cast<int>(tracksInfo.maxBitrate / kilobitFactor),
			                            result);
		}

		if(tracksInfo.maxYear > 0)
		{
			insertIntervalIntoInfoField(InfoStrings::Year, tracksInfo.minYear, tracksInfo.maxYear, result);
		}

		insertFilesizeIntoInfoField(tracksInfo.filesize, result);
		insertFiletypeIntoInfoField(tracksInfo.filetypes, result);
		insertPlayingTimeIntoInfoField(tracksInfo.length, result);
		insertGenreIntoInfoField(tracksInfo.genres, result);
		if(tracksInfo.tracksCount == 1)
		{
			insertCommentIntoInfoField(tracksInfo.comments, result);
		}
		insertCreatedatesIntoInfoField(tracksInfo.minCreateDate, tracksInfo.maxCreateDate, result);
		insertModifydatesIntoInfoField(tracksInfo.minModifyDate, tracksInfo.maxModifyDate, result);

		return result;
	}

	Util::Set<QString> updatePaths(const MetaData& track, Util::Set<QString> paths)
	{
		const auto path = Util::File::isWWW(track.filepath())
		                  ? track.filepath()
		                  : Util::File::getParentDirectory(track.filepath());
		paths << path;

		return paths;
	}
}

struct LibraryItemInfo::Private
{
	Util::Set<QString> albums;
	Util::Set<QString> artists;
	Util::Set<QString> albumArtists;

	Util::Set<AlbumId> albumIds;
	Util::Set<ArtistId> artistIds;
	Util::Set<ArtistId> albumArtistIds;

	InfoFieldMap basicInfo;
	Util::Set<QString> paths;

	explicit Private(const MetaDataList& metaDataList)
	{
		auto tracksInfo = TracksInfo {};
		tracksInfo.tracksCount = metaDataList.count();
		if(metaDataList.size() == 1)
		{
			tracksInfo.tracknum = metaDataList[0].trackNumber();
		}

		for(const auto& metadata: metaDataList)
		{
			artists.insert(metadata.artist());
			albums.insert(metadata.album());
			albumArtists.insert(metadata.albumArtist());

			albumIds.insert(metadata.albumId());
			artistIds.insert(metadata.artistId());
			albumArtistIds.insert(metadata.albumArtistId());

			tracksInfo = updateTracksInfo(metadata, tracksInfo);
			paths = updatePaths(metadata, std::move(paths));
		}

		basicInfo = convertTracksInfoToInfoFieldMap(tracksInfo);
	}
};

LibraryItemInfo::LibraryItemInfo(const MetaDataList& metaDataList) :
	m {Pimpl::make<Private>(metaDataList)} {}

LibraryItemInfo::~LibraryItemInfo() = default;

QStringList LibraryItemInfo::paths() const
{
	QStringList links;

	const auto dark = (GetSetting(Set::Player_Style) == 1);
	Util::Algorithm::transform(m->paths, links, [&](const auto& path) {
		return Util::createLink(path, dark, false, path);
	});

	return links;
}

const Util::Set<QString>& LibraryItemInfo::albums() const { return m->albums; }

const Util::Set<QString>& LibraryItemInfo::artists() const { return m->artists; }

const Util::Set<QString>& LibraryItemInfo::albumArtists() const { return m->albumArtists; }

const Util::Set<AlbumId>& LibraryItemInfo::albumIds() const { return m->albumIds; }

const Util::Set<ArtistId>& LibraryItemInfo::artistIds() const { return m->artistIds; }

QString LibraryItemInfo::calcArtistString() const
{
	if(albumArtists().size() == 1)
	{
		return albumArtists().first();
	}

	if(artists().size() == 1)
	{
		return artists().first();
	}

	return QString::number(artists().size()) + " " + Lang::get(Lang::VariousArtists);
}

QString LibraryItemInfo::calcAlbumString() const
{
	return (albums().size() == 1)
	       ? albums().first()
	       : QString::number(artists().size()) + " " + Lang::get(Lang::VariousAlbums);
}

AdditionalInfo LibraryItemInfo::additionalInfo() const
{
	AdditionalInfo result;
	for(auto it = m->basicInfo.begin(); it != m->basicInfo.end(); it++)
	{
		const auto& infoKey = it.key();
		const auto& value = it.value();

		result << StringPair {LibraryItemInfo::convertInfoKeyToString(infoKey), value};
	}

	const auto additionalData = this->additionalData();
	for(const auto& item: additionalData)
	{
		result << item;
	}

	return result;
}

QString LibraryItemInfo::convertInfoKeyToString(InfoStrings infoKey)
{
	switch(infoKey)
	{
		case InfoStrings::TrackCount:
			return QString("#") + Lang::get(Lang::Tracks);
		case InfoStrings::AlbumCount:
			return QString("#") + Lang::get(Lang::Albums);
		case InfoStrings::ArtistCount:
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
			return {};
	}
}
