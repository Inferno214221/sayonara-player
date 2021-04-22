/* ${CLASS_NAME}.h */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "TaggingExtraFields.h"
#include "TaggingUtils.h"

#include "Utils/MetaData/MetaData.h"

#include "ID3v2/Popularimeter.h"
#include "ID3v2/Discnumber.h"
#include "ID3v2/AlbumArtist.h"
#include "Xiph/AlbumArtist.h"
#include "Xiph/PopularimeterFrame.h"
#include "Xiph/DiscnumberFrame.h"
#include "MP4/AlbumArtist.h"
#include "MP4/DiscnumberFrame.h"
#include "MP4/PopularimeterFrame.h"

namespace Tagging
{
	void readDiscnumber(MetaData& track, const Tagging::ParsedTag& parsedTag)
	{
		auto setter = [](auto& track, const auto& discnumber){
			track.setDiscnumber(discnumber.disc);
			track.setDiscCount(discnumber.disccount);
		};

		if(parsedTag.type == Tagging::TagType::ID3v2)
		{
			tryToRead<ID3v2::DiscnumberFrame, Models::Discnumber>(parsedTag.id3Tag(), track, setter);
		}

		else if(parsedTag.type == Tagging::TagType::Xiph)
		{
			tryToRead<Xiph::DiscnumberFrame, Models::Discnumber>(parsedTag.xiphTag(), track, setter);
		}

		else if(parsedTag.type == Tagging::TagType::MP4)
		{
			tryToRead<MP4::DiscnumberFrame, Models::Discnumber>(parsedTag.mp4Tag(), track, setter);
		}
	}

	void writeDiscnumber(const Tagging::ParsedTag& parsedTag, const Models::Discnumber& discnumber)
	{
		if(parsedTag.type == Tagging::TagType::ID3v2)
		{
			tryToWrite<ID3v2::DiscnumberFrame>(parsedTag.id3Tag(), discnumber);
		}

		else if(parsedTag.type == Tagging::TagType::Xiph)
		{
			tryToWrite<Xiph::DiscnumberFrame>(parsedTag.xiphTag(), discnumber);
		}

		else if(parsedTag.type == Tagging::TagType::MP4)
		{
			tryToWrite<MP4::DiscnumberFrame>(parsedTag.mp4Tag(), discnumber);
		}
	}

	void readPopularimeter(MetaData& track, const Tagging::ParsedTag& parsedTag)
	{
		auto setter = [](auto& track, const auto& popularimeter){
			track.setRating(popularimeter.rating);
		};

		if(parsedTag.type == Tagging::TagType::ID3v2)
		{
			tryToRead<ID3v2::PopularimeterFrame, Models::Popularimeter>(parsedTag.id3Tag(), track, setter);
		}

		else if(parsedTag.type == Tagging::TagType::Xiph)
		{
			tryToRead<Xiph::PopularimeterFrame, Models::Popularimeter>(parsedTag.xiphTag(), track, setter);
		}

		else if(parsedTag.type == Tagging::TagType::MP4)
		{
			tryToRead<MP4::PopularimeterFrame, Models::Popularimeter>(parsedTag.mp4Tag(), track, setter);
		}
	}

	void writePopularimeter(const Tagging::ParsedTag& parsedTag, const Models::Popularimeter& popularimeter)
	{
		if(parsedTag.type == Tagging::TagType::ID3v2)
		{
			tryToWrite<ID3v2::PopularimeterFrame>(parsedTag.id3Tag(), popularimeter);
		}

		else if(parsedTag.type == Tagging::TagType::Xiph)
		{
			tryToWrite<Xiph::PopularimeterFrame>(parsedTag.xiphTag(), popularimeter);
		}

		else if(parsedTag.type == Tagging::TagType::MP4)
		{
			tryToWrite<MP4::PopularimeterFrame>(parsedTag.mp4Tag(), popularimeter);
		}
	}

	void readAlbumArtist(MetaData& track, const Tagging::ParsedTag& parsedTag)
	{
		auto setter = [](auto& track, const auto& albumArtist){
			track.setAlbumArtist(albumArtist);
		};

		if(parsedTag.type == Tagging::TagType::ID3v2)
		{
			tryToRead<ID3v2::AlbumArtistFrame, QString>(parsedTag.id3Tag(), track, setter);
		}

		else if(parsedTag.type == Tagging::TagType::Xiph)
		{
			tryToRead<Xiph::AlbumArtistFrame, QString>(parsedTag.xiphTag(), track, setter);
		}

		else if(parsedTag.type == Tagging::TagType::MP4)
		{
			tryToRead<MP4::AlbumArtistFrame, QString>(parsedTag.mp4Tag(), track, setter);
		}
	}

	void writeAlbumArtist(const Tagging::ParsedTag& parsedTag, const QString& albumArtist)
	{
		if(parsedTag.type == Tagging::TagType::ID3v2)
		{
			tryToWrite<ID3v2::AlbumArtistFrame>(parsedTag.id3Tag(), albumArtist);
		}

		else if(parsedTag.type == Tagging::TagType::Xiph)
		{
			tryToWrite<Xiph::AlbumArtistFrame>(parsedTag.xiphTag(), albumArtist);
		}

		else if(parsedTag.type == Tagging::TagType::MP4)
		{
			tryToWrite<MP4::AlbumArtistFrame>(parsedTag.mp4Tag(), albumArtist);
		}
	}
}