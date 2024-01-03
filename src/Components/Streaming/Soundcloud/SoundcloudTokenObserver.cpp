/* SoundcloudTokenObserver.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "SoundcloudTokenObserver.h"
#include "SoundcloudJsonParser.h"
#include "SoundcloudWebAccess.h"

#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/WebClientImpl.h"
#include "Utils/Settings/Settings.h"

#include <QString>
#include <QTimer>

namespace SC
{
	struct TokenObserver::Private
	{
		int tokenRetryDuration {0};
	};

	TokenObserver::TokenObserver(QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>()} {}

	TokenObserver::~TokenObserver() = default;

	void TokenObserver::start()
	{
		obtainToken();
	}

	void TokenObserver::obtainToken()
	{
		auto* webClient = new WebClientImpl(this);
		connect(webClient, &WebClient::sigFinished, this, &TokenObserver::tokenObtained);

		webClient->run(createLinkObtainToken());
	}

	void TokenObserver::tokenObtained()
	{
		auto* webClient = dynamic_cast<WebClient*>(sender());
		if(webClient->status() == WebClient::Status::GotData)
		{
			const auto token = QString::fromLocal8Bit(webClient->data());
			if(!token.isEmpty())
			{
				spLog(Log::Info, this) << "Soundcloud token obtained";

				m->tokenRetryDuration = 0;

				SetSetting(SetNoDB::Soundcloud_AuthToken, token);

				constexpr const auto milliSeconds = 20 * 60 * 1000;
				QTimer::singleShot(milliSeconds, this, &TokenObserver::obtainToken);
			}

			else
			{
				spLog(Log::Warning, this) << "Soundcloud token could not be obtained";

				constexpr const auto OneMinute = 60'000;
				constexpr const auto OneSecond = 1'000;
				m->tokenRetryDuration = std::max(OneMinute, m->tokenRetryDuration + OneSecond);

				QTimer::singleShot(m->tokenRetryDuration, this, &TokenObserver::obtainToken);
			}
		}

		webClient->deleteLater();
	}
}