/* MP4Frame.h */

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



#ifndef ABSTRACT_MP4_FRAME_H_
#define ABSTRACT_MP4_FRAME_H_

#include "Utils/Tagging/AbstractFrame.h"

#include "taglib/tag.h"
#include "taglib/tstringlist.h"
#include "taglib/mp4tag.h"

#include <QString>

namespace MP4
{
template<typename Model_t>
class MP4Frame :
		protected Tagging::AbstractFrame<TagLib::MP4::Tag>
{
protected:
	TagLib::MP4::ItemListMap::ConstIterator find_key(const TagLib::MP4::ItemListMap& ilm) const
	{
		for(TagLib::MP4::ItemListMap::ConstIterator it=ilm.begin(); it!=ilm.end(); it++){
			QString key = this->key();
			if( this->convert_string(it->first).compare(key, Qt::CaseInsensitive) == 0){
				return it;
			}
		}

		return ilm.end();
	}

	virtual bool map_tag_to_model(Model_t& model)=0;
	virtual bool map_model_to_tag(const Model_t& model)=0;


public:
	MP4Frame(TagLib::MP4::Tag* tag, const QString& identifier) :
		Tagging::AbstractFrame<TagLib::MP4::Tag>(tag, identifier) {}

	virtual ~MP4Frame()	= default;

	bool read(Model_t& model)
	{
		TagLib::MP4::Tag* key = this->tag();
		if(!key) {
			return false;
		}

		const TagLib::MP4::ItemListMap& ilm = key->itemListMap();

		bool found = (find_key(ilm) != ilm.end());
		if(!found){
			return false;
		}

		bool success = map_tag_to_model(model);

		return success;
	}

	bool write(const Model_t& model)
	{
		TagLib::MP4::Tag* tag = this->tag();
		if(!tag) {
			return false;
		}

		TagLib::MP4::ItemListMap& ilm = tag->itemListMap();

		auto itcopy=ilm.begin();
		for(auto it=ilm.begin(); it!=ilm.end(); it++)
		{
			QString key = this->key();
			if( this->convert_string(it->first).compare(key, Qt::CaseInsensitive) == 0)
			{
				ilm.erase(it);
				it = itcopy;
			}

			else{
				itcopy = it;
			}
		}

		return map_model_to_tag(model);
	}

	bool is_frame_found()
	{
		TagLib::MP4::Tag* key = this->tag();
		if(!key) {
			return false;
		}

		const TagLib::MP4::ItemListMap& ilm = key->itemListMap();

		return (find_key(ilm) != ilm.end());
	}
};
}

#endif // ABSTRACT_MP4_FRAME_H_
