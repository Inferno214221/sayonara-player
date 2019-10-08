/* LanguageUtils.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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
		 * @param four_letter
		 * @return
		 */
		QString get_share_path(const QString& four_letter);

		/**
		 * @brief get the complete path to a language file out of the ftp directory
		 * @ingroup Language
		 * @param four_letter
		 * @return
		 */
		QString get_ftp_path(const QString& four_letter);

		/**
		 * @brief get the complete http url to a language file
		 * @ingroup Language
		 * @param four_letter
		 * @return
		 */
		QString get_http_path(const QString& four_letter);

		/**
		 * @brief Returns the path where checksums can be fetched from
		 * @ingroup Language
		 * @return ftp url
		 */
		QString get_checksum_ftp_path();

		/**
		 * @brief Returns the path where checksums can be fetched from
		 * @ingroup Language
		 * @return http url
		 */
		QString get_checksum_http_path();

		/**
		 * @brief get_home_target_path
		 * @ingroup Language
		 * @param four_letter
		 * @return
		 */
		QString get_home_target_path(const QString& four_letter);

		/**
		 * @brief Returns the correct language file either from share dir
		 * or home dir
		 * @ingroup Language
		 * @param four_letter language code
		 * @return path to qm language file
		 */
		QString get_used_language_file(const QString& four_letter);

		/**
		 * @brief Returns the icon path in share directory
		 * @ingroup Language
		 * @param four_letter language code
		 * @return
		 */
		QString get_icon_path(const QString& four_letter);

		/**
		 * @brief Extracts the language Code out of a sayonara_lang string
		 * @ingroup Language
		 * @param language_file filename containing sayonara_lang
		 * @return
		 */
		QString extract_four_letter(const QString& language_file);

		/**
		 * @brief calculates the checksum for the currently used language
		 * file (either home or share directory)
		 * @ingroup Language
		 * @param four_letter language code
		 * @return md5 checksum
		 */
		QString get_checksum(const QString& four_letter);

		/**
		 * @brief Get the language version out of translations/versions file
		 * in home dir
		 * @ingroup Language
		 * @param four_letter language code
		 * @return Sayonara version where the file belongs to
		 */
		QString get_language_version(const QString& four_letter);

		/**
		 * @brief Update language version in translations/versions file
		 * to current Sayonara Version
		 * @ingroup Language
		 * @param four_letter
		 */
		void update_language_version(const QString& four_letter);

		/**
		 * @brief Returns if the language version in translations/versions
		 * file is smaller than the current sayonara version
		 * @ingroup Language
		 * @param four_letter
		 * @return false if Sayonara Version <= Language version, true else
		 */
		bool is_outdated(const QString& four_letter);

		/**
		 * @brief get_similar_language_4
		 * @ingroup Language
		 * @param four_letter language code
		 * @return four letter code if there's a language from another region,
		 * empty string else
		 */
		QString get_similar_language_4(const QString& four_letter);

		#ifdef SAYONARA_WITH_TESTS
			void set_test_mode();
			void set_language_version(const QString& four_letter, const QString& version);
		#endif
	}
}

#endif // LANGUAGEUTILS_H
