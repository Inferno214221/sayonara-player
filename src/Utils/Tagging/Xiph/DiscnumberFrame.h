/* DiscnumberFrame.h */

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



#ifndef DISCNUMBERXIPHFRAME_H
#define DISCNUMBERXIPHFRAME_H

#include "Utils/Tagging/Models/Discnumber.h"
#include "XiphFrame.h"

/**
 * @ingroup Tagging
 */
namespace Xiph
{
	/**
	 * @brief The DiscnumberFrame class
	 * @ingroup Xiph
	 */
	class DiscnumberFrame :
		public Xiph::XiphFrame<Models::Discnumber>
	{
	public:
		DiscnumberFrame(TagLib::Ogg::XiphComment* tag);
		~DiscnumberFrame();

	protected:
		bool map_tag_to_model(Models::Discnumber& model);
		bool map_model_to_tag(const Models::Discnumber& model);
	};
}

#endif // DISCNUMBERFRAME_H
