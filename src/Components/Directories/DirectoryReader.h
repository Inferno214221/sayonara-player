/* DirectoryReader.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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
class DirectoryReader final
{
	PIMPL(DirectoryReader)

	public:
		DirectoryReader();
		~DirectoryReader();

		/**
		 * @brief set name filters
		 * @param filter name filters
		 */
		void set_filter(const QStringList& filter);

		/**
		 * @brief set a single name filter
		 * @param filter name filter
		 */
		void set_filter(const QString& filter);

		/**
		 * @brief fetch all files recursively for base_dir. Only files matching the name filter will be extracted
		 * @param base_dir the directory of interest
		 * @param files this array will be filled with the found absolute file paths
		 */
		void files_in_directory_recursive(const QDir& base_dir, QStringList& files) const;


		void files_in_directory(const QDir& base_dir, QStringList& files) const;


		/**
		 * @brief extracts MetaData structures from a list of files
		 * @param paths List of filepaths
		 * @return A list of MetaData extracted from paths
		 */
		MetaDataList metadata_from_filelist(const QStringList& paths);

		/**
		 * @brief Finds files recursively in the given directory with the given filter
		 * @param dir directory to search in
		 * @param filename filenames to find
		 * @return list of all found files
		 */
		QStringList find_files_rec(const QDir& dir, const QString& filename);
};

#endif
