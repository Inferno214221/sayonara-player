/* ASXParser.h */

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

#ifndef SAYONARA_PARSER_ASXPARSER_H
#define SAYONARA_PARSER_ASXPARSER_H

#include "AbstractPlaylistParser.h"

class ASXParser :
	public AbstractPlaylistParser
{
	public:
		ASXParser(const QString& filename,
		          const std::shared_ptr<Util::FileSystem>& fileSystem,
		          const std::shared_ptr<Tagging::TagReader>& tagReader);
		~ASXParser() override;

	protected:
		void parse() override;
};

#endif // SAYONARA_PARSER_ASXPARSER_H
