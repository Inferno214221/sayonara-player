/* XiphFrame.h */

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



#ifndef ABSTRACT_XIPH_FRAME_H_
#define ABSTRACT_XIPH_FRAME_H_

#include "Utils/Tagging/AbstractFrame.h"
#include <QString>
#include <taglib/tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/tstring.h>
#include <taglib/tstringlist.h>

namespace Xiph
{
	template<typename Model_t>
	class XiphFrame :
			protected Tagging::AbstractFrame<TagLib::Ogg::XiphComment>
	{
		protected:
			virtual bool map_tag_to_model(Model_t& model)=0;
			virtual bool map_model_to_tag(const Model_t& model)=0;

			// those methods are usually called by the implementations
			// in order to retrieve and write back the data easily
			bool value(TagLib::String& str) const
			{
				TagLib::Ogg::XiphComment* tag = this->tag();
				const TagLib::Ogg::FieldListMap& map = tag->fieldListMap();
				TagLib::Ogg::FieldListMap::ConstIterator it = map.find( this->tag_key() );
				if(it == map.end()){
					str = TagLib::String();
					return false;
				}

				str = it->second.front();
				return true;
			}

			void set_value(const TagLib::String& value)
			{
				TagLib::Ogg::XiphComment* tag = this->tag();
				tag->addField(this->tag_key(), value, true);
			}

			void set_value(const QString& value)
			{
				TagLib::String str = this->cvt_string(value);
				set_value(str);
			}

		public:
			XiphFrame(TagLib::Ogg::XiphComment* tag, const QString& identifier) :
				Tagging::AbstractFrame<TagLib::Ogg::XiphComment>(tag, identifier) {}

			virtual ~XiphFrame() {}

			bool read(Model_t& model)
			{
				if(!this->tag()){
					return false;
				}

				bool success = map_tag_to_model(model);

				return success;
			}

			bool write(const Model_t& model)
			{
				TagLib::Ogg::XiphComment* tag = this->tag();
				if(!tag) {
					return false;
				}

				TagLib::String key = this->tag_key();
				if(!key.isEmpty())
				{
					tag->removeField( this->tag_key() );
				}

				return map_model_to_tag(model);
			}

			virtual bool is_frame_found() const
			{
				if(this->tag_key().isEmpty())
				{
					return false;
				}

				return this->tag()->contains("METADTA_BLOCK_PICTURE");
			}
	};
}

#endif // ABSTRACTFRAME_H
