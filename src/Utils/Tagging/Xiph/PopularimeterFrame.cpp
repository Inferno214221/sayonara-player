/* PopularimeterFrame.cpp */

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



#include "PopularimeterFrame.h"

Xiph::PopularimeterFrame::PopularimeterFrame(TagLib::Ogg::XiphComment* tag) :
	XiphFrame<Models::Popularimeter>(tag, "RATING")
{}

Xiph::PopularimeterFrame::~PopularimeterFrame() {}

bool Xiph::PopularimeterFrame::map_tag_to_model(Models::Popularimeter& model)
{
	TagLib::String str;
	bool success = this->value(str);
	if(!success){
		return false;
	}

	auto bRating = static_cast<Byte>(convert_string(str).toInt());
	if(bRating <= 5){
		Rating rating = static_cast<Rating>(bRating);
		model.set_rating(rating);
	}

	else{
		model.set_rating_byte(bRating);
	}

	return true;
}

bool Xiph::PopularimeterFrame::map_model_to_tag(const Models::Popularimeter& model)
{
	Rating rating = model.get_rating();
	auto iRating = static_cast<int>(rating);
	set_value( QString::number(iRating) );
	return true;
}
