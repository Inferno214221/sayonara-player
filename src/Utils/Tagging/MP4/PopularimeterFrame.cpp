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


MP4::PopularimeterFrame::PopularimeterFrame(TagLib::MP4::Tag* tag) :
	MP4::MP4Frame<Models::Popularimeter>(tag, "rtng")
{

}

MP4::PopularimeterFrame::~PopularimeterFrame() {}

bool MP4::PopularimeterFrame::map_tag_to_model(Models::Popularimeter& model)
{
	TagLib::MP4::Tag* tag = this->tag();
	TagLib::MP4::ItemListMap ilm = tag->itemListMap();
	TagLib::MP4::Item item = ilm[tag_key()];

	if(item.isValid())
	{
		uint8_t rating = (uint8_t) item.toByte();
		if(rating <= 5){
			model.set_rating(rating);
		}

		else{
			model.set_rating_byte(rating);
		}

		return true;
	}

	return false;
}


bool MP4::PopularimeterFrame::map_model_to_tag(const Models::Popularimeter& model)
{
	TagLib::MP4::Tag* tag = this->tag();
	TagLib::MP4::ItemListMap& ilm = tag->itemListMap();
	TagLib::MP4::Item item((uchar) model.get_rating_byte());

	ilm.insert(tag_key(), item);

	return true;
}
