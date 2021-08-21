/* Lyrics.cpp */

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

#include "Lyrics.h"

#include <taglib/tstring.h>

#include <optional>

ID3v2::LyricsFrame::LyricsFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<QString, TagLib::ID3v2::UnsynchronizedLyricsFrame>(tag, "USLT") {}

ID3v2::LyricsFrame::~LyricsFrame() = default;

void ID3v2::LyricsFrame::mapDataToFrame(const QString& data, TagLib::ID3v2::UnsynchronizedLyricsFrame* frame)
{
	const auto dataUtf8 = data.toUtf8();
	const auto str = TagLib::String(dataUtf8.constData(), TagLib::String::Type::UTF8);
	frame->setText(str);
}

std::optional<QString> ID3v2::LyricsFrame::mapFrameToData(const TagLib::ID3v2::UnsynchronizedLyricsFrame* frame) const
{
	const auto tagString = frame->text();
	const auto str = QString::fromUtf8(frame->toString().toCString(true));

	return std::optional(str);
}

TagLib::ID3v2::Frame* ID3v2::LyricsFrame::createId3v2Frame()
{
	return new TagLib::ID3v2::UnsynchronizedLyricsFrame(TagLib::String::UTF8);
}


