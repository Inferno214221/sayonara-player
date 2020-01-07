/* Cover.h */

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



#ifndef XIPH_COVER_H
#define XIPH_COVER_H

#include "Utils/Tagging/Xiph/XiphFrame.h"
#include "Utils/Tagging/Models/Cover.h"
#include "XiphFrame.h"

/**
 * @ingroup Tagging
 */
namespace Xiph
{
	class CoverFrame :
		public XiphFrame<Models::Cover>
	{
		public:
			CoverFrame(TagLib::Ogg::XiphComment* tag);
			~CoverFrame() override;

			bool is_frame_found() const override;

		protected:
			bool map_tag_to_model(Models::Cover& model) override;
			bool map_model_to_tag(const Models::Cover& model) override;
	};
}

#endif // COVER_H
