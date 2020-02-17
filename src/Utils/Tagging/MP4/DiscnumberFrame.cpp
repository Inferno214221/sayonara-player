/* DiscnumberFrame.cpp */

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

#include "DiscnumberFrame.h"
#include "taglib/mp4item.h"

MP4::DiscnumberFrame::DiscnumberFrame(TagLib::MP4::Tag* tag) :
    MP4::MP4Frame<Models::Discnumber>(tag, "disk")
{}

MP4::DiscnumberFrame::~DiscnumberFrame() {}

bool MP4::DiscnumberFrame::map_tag_to_model(Models::Discnumber& model)
{
	TagLib::MP4::Tag* tag = this->tag();

	TagLib::MP4::ItemListMap ilm = tag->itemListMap();
	TagLib::MP4::Item item = ilm[tag_key()];

	if(item.isValid()){
		TagLib::MP4::Item::IntPair p = item.toIntPair();
		model.disc = p.first;
		model.disccount = p.second;
		return true;
	}

	return false;
}

bool MP4::DiscnumberFrame::map_model_to_tag(const Models::Discnumber& model)
{
	TagLib::MP4::ItemListMap& ilm = this->tag()->itemListMap();
	TagLib::MP4::Item item(model.disc, model.disccount);
	TagLib::String key_str = tag_key();

	auto it = ilm.find(key_str);
	while(it != ilm.end()){
		ilm.erase(it);
		it = ilm.find(key_str);
	}

	ilm.insert(key_str, item);

    return true;
}

