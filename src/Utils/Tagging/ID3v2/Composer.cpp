/* AlbumArtist.cpp */

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

#include "Composer.h"

#include <taglib/textidentificationframe.h>

#include <optional>

ID3v2::ComposerFrame::ComposerFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<QString, TagLib::ID3v2::TextIdentificationFrame>(tag, "\xA9wrt") {}

ID3v2::ComposerFrame::~ComposerFrame() = default;

void ID3v2::ComposerFrame::mapDataToFrame(const QString& data, TagLib::ID3v2::TextIdentificationFrame* frame)
{
	const auto dataUtf8 = data.toUtf8();
	const auto dataString = TagLib::String(dataUtf8.constData(), TagLib::String::UTF8);
	frame->setText(dataString);
}

std::optional<QString> ID3v2::ComposerFrame::mapFrameToData(const TagLib::ID3v2::TextIdentificationFrame* frame) const
{
	const auto tagString = frame->toString();
	const auto str = QString::fromUtf8(tagString.toCString(true));

	return std::optional(str);
}

TagLib::ID3v2::Frame* ID3v2::ComposerFrame::createId3v2Frame()
{
	return new TagLib::ID3v2::TextIdentificationFrame("\xA9wrt", TagLib::String::UTF8);
}
