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
#include <QList>
#include <QPair>
#include <QMap>
#include "Utils/Pimpl.h"

class QJsonObject;

namespace Lyrics
{
	/**
	 * @brief The ServerTemplate struct
	 * @ingroup Lyrics
	 */
	class Server
	{
		PIMPL(Server)

		public:
			Server();
			~Server();

			using StartEndTag=QPair<QString, QString>;
			using StartEndTags=QList<StartEndTag>;
			using Replacement=QPair<QString, QString>;
			using Replacements=QList<Replacement>;

			bool can_fetch_directly() const;
			bool can_search() const;

			QString name() const;
			void set_name(const QString& name);

			QString address() const;
			void set_address(const QString& address);

			Replacements replacements() const;
			void set_replacements(const Replacements& replacements);

			QString direct_url_template() const;
			void set_direct_url_template(const QString& direct_url_template);

			StartEndTags start_end_tag() const;
			void set_start_end_tag(const StartEndTags& start_end_tag);

			bool is_start_tag_included() const;
			void set_is_start_tag_included(bool is_start_tag_included);

			bool is_end_tag_included() const;
			void set_is_end_tag_included(bool is_end_tag_included);

			bool is_numeric() const;
			void set_is_numeric(bool is_numeric);

			bool is_lowercase() const;
			void set_is_lowercase(bool is_lowercase);

			QString error_string() const;
			void set_error_string(const QString& error_string);

			QString search_result_regex() const;
			void set_search_result_regex(const QString& search_result_regex);

			QString search_result_url_template() const;
			void set_search_result_url_template(const QString& search_result_url_template);

			QString search_url_template() const;
			void set_search_url_template(const QString& search_url_template);

			QJsonObject to_json();
			static Lyrics::Server* from_json(const QJsonObject& json);

			static QString apply_replacements(const QString& str, const Server::Replacements& replacements);

		private:
			QString apply_replacements(const QString& str) const;
	};
}



#endif /* LYRICSERVER_H_ */
