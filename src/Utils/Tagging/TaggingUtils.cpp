/* ${CLASS_NAME}.h */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "TaggingUtils.h"

#include <QString>

#include <taglib/tstring.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>
#include <taglib/wavfile.h>

namespace
{
	Tagging::TagType getTagTypeFromTag(TagLib::Tag* tag)
	{
		if(dynamic_cast<TagLib::ID3v2::Tag*>(tag) != nullptr)
		{
			return Tagging::TagType::ID3v2;
		}

		else if(dynamic_cast<TagLib::ID3v1::Tag*>(tag) != nullptr)
		{
			return Tagging::TagType::ID3v1;
		}

		else if(dynamic_cast<TagLib::Ogg::XiphComment*>(tag) != nullptr)
		{
			return Tagging::TagType::Xiph;
		}

		else if(dynamic_cast<TagLib::MP4::Tag*>(tag) != nullptr)
		{
			return Tagging::TagType::MP4;
		}

		return Tagging::TagType::Unsupported;
	}

	Tagging::ParsedTag getParsedTagFromMpeg(TagLib::MPEG::File* file)
	{
		Tagging::ParsedTag parsedTag;

		if(file && file->hasID3v2Tag())
		{
			parsedTag.tag = file->ID3v2Tag();
			parsedTag.type = Tagging::TagType::ID3v2;
		}

		else if(file && file->hasID3v1Tag())
		{
			parsedTag.tag = file->ID3v1Tag();
			parsedTag.type = Tagging::TagType::ID3v1;
		}

		else if(file)
		{
			parsedTag.tag = file->ID3v2Tag(true);
			parsedTag.type = Tagging::TagType::ID3v2;
		}

		return parsedTag;
	}

	Tagging::ParsedTag getParsedTagFromFlac(TagLib::FLAC::File* file)
	{
		Tagging::ParsedTag parsedTag;

		if(file && file->hasXiphComment())
		{
			parsedTag.tag = file->xiphComment();
			parsedTag.type = Tagging::TagType::Xiph;
		}

		else if(file && file->hasID3v2Tag())
		{
			parsedTag.tag = file->ID3v2Tag();
			parsedTag.type = Tagging::TagType::ID3v2;
		}

		else if(file && file->hasID3v1Tag())
		{
			parsedTag.tag = file->ID3v1Tag();
			parsedTag.type = Tagging::TagType::ID3v1;
		}

		else if(file && file->tag())
		{
			parsedTag.tag = file->tag();
			parsedTag.type = Tagging::TagType::Unknown;
		}

		else if(file && !file->tag())
		{
			parsedTag.tag = file->ID3v2Tag(true);
			parsedTag.type = Tagging::TagType::ID3v2;
		}

		return parsedTag;
	}

	Tagging::ParsedTag getParsedTagFromMP4(TagLib::MP4::File* file)
	{
		Tagging::ParsedTag parsedTag;

		if(file && file->hasMP4Tag())
		{
			parsedTag.tag = file->tag();
			parsedTag.type = Tagging::TagType::MP4;
		}

		return parsedTag;
	}

	Tagging::ParsedTag getParsedTagFromWAV(TagLib::RIFF::WAV::File* file)
	{
		return {
			file->hasID3v2Tag()
			? dynamic_cast<TagLib::Tag*>(file->tag())
			: dynamic_cast<TagLib::Tag*>(file->InfoTag()),
			file->hasID3v2Tag()
			? Tagging::TagType::ID3v2
			: Tagging::TagType::Unknown
		};
	}
}

namespace Tagging
{
	TagLib::MP4::Tag* ParsedTag::mp4Tag() const
	{
		return dynamic_cast<TagLib::MP4::Tag*>(this->tag);
	}

	TagLib::ID3v2::Tag* ParsedTag::id3Tag() const
	{
		return dynamic_cast<TagLib::ID3v2::Tag*>(this->tag);
	}

	TagLib::Ogg::XiphComment* ParsedTag::xiphTag() const
	{
		return dynamic_cast<TagLib::Ogg::XiphComment*>(this->tag);
	}

	TagLib::String convertString(const QString& str)
	{
		return TagLib::String(str.toUtf8().data(), TagLib::String::Type::UTF8);
	}

	QString convertString(const TagLib::String& str)
	{
		return QString(str.toCString(true));
	}

	ParsedTag getParsedTagFromFileRef(const TagLib::FileRef& fileRef)
	{
		ParsedTag parsedTag;

		parsedTag.tag = nullptr;
		parsedTag.type = TagType::Unsupported;

		if(auto* mpg = dynamic_cast<TagLib::MPEG::File*>(fileRef.file()); mpg)
		{
			parsedTag = getParsedTagFromMpeg(mpg);
		}

		else if(auto* flac = dynamic_cast<TagLib::FLAC::File*>(fileRef.file()); flac)
		{
			parsedTag = getParsedTagFromFlac(flac);
		}

		else if(auto* mp4 = dynamic_cast<TagLib::MP4::File*>(fileRef.file()); mp4)
		{
			parsedTag = getParsedTagFromMP4(mp4);
		}

		else if(auto* wavFile = dynamic_cast<TagLib::RIFF::WAV::File*>(fileRef.file()); wavFile)
		{
			parsedTag = getParsedTagFromWAV(wavFile);
		}

		else if(fileRef.file())
		{
			parsedTag.tag = fileRef.tag();

			const auto tagType = getTagTypeFromTag(parsedTag.tag);
			parsedTag.type = (tagType == Tagging::TagType::Unsupported)
			                 ? Tagging::TagType::Unknown
			                 : tagType;
		}

		return parsedTag;
	}

	Tagging::TagType getTagType(const QString& filepath)
	{
		auto fileRef = TagLib::FileRef(TagLib::FileName(filepath.toUtf8()));
		return (isValidFile(fileRef))
		       ? getParsedTagFromFileRef(fileRef).type
		       : TagType::Unknown;
	}

	QString tagTypeToString(Tagging::TagType type)
	{
		switch(type)
		{
			case Tagging::TagType::ID3v1:
				return "ID3v1";
			case Tagging::TagType::ID3v2:
				return "ID3v2";
			case Tagging::TagType::Xiph:
				return "Xiph";
			case Tagging::TagType::MP4:
				return "MP4";
			case Tagging::TagType::Unknown:
				return "Unknown";
			default:
				return "Partially unsupported";
		}
	}

	bool isValidFile(const TagLib::FileRef& fileRef)
	{
		return (!fileRef.isNull() && fileRef.tag() && fileRef.file() && fileRef.file()->isValid());
	}
}