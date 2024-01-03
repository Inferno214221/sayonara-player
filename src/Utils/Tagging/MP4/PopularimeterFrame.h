/* PopularimeterFrame.h */

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

#ifndef SAYONARA_MP4_POPULARIMETERFRAME_H
#define SAYONARA_MP4_POPULARIMETERFRAME_H

#include "MP4Frame.h"
#include "Utils/Tagging/Models/Popularimeter.h"

#include <optional>

namespace MP4
{
	class PopularimeterFrame :
		public MP4::MP4Frame<Models::Popularimeter>
	{
		public:
			PopularimeterFrame(TagLib::MP4::Tag* tag);
			~PopularimeterFrame() override;

		protected:
			std::optional<Models::Popularimeter> mapItemToData(const TagLib::MP4::Item& item) const override;
			std::optional<TagLib::MP4::Item> mapDataToItem(const Models::Popularimeter& model) override;
	};
}

#endif // SAYONARA_MP4_POPULARIMETERFRAME_H
