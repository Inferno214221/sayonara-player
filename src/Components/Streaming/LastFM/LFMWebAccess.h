/* WebAccess.h */

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


/*
 * WebAccess.h
 *
 *  Created on: Oct 22, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef SAYONARA_LASTFM_WEBACCESS_H
#define SAYONARA_LASTFM_WEBACCESS_H

#include <QObject>
#include <QMap>

class QByteArray;

namespace LastFM
{
	using UrlParams = QMap<QString, QString>;
	class WebAccess :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigFinished();
			void sigResponse(const QByteArray& response);
			void sigError(const QString& error);

		public:
			void callUrl(const QString& url);
			void callPostUrl(const QString& url, const QByteArray& postData);

		private:
			bool checkError(const QByteArray& data);

		private slots: // NOLINT(readability-redundant-access-specifiers)
			void webClientFinished();
	};

	QByteArray createPostData(UrlParams signatureData);
}
#endif /* SAYONARA_LASTFM_WEBACCESS_H */
