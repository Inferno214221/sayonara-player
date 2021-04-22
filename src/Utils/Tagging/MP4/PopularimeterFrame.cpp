/* PopularimeterFrame.cpp */

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

#include "PopularimeterFrame.h"

MP4::PopularimeterFrame::PopularimeterFrame(TagLib::MP4::Tag* tag) :
	MP4::MP4Frame<Models::Popularimeter>(tag, "rtng") {}

MP4::PopularimeterFrame::~PopularimeterFrame() = default;

std::optional<Models::Popularimeter> MP4::PopularimeterFrame::mapItemToData(const TagLib::MP4::Item& item) const
{
	auto popularimeter = Models::Popularimeter {};
	const auto ratingByte = item.toByte();
	if(ratingByte <= 5)
	{
		popularimeter.rating = static_cast<Rating>(ratingByte);
	}

	else
	{
		popularimeter.setRatingByte(ratingByte);
	}

	return std::optional(popularimeter);
}

std::optional<TagLib::MP4::Item> MP4::PopularimeterFrame::mapDataToItem(const Models::Popularimeter& popularimeter)
{
	const auto byte = static_cast<uchar>(popularimeter.ratingByte());
	const auto item = TagLib::MP4::Item(byte);

	return std::optional(item);
}
