/* AlbumArtist.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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



#include "AlbumArtist.h"

MP4::AlbumArtistFrame::AlbumArtistFrame(TagLib::MP4::Tag* tag) :
	MP4::MP4Frame<QString>(tag, "aART") {}

MP4::AlbumArtistFrame::~AlbumArtistFrame() {}

bool MP4::AlbumArtistFrame::map_tag_to_model(QString& model)
{
	TagLib::MP4::Tag* tag = this->tag();
	const TagLib::MP4::ItemListMap& ilm = tag->itemListMap();

	auto it = find_key(ilm);
	if(it == ilm.end()){
		return false;
	}

	TagLib::MP4::Item item = it->second;
	if(!item.isValid()){
		return false;
	}

	TagLib::StringList items = item.toStringList();
	if(items.size() > 0){
		model = convert_string( *items.begin() );
		return true;
	}

	return false;
}

bool MP4::AlbumArtistFrame::map_model_to_tag(const QString& model)
{
	TagLib::MP4::Tag* tag = this->tag();

	TagLib::String key_str = tag_key();
	TagLib::MP4::ItemListMap& ilm = tag->itemListMap();
	TagLib::String str = convert_string(model);
	TagLib::StringList strings;

	strings.append(str);
	ilm.insert(key_str, strings);
	return true;
}
