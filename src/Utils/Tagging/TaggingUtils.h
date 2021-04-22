/* TaggingEnums.h */

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

#ifndef SAYONARA_TAGGING_ENUMS_H
#define SAYONARA_TAGGING_ENUMS_H

#include <taglib/audioproperties.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/tstring.h>
#include <taglib/fileref.h>

#include <QString>

namespace Tagging
{
	/**
	 * @brief The Quality enum
	 */
	enum class Quality :
		unsigned char
	{
			Fast = TagLib::AudioProperties::Fast,
			Standard = TagLib::AudioProperties::Average,
			Quality = TagLib::AudioProperties::Accurate,
			Dirty
	};

	enum class TagType :
		unsigned char
	{
			ID3v1 = 0,
			ID3v2,
			Xiph,
			MP4,
			Unsupported,
			Unknown
	};

	struct ParsedTag
	{
		TagLib::Tag* tag {nullptr};
		TagType type {TagType::Unknown};

		TagLib::MP4::Tag* mp4Tag() const;
		TagLib::ID3v2::Tag* id3Tag() const;
		TagLib::Ogg::XiphComment* xiphTag() const;
	};

	bool isValidFile(const TagLib::FileRef& fileRef);

	Tagging::TagType getTagType(const QString& filepath);
	QString tagTypeToString(Tagging::TagType);
	Tagging::ParsedTag getParsedTagFromFileRef(const TagLib::FileRef& fileRef);

	TagLib::String convertString(const QString& str);
	QString convertString(const TagLib::String& str);
}
#endif // SAYONARA_TAGGING_ENUMS_H

