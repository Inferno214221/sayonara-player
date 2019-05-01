/* LastFM.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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


/*
 * LastFM.h
 *
 *  Created on: Apr 19, 2011
 *      Author: Lucio Carreras
 */

#ifndef LASTFM_H_
#define LASTFM_H_

#include "Utils/Pimpl.h"

#include <QObject>

// singleton base LastFM API class
// signals and slots are handled by the adapter class
namespace LastFM
{
	class Base :
			public QObject
	{
		Q_OBJECT
		PIMPL(Base)

		signals:
			void sig_logged_in(bool);

		private slots:
			void lfm_active_changed();
			void login_thread_finished(bool success);
			void similar_artists_fetched(IdList artist_ids);
			void current_track_changed(const MetaData& md);
			void position_ms_changed(MilliSeconds pos_ms);
			void scrobble_response_received(const QByteArray& data);
			void scrobble_error_received(const QString& str);

		public:
			Base();
			virtual ~Base();

			void login(const QString& username, const QString& password);
			bool is_logged_in();

		private:
			bool init_track_changed_thread();
			void get_similar_artists(const QString& artist);

			void reset_scrobble();
			bool check_scrobble(MilliSeconds pos_ms);
			void scrobble(const MetaData& md);
			bool update_track(const MetaData& md);
	};
}

#endif /* LASTFM_H_ */
