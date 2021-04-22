/* Popularimeter.h */

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

#ifndef SAYONARA_ID3V2_POPULARIMETER_H
#define SAYONARA_ID3V2_POPULARIMETER_H

#include "ID3v2Frame.h"
#include "Utils/Tagging/Models/Popularimeter.h"

#include <taglib/popularimeterframe.h>

namespace ID3v2
{
	class PopularimeterFrame :
		public ID3v2Frame<Models::Popularimeter, TagLib::ID3v2::PopularimeterFrame>
	{
		public:
			PopularimeterFrame(TagLib::ID3v2::Tag* tag);
			~PopularimeterFrame() override;

		protected:
			TagLib::ID3v2::Frame* createId3v2Frame() override;

			std::optional<Models::Popularimeter>
			mapFrameToData(const TagLib::ID3v2::PopularimeterFrame* frame) const override;
			void mapDataToFrame(const Models::Popularimeter& model, TagLib::ID3v2::PopularimeterFrame* frame) override;
	};
}

#endif // SAYONARA_ID3V2_POPULARIMETER_H
