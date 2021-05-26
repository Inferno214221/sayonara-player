/* AsyncWebAccess.h */

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

#ifndef SAYONARA_ASYNC_WEB_ACCESS_H
#define SAYONARA_ASYNC_WEB_ACCESS_H

#include "AbstractWebAccess.h"
#include "Utils/Pimpl.h"

#include <QObject>

class QImage;

/**
 * @brief Asynchgronous web access class
 * @ingroup Helper
 */
class AsyncWebAccess :
	public QObject,
	public AbstractWebAccess
{
	Q_OBJECT
	PIMPL(AsyncWebAccess)

	public:
		enum class Behavior :
			uint8_t
		{
				AsBrowser = 0,
				AsSayonara,
				Random,
				None
		};

		enum class Status :
			uint8_t
		{
				NoError = 0,
				GotData,
				AudioStream,
				NoData,
				NoHttp,
				NotFound,
				Timeout,
				Error
		};

	signals:
		void sigFinished();
		void sigStopped();

	public:
		explicit AsyncWebAccess(QObject* parent = nullptr,
		                        AsyncWebAccess::Behavior behavior = AsyncWebAccess::Behavior::AsBrowser);

		virtual ~AsyncWebAccess() override;

		QByteArray data() const;
		bool hasData() const;

		QImage image() const;
		QString url() const;

		AsyncWebAccess::Status status() const;
		bool hasError() const;

		void setBehavior(AsyncWebAccess::Behavior behavior);
		void setRawHeader(const QMap<QByteArray, QByteArray>& header);

		void run(const QString& url, int timeout = 4000);
		void runPost(const QString& url, const QByteArray& postData, int timeout = 4000);

	public slots:
		void stop() override;

	private slots:
		void dataAvailable();
		void finished();
		void timeout();
};

class AsyncWebAccessStopper :
	public QObject
{
		friend class AsyncWebAccess;

	Q_OBJECT
	PIMPL(AsyncWebAccessStopper)

	signals:
		void sigFinished();
		void sigTimeout();
		void sigStopped();

	private:
		explicit AsyncWebAccessStopper(QObject* parent);
		~AsyncWebAccessStopper() noexcept;

	private slots:
		void startTimer(int timeout);
		void stopTimer();
		void timeout();
		void stop();
};

#endif // SAYONARA_ASYNC_WEB_ACCESS_H
