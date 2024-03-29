/* WebAccess.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
 * WebAccess.h
 *
 *  Created on: Oct 22, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef SAYONARA_LASTFM_WEBACCESS_H
#define SAYONARA_LASTFM_WEBACCESS_H

#include "Utils/Pimpl.h"

#include <QObject>
#include <QMap>

class QByteArray;
class QString;

class WebClientFactory;
class WebClient;
namespace LastFM
{
	using UrlParams = QMap<QString, QString>;
	class WebAccess :
		public QObject
	{
		Q_OBJECT
		PIMPL(WebAccess)

		signals:
			void sigFinished();

		public:
			explicit WebAccess(const std::shared_ptr <WebClientFactory>& webClientFactory, QObject* parent = nullptr);
			~WebAccess() override;

			void callUrl(const QString& url);
			void callPostUrl(const QString& url, const QByteArray& postData);

			[[nodiscard]] QByteArray data() const;

		private:
			WebClient* initWebClient();

		private slots: // NOLINT(readability-redundant-access-specifiers)
			void webClientFinished();
	};

	QByteArray createPostData(UrlParams signatureData);
}
#endif /* SAYONARA_LASTFM_WEBACCESS_H */
