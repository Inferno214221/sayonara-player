/* IcyWebAccess.h */

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

#ifndef ICYWEBACCESS_H
#define ICYWEBACCESS_H

#include "Utils/Pimpl.h"

#include <QObject>
#include <QAbstractSocket>

class QUrl;
class IcyWebAccess :
	public QObject
{
	Q_OBJECT
	PIMPL(IcyWebAccess)

	public:
		explicit IcyWebAccess(QObject* parent = nullptr);
		~IcyWebAccess() override;

		enum class Status :
			uint8_t
		{
			WriteError = 0,
			WrongAnswer,
			OtherError,
			NotExecuted,
			Success
		};

		[[nodiscard]] IcyWebAccess::Status status() const;
		void check(const QUrl& url);
		void stop();

	signals:
		void sigFinished();

	private slots:
		void connected();
		void disconnected();
		void errorReceived(QAbstractSocket::SocketError socketState);
		void dataAvailable();
};

#endif // ICYWEBACCESS_H
