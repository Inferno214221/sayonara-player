/* LyricsFrame.cpp */

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

#include "LyricsFrame.h"

Xiph::LyricsFrame::LyricsFrame(TagLib::Ogg::XiphComment* tag) :
	Xiph::XiphFrame<QString>(tag, "LYRICS") {}

std::optional<QString> Xiph::LyricsFrame::mapTagToData() const
{
	const auto lyricData = stringData();

	return (lyricData.has_value())
	       ? std::optional(Tagging::convertString(lyricData.value()))
	       : std::nullopt;
}

void Xiph::LyricsFrame::mapDataToTag(const QString& lyrics)
{
	setStringData(lyrics);
}

Xiph::LyricsFrame::~LyricsFrame() = default;
