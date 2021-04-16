/* Cover.h */

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

#ifndef SAYONARA_ID3V2_COVER_H
#define SAYONARA_ID3V2_COVER_H

#include "ID3v2Frame.h"
#include "Utils/Tagging/Models/Cover.h"

#include <QByteArray>
#include <QString>

#include <taglib/attachedpictureframe.h>

namespace ID3v2
{
	/**
	 * @brief The DiscnumberFrame class
	 * @ingroup ID3v2
	 */
	class CoverFrame :
			public ID3v2Frame<Models::Cover, TagLib::ID3v2::AttachedPictureFrame>
	{
		public:
			CoverFrame(TagLib::ID3v2::Tag* tag);
			~CoverFrame() override;

			void map_model_to_frame(const Models::Cover& model, TagLib::ID3v2::AttachedPictureFrame* frame) override;
			void map_frame_to_model(const TagLib::ID3v2::AttachedPictureFrame* frame, Models::Cover& model) override;

			TagLib::ID3v2::Frame* create_id3v2_frame() override;
	};
}

#endif // SAYONARA_ID3V2_COVER_H
