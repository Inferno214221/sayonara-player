/* AbstractFrame.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef SAYONARA_TAGGING_ABSTRACTFRAME_H
#define SAYONARA_TAGGING_ABSTRACTFRAME_H

#include "Utils/Pimpl.h"
#include "Utils/Tagging/TaggingUtils.h"

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
			explicit AbstractFrameHelper(const QByteArray& key = QByteArray());
			virtual ~AbstractFrameHelper();

		protected:
			[[nodiscard]] QByteArray key() const;
			[[nodiscard]] TagLib::ByteVector tagKey() const;

		private:
		PIMPL(AbstractFrameHelper)
	};

	template<typename TagImpl>
	class AbstractFrame :
		public AbstractFrameHelper
	{
		private:
			TagImpl* mTag {nullptr};

		protected:
			explicit AbstractFrame(TagImpl* tag, const QByteArray& key = QByteArray()) :
				AbstractFrameHelper(key),
				mTag(tag) {}

			~AbstractFrame() override = default;

			TagImpl* tag() const { return mTag; }
	};
}
#endif // SAYONARA_TAGGING_ABSTRACTFRAME_H
