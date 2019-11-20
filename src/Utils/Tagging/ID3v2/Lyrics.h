/* Lyrics.h */

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



#ifndef LYRICS_FRAME_H
#define LYRICS_FRAME_H

#include <QString>
#include "ID3v2Frame.h"
#include "taglib/unsynchronizedlyricsframe.h"

namespace ID3v2
{
	class LyricsFrame :
			public ID3v2Frame<QString, TagLib::ID3v2::UnsynchronizedLyricsFrame>
	{
		public:
			explicit LyricsFrame(TagLib::ID3v2::Tag* tag);
			~LyricsFrame() override;

			void map_model_to_frame(const QString& model, TagLib::ID3v2::UnsynchronizedLyricsFrame* frame) override;
			void map_frame_to_model(const TagLib::ID3v2::UnsynchronizedLyricsFrame* frame, QString& model) override;

			TagLib::ID3v2::Frame* create_id3v2_frame() override;
	};
}

#endif // LYRICS_FRAME_H
