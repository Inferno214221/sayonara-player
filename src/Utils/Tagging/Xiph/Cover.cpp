/* Cover.cpp */

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

#include "Cover.h"

#include "Utils/Logger/Logger.h"

#include <taglib/flacpicture.h>
#include <taglib/flacfile.h>
#include <taglib/oggfile.h>
#include <taglib/tmap.h>

#include <optional>

namespace TL = TagLib;

namespace
{
	TL::FLAC::Picture* getBestFittingPicture(const TL::List<TL::FLAC::Picture*>& pictures)
	{
		if(pictures.isEmpty())
		{
			return nullptr;
		}

		TL::FLAC::Picture* candidate = nullptr;
		for(auto* picture: pictures)
		{
			if(picture->type() == TL::FLAC::Picture::FrontCover)
			{
				if(picture->data().size() < 100)
				{
					continue;
				}

				candidate = picture;
			}

			else if((picture->type() == TL::FLAC::Picture::Other) && !candidate)
			{
				if(picture->data().size() < 100)
				{
					continue;
				}

				candidate = picture;
			}
		}

		return (!candidate)
		       ? pictures[0]
		       : candidate;
	}
}

Xiph::CoverFrame::CoverFrame(TagLib::Ogg::XiphComment* tag) :
	Xiph::XiphFrame<Models::Cover>(tag, "") {}

Xiph::CoverFrame::~CoverFrame() = default;

std::optional<Models::Cover> Xiph::CoverFrame::mapTagToData() const
{
	const auto pictures = tag()->pictureList();
	if(pictures.isEmpty())
	{
		const auto cover = Models::Cover(QString(), QByteArray());
		return std::optional(cover);
	}

	auto* picture = getBestFittingPicture(pictures);
	if(picture)
	{
		const auto data = picture->data();
		const auto cover = Models::Cover(
			Tagging::convertString(picture->mimeType()),
			QByteArray(data.data(), static_cast<int>(data.size()))
		);

		return std::optional(cover);
	}

	return std::optional<Models::Cover>();
}

void Xiph::CoverFrame::mapDataToTag(const Models::Cover& cover)
{
	this->tag()->removeAllPictures();

	const auto dataSize = static_cast<unsigned int>(cover.imageData.size());
	const auto imageData = TL::ByteVector(cover.imageData.data(), dataSize);

	auto* picture = new TL::FLAC::Picture();

	picture->setType(TL::FLAC::Picture::FrontCover);
	picture->setMimeType(Tagging::convertString(cover.mimeType));
	picture->setDescription(TL::String("Front Cover By Sayonara"));
	picture->setData(TL::ByteVector(cover.imageData.data(), dataSize));

	tag()->addPicture(picture); // do not delete the picture, because tag will take ownership
}

bool Xiph::CoverFrame::isFrameAvailable() const
{
	const auto hasEntries = (this->tag()->pictureList().isEmpty() == false);
	spLog(Log::Develop, this) << "Picture list has " << this->tag()->pictureList().size() << " entries";

	return hasEntries;
}
