/* Composer.h */

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

#ifndef SAYONARA_ID3V2_COMPOSER_H
#define SAYONARA_ID3V2_COMPOSER_H

#include "ID3v2Frame.h"

namespace ID3v2
{
	/**
	 * @brief The AlbumArtistFrame class
	 * @ingroup ID3v2
	 */
	class ComposerFrame :
			public ID3v2Frame<QString, TagLib::ID3v2::TextIdentificationFrame>
	{
		public:
			explicit ComposerFrame(TagLib::ID3v2::Tag* tag);
			~ComposerFrame() override;

		protected:
			TagLib::ID3v2::Frame* create_id3v2_frame() override;

			void map_model_to_frame(const QString& model, TagLib::ID3v2::TextIdentificationFrame* frame) override;
			void map_frame_to_model(const TagLib::ID3v2::TextIdentificationFrame* frame, QString& model) override;
	};
}

#endif // SAYONARA_ID3V2_COMPOSER_H
