/* FileOperations.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include "Utils/Pimpl.h"

#include <QThread>

class DirectoryCopyThread : public QThread
{
	Q_OBJECT
	PIMPL(DirectoryCopyThread)

public:
	DirectoryCopyThread(QObject* parent, LibraryId target_library_id, const QStringList& source_dirs, const QString& target_dir);
	~DirectoryCopyThread();

	LibraryId target_library() const;

protected:
	void run() override;
};

class FileCopyThread : public QThread
{
	Q_OBJECT
	PIMPL(FileCopyThread)

public:
	FileCopyThread(QObject* parent, LibraryId target_library_id, const QStringList& source_files, const QString& target_dir);
	~FileCopyThread();

	LibraryId target_library() const;

protected:
	void run() override;
};


class FileOperations : public QObject
{
	Q_OBJECT

signals:
	void sig_copy_finished();
	void sig_copy_started();

public:
	explicit FileOperations(QObject *parent = 0);
	~FileOperations();

	bool move_dirs(const QStringList& source_dirs, const QString& target_dir);
	bool copy_dirs(const QStringList& source_dirs, const QString& target_dir);
	bool rename_dir(const QString& source_dir, const QString& target_dir);

	bool move_files(const QStringList& files, const QString& target_dir);
	bool copy_files(const QStringList& files, const QString& target_dir);

	bool rename_file(const QString& old_name, const QString& new_name);

private slots:
	void copy_dir_thread_finished();
	void copy_file_thread_finished();

};

#endif // FILEOPERATIONS_H
