/* ${CLASS_NAME}.h */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_TAGGINGEXTRAFIELDS_H
#define SAYONARA_PLAYER_TAGGINGEXTRAFIELDS_H

class MetaData;
class QString;

namespace Models
{
	struct Discnumber;
	struct Popularimeter;
}

namespace Tagging
{
	template<typename FrameType, typename Model, typename Tag, typename ConversionFunction>
	void tryToRead(Tag* tag, MetaData& track, ConversionFunction fn)
	{
		if(tag)
		{
			Model model;
			auto frame = FrameType(tag);
			const auto success = frame.read(model);
			if(success)
			{
				fn(track, model);
			}
		}
	}

	template<typename FrameType, typename Tag, typename Model>
	void tryToWrite(Tag* tag, const Model& model)
	{
		if(tag)
		{
			FrameType frame(tag);
			frame.write(model);
		}
	}

	struct ParsedTag;

	void readDiscnumber(MetaData& track, const Tagging::ParsedTag& parsedTag);
	void writeDiscnumber(const Tagging::ParsedTag& parsedTag, const Models::Discnumber& discnumber);

	void readPopularimeter(MetaData& track, const Tagging::ParsedTag& parsedTag);
	void writePopularimeter(const Tagging::ParsedTag& parsedTag, const Models::Popularimeter& popularimeter);

	void readAlbumArtist(MetaData& track, const Tagging::ParsedTag& parsedTag);
	void writeAlbumArtist(const Tagging::ParsedTag& parsedTag, const QString& albumArtist);
}

#endif //SAYONARA_PLAYER_TAGGINGEXTRAFIELDS_H
