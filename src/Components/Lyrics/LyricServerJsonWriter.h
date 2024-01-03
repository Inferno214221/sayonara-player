/* LyricServerJsonWriter.h */

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



#ifndef LYRICSERVERJSONWRITER_H
#define LYRICSERVERJSONWRITER_H

#include <QList>

class QJsonObject;

namespace Lyrics
{
	class Server;
	namespace ServerJsonWriter
	{
		QJsonObject toJson(Lyrics::Server* server);
	}

	namespace ServerJsonReader
	{
		Server* fromJson(const QJsonObject& json);
		QList<Server*> parseJsonFile(const QString& filename);
	}
}

#endif // LYRICSERVERJSONWRITER_H
