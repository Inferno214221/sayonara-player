/* Lyrics.cpp, (Created on 06.01.2024) */

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

#include "Lyrics.h"
#include "Utils/Tagging/TaggingUtils.h"

#include <taglib/tstringlist.h>

namespace
{
	constexpr const auto* Identifier = "\251lyr";
	constexpr const auto* PropertyMapKey = "LYRICS";
}

namespace MP4
{
	Lyrics::Lyrics(TagLib::MP4::Tag* tag) :
		MP4Frame<QString>(tag, Identifier) {}

	std::optional<QString> Lyrics::mapItemToData(const TagLib::MP4::Item& item) const
	{
		const auto stringList = item.toStringList();
		return (!stringList.isEmpty())
		       ? std::optional {Tagging::convertString(stringList[0])}
		       : std::nullopt;
	}

	std::optional<TagLib::MP4::Item> Lyrics::mapDataToItem(const QString& model)
	{
		return TagLib::StringList {Tagging::convertString(model)};
	}

	std::optional<QString> Lyrics::findDataInPropertyMap() const
	{
		const auto propertyMap = tag()->properties();
		if(propertyMap.contains(PropertyMapKey))
		{
			const auto strList = propertyMap[PropertyMapKey];
			if(!strList.isEmpty())
			{
				return Tagging::convertString(strList[0]);
			}
		}

		return std::nullopt;
	}

	QByteArray Lyrics::propertyMapKey() const { return PropertyMapKey; }
}