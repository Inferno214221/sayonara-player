/* FileOperations.h */

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

#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include "Utils/Pimpl.h"

#include <QThread>

class FileOperationThread : public QThread
{
	Q_OBJECT
	PIMPL(FileOperationThread)

	public:
		enum Mode
		{
			Copy=0,
			Move,
			Rename
		};

		FileOperationThread(QObject* parent, LibraryId target_library_id, const QStringList& source_paths, const QString& target_dir, Mode mode);
		~FileOperationThread() override;

		LibraryId target_library() const;
		QList<StringPair> successful_paths() const;

	protected:
		void run() override;
};

class FileDeleteThread : public QThread
{
	Q_OBJECT
	PIMPL(FileDeleteThread)

	public:
		FileDeleteThread(QObject* parent, const QStringList& source_paths);
		~FileDeleteThread() override;

		QStringList paths() const;

	protected:
		void run() override;
};


class FileOperations :
	public QObject
{
	Q_OBJECT

	signals:
		void sig_finished();
		void sig_started();

	public:
		explicit FileOperations(QObject *parent=nullptr);
		~FileOperations() override;

		bool rename_path(const QString& path, const QString& new_name);
		bool rename_by_expression(const QString& old_name, const QString& expression);
		bool move_paths(const QStringList& paths, const QString& target_dir);
		bool copy_paths(const QStringList& paths, const QString& target_dir);
		bool delete_paths(const QStringList& paths);
		static QStringList supported_tag_replacements();

	private slots:
		void copy_thread_finished();
		void move_thread_finished();
		void delete_thread_finished();
};

#endif // FILEOPERATIONS_H
