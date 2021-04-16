/* TaggingCover.cpp */

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

#include "TaggingCover.h"
#include "TaggingEnums.h"
#include "Tagging.h"

#include "Models/Cover.h"
#include "ID3v2/Cover.h"
#include "MP4/Cover.h"
#include "Xiph/Cover.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/CoverUtils.h"
#include "Utils/StandardPaths.h"

#include <QByteArray>
#include <QString>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>

#include <taglib/fileref.h>
#include <taglib/flacpicture.h>
#include <taglib/tlist.h>

using Tagging::ParsedTag;

bool Tagging::Covers::writeCover(const QString& filepath, const QPixmap& cover)
{
	const auto tmpFilepath = Util::coverTempDirectory("tmp.png");

	auto success = cover.save(tmpFilepath);
	if(!success){
		spLog(Log::Warning, "Tagging") << "Can not save temporary cover: " << tmpFilepath;
		spLog(Log::Warning, "Tagging") << "Is image valid? " << !cover.isNull();
		return false;
	}

	success = writeCover(filepath, tmpFilepath);
	QFile::remove(tmpFilepath);

	return success;
}


bool Tagging::Covers::writeCover(const QString& filepath, const QString& coverImagePath)
{
	QString errorMessage = "Cannot save cover. ";

	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::isValidFile(f)){
		spLog(Log::Warning, "Tagging") << "Cannot write cover for " << filepath;
		return false;
	}

	QByteArray data;
	bool success = ::Util::File::readFileIntoByteArray(coverImagePath, data);
	if(data.isEmpty() || !success){
		spLog(Log::Warning, "Tagging") << errorMessage << "No image data available: " << coverImagePath;
		return false;
	}

	QString mimeType = "image/";
	QString ext = ::Util::File::getFileExtension(coverImagePath);
	if(ext.compare("jpg", Qt::CaseInsensitive) == 0){
		mimeType += "jpeg";
	}

	else if(ext.compare("png", Qt::CaseInsensitive) == 0){
		mimeType += "png";
	}

	else{
		spLog(Log::Warning, "Tagging") << errorMessage << "Unknown mimetype: '" << ext << "'";
		return false;
	}

	Models::Cover cover(mimeType, data);
	Tagging::ParsedTag parsedTag = Tagging::Utils::getTagTypeFromFileref(f);
	Tagging::TagType tagType = parsedTag.type;

	if(tagType == Tagging::TagType::ID3v2)
	{
		auto* id3v2 = parsedTag.id3Tag();

		ID3v2::CoverFrame coverFrame(id3v2);
		if(!coverFrame.write(cover))
		{
			spLog(Log::Warning, "Tagging") << "ID3v2 Cannot write cover";
			return false;
		}
	}

	else if(tagType == Tagging::TagType::MP4)
	{
		auto* mp4 = parsedTag.mp4Tag();

		MP4::CoverFrame coverFrame(mp4);
		if(!coverFrame.write(cover))
		{
			spLog(Log::Warning, "Tagging") << "MP4 Cannot write cover";
			return false;
		}
	}

	else if(tagType == Tagging::TagType::Xiph)
	{

#ifdef WITH_SYSTEM_TAGLIB
		if(TAGLIB_MINOR_VERSION == 11 && TAGLIB_PATCH_VERSION == 1)
		{
			sp_log(Log::Warning, "TaggingCover") << "Not writing cover due to taglib bug";
			return false;
		}
#endif

		auto* xiph = parsedTag.xiphTag();
		Xiph::CoverFrame coverFrame(xiph);
		if(!coverFrame.write(cover))
		{
			spLog(Log::Warning, "Tagging") << "Xiph Cannot write cover";
			return false;
		}
	}

	return f.save();
}

QPixmap Tagging::Covers::extractCover(const QString& filepath)
{
	QByteArray data;
	QString mime;

	bool success = extractCover(filepath, data, mime);
	if(!success){
		return QPixmap();
	}

	return QPixmap::fromImage(QImage::fromData(data));
}


bool Tagging::Covers::extractCover(const ParsedTag& parsedTag, QByteArray& coverData, QString& mimeType)
{
	Models::Cover cover;
	Tagging::TagType tag_type = parsedTag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
		{
			auto* id3v2 = parsedTag.id3Tag();
			ID3v2::CoverFrame coverFrame(id3v2);

			if(!coverFrame.is_frame_found()){
				return false;
			}

			coverFrame.read(cover);
		}

		break;

		case Tagging::TagType::Xiph:
		{
			auto* xiph = parsedTag.xiphTag();
			Xiph::CoverFrame coverFrame(xiph);
			if(!coverFrame.read(cover)){
				return false;
			}
		}

		break;

		case Tagging::TagType::MP4:
		{
			auto* mp4 = parsedTag.mp4Tag();
			MP4::CoverFrame coverFrame(mp4);
			if(!coverFrame.read(cover)){
				return false;
			}
		}

		break;

		default:
			return false;
	}

	coverData = cover.imageData;
	mimeType = cover.mimeType;

	return !(coverData.isEmpty());
}


bool Tagging::Covers::extractCover(const QString& filepath, QByteArray& coverData, QString& mimeType)
{
	TagLib::FileRef fileref(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::isValidFile(fileref))
	{
		spLog(Log::Warning, "Tagging") << "Cannot extract cover for " << filepath;
		return false;
	}

	Tagging::ParsedTag parsedTag = Tagging::Utils::getTagTypeFromFileref(fileref);

	return extractCover(parsedTag, coverData, mimeType);

}

bool Tagging::Covers::hasCover(const ParsedTag& parsedTag)
{
	Tagging::TagType tag_type = parsedTag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
		{
			auto id3v2 = parsedTag.id3Tag();
			ID3v2::CoverFrame coverFrame(id3v2);
			return coverFrame.is_frame_found();
		}

		case Tagging::TagType::MP4:
		{
			auto mp4 = parsedTag.mp4Tag();
			MP4::CoverFrame coverFrame(mp4);
			return coverFrame.is_frame_found();
		}

		case Tagging::TagType::Xiph:
		{
			auto xiph = parsedTag.xiphTag();
			Xiph::CoverFrame coverFrame(xiph);
			return coverFrame.is_frame_found();
		}

		default:
			return false;
	}
}

bool Tagging::Covers::hasCover(const QString& filepath)
{
	QFileInfo fi(filepath);
	if(fi.size() <= 0){
		return false;
	}

	TagLib::FileRef fileref(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::isValidFile(fileref)){
		spLog(Log::Warning, "Tagging") << "Cannot determine cover for " << filepath;
		return false;
	}

	Tagging::ParsedTag parsedTag = Tagging::Utils::getTagTypeFromFileref(fileref);

	return hasCover(parsedTag);
}


bool Tagging::Covers::isCoverSupported(const QString& filepath)
{
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::isValidFile(f)){
		return false;
	}

	Tagging::ParsedTag parsedTag = Tagging::Utils::getTagTypeFromFileref(f);
	Tagging::TagType tag_type = parsedTag.type;

	bool supported = (tag_type == Tagging::TagType::ID3v2 || tag_type == Tagging::TagType::MP4);

	if(	((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION == 11) && (TAGLIB_PATCH_VERSION == 1)) ||
		((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION < 10)) )
	{
		return supported;
	}

	else {
		return supported || (tag_type == Tagging::TagType::Xiph);
	}
}

