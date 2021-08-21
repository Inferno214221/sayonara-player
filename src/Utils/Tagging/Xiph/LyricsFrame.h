/* LyricsFrame.h */

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

#ifndef SAYONARA_XIPH_LYRICS_H
#define SAYONARA_XIPH_LYRICS_H

#include "XiphFrame.h"

#include <optional>

namespace Xiph
{
	class LyricsFrame :
		public Xiph::XiphFrame<QString>
	{
		public:
			explicit LyricsFrame(TagLib::Ogg::XiphComment* tag);
			~LyricsFrame() override;

		protected:
			std::optional<QString> mapTagToData() const  override;
			void mapDataToTag(const QString& lyrics) override;
	};
}

#endif // SAYONARA_XIPH_LYRICS_H
