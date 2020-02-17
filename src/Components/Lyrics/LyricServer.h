/* LyricServer.h */

/* Copyright (C) 2012 Michael Lugmair (Lucio Carreras)
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

			bool canFetchDirectly() const;
			bool canSearch() const;

			QString name() const;
			void setName(const QString& name);

			QString address() const;
			void setAddress(const QString& address);

			Replacements replacements() const;
			void setReplacements(const Replacements& replacements);

			QString directUrlTemplate() const;
			void setDirectUrlTemplate(const QString& directUrlTemplate);

			StartEndTags startEndTag() const;
			void setStartEndTag(const StartEndTags& startEndTag);

			bool isStartTagIncluded() const;
			void setIsStartTagIncluded(bool isStartTagIncluded);

			bool isEndTagIncluded() const;
			void setIsEndTagIncluded(bool isEndTagIncluded);

			bool isNumeric() const;
			void setIsNumeric(bool isNumeric);

			bool isLowercase() const;
			void setIsLowercase(bool isLowercase);

			QString errorString() const;
			void setErrorString(const QString& errorString);

			QString searchResultRegex() const;
			void setSearchResultRegex(const QString& searchResultRegex);

			QString searchResultUrlTemplate() const;
			void setSearchResultUrlTemplate(const QString& searchResultUrlTemplate);

			QString searchUrlTemplate() const;
			void setSearchUrlTemplate(const QString& searchUrlTemplate);

			QJsonObject toJson();
			static Lyrics::Server* fromJson(const QJsonObject& json);

			static QString applyReplacements(const QString& str, const Server::Replacements& replacements);

		private:
			QString applyReplacements(const QString& str) const;
	};
}



#endif /* LYRICSERVER_H_ */
