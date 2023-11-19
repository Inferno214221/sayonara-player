/* Broadcaster.h */

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

#ifndef BROADCASTER_H
#define BROADCASTER_H

#include "Utils/Pimpl.h"
#include <gst/gst.h>

namespace PipelineExtensions
{
	class BroadcastDataReceiver
	{
		public:
			virtual ~BroadcastDataReceiver() = default;
			virtual void setRawData(const QByteArray& data) = 0;
	};

	/**
	 * @brief The Broadcaster class
	 * @ingroup EngineInterfaces
	 */
	class BroadcastBin
	{
		PIMPL(BroadcastBin)

		public:
			BroadcastBin(PipelineExtensions::BroadcastDataReceiver* broadcastDataReceiver, GstElement* pipeline,
			             GstElement* tee);
			virtual ~BroadcastBin();

			bool init();
			bool setEnabled(bool b);
			bool isEnabled() const;
	};
}

#endif // BROADCASTER_H
