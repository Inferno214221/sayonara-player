/* Shutdown.h */

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

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include "Utils/Macros.h"
#ifdef SAYONARA_WITH_SHUTDOWN

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

/**
 * @brief The Shutdown class
 * @ingroup Helper
 */
class Shutdown : public QObject
{
	Q_OBJECT
	SINGLETON_QOBJECT(Shutdown)
	PIMPL(Shutdown)

signals:
	void sig_time_to_go(MilliSeconds ms);
	void sig_started(MilliSeconds ms);
	void sig_stopped();

private slots:
	void timeout();
	void countdown_timeout();
	void playlist_finished();

public:
	bool is_running() const;
	void stop();
	void shutdown(MilliSeconds ms=0);
	void shutdown_after_end();
};

#endif // SAYONARA_WITH_SHUTDOWN

#endif // SHUTDOWN_H
