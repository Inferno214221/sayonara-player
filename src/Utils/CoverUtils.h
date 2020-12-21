/* CoverHelper.h */

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

#ifndef COVERHELPER_H
#define COVERHELPER_H

namespace DB
{
	class Connector;
}

class QString;
class QPixmap;
/**
 * @ingroup Covers
 */

namespace Util
{
	namespace Covers
	{
		enum Source
		{
			Database,
			SayonaraDir,
			Library,
			AudioFile,
			WWW,
			Unknown
		};

		/**
		 * @brief calc_cover_token calculate the hash for a cover
		 * @param artist artist name
		 * @param album album name
		 * @return hash of cover name
		 */
		QString calcCoverToken(const QString& artist, const QString& album);

		void deleteTemporaryCovers();


	}
}

#endif // COVERHELPER_H
