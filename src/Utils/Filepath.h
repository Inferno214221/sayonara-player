/* Filepath.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef FILEPATH_H
#define FILEPATH_H

#include "Utils/Pimpl.h"

namespace Util
{
	class Filepath
	{
		PIMPL(Filepath)

		public:
			Filepath(const QString& path);
			Filepath(const Filepath& other);
			~Filepath();

			Filepath& operator=(const QString& path);
			Filepath& operator=(const Filepath& path);

			bool operator==(const QString& path) const;
			bool operator==(const Filepath& path) const;

			QString path() const;
			QString fileystemPath() const;

			bool isResource() const;
			bool isFilesystemPath() const;
			bool isUrl() const;
	};
}


#endif // FILEPATH_H
