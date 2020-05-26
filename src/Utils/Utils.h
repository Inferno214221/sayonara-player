/* Helper.h */

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

/*
 * Helper.cpp
 *
 *  Created on: Apr 4, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef UTIL_HELPER_H
#define UTIL_HELPER_H

class QString;
class QDateTime;
class QPixmap;
class QColor;

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
	uint64_t currentDateToInt();
	uint64_t dateToInt(const QDateTime& date);
	QDateTime intToDate(uint64_t date);

	/**
	 * @brief Transform all letters after a space to upper case
	 * @param str input string
	 * @return result string
	 */
	QString stringToFirstUpper(const QString& str);

	/**
	 * @brief Transform only first letter to upper case
	 * @param str input string
	 * @return result string
	 */
	QString stringToVeryFirstUpper(const QString& str);

	/**
	 * @brief Convert milliseconds to string
	 * @param msec milliseconds
	 * @param format $D for days, $H for hours, $M for minutes
	 * $S for secods, A little 'e' behind the number will
	 * result in the unit displayed after the string
	 * @return converted milliseconds
	 */
	QString msToString(MilliSeconds msec, const QString& format);

	QString convertNotNull(const QString& str);


	/**
	 * @brief get sayonara path in home directory
	 * @return
	 */
	QString sayonaraPath();
	QString sayonaraPath(const QString& append_path);


	/**
	 * @brief get share path of sayonara
	 * @return ./share on windows, share path of unix system
	 */
	QString sharePath();
	QString sharePath(const QString& append_path);

	/**
	 * @brief get a temporary directory. usually /tmp/sayonara
	 * @return
	 */
	QString tempPath();
	QString tempPath(const QString& append_path);

	/**
	 * @brief create a link string
	 * @param name appearing name in link
	 * @param target target url (if not given, name is taken)
	 * @param underline if link should be underlined
	 * @return link string
	 */
	QString createLink(const QString& name,
						bool dark=true,
						bool underline=true);

	QString createLink(const QString& name,
						bool dark,
						bool underline,
						const QString& target);

	QString createLink(const QString& name,
						const QColor& color,
						bool underline,
						const QString& target);

	/**
	 * @brief get all supported sound file extensions
	 * @return
	 */
	QStringList soundfileExtensions(bool with_asterisk=true);

	/**
	 * @brief get filter for file reader or file chooser
	 * @return
	 */
	QString soundfileFilter();

	/**
	 * @brief get all supported playlist file extensions
	 * @return
	 */
	QStringList playlistExtensions(bool with_asterisk=true);

	/**
	 * @brief get all supported podcast file extensions
	 * @return
	 */
	QStringList podcastExtensions(bool with_asterisk=true);

	QStringList imageExtensions(bool with_asterisk=true);


	enum Extension
	{
		Soundfile=1<<0,
		Playlist=1<<1,
		Podcast=1<<2,
		Haltdeimaul=1<<3
	};

	using Extensions=uint16_t;

	/**
	 * @brief get filter for file chooser dialog based on extensions
	 * @param extensions disjunction of Extension
	 * @param name name that should appear in the file dialog
	 * @return concatenated list of extensions
	 */
	QString getFileFilter(Extensions extensions, const QString& name);

	/**
	 * @brief get a random val between min max
	 * @param min minimum included value
	 * @param max maximum included value
	 * @return random number
	 */
	int randomNumber(int min, int max);


	QString randomString(int max_chars);


	/**
	 * @brief gets value out of tag
	 * @param tag form: grandparent.parent.child
	 * @param xml_doc content of the xml document
	 * @return extracted string
	 */
	QString easyTagFinder(const QString&  tag, const QString& xml_doc);

	/**
	 * @brief calculate a md5 hashsum
	 * @param data input data
	 * @return hashsum
	 */
	QByteArray calcHash(const QByteArray&  data);


	/**
	 * @brief sleep
	 * @param ms milliseconds to sleep
	 */
	void sleepMs(uint64_t ms);


	/**
	 * @brief get all ip addresses of the host
	 * @return list of ip addresses
	 */
	QStringList ipAddresses();


	QByteArray convertPixmapToByteArray(const QPixmap& pm);
	QByteArray convertPixmapToByteArray(const QPixmap& pm, const char* format);
	QPixmap convertByteArrayToPixmap(const QByteArray& arr);

	/**
	 * @brief set an environment variable. This function is platform independent
	 * @param key variable name
	 * @param value variable value
	 */
	void setEnvironment(const QString& key, const QString& value);
	void unsetEnvironment(const QString& key);
	QString getEnvironment(const char* key);
}

#endif
