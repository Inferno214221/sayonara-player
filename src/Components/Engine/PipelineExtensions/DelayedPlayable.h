/* DelayedPlayHandler.h */

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

#ifndef DELAYEDPLAYHANDLER_H
#define DELAYEDPLAYHANDLER_H

#include "Utils/Pimpl.h"
#include <QObject>

namespace PipelineExtensions
{
	/**
	 * @brief The DelayedPlayable class
	 * @ingroup EngineInterfaces
	 */
	class DelayedPlayable
	{
		PIMPL(DelayedPlayable)

		public:
			DelayedPlayable();
			~DelayedPlayable();

		public:
			virtual void play()=0;

			void playIn(MilliSeconds ms);
			void abortDelayedPlaying();
	};


	/**
	 * @brief Pure private class. Only used by Delayed Pipeline
	 * @ingroup EngineInterfaces
	 */
	class DelayedPlayableLogic : public QObject
	{
		Q_OBJECT
		PIMPL(DelayedPlayableLogic)

		friend class DelayedPlayable;

		private:
			DelayedPlayableLogic(DelayedPlayable* dph);
			~DelayedPlayableLogic();

			void startTimer(MilliSeconds ms);
			void stopTimer();
	};
}

#endif // DELAYEDPLAYHANDLER_H
