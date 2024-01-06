/* TaggingLyrics.cpp */

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

#include "TaggingLyrics.h"

#include "Tagging.h"
#include "Tagging/ID3v2/Lyrics.h"
#include "Tagging/Xiph/LyricsFrame.h"
#include "Tagging/MP4/Lyrics.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QString>

#include <taglib/fileref.h>

namespace
{
	template<typename Frame_t, typename Tag_t>
	bool writeLyrics(Tag_t* tag, const QString& lyrics)
	{
		auto frame = Frame_t {tag};
		return frame.write(lyrics);
	}

	template<typename Frame_t, typename Tag_t>
	QString readLyrics(Tag_t* tag)
	{
		QString lyrics;

		const auto lyricsFrame = Frame_t {tag};
		if(lyricsFrame.isFrameAvailable())
		{
			lyricsFrame.read(lyrics);
		}

		return lyrics;
	}
}

bool Tagging::writeLyrics(const QString& filepath, const QString& lyricsData)
{
	auto fileRef = TagLib::FileRef(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::isValidFile(fileRef))
	{
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << filepath;
		return false;
	}

	auto success = false;

	const auto parsedTag = Tagging::getParsedTagFromFileRef(fileRef);
	if((parsedTag.type == Tagging::TagType::ID3v2) && parsedTag.id3Tag())
	{
		success = ::writeLyrics < ID3v2::LyricsFrame > (parsedTag.id3Tag(), lyricsData);
	}

	else if((parsedTag.type == Tagging::TagType::Xiph) && parsedTag.xiphTag())
	{
		success = ::writeLyrics < Xiph::LyricsFrame > (parsedTag.xiphTag(), lyricsData);
	}

	else if((parsedTag.type == Tagging::TagType::MP4) && parsedTag.mp4Tag())
	{
		success = ::writeLyrics < MP4::Lyrics > (parsedTag.mp4Tag(), lyricsData);
	}

	return (success)
	       ? fileRef.save()
	       : false;
}

bool Tagging::extractLyrics(const QString& filepath, QString& lyricsData)
{
	lyricsData.clear();

	const auto fileRef = TagLib::FileRef(TagLib::FileName(filepath.toUtf8()));
	if(!Tagging::isValidFile(fileRef))
	{
		spLog(Log::Warning, "Tagging") << "Cannot open tags for " << filepath;
		return false;
	}

	const auto parsedTag = Tagging::getParsedTagFromFileRef(fileRef);

	if((parsedTag.type == Tagging::TagType::ID3v2) && parsedTag.id3Tag())
	{
		lyricsData = ::readLyrics<ID3v2::LyricsFrame>(parsedTag.id3Tag());
	}

	else if((parsedTag.type == Tagging::TagType::Xiph) && parsedTag.xiphTag())
	{
		lyricsData = ::readLyrics<Xiph::LyricsFrame>(parsedTag.xiphTag());
	}

	else if((parsedTag.type == Tagging::TagType::MP4) && parsedTag.mp4Tag())
	{
		lyricsData = ::readLyrics<MP4::Lyrics>(parsedTag.mp4Tag());
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
	        (parsedTag.type == Tagging::TagType::Xiph) ||
	        (parsedTag.type == Tagging::TagType::MP4));
}

