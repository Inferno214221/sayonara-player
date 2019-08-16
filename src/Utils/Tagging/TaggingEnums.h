/* TaggingEnums.h */

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



#ifndef TAGGINGENUMS
#define TAGGINGENUMS

#include "taglib/audioproperties.h"
#include "taglib/id3v1tag.h"
#include "taglib/id3v2tag.h"
#include "taglib/mp4tag.h"
#include "taglib/xiphcomment.h"

namespace Tagging
{
		/**
		 * @brief The Quality enum
		 */
		enum class Quality : unsigned char
		{
			Fast=TagLib::AudioProperties::Fast,
			Standard=TagLib::AudioProperties::Average,
			Quality=TagLib::AudioProperties::Accurate,
			Dirty
		};

		enum class TagType : unsigned char
		{
			ID3v1=0,
			ID3v2,
			Xiph,
			MP4,
			Unsupported,
			Unknown
		};

		struct ParsedTag
		{
			TagLib::Tag* tag;
			TagType type;

			TagLib::MP4::Tag* mp4_tag() const
			{
				return dynamic_cast<TagLib::MP4::Tag*>(this->tag);
			}

			TagLib::ID3v2::Tag* id3_tag() const
			{
				return dynamic_cast<TagLib::ID3v2::Tag*>(this->tag);
			}

			TagLib::Ogg::XiphComment* xiph_tag() const
			{
				return dynamic_cast<TagLib::Ogg::XiphComment*>(this->tag);
			}
		};
}
#endif // TAGGINGENUMS

