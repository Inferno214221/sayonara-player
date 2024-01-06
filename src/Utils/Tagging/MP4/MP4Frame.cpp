/* MP4Frame.cpp, (Created on 06.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#include "MP4Frame.h"
#include "Utils/Tagging/TaggingUtils.h"

#include <QByteArray>

#include <taglib/tpropertymap.h>
#include <taglib/mp4item.h>

namespace MP4
{
	TagLib::MP4::ItemMap::ConstIterator findKey(const QByteArray& key, const TagLib::MP4::ItemMap& itemMap)
	{
		return std::find_if(itemMap.begin(), itemMap.end(), [&](const auto& itemList) {
			const auto convertedString = Tagging::convertString(itemList.first);
			return (convertedString.toLower() == key.toLower());
		});
	}

	void eraseAllFromTag(TagLib::MP4::Tag* tag, const QString& key)
	{
		const auto taglibKey = Tagging::convertString(key);
		while(tag->contains(taglibKey))
		{
			tag->removeItem(taglibKey);
		}
	}

	bool isFrameAvailable(TagLib::MP4::Tag* tag, const QByteArray& key, const QByteArray& propertyMapKey)
	{
		if(!tag)
		{
			return false;
		}

		const auto& itemMap = tag->itemMap();
		if(findKey(key, itemMap) != itemMap.end())
		{
			return true;
		}

		const auto propertyMap = tag->properties();
		return propertyMap.contains(propertyMapKey.constData());
	}
}
