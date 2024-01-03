/* VersionChecker.cpp */

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

#include "VersionChecker.h"

#include "Utils/Utils.h"
#include "Utils/WebAccess/WebClientImpl.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/Style.h"

VersionChecker::VersionChecker(QObject* parent) :
	QObject(parent)
{
	constexpr const auto* Url = "http://sayonara-player.com/current_version";
	auto* webClient = new WebClientImpl(this);
	webClient->run(Url);
	connect(webClient, &WebClient::sigFinished, this, &VersionChecker::versionCheckFinished);
}

VersionChecker::~VersionChecker() = default;

void VersionChecker::versionCheckFinished()
{
	auto* webClient = dynamic_cast<WebClient*>(sender());
	if(!webClient)
	{
		return;
	}

	const auto status = webClient->status();
	const auto data = webClient->data();

	webClient->deleteLater();

	if(status != WebClient::Status::GotData || data.isEmpty())
	{
		return;
	}

	const auto newVersion = QString(data).trimmed();
	const auto curVersion = GetSetting(Set::Player_Version);

	spLog(Log::Info, this) << "Newest Version: " << newVersion;
	spLog(Log::Info, this) << "This Version:   " << curVersion;

	if(GetSetting(Set::Player_NotifyNewVersion))
	{
		if(newVersion > curVersion)
		{
			Message::info(tr("A new version is available!") + "<br />" +
			              Util::createLink("http://sayonara-player.com", Style::isDark()));
		}
	}

	emit sigFinished();
}
