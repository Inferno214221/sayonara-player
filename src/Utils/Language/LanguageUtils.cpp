/* LanguageUtils.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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
		QString filepath = Util::sayonara_path("translations/versions");
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

	void set_language_version(const QString& four_letter, const QString& version)
	{
		// never allow to downgrade a downloaded version
		if(is_outdated(four_letter))
		{
			m_settings->setValue(four_letter, version);
		}
	}

	QString get_language_version(const QString& four_letter)
	{
		return m_settings->value(four_letter).toString();
	}

	bool is_outdated(const QString& four_letter)
	{
		QString lv = get_language_version(four_letter);
		QString pv = GetSetting(::Set::Player_Version);

		if(lv.isEmpty()){
			return true;
		}

		return (lv < pv);
	}
};

namespace Language=Util::Language;

static bool check_four_letter(const QString& four_letter)
{
	QRegExp re("^[a-z]{2}_[A-Z]{2}(\\.[A-Z0-9\\-]+[0-9])?$");
	int idx = re.indexIn(four_letter);

	return (idx == 0);
}

QString Language::get_share_path(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}
	return Util::share_path("translations") + "/" + QString("sayonara_lang_%1.qm").arg(four_letter);
}

QString Language::get_ftp_path(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}
	return QString("ftp://sayonara-player.com/translation/sayonara_lang_%1.qm").arg(four_letter);
}

QString Language::get_http_path(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	return QString("https://sayonara-player.com/sw/translation/sayonara_lang_%1.qm").arg(four_letter);
}

QString Language::get_checksum_ftp_path()
{
	return "ftp://sayonara-player.com/translation/checksum";
}

QString Language::get_checksum_http_path()
{
	return "https://sayonara-player.com/sw/translation/checksum";
}


QString Language::get_home_target_path(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	QString translation_dir = Util::sayonara_path("translations");
	if(!Util::File::exists(translation_dir)){
		Util::File::create_dir(Util::sayonara_path("translations"));
	}

	return translation_dir + "/" + QString("sayonara_lang_%1.qm").arg(four_letter);
}

bool Language::is_outdated(const QString& four_letter)
{
	return LanguageVersionHelper::instance()->is_outdated(four_letter);
}

QString Language::get_similar_language_4(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	QString two_letter = four_letter.left(2);
	QStringList translations_paths
	{
		Util::sayonara_path("translations"),
		Util::share_path("translations")
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


QString Language::get_used_language_file(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	{ // check if home path or share path version is better or exists
		if(is_outdated(four_letter)) // not available or older than in share path
		{
			if(Util::File::exists(get_share_path(four_letter)))
			{
				return get_share_path(four_letter);
			}
		}

		if(Util::File::exists(get_home_target_path(four_letter))){
			return get_home_target_path(four_letter);
		}
	}

	{ // try to find from other region
		return get_similar_language_4(four_letter);
	}
}

QString Language::extract_four_letter(const QString& language_file)
{
	QRegExp re(".*sayonara_lang_"
		"([a-z]{2}_[A-Z]{2}(\\.[A-Z0-9\\-]+[0-9])?)\\.(ts|qm)$"
	);

	int idx = re.indexIn(language_file);
	if(idx < 0){
		return QString();
	}

	QString four_letter = re.cap(1);
	if(!check_four_letter(four_letter)){
		return QString();
	}

	return four_letter;
}

QString Language::get_icon_path(const QString& four_letter)
{
	return Util::share_path(QString("translations/icons/%1.png").arg(four_letter));
}

QString Language::get_checksum(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	QString path = get_used_language_file(four_letter);
	return QString::fromUtf8(Util::File::calc_md5_sum(path));
}


QString Language::get_language_version(const QString& four_letter)
{
	if(!Util::File::exists(get_home_target_path(four_letter)))
	{
		LanguageVersionHelper::instance()->set_language_version(four_letter, QString());
		return QString();
	}

	return LanguageVersionHelper::instance()->get_language_version(four_letter);
}

void Language::update_language_version(const QString& four_letter)
{
	QString version = GetSetting(::Set::Player_Version);
	if(!Util::File::exists(get_home_target_path(four_letter)))
	{
		version = QString();
	}

	LanguageVersionHelper::instance()->set_language_version(four_letter, version);
}

#ifdef SAYONARA_WITH_TESTS
	void Language::set_test_mode()
	{
		s_test_mode = true;
	}

	void Language::set_language_version(const QString& four_letter, const QString& version)
	{
		LanguageVersionHelper::instance()->set_language_version(four_letter, version);
	}
#endif


