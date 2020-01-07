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

#ifndef MP4COVER_H
#define MP4COVER_H

#include "MP4Frame.h"
#include "Utils/Tagging/Models/Cover.h"

#include "taglib/mp4coverart.h"
#include "taglib/tag.h"

namespace MP4
{
	class CoverFrame :
			public MP4Frame<Models::Cover>
	{
	public:
		explicit CoverFrame(TagLib::MP4::Tag* tag);
		~CoverFrame() override;


		// AbstractFrame interface
	protected:
		bool map_tag_to_model(Models::Cover& model) override;
		bool map_model_to_tag(const Models::Cover& model) override;
	};
}
#endif // MP4COVER_H
