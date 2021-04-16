/* PopularimeterFrame.h */

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

#ifndef SAYONARA_XIPH_POPULARIMETER_H
#define SAYONARA_XIPH_POPULARIMETER_H

#include "Utils/Tagging/Xiph/XiphFrame.h"
#include "Utils/Tagging/Models/Popularimeter.h"

/**
 * @ingroup Tagging
 */
namespace Xiph
{
	/**
	 * @brief The PopularimeterFrame class
	 * @ingroup Xiph
	 */
	class PopularimeterFrame :
			public XiphFrame<Models::Popularimeter>
	{
	public:
		PopularimeterFrame(TagLib::Ogg::XiphComment* tag);
		~PopularimeterFrame() override;

	protected:
		bool map_tag_to_model(Models::Popularimeter& model) override;
		bool map_model_to_tag(const Models::Popularimeter& model) override;
	};
}

#endif // SAYONARA_XIPH_POPULARIMETER_H
