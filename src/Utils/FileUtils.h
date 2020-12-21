/* FileUtils.h */

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

#ifndef FileUtils_H
#define FileUtils_H

#include <QList>
#include <utility>
#include "typedefs.h"

class QStringList;
class QString;
class QChar;
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
		QString			cleanFilename(const QString& filename);

		QByteArray		getMD5Sum(const QString& filename);

		/**
		 * @brief Remove all files from directory
		 * @param dirName directory name
		 * @param filters file name filters
		 */
		void			removeFilesInDirectory(const QString& dirName, const QStringList& filters);
		void			removeFilesInDirectory(const QString& dirName);

		/**
		 * @brief Remove all given files (also directories can be specified)
		 * @param files list of files
		 */
		void			deleteFiles(const QStringList& files);

		/**
		 * @brief get parent directory of a filepath
		 * @param path File- or directory path
		 * @return
		 */
		QString			getParentDirectory(const QString& path);

		/**
		 * @brief extract pure filename from a complete file path
		 * @param path complete file path
		 * @return pure filename
		 */
		QString			getFilenameOfPath(const QString& path);

		/**
		 * @brief split filename into the dir and filename
		 * @param src
		 * @param path
		 * @param filename
		 */
		void			splitFilename(const QString& src, QString& dir, QString& filename);
		std::pair<QString, QString> splitFilename(const QString& src);

		/**
		 * @brief returns all parts of a directory path
		 * @param path
		 * @return
		 */
		QStringList		splitDirectories(const QString& path);

		/**
		 * @brief get file extension
		 * @param filename filename to get the extension for
		 * @return extension string
		 */
		QString			getFileExtension(const QString& filename);

		/**
		 * @brief extract parent folder of a file list (see also get_parentDirectory(const QString& path)
		 * @param list file list
		 * @return List of parent folders
		 */
		QStringList		getParentDirectories(const QStringList& list);

		/**
		 * @brief get absolute filename of file
		 * @param filename
		 * @return
		 */
		QString			getAbsoluteFilename(const QString& filename);

		/**
		 * @brief create all directories necessary to access path
		 * @param path full target path
		 */
		bool			createDirectories(const QString& path);

		/**
		 * @brief create_symlink
		 * @param source
		 * @param target
		 * @return
		 */
		bool			createSymlink(const QString& source, const QString& target);


		/**
		 * @brief convert filesize to string
		 * @param filesize in bytes
		 * @return converted string
		 */
		QString			getFilesizeString(Filesize filesize);


		/**
		 * @brief Tell whether filename is absolute
		 * @param filename the filename to check
		 * @return true if filename is absolute, false else
		 */
		bool			isAbsolute(const QString& filename);


		QList<QChar>	invalidFilenameChars();


		/**
		 * @brief Write raw data to file
		 * @param raw_data raw data
		 * @param filename target_filename
		 * @return true if successful, false else
		 */
		bool			writeFile(const QByteArray& raw_data, const QString& filename);

		/**
		 * @brief read a complete file into a string
		 * @param filename filename
		 * @param content target reference to content
		 * @return true if file could be read, false else
		 */
		bool readFileIntoString(const QString& filename, QString& content);

		/**
		 * @brief read a complete file into a byte array
		 * @param filename filename
		 * @param content target reference to content
		 * @return true if file could be read, false else
		 */
		bool readFileIntoByteArray(const QString& filename, QByteArray& content);

		/**
		 * @brief Check, if file is valid. Web URLs are always valid
		 * @param filepath path to file or resource
		 * @return true, if file exists or if Web URL. false else
		 */
		bool checkFile(const QString& filepath);

		/**
		 * @brief get_commonDirectory
		 * @param paths
		 * @return
		 */
		QString getCommonDirectory(const QStringList& paths);

		/**
		 * @brief get_commonDirectory
		 * @param dir1
		 * @param dir2
		 * @return
		 */
		QString getCommonDirectory(QString dir1, QString dir2);

		/**
		 * @brief createDir
		 * @param dirName
		 * @return
		 */
		bool createDir(const QString& dirName);

		/**
		 * @brief copyDir
		 * @param srcDir
		 * @param targetDirectory
		 * @return
		 */
		bool copyDir(const QString& srcDir, const QString& targetDirectory, QString& new_filename);

		/**
		 * @brief moveDir
		 * @param srcDir
		 * @param targetDirectory
		 * @return
		 */
		bool moveDir(const QString& srcDir, const QString& targetDirectory, QString& new_filename);

		/**
		 * @brief renameDir
		 * @param srcDir
		 * @param newName
		 * @return
		 */
		bool renameDir(const QString& srcDir, const QString& newName);

		/**
		 * @brief can_copyDir
		 * @param srcDir
		 * @param targetDirectory
		 * @return
		 */
		bool canCopyDir(const QString& srcDir, const QString& targetDirectory);

		/**
		 * @brief move_file
		 * @param file
		 * @param dir
		 * @return
		 */
		bool moveFile(const QString& file, const QString& dir, QString& newName);

		/**
		 * @brief copy_file
		 * @param file
		 * @param dir
		 * @return
		 */
		bool copyFile(const QString& file, const QString& dir, QString& newName);

		/**
		 * @brief move_files
		 * @param files
		 * @param dir
		 * @return
		 */
		bool moveFiles(const QStringList& files, const QString& dir, QStringList& newNames);

		/**
		 * @brief rename_file
		 * @param oldName
		 * @param newName
		 * @return
		 */
		bool renameFile(const QString& oldName, const QString& newName);

		/**
		 * @brief copy_files
		 * @param files
		 * @param dir
		 * @return
		 */
		bool copyFiles(const QStringList& files, const QString& dir, QStringList& new_files);


		// Everything clear
		/**
		 * @brief is_url
		 * @param str
		 * @return
		 */
		bool isUrl(const QString& str);

		/**
		 * @brief is_www
		 * @param str
		 * @return
		 */
		bool isWWW(const QString& str);

		/**
		 * @brief is_file
		 * @param filename
		 * @return
		 */
		bool isFile(const QString& filename);

		/**
		 * @brief isDir
		 * @param filename
		 * @return
		 */
		bool isDir(const QString& filename);

		/**
		 * @brief is_soundfile
		 * @param filename
		 * @return
		 */
		bool isSoundFile(const QString& filename);

		/**
		 * @brief is_playlistfile
		 * @param filename
		 * @return
		 */
		bool isPlaylistFile(const QString& filename);

		/**
		 * @brief is_imagefile
		 * @param filename
		 * @return
		 */
		bool isImageFile(const QString& filename);

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
		bool isSamePath(const QString& filename1, const QString& filename2);

		/**
		 * @brief Checks if dir is a subdir of parentDir
		 * @param dir the dir of interest
		 * @param otherDir the maybe-parentdir
		 * @return
		 */
		bool isSubdir(const QString& dir, const QString& parentDir);
	}
}

#endif // FileUtils_H
