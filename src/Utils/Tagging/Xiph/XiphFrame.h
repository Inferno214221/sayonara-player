/* XiphFrame.h */

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

#ifndef SAYONARA_ABSTRACT_XIPH_FRAME_H
#define SAYONARA_ABSTRACT_XIPH_FRAME_H

#include "Utils/Tagging/AbstractFrame.h"

#include <QString>

#include <taglib/tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/tstring.h>
#include <taglib/tstringlist.h>

#include <optional>

namespace Xiph
{
	template<typename Model_t>
	class XiphFrame :
		protected Tagging::AbstractFrame<TagLib::Ogg::XiphComment>
	{
		protected:
			virtual std::optional<Model_t> mapTagToData() const = 0;
			virtual void mapDataToTag(const Model_t& model) = 0;

			std::optional<TagLib::String> stringData() const
			{
				const auto& map = tag()->fieldListMap();
				const auto it = map.find(tagKey());
				return (it == map.end())
					? std::optional<TagLib::String>{}
					: std::optional(it->second.front());
			}

			void setStringData(const TagLib::String& value)
			{
				if(tag())
				{
					tag()->addField(tagKey(), value, true);
				}
			}

			void setStringData(const QString& value)
			{
				setStringData(Tagging::convertString(value));
			}

		public:
			XiphFrame(TagLib::Ogg::XiphComment* tag, const QString& identifier) :
				Tagging::AbstractFrame<TagLib::Ogg::XiphComment>(tag, identifier) {}

			virtual ~XiphFrame() = default;

			bool read(Model_t& model) const
			{
				const auto data = (tag() != nullptr)
					? mapTagToData()
					: std::nullopt;

				if(data.has_value())
				{
					model = data.value();
				}

				return data.has_value();
			}

			bool write(const Model_t& model)
			{
				if(!tag())
				{
					return false;
				}

				if(!tagKey().isEmpty())
				{
					tag()->removeFields(tagKey());
				}

				try {
					mapDataToTag(model);
				} catch (std::exception& /* e */) {
					return false;
				}

				return true;
			}

			virtual bool isFrameAvailable() const
			{
				return (!tagKey().isEmpty() && tag()->contains("METADATA_BLOCK_PICTURE"));
			}
	};
}

#endif // SAYONARA_ABSTRACT_XIPH_FRAME_H
