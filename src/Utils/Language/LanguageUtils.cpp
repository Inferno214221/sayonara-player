// clazy:excludeall=qstring-arg
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

namespace Util::Language
{
	namespace
	{
		std::unique_ptr<QSettings> makeLanguageSettings(const QString& filename)
		{
			return std::make_unique<QSettings>(filename, QSettings::NativeFormat);
		}

		QString standardSettingFilename()
		{
			return QDir(Util::translationsPath()).absoluteFilePath("versions");
		}

		class SettingAccessor
		{
			public:
				~SettingAccessor() = default;

				static SettingAccessor& instance()
				{
					static SettingAccessor hlp;
					return hlp;
				}

				void setLanguageSettingSource(const QString& filename)
				{
					mSettings = std::unique_ptr<QSettings>(makeLanguageSettings(filename));
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
					const auto languageVersion = getLanguageVersion(languageCode);
					const auto playerVersion = GetSetting(::Set::Player_Version);

					return languageVersion.isEmpty() || (languageVersion < playerVersion);
				}

			private:
				std::unique_ptr<QSettings> mSettings;

				SettingAccessor() :
					mSettings {makeLanguageSettings(standardSettingFilename())} {}
		};

		bool checkLanguageCode(const QString& languageCode)
		{
			const auto re = QRegExp("^[a-z]{2}(_[A-Z]{2})?(\\.[A-Z0-9\\-]+[0-9])?$");
			const auto idx = re.indexIn(languageCode);

			return (idx == 0);
		}
	}

	QString getSharePath(const QString& languageCode)
	{
		return checkLanguageCode(languageCode)
		       ? Util::sharePath("translations") + "/" + QString("sayonara_lang_%1.qm").arg(languageCode)
		       : QString {};
	}

	QString getFtpPath(const QString& languageCode)
	{
		return checkLanguageCode(languageCode)
		       ? QString("ftp://sayonara-player.com/translation/sayonara_lang_%1.qm").arg(languageCode)
		       : QString {};
	}

	QString getHttpPath(const QString& languageCode)
	{
		return checkLanguageCode(languageCode)
		       ? QString("https://sayonara-player.com/files/translation/sayonara_lang_%1.qm").arg(languageCode)
		       : QString();
	}

	QString getChecksumHttpPath()
	{
		return "https://sayonara-player.com/files/translation/checksum";
	}

	QString getHomeTargetPath(const QString& languageCode)
	{
		if(checkLanguageCode(languageCode))
		{
			const auto translationDir = Util::translationsPath();
			return QString("%1/sayonara_lang_%2.qm")
				.arg(translationDir)
				.arg(languageCode);
		}

		return {};
	}

	bool isOutdated(const QString& languageCode)
	{
		return SettingAccessor::instance().isOutdated(languageCode);
	}

	QString getSimilarLanguage4(const QString& languageCode)
	{
		if(!checkLanguageCode(languageCode))
		{
			return {};
		}

		const auto twoLetter = languageCode.left(2);
		const auto translationsPaths = QStringList {
			Util::translationsPath(),
			Util::sharePath("translations")
		};

		for(const auto& translationPath: translationsPaths)
		{
			if(!Util::File::exists(translationPath))
			{
				continue;
			}

			const auto dir = QDir(translationPath);
			const auto entries = dir.entryList(QDir::Files);
			const auto re = QRegExp("sayonara_lang_([a-z]{2})_.+qm");

			for(const auto& entry: entries)
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

		return {};
	}

	QString getUsedLanguageFile(const QString& languageCode)
	{
		if(!checkLanguageCode(languageCode))
		{
			return {};
		}

		if(isOutdated(languageCode)) // not available or older than in share path
		{
			auto languageInSharePath = getSharePath(languageCode);
			if(Util::File::exists(languageInSharePath))
			{
				return languageInSharePath;
			}
		}

		if(Util::File::exists(getHomeTargetPath(languageCode)))
		{
			return getHomeTargetPath(languageCode);
		}

		// try to find from other region
		return getSimilarLanguage4(languageCode);
	}

	QString extractLanguageCode(const QString& languageFile)
	{
		const auto re = QRegExp(
			".*sayonara_lang_"
			"([a-z]{2}(_[A-Z]{2})?(\\.[A-Z0-9\\-]+[0-9])?)\\.(ts|qm)$");

		if(const auto idx = re.indexIn(languageFile); idx >= 0)
		{
			const auto languageCode = re.cap(1);
			return checkLanguageCode(languageCode)
			       ? languageCode
			       : QString();
		}

		return {};
	}

	QString getIconPath(const QString& languageCode)
	{
		if(checkLanguageCode(languageCode))
		{
			const auto possibleLanguageCodes = std::array {languageCode, languageCode.left(2)};
			for(const auto& possibleLanguageCode: possibleLanguageCodes)
			{
				auto filename = Util::sharePath("translations/icons/%1.png").arg(possibleLanguageCode);
				if(Util::File::exists(filename))
				{
					return filename;
				}
			}
		}

		return {};
	}

	QString getChecksum(const QString& languageCode)
	{
		if(checkLanguageCode(languageCode))
		{
			const auto path = getUsedLanguageFile(languageCode);
			return QString::fromUtf8(Util::File::getMD5Sum(path));
		}

		return {};
	}

	bool importLanguageFile(const QString& filename)
	{
		const auto languageCode = extractLanguageCode(filename);
		if(languageCode.isEmpty())
		{
			return false;
		}

		const auto targetPath = Language::getHomeTargetPath(languageCode);
		auto file = QFile(filename);
		const auto success = file.copy(targetPath);
		if(success)
		{
			updateLanguageVersion(languageCode);
		}

		return success;
	}

	void updateLanguageVersion(const QString& languageCode)
	{
		const auto version = Util::File::exists(getHomeTargetPath(languageCode))
		                     ? GetSetting(::Set::Player_Version)
		                     : QString();

		SettingAccessor::instance().setLanguageVersion(languageCode, version);
	}

#ifdef SAYONARA_WITH_TESTS

	void setLanguageSettingFilename(const QString& filename)
	{
		SettingAccessor::instance().setLanguageSettingSource(filename);
	}

	void setLanguageVersion(const QString& languageCode, const QString& version)
	{
		SettingAccessor::instance().setLanguageVersion(languageCode, version);
	}

#endif

	QLocale getCurrentLocale()
	{
		const auto languageCode = GetSetting(::Set::Player_Language);
		return {languageCode};
	}

	QStringList getCurrentQtTranslationPaths()
	{
		const auto languageCode = GetSetting(::Set::Player_Language);
		if(languageCode.size() < 2)
		{
			return {};
		}

		const auto twoLetter = languageCode.left(2);
		const auto filePrefixes = std::array {
			"qt",
			"qtbase",
			"qtlocation"
		};

		QStringList paths;
		Util::Algorithm::transform(filePrefixes, paths, [&](const QString& prefix) {
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

	QString convertOldLanguage(const QString& oldLanguageName)
	{
		constexpr const auto FallbackLanguageCode = "en";
		const auto languages = availableLanguages().keys();
		const auto languageCode = extractLanguageCode(oldLanguageName);
		if(languageCode.size() != 2)
		{
			return FallbackLanguageCode;
		}

		const auto it = Util::Algorithm::find(languages, [&](const auto& language) {
			return (language.startsWith(languageCode) && (languages.size() > 4));
		});

		return (it != languages.end()) ? *it : FallbackLanguageCode;
	}

	QMap<QString, QLocale> availableLanguages()
	{
		const auto directories = std::array {
			QDir(Util::translationsSharePath()),
			QDir(Util::translationsPath())
		};

		QMap<QString, QLocale> ret;
		for(const auto& directory: directories)
		{
			if(directory.exists())
			{
				const auto entries = directory.entryList(QStringList {"*.qm"}, QDir::Files);
				for(const auto& entry: entries)
				{
					const auto key = extractLanguageCode(entry);
					if(!key.isEmpty())
					{
						ret[key] = QLocale(key);
					}
				};
			}
		}

		ret.remove("en_US");

		return ret;
	}
}