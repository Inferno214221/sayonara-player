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

namespace
{
	QString readLyricsFromId3(const Tagging::ParsedTag& parsedTag)
	{
		QString lyrics;

		auto* id3v2 = parsedTag.id3Tag();
		const auto lyricsFrame = ID3v2::LyricsFrame(id3v2);

		if(lyricsFrame.isFrameAvailable())
		{
			lyricsFrame.read(lyrics);
		}

		return lyrics;
	}

	QString readLyricsFromXiph(const Tagging::ParsedTag& parsedTag)
	{
		QString lyrics;

		auto* xiph = parsedTag.xiphTag();
		const auto lyricsFrame = Xiph::LyricsFrame(xiph);
		lyricsFrame.read(lyrics);

		return lyrics;
	}

	bool writeLyricsToId3(const Tagging::ParsedTag& parsedTag, const QString& lyrics)
	{
		auto* id3v2 = parsedTag.id3Tag();
		auto lyricsFrame = ID3v2::LyricsFrame(id3v2);

		return lyricsFrame.write(lyrics);
	}

	bool writeLyricsToXiph(const Tagging::ParsedTag& parsedTag, const QString& lyrics)
	{
		auto* xiph = parsedTag.xiphTag();
		auto lyricsFrame = Xiph::LyricsFrame(xiph);

		return lyricsFrame.write(lyrics);
	}
}

bool Tagging::writeLyrics(const MetaData& track, const QString& lyricsData)
{
	auto fileRef = TagLib::FileRef(TagLib::FileName(track.filepath().toUtf8()));
	if(!Tagging::isValidFile(fileRef))
	{
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << track.filepath();
		return false;
	}

	auto success = false;

	const auto parsedTag = Tagging::getParsedTagFromFileRef(fileRef);
	if((parsedTag.type == Tagging::TagType::ID3v2) && parsedTag.id3Tag())
	{
		success = writeLyricsToId3(parsedTag, lyricsData);
	}

	else if((parsedTag.type == Tagging::TagType::Xiph) && parsedTag.xiphTag())
	{
		success = writeLyricsToXiph(parsedTag, lyricsData);
	}

	return (success)
	       ? fileRef.save()
	       : false;
}

bool Tagging::extractLyrics(const MetaData& track, QString& lyricsData)
{
	lyricsData.clear();

	const auto fileRef = TagLib::FileRef(TagLib::FileName(track.filepath().toUtf8()));
	if(!Tagging::isValidFile(fileRef))
	{
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << track.filepath();
		return false;
	}

	const auto parsedTag = Tagging::getParsedTagFromFileRef(fileRef);

	if((parsedTag.type == Tagging::TagType::ID3v2) && parsedTag.id3Tag())
	{
		lyricsData = readLyricsFromId3(parsedTag);
	}

	else if((parsedTag.type == Tagging::TagType::Xiph) && parsedTag.xiphTag())
	{
		lyricsData = readLyricsFromXiph(parsedTag);
	}

	return (!lyricsData.isEmpty());
}

bool Tagging::isLyricsSupported(const QString& filepath)
{
	const auto fileRef = TagLib::FileRef(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::isValidFile(fileRef))
	{
		return false;
	}

	const auto parsedTag = Tagging::getParsedTagFromFileRef(fileRef);
	return ((parsedTag.type == Tagging::TagType::ID3v2) ||
	        (parsedTag.type == Tagging::TagType::Xiph));
}

