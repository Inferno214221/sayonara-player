/* MP4Frame.h */

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

#ifndef SAYONARA_ABSTRACT_MP4_FRAME_H
#define SAYONARA_ABSTRACT_MP4_FRAME_H

#include "Utils/Tagging/AbstractFrame.h"
#include "Utils/Algorithm.h"

#include <QString>

#include <taglib/tag.h>
#include <taglib/tstringlist.h>
#include <taglib/mp4tag.h>

#include <algorithm>
#include <optional>

namespace MP4
{
	template<typename Model_t>
	class MP4Frame :
		protected Tagging::AbstractFrame<TagLib::MP4::Tag>
	{
		protected:
			TagLib::MP4::ItemListMap::ConstIterator findKey(const TagLib::MP4::ItemListMap& itemListMap) const
			{
				return std::find_if(itemListMap.begin(), itemListMap.end(), [&](const auto& itemList) {
					const auto convertedString = Tagging::convertString(itemList.first);
					return (convertedString.compare(key(), Qt::CaseInsensitive) == 0);
				});
			}

			void eraseAllFromItemListMap(TagLib::MP4::ItemListMap& itemListMap, const QString& key)
			{
				auto itBegin = itemListMap.begin();
				for(auto it = itemListMap.begin(); it != itemListMap.end(); it++)
				{
					if(Tagging::convertString(it->first).compare(key, Qt::CaseInsensitive) == 0)
					{
						itemListMap.erase(it);
						it = itBegin;
					}

					else
					{
						itBegin = it;
					}
				}
			}

			virtual std::optional<Model_t> mapItemToData(const TagLib::MP4::Item& item) const = 0;
			virtual std::optional<TagLib::MP4::Item> mapDataToItem(const Model_t& model) = 0;

		public:
			MP4Frame(TagLib::MP4::Tag* tag, const QString& identifier) :
				Tagging::AbstractFrame<TagLib::MP4::Tag>(tag, identifier) {}

			virtual ~MP4Frame() = default;

			bool read(Model_t& data) const
			{
				if(!tag())
				{
					return false;
				}

				const auto& itemListMap = tag()->itemListMap();
				const auto it = findKey(itemListMap);
				if(it != itemListMap.end() && it->second.isValid())
				{
					const auto optionalData = mapItemToData(it->second);
					if(optionalData.has_value())
					{
						data = optionalData.value();
						return true;
					}
				}

				return false;
			}

			bool write(const Model_t& data)
			{
				if(!tag())
				{
					return false;
				}

				auto& itemListMap = tag()->itemListMap();
				eraseAllFromItemListMap(itemListMap, key());

				const auto item = mapDataToItem(data);
				if(item.has_value() && item.value().isValid())
				{
					tag()->itemListMap().insert(tagKey(), *item);
				}

				return item.has_value();
			}

			bool isFrameAvailable() const
			{
				if(!tag())
				{
					return false;
				}

				const auto& itemListMap = tag()->itemListMap();
				return (findKey(itemListMap) != itemListMap.end());
			}
	};
}

#endif // SAYONARA_ABSTRACT_MP4_FRAME_H
