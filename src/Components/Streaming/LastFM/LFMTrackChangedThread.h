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

#include "ArtistMatch.h"
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

	signals:
		void sigSimilarArtistsAvailable(const IdList& artistIds);

	public:
		explicit TrackChangedThread(QObject* parent=nullptr);
		~TrackChangedThread();

		void searchSimilarArtists(const MetaData& md);
		void updateNowPlaying(const QString& sessionKey, const MetaData& md);

	private:
		void evaluateArtistMatch(const ArtistMatch& artistMatch);
		QMap<QString, int> filterAvailableArtists(const ArtistMatch& artistMatch, ArtistMatch::Quality quality);

	private slots:
		void similarArtistResponseReceived(const QByteArray& data);
		void similarArtistsErrorReceived(const QString& error);

		void updateResponseReceived(const QByteArray& response);
		void updateErrorReceived(const QString& error);
	};
}
#endif /* LFMTRACKCHANGEDTHREAD_H_ */
