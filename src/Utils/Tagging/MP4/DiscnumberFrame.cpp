/* DiscnumberFrame.cpp */

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

#include "DiscnumberFrame.h"

#include <taglib/mp4item.h>

MP4::DiscnumberFrame::DiscnumberFrame(TagLib::MP4::Tag* tag) :
	MP4::MP4Frame<Models::Discnumber>(tag, "disk") {}

MP4::DiscnumberFrame::~DiscnumberFrame() = default;

std::optional<Models::Discnumber> MP4::DiscnumberFrame::mapItemToData(const TagLib::MP4::Item& item) const
{
	const auto intPair = item.toIntPair();
	const auto discnumber = Models::Discnumber(intPair.first, intPair.second);

	return std::optional(discnumber);
}

std::optional<TagLib::MP4::Item> MP4::DiscnumberFrame::mapDataToItem(const Models::Discnumber& discnumber)
{
	const auto item = TagLib::MP4::Item(discnumber.disc, discnumber.disccount);

	return std::optional(item);
}
