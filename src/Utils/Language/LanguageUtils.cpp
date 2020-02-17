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

#include "FileUtils.h"
#include "Settings/Settings.h"
#include "Utils.h"

#include <QFile>
#include <QRegExp>
#include <QDir>
#include <QMap>
#include <QSettings>

static bool s_test_mode = false;

class LanguageVersionHelper
{
	QSettings* m_settings=nullptr;

	LanguageVersionHelper()
	{
		QString filepath = Util::sayonaraPath("translations/versions");
		if(s_test_mode){
			filepath = "/tmp/versions";
		}

		m_settings = new QSettings(filepath, QSettings::NativeFormat);
	}

public:
	~LanguageVersionHelper()
	{
		delete m_settings;
	}

	static LanguageVersionHelper* instance()
	{
		static LanguageVersionHelper hlp;
		return &hlp;
	}

	void set_language_version(const QString& fourLetter, const QString& version)
	{
		// never allow to downgrade a downloaded version
		if(is_outdated(fourLetter))
		{
			m_settings->setValue(fourLetter, version);
		}
	}

	QString get_language_version(const QString& fourLetter)
	{
		return m_settings->value(fourLetter).toString();
	}

	bool is_outdated(const QString& fourLetter)
	{
		QString lv = get_language_version(fourLetter);
		QString pv = GetSetting(::Set::Player_Version);

		if(lv.isEmpty()){
			return true;
		}

		return (lv < pv);
	}
};

namespace Language=Util::Language;

static bool check_fourLetter(const QString& fourLetter)
{
	QRegExp re("^[a-z]{2}_[A-Z]{2}(\\.[A-Z0-9\\-]+[0-9])?$");
	int idx = re.indexIn(fourLetter);

	return (idx == 0);
}

QString Language::getSharePath(const QString& fourLetter)
{
	if(!check_fourLetter(fourLetter)){
		return QString();
	}
	return Util::sharePath("translations") + "/" + QString("sayonara_lang_%1.qm").arg(fourLetter);
}

QString Language::getFtpPath(const QString& fourLetter)
{
	if(!check_fourLetter(fourLetter)){
		return QString();
	}
	return QString("ftp://sayonara-player.com/translation/sayonara_lang_%1.qm").arg(fourLetter);
}

QString Language::getHttpPath(const QString& fourLetter)
{
	if(!check_fourLetter(fourLetter)){
		return QString();
	}

	return QString("https://sayonara-player.com/sw/translation/sayonara_lang_%1.qm").arg(fourLetter);
}

QString Language::getChecksumFtpPath()
{
	return "ftp://sayonara-player.com/translation/checksum";
}

QString Language::getChecksumHttpPath()
{
	return "https://sayonara-player.com/sw/translation/checksum";
}


QString Language::getHomeTargetPath(const QString& fourLetter)
{
	if(!check_fourLetter(fourLetter)){
		return QString();
	}

	QString translation_dir = Util::sayonaraPath("translations");
	if(!Util::File::exists(translation_dir)){
		Util::File::createDir(Util::sayonaraPath("translations"));
	}

	return translation_dir + "/" + QString("sayonara_lang_%1.qm").arg(fourLetter);
}

bool Language::isOutdated(const QString& fourLetter)
{
	return LanguageVersionHelper::instance()->is_outdated(fourLetter);
}

QString Language::getSimilarLanguage4(const QString& fourLetter)
{
	if(!check_fourLetter(fourLetter)){
		return QString();
	}

	QString two_letter = fourLetter.left(2);
	QStringList translations_paths
	{
		Util::sayonaraPath("translations"),
		Util::sharePath("translations")
	};

	for(const QString& translation_path : translations_paths)
	{
		if(!Util::File::exists(translation_path))
		{
			continue;
		}

		QDir dir(translation_path);
		QStringList entries = dir.entryList(QDir::Files);
		QRegExp re("sayonara_lang_([a-z]{2})_.+qm");

		for(const QString& entry : entries)
		{
			if(re.indexIn(entry) < 0){
				continue;
			}
			QString entry_two_letter = re.cap(1);

			if(entry_two_letter == two_letter){
				return translation_path + "/" + entry;
			}
		}
	}

	return QString();
}


QString Language::getUsedLanguageFile(const QString& fourLetter)
{
	if(!check_fourLetter(fourLetter)){
		return QString();
	}

	{ // check if home path or share path version is better or exists
		if(isOutdated(fourLetter)) // not available or older than in share path
		{
			const QString language_in_share_path = getSharePath(fourLetter);
			if(Util::File::exists(language_in_share_path))
			{
				return language_in_share_path;
			}
		}

		if(Util::File::exists(getHomeTargetPath(fourLetter)))
		{
			return getHomeTargetPath(fourLetter);
		}
	}

	{ // try to find from other region
		return getSimilarLanguage4(fourLetter);
	}
}

QString Language::extractFourLetter(const QString& language_file)
{
	QRegExp re(".*sayonara_lang_"
		"([a-z]{2}_[A-Z]{2}(\\.[A-Z0-9\\-]+[0-9])?)\\.(ts|qm)$"
	);

	int idx = re.indexIn(language_file);
	if(idx < 0){
		return QString();
	}

	QString fourLetter = re.cap(1);
	if(!check_fourLetter(fourLetter)){
		return QString();
	}

	return fourLetter;
}

QString Language::getIconPath(const QString& fourLetter)
{
	return Util::sharePath(QString("translations/icons/%1.png").arg(fourLetter));
}

QString Language::getChecksum(const QString& fourLetter)
{
	if(!check_fourLetter(fourLetter)){
		return QString();
	}

	QString path = getUsedLanguageFile(fourLetter);
	return QString::fromUtf8(Util::File::getMD5Sum(path));
}


QString Language::getLanguageVersion(const QString& fourLetter)
{
	if(!Util::File::exists(getHomeTargetPath(fourLetter)))
	{
		LanguageVersionHelper::instance()->set_language_version(fourLetter, QString());
		return QString();
	}

	return LanguageVersionHelper::instance()->get_language_version(fourLetter);
}

void Language::updateLanguageVersion(const QString& fourLetter)
{
	QString version = GetSetting(::Set::Player_Version);
	if(!Util::File::exists(getHomeTargetPath(fourLetter)))
	{
		version = QString();
	}

	LanguageVersionHelper::instance()->set_language_version(fourLetter, version);
}

#ifdef SAYONARA_WITH_TESTS
	void Language::setTestMode()
	{
		s_test_mode = true;
	}

	void Language::setLanguageVersion(const QString& fourLetter, const QString& version)
	{
		LanguageVersionHelper::instance()->set_language_version(fourLetter, version);
	}
#endif


