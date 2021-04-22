/* AlbumArtist.cpp */

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

#include "AlbumArtist.h"

#include <QString>

MP4::AlbumArtistFrame::AlbumArtistFrame(TagLib::MP4::Tag* tag) :
	MP4::MP4Frame<QString>(tag, "aART") {}

MP4::AlbumArtistFrame::~AlbumArtistFrame() = default;

std::optional<QString> MP4::AlbumArtistFrame::mapItemToData(const TagLib::MP4::Item& item) const
{
	const auto items = item.toStringList();
	return (item.isValid() && !items.isEmpty())
	       ? std::optional(Tagging::convertString(*items.begin()))
	       : std::nullopt;
}

std::optional<TagLib::MP4::Item> MP4::AlbumArtistFrame::mapDataToItem(const QString& data)
{
	TagLib::StringList strings;
	strings.append(Tagging::convertString(data));

	return std::optional<TagLib::MP4::Item>(strings);
}
