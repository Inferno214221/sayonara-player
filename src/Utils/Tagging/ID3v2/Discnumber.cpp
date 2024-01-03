/* Discnumber.cpp */

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

#include "Discnumber.h"

#include <QStringList>

#include <optional>

ID3v2::DiscnumberFrame::DiscnumberFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<Models::Discnumber, TagLib::ID3v2::TextIdentificationFrame>(tag, "TPOS") {}

ID3v2::DiscnumberFrame::~DiscnumberFrame() = default;

void ID3v2::DiscnumberFrame::mapDataToFrame(const Models::Discnumber& discnumber,
                                            TagLib::ID3v2::TextIdentificationFrame* frame)
{
	const auto discnumberData = discnumber.toString().toLatin1();
	const auto data = TagLib::String(discnumberData.constData(), TagLib::String::Latin1);
	frame->setText(data);
}

TagLib::ID3v2::Frame* ID3v2::DiscnumberFrame::createId3v2Frame()
{
	return new TagLib::ID3v2::TextIdentificationFrame("TPOS", TagLib::String::Latin1);
}

std::optional<Models::Discnumber>
ID3v2::DiscnumberFrame::mapFrameToData(const TagLib::ID3v2::TextIdentificationFrame* frame) const
{
	const auto text = frame->toString();
	const auto str = QString::fromLatin1(text.toCString());

	auto discnumber = Models::Discnumber(1, 1);

	const auto lst = str.split('/');
	if(lst.size() > 0)
	{
		discnumber.disc = lst[0].toInt();
	}

	if(lst.size() > 1)
	{
		discnumber.disccount = lst[1].toInt();
	}

	return std::optional(discnumber);
}

