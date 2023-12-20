/* M3UParser.h */

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

#ifndef M3UPARSER_H
#define M3UPARSER_H

#include "AbstractPlaylistParser.h"

class M3UParser :
	public AbstractPlaylistParser
{
	public:
		M3UParser(const QString& filename,
		          const std::shared_ptr<Util::FileSystem>& fileSystem,
		          const std::shared_ptr<Tagging::TagReader>& tagReader);
		~M3UParser() override;

		static void saveM3UPlaylist(QString filename, const MetaDataList& tracks, bool relative);

	private:
		void parse() override;
};

#endif // M3UPARSER_H
