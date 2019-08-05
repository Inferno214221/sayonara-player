/* Helper.h */

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

/*
 * Helper.cpp
 *
 *  Created on: Apr 4, 2011
 *      Author: Lucio Carreras
 */

#ifndef UTIL_HELPER_H
#define UTIL_HELPER_H

class QString;
class QDateTime;
class QPixmap;

#include "typedefs.h"
#include "Utils/Macros.h"

#ifndef CAST_MACROS
	#define scast(x, y) static_cast<x>(y)
	#define dcast(x, y) dynamic_cast<x>(y)
	#define rcast(x, y) reinterpret_cast<x>(y)
	#define CAST_MACROS
#endif

/**
 * @brief Helper functions
 * @ingroup Helper
 */
namespace Util
{
	uint64_t current_date_to_int();
	uint64_t date_to_int(const QDateTime& date);
	QDateTime int_to_date(uint64_t date);

	/**
	 * @brief Transform all letters after a space to upper case
	 * @param str input string
	 * @return result string
	 */
	QString cvt_str_to_first_upper(const QString& str);

	/**
	 * @brief Transform only first letter to upper case
	 * @param str input string
	 * @return result string
	 */
	QString cvt_str_to_very_first_upper(const QString& str);

	/**
	 * @brief Convert milliseconds to string
	 * @param msec milliseconds
	 * @param empty_zero if false, prepend a zero to numbers < 10
	 * @param colon if true, set colon between minutes and seconds
	 * @param show_days if true, days will be shown, too
	 * @return converted milliseconds
	 */
	QString cvt_ms_to_string(MilliSeconds msec, bool empty_zero = false, bool colon=true, bool show_days=true);

	QString cvt_not_null(const QString& str);


	/**
	 * @brief get sayonara path in home directory
	 * @return
	 */
	QString sayonara_path();
	QString sayonara_path(const QString& append_path);


	/**
	 * @brief get share path of sayonara
	 * @return ./share on windows, share path of unix system
	 */
	QString share_path();
	QString share_path(const QString& append_path);

	/**
	 * @brief get library path of sayonara
	 * @return ./lib on windows, lib path of unix system
	 */
	QString lib_path();
	QString lib_path(const QString& append_path);

	/**
	 * @brief create a link string
	 * @param name appearing name in link
	 * @param target target url (if not given, name is taken)
	 * @param underline if link should be underlined
	 * @return link string
	 */
	QString create_link(const QString& name,
						bool dark=true,
						bool underline=true);

	QString create_link(const QString& name,
						bool dark,
						bool underline,
						const QString& target);


	/**
	 * @brief get all supported sound file extensions
	 * @return
	 */
	QStringList soundfile_extensions(bool with_asterisk=true);

	/**
	 * @brief get all supported playlist file extensions
	 * @return
	 */
	QStringList playlist_extensions(bool with_asterisk=true);

	/**
	 * @brief get all supported podcast file extensions
	 * @return
	 */
	QStringList podcast_extensions(bool with_asterisk=true);

	QStringList image_extensions(bool with_asterisk=true);


	/**
	 * @brief get a random val between min max
	 * @param min minimum included value
	 * @param max maximum included value
	 * @return random number
	 */
	int random_number(int min, int max);


	QString random_string(int max_chars);


	/**
	 * @brief gets value out of tag
	 * @param tag form: grandparent.parent.child
	 * @param xml_doc content of the xml document
	 * @return extracted string
	 */
	QString easy_tag_finder(const QString&  tag, const QString& xml_doc);

	/**
	 * @brief calculate a md5 hashsum
	 * @param data input data
	 * @return hashsum
	 */
	QByteArray calc_hash(const QByteArray&  data);


	/**
	 * @brief sleep
	 * @param ms milliseconds to sleep
	 */
	void sleep_ms(uint64_t ms);


	/**
	 * @brief get all ip addresses of the host
	 * @return list of ip addresses
	 */
	QStringList ip_addresses();


	QByteArray cvt_pixmap_to_bytearray(const QPixmap& pm);
	QPixmap cvt_bytearray_to_pixmap(const QByteArray& arr);

	/**
	 * @brief set an environment variable. This function is platform independent
	 * @param key variable name
	 * @param value variable value
	 */
	void set_environment(const QString& key, const QString& value);
	void unset_environment(const QString& key);

}

#endif
