/* DirectoryReader.h */

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

#ifndef DIRECTORY_READER
#define DIRECTORY_READER

#include "Utils/Pimpl.h"

class QDir;

/**
 * @brief Directory reader functions
 * @ingroup Helper
 */
class DirectoryReader
{
	PIMPL(DirectoryReader)

	public:
		DirectoryReader();
		DirectoryReader(const QStringList& filter);
		~DirectoryReader();

		void setFilter(const QStringList& filter);
		void setFilter(const QString& filter);

		/**
		 * @brief fetch all files recursively for baseDirOrig. Only files matching the name filter will be extracted
		 * @param baseDirOrig the directory of interest
		 * @param files this array will be filled with the found absolute file paths
		 */
		void scanFilesRecursive(const QDir& baseDirOrig, QStringList& files) const;

		void scanFiles(const QDir& baseDir, QStringList& files) const;

		MetaDataList scanMetadata(const QStringList& fileList);
};

#endif
