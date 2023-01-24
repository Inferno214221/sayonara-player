/* Shutdown.h */

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

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include "Utils/Macros.h"
#include "Utils/Pimpl.h"

#include <QObject>

class PlayManager;
class Shutdown :
	public QObject
{
	Q_OBJECT

	signals:
		void sigTimeToGoChanged(MilliSeconds ms);
		void sigStarted(MilliSeconds ms);
		void sigStopped();

	public:
		static Shutdown* create(PlayManager* playManager);

		[[nodiscard]] virtual bool isRunning() const = 0;
		virtual void stop() = 0;
		virtual void shutdown(MilliSeconds ms = 0) = 0;
		virtual void shutdownAfterSessionEnd() = 0;
};

#endif // SHUTDOWN_H
