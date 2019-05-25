/* AbstractFrame.h */

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



#ifndef TAGGINGABSTRACTFRAME_H
#define TAGGINGABSTRACTFRAME_H

#include "Utils/Pimpl.h"

#include <QString>

namespace TagLib
{
	class Tag;
	class String;
}

namespace Tagging
{
	class AbstractFrameHelper
	{
		public:
			explicit AbstractFrameHelper(const QString& key=QString());
			virtual ~AbstractFrameHelper();

		protected:
			QString cvt_string(const TagLib::String& str) const;
			TagLib::String cvt_string(const QString& str) const;
			QString key() const;
			TagLib::String tag_key() const;

		private:
			PIMPL(AbstractFrameHelper)
	};

	template<typename TagImpl>
	class AbstractFrame :
			public AbstractFrameHelper
	{
		private:
				TagImpl*	_tag=nullptr;

		protected:
			AbstractFrame(TagImpl* tag, const QString& key=QString()) :
				AbstractFrameHelper(key)
			{
				_tag = tag;
			}

			TagImpl* tag() const
			{
				return _tag;
			}

			void set_tag(TagImpl* tag)
			{
				_tag  = tag;
			}
	};
}
#endif // TAGGINGABSTRACTFRAME_H
