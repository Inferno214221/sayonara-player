/* TaggingCover.cpp */

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

#include <QByteArray>
#include <QString>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>

#include "taglib/fileref.h"
#include "taglib/flacpicture.h"
#include "taglib/tlist.h"

using Tagging::ParsedTag;

bool Tagging::Covers::write_cover(const QString& filepath, const QPixmap& cover)
{
	QString tmp_filepath = ::Util::sayonara_path("tmp.png");

	bool success = cover.save(tmp_filepath);
	if(!success){
		sp_log(Log::Warning, "Tagging") << "Can not save temporary cover: " << tmp_filepath;
		sp_log(Log::Warning, "Tagging") << "Is image valid? " << !cover.isNull();
		return false;
	}

	success = write_cover(filepath, tmp_filepath);
	QFile::remove(tmp_filepath);

	return success;
}


bool Tagging::Covers::write_cover(const QString& filepath, const QString& cover_image_path)
{
	QString error_msg = "Cannot save cover. ";

	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::is_valid_file(f)){
		sp_log(Log::Warning, "Tagging") << "Cannot write cover for " << filepath;
		return false;
	}

	QByteArray data;
	bool success = ::Util::File::read_file_into_byte_arr(cover_image_path, data);
	if(data.isEmpty() || !success){
		sp_log(Log::Warning, "Tagging") << error_msg << "No image data available: " << cover_image_path;
		return false;
	}

	QString mime_type = "image/";
	QString ext = ::Util::File::get_file_extension(cover_image_path);
	if(ext.compare("jpg", Qt::CaseInsensitive) == 0){
		mime_type += "jpeg";
	}

	else if(ext.compare("png", Qt::CaseInsensitive) == 0){
		mime_type += "png";
	}

	else{
		sp_log(Log::Warning, "Tagging") << error_msg << "Unknown mimetype: '" << ext << "'";
		return false;
	}

	Models::Cover cover(mime_type, data);
	Tagging::ParsedTag parsed_tag = Tagging::Utils::tag_type_from_fileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	if(tag_type == Tagging::TagType::ID3v2)
	{
		auto* id3v2 = parsed_tag.id3_tag();

		ID3v2::CoverFrame cover_frame(id3v2);
		if(!cover_frame.write(cover))
		{
			sp_log(Log::Warning, "Tagging") << "ID3v2 Cannot write cover";
			return false;
		}
	}

	else if(tag_type == Tagging::TagType::MP4)
	{
		auto* mp4 = parsed_tag.mp4_tag();

		MP4::CoverFrame cover_frame(mp4);
		if(!cover_frame.write(cover))
		{
			sp_log(Log::Warning, "Tagging") << "MP4 Cannot write cover";
			return false;
		}
	}

	else if(tag_type == Tagging::TagType::Xiph)
	{

#ifdef WITH_SYSTEM_TAGLIB
		if(TAGLIB_MINOR_VERSION == 11 && TAGLIB_PATCH_VERSION == 1)
		{
			sp_log(Log::Warning, "TaggingCover") << "Not writing cover due to taglib bug";
			return false;
		}
#endif

		auto* xiph = parsed_tag.xiph_tag();
		Xiph::CoverFrame cover_frame(xiph);
		if(!cover_frame.write(cover))
		{
			sp_log(Log::Warning, "Tagging") << "Xiph Cannot write cover";
			return false;
		}
	}

	return f.save();
}

QPixmap Tagging::Covers::extract_cover(const QString& filepath)
{
	QByteArray data;
	QString mime;

	bool success = extract_cover(filepath, data, mime);
	if(!success){
		return QPixmap();
	}

	return QPixmap::fromImage(QImage::fromData(data));
}


bool Tagging::Covers::extract_cover(const ParsedTag& parsed_tag, QByteArray& cover_data, QString& mime_type)
{
	Models::Cover cover;
	Tagging::TagType tag_type = parsed_tag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
		{
			auto* id3v2 = parsed_tag.id3_tag();
			ID3v2::CoverFrame cover_frame(id3v2);

			if(!cover_frame.is_frame_found()){
				return false;
			}

			cover_frame.read(cover);
		}

		break;

		case Tagging::TagType::Xiph:
		{
			auto* xiph = parsed_tag.xiph_tag();
			Xiph::CoverFrame cover_frame(xiph);
			if(!cover_frame.read(cover)){
				return false;
			}
		}

		break;

		case Tagging::TagType::MP4:
		{
			auto* mp4 = parsed_tag.mp4_tag();
			MP4::CoverFrame cover_frame(mp4);
			if(!cover_frame.read(cover)){
				return false;
			}
		}

		break;

		default:
			return false;
	}

	cover_data = cover.image_data;
	mime_type = cover.mime_type;

	return !(cover_data.isEmpty());
}


bool Tagging::Covers::extract_cover(const QString& filepath, QByteArray& cover_data, QString& mime_type)
{
	TagLib::FileRef fileref(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::is_valid_file(fileref))
	{
		sp_log(Log::Warning, "Tagging") << "Cannot extract cover for " << filepath;
		return false;
	}

	Tagging::ParsedTag parsed_tag = Tagging::Utils::tag_type_from_fileref(fileref);

	return extract_cover(parsed_tag, cover_data, mime_type);

}

bool Tagging::Covers::has_cover(const ParsedTag& parsed_tag)
{
	Tagging::TagType tag_type = parsed_tag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
		{
			auto id3v2 = parsed_tag.id3_tag();
			ID3v2::CoverFrame cover_frame(id3v2);
			return cover_frame.is_frame_found();
		}

		case Tagging::TagType::MP4:
		{
			auto mp4 = parsed_tag.mp4_tag();
			MP4::CoverFrame cover_frame(mp4);
			return cover_frame.is_frame_found();
		}

		case Tagging::TagType::Xiph:
		{
			auto xiph = parsed_tag.xiph_tag();
			Xiph::CoverFrame cover_frame(xiph);
			return cover_frame.is_frame_found();
		}

		default:
			return false;
	}
}

bool Tagging::Covers::has_cover(const QString& filepath)
{
	QFileInfo fi(filepath);
	if(fi.size() <= 0){
		return false;
	}

	TagLib::FileRef fileref(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::is_valid_file(fileref)){
		sp_log(Log::Warning, "Tagging") << "Cannot determine cover for " << filepath;
		return false;
	}

	Tagging::ParsedTag parsed_tag = Tagging::Utils::tag_type_from_fileref(fileref);

	return has_cover(parsed_tag);
}


bool Tagging::Covers::is_cover_supported(const QString& filepath)
{
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::is_valid_file(f)){
		return false;
	}

	Tagging::ParsedTag parsed_tag = Tagging::Utils::tag_type_from_fileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	bool supported = (tag_type == Tagging::TagType::ID3v2 || tag_type == Tagging::TagType::MP4);

#ifdef SAYONARA_INTERNAL_TAGLIB
	return supported || (tag_type == Tagging::TagType::Xiph);
#else
	if(	((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION == 11) && (TAGLIB_PATCH_VERSION == 1)) ||
		((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION < 10)) )
	{
		return supported;
	}

	else {
		return supported || (tag_type == Tagging::TagType::Xiph);
	}
#endif
}

