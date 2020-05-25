/* FileOperations.h */

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

#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include "Utils/Pimpl.h"

#include <QObject>
#include <QStringList>

class FileOperations :
	public QObject
{
	Q_OBJECT

	signals:
		void sigFinished();
		void sigStarted();

	public:
		explicit FileOperations(QObject* parent=nullptr);
		~FileOperations() override;

		bool renamePath(const QString& path, const QString& newName);
		bool renameByExpression(const QString& oldName, const QString& expression) const;
		bool movePaths(const QStringList& paths, const QString& targetDir);
		bool copyPaths(const QStringList& paths, const QString& targetDir);
		bool deletePaths(const QStringList& paths);
		static QStringList supportedReplacementTags();

	private slots:
		void copyThreadFinished();
		void moveThreadFinished();
		void deleteThreadFinished();
};

#endif // FILEOPERATIONS_H
