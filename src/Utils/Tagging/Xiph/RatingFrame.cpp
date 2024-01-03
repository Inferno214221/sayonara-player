/* PopularimeterFrame.cpp */

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

#include "RatingFrame.h"

#include <optional>

namespace
{
	std::optional<Models::Popularimeter> popularimeterFromQString(const QString& data)
	{
		const auto iRating = data.toInt();
		auto ratingByte = static_cast<Byte>(iRating);

		auto pop = Models::Popularimeter();
		if(ratingByte <= 5)
		{
			pop.rating = static_cast<Rating>(ratingByte);
		}

		else
		{
			pop.setRatingByte(ratingByte);
		}

		return std::optional(pop);
	}
}

Xiph::RatingFrame::RatingFrame(TagLib::Ogg::XiphComment* tag) :
	XiphFrame<Models::Popularimeter>(tag, "RATING") {}

Xiph::RatingFrame::~RatingFrame() = default;

std::optional<Models::Popularimeter> Xiph::RatingFrame::mapTagToData() const
{
	const auto popData = this->stringData();

	return (popData.has_value())
	       ? popularimeterFromQString(Tagging::convertString(popData.value()))
	       : std::optional<Models::Popularimeter>();
}

void Xiph::RatingFrame::mapDataToTag(const Models::Popularimeter& pop)
{
	const auto iRating = static_cast<int>(pop.rating);
	setStringData(QString::number(iRating));
}
