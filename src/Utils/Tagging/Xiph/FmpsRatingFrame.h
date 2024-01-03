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

#ifndef SAYONARA_XIPH_FMPS_RATING_FRAME_H
#define SAYONARA_XIPH_FMPS_RATING_FRAME_H

#include "Utils/Tagging/Xiph/XiphFrame.h"
#include "Utils/Tagging/Models/Popularimeter.h"

#include <optional>

namespace Xiph
{
	class FmpsRatingFrame :
		public XiphFrame<Models::Popularimeter>
	{
		public:
			FmpsRatingFrame(TagLib::Ogg::XiphComment* tag);
			~FmpsRatingFrame() override;

		protected:
			std::optional<Models::Popularimeter> mapTagToData() const override;
			void mapDataToTag(const Models::Popularimeter& pop) override;
	};

	class FmpsUserRatingFrame :
		public XiphFrame<Models::Popularimeter>
	{
		public:
			FmpsUserRatingFrame(TagLib::Ogg::XiphComment* tag);
			~FmpsUserRatingFrame() override;

		protected:
			std::optional<Models::Popularimeter> mapTagToData() const override;
			void mapDataToTag(const Models::Popularimeter& pop) override;
	};
}

#endif // SAYONARA_XIPH_FMPS_RATING_FRAME_H
