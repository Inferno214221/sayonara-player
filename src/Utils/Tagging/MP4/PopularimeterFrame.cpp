/* RtngFrame.cpp */

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

#include "PopularimeterFrame.h"
#include <QString>
#include "Tagging/TaggingUtils.h"

#include <optional>

namespace
{
	[[nodiscard]] int fourBytesToInteger(const int32_t fourBytes)
	{
		const auto chars = std::array {
			(fourBytes >> 16) & 0xFF,
			(fourBytes >> 8) & 0xFF,
			(fourBytes & 0xFF)
		};

		auto str = QString {};
		if(chars[0] != 0)
		{
			str.push_back(chars[0]);
		}
		str.push_back(chars[1]);
		str.push_back(chars[2]);

		return str.toInt();
	}

	[[nodiscard]] Rating byteToRating(const uchar byte)
	{
		if(byte == 0)
		{
			return Rating::Zero;
		}
		if(byte <= 20)
		{
			return Rating::One;
		}

		if(byte <= 40)
		{
			return Rating::Two;
		}

		if(byte <= 60)
		{
			return Rating::Three;
		}

		if(byte <= 80)
		{
			return Rating::Four;
		}

		return Rating::Five;
	}

	[[nodiscard]] int ratingToInt(const Rating rating)
	{
		switch(rating)
		{
			case Rating::Zero:
				return 0;
			case Rating::One:
				return 20;
			case Rating::Two:
				return 40;
			case Rating::Three:
				return 60;
			case Rating::Four:
				return 80;
			case Rating::Five:
				return 100;
			case Rating::Last:
			default:
				return 0;
		}
	}

	std::optional<Models::Popularimeter> parseRatingInt(const int i)
	{
		auto popularimeter = Models::Popularimeter {};
		popularimeter.rating = byteToRating(static_cast<uchar>(i));

		return {popularimeter};
	}

	std::optional<Models::Popularimeter> parseRatingString(const TagLib::String& str)
	{
		auto ok = false;
		const auto i = str.toInt(&ok);
		return ok
		       ? parseRatingInt(i)
		       : std::nullopt;
	}

	std::optional<Models::Popularimeter> mapItemToData(const TagLib::MP4::Item& item)
	{
		if(const auto stringList = item.toStringList(); !stringList.isEmpty())
		{
			const auto popularimeter = parseRatingString(stringList[0]);
			if(popularimeter.has_value())
			{
				return popularimeter;
			}
		}

		const auto value = item.toInt();
		return (value > 100)
		       ? parseRatingInt(fourBytesToInteger(value))
		       : parseRatingInt(value);
	}

	std::optional<TagLib::MP4::Item> mapDataToItem(const Models::Popularimeter& popularimeter)
	{
		const auto value = ratingToInt(popularimeter.rating);
		const auto str = Tagging::convertString(QString::number(value));
		const auto item = TagLib::MP4::Item(str);

		return {item};
	}
}

namespace MP4
{
	RtngFrame::RtngFrame(TagLib::MP4::Tag* tag) :
		MP4::MP4Frame<Models::Popularimeter>(tag, "rtng") {}

	RtngFrame::~RtngFrame() = default;

	std::optional<Models::Popularimeter> RtngFrame::mapItemToData(const TagLib::MP4::Item& item) const
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

		return {popularimeter};
	}

	std::optional<TagLib::MP4::Item> RtngFrame::mapDataToItem(const Models::Popularimeter& /*popularimeter*/)
	{
		return std::nullopt;
	}

	ITunesRatingFrame::ITunesRatingFrame(TagLib::MP4::Tag* tag) :
		MP4::MP4Frame<Models::Popularimeter>(tag, "----:com.apple.iTunes:RATING") {}

	ITunesRatingFrame::~ITunesRatingFrame() = default;

	std::optional<Models::Popularimeter> ITunesRatingFrame::mapItemToData(const TagLib::MP4::Item& item) const
	{
		return ::mapItemToData(item);
	}

	std::optional<TagLib::MP4::Item> ITunesRatingFrame::mapDataToItem(const Models::Popularimeter& popularimeter)
	{
		return ::mapDataToItem(popularimeter);
	}

	MediaMonkeyRateFrame::MediaMonkeyRateFrame(TagLib::MP4::Tag* tag) :
		MP4::MP4Frame<Models::Popularimeter>(tag, "rate") {}

	MediaMonkeyRateFrame::~MediaMonkeyRateFrame() = default;

	std::optional<Models::Popularimeter> MediaMonkeyRateFrame::mapItemToData(const TagLib::MP4::Item& item) const
	{
		return ::mapItemToData(item);
	}

	std::optional<TagLib::MP4::Item> MediaMonkeyRateFrame::mapDataToItem(const Models::Popularimeter& popularimeter)
	{
		return ::mapDataToItem(popularimeter);
	}
}
