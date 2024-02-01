/* WebClient.h */

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

#ifndef SAYONARA_PLAYER_WEBCLIENT_H
#define SAYONARA_PLAYER_WEBCLIENT_H

#include <QByteArray>
#include <QObject>
#include <QString>

class WebClient :
	public QObject
{
	Q_OBJECT

	signals:
		void sigFinished();
		void sigStopped();

	public:
		static constexpr const auto Timeout = 4000;

		enum class Mode :
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

		explicit WebClient(QObject* parent);
		~WebClient() override;

		[[nodiscard]] virtual QByteArray data() const = 0;
		[[nodiscard]] virtual bool hasData() const = 0;
		[[nodiscard]] virtual QByteArray errorData() const = 0;

		[[nodiscard]] virtual QString url() const = 0;

		[[nodiscard]] virtual Status status() const = 0;
		[[nodiscard]] virtual bool hasError() const = 0;

		virtual void setMode(Mode mode) = 0;
		virtual void setRawHeader(const QMap<QByteArray, QByteArray>& header) = 0;

		virtual void run(const QString& url, int timeout = Timeout) = 0; // NOLINT(google-default-arguments)
		virtual void runPost(const QString& url, const QByteArray& postData,
		                     int timeout = Timeout) = 0; // NOLINT(google-default-arguments)

	public slots: // NOLINT(readability-redundant-access-specifiers)
		virtual void stop() = 0;
};

#endif // ABSTRACTWEBACCESS_H

