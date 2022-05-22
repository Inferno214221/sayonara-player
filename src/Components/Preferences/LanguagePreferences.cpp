/* LanguagePreferences.cpp */
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
#include "LanguagePreferences.h"
#include "Utils/Logger/Logger.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"

#include <QRegExp>
#include <algorithm>

namespace
{
	constexpr const auto ClassName = "LanguagePreferences";

	bool checkLineForUpdate(const QString& currentLanguageCode, const QString& currentChecksum, const QString& line)
	{
		const auto splitted = line.split(QRegExp("\\s+"));
		if(splitted.size() != 2)
		{
			return false;
		}

		const auto languageCode = Util::Language::extractLanguageCode(splitted[1]);
		if(languageCode == currentLanguageCode)
		{
			const auto& checksum = splitted[0];
			return (currentChecksum != checksum);
		}

		return false;
	}

	bool checkDataForUpdate(const QString& currentLanguageCode, const QString& data)
	{
		const auto currentChecksum = Util::Language::getChecksum(currentLanguageCode);
		const auto isUpdateAvailable = Util::Algorithm::contains(data.split("\n"), [&](const auto& row) {
			return checkLineForUpdate(currentLanguageCode, currentChecksum, row);
		});

		if(!isUpdateAvailable)
		{
			spLog(Log::Info, ClassName) << "No need to update language " << currentLanguageCode;
			return false;
		}

		spLog(Log::Info, ClassName) << "Language update available for " << currentLanguageCode;

		return true;
	}

	bool replaceLanguageFile(const QString& languageCode, const QByteArray& data)
	{
		const auto filepath = Util::Language::getHomeTargetPath(languageCode);
		const auto written = Util::File::writeFile(data, filepath);
		if(written)
		{
			spLog(Log::Info, ClassName) << "Language file written to " << filepath;

			Util::Language::updateLanguageVersion(languageCode);

			Settings::instance()->shout<Set::Player_Language>();
		}

		else
		{
			spLog(Log::Warning, ClassName) << "Could not write language file to " << filepath;
		}

		return written;
	}
} // namespace

LanguagePreferences::LanguagePreferences(QObject* parent) :
	QObject {parent} {}

LanguagePreferences::~LanguagePreferences() = default;

auto LanguagePreferences::getAllLanguages() -> std::pair<QList<LanguagePreferences::LanguageData>, int>
{
	QList<LanguagePreferences::LanguageData> languageData;

	const auto playerLanguage = GetSetting(Set::Player_Language);
	const auto locales = Util::Language::availableLanguages();

	auto englishIndex = -1;
	auto currentIndex = -1;
	auto i = 0;
	for(auto it = locales.begin(); it != locales.end(); it++, i++)
	{
		const auto& languageCode = it.key();
		const auto& locale = it.value();
		const auto iconPath = Util::Language::getIconPath(languageCode);

		auto languageName = Util::stringToVeryFirstUpper(locale.nativeLanguageName().toCaseFolded());
		if(languageCode.startsWith("en", Qt::CaseInsensitive))
		{
			languageName = "English";
			englishIndex = i;
		}

		languageData << LanguageData {
			languageCode,
			languageName,
			iconPath
		};

		if(languageCode.toLower() == playerLanguage.toLower())
		{
			currentIndex = i;
		}
	}

	currentIndex = std::max(englishIndex, currentIndex);
	currentIndex = std::max(0, currentIndex);

	return std::make_pair(languageData, currentIndex);
}

void LanguagePreferences::checkForUpdate(const QString& languageCode)
{
	auto* awa = new AsyncWebAccess(this);
	const auto url = Util::Language::getChecksumHttpPath();

	connect(awa, &AsyncWebAccess::sigFinished, this, [this, awa, languageCode]() {
		updateCheckFinished(awa, languageCode);
	});
	awa->run(url);
}

void LanguagePreferences::updateCheckFinished(AsyncWebAccess* awa, const QString& languageCode)
{
	const auto data = QString::fromUtf8(awa->data());
	const auto hasError = awa->hasError();

	awa->deleteLater();

	if(hasError || data.isEmpty())
	{
		emit sigWarning(tr("Cannot check for language update"));
		spLog(Log::Warning, this) << "Cannot download checksums " << awa->url();
		return;
	}

	const auto canUpdate = checkDataForUpdate(languageCode, data);
	if(canUpdate)
	{
		downloadUpdate(languageCode);
	}

	else
	{
		emit sigInfo(tr("Language is up to date"));
	}
}

void LanguagePreferences::downloadUpdate(const QString& languageCode)
{
	const auto url = Util::Language::getHttpPath(languageCode);

	auto* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, [this, awa, languageCode]() {
		downloadFinished(awa, languageCode);
	});
	awa->run(url);
}

void LanguagePreferences::downloadFinished(AsyncWebAccess* awa, const QString& languageCode)
{
	const auto data = awa->data();
	const auto hasError = awa->hasError();

	awa->deleteLater();

	if(hasError || data.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot download file from " << awa->url();
		emit sigWarning(tr("Cannot fetch language update"));
		return;
	}

	const auto success = replaceLanguageFile(languageCode, data);
	if(success)
	{
		emit sigInfo(tr("Language was updated successfully") + ".");
	}
	else
	{
		emit sigWarning(QObject::tr("Cannot fetch language update"));
	}
}

QString LanguagePreferences::importLanguage(const QString& filename)
{
	if(filename.isEmpty())
	{
		return {};
	}

	const auto success = Util::Language::importLanguageFile(filename);
	if(!success)
	{
		emit sigWarning(tr("The language file could not be imported"));
		return {};
	}

	auto newLanguageCode = Util::Language::extractLanguageCode(filename);
	emit sigInfo(tr("The language file was imported successfully"));

	return newLanguageCode;
}
