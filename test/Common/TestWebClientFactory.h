/* ${CLASS_NAME}.h */
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
#ifndef SAYONARA_PLAYER_TESTWEBCLIENTFACTORY_H
#define SAYONARA_PLAYER_TESTWEBCLIENTFACTORY_H

#include "Utils/WebAccess/WebClientFactory.h"
#include "Utils/WebAccess/WebClient.h"

#include <QByteArray>
#include <QList>

class TestWebClient :
	public WebClient
{
	public:
		explicit TestWebClient(QObject* parent) :
			WebClient(parent) {}

		~TestWebClient() override = default;

		[[nodiscard]] QByteArray data() const override
		{
			return mData;
		}

		[[nodiscard]] bool hasData() const override
		{
			return (!mData.isEmpty());
		}

		[[nodiscard]] QString url() const override
		{
			return mUrl;
		}

		[[nodiscard]] Status status() const override
		{
			return mStatus;
		}

		[[nodiscard]] bool hasError() const override
		{
			return mHasError;
		}

		void setMode(const Mode /*mode*/) override {}

		void setRawHeader(const QMap<QByteArray, QByteArray>& /*header*/) override {}

		void run(const QString& url, int /*timeout*/) override
		{
			mHasError = false;
			mUrl = url;
		}

		void runPost(const QString& url, const QByteArray& /*postData*/, int /*timeout*/) override
		{
			mHasError = false;
			mUrl = url;
		}

		void stop() override {}

		void fireData(const QByteArray& data, const WebClient::Status status = WebClient::Status::NoError)
		{
			mData = data;
			mHasError = false;
			mStatus = status;
			emit sigFinished();
		}

		[[maybe_unused]] void fireError()
		{
			mData.clear();
			mHasError = true;
			mStatus = WebClient::Status::Error;
			emit sigFinished();
		}

		[[maybe_unused]] void fireTimeout()
		{
			mData.clear();
			mHasError = false;
			mStatus = WebClient::Status::Timeout;
			emit sigFinished();
		}

	private:
		QByteArray mData;
		QString mUrl;
		WebClient::Status mStatus {WebClient::Status::NoData};
		bool mHasError {false};
};

class TestWebClientFactory :
	public WebClientFactory
{
	public:
		TestWebClientFactory() = default;
		~TestWebClientFactory() override = default;

		WebClient* createClient(QObject* parent) override
		{
			auto* client = new TestWebClient(parent);
			mClients << client;
			return client;
		}

		[[nodiscard]] QList<TestWebClient*> clients() const
		{
			return mClients;
		}

		[[nodiscard]] TestWebClient* clientByUrl(const QString& url)
		{
			const auto it = std::find_if(mClients.begin(), mClients.end(), [&](const auto* client) {
				return (client->url() == url);
			});

			return (it != mClients.end()) ? *it : nullptr;
		}

	private:
		QList<TestWebClient*> mClients;
};

#endif //SAYONARA_PLAYER_TESTWEBCLIENTFACTORY_H
