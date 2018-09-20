/* Cover.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#include "Cover.h"

MP4::CoverFrame::CoverFrame(TagLib::MP4::Tag* tag) :
	MP4Frame<Models::Cover>(tag, "covr") {}

MP4::CoverFrame::~CoverFrame() {}

bool MP4::CoverFrame::map_tag_to_model(Models::Cover& model)
{
	TagLib::MP4::Tag* tag = this->tag();

	const TagLib::MP4::ItemListMap& ilm = tag->itemListMap();
	TagLib::MP4::ItemListMap::ConstIterator it = find_key(ilm);
	if(it == ilm.end()){
		return false;
	}

	TagLib::MP4::Item item = it->second;
	if(!item.isValid()){
		return false;
	}

	TagLib::MP4::CoverArtList arts = item.toCoverArtList();
	if(arts.isEmpty()){
		return  false;
	}

	TagLib::MP4::CoverArt art = arts[0];
	model.image_data = QByteArray(art.data().data(), art.data().size());

	return true;
}

bool MP4::CoverFrame::map_model_to_tag(const Models::Cover& model)
{
	TagLib::MP4::Tag* tag = this->tag();
	const QByteArray& image_data = model.image_data;

	TagLib::ByteVector taglib_data;
	taglib_data.setData(image_data.data(), image_data.size());

	TagLib::MP4::CoverArt::Format format;
	switch(model.get_mime_type()){
		case Models::Cover::MimeType::PNG:
			format = TagLib::MP4::CoverArt::PNG;
			break;
		case Models::Cover::MimeType::JPEG:
			format = TagLib::MP4::CoverArt::JPEG;
			break;
		default:
			return false;
	}

	TagLib::MP4::CoverArt art(format, taglib_data);
	TagLib::MP4::CoverArtList arts;
	arts.append(art);
	TagLib::MP4::Item item(arts);
	TagLib::String tag_str = cvt_string(key());

	tag->itemListMap().insert(tag_str, item);

	return true;
}
