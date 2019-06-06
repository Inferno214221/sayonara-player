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

class QString;
template<typename A, typename B>
class QMap;

namespace Util
{
	namespace Language
	{
		QString get_share_path(const QString& four_letter);
		QString get_ftp_path(const QString& four_letter);
		QString get_http_path(const QString& four_letter);

		QString get_checksum_ftp_path();
		QString get_checksum_http_path();

		QString get_home_target_path(const QString& four_letter);
		QString get_used_language_file(const QString& four_letter);
		QString get_icon_path(const QString& four_letter);
		QString extract_four_letter(const QString& language_file);
		QString get_checksum(const QString& four_letter);

		QString get_language_version(const QString& four_letter);
		void update_language_version(const QString& four_letter);

		bool is_outdated(const QString& four_letter);
		QString get_similar_language_4(const QString& four_letter);

		#ifdef DEBUG
			void set_test_mode();
			void set_language_version(const QString& four_letter, const QString& version);
		#endif
	}
}

#endif // LANGUAGEUTILS_H
