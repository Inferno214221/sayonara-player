/* FileUtils.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef FileUtils_H
#define FileUtils_H

#include <cstdint>
#include <utility>

class QStringList;
class QString;
class QByteArray;

namespace Util
{
	/**
	 * @brief FileUtils functions
	 * @ingroup FileHelper
	 */
	namespace File
	{
		/**
		 * @brief formatter for filepaths. Removes double separators and replaces them with os specific separators.
		 * @param filename input filename
		 * @return nicely formatted filename
		 */
		QString			clean_filename(const QString& filename);

		/**
		 * @brief calc_file_extension
		 * @param filename
		 * @return
		 */
		QString			calc_file_extension(const QString& filename);

		QByteArray		calc_md5_sum(const QString& filename);

		/**
		 * @brief Remove all files from directory
		 * @param dir_name directory name
		 * @param filters file name filters
		 */
		void			remove_files_in_directory(const QString& dir_name, const QStringList& filters);
		void			remove_files_in_directory(const QString& dir_name);

		/**
		 * @brief Remove all given files (also directories can be specified)
		 * @param files list of files
		 */
		void			delete_files(const QStringList& files);

		/**
		 * @brief get parent directory of a filepath
		 * @param path File- or directory path
		 * @return
		 */
		QString			get_parent_directory(const QString& path);

		/**
		 * @brief extract pure filename from a complete file path
		 * @param path complete file path
		 * @return pure filename
		 */
		QString			get_filename_of_path(const QString& path);

		/**
		 * @brief split filename into the dir and filename
		 * @param src
		 * @param path
		 * @param filename
		 */
		void			split_filename(const QString& src, QString& dir, QString& filename);
		std::pair<QString, QString> split_filename(const QString& src);

		/**
		 * @brief returns all parts of a directory path
		 * @param path
		 * @return
		 */
		QStringList		split_directories(const QString& path);

		/**
		 * @brief get file extension
		 * @param filename filename to get the extension for
		 * @return extension string
		 */
		QString			get_file_extension(const QString& filename);

		/**
		 * @brief extract parent folder of a file list (see also get_parent_directory(const QString& path)
		 * @param list file list
		 * @return List of parent folders
		 */
		QStringList		get_parent_directories(const QStringList& list);

		/**
		 * @brief get absolute filename of file
		 * @param filename
		 * @return
		 */
		QString			get_absolute_filename(const QString& filename);

		/**
		 * @brief create all directories necessary to access path
		 * @param path full target path
		 */
		bool			create_directories(const QString& path);

		/**
		 * @brief create_symlink
		 * @param source
		 * @param target
		 * @return
		 */
		bool			create_symlink(const QString& source, const QString& target);


		/**
		 * @brief convert filesize to string
		 * @param filesize in bytes
		 * @return converted string
		 */
		QString			calc_filesize_str(uint64_t filesize);


		/**
		 * @brief Tell whether filename is absolute
		 * @param filename the filename to check
		 * @return true if filename is absolute, false else
		 */
		bool			is_absolute(const QString& filename);


		/**
		 * @brief Write raw data to file
		 * @param raw_data raw data
		 * @param filename target_filename
		 * @return true if successful, false else
		 */
		bool			write_file(const QByteArray& raw_data, const QString& filename);

		/**
		 * @brief read a complete file into a string
		 * @param filename filename
		 * @param content target reference to content
		 * @return true if file could be read, false else
		 */
		bool read_file_into_str(const QString& filename, QString& content);

		/**
		 * @brief read a complete file into a byte array
		 * @param filename filename
		 * @param content target reference to content
		 * @return true if file could be read, false else
		 */
		bool read_file_into_byte_arr(const QString& filename, QByteArray& content);

		/**
		 * @brief Check, if file is valid. Web URLs are always valid
		 * @param filepath path to file or resource
		 * @return true, if file exists or if Web URL. false else
		 */
		bool check_file(const QString& filepath);

		/**
		 * @brief is_in_sayonara_dir
		 * @param path
		 * @return
		 */
		bool is_in_sayonara_dir(const QString& path);

		/**
		 * @brief get_common_directory
		 * @param paths
		 * @return
		 */
		QString get_common_directory(const QStringList& paths);

		/**
		 * @brief get_common_directory
		 * @param dir1
		 * @param dir2
		 * @return
		 */
		QString get_common_directory(QString dir1, QString dir2);

		/**
		 * @brief create_dir
		 * @param dir_name
		 * @return
		 */
		bool create_dir(const QString& dir_name);

		/**
		 * @brief copy_dir
		 * @param src_dir
		 * @param target_dir
		 * @return
		 */
		bool copy_dir(const QString& src_dir, const QString& target_dir, QString& new_filename);

		/**
		 * @brief move_dir
		 * @param src_dir
		 * @param target_dir
		 * @return
		 */
		bool move_dir(const QString& src_dir, const QString& target_dir, QString& new_filename);

		/**
		 * @brief rename_dir
		 * @param src_dir
		 * @param new_name
		 * @return
		 */
		bool rename_dir(const QString& src_dir, const QString& new_name);

		/**
		 * @brief can_copy_dir
		 * @param src_dir
		 * @param target_dir
		 * @return
		 */
		bool can_copy_dir(const QString& src_dir, const QString& target_dir);

		/**
		 * @brief move_file
		 * @param file
		 * @param dir
		 * @return
		 */
		bool move_file(const QString& file, const QString& dir, QString& new_name);

		/**
		 * @brief copy_file
		 * @param file
		 * @param dir
		 * @return
		 */
		bool copy_file(const QString& file, const QString& dir, QString& new_name);

		/**
		 * @brief move_files
		 * @param files
		 * @param dir
		 * @return
		 */
		bool move_files(const QStringList& files, const QString& dir, QStringList& new_names);

		/**
		 * @brief rename_file
		 * @param old_name
		 * @param new_name
		 * @return
		 */
		bool rename_file(const QString& old_name, const QString& new_name);

		/**
		 * @brief copy_files
		 * @param files
		 * @param dir
		 * @return
		 */
		bool copy_files(const QStringList& files, const QString& dir, QStringList& new_files);


		// Everything clear
		/**
		 * @brief is_url
		 * @param str
		 * @return
		 */
		bool is_url(const QString& str);

		/**
		 * @brief is_www
		 * @param str
		 * @return
		 */
		bool is_www(const QString& str);

		/**
		 * @brief is_file
		 * @param filename
		 * @return
		 */
		bool is_file(const QString& filename);

		/**
		 * @brief is_dir
		 * @param filename
		 * @return
		 */
		bool is_dir(const QString& filename);

		/**
		 * @brief is_soundfile
		 * @param filename
		 * @return
		 */
		bool is_soundfile(const QString& filename);

		/**
		 * @brief is_playlistfile
		 * @param filename
		 * @return
		 */
		bool is_playlistfile(const QString& filename);

		/**
		 * @brief is_imagefile
		 * @param filename
		 * @return
		 */
		bool is_imagefile(const QString& filename);

		/**
		 * @brief exists
		 * @param filename
		 * @return
		 */
		bool exists(const QString& filename);

		/**
		 * @brief Compares two filepaths by cleaning them
		 * @param filename1
         * @param filename2
		 * @return
		 */
		bool is_same_path(const QString& filename1, const QString& filename2);
	}
}

#endif // FileUtils_H
