/* DelayedPlayHandler.h */

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

#ifndef DELAYEDPLAYHANDLER_H
#define DELAYEDPLAYHANDLER_H

#include "Utils/Pimpl.h"
#include <QObject>

namespace PipelineExtensions
{
	class DelayedPlayHandler
	{
		PIMPL(DelayedPlayHandler)

		public:
			DelayedPlayHandler();
			~DelayedPlayHandler();

		public:
			virtual void play()=0;

			void play_in(MilliSeconds ms);
			void abort_delayed_playing();
	};

	class Logic : public QObject
	{
		Q_OBJECT
		PIMPL(Logic)

		friend class DelayedPlayHandler;

		private:
			Logic(DelayedPlayHandler* dph);
			~Logic();

			void start_timer(MilliSeconds ms);
			void stop_timer();
	};
}

#endif // DELAYEDPLAYHANDLER_H
