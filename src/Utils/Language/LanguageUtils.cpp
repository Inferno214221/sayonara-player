/* LanguageUtils.cpp */

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

#include "LanguageUtils.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"

#include <QFile>
#include <QRegExp>
#include <QDir>
#include <QMap>
#include <QSettings>
#include <QLocale>
#include <QLibraryInfo>

static bool s_test_mode = false;

class LanguageVersionHelper
{
		QSettings* mSettings = nullptr;

		LanguageVersionHelper()
		{
			const auto filepath = (s_test_mode == false)
			                ? QDir(Util::translationsPath()).absoluteFilePath("versions")
			                : Util::tempPath("versions");

			mSettings = new QSettings(filepath, QSettings::NativeFormat);
		}

	public:
		~LanguageVersionHelper()
		{
			delete mSettings;
		}

		static LanguageVersionHelper* instance()
		{
			static LanguageVersionHelper hlp;
			return &hlp;
		}

		void setLanguageVersion(const QString& languageCode, const QString& version)
		{
			// never allow to downgrade a downloaded version
			if(isOutdated(languageCode))
			{
				mSettings->setValue(languageCode, version);
			}
		}

		QString getLanguageVersion(const QString& languageCode)
		{
			return mSettings->value(languageCode).toString();
		}

		bool isOutdated(const QString& languageCode)
		{
			QString lv = getLanguageVersion(languageCode);
			QString pv = GetSetting(::Set::Player_Version);

			if(lv.isEmpty())
			{
				return true;
			}

			return (lv < pv);
		}
};

namespace Language = Util::Language;

static bool checkLanguageCode(const QString& languageCode)
{
	QRegExp re("^[a-z]{2}(_[A-Z]{2})?(\\.[A-Z0-9\\-]+[0-9])?$");
	int idx = re.indexIn(languageCode);

	return (idx == 0);
}

QString Language::getSharePath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}
	return Util::sharePath("translations") + "/" + QString("sayonara_lang_%1.qm").arg(languageCode);
}

QString Language::getFtpPath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}
	return QString("ftp://sayonara-player.com/translation/sayonara_lang_%1.qm").arg(languageCode);
}

QString Language::getHttpPath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}

	return QString("https://sayonara-player.com/files/translation/sayonara_lang_%1.qm").arg(
		languageCode);
}

QString Language::getChecksumFtpPath()
{
	return "ftp://sayonara-player.com/translation/checksum";
}

QString Language::getChecksumHttpPath()
{
	return "https://sayonara-player.com/files/translation/checksum";
}

QString Language::getHomeTargetPath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}

	const auto translationDir = Util::translationsPath();
	const auto languagePath = QString("%1/sayonara_lang_%2.qm")
		.arg(translationDir)
		.arg(languageCode);

	return languagePath;
}

bool Language::isOutdated(const QString& languageCode)
{
	return LanguageVersionHelper::instance()->isOutdated(languageCode);
}

QString Language::getSimilarLanguage4(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}

	const auto twoLetter = languageCode.left(2);
	const auto translationsPaths = QStringList
		{
			Util::translationsPath(),
			Util::sharePath("translations")
		};

	for(const auto& translationPath : translationsPaths)
	{
		if(!Util::File::exists(translationPath))
		{
			continue;
		}

		const auto dir = QDir(translationPath);
		const auto entries = dir.entryList(QDir::Files);
		const auto re = QRegExp("sayonara_lang_([a-z]{2})_.+qm");

		for(const auto& entry : entries)
		{
			if(re.indexIn(entry) < 0)
			{
				continue;
			}

			const auto entryTwoLetter = re.cap(1);
			if(entryTwoLetter == twoLetter)
			{
				return translationPath + "/" + entry;
			}
		}
	}

	return QString();
}

QString Language::getUsedLanguageFile(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}

	{ // check if home path or share path version is better or exists
		if(isOutdated(languageCode)) // not available or older than in share path
		{
			const auto languageInSharePath = getSharePath(languageCode);
			if(Util::File::exists(languageInSharePath))
			{
				return languageInSharePath;
			}
		}

		if(Util::File::exists(getHomeTargetPath(languageCode)))
		{
			return getHomeTargetPath(languageCode);
		}
	}

	{ // try to find from other region
		return getSimilarLanguage4(languageCode);
	}
}

QString Language::extractLanguageCode(const QString& languageFile)
{
	const auto re = QRegExp
		(
			".*sayonara_lang_"
			"([a-z]{2}(_[A-Z]{2})?(\\.[A-Z0-9\\-]+[0-9])?)\\.(ts|qm)$"
		);

	const auto idx = re.indexIn(languageFile);
	if(idx < 0)
	{
		return QString();
	}

	const auto languageCode = re.cap(1);
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}

	return languageCode;
}

QString Language::getIconPath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}

	auto filename = Util::sharePath("translations/icons/%1.png").arg(languageCode);
	if(!QFile(filename).exists())
	{
		filename = Util::sharePath("translations/icons/%1.png").arg(languageCode.left(2));
		if(!QFile(filename).exists())
		{
			return QString();
		}
	}

	return filename;
}

QString Language::getChecksum(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode))
	{
		return QString();
	}

	const auto path = getUsedLanguageFile(languageCode);
	return QString::fromUtf8(Util::File::getMD5Sum(path));
}

bool Language::importLanguageFile(const QString& filename)
{
	const auto languageCode = extractLanguageCode(filename);
	if(languageCode.isEmpty())
	{
		return false;
	}

	const auto targetPath = Language::getHomeTargetPath(languageCode);
	auto file = QFile(filename);
	bool success = file.copy(targetPath);
	if(success)
	{
		updateLanguageVersion(languageCode);
	}

	return success;
}

QString Language::getLanguageVersion(const QString& languageCode)
{
	if(!Util::File::exists(getHomeTargetPath(languageCode)))
	{
		LanguageVersionHelper::instance()->setLanguageVersion(languageCode, QString());
		return QString();
	}

	return LanguageVersionHelper::instance()->getLanguageVersion(languageCode);
}

void Language::updateLanguageVersion(const QString& languageCode)
{
	const auto version = Util::File::exists(getHomeTargetPath(languageCode))
	                     ? GetSetting(::Set::Player_Version)
	                     : QString();

	LanguageVersionHelper::instance()->setLanguageVersion(languageCode, version);
}

#ifdef SAYONARA_WITH_TESTS

void Language::setTestMode()
{
	s_test_mode = true;
}

void Language::setLanguageVersion(const QString& languageCode, const QString& version)
{
	LanguageVersionHelper::instance()->setLanguageVersion(languageCode, version);
}

#endif

QLocale Language::getCurrentLocale()
{
	const auto languageCode = GetSetting(::Set::Player_Language);
	return QLocale(languageCode);
}

QStringList Language::getCurrentQtTranslationPaths()
{
	const auto languageCode = GetSetting(::Set::Player_Language);
	if(languageCode.size() < 2)
	{
		return QStringList();
	}

	const auto twoLetter = languageCode.left(2);
	const auto filePrefixes = QStringList
		{
			"qt",
			"qtbase",
			"qtlocation"
		};

	QStringList paths;
	Util::Algorithm::transform(filePrefixes, paths, [twoLetter](const QString& prefix) {
		return QString("%1/%2_%3.qm")
			.arg(QLibraryInfo::location(QLibraryInfo::TranslationsPath))
			.arg(prefix)
			.arg(twoLetter);
	});

	QStringList ret;
	Util::Algorithm::copyIf(paths, ret, [](const auto& path) {
		return (Util::File::exists(path));
	});

	return ret;
}


QString Language::convertOldLanguage(const QString& oldLanguageName)
{
	const auto languages = availableLanguages().keys();
	const auto languageCode = extractLanguageCode(oldLanguageName);
	if(languageCode.size() != 2){
		return "en";
	}

	const auto it = Util::Algorithm::find(languages, [&](const auto& language){
		return (language.startsWith(languageCode) && (languages.size() > 4));
	});

	return (it != languages.end()) ? *it : "en";
}

QMap<QString, QLocale> Language::availableLanguages()
{
	const auto directories = QList<QDir>
		{
			QDir(Util::translationsSharePath()),
			QDir(Util::translationsPath())
		};

	QMap<QString, QLocale> ret;
	for(const auto& directory : directories)
	{
		if(!directory.exists()) {
			continue;
		}

		const auto entries = directory.entryList(QStringList{"*.qm"}, QDir::Files);
		for(const auto& entry : entries)
		{
			const auto key = extractLanguageCode(entry);
			if(!key.isEmpty()) {
				ret[key] = QLocale(key);
			}
		}
	}

	ret.remove("en_US");

	return ret;
}