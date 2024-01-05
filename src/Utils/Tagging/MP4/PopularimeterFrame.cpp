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

	std::optional<Models::Popularimeter> parseRatingInt(const int i)
	{
		auto popularimeter = Models::Popularimeter {};
		if(i == 0)
		{
			popularimeter.rating = Rating::Zero;
		}
		else if(i <= 5)
		{
			popularimeter.rating = static_cast<Rating>(i);
		}
		else if(i <= 10) // 6 - 10
		{
			popularimeter.rating = static_cast<Rating>(i / 2);
		}
		else
		{
			popularimeter.setRatingByte(static_cast<uchar>(i));
		}

		return {popularimeter};
	}

	std::optional<Models::Popularimeter> parseRatingString(const TagLib::String& str)
	{
		auto ok = false;
		const auto i = str.toInt(&ok);
		if(!ok)
		{
			return std::nullopt;
		}

		return parseRatingInt(i);
	}

	std::optional<Models::Popularimeter> mapItemToData(const TagLib::MP4::Item& item)
	{
		TagLib::StringList strings;
		const auto stringList = item.toStringList();
		if(!stringList.isEmpty())
		{
			auto popularimeter = parseRatingString(stringList[0]);
			if(popularimeter.has_value())
			{
				return popularimeter;
			}
		}

		return {parseRatingInt(item.toInt())};
	}

	std::optional<TagLib::MP4::Item> mapDataToItem(const Models::Popularimeter& popularimeter)
	{
		const auto i = static_cast<uchar>(popularimeter.ratingByte());
		const auto item = TagLib::MP4::Item(TagLib::StringList {Tagging::convertString(QString::number(i))});
		//const auto item = TagLib::MP4::Item(i);

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
