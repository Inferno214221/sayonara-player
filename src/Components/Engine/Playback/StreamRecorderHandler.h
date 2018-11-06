/* StreamRecorderHandler.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#ifndef STREAMRECORDERHANDLER_H
#define STREAMRECORDERHANDLER_H

#include "Utils/Pimpl.h"
#include "Components/Engine/gstfwd.h"

namespace StreamRecorder
{
	struct Data;
}

namespace Pipeline
{
	class StreamRecorderHandler
	{
		PIMPL(StreamRecorderHandler)

	public:
		StreamRecorderHandler();
		virtual ~StreamRecorderHandler();

		void set_streamrecorder_target_path(const QString& path);
		StreamRecorder::Data* streamrecorder_data() const;

		virtual GstElement* get_streamrecorder_sink_element() const=0;
	};
}

#endif // STREAMRECORDERHANDLER_H
