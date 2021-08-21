/* Popularimeter.cpp */

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

#include "Popularimeter.h"

#include <optional>

using ID3v2::PopularimeterFrame;

PopularimeterFrame::PopularimeterFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<Models::Popularimeter, TagLib::ID3v2::PopularimeterFrame>(tag, "POPM") {}

PopularimeterFrame::~PopularimeterFrame() = default;

TagLib::ID3v2::Frame* PopularimeterFrame::createId3v2Frame()
{
	return new TagLib::ID3v2::PopularimeterFrame();
}

void PopularimeterFrame::mapDataToFrame(const Models::Popularimeter& data,
                                        TagLib::ID3v2::PopularimeterFrame* frame)
{
	frame->setEmail(TagLib::String(data.email.toUtf8().data(), TagLib::String::UTF8));
	frame->setCounter(data.playcount);
	frame->setRating(data.ratingByte());
	frame->render();
}

std::optional<Models::Popularimeter>
PopularimeterFrame::mapFrameToData(const TagLib::ID3v2::PopularimeterFrame* frame) const
{
	auto pop = Models::Popularimeter();

	pop.playcount = frame->counter();
	pop.email = QString::fromLatin1(frame->email().toCString());
	pop.setRatingByte((Byte) frame->rating());

	return std::optional(pop);
}