/* WebClientImpl.h */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_WEBCLIENTIMPL_H
#define SAYONARA_PLAYER_WEBCLIENTIMPL_H

#include "WebClient.h"
#include "Utils/Pimpl.h"

class AbstractWebClientStopper :
	public QObject
{
	Q_OBJECT
	PIMPL(AbstractWebClientStopper)

	signals:
		void sigFinished();
		void sigTimeout();
		void sigStopped();

	public:
		explicit AbstractWebClientStopper(QObject* parent);
		~AbstractWebClientStopper() noexcept override;

		void startTimer(int timeout);
		void stopTimer();
		void stop();

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void timeout();
};

class WebClientImpl :
	public WebClient
{
	Q_OBJECT
	PIMPL(WebClientImpl)

	public:
		explicit WebClientImpl(QObject* parent);
		~WebClientImpl() override;

		void run(const QString& url, int timeout=WebClient::Timeout) override;
		void runPost(const QString& url, const QByteArray& postData, int timeout) override;

		void setMode(WebClientImpl::Mode mode) override;
		void setRawHeader(const QMap<QByteArray, QByteArray>& header) override;

		[[nodiscard]] bool hasData() const override;
		[[nodiscard]] QByteArray data() const override;
		[[nodiscard]] QString url() const override;
		[[nodiscard]] WebClientImpl::Status status() const override;
		[[nodiscard]] bool hasError() const override;

	public slots: // NOLINT(readability-redundant-access-specifiers)
		void stop() override;

	private slots:
		void dataAvailable();
		void finished();
		void timeout();

	private:
		void reset();
};

#endif //SAYONARA_PLAYER_WEBCLIENTIMPL_H
