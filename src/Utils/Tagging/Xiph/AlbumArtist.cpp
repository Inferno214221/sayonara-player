/* AlbumArtist.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include <optional>

Xiph::AlbumArtistFrame::AlbumArtistFrame(TagLib::Ogg::XiphComment* tag) :
	Xiph::XiphFrame<QString>(tag, "ALBUMARTIST") {}

Xiph::AlbumArtistFrame::~AlbumArtistFrame() = default;

std::optional<QString> Xiph::AlbumArtistFrame::mapTagToData() const
{
	const auto albumArtistData = stringData();

	return (albumArtistData.has_value())
	       ? std::optional(Tagging::convertString(albumArtistData.value()))
	       : std::nullopt;
}

void Xiph::AlbumArtistFrame::mapDataToTag(const QString& albumArtist)
{
	setStringData(albumArtist);
}