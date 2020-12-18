/* LFMTrackChangedThread.h

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Michael Lugmair (Lucio Carreras),
 * Jul 18, 2012
 *
 */

#ifndef LFMTRACKCHANGEDTHREAD_H_
#define LFMTRACKCHANGEDTHREAD_H_

#include "DynamicPlayback/ArtistMatch.h"
#include "Utils/Pimpl.h"

#include <QObject>

class SmartCompare;

namespace LastFM
{
	class TrackChangedThread :
			public QObject
	{
		Q_OBJECT
		PIMPL(TrackChangedThread)

	public:
		explicit TrackChangedThread(QObject* parent=nullptr);
		~TrackChangedThread();

		void updateNowPlaying(const QString& sessionKey, const MetaData& md);

	private slots:
		void updateResponseReceived(const QByteArray& response);
		void updateErrorReceived(const QString& error);
	};
}
#endif /* LFMTRACKCHANGEDTHREAD_H_ */
