/* ReloadThreadFileScanner.h */
/*
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

#ifndef SAYONARA_PLAYER_RELOADTHREADFILESCANNER_H
#define SAYONARA_PLAYER_RELOADTHREADFILESCANNER_H

#include <QObject>

#include <memory>

class QDir;
class QString;
class QStringList;
namespace Library
{
	class ReloadThreadFileScanner :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigCurrentDirectoryChanged(const QString& dir);

		public:
			virtual ~ReloadThreadFileScanner() = default;

			virtual QStringList getFilesRecursive(const QDir& baseDir) = 0;
			virtual bool exists(const QString& filename) = 0;
			virtual bool checkFile(const QString& filename) = 0;

			static ReloadThreadFileScanner* create();
	};
}

#endif //SAYONARA_PLAYER_RELOADTHREADFILESCANNER_H
