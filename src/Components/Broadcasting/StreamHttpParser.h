/* StreamHttpParser.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef STREAMHTTPPARSER_H
#define STREAMHTTPPARSER_H

#include "Utils/Pimpl.h"

/**
 * @brief Parses requests out of the Client.
 * @ingroup Broadcasting
 */
class StreamHttpParser
{
	PIMPL(StreamHttpParser)

	public:
		/**
		 * @brief The HttpAnswer enum
		 */
		enum class HttpAnswer :
			unsigned char
		{
			Fail = 0,
			OK,
			Reject,
			Ignore,
			Playlist,
			HTML5,
			MP3,
			BG,
			Favicon,
			MetaData
		};

		StreamHttpParser();
		~StreamHttpParser();

		HttpAnswer parse(const QByteArray& data);

		QString host() const;
		bool isIcyStream() const;

		static QString answerString(HttpAnswer answer);
};

#endif // STREAMHTTPPARSER_H
