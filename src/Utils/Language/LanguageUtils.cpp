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
	QSettings* mSettings=nullptr;

	LanguageVersionHelper()
	{
		QString filepath = Util::sayonaraPath("translations/versions");
		if(s_test_mode){
			filepath = "/tmp/versions";
		}

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

		if(lv.isEmpty()){
			return true;
		}

		return (lv < pv);
	}
};

namespace Language=Util::Language;

static bool checkLanguageCode(const QString& languageCode)
{
	QRegExp re("^[a-z]{2}(_[A-Z]{2})?(\\.[A-Z0-9\\-]+[0-9])?$");
	int idx = re.indexIn(languageCode);

	return (idx == 0);
}

QString Language::getSharePath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode)){
		return QString();
	}
	return Util::sharePath("translations") + "/" + QString("sayonara_lang_%1.qm").arg(languageCode);
}

QString Language::getFtpPath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode)){
		return QString();
	}
	return QString("ftp://sayonara-player.com/translation/sayonara_lang_%1.qm").arg(languageCode);
}

QString Language::getHttpPath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode)){
		return QString();
	}

	return QString("https://sayonara-player.com/files/translation/sayonara_lang_%1.qm").arg(languageCode);
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
	if(!checkLanguageCode(languageCode)){
		return QString();
	}

	const QString translationDir = Util::sayonaraPath("translations");
	if(!Util::File::exists(translationDir)){
		Util::File::createDir(Util::sayonaraPath("translations"));
	}

	return translationDir + "/" + QString("sayonara_lang_%1.qm").arg(languageCode);
}

bool Language::isOutdated(const QString& languageCode)
{
	return LanguageVersionHelper::instance()->isOutdated(languageCode);
}

QString Language::getSimilarLanguage4(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode)){
		return QString();
	}

	QString twoLetter = languageCode.left(2);
	QStringList translationsPaths
	{
		Util::sayonaraPath("translations"),
		Util::sharePath("translations")
	};

	for(const QString& translationPath : translationsPaths)
	{
		if(!Util::File::exists(translationPath))
		{
			continue;
		}

		const QDir dir(translationPath);
		const QStringList entries = dir.entryList(QDir::Files);
		const QRegExp re("sayonara_lang_([a-z]{2})_.+qm");

		for(const QString& entry : entries)
		{
			if(re.indexIn(entry) < 0){
				continue;
			}

			const QString entryTwoLetter = re.cap(1);
			if(entryTwoLetter == twoLetter){
				return translationPath + "/" + entry;
			}
		}
	}

	return QString();
}


QString Language::getUsedLanguageFile(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode)){
		return QString();
	}

	{ // check if home path or share path version is better or exists
		if(isOutdated(languageCode)) // not available or older than in share path
		{
			const QString languageInSharePath = getSharePath(languageCode);
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
	const QRegExp re
	(
		".*sayonara_lang_"
		"([a-z]{2}(_[A-Z]{2})?(\\.[A-Z0-9\\-]+[0-9])?)\\.(ts|qm)$"
	);

	int idx = re.indexIn(languageFile);
	if(idx < 0){
		return QString();
	}

	QString languageCode = re.cap(1);
	if(!checkLanguageCode(languageCode)){
		return QString();
	}

	return languageCode;
}

QString Language::getIconPath(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode)){
		return QString();
	}

	QString filename = Util::sharePath("translations/icons/%1.png").arg(languageCode);
	if(!QFile(filename).exists())
	{
		filename = Util::sharePath("translations/icons/%1.png").arg(languageCode.left(2));
		if(!QFile(filename).exists()){
			return QString();
		}
	}

	return filename;
}

QString Language::getChecksum(const QString& languageCode)
{
	if(!checkLanguageCode(languageCode)){
		return QString();
	}

	QString path = getUsedLanguageFile(languageCode);
	return QString::fromUtf8(Util::File::getMD5Sum(path));
}

bool Language::importLanguageFile(const QString& filename)
{
	const QString languageCode = extractLanguageCode(filename);
	if(languageCode.isEmpty()){
		return false;
	}

	const QString targetPath = Language::getHomeTargetPath(languageCode);
	QFile file(filename);
	bool success = file.copy(targetPath);
	if(success){
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
	QString version = GetSetting(::Set::Player_Version);
	if(!Util::File::exists(getHomeTargetPath(languageCode)))
	{
		version = QString();
	}

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
		QString languageCode = GetSetting(::Set::Player_Language);
		return QLocale(languageCode);
	}

	QStringList Language::getCurrentQtTranslationPaths()
	{
		const QString languageCode = GetSetting(::Set::Player_Language);
		if(languageCode.size() < 2){
			return QStringList();
		}

		const QString twoLetter = languageCode.left(2);
		const QStringList filePrefixes
		{
			"qt",
			"qtbase",
			"qtlocation"
		};

		QStringList paths;
		Util::Algorithm::transform(filePrefixes, paths, [twoLetter](const QString& prefix){
			return QString("%1/%2_%3.qm")
						.arg(QLibraryInfo::location(QLibraryInfo::TranslationsPath))
						.arg(prefix)
						.arg(twoLetter);
		});

		QStringList ret;
		for(const QString& path : paths)
		{
			if(Util::File::exists(path)){
				ret << path;
			}
		}

		return ret;
	}
