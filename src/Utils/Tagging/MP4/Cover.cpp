/* Cover.cpp */

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

#include "Cover.h"

MP4::CoverFrame::CoverFrame(TagLib::MP4::Tag* tag) :
	MP4Frame<Models::Cover>(tag, "covr") {}

MP4::CoverFrame::~CoverFrame() = default;

std::optional<Models::Cover> MP4::CoverFrame::mapItemToData(const TagLib::MP4::Item& item) const
{
	const auto coverArtList = item.toCoverArtList();
	if(coverArtList.isEmpty())
	{
		return std::optional<Models::Cover> {};
	}

	const auto& coverArt = coverArtList[0];
	auto cover = Models::Cover {};
	cover.imageData = QByteArray(coverArt.data().data(), coverArt.data().size());

	return std::optional(cover);
}

std::optional<TagLib::MP4::Item> MP4::CoverFrame::mapDataToItem(const Models::Cover& cover)
{
	TagLib::MP4::CoverArt::Format format;
	switch(cover.convertMimeType())
	{
		case Models::Cover::MimeType::PNG:
			format = TagLib::MP4::CoverArt::PNG;
			break;
		case Models::Cover::MimeType::JPEG:
			format = TagLib::MP4::CoverArt::JPEG;
			break;
		default:
			return std::optional<TagLib::MP4::Item> {};
	}

	const auto taglibData = TagLib::ByteVector(cover.imageData.data(), cover.imageData.size());
	const auto coverArt = TagLib::MP4::CoverArt(format, taglibData);

	TagLib::MP4::CoverArtList coverArtList;
	coverArtList.append(coverArt);

	const auto item = TagLib::MP4::Item(coverArtList);

	return std::optional(item);
}