/* LanguageUtils.h */

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

#ifndef LANGUAGEUTILS_H
#define LANGUAGEUTILS_H

#include "Utils/Macros.h"

class QString;
class QStringList;
class QLocale;

template<typename A, typename B>
class QMap;

namespace Util
{
	/**
	 * @brief language namespace
	 * @ingroup Language
	 */
	namespace Language
	{
		/**
		 * @brief get the complete path to a language file out of the share directory
		 * @ingroup Language
		 * @param fourLetter
		 * @return
		 */
		QString getSharePath(const QString& fourLetter);

		/**
		 * @brief get the complete path to a language file out of the ftp directory
		 * @ingroup Language
		 * @param fourLetter
		 * @return
		 */
		QString getFtpPath(const QString& fourLetter);

		/**
		 * @brief get the complete http url to a language file
		 * @ingroup Language
		 * @param fourLetter
		 * @return
		 */
		QString getHttpPath(const QString& fourLetter);

		/**
		 * @brief Returns the path where checksums can be fetched from
		 * @ingroup Language
		 * @return ftp url
		 */
		QString getChecksumFtpPath();

		/**
		 * @brief Returns the path where checksums can be fetched from
		 * @ingroup Language
		 * @return http url
		 */
		QString getChecksumHttpPath();

		/**
		 * @brief get_home_target_path
		 * @ingroup Language
		 * @param fourLetter
		 * @return
		 */
		QString getHomeTargetPath(const QString& fourLetter);

		/**
		 * @brief Returns the correct language file either from share dir
		 * or home dir
		 * @ingroup Language
		 * @param fourLetter language code
		 * @return path to qm language file
		 */
		QString getUsedLanguageFile(const QString& fourLetter);

		/**
		 * @brief Returns the icon path in share directory
		 * @ingroup Language
		 * @param fourLetter language code
		 * @return
		 */
		QString getIconPath(const QString& fourLetter);

		/**
		 * @brief Extracts the language Code out of a sayonara_lang string
		 * @ingroup Language
		 * @param language_file filename containing sayonara_lang
		 * @return four or two letter code
		 */
		QString extractLanguageCode(const QString& languageFile);

		/**
		 * @brief calculates the checksum for the currently used language
		 * file (either home or share directory)
		 * @ingroup Language
		 * @param fourLetter language code
		 * @return md5 checksum
		 */
		QString getChecksum(const QString& fourLetter);

		/**
		 * @brief Get the language version out of translations/versions file
		 * in home dir
		 * @ingroup Language
		 * @param fourLetter language code
		 * @return Sayonara version where the file belongs to
		 */
		QString getLanguageVersion(const QString& fourLetter);

		/**
		 * @brief Update language version in translations/versions file
		 * to current Sayonara Version
		 * @ingroup Language
		 * @param fourLetter
		 */
		void updateLanguageVersion(const QString& fourLetter);

		/**
		 * @brief Returns if the language version in translations/versions
		 * file is smaller than the current sayonara version
		 * @ingroup Language
		 * @param fourLetter
		 * @return false if Sayonara Version <= Language version, true else
		 */
		bool isOutdated(const QString& fourLetter);

		/**
		 * @brief get_similar_language_4
		 * @ingroup Language
		 * @param fourLetter language code
		 * @return four letter code if there's a language from another region,
		 * empty string else
		 */
		QString getSimilarLanguage4(const QString& fourLetter);

		/**
		 * @brief get the current selected locale based on the current language file
		 * @return
		 */
		QLocale getCurrentLocale();

		/**
		 * @brief get all qt paths which include translations
		 * @return
		 */
		QStringList getCurrentQtTranslationPaths();

		/**
		 * @brief Imports a qm file. The filename must contain a valid four
		 * or two letter code
		 * @param filename
		 * @return true, if the filename matches the pattern
		 */
		bool importLanguageFile(const QString& filename);


		/**
		 * @brief Converts two letter into four letter
		 * @param two letter language filename
		 * @return four letter key if available, en_GB per default
		 */
		QString convertOldLanguage(const QString& languageCode);


		/**
		 * @brief Returns all languages located in user path and all
		 * languages in sayonara path
		 * @return map with four letter key as key and the locale as value
		 */
		QMap<QString, QLocale> availableLanguages();

		#ifdef SAYONARA_WITH_TESTS
			void setTestMode();
			void setLanguageVersion(const QString& fourLetter, const QString& version);
		#endif
	}
}

#endif // LANGUAGEUTILS_H
