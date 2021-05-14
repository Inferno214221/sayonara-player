/* VersionChecker.cpp */

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

#include "VersionChecker.h"

#include "Utils/Utils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/Style.h"

VersionChecker::VersionChecker(QObject* parent) :
	QObject(parent)
{
	auto* awa = new AsyncWebAccess(this);
	awa->run("http://sayonara-player.com/current_version");
	connect(awa, &AsyncWebAccess::sigFinished, this, &VersionChecker::versionCheckFinished);
}

VersionChecker::~VersionChecker() = default;

void VersionChecker::versionCheckFinished()
{
	auto* awa = dynamic_cast<AsyncWebAccess*>(sender());
	if(!awa)
	{
		return;
	}

	const auto status = awa->status();
	const auto data = awa->data();

	awa->deleteLater();

	if(status != AsyncWebAccess::Status::GotData || data.isEmpty())
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
