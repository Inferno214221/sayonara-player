/* Popularimeter.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

ID3v2::PopularimeterFrame::PopularimeterFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<Models::Popularimeter, TagLib::ID3v2::PopularimeterFrame>(tag, "POPM") {}

ID3v2::PopularimeterFrame::~PopularimeterFrame() {}

TagLib::ID3v2::Frame* ID3v2::PopularimeterFrame::create_id3v2_frame()
{
	return new TagLib::ID3v2::PopularimeterFrame();
}

void ID3v2::PopularimeterFrame::map_model_to_frame(const Models::Popularimeter& model, TagLib::ID3v2::PopularimeterFrame* frame)
{
	frame->setEmail(TagLib::String(model.email.toUtf8().data(), TagLib::String::UTF8));
	frame->setCounter(model.playcount);
	frame->setRating(model.get_rating_byte());
	frame->render();
}

void ID3v2::PopularimeterFrame::map_frame_to_model(const TagLib::ID3v2::PopularimeterFrame* frame, Models::Popularimeter& model)
{
	model.playcount = frame->counter();
	model.email = QString::fromLatin1(frame->email().toCString());
	model.set_rating_byte((Byte) frame->rating());
}
