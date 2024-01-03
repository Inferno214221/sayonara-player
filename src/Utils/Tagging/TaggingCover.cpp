/* TaggingCover.cpp */

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

#include "TaggingCover.h"
#include "TaggingUtils.h"

#include "Models/Cover.h"
#include "ID3v2/Cover.h"
#include "MP4/Cover.h"
#include "Xiph/Cover.h"

#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"
#include "Utils/StandardPaths.h"

#include <QByteArray>
#include <QString>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>

#include <taglib/fileref.h>

#include <optional>

using Tagging::ParsedTag;

namespace
{
	struct TagInfo
	{
		TagLib::FileRef fileRef;
		Tagging::ParsedTag parsedTag;
	};

	template<typename T>
	bool isCoverFrameAvailable(const T& frame)
	{
		return (frame.isFrameAvailable());
	}

	template<typename T>
	bool writeCoverFrame(T& frame, const Models::Cover& coverModel)
	{
		const auto success = frame.write(coverModel);
		if(!success)
		{
			spLog(Log::Warning, &frame) << "Cannot write cover";
		}

		return success;
	}

	template<typename T>
	std::optional<Models::Cover> readCoverFrame(const T& frame)
	{
		Models::Cover cover;
		return (frame.isFrameAvailable() && frame.read(cover))
		       ? std::optional(cover)
		       : std::nullopt;
	}

	std::optional<TagInfo> parsedTagFromPath(const QString& filepath)
	{
		TagInfo info;

		info.fileRef = (QFileInfo(filepath).size() > 0)
		               ? TagLib::FileRef(TagLib::FileName(filepath.toUtf8()))
		               : TagLib::FileRef();

		info.parsedTag = Tagging::isValidFile(info.fileRef)
		                 ? Tagging::getParsedTagFromFileRef(info.fileRef)
		                 : Tagging::ParsedTag();

		return (!info.fileRef.isNull() && info.parsedTag.tag)
		       ? std::optional(info)
		       : std::nullopt;
	}

	QString getMimetype(const QString& path)
	{
		const auto extension = ::Util::File::getFileExtension(path);
		if(extension.toLower() == "jpg")
		{
			return QStringLiteral("image/jpeg");
		}

		else if(extension.toLower() == "png")
		{
			return QStringLiteral("image/png");
		}

		spLog(Log::Warning, "Tagging") << "Unknown mimetype: '" << extension << "'";

		return QString();
	}

	bool writeCoverToTag(const Tagging::ParsedTag& parsedTag, const Models::Cover& coverModel)
	{
		const auto tagType = parsedTag.type;

		if(tagType == Tagging::TagType::ID3v2)
		{
			auto frame = ID3v2::CoverFrame(parsedTag.id3Tag());
			return writeCoverFrame(frame, coverModel);
		}

		else if(tagType == Tagging::TagType::MP4)
		{
			auto frame = MP4::CoverFrame(parsedTag.mp4Tag());
			return writeCoverFrame(frame, coverModel);
		}

		else if(tagType == Tagging::TagType::Xiph)
		{
			auto frame = Xiph::CoverFrame(parsedTag.xiphTag());
			return writeCoverFrame(frame, coverModel);
		}

		return false;
	}

	std::optional<Models::Cover> readCoverFromTag(const Tagging::ParsedTag& parsedTag)
	{
		if(parsedTag.type == Tagging::TagType::ID3v2)
		{
			return readCoverFrame(ID3v2::CoverFrame(parsedTag.id3Tag()));
		}

		else if(parsedTag.type == Tagging::TagType::Xiph)
		{
			return readCoverFrame(Xiph::CoverFrame(parsedTag.xiphTag()));
		}

		else if(parsedTag.type == Tagging::TagType::MP4)
		{
			return readCoverFrame(MP4::CoverFrame(parsedTag.mp4Tag()));
		}

		return std::nullopt;
	}

	std::optional<Models::Cover> createCoverModelFromImagePath(const QString& imagePath)
	{
		const auto mimeType = getMimetype(imagePath);
		if(mimeType.isEmpty())
		{
			return std::nullopt;
		}

		QByteArray data;
		const auto success = ::Util::File::readFileIntoByteArray(imagePath, data);
		if(data.isEmpty() || !success)
		{
			spLog(Log::Warning, "Tagging") << "No image data available: " << imagePath;
			return std::nullopt;
		}

		return Models::Cover(mimeType, data);
	}
}

bool Tagging::writeCover(const QString& filepath, const QPixmap& cover)
{
	const auto imageFilepath = Util::coverTempDirectory("tmp.png");
	if(!cover.save(imageFilepath))
	{
		spLog(Log::Warning, "Tagging") << "Can not save temporary cover: " << imageFilepath;
		spLog(Log::Warning, "Tagging") << "Is image valid? " << !cover.isNull();
		return false;
	}

	const auto success = writeCover(filepath, imageFilepath);
	QFile::remove(imageFilepath);

	return success;
}

bool Tagging::writeCover(const QString& filepath, const QString& coverImagePath)
{
	const auto optTagInfo = parsedTagFromPath(filepath);
	if(optTagInfo.has_value())
	{
		const auto coverModel = createCoverModelFromImagePath(coverImagePath);
		if(coverModel.has_value())
		{
			auto tagInfo = optTagInfo.value();
			return (writeCoverToTag(tagInfo.parsedTag, coverModel.value()) &&
			        tagInfo.fileRef.save());
		}
	}

	spLog(Log::Warning, "TaggingCover") << "Cannot save cover: " << filepath;
	return false;
}

QPixmap Tagging::extractCover(const QString& filepath)
{
	QByteArray data;
	QString mime;

	const auto success = extractCover(filepath, data, mime);
	return (success)
	       ? QPixmap::fromImage(QImage::fromData(data))
	       : QPixmap();
}

bool Tagging::extractCover(const ParsedTag& parsedTag, QByteArray& coverData, QString& mimeType)
{
	const auto cover = readCoverFromTag(parsedTag);
	if(cover.has_value())
	{
		coverData = cover.value().imageData;
		mimeType = cover.value().mimeType;
	}

	return !(coverData.isEmpty());
}

bool Tagging::extractCover(const QString& filepath, QByteArray& coverData, QString& mimeType)
{
	const auto tagInfo = parsedTagFromPath(filepath);
	return (tagInfo.has_value())
	       ? extractCover(tagInfo.value().parsedTag, coverData, mimeType)
	       : false;
}

bool Tagging::hasCover(const ParsedTag& parsedTag)
{
	switch(parsedTag.type)
	{
		case Tagging::TagType::ID3v2:
			return isCoverFrameAvailable(ID3v2::CoverFrame(parsedTag.id3Tag()));

		case Tagging::TagType::MP4:
			return isCoverFrameAvailable(MP4::CoverFrame(parsedTag.mp4Tag()));

		case Tagging::TagType::Xiph:
			return isCoverFrameAvailable(Xiph::CoverFrame(parsedTag.xiphTag()));

		default:
			return false;
	}
}

bool Tagging::hasCover(const QString& filepath)
{
	const auto tagInfo = parsedTagFromPath(filepath);
	return (tagInfo.has_value())
	       ? hasCover(tagInfo.value().parsedTag)
	       : false;
}

bool Tagging::isCoverSupported(const QString& filepath)
{
	const auto tagInfo = parsedTagFromPath(filepath);
	if(!tagInfo.has_value())
	{
		return false;
	}

	const auto parsedTag = tagInfo.value().parsedTag;
	const auto isSupported =
		(parsedTag.type == Tagging::TagType::ID3v2 || parsedTag.type == Tagging::TagType::MP4);

	return ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION <= 10))
	       ? isSupported
	       : isSupported || (parsedTag.type == Tagging::TagType::Xiph);
}

