/* LoginThread.h */

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

#ifndef SAYONARA_LASTFM_LOGIN_THREAD_H
#define SAYONARA_LASTFM_LOGIN_THREAD_H

#include <QObject>
#include "Utils/globals.h"
#include "Utils/Pimpl.h"

namespace LastFM
{
	struct LoginInfo
	{
		QString name;
		QString key;
		QString error;
		int errorCode {0};
		bool hasError {false};

		[[nodiscard]] bool isKeyValid() const { return (key.size() >= 32); }

		[[nodiscard]] bool isLoggedIn() const { return isKeyValid() && !hasError; }
	};

	class LoginThread :
		public QObject
	{
		Q_OBJECT
		PIMPL(LoginThread)

		signals:
			void sigFinished();

		public:
			explicit LoginThread(QObject* parent = nullptr);
			~LoginThread() override;

			void login(const QString& username, const QString& password);

			[[nodiscard]] LoginInfo loginInfo() const;

		private slots:
			void webClientFinished();
	};
}

#endif // SAYONARA_LASTFM_LOGIN_THREAD_H
