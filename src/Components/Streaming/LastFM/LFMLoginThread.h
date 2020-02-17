/* LoginThread.h */

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

#ifndef LoginThread_H
#define LoginThread_H

#include <QObject>
#include "Utils/globals.h"
#include "Utils/Pimpl.h"

namespace LastFM
{
    struct LoginStuff
    {
        QString		token;
        QString		sessionKey;
        bool		loggedIn;
        bool		subscriber;
        QString		error;

        LoginStuff()
        {
            loggedIn = false;
            subscriber = false;
        }
    };


    class LoginThread :
            public QObject
    {
        Q_OBJECT
        PIMPL(LoginThread)

    signals:
        void sigTokenReceived(const QString& token);
        void sigError(const QString& error);
        void sigLoggedIn(bool success);

    public:
        explicit LoginThread(QObject* parent=nullptr);
        ~LoginThread();

        void login(const QString& username, const QString& password);

        LoginStuff getLoginStuff();

    private slots:
        void webaccessResponseReceived(const QByteArray& data);
        void webaccessErrorReceived(const QString& response);
    };
}

#endif // LoginThread_H
