/* LFMTrackChangedThread.h

 * Copyright (C) 2011-2019 Lucio Carreras
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
 * created by Lucio Carreras,
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
		void sig_similar_artists_available(const IdList& artist_ids);


	public:
		explicit TrackChangedThread(QObject* parent=nullptr);
		~TrackChangedThread();

		void search_similar_artists(const MetaData& md);
		void update_now_playing(const QString& session_key, const MetaData& md);


	private:
		void evaluate_artist_match(const ArtistMatch& artist_match);

		QMap<QString, int> filter_available_artists(const ArtistMatch& artist_match, ArtistMatch::Quality quality);


	private slots:
		void response_sim_artists(const QByteArray& data);
		void error_sim_artists(const QString& error);

		void response_update(const QByteArray& response);
		void error_update(const QString& error);
	};
}
#endif /* LFMTRACKCHANGEDTHREAD_H_ */
