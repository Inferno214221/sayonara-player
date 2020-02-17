/* Discnumber.h */

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

#ifndef DISCNUMBER_H
#define DISCNUMBER_H

#include "ID3v2Frame.h"
#include "Utils/Tagging/Models/Discnumber.h"

#include "taglib/textidentificationframe.h"

namespace ID3v2
{
	/**
	 * @brief The DiscnumberFrame class
	 * @ingroup ID3v2
	 */
	class DiscnumberFrame :
			public ID3v2Frame<Models::Discnumber, TagLib::ID3v2::TextIdentificationFrame>
	{
		public:
			DiscnumberFrame(TagLib::ID3v2::Tag* tag);
			~DiscnumberFrame() override;

			void map_model_to_frame(const Models::Discnumber& model, TagLib::ID3v2::TextIdentificationFrame* frame) override;
			void map_frame_to_model(const TagLib::ID3v2::TextIdentificationFrame* frame, Models::Discnumber& model) override;

			TagLib::ID3v2::Frame* create_id3v2_frame() override;
	};
}

#endif // DISCNUMBER_H
