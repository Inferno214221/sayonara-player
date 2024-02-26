/* id3.cpp */

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

#include "Tagging.h"
#include "Tagging/TaggingCover.h"
#include "Tagging/TaggingExtraFields.h"
#include "Tagging/TaggingUtils.h"
#include "Tagging/Models/Discnumber.h"
#include "Tagging/Models/Popularimeter.h"
#include "Tagging/FileTypeResolver.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QFileInfo>
#include <QRegExp>
#include <QStringList>
#include <QDateTime>

using namespace Tagging::Utils;
namespace FileUtils = ::Util::File;

namespace
{
	struct ReadingProperties
	{
		TagLib::AudioProperties::ReadStyle readStyle {TagLib::AudioProperties::ReadStyle::Fast};
		bool readAudioProperties {true};
	};

	ReadingProperties getReadingProperties(Tagging::Quality quality)
	{
		ReadingProperties readingProperties;

		switch(quality)
		{
			case Tagging::Quality::Quality:
				readingProperties.readStyle = TagLib::AudioProperties::Accurate;
				break;
			case Tagging::Quality::Standard:
				readingProperties.readStyle = TagLib::AudioProperties::Average;
				break;
			case Tagging::Quality::Fast:
				readingProperties.readStyle = TagLib::AudioProperties::Fast;
				break;
			case Tagging::Quality::Dirty:
				readingProperties.readStyle = TagLib::AudioProperties::Fast;
				readingProperties.readAudioProperties = false;
				break;
		};

		return readingProperties;
	}

	QString getTitleFromFilename(const QString& filepath)
	{
		const auto filename = FileUtils::getFilenameOfPath(filepath);
		return (filename.size() > 4)
		       ? filename.left(filename.length() - 4)
		       : filename;
	}

	void setDate(MetaData& track)
	{
		const auto fileInfo = QFileInfo(track.filepath());
		QDateTime createDate;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
		createDate = fileInfo.birthTime();
		if(!createDate.isValid())
		{
			createDate = fileInfo.metadataChangeTime();
		}
#else
		createDate = fileInfo.created();
#endif

		if(!createDate.isValid())
		{
			createDate = fileInfo.lastModified();
		}

		track.setCreatedDate(::Util::dateToInt(createDate));
		track.setModifiedDate(Util::dateToInt(fileInfo.lastModified()));
	}

	QStringList extractGenres(const QString& genreString)
	{
		auto genres = genreString.split(QRegExp(",|/|;"));
		Util::Algorithm::transform(genres, [](const auto& genre) {
			return genre.trimmed();
		});

		genres.removeDuplicates();
		genres.removeAll("");

		return genres;
	}

	template<typename T, typename Setter>
	void updateIfUnequal(const T& value1, const T& value2, const Tagging::ParsedTag& tag, const Setter& setter)
	{
		if(value1 != value2)
		{
			setter(tag, value2);
		}
	}
}

bool Tagging::Utils::getMetaDataOfFile(MetaData& track, Quality quality)
{
	Tagging::FileTypeResolver::addFileTypeResolver();

	const auto fileInfo = QFileInfo(track.filepath());
	track.setFilesize(static_cast<Filesize>(fileInfo.size()));
	setDate(track);

	if(fileInfo.size() <= 0)
	{
		return false;
	}

	const auto readingProperties = getReadingProperties(quality);
	auto fileRef = TagLib::FileRef(TagLib::FileName(track.filepath().toUtf8()),
	                               readingProperties.readAudioProperties,
	                               readingProperties.readStyle);

	if(!isValidFile(fileRef))
	{
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << track.filepath() << ": Err 1";
		return false;
	}

	const auto parsedTag = getParsedTagFromFileRef(fileRef);
	if(!parsedTag.tag)
	{
		return false;
	}

	const auto artist = convertString(parsedTag.tag->artist());
	const auto album = convertString(parsedTag.tag->album());
	const auto title = convertString(parsedTag.tag->title());
	const auto genre = convertString(parsedTag.tag->genre());
	const auto comment = convertString(parsedTag.tag->comment());
	const auto year = parsedTag.tag->year();
	const auto trackNumber = parsedTag.tag->track();
	const auto bitrate = (quality != Quality::Dirty)
	                     ? fileRef.audioProperties()->bitrate() * 1000
	                     : 0;

	const auto length = (quality != Quality::Dirty)
	                    ? fileRef.audioProperties()->length() * 1000
	                    : 0;

	const auto genres = extractGenres(genre);

	track.setAlbum(album);
	track.setArtist(artist);
	track.setTitle(title.isEmpty() ? getTitleFromFilename(track.filepath()) : title);
	track.setDurationMs(length);
	track.setYear(Year(year));
	track.setTrackNumber(static_cast<TrackNum>(trackNumber));
	track.setBitrate(Bitrate(bitrate));
	track.setGenres(genres);
	track.setComment(comment);

	if(const auto albumArtist = Tagging::readAlbumArtist(parsedTag); albumArtist.has_value())
	{
		track.setAlbumArtist(albumArtist.value());
	}

	if(const auto discnumber = Tagging::readDiscnumber(parsedTag); discnumber.has_value())
	{
		track.setDiscnumber(discnumber.value().disc);
		track.setDiscCount(discnumber.value().disccount);
	}

	if(const auto popularimeter = Tagging::readPopularimeter(parsedTag); popularimeter.has_value())
	{
		track.setRating(popularimeter->rating);
	}

	const auto hasCover = static_cast<int>(Tagging::hasCover(parsedTag));
	track.addCustomField("has-album-art", "", QString::number(hasCover));

	return true;
}

bool Tagging::Utils::setMetaDataOfFile(const MetaData& track)
{
	Tagging::FileTypeResolver::addFileTypeResolver();

	const auto filepath = track.filepath();
	const auto fileInfo = QFileInfo(filepath);
	if(fileInfo.size() <= 0)
	{
		return false;
	}

	auto fileRef = TagLib::FileRef(TagLib::FileName(filepath.toUtf8()));
	if(!isValidFile(fileRef))
	{
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << track.filepath() << ": Err 2";
		return false;
	}

	const auto album = convertString(track.album());
	const auto artist = convertString(track.artist());
	const auto title = convertString(track.title());
	const auto genre = convertString(track.genresToString());
	const auto comment = convertString(track.comment());

	const auto parsedTag = getParsedTagFromFileRef(fileRef);
	if(!parsedTag.tag)
	{
		return false;
	}

	parsedTag.tag->setAlbum(album);
	parsedTag.tag->setArtist(artist);
	parsedTag.tag->setTitle(title);
	parsedTag.tag->setGenre(genre);
	parsedTag.tag->setYear(track.year());
	parsedTag.tag->setTrack(track.trackNumber());
	parsedTag.tag->setComment(comment);

	Tagging::writePopularimeter(parsedTag, Models::Popularimeter("sayonara", track.rating(), 0));
	Tagging::writeDiscnumber(parsedTag, Models::Discnumber(track.discnumber(), track.discCount()));
	Tagging::writeAlbumArtist(parsedTag, track.albumArtist());

	const auto success = fileRef.save();
	if(!success)
	{
		spLog(Log::Warning, "Tagging") << "Could not save " << track.filepath();
	}

	return success;
}

bool Tagging::Utils::setOnlyChangedMetaDataOfFile(const MetaData& oldTrack, const MetaData& newTrack)
{
	Tagging::FileTypeResolver::addFileTypeResolver();

	const auto filepath = newTrack.filepath();
	const auto fileInfo = QFileInfo(filepath);
	if(fileInfo.size() <= 0)
	{
		return false;
	}

	auto fileRef = TagLib::FileRef(TagLib::FileName(filepath.toUtf8()));
	if(!isValidFile(fileRef))
	{
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << newTrack.filepath() << ": Err 2";
		return false;
	}

	const auto parsedTag = getParsedTagFromFileRef(fileRef);
	if(!parsedTag.tag)
	{
		return false;
	}

	updateIfUnequal(oldTrack.album(), newTrack.album(), parsedTag, [](const auto& ptag, const auto& value) {
		ptag.tag->setAlbum(convertString(value));
	});

	updateIfUnequal(oldTrack.artist(), newTrack.artist(), parsedTag, [](const auto& ptag, const auto& value) {
		ptag.tag->setArtist(convertString(value));
	});

	updateIfUnequal(oldTrack.title(), newTrack.title(), parsedTag, [](const auto& ptag, const auto& value) {
		ptag.tag->setTitle(convertString(value));
	});

	updateIfUnequal(oldTrack.genresToString(), newTrack.genresToString(), parsedTag,
	                [](const auto& tag, const auto& value) {
		                tag.tag->setGenre(convertString(value));
	                });

	updateIfUnequal(oldTrack.year(), newTrack.year(), parsedTag, [](const auto& ptag, const auto& value) {
		ptag.tag->setYear(value);
	});

	updateIfUnequal(oldTrack.trackNumber(), newTrack.trackNumber(), parsedTag, [](const auto& ptag, const auto& value) {
		ptag.tag->setTrack(value);
	});

	updateIfUnequal(oldTrack.comment(), newTrack.comment(), parsedTag, [](const auto& ptag, const auto& value) {
		ptag.tag->setComment(convertString(value));
	});

	updateIfUnequal(oldTrack.rating(), newTrack.rating(), parsedTag, [](const auto& ptag, const auto& value) {
		Tagging::writePopularimeter(ptag, Models::Popularimeter("sayonara", value, 0));
	});

	updateIfUnequal(std::pair {oldTrack.discnumber(), oldTrack.discCount()},
	                std::pair {newTrack.discnumber(), newTrack.discCount()},
	                parsedTag,
	                [](const auto& ptag, const auto& value) {
		                Tagging::writeDiscnumber(ptag, Models::Discnumber(value.first, value.second));
	                });

	updateIfUnequal(oldTrack.albumArtist(), newTrack.albumArtist(), parsedTag,
	                [](const auto& ptag, const auto& value) {
		                Tagging::writeAlbumArtist(ptag, value);
	                });

	const auto success = fileRef.save();
	if(!success)
	{
		spLog(Log::Warning, "Tagging") << "Could not save " << newTrack.filepath();
	}

	return success;
}
