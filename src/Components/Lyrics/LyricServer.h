/* LyricServer.h */

/* Copyright (C) 2012  Lucio Carreras
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

#ifndef LYRICSERVER_H_
#define LYRICSERVER_H_

#include <QString>
#include <QMap>

namespace Lyrics
{
	/**
	 * @brief The ServerTemplate struct
	 * @ingroup Lyrics
	 */
	class Server
	{
		public:
			virtual QString name() const=0;
			virtual QString address() const=0;
			virtual QMap<QString, QString> replacements() const=0;
			virtual QString call_policy() const=0;
			virtual QMap<QString, QString> start_end_tag() const=0;
			virtual bool is_start_tag_included() const=0;
			virtual bool is_end_tag_included() const=0;
			virtual bool is_numeric() const=0;
			virtual bool is_lowercase() const=0;
			virtual QString error_string() const=0;

			virtual bool can_fetch_directly() const;
			virtual bool can_search() const;

			/**
			 * @brief return the url to the search site
			 * @param artist
			 * @param title
			 * @return
			 */
			virtual QString search_address(QString artist, QString title) const;

			/**
			 * @brief Parse the result page and return the real address
			 * @param search_result website appearing after search was triggered
			 * @return
			 */
			virtual QString parse_search_result(const QString& search_result);
	};

	class SearchableServer : public Server
	{
		public:
			QString address() const;
			QMap<QString, QString> replacements() const;
			QString call_policy() const;
			bool can_search() const;
	};
}



#endif /* LYRICSERVER_H_ */
