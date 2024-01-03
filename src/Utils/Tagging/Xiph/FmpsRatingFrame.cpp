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

#include "FmpsRatingFrame.h"

#include <QStringList>

#include <algorithm>
#include <array>
#include <optional>
#include <utility>

namespace
{
	std::optional<float> getFloat(const QString& str)
	{
		auto ok = false;
		const auto f = str.toFloat(&ok);
		return ok ?
		       std::optional(f) :
		       std::nullopt;
	}

	std::optional<Rating> ratingFromString(const QString& data)
	{
		const auto fRating = getFloat(data);
		if(!fRating || (fRating.value() < 0) || (fRating.value() > 1.0f))
		{
			return std::nullopt;
		}

		constexpr const auto mapping = std::array {
			std::make_pair(1.0f / 6.0f, Rating::Zero),
			std::make_pair(2.0f / 6.0f, Rating::One),
			std::make_pair(3.0f / 6.0f, Rating::Two),
			std::make_pair(4.0f / 6.0f, Rating::Three),
			std::make_pair(5.0f / 6.0f, Rating::Four),
			std::make_pair(1.0f, Rating::Five)
		};

		const auto it = std::find_if(mapping.begin(), mapping.end(), [&](const auto& mappingPair) {
			return (fRating.value() < mappingPair.first);
		});

		return (it != mapping.end())
		       ? std::optional(it->second)
		       : std::nullopt;
	}

	std::optional<Models::Popularimeter> popularimeterFromQString(const QString& data)
	{
		// “Alice Abba::1.0;;Bob Beatles::133.0”.
		const auto entries = data.split(";;");
		QList<Models::Popularimeter> popularimeters;
		for(const auto& entry: entries)
		{
			const auto splitted = entry.split("::");
			const auto username = splitted.first();
			const auto rating = ratingFromString(splitted.last());
			if(rating.has_value())
			{
				if(entry.toLower().contains("sayonara"))
				{
					return Models::Popularimeter(username, rating.value(), 0);
				}

				else
				{
					popularimeters.push_back({username, rating.value(), 0});
				}
			}
		}

		return (popularimeters.isEmpty())
		       ? std::nullopt
		       : std::optional {popularimeters[0]};
	}
}

Xiph::FmpsRatingFrame::FmpsRatingFrame(TagLib::Ogg::XiphComment* tag) :
	XiphFrame<Models::Popularimeter>(tag, "FMPS_RATING") {}

Xiph::FmpsRatingFrame::~FmpsRatingFrame() = default;

std::optional<Models::Popularimeter> Xiph::FmpsRatingFrame::mapTagToData() const
{
	const auto popData = this->stringData();

	return (popData.has_value())
	       ? popularimeterFromQString(Tagging::convertString(popData.value()))
	       : std::optional<Models::Popularimeter>();
}

void Xiph::FmpsRatingFrame::mapDataToTag(const Models::Popularimeter& pop)
{
	const auto fRating = static_cast<int>(pop.rating) * 0.2f;
	setStringData(QString::number(fRating));
}

/* FMPS User rating */

Xiph::FmpsUserRatingFrame::FmpsUserRatingFrame(TagLib::Ogg::XiphComment* tag) :
	XiphFrame<Models::Popularimeter>(tag, "FMPS_RATING_USER") {}

Xiph::FmpsUserRatingFrame::~FmpsUserRatingFrame() = default;

std::optional<Models::Popularimeter> Xiph::FmpsUserRatingFrame::mapTagToData() const
{
	const auto popData = this->stringData();

	return (popData.has_value())
	       ? popularimeterFromQString(Tagging::convertString(popData.value()))
	       : std::optional<Models::Popularimeter>();
}

void Xiph::FmpsUserRatingFrame::mapDataToTag(const Models::Popularimeter& /* pop */)
{
	// sayonara does not set user rating (there needs to be an option where you can enter your username)
}
