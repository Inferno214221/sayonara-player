/* Broadcaster.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef SAYONARA_ENGINE_BROADCASTER_H
#define SAYONARA_ENGINE_BROADCASTER_H

#include <gst/gst.h>
#include <functional>
#include <memory>

class QByteArray;
namespace PipelineExtensions
{
	class RawDataReceiver
	{
		public:
			virtual ~RawDataReceiver();
			virtual void setRawData(const QByteArray& data) = 0;
	};

	using RawDataReceiverPtr = std::shared_ptr<RawDataReceiver>;

	class Broadcastable
	{
		public:
			virtual ~Broadcastable();

			virtual void setBroadcastingEnabled(bool b) = 0;
			[[nodiscard]] virtual bool isBroadcastingEnabled() const = 0;
	};

	class Broadcaster
	{
		public:
			virtual ~Broadcaster();

			virtual bool setEnabled(bool b) = 0;
			[[nodiscard]] virtual bool isEnabled() const = 0;
	};

	std::shared_ptr<Broadcaster> createBroadcaster(const RawDataReceiverPtr& broadcastDataReceiver,
	                                               GstElement* pipeline,
	                                               GstElement* tee);

	RawDataReceiverPtr createRawDataReceiver(std::function<void(const QByteArray&)>&& callback);
}

#endif // SAYONARA_ENGINE_BROADCASTER_H
