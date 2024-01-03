/* LyricWebpageParser.h */

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



#ifndef LYRICWEBPAGEPARSER_H
#define LYRICWEBPAGEPARSER_H

#include <QMap>
#include <QString>
#include <QByteArray>

namespace Lyrics
{
	class Server;
	namespace WebpageParser
	{
		QString parseWebpage(const QByteArray& data, const QMap<QString, QString>& regex_conversions, Server* server);
	}
}

#endif // LYRICWEBPAGEPARSER_H
