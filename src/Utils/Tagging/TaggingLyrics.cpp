/* TaggingLyrics.cpp */

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

#include "TaggingLyrics.h"
#include "Tagging.h"
#include "Tagging/ID3v2/Lyrics.h"
#include "Tagging/Xiph/LyricsFrame.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QString>

#include <taglib/fileref.h>

bool Tagging::Lyrics::writeLyrics(const MetaData& md, const QString& lyrics_data)
{
	QString filepath = md.filepath();
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::Utils::isValidFile(f)){
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << md.filepath();
		return false;
	}

	bool success = false;

	Tagging::ParsedTag parsed_tag = Tagging::Utils::getTagTypeFromFileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
			{
				auto id3v2 = parsed_tag.id3Tag();
				ID3v2::LyricsFrame lyrics_frame(id3v2);
				success = lyrics_frame.write(lyrics_data);
			}

			break;

		case Tagging::TagType::Xiph:
			{
				auto xiph = parsed_tag.xiphTag();
				Xiph::LyricsFrame lyrics_frame(xiph);
				success = lyrics_frame.write(lyrics_data);
			}

			break;

		default:
			return false;
	}

	Q_UNUSED(success)
	return f.save();
}


bool Tagging::Lyrics::extractLyrics(const MetaData& md, QString& lyrics_data)
{
	lyrics_data.clear();

	QString filepath = md.filepath();
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));

	if(!Tagging::Utils::isValidFile(f)){
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << md.filepath();
		return false;
	}

	Tagging::ParsedTag parsed_tag = Tagging::Utils::getTagTypeFromFileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	switch(tag_type)
	{
		case Tagging::TagType::ID3v2:
			{
				auto id3v2 = parsed_tag.id3Tag();
				ID3v2::LyricsFrame lyrics_frame(id3v2);

				if(!lyrics_frame.is_frame_found()){
					return false;
				}

				lyrics_frame.read(lyrics_data);
			}

			break;

		case Tagging::TagType::Xiph:
			{
				auto xiph = parsed_tag.xiphTag();
				Xiph::LyricsFrame lyrics_frame(xiph);
				lyrics_frame.read(lyrics_data);
			}

			break;

		default:
			return false;
	}

	return !(lyrics_data.isEmpty());
}


bool Tagging::Lyrics::isLyricsSupported(const QString& filepath)
{
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));

	if(!Tagging::Utils::isValidFile(f)){
		return false;
	}

	Tagging::ParsedTag parsed_tag = Tagging::Utils::getTagTypeFromFileref(f);
	Tagging::TagType tag_type = parsed_tag.type;

	return ((tag_type == Tagging::TagType::ID3v2) ||
			(tag_type == Tagging::TagType::Xiph));
}

